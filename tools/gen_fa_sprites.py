#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
gen_fa_sprites.py —— 把美术管线的 .i8 索引色成品打成固件资源（manifest 驱动）
================================================================================

输入
    <art>/manifest.json            ★ Job 表的唯一真相：manifest["jobs"]，共 73 条
    <art>/out/<id>.i8              ★ 只读，本脚本绝不写这个目录
    <art>/out/<id>_<n>.i8          （多帧 job）

输出（都在 main/；另有 tools/_gen_fa_sprites_report.txt 记录本次全部对账实数）
    fa_sprites.bin   纯二进制帧数据
    fa_sprites.c     dsc 表 + fa_anims[] + fa_anim_find()
    fa_sprites.h     对外接口 + 38 个 fa_res_* 的 extern

★★ 三条必须守住的（全是踩过的坑）
1) **绝不用 glob 扫 art/out。** 那里除了 264 个正式帧，还堆着 22 个 `_zz_*` 备份副本
   和 5 张实验图（cat_raw / cat_fixed / cat_ref / cat_box / cat_near）。
   一律按 manifest 的 job 表取文件 —— 这是「按名单取」而不是「按目录扫」。
2) **尺寸必须显式给，不能从字节数开方推。** 176×220 = 38720，开方 196.77，
   197² = 38809 ≠ 38720 —— 原来的方形假设会在明信片上直接抛错。
3) **LVGL 的 header.cf / stride / data_size 是一次性的坑**：
   - cf 填错 → **静默不画**（一个像素都不出来，也不报错，和 I8 同一个病）
   - data_size 填大 → 读越界
   - rgb565a8 的布局是 **RGB565 平面在前、A8 平面在后**（不是逐像素交错）
     依据：managed_components/lvgl__lvgl/tests/test_images/stride_align1/UNCOMPRESSED/
           test_RGB565A8_NONE_align1.c —— w=71 h=60 stride=144，实测
           前 144×60 = 8640 字节是颜色平面、其后 71×60 = 4260 字节是 alpha 平面。
           本工程所有宽度都是偶数 → stride = w*2 天然是 4 的倍数，行内无需填充，
           颜色平面 = w*h*2 字节、alpha 平面紧接其后，data_size = w*h*3。

格式（用户 2026-09-18 拍板；2026-09-19 第四次改版把「一个 agent」扩成「一个集合」）
    agent ∈ {postcard, scene-bg}  → RGB565    2 字节/px，无 alpha 平面
    其余                          → RGB565A8  3 字节/px（RGB565 平面 + A8 平面）
    依据不是名字，是「整幅铺满、一个透明像素都没有」这个性质，见 FULLBLEED_AGENTS。

★★ 2026-09-19（第十一轮）改的是**数据来源**，不是格式：
    上面那两类（49 张）改从 <art>/out/<id>.rgb565 直取，不再经过 256 色的 .i8。
    体积一个字节不变（两种都是 2B/px），但天空/雪坡那种大面积渐变不再撒碎点。
    缺 .rgb565 时**直接报错停下**，不退回 .i8（退回是静默降级，见下面的说明）。
    精灵（char-anim / scene-prop / icon-set）照旧走 .i8 —— 它们的色板是画风的一部分。

用法（工程根目录）
    python tools/gen_fa_sprites.py
    python tools/gen_fa_sprites.py --fmt rgb565a8    # 覆盖全局格式（排查用）
    python tools/gen_fa_sprites.py --dry-run         # 只对账、不写文件
    python tools/gen_fa_sprites.py --skip a,b,c      # 跳过若干 job（不写进固件）

★ 素材目录默认按下面 ART_CANDIDATES 依次找（本机的工作副本里没有 art/，
  素材一直住在交接包那棵树下）。找不到会自动往下一个候选走，最后才报错。
  想指定就用 --art，或设环境变量 FA_ART。

★ --skip 是 2026-09-19 加的，用于「下线某个素材但不动 manifest」。
  为什么不动 manifest：manifest 是美术管线的账本（出图记录、质检状态都在里面），
  为了固件瘦身去删条目，等于把出图记录一起删了 —— 将来想回收根本找不回来。
  跳过只影响这次打包：帧照样留在 art/out，随时能加回来。

★★ 2026-09-19 第二次：下线改成**写死在源码里**（见下面的 OFFLINE 常量），
  不再靠命令行参数。命令行的 --skip 现在只用来做临时试验，它**追加**在
  常驻名单之上。理由见 OFFLINE 那一段的说明 —— 靠人记得带参数的东西，
  迟早会有人忘了带，而后果（固件悄悄长一兆）是静默的。

  ★ 想让某条素材重新进固件：把它的 id 从 OFFLINE 里挪走，重跑本脚本。

  当前常驻下线 31 条（约 1.30 MB）：
      snail_a bee_b butterfly_b turtle_a          ← 访客 8 条里剩下的 4 条
      fx_flag fx_burst fx_halo fx_bubble          ← fx 8 条里剩下的 4 条
      item_food item_charm item_gear trophy_a trophy_b souvenir_b clover
      cat_return souvenir_a                    ← 2026-09-19 第六次追加的 2 条
      house tree bookshelf bed table mailbox fence lantern rug
      window stove rocks flowers cloud        ← 2026-09-19 第五次追加的 14 条

  ★★ 2026-09-20（第四十二轮）**取回 fx_rain / fx_snow / fx_steam / fx_sparkle**
     给天气图层当粒子（用户要的「下雨呀之类的三四种天气」）。
     这是本名单**第二次往外套** —— 也正好证了它当初改写成常量的用意：
     「下线」不是「删除」，素材一直在 art/out 里躺着，随时能接回来。
     体积 34,560 B ≈ 33.75 KB，从 app 余量里出（余量还有 351 KB，宽裕）。

  ★★ 2026-09-19（第十六轮）访客组从 8 条收到 4 条 —— `bee_a` / `snail_b` /
     `butterfly_a` / `turtle_b` 被取回去给小游戏当素材（理由与用途逐条写在
     OFFLINE 常量里那个新的注释块中）。这是本名单**第一次往里挪出**条目。
"""

import json
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
PROJ = os.path.dirname(HERE)
MAIN = os.path.join(PROJ, "main")
REPORT = os.path.join(HERE, "_gen_fa_sprites_report.txt")

DEFAULT_ART = r"D:\Espressif\travel-firmware\art"

# ★ 2026-09-19 第四次改版修的：DEFAULT_ART 一直指着**旧工程名** travel-firmware，
#   工作副本换成 faraway-firmware 之后就再也指不到了 —— 不带 --art 跑会直接
#   以「找不到 manifest」退出（而且看起来像素材没了，其实是路径旧了）。
#   现在按这个顺序找第一个真实存在的，谁都不改也能跑。
ART_CANDIDATES = [
    os.environ.get("FA_ART", ""),
    DEFAULT_ART,
    r"D:\文件\去远方-交接包-20260918\去远方-交接包-20260918\03-素材流水线\art",
    os.path.join(os.path.dirname(PROJ), "03-素材流水线", "art"),
]

I8_PAL_BYTES = 1024
ALIGN = 16                     # 每帧起始对齐

FMT_BPP = {"rgb565": 2, "rgb565a8": 3}
FMT_CF = {"rgb565": "LV_COLOR_FORMAT_RGB565", "rgb565a8": "LV_COLOR_FORMAT_RGB565A8"}
POSTCARD_AGENT = "postcard"
POSTCARD_FMT = "rgb565"
DEFAULT_FMT = "rgb565a8"

# ★★ 2026-09-19 第四次改版：走 RGB565 的 agent 从「postcard 一个」变成一个集合。
#   判据不是「叫什么名字」，而是「这张图会不会被叠在别的东西上面」——
#     · postcard  —— 24 张明信片，整幅铺满，本来就不许有透明像素；
#     · scene-bg  —— 首页那张 240x320 整图，同样是整幅铺满、一个透明像素都没有。
#   RGB565 是 2 字节/px，比 RGB565A8 的 3 字节/px 省三分之一。
#   首页那张 240x320 = 76,800 px，选错格式就是白扔 76,800 字节。
#   ⇒ 以后凡是「整幅铺满、不抠底」的新素材，都走 suffix="postcard" 并挂到
#     这个集合里来，别单独去改 job_fmt 的分支。
FULLBLEED_AGENTS = {POSTCARD_AGENT, "scene-bg"}

OUT = []
def log(s=""):
    OUT.append(s)
    try:
        print(s)
    except Exception:
        pass


# --------------------------------------------------------------------- I8 读写

def read_i8(path, w, h):
    """读 .i8 -> (palette[256] of (B,G,R,A) 内存序, indices)

    ★ 布局：[1024 字节调色板][w*h 个 1 字节索引]。调色板每项 4 字节，
      顺序 B,G,R,A —— 与 lv_color32_t 内存布局一致，转码时直接取用。
      索引 0 = 透明。★ 长宽由调用方显式给定，不从字节数开方推。
    """
    with open(path, "rb") as f:
        raw = f.read()
    need = I8_PAL_BYTES + w * h
    if len(raw) != need:
        raise ValueError("%s：实际 %d 字节，期望 %d（= 1024 + %d x %d）"
                         % (os.path.basename(path), len(raw), need, w, h))
    pal = [(raw[i * 4], raw[i * 4 + 1], raw[i * 4 + 2], raw[i * 4 + 3])
           for i in range(256)]
    return pal, raw[I8_PAL_BYTES:]


def _rgb565(pal, v):
    b, g, r, _a = pal[v]
    return ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)


def enc_rgb565(pal, idx):
    """RGB565：2 字节/px，小端。行 stride = w*2（所有宽度为偶数，天然 4 对齐）。"""
    out = bytearray(len(idx) * 2)
    j = 0
    for v in idx:
        x = _rgb565(pal, v)
        out[j] = x & 0xFF
        out[j + 1] = (x >> 8) & 0xFF
        j += 2
    return bytes(out)


def enc_rgb565a8(pal, idx):
    """RGB565A8：颜色平面在前（w*h*2 字节），alpha 平面在后（w*h 字节）。"""
    n = len(idx)
    out = bytearray(n * 3)
    j = 0
    for v in idx:
        x = _rgb565(pal, v)
        out[j] = x & 0xFF
        out[j + 1] = (x >> 8) & 0xFF
        j += 2
    base = n * 2
    for i, v in enumerate(idx):
        out[base + i] = pal[v][3]
    return bytes(out)


def count_transparent(pal, idx):
    return sum(1 for v in idx if pal[v][3] == 0)


# --------------------------------------------------------------------- job 解析

def job_geometry(j):
    """从 manifest 的 job 记录出 (w, h, frames, kind)。"""
    kind = j.get("kind")
    if kind == "sheet":
        c = j.get("cell")
        return c, c, int(j["frames"]), kind
    return int(j["w"]), int(j["h"]), 1, "single"


def job_paths(out_dir, jid, frames):
    if frames > 1:
        return [os.path.join(out_dir, "%s_%d.i8" % (jid, i)) for i in range(frames)]
    return [os.path.join(out_dir, "%s.i8" % jid)]


def job_fmt(j, override):
    if override:
        return override
    return POSTCARD_FMT if j.get("agent") in FULLBLEED_AGENTS else DEFAULT_FMT


# --------------------------------------------------------------------- 常驻下线名单

# ★★ 2026-09-19：这些 job **永久**不进固件，写在源码里而不是靠命令行 --skip。
#   为什么改成写死：--skip 只在这一次打包生效，换个同事、换台机器、或者隔几天
#   重新跑一遍忘了带参数，那一兆多的死素材就全回来了 —— 固件会悄悄长大一兆，
#   而没有任何一处会报错。写在这里，谁跑都一样。
#
#   判定依据：全工程 grep 过一遍，这些 id **没有任何一处** fa_anim_find / fa_res_*
#   引用（唯一例外见下面 souvenir_a 的说明）。
#     访客 4 条        —— 这一组原本 8 条（界面层那三只访客早已整段删除，
#                        见 fa_view.c「访客（已下线）」）；第十六轮**取回 4 条**
#                        给小游戏当素材（兔子靶子 + 三种记忆牌面），
#                        剩下这 4 条仍然一个引用点都没有。
#     fx_* 8 条        —— 特效一次都没接（本来打算做收获卡的光晕，后来用色块代替了）
#     item_* 3 组      —— 便当 / 护身符 / 装备，没有对应的玩法
#     trophy_a/b 2 组  —— 奖杯，没有对应的玩法
#     souvenir_b       —— 纪念品那两套里没用的一套，代码用的是 souvenir_a
#     clover           —— 6 帧三叶草动画；HUD 上那颗是三个圆拼的（mk_box），没用图
#
#   ★ 2026-09-19（第五次）追加 14 条 —— house 与 13 件小院道具：
#       用户原话「去掉道具及布置家的这个功能」。
#       scene-prop 这一组一共 14 件 single（house / tree / bookshelf / bed / table /
#       mailbox / fence / lantern / rug / window / stove / rocks / flowers / cloud），
#       其中 house 从第三次起就叠在首页上（fa_res_house，缩 0.75 摆到 (2,188)），
#       另外 13 件是第四次「布置小院」的候选项。
#       ⚠ 这一版是**真下线**，不是「先藏起来」：
#         ① 首页底图换成了一张**画里自带房子**的图（像素-白天），
#            再叠精灵屋就是两栋房子重影；
#         ② 「布置小院」这个玩法按用户要求整个删除。
#       实测收益：这 14 张不进图集，省下约 200 KB（都在 app 分区里）。
#
#   ⚠ 素材本身一帧都没删（art/out 照旧），manifest 也没动 —— 想回收就把 id 从这里
#     挪走再重跑本脚本。这就是「下线」而不是「删除」。
#   ⚠ 反过来：**删掉这 14 条里的任何一条**，都必须在同一版里把对应的 fa_view.c /
#     fa_game.c 代码删干净，否则编出来是一张谁都用不到的死图（固件悄悄变大）。
OFFLINE = {
    # ★★ 2026-09-19（第十六轮）：原来这 8 条访客素材整组下线。这一轮**取回 4 条**：
    #     bee_a     —— 打兔子小游戏的靶子（帧 0 = 长耳正面兔，正好只露头，适合从洞里冒）
    #     snail_b   —— 记忆翻牌的牌面①（帧 1 = 蜗牛螺旋壳，圆形轮廓）
    #     butterfly_a —— 牌面②（帧 0 = 双翅完全张开，最宽，轮廓最好认）
    #     turtle_b  —— 牌面③（帧 0 = 龟壳 + 兔头，扁长轮廓）
    #   剩下 4 条（snail_a / bee_b / butterfly_b / turtle_a）**继续下线** ——
    #   每个动物只需要一套，另一套没有引用点。想换风格就把 id 对调。
    #   ⚠ 这 8 条的 id 名和画面对不上，是本工程最坑的一件事（记着，别再被名字骗）：
    #     它们画的全是「**兔子 + 某小动物**」的混搭 —— snail = 兔 + 蜗牛壳、
    #     bee = 长耳兔、butterfly = 兔 + 蝶翅、turtle = 兔 + 龟壳。
    #     而 manifest 的 subject 写的是「一只小蜗牛 / 一只圆胖的小蜜蜂 / …」。
    #     用户说的「访客里有兔子」指的就是这个（见第十六轮的交接说明）。
    #   体积：4 条 x 4 帧 x 48x48 x 3B = 110,592 B ≈ 108 KB（app 余量 549 KB -> 441 KB）。
    "snail_a", "bee_b",
    "butterfly_b", "turtle_a",
    # ★★ 2026-09-20（第四十二轮）**取回 4 条** —— 天气图层要用的粒子：
    #     fx_rain    —— 雨
    #     fx_snow    —— 雪
    #     fx_steam   —— 雾（一缕升起的白色水汽）
    #     fx_sparkle —— 星光（一簇小小的星光爆点）
    #   这是本名单**第二次往外套**（第一次见上面第十六轮那 4 条访客）。
    #   用户原话：「给主界面还有旅行界面，加一个天气的图层，弄三四种天气，
    #   然后随机叠加一会儿」—— 素材是现成的，八条 fx 从建表起就在这儿躺着，
    #   原来的注释写的是「特效一次都没接」。这一轮算把它接上线。
    #   体积：4 条 x (4+4+4+8) 帧 x 24x24 x 3B = 34,560 B ≈ 33.75 KB。
    #
    # ★★★ 2026-09-21（第四十五轮）**当天就收回去 3 条** —— 天气图层改版换素材了。
    #   用户原话：「雨再重新画个雨点的动态效果，跟雪一样，也要小一些，现在是色块
    #   不好看」「我感觉天气还得再调整调整、星星呀、雾之类的，都不太行」。
    #   于是 8 条 fx 里**只留 fx_snow**，另外 3 条各有各的替换：
    #     fx_rain    → **fx_drop**    旧素材一格里是一堆 4x4 米白方块，
    #                                缩放出 30px 之后就是一片色块贴纸。
    #                                新素材是 1px 宽的细斜雨丝（头亮尾淡）。
    #     fx_steam   → **不用素材**   雾改用 LVGL 图元画 5 条横长的柔边白带
    #                                （见 fa_view.c 的 wx_fog 那一段）。
    #                                旧素材画的是「竖直的水汽柱」，压在浅色天空上
    #                                读起来是「屏幕上有几缕烟」，不是雾。
    #     fx_sparkle → **fx_firefly** 旧素材是八芒星爆闪（7->138->9 的一炸一收），
    #                                撒在草地上像贴纸。新素材是暖金圆点 + 柔光晕。
    #   ⇒ 这三条只在 art/out 留着（素材一帧没删），**不进图集**。
    #   ⇒ 回收体积：4+4+8 = 16 帧 x 24x24 x 3B = **27,648 B ≈ 27 KB**。
    #   ⚠ 这 3 条在 art/out 里的文件**不能删**：本工程的原则是「下线不删除」，
    #     哪天要换回素材画法，把它们从这行挪走再重跑本脚本即可。
    "fx_rain", "fx_steam", "fx_sparkle",
    # ⚠ 另外 4 条继续下线，理由各不同：
    #     fx_flag  —— 一面小三角旗，不是天气
    #     fx_burst / fx_halo / fx_bubble —— 都是 **64x64** 的大特效（爆闪 / 光环 /
    #       气泡），单帧 12 KB，拿来做雨雪粒子尺寸和体量都不对。
    #       ★ 第四十五轮做「肥皂泡泡」天气时**又查了一遍 fx_bubble**，确认还是不能用：
    #         64x64 且是 8 帧的向上飘气泡（我们要的是 24x24 的向下飘），
    #         尺寸、方向都不对，所以另画了 fx_soap。
    "fx_flag", "fx_burst", "fx_halo", "fx_bubble",
    "item_food", "item_charm", "item_gear",
    "trophy_a", "trophy_b",
    "clover",
    # ---- 2026-09-19（第六次）：两条「谁都引用不到」的死素材 ----
    #   cat_return   8 帧 x 64x64 x 3B = 96 KB
    #        全代码库没有一处 play_set(..., "cat_return")。猫的动作只有
    #        IDLE_POOL / SIT_POOL 两个池 + 出发/归来那几处，这条从建表起就没播过。
    #   ★★ 2026-09-21（第四十七轮）**souvenir_a / souvenir_b 两条一起取回** ——
    #      收藏册新增「纪念品」图鉴（两页 3x4）。这两条原本都在本名单里：
    #        · souvenir_a 是第六次下线的（理由见下），
    #        · souvenir_b 从建表起就在（「两套里没用的一套」）。
    #      现在两套都用上了：a = 地方特产（桂花糕…漆器小碗），
    #                    b = 自然拾遗（海边贝壳…一枚老铜钱），合计 24 件。
    #      ⚠ 尺寸**不是 64x64 了**：改成 44x44（24 帧 x 64x64 x 3B = 288 KB 装不下
    #        —— app 分区只剩 259 KB）。缩法与碎块清理见
    #        03-素材流水线/tools/shrink_icons.py 的文档字符串，那里有完整推导，
    #        包括「为什么不能改 manifest 的 cell 再重跑 pixelize」（实测重跑不可复现）。
    #      ⚠ 体积：24 帧 x 44x44 x 3B = **139,392 B ≈ 136 KB**
    #        （进图集是 RGB565A8，3 B/px；.i8 只是 1 B/px 的中间格式，别拿它算）。
    #   剩下这一条合计 96 KB。检出方式：把 gen_fa_sprites.py 生成的
    #   fa_anims[] 表逐条拿去和 main/*.c 里剥掉注释后的源码对，看谁没被 play_set。
    "cat_return",
    # ---- 2026-09-19（第五次）：布置小院下线，房子与 13 件道具一起去掉 ----
    "house", "tree", "bookshelf", "bed", "table",
    "mailbox", "fence", "lantern", "rug",
    "window", "stove", "rocks", "flowers", "cloud",
}


# --------------------------------------------------------------------- 主流程

def main():
    argv = sys.argv[1:]
    art = None
    fmt_override = None
    dry = False
    skip = set(OFFLINE)
    i = 0
    while i < len(argv):
        a = argv[i]
        if a == "--art" and i + 1 < len(argv):
            art = argv[i + 1]; i += 2
        elif a.startswith("--art="):
            art = a.split("=", 1)[1]; i += 1
        elif a == "--fmt" and i + 1 < len(argv):
            fmt_override = argv[i + 1]; i += 2
        elif a.startswith("--fmt="):
            fmt_override = a.split("=", 1)[1]; i += 1
        elif a == "--skip" and i + 1 < len(argv):
            # ★ 注意是 |= 而不是 =：命令行的 --skip 是**追加**到常驻下线名单上，
            #   不是覆盖。写成 = 会把 OFFLINE 里那几十条全放过，固件白白长大一兆。
            skip |= set(x.strip() for x in argv[i + 1].split(",") if x.strip()); i += 2
        elif a.startswith("--skip="):
            skip |= set(x.strip() for x in a.split("=", 1)[1].split(",") if x.strip()); i += 1
        elif a == "--dry-run":
            dry = True; i += 1
        else:
            sys.exit("不认识的参数：%s" % a)

    if fmt_override and fmt_override not in FMT_BPP:
        sys.exit("--fmt 只能是 rgb565 或 rgb565a8，收到 %r" % fmt_override)

    # ★ 没显式给 --art 就在候选表里挑第一个真有的（见 ART_CANDIDATES 的说明）。
    #   报错时把找过的每一个路径都列出来 —— 只报「找不到 manifest」会让人以为
    #   素材被删了，其实是路径旧了。
    if not art:
        for c in ART_CANDIDATES:
            if c and os.path.exists(os.path.join(c, "manifest.json")):
                art = c
                break
        else:
            sys.exit("找不到 manifest，找过这些位置：\n  " +
                     "\n  ".join(c or "(空)" for c in ART_CANDIDATES) +
                     "\n用 --art 指定素材目录。")

    man_path = os.path.join(art, "manifest.json")
    out_dir = os.path.join(art, "out")
    if not os.path.exists(man_path):
        sys.exit("找不到 manifest：%s" % man_path)
    if not os.path.isdir(out_dir):
        sys.exit("找不到素材目录：%s" % out_dir)

    with open(man_path, "r", encoding="utf-8") as f:
        man = json.load(f)
    jobs = man.get("jobs")
    if not isinstance(jobs, list) or not jobs:
        sys.exit("manifest[\"jobs\"] 不是非空 list，无法继续")

    log("manifest   %s" % man_path)
    log("jobs       %d 条（manifest 顺序，保证可复现）" % len(jobs))
    if skip:
        log("skip       %d 条：%s" % (len(skip), " ".join(sorted(skip))))
    log("art/out    %s" % out_dir)
    log()

    blob = bytearray()
    sheets = []      # (id, w, h, frames, offsets)
    singles = []     # (id, w, h, fmt, off)
    problems = []
    total_px = 0
    pad_total = 0
    by_n = {}

    for j in jobs:
        jid = j.get("id")
        if jid in skip:
            log("  %-16s --- 跳过（下线名单）" % jid)
            continue
        try:
            w, h, frames, kind = job_geometry(j)
        except Exception as e:
            problems.append("%s：manifest 字段不全（%s）" % (jid, e))
            continue

        fmt = job_fmt(j, fmt_override)
        bpp = FMT_BPP[fmt]
        paths = job_paths(out_dir, jid, frames)
        miss = [p for p in paths if not os.path.exists(p)]
        if miss:
            problems.append("%s：缺 %d/%d 帧（例如 %s）"
                            % (jid, len(miss), frames, os.path.basename(miss[0])))
            continue

        # ★★ 2026-09-19（第十一轮）：满幅素材优先取 pixelize 直出的 .rgb565。
        #   为什么：.i8 是 **256 色板**的中间格式，对整幅渐变（天空 / 雪坡 / 沙丘）
        #   是**纯损失** —— 全局色板在那种大面积渐变上只分到十几个色号，源图里
        #   相邻的近似色被甩到相隔很远的两个色号上，平地里就撒出一层异色碎点
        #   （用户报的「明信片和无猫底图脏脏的」，实测碎点率被这一步翻了一倍）。
        #   而 .rgb565 与 .i8 在固件里的**体积完全相同**（都是 2B/px），
        #   所以这一步是白赚的精度。详见 pixelize.py 的 emit_rgb565()。
        #
        #   ⚠ 「找不到就报错」而不是「悄悄退回 .i8」：退回是**静默降级** ——
        #     图照样出、固件照样编、颜色悄悄脏回去，没有任何一处提示。
        #     宁可让打包停下，也不接受这种失败方式。
        #   ⚠ 只认单帧（满幅素材全是 kind=single）：多帧的命名是 <id>_<n>.i8，
        #     直出文件的命名规则还没定，不做。
        direct = None
        if (bpp == 2 and not fmt_override and frames == 1
                and j.get("agent") in FULLBLEED_AGENTS):
            cand = os.path.join(out_dir, jid + ".rgb565")
            if not os.path.exists(cand):
                problems.append(
                    "%s：agent=%s 是满幅素材，但 art/out 里没有 %s.rgb565"
                    "（拿 256 色 .i8 打包会让这一张脏回去）。"
                    "先跑：python tools/pixelize.py batch %s"
                    % (jid, j.get("agent"), jid, jid))
                continue
            need = w * h * 2
            if os.path.getsize(cand) != need:
                problems.append("%s：%s 大小 %d != %d（%dx%d 的 565 应为 %d）"
                                % (jid, os.path.basename(cand),
                                   os.path.getsize(cand), need, w, h, need))
                continue
            direct = cand

        offsets = []
        frame_bytes = 0
        transparent = 0
        bad = None
        for p in paths:
            if direct:
                with open(direct, "rb") as f:
                    enc = f.read()
            else:
                try:
                    pal, idx = read_i8(p, w, h)
                except ValueError as e:
                    bad = str(e)
                    break
                enc = enc_rgb565(pal, idx) if bpp == 2 else enc_rgb565a8(pal, idx)
                if bpp == 3:
                    transparent += count_transparent(pal, idx)
            pad = (-len(blob)) % ALIGN
            if pad:
                blob += b"\x00" * pad
                pad_total += pad
            offsets.append(len(blob))
            blob += enc
            frame_bytes += len(enc)
        if bad:
            problems.append(bad)
            continue

        total_px += w * h * frames
        by_n[jid] = (w, h, frames, fmt, frame_bytes)

        if frames > 1:
            sheets.append((jid, w, h, frames, offsets))
        else:
            singles.append((jid, w, h, fmt, offsets[0]))

        log("  %-16s %3dx%-3d x%-2d %-8s %8d B  累计 %9d B%s"
            % (jid, w, h, frames, fmt, frame_bytes, len(blob),
               "   ★直出565" if direct else ""))
        if bpp == 2 and transparent:
            problems.append("%s：RGB565 无 alpha 平面，但有 %d 个全透明像素"
                            "（会被画成调色板 0 号色）" % (jid, transparent))

    log()
    if problems:
        log("运行中止，未写出任何文件：")
        for m in problems:
            log("  x " + m)
        raise SystemExit(_flush(1))

    # ---------------------------------------------------------------- 对账
    log("=" * 72)
    log("对账")
    log("  总帧数        %d" % sum(v[2] for v in by_n.values()))
    log("  总像素        %d" % total_px)
    log("  ALIGN=%-2d 填充  %d B" % (ALIGN, pad_total))
    log("  fa_sprites.bin %d B（%.3f MiB）" % (len(blob), len(blob) / 1048576.0))
    log("  多帧 job（进 fa_anims[]）%d 条" % len(sheets))
    log("  单张 job（导 fa_res_*）  %d 条" % len(singles))
    log("=" * 72)

    if dry:
        log("--dry-run：不写文件。")
        return _flush(0)

    # ---------------------------------------------------------------- bin
    bin_path = os.path.join(MAIN, "fa_sprites.bin")
    with open(bin_path, "wb") as f:
        f.write(blob)

    # ---------------------------------------------------------------- .c
    L = []
    A = L.append
    A("// 自动生成，勿手改 —— 由 tools/gen_fa_sprites.py 产出（manifest 驱动）。")
    A("// 数据来自 <art>/out/*.i8（精灵）与 <art>/out/*.rgb565（满幅素材），已按 manifest 逐 job 转码：")
    A("//   agent ∈ FULLBLEED_AGENTS -> RGB565 2B/px（直取 .rgb565，不经 256 色板）")
    A("//   其余                      -> RGB565A8（RGB565 平面在前、alpha 平面在后）")
    A("//   不认 I8 索引色：LVGL 9 的软件渲染 blend_to_* 表里只有 I1，没有 I8，")
    A("//   递 I8 进去会落到 default 分支 —— 一个像素都不画、且不报任何错。")
    A('#include "fa_sprites.h"')
    A("")
    A("// ★ EMBED_FILES 生成的符号：fa_sprites.bin 的起始地址（见 main/CMakeLists.txt）。")
    A('extern const uint8_t _binary_fa_sprites_bin_start[] asm("_binary_fa_sprites_bin_start");')
    A("")
    A("#define FA_FRAME_AT(off) (const uint8_t *)(_binary_fa_sprites_bin_start + (off))")
    A("")

    def dsc_block(w, h, fmt, off, lead, tail):
        """一段 lv_image_dsc_t 初始化器。

        lead=True  —— 用于数组元素，自带外层 `{`。
        lead=False —— 用于「整个对象就是一个 dsc」的单张资源：
                      外层 `{` 已经由 `= {` 那行给出，这里绝不能再补一个，
                      否则会多一层括号，编译器会把它当成在初始化第一个标量成员，
                      报 "'lv_image_header_t' has no member named 'header'" +
                      "excess elements in scalar initializer"（2026-09-18 踩过）。
        """
        bpp = FMT_BPP[fmt]
        stride = w * 2
        size = w * h * bpp
        l1 = "{ .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = %s," % FMT_CF[fmt]
        if not lead:
            l1 = ".header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = %s," % FMT_CF[fmt]
        return [
            "    " + l1,
            "                  .flags = 0, .w = %d, .h = %d, .stride = %d }," % (w, h, stride),
            "      .data_size = %d, .data = FA_FRAME_AT(%d) }%s" % (size, off, tail),
        ]

    # 多帧：<id>_dsc[] 静态数组
    for jid, w, h, frames, offsets in sheets:
        fmt = by_n[jid][3]
        A("// ---- %s  %dx%d x%d  %s ----" % (jid, w, h, frames, fmt))
        A("static const lv_image_dsc_t %s_dsc[%d] = {" % (jid, frames))
        for off in offsets:
            L.extend(dsc_block(w, h, fmt, off, True, ","))
        A("};")
        A("")

    # 单张：fa_res_<id>（fa_view.c 的 DEST_CARD[24] 等直接引用）
    A("// ---- 单张资源（kind == single）—— 导出 fa_res_<id> ----")
    for jid, w, h, fmt, off in singles:
        A("// %s  %dx%d  %s" % (jid, w, h, fmt))
        A("const lv_image_dsc_t fa_res_%s = {" % jid)
        L.extend(dsc_block(w, h, fmt, off, False, ";"))
        A("")
        A("")

    A("// ---- 动画总表（只含多帧 job）----")
    A("const fa_anim_t fa_anims[FA_ANIM_COUNT] = {")
    for jid, w, h, frames, offsets in sheets:
        A('    { "%s", %3d, %3d, %2d, %s_dsc },' % (jid, w, h, frames, jid))
    A("};")
    A("")
    A("// ---- 查找 ----")
    A("const fa_anim_t *fa_anim_find(const char *id)")
    A("{")
    A("    if (!id) return NULL;")
    A("    for (int i = 0; i < FA_ANIM_COUNT; i++) {")
    A("        const char *a = fa_anims[i].id;")
    A("        const char *b = id;")
    A("        while (*a && *a == *b) { a++; b++; }")
    A("        if (*a == 0 && *b == 0) return &fa_anims[i];")
    A("    }")
    A("    return NULL;")
    A("}")
    A("")
    c_path = os.path.join(MAIN, "fa_sprites.c")
    with open(c_path, "w", encoding="utf-8", newline="\n") as f:
        f.write("\n".join(L))

    # ---------------------------------------------------------------- .h
    H = []
    B = H.append
    B("// 自动生成，勿手改 —— 由 tools/gen_fa_sprites.py 产出（manifest 驱动）。")
    B("#pragma once")
    B("")
    B('#include "lvgl.h"')
    B("")
    B("// 多帧动画条数（= manifest 里 kind==sheet 的 job 数）。单张资源不在这个表里，")
    B("// 它们各自导出 fa_res_<id>，直接取地址用。")
    B("#define FA_ANIM_COUNT %d" % len(sheets))
    B("")
    B("// 一条动画。帧图像全在 flash 里（EMBED 的 .bin，走 .rodata），不占 RAM。")
    B("typedef struct {")
    B("    const char *id;                 // 与 manifest 的 job id 同名")
    B("    uint8_t w, h;                   // 单帧像素尺寸")
    B("    uint8_t frames;                 // 帧数")
    B("    const lv_image_dsc_t *dsc;      // frames 个描述符，连续排布")
    B("} fa_anim_t;")
    B("")
    B("extern const fa_anim_t fa_anims[FA_ANIM_COUNT];")
    B("")
    B("// 按 id 找动画；找不到返回 NULL。")
    B("const fa_anim_t *fa_anim_find(const char *id);")
    B("")
    B("// ---- 单张资源（kind == single）：%d 条 ----" % len(singles))
    for jid, w, h, fmt, off in singles:
        B("extern const lv_image_dsc_t fa_res_%s;   // %dx%d %s" % (jid, w, h, fmt))
    B("")
    h_path = os.path.join(MAIN, "fa_sprites.h")
    with open(h_path, "w", encoding="utf-8", newline="\n") as f:
        f.write("\n".join(H))

    log("[out ] %s  %d B" % (bin_path, len(blob)))
    log("[out ] %s  %d B" % (c_path, os.path.getsize(c_path)))
    log("[out ] %s  %d B" % (h_path, os.path.getsize(h_path)))
    log("[sum ] %d 条动画 / %d 张单图 / %d 帧 / %d px"
        % (len(sheets), len(singles), sum(v[2] for v in by_n.values()), total_px))
    return _flush(0)


def _flush(code):
    try:
        with open(REPORT, "w", encoding="utf-8", newline="\n") as f:
            f.write("\n".join(OUT) + "\n")
    except Exception:
        pass
    return code


if __name__ == "__main__":
    sys.exit(main())
