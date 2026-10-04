#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""字库覆盖的**直接**验证（不是计数推断）。

做法：把生成出来的 LVGL 字体的 cmap 解出来，与"我自己独立扫源码得到的字符集"逐字对撞。

tiny 的 cmap 结构（由 lv_font_conv 生成）：
  cmaps[0] = FORMAT0_TINY : range_start=32,  range_length=95      → ASCII 32..126
  cmaps[1] = SPARSE_FULL  : range_start=176, unicode_list_1[], list_length=N
                            ⇒ 码点 = 176 + unicode_list_1[i]        （★ 表里存的是**偏移不是码点**）

判据（可证伪）：
  · tiny 的码点集 **恰好等于** 基础组字符集（main/*.c|h 除 fa_talk.c，剥注释、取非 ASCII）
  · ui   的码点集 **恰好等于** 基础组 ∪ 猫语组
  · 阳性：「最高」在 tiny 里（选择页副行用的就是 tiny）
  · 阴性：只在 fa_talk.c 出现的字（如「鸬」）**不在** tiny、**在** ui
"""
import os
import re
import sys

ROOT = r"D:\Games\faraway-fw\main"


def strip_comments(t: str) -> str:
    t = re.sub(r"/\*.*?\*/", "", t, flags=re.S)
    t = re.sub(r"//[^\n]*", "", t)
    return t


def scan(files):
    s = set()
    for f in files:
        t = open(os.path.join(ROOT, f), encoding="utf-8", errors="replace").read()
        for ch in strip_comments(t):
            if ord(ch) > 127:
                s.add(ch)
    return s


def font_codepoints(path: str):
    """从生成的字体 .c 里解出 cmap 覆盖的码点（只取非 ASCII 段）。"""
    t = open(path, encoding="utf-8", errors="replace").read()
    # 找 cmaps[] 块
    m = re.search(r"cmaps\[\]\s*=\s*\{(.*?)\n\};", t, flags=re.S)
    if not m:
        raise SystemExit("找不到 cmaps[] 块: %s" % path)
    body = m.group(1)
    cps = set()
    # 收集 unicode_list_N 的内容
    lists = {}
    for lm in re.finditer(r"unicode_list_(\d+)\[\]\s*=\s*\{(.*?)\};", t, flags=re.S):
        nums = [int(x, 16) for x in re.findall(r"0x[0-9a-fA-F]+", lm.group(2))]
        lists[lm.group(1)] = nums
    # 逐个 cmap
    for cm in re.finditer(r"\{([^{}]*?range_start[^{}]*?)\}", body, flags=re.S):
        seg = cm.group(1)
        rs = int(re.search(r"\.range_start\s*=\s*(\d+)", seg).group(1))
        rl = int(re.search(r"\.range_length\s*=\s*(\d+)", seg).group(1))
        typ = re.search(r"\.type\s*=\s*LV_FONT_FMT_TXT_CMAP_(\w+)", seg)
        typ = typ.group(1) if typ else "?"
        ul = re.search(r"\.unicode_list\s*=\s*unicode_list_(\d+)", seg)
        ln = re.search(r"\.list_length\s*=\s*(\d+)", seg)
        ln = int(ln.group(1)) if ln else 0
        if typ.startswith("SPARSE") and ul:
            arr = lists.get(ul.group(1), [])
            for v in arr[:ln]:
                cps.add(rs + v)
        elif typ.startswith("FORMAT0") and not ul:
            for cp in range(rs, rs + rl):
                cps.add(cp)
    return {cp for cp in cps if cp > 127}


files = [f for f in os.listdir(ROOT) if f.endswith(".c") or f.endswith(".h")]
base_files = sorted(f for f in files if f != "fa_talk.c")
base = scan(base_files)
cat = scan(["fa_talk.c"]) - base

tiny = font_codepoints(os.path.join(ROOT, "fonts", "fa_font_tiny.c"))
ui = font_codepoints(os.path.join(ROOT, "fonts", "fa_font_ui.c"))

# ★ 两边要化成同一种类型再比：字体 cmap 给的是**码点(int)**，源码扫出来的是**字符(str)**。
#   （本脚本第一版就是这里写错了：拿 int 集合和 str 集合比 ⇒ 判据恒 False，
#     但两个差集都印成空 ⇒ 一眼看出是类型问题而不是覆盖问题。）
base_cp = {ord(c) for c in base}
cat_cp = {ord(c) for c in cat}

print("基础组文件 %d 个；基础组字符 %d；猫语组净增 %d" % (len(base_files), len(base), len(cat)))
print("tiny 码点数 = %d ｜ ui 码点数 = %d" % (len(tiny), len(ui)))
print()
print("【判据 1】tiny == 基础组 ? %s" % (tiny == base_cp))
if tiny != base_cp:
    print("   只在 tiny 不在基础组:", "".join(sorted(chr(c) for c in tiny - base_cp))[:80])
    print("   只在基础组不在 tiny:", "".join(sorted(chr(c) for c in base_cp - tiny))[:80])
print("【判据 2】ui == 基础组 ∪ 猫语组 ? %s" % (ui == (base_cp | cat_cp)))
if ui != (base_cp | cat_cp):
    print("   ui 缺:", "".join(sorted(chr(c) for c in (base_cp | cat_cp) - ui))[:80])
    print("   ui 多:", "".join(sorted(chr(c) for c in ui - (base_cp | cat_cp)))[:80])

pos = [("最", 0x6700), ("高", 0x9AD8)]
print()
print("【阳性对照】必须都在 tiny（选择页副行用 tiny）")
for ch, cp in pos:
    print("   %s U+%04X   tiny=%s  ui=%s  (在基础组=%s)" % (ch, cp, cp in tiny, cp in ui, ch in base))

# 阴性对照：只在 fa_talk.c 里的字
neg = [c for c in sorted(cat) if c not in base][:5]
print()
print("【阴性对照】只喂 ui 的字，必须**不在** tiny")
for ch in neg:
    cp = ord(ch)
    print("   %s U+%04X   tiny=%s（应 False）  ui=%s（应 True）" % (ch, cp, cp in tiny, cp in ui))

ok = (tiny == base_cp) and (ui == (base_cp | cat_cp)) and all(cp in tiny for _, cp in pos) \
     and all(ord(c) not in tiny and ord(c) in ui for c in neg)
print()
print("VERDICT =", "ALL PASS" if ok else "FAIL")
sys.exit(0 if ok else 2)
