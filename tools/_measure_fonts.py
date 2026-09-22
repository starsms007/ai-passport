# -*- coding: utf-8 -*-
"""量一下关键字符串在候选字体/字号下的实际宽度，用来决定 fa_font_* 的字号。

为什么必须实测：底栏 pill 只有 36px 宽，而「12时」在 15px 下已经占 31px ——
再加粗 1px 字宽、再放大 1px 字号都可能越界。靠眼睛估必然翻车。
"""
import os
from PIL import ImageFont

FONTS = {
    "REG  ": r"C:\Windows\Fonts\NotoSansSC-VF.ttf",
    "MED  ": r"C:\Windows\Fonts\Noto Sans SC Medium (TrueType).otf",
    "BOLD ": r"C:\Windows\Fonts\Noto Sans SC Bold (TrueType).otf",
    "SERIF": r"C:\Windows\Fonts\STZHONGS.TTF",
}

# (标签, 字符串, 可用宽度)
CASES = [
    ("pill 12时",     "12时",              36),
    ("pill 试玩",     "试玩",              36),
    ("pill 收集",     "收集",              36),
    ("pill 5分",      "5分",               36),
    ("HUD 999",       "999",               46),
    ("batt 100%",     "100%",              38),
    ("batt 56%",      "56%",               38),
    ("mini 接住 0",   "接住 0",            70),
    ("mini 接住 12",  "接住 12",           70),
    ("mini 00:15",    "00:15",             62),
    ("away 旅行中",   "旅行中",           200),
    ("away 00:00",    "00:00",            200),
    ("dexempty",      "还没有收集到东西", 208),
    ("cardhint",      "● 收下　长按设背景", 176),
    ("intro 攒下的猫罐头，能早些回家。", "攒下的猫罐头，能早些回家。", 208),
    ("intro 就寄一张明信片回来。",       "就寄一张明信片回来。",       208),
    ("intro 抬头看了你一眼。",           "抬头看了你一眼。",           208),
    ("dex 最高 12 个", "最高 12 个",      168),
]


def w(font, s):
    return font.getlength(s)


def main():
    have = {k: v for k, v in FONTS.items() if os.path.exists(v)}
    for k in FONTS:
        if k not in have:
            print("  缺失字体 %s = %s" % (k, FONTS[k]))

    for size in (15, 16, 17, 18):
        print("\n===== size %d =====" % size)
        f = {k: ImageFont.truetype(p, size) for k, p in have.items()}
        for label, s, box in CASES:
            row = []
            for k in ("REG  ", "MED  ", "BOLD "):
                if k in f:
                    v = w(f[k], s)
                    row.append("%s %5.1f%s" % (k.strip(), v, "*" if v > box else " "))
            print("  %-32s box=%3d  %s" % (label, box, " | ".join(row)))

    # serif（标题）单独看：小游戏 三个字在 24 / 26 / 28px 下多宽
    if "SERIF" in have:
        print("\n===== 华文中宋（标题字面）=====")
        for size in (20, 22, 24, 26, 28):
            f = ImageFont.truetype(have["SERIF"], size)
            print("  %2dpx  小游戏 %6.1f   去远方 %6.1f" % (size, w(f, "小游戏"), w(f, "去远方")))


if __name__ == "__main__":
    main()
