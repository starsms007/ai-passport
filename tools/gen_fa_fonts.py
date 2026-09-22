#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
gen_fa_fonts.py —— 生成《去远方》固件用的中文字库
==================================================

扫 main/*.c main/*.h 里出现的非 ASCII 字符（汉字 + 全角标点 + ●），调 lv_font_conv
生成 5 个 LVGL 字库：

    字库             源字体                     字号  bpp  字符集
    fa_font_title    华文中宋 STZHONGS           24   4    TITLE_TEXT + ASCII
    fa_font_ui       思源黑 Noto Sans SC **Bold**  16   4    全字符集 + **fa_talk.c** + ASCII
    fa_font_tiny     思源黑 Noto Sans SC **Medium** 13   4    全字符集（**不含 fa_talk.c**）+ ASCII
    fa_font_game     思源黑 Noto Sans SC **Bold**  24   4    GAME_TEXT（小游戏那一族的 24px 大字）
    fa_font_small    思源黑 Noto Sans SC **Medium** 12   4    同 tiny（第四十六轮新增 · 设置页专用）

★★ 2026-09-20（第十八轮）**字符集分成了两组**，原因是一次很实在的账：
   `main/fa_talk.c` 是「猫咪对我说的话」那 290 句（50 句通用 + 24 处风景各 10 句），
   它带来约 330 个新字形。这些字**只有主界面那个气泡会用**，而气泡用的是
   fa_font_ui（16px）—— fa_font_tiny（13px）一个都用不上。
   两支字库的字形单价差不多（16px 约 136 B、13px 约 85 B），
   要是照老写法「全字符集」两边都收，等于白掏约 28 KB。
   ⇒ 现在扫描分两组：
        基础组 = main/*.c main/*.h（除 fa_talk.c）  → ui 与 tiny 都要
        猫语组 = 仅 fa_talk.c                        → **只有 ui 要**
   ⚠ 往 fa_talk.c 里写句子时记着这条：**那 290 句只能用 16px 显示**。
     哪天想改成 13px 显示，先把这个分组去掉（否则真机上方框）。

★ 2026-09-19（第十四轮）新增 fa_font_game：小游戏那一族界面用的大字。
★ 2026-09-19（第十六轮）它的字面从「接罐头」换成「小游戏新获得？石剪布」——
  因为封面标题改成单行「小游戏」，而且这一轮新加了「新获得」闪字、
  猜拳的三张牌（石/剪/布）与猫那一格的「？」，全挂在这支字库上。

★ 2026-09-19（第十三轮）字号与字重调过一次，理由是用户实机反馈
  「尽可能的把字体再放大一些，加粗一些…现在有些太小了」。
  15 -> 16 和 11 -> 13 都是**实测出来的上限**，不是随手取的：
      fa_font_ui 若到 17px，底栏 36px 的格子装不下「12时」（37px），
      开场正文 208px 的框也装不下最长那句（221px）；
      fa_font_tiny 到 13px 时用 Medium 而不是 Bold —— 中文在 13px 笔画已经很密，
      Bold 会把「攒」「藏」这类字糊成一团墨。改之前先跑一遍
      `python tools/_measure_fonts.py` 量宽度，别凭感觉调。

为什么扫源码而不手写字符表：
    漏一个字，屏幕上就是一个方框。手写清单迟早会漏 —— 扫源码则
    「代码里写的每个字都能显示」是构造性保证的。新增文案后重跑本脚本即可。

用法（工程根目录）：
    python tools/gen_fa_fonts.py

★ 新增一支字库时，**必须**同时在 main/CMakeLists.txt 的 SRCS 里补一行
  （那里是显式列举，不是 GLOB）。本脚本末尾的 check_cmakelists() 会硬校验
  这件事，漏了就 exit 1 —— 详见该函数的注释。
"""

import os
import re
import shutil
import subprocess
import sys

# ---------------------------------------------------------------- 路径

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
MAIN = os.path.join(ROOT, "main")
FONTDIR = os.path.join(MAIN, "fonts")

# ★ 2026-09-19 换机修正：原来这里硬编码的是**旧机器**的 node 绝对路径
#   （C:\Users\liuruopeng\...），换机后脚本会直接以「lv_font_conv 不可用」退出。
#   现在按 环境变量 → PATH → 本机已知路径 依次找。
#
#   lv_font_conv 没装的话（新机器第一次跑必然没有）：
#       mkdir D:\Espressif\_fonttools
#       cd    D:\Espressif\_fonttools
#       npm install lv_font_conv --registry=https://registry.npmmirror.com
#   走 npmmirror 是因为本机实测官方 registry 很慢；github.com 在这台机器上
#   完全不可达，不要试图从 GitHub 取这个包。
NODE = (os.environ.get("FA_NODE")
        or shutil.which("node")
        or r"C:\Users\74680\.workbuddy\binaries\node\versions\22.22.2-3\node.exe")
LVFC = os.environ.get(
    "FA_LVFC",
    r"D:\Espressif\_fonttools\node_modules\lv_font_conv\lv_font_conv.js")

SERIF = os.environ.get("FA_SERIF", r"C:\Windows\Fonts\STZHONGS.TTF")       # 华文中宋
SANS = os.environ.get("FA_SANS", r"C:\Windows\Fonts\NotoSansSC-VF.ttf")    # 思源黑

# ★★ 2026-09-19（第十三轮）新加两个**静态字重**文件。
#   为什么不直接用上面那份可变字重 NotoSansSC-VF.ttf 调 wght 轴：
#   lv_font_conv 底下是 opentype.js，可变字体它只取**默认实例**（Regular），
#   轴上要的粗细根本读不出来 —— 喂进去只会得到又一份 Regular。
#   所以改用同族的静态字重文件：字族仍是思源黑，只是笔画更重，
#   「字体跟时间选择字体一致」这条因此仍然成立。
#   ⚠ 这两份的扩展名是 **.otf**（CFF 轮廓），跟常规的 .ttf 不是一套。
#     生成完别只看退出码 —— 用脚本末尾那段「真实位图 KB」确认真的出字形了。
SANS_MED = os.environ.get(
    "FA_SANS_MED", r"C:\Windows\Fonts\Noto Sans SC Medium (TrueType).otf")
SANS_BOLD = os.environ.get(
    "FA_SANS_BOLD", r"C:\Windows\Fonts\Noto Sans SC Bold (TrueType).otf")

NONASCII = re.compile(r"[^\x00-\x7F]")
ASCII_RANGE = "0x20-0x7E"

# ★ 第十八轮：非 fonts/ 的**手写数据文件**，必须登记进 main/CMakeLists.txt。
#   fa_talk.c =「猫咪对我说的话」那 290 句（由 _dl/_talk18.py 产出）。
EXTRA_SRCS = ("fa_talk.c",)

# ★ fa_font_title（24px 华文中宋）只收录这里列出的字 + ASCII。
#   开场剧情的标题用的就是这个字面（见 fa_view.c 的 INTRO_TITLE 表），
#   所以这份清单必须和那边**同步** —— 漏一个字，屏幕上就是一个方框。
#   ★ 第十三轮新增「小游戏」：收藏册里那一格的封面标题也走这个字面
#     （见 fa_view.c 的 U.dex_mask_t）。它**不在**开场标题表里，
#     但走的是同一个 LVGL 字体对象，所以必须并进这份清单。
TITLE_TEXT = ("去远方"
              "又是一天"        # 开场第 1 页标题
              "小猫站了出来"     # 开场第 2 页标题
              "它会带礼物回来"   # 开场第 3 页标题
              "小游戏")         # 收藏册 → 小游戏那一格的封面标题（小字那段）

# ★★★ 2026-09-19（第十四轮）新增第 4 支字库 fa_font_game。
#   它是**小游戏**那一族界面专用的大字（24px 思源黑 Bold）：
#     ① 收藏册里那一格的封面标题（现在写「小游戏」）
#     ② 小游戏选择页的标题「小游戏」
#     ③ 到达卡上「新获得」那三个字（第十六轮新增）
#     ④ 剪刀石头布：猫那一格的「？」与玩家三张牌的「石 / 剪 / 布」
#   ★ 为什么不是复用 fa_font_title（也是 24px）：那是**华文中宋**，
#     而游戏这一族选定的款式是「黑体 Bold」（第十三轮候选图里的 B 款）。
#   ⚠ 「小游戏」这三个字**仍然留在 TITLE_TEXT 里**：fa_font_title 还挂在开场
#     剧情的大标题上（fa_view.c 的 U.intro_title）。
#   ★★ 第十六轮把字面换了一轮：
#     ~~"接罐头"~~（那是旧标题「接罐头 + 小游戏」两段并排时的大字段，
#       这一轮封面改成单行「小游戏」，这三个字**已经没有地方用**了）
#     -> "小游戏新获得？石剪布"
#     ⚠ ？ 是全角问号 U+FF1F（不是 ASCII 的 ?），别顺手写成半角 ——
#       写错了这一格就是个方框，而且**编译期一点提示都没有**。
#     ⚠ 加一个字大约 (24*24)/2 = 288 字节，这支字库小到可以忽略；
#       但**改了字面就必须重跑本脚本**，否则真机缺字（方框），
#       而且没人会立刻发现 —— 「新获得」只有收新明信片那一瞬间才出现。
#       判据：改过 fa_view.c 里任何挂 fa_font_game 的字符串，就回来核对这四个地方。
# ★★★ 2026-09-20（第二十轮）：新增「**设置**」两个字。
#   收藏册「小游戏」那一格的图位改成了**上下两格**（上 2/3 小游戏 · 下 1/3 设置），
#   下格那个「设置」用的是**这支 24px 字库**（见 fa_view.c 的 U.dex2_dn_t1）。
#   ⚠ 加字之后**必须重跑本脚本**：不重跑，真机上就是两个方框，
#     而且编译期一点提示都没有（字库里没有这个字形，LVGL 画空）。
#   ⚠ 这两个字**不是**走 fa_font_ui 的 —— 虽然 ui 字库（16px）是扫源码自动收的、
#     写进 fa_view.c 就有了，但设计稿上「设置」是 24px 的大字，
#     与上面的「小游戏」同一号，所以只能进这支固定字表。
GAME_TEXT = "小游戏新获得？石剪布设置"

# ★ 必须在取字符前剥掉注释。
#   注释里写的是给人看的中文，屏幕上一个都不会显示。把它们算进字符集，
#   字符数会涨好几倍，字库体积跟着翻，白占 flash。
BLOCK_COMMENT = re.compile(r"/\*.*?\*/", re.S)
LINE_COMMENT = re.compile(r"//[^\n]*")


def strip_comments(text):
    return LINE_COMMENT.sub("", BLOCK_COMMENT.sub("", text))


def read(p):
    with open(p, "r", encoding="utf-8") as f:
        return f.read()


# ★ 第十八轮：这一支是「只有 16px 字库要收」的字符来源（见文件开头那段账）。
#   ⚠ 名字写错/文件改名 -> 这里静默少收一批字 -> 真机上是一排方框，
#     所以下面 scan_sources() 对它的存在性做硬校验（找不到就 exit 1）。
TALK_SRC = "fa_talk.c"


def scan_sources():
    """扫 main/*.c main/*.h 的非 ASCII（剥注释，跳过生成物）。

    返回 (base_chars, talk_chars)：
        base_chars —— 除 fa_talk.c 以外全部源文件的字（ui 与 tiny 都要）
        talk_chars —— 仅 fa_talk.c 的字（**只有 ui 要**）
    """
    skip = {"fa_sprites.c", "fa_sprites.h", "fa_fonts.h", TALK_SRC}
    chars = set()
    files = []
    for fn in sorted(os.listdir(MAIN)):
        if not fn.endswith((".c", ".h")) or fn in skip:
            continue
        files.append(fn)
        chars |= set(NONASCII.findall(strip_comments(read(os.path.join(MAIN, fn)))))

    talk_path = os.path.join(MAIN, TALK_SRC)
    if not os.path.exists(talk_path):
        sys.exit("找不到 %s —— 它应该在 main/ 下（猫咪对我说的话那 290 句）。"
                 "文件被删/改名的话，要么把它放回来，要么删掉本脚本里的分组。" % TALK_SRC)
    talk = set(NONASCII.findall(strip_comments(read(talk_path))))
    talk -= chars          # 只留「别的文件里没有」的那些，报数才是净增

    print("[scan] 源文件 %d 个：%s" % (len(files), " ".join(files)))
    print("[scan] 基础组非 ASCII 字符 %d 个" % len(chars))
    print("[scan] 猫语组（%s）净增 %d 个：%s"
          % (TALK_SRC, len(talk), "".join(sorted(talk))))
    return chars, talk


def run_font(name, src_font, size, symbols, with_ascii):
    out = os.path.join(FONTDIR, name + ".c")

    if not os.path.exists(NODE) or not os.path.exists(LVFC):
        sys.exit("lv_font_conv 不可用：\n  node = %s\n  conv = %s" % (NODE, LVFC))
    if not os.path.exists(src_font):
        sys.exit("源字体不存在：%s" % src_font)

    cmd = [NODE, LVFC,
           "--font", src_font,
           "--size", str(size),
           "--bpp", "4",
           "--format", "lvgl",
           # ★★ 必须关压缩，否则真机上一个字都不显示（2026-09-17 踩过）
           #
           #  1) lv_font_conv 1.5.3 默认输出「字节级 RLE」压缩位图，
           #     font_dsc 里 bitmap_format = 1（LV_FONT_FMT_TXT_COMPRESSED）。
           #  2) LVGL 9 默认不开 LV_USE_FONT_COMPRESSED，
           #     lv_font_get_bitmap_fmt_txt() 走到 #else 分支直接 return NULL
           #     → 字形位图全拿不到 → 框线、底色、布局都在，一个字都不出来。
           #  3) 就算把开关打开也没用：LVGL 9.5 把 fmt_txt 挪进 src/font/fmt_txt/
           #     并换了压缩算法，跟 1.5.3 的老格式不是一回事，强行解只会花屏。
           #
           #  关压缩 = PLAIN 格式 = 原始 4bpp 位图数组，LVGL 8/9 通吃。
           "--no-compress",
           "--lv-include", "lvgl.h",
           "--lv-font-name", name,
           "-o", out]

    if with_ascii:
        cmd += ["--range", ASCII_RANGE]
    if symbols:
        cmd += ["--symbols", "".join(sorted(symbols))]

    os.makedirs(FONTDIR, exist_ok=True)
    r = subprocess.run(cmd, capture_output=True, text=True)
    if r.returncode != 0:
        sys.exit("字库生成失败 %s\n%s\n%s" % (name, r.stdout, r.stderr))
    if r.stdout.strip():
        print("  " + r.stdout.strip().replace("\n", "\n  "))
    return out


def bitmap_bytes(c_path):
    """从生成的 .c 里累加真实位图字节数。

    ★ 别拿 .c 的文件大小估 flash 占用 —— 位图在源码里是按 "0x00, 0x01"
    文本展开的，源码体积约是真实位图的 5 倍。
    """
    src = read(c_path)
    total = 0
    for w, h in re.findall(r"\.box_w\s*=\s*(\d+)\s*,?\s*\.box_h\s*=\s*(\d+)", src):
        total += (int(w) * int(h)) // 2
    return total


def check_cmakelists(names):
    """核对每支字库都登记进了 main/CMakeLists.txt。

    ★★★ 2026-09-19（第十四轮）踩的坑，必须记住：
        main/CMakeLists.txt 的 SRCS 是**显式列举**，不是 GLOB
        （GLOB 也得 CONFIGURE_DEPENDS 才会感知新文件）。
        所以 gen_fa_fonts.py 新产出一支字体后，如果没手工往 SRCS 里补一行，
        .c 会**静静地不进构建图** —— 不报错、不警告，
        直到链接期才炸成 "undefined reference to `fa_font_xxx'"，
        而且这个报错看着像声明问题，很容易往 fa_fonts.h 那边查错方向。

    这里做成**硬失败**而不是警告：少一支字库必然导致真机缺字，
    没有任何「先跑起来再说」的余地。
    """
    cmake = os.path.join(MAIN, "CMakeLists.txt")
    src = read(cmake)
    missing = [n for n in names if ('"fonts/%s.c"' % n) not in src]
    if missing:
        print("\n[FAIL] 以下字库没有登记进 main/CMakeLists.txt：")
        for n in missing:
            print('       请补一行：        "fonts/%s.c"' % n)
        print("       （SRCS 是显式列举；漏登的 .c 不会进构建图，")
        print('         错误形态是链接期 undefined reference to `%s\'）' % missing[0])
        sys.exit(1)
    # ★★ 第十八轮新增：**非 fonts/ 的纯数据文件**也要登记（只有 fa_talk.c 一个）。
    #   它一旦漏登，表现和上面一模一样（链接期 undefined reference），
    #   但报错的符号会是 FA_TALK_BASE —— 那条线很容易往「表写错了」的方向查，
    #   其实是「表根本没参与链接」。所以一并在这里卡死。
    extra = [f for f in EXTRA_SRCS if ('"%s"' % f) not in src]
    if extra:
        print("\n[FAIL] 以下数据文件没有登记进 main/CMakeLists.txt：")
        for f in extra:
            print('       请补一行：        "%s"' % f)
        print("       （错误形态同样是链接期 undefined reference to `FA_TALK_BASE\'）")
        sys.exit(1)
    print("[cmk ] main/CMakeLists.txt 已登记 %d 支字库 + %d 个数据文件"
          % (len(names), len(EXTRA_SRCS)))


def main():
    title_chars = set(NONASCII.findall(TITLE_TEXT))
    base_chars, talk_chars = scan_sources()
    all_chars = base_chars | talk_chars | title_chars

    # ★ 第十八轮的两档（见文件开头）：
    #   ui   = 全部（气泡那 290 句要用 16px 显示）
    #   tiny = 基础组（那 290 句永远不会用 13px 显示）
    ui_chars   = all_chars
    tiny_chars = base_chars | title_chars

    fonts = [
        # title 也要 ASCII：明信片卡上的大号数字走这个字面
        ("fa_font_title", SERIF,     24, title_chars, True),
        # ★ 第十三轮：15 -> **16**，字重 Regular -> **Bold**。
        #   16 是实测上限，见文件开头的说明；再大就撑破底栏格子与开场正文框。
        ("fa_font_ui",    SANS_BOLD, 16, ui_chars,    True),
        # ★ 第十三轮：11 -> **13**，字重 Regular -> **Medium**（不是 Bold，
        #   13px 中文用 Bold 会糊；理由见文件开头）。
        ("fa_font_tiny",  SANS_MED,  13, tiny_chars,  True),
        # ★ 第十四轮新增；**第十六轮字面换过**（接罐头 -> 小游戏新获得？石剪布）。
        #   它是小游戏那一族界面唯一的大字：封面标题 / 选择页标题 /「新获得」/
        #   猜拳的三张牌 + 猫那一格的问号。24px 是「比 16 大一些」的落点。
        #   ~~旧说明：三个字 72px + 16px 的「小游戏」48px 与字距，整行在 176 内~~
        #     —— 那只对「两段并排的封面标题」成立，而那段版式已经没了。
        #   现在最长的一处是「新获得」三个字：3×24 + 3×6 = 90px，装进 104 的胶囊。
        #   不带 ASCII —— 这一族没有英文数字。
        ("fa_font_game",  SANS_BOLD, 24, set(GAME_TEXT), False),
        # ★★ 2026-09-21（第四十六轮）新增第 5 支字库 fa_font_small。
        #   来源：设置页那一页的**三块按键表**（设置/主页/旅游）要塞 14 行进一张卡片，
        #   13px 排下来偏挤 —— 用户原话「可以把字体稍微调小一些，现在太紧凑了」。
        #   ⚠⚠ **实测：12px 的 line_height 还是 15**（和 13px 一模一样，见
        #      `_measure_fonts.py` 与真机产出的 .line_height）。也就是说
        #      **缩字号并不省高度**，「不紧凑」是靠**行距 +1**和**列距 4->8**腾出来的，
        #      字号只是顺带小一号。别以为改成 12px 就能多塞两行。
        #   ★ 为什么单开一支而不是把 fa_font_tiny 从 13 降到 12：
        #      tiny 是**全局共用**的（册子顶栏 / 小游戏 / 到达卡 / 开场提示全在用），
        #      改它等于把其它七八处的字号一起动了。设置页是唯一需要 12px 的地方。
        #   ★ 字面直接用 tiny_chars（基础组 + 标题），**不另开一张固定清单**：
        #      设置页那些字（含 `【】`）本来就写在这个工程的源码字符串里，
        #      扫源码自动收 = 「写进去就能显示」这条构造性保证对设置页同样成立。
        #      代价是这支字库和 tiny 差不多大（约 30 KB），flash 里换零漏字风险，值。
        #   ★ 带 ASCII：设置页现在没有数字，但以后加一行「亮度 60%」之类就不用再动字库。
        ("fa_font_small", SANS_MED,  12, tiny_chars,  True),
    ]

    hdr = [
        "// 自动生成，勿手改 —— 由 tools/gen_fa_fonts.py 产出。",
        "// 改完界面文案一定要重跑:漏掉的字在屏幕上就是方框。",
        "#pragma once",
        "",
        '#include "lvgl.h"',
        "",
    ]

    src_total = 0
    bmp_total = 0
    for name, font, size, chars, ascii_ok in fonts:
        p = run_font(name, font, size, chars, ascii_ok)
        ss = os.path.getsize(p)
        bs = bitmap_bytes(p)
        src_total += ss
        bmp_total += bs
        hdr.append("LV_FONT_DECLARE(%s);" % name)
        print("[font] %-14s %2dpx  源码 %6.1f KB  真实位图 %6.1f KB"
              % (name, size, ss / 1024.0, bs / 1024.0))

    hdr.append("")
    with open(os.path.join(MAIN, "fa_fonts.h"), "w", encoding="utf-8", newline="\n") as f:
        f.write("\n".join(hdr))

    print("[sum ] 源码合计 %.1f KB / 真实位图合计 %.1f KB" % (src_total / 1024.0, bmp_total / 1024.0))
    print("[out ] main/fa_fonts.h + main/fonts/*.c")
    # 最后一道闸：字库必须登记进 CMakeLists，否则根本不参与链接（见函数注释）
    check_cmakelists([n for n, *_ in fonts])


if __name__ == "__main__":
    main()
