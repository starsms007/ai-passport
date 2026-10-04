#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""8MB 整片的分段差异核对（发布级）。

判据（同 IDF 同配置的两版之间）：
  · 分区表 0x8000..0x9000  : 应逐字节相同
  · bootloader 0x0..0x8000 : 功能代码应相同；**允许** `esp_bootloader_desc_t`
                             描述块（版本串 / 编译日期 / 编译时间）不同
  · app     0x10000..      : 通常不同（这就是本轮改的东西）
  · 其余为 0xFF 填充

★ 输出**只用 ASCII**：本机控制台是 GBK，emoji/部分符号会让 print 抛 UnicodeEncodeError
  （本脚本第一版就因此在三行输出后崩掉，判据只印出一半）。
用法：python _diff_images.py <A.bin> <B.bin>
"""
import hashlib
import sys

A, B = sys.argv[1], sys.argv[2]
a = open(A, "rb").read()
b = open(B, "rb").read()
print("A =", A)
print("B =", B)
print("SIZE %d vs %d  %s" % (len(a), len(b), "SAME" if len(a) == len(b) else "**DIFFER**"))
print("SHA  A = %s" % hashlib.sha256(a).hexdigest()[:32])
print("     B = %s" % hashlib.sha256(b).hexdigest()[:32])


def first_diff(sa, sb, base):
    n = min(len(sa), len(sb))
    for i in range(n):
        if sa[i] != sb[i]:
            return base + i
    return None if len(sa) == len(sb) else base + n


# bootloader 的描述块：实测在 0x28..0x60（esp_bootloader_desc_t）
DESC_LO, DESC_HI = 0x24, 0x64

print("\n--- regions ---")
regions = [
    ("bootloader hdr+desc 0x0..0x%X" % DESC_LO, 0x0, DESC_LO),
    ("bootloader DESC   0x%X..0x%X" % (DESC_LO, DESC_HI), DESC_LO, DESC_HI),
    ("bootloader code   0x%X..0x8000" % DESC_HI, DESC_HI, 0x8000),
    ("partition table   0x8000..0x9000", 0x8000, 0x9000),
    ("app               0x10000..", 0x10000, min(len(a), len(b))),
]
for name, s, e in regions:
    sa, sb = a[s:e], b[s:e]
    same = sa == sb
    fd = first_diff(sa, sb, s)
    print("  %-34s A=%9d B=%9d  %s" % (
        name, len(sa), len(sb),
        "IDENTICAL" if same else ("DIFF (first @0x%05X)" % fd)))

print("\n--- verdict ---")
ok = (a[0:DESC_LO] == b[0:DESC_LO]
      and a[DESC_HI:0x9000] == b[DESC_HI:0x9000]
      and a[0x10000:] != b[0x10000:])
print("  header(0x0..0x%X) same + bootloader code & partition same + app DIFF => %s"
      % (DESC_LO, "PASS (only app + bootloader desc block)" if ok else "CHECK MANUALLY"))

# 描述块里的可读串（帮助判断差异性质）
print("\n--- bootloader desc strings ---")
for tag, buf in (("A", a), ("B", b)):
    seg = buf[DESC_LO:DESC_HI]
    txt = "".join(chr(c) if 32 <= c < 127 else "." for c in seg)
    print("  %s: %s" % (tag, txt))

sys.exit(0 if ok else 2)
