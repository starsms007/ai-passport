#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""fix_home_sky.py —— 把首页底图（home）的天空压干净。

【为什么需要它：实测出来的因果链】

  1. tools/palette.json 里 sky 家族的 8 档亮度是
         95 / 99 / 108 / 123 / 141 / 164 / 192 / 223
     —— **164 -> 192 之间有一个 28 的断层**。
  2. 首页底图原图的天空主色是 (143,185,228)，lum = **177.3**，**正好卡在断层里**。
  3. 加权最近色（pixelize.py:nearest_index，权重 2/4/3、绿最重）落不下来，
     就把天空拆到四个**色相各不相同**的邻居上：
         sky.5 (130,174,205)  偏蓝
         sky.6 (163,200,224)  中间
         lake.7 (154,208,220) 偏青
         water.7 (168,216,232) 偏青
     四种蓝混在一整片天空里 = 视觉上的「脏」。
  4. 而能落到的那一档 sky.6，蓝味只有原色的一半（b-g 24 对 43）⇒ 顺带**发灰**。

【做法】

  给 home 这一张**单独**补 5 档饱和天蓝，再把天空压成其中一档纯色（默认第 1 档）。

  ★ 为什么可以只改这一张、不动全局色板：
    `gen_fa_sprites.py` 的 `read_i8()` 只读 .i8 文件**自带的**调色板，
    而且只校验字节数，从不比对 palette.json —— 每个 .i8 本来就是
    [1024 字节调色板副本][w*h 索引]。所以把 home **自己用不到的**空闲槽位
    改成天蓝，全局调色板一个字节不动，其余 291 张素材完全不受影响。

【天空怎么认】

  按**颜色**取候选（蓝系），再对候选做连通域，**取最大的那一块**当天空。
  这样窗户玻璃那几小块蓝、水面反光之类不会误伤 —— 它们面积远小于天空。
  比「按索引改」安全（同一个索引在天空和窗户里都可能出现）。

【幂等】

  连跑两次结果一致：判据只看颜色，不看历史；槽位的合法性也允许「本来就已是天蓝」。

用法（工程根目录，或直接给绝对路径）
    python tools/fix_home_sky.py --i8 <art>/out/home.i8            # 只预览，写 PNG 不写 i8
    python tools/fix_home_sky.py --i8 <art>/out/home.i8 --apply    # 真正落盘
    python tools/fix_home_sky.py --i8 ... --pick 3                 # 换一档（0..4，默认 1）
"""
import argparse
import collections
import os
import sys

W, H = 240, 320
PAL_BYTES = 1024                      # 256 项 x 4 字节，顺序 B,G,R,A
REF = (143, 185, 228)                 # 原图天空参考色（实测中位）
TARGET_LUMS = (168, 176, 184, 192, 202)
DEFAULT_SLOTS = (240, 241, 242, 243, 244)
DEFAULT_I8 = (r"D:\文件\去远方-交接包-20260918\去远方-交接包-20260918"
              r"\03-素材流水线\art\out\home.i8")


def lum(c):
    return 0.299 * c[0] + 0.587 * c[1] + 0.114 * c[2]


def tones(base=REF, lums=TARGET_LUMS):
    """按目标亮度反推天蓝档：色相沿用 base 的比例，只缩放亮度。"""
    bl = lum(base)
    out = []
    for L in lums:
        k = L / bl
        out.append(tuple(max(0, min(255, int(round(v * k)))) for v in base))
    return out


def is_blueish(c, b_r_min=22):
    """蓝系。

    b_r_min 从 25 放宽到 22 是量出来的：树把左上角那片天空切成独立小块，
    它在色板里落到偏青的一档（b-r 正好 = 25），卡在 >25 之外就一直漏修，
    在画面上留一块淡青斑。云的下沿（b-r 只有 8~18）离 22 还远，不受影响。
    """
    r, g, b = c
    return b > 140 and (b - r) >= b_r_min and b >= g >= r - 4


def choose_slots(used, rgbp, T, want):
    """挑 want 个槽位写天蓝。

    合法 = 「本图没引用的空槽」 或 「已被引用、但颜色正好就是我们要写的那几档天蓝」。
    后半条是为了**幂等**：脚本跑第二遍时，天空用的那个索引已经被引用了，
    但它的颜色正是上一遍写进去的天蓝，所以照样算合法、挑出来的是同一组槽位。
    """
    if want > len(T):
        sys.exit("want 比天蓝档数还多")
    free = [s for s in range(1, 256) if s not in used]
    ours = [s for s in range(1, 256) if s in used and rgbp[s] in T]
    pool = sorted(set(free) | set(ours))
    if len(pool) < want:
        sys.exit("可用的空闲槽位只有 %d 个，不够 %d 个 —— 先看看图里是不是已经被改过"
                 % (len(pool), want))
    return pool[:want]


def grow_from(seed, idx, rgbp, w, h, sky_lum, dl=48, iters=8):
    """从天空种子往外长，吸收紧邻的「淡色过渡像素」。

    只收同时满足两条的邻居：
      · 不偏红   b >= r
      · 亮度接近  |lum - sky_lum| < dl
    云主体是白色（与天空差 60 以上）吃不掉；红砖顶、绿树、草地都因 b < r 被挡住。
    """
    cur = set(seed)
    for _ in range(iters):
        add = set()
        for p in cur:
            x, y = p % w, p // w
            for dy in (-1, 0, 1):
                ny = y + dy
                if ny < 0 or ny >= h:
                    continue
                for dx in (-1, 0, 1):
                    nx = x + dx
                    if nx < 0 or nx >= w:
                        continue
                    q = ny * w + nx
                    if q in cur or q in add:
                        continue
                    c = rgbp[idx[q]]
                    if c[2] >= c[0] and abs(lum(c) - sky_lum) < dl:
                        add.add(q)
        if not add:
            break
        cur |= add
    return cur


def biggest_component(mask, w, h):
    """8 连通的面积最大连通块，返回它的下标列表。"""
    seen = bytearray(w * h)
    best = []
    for start in range(w * h):
        if not mask[start] or seen[start]:
            continue
        q = collections.deque([start])
        seen[start] = 1
        comp = []
        while q:
            p = q.popleft()
            comp.append(p)
            x, y = p % w, p // w
            for dy in (-1, 0, 1):
                ny = y + dy
                if ny < 0 or ny >= h:
                    continue
                for dx in (-1, 0, 1):
                    nx = x + dx
                    if nx < 0 or nx >= w:
                        continue
                    np_ = ny * w + nx
                    if mask[np_] and not seen[np_]:
                        seen[np_] = 1
                        q.append(np_)
        if len(comp) > len(best):
            best = comp
    return best


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--i8", default=DEFAULT_I8)
    ap.add_argument("--apply", action="store_true")
    ap.add_argument("--pick", type=int, default=1, help="0..4，默认 1（最贴原色那档）")
    # ★ 区域生长的两个参数：宽了会把云的下沿啃掉（云在色板里落到浅灰蓝上，
    #   亮度比天空高 30~55），窄了收不干净太阳那圈光晕。25 / 4 是量出来的折中。
    ap.add_argument("--grow-dl", type=int, default=25, help="亮度容差，默认 25")
    # 轮数给足、靠 `if not add: break` 自然收敛。
    # ★ 只给 4 轮会「没长透」：第一次跑完再跑第二次还能多收 77 px，
    #   结果就不是幂等的了。判据（dl=25）本身已经挡住云，轮数多不会啃云。
    ap.add_argument("--grow-iters", type=int, default=64, help="向外扩张轮数上限，默认 64（跑到不动点）")
    ap.add_argument("--png-dir", default=r"D:\Espressif\_home_out")
    a = ap.parse_args()

    if not (0 <= a.pick < len(TARGET_LUMS)):
        sys.exit("--pick 越界")

    raw = bytearray(open(a.i8, "rb").read())
    if len(raw) != PAL_BYTES + W * H:
        sys.exit("%s：%d 字节，期望 %d" % (a.i8, len(raw), PAL_BYTES + W * H))

    pal = [(raw[i * 4], raw[i * 4 + 1], raw[i * 4 + 2], raw[i * 4 + 3])
           for i in range(256)]          # B,G,R,A
    rgbp = [(p[2], p[1], p[0]) for p in pal]
    idx = bytearray(raw[PAL_BYTES:])
    used = set(idx)

    T = tones()
    slots = choose_slots(used, rgbp, T, len(TARGET_LUMS))

    print("参考色 %s  lum=%.1f  b-g=%d" % (REF, lum(REF), REF[2] - REF[1]))
    for k, (L, c) in enumerate(zip(TARGET_LUMS, T)):
        print("  档%d 目标 lum=%3d -> %-16s 实际 lum=%.0f  b-g=%d%s"
              % (k, L, str(c), lum(c), c[2] - c[1], "   <= 采用" if k == a.pick else ""))
    print("槽位 %s（写前颜色 %s）" % (slots, [str(rgbp[s]) for s in slots]))

    # ---- 认天空 ----
    mask = [is_blueish(rgbp[i]) for i in idx]
    sky = biggest_component(mask, W, H)
    print("蓝系候选中最大连通块 = %d px（占全图 %.1f%%）" % (len(sky), 100.0 * len(sky) / (W * H)))
    inside = [p for p in sky if p < 40 * W]
    print("  其中顶部 40 行内 %d px —— 天空应当绝大部分在这块" % len(inside))

    # 别的蓝块（窗户等）有多大
    comps = _all_components(mask, W, H)
    rest = sorted((len(c) for c in comps if len(c) < len(sky)), reverse=True)[:5]
    print("  其余蓝块前 5 大：%s px（保持不动）" % rest)

    # ---- 补：被树/房子切成另一块的蓝天 ----
    # 树是绿的、不满足蓝系判据，会把左上角那一小片蓝天从主连通块里切开，
    # 于是它逃过第一轮、在画面上留一块偏青的斑。判据：整块都落在上半屏。
    # （窗户玻璃也在 y 170 以下，但它的下沿远超 150，所以进不来。）
    main = set(sky)
    extra = []
    for c in comps:
        if len(c) < 30 or c is sky:
            continue
        if max(p // W for p in c) < 150:
            extra.extend(p for p in c if p not in main)
    if extra:
        print("  另外收进 %d px（被树切开的另一块蓝天）" % len(extra))
        sky = list(main) + extra

    # ---- 区域生长：把紧贴天空的「过渡像素」也收进来 ----
    # 太阳原来有一圈淡黄光晕，量化后落在色板上变成了偏青的灰环；它不满足
    # 「蓝系」判据，所以第一轮漏掉，会在纯蓝天上留下一圈很显眼的脏边。
    # 这里从天空块往外长：只看**紧邻**的像素，且要求「不偏红(b>=r)」且亮度接近天空。
    # 云主体是白色（亮度差 60+）不会被吃掉，云的下沿那点抗锯齿反而会被收干净。
    sky_set = grow_from(sky, idx, rgbp, W, H, lum(T[a.pick]),
                        dl=a.grow_dl, iters=a.grow_iters)
    print("区域生长后 %d px（多收 %d px 的过渡边）" % (len(sky_set), len(sky_set) - len(sky)))
    sky = list(sky_set)

    # ---- 改 ----
    pick_idx = slots[a.pick]
    before_hist = _sky_hist(idx, sky)
    for p in sky:
        idx[p] = pick_idx
    for k, s in enumerate(slots):
        r, g, b = T[k]
        raw[s * 4 + 0] = b
        raw[s * 4 + 1] = g
        raw[s * 4 + 2] = r
        raw[s * 4 + 3] = 255
    raw[PAL_BYTES:] = idx

    after_hist = _sky_hist(bytearray(raw[PAL_BYTES:]), sky)
    print("")
    print("天空区改前 %d 种色 -> 改后 %d 种色" % (len(before_hist), len(after_hist)))
    print("  改前主色：" + "  ".join("%s x%d" % (k, v) for k, v in
                                   sorted(before_hist.items(), key=lambda kv: -kv[1])[:5]))
    print("  改后主色：" + "  ".join("%s x%d" % (k, v) for k, v in
                                   sorted(after_hist.items(), key=lambda kv: -kv[1])[:5]))

    # ---- 导 PNG 预览 ----
    try:
        from PIL import Image
        os.makedirs(a.png_dir, exist_ok=True)
        img = Image.new("RGB", (W, H))
        img.putdata([_px(raw, i) for i in range(W * H)])
        img.save(os.path.join(a.png_dir, "home_fixed_1x.png"))
        img.resize((W * 3, H * 3), Image.NEAREST).save(
            os.path.join(a.png_dir, "home_fixed_3x.png"))
        img.crop((0, 0, W, 132)).resize((W * 4, 132 * 4), Image.NEAREST).save(
            os.path.join(a.png_dir, "home_fixed_top4x.png"))
        print("")
        print("已存 %s 下的 home_fixed_1x / _3x / _top4x.png" % a.png_dir)
    except ImportError:
        print("（没装 Pillow，跳过 PNG 预览）")

    if a.apply:
        open(a.i8, "wb").write(bytes(raw))
        print("")
        print("★ 已写回 %s（%d 字节）" % (a.i8, len(raw)))
    else:
        print("")
        print("（预览模式，没写 i8 —— 加 --apply 才落盘）")


def _px(raw, i):
    """从 raw 里取第 i 个像素的 RGB（索引查自带调色板）。"""
    v = raw[PAL_BYTES + i]
    b, g, r = raw[v * 4], raw[v * 4 + 1], raw[v * 4 + 2]
    return (r, g, b)


def _all_components(mask, w, h):
    seen = bytearray(w * h)
    out = []
    for start in range(w * h):
        if not mask[start] or seen[start]:
            continue
        q = collections.deque([start])
        seen[start] = 1
        comp = []
        while q:
            p = q.popleft()
            comp.append(p)
            x, y = p % w, p // w
            for dy in (-1, 0, 1):
                ny = y + dy
                if ny < 0 or ny >= h:
                    continue
                for dx in (-1, 0, 1):
                    nx = x + dx
                    if nx < 0 or nx >= w:
                        continue
                    np_ = ny * w + nx
                    if mask[np_] and not seen[np_]:
                        seen[np_] = 1
                        q.append(np_)
        out.append(comp)
    return out


def _sky_hist(idx, sky):
    h = {}
    for p in sky:
        h[idx[p]] = h.get(idx[p], 0) + 1
    return h


if __name__ == "__main__":
    main()
