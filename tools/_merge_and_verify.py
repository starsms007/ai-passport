#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""r57：8MB 整片合并 + 三段逐字节校验（替代已不存在的 _verify_wrap.py）。

r56/r53 的镜像头 byte[3] 都是 0x3F ⇒ flash_size 枚举 = 3（8MB）、spi_speed = 15（80MHz）。
所以合并时显式给 --flash_size 8MB（不能用 @flash_args —— 它带的是 detect，merge_bin 不认）。
"""
import hashlib
import os
import subprocess
import sys

ROOT = r"D:\Games\faraway-fw"
B = os.path.join(ROOT, "build")
full = os.path.join(B, "faraway-8mb-full.bin")
bl = os.path.join(B, "bootloader", "bootloader.bin")
pt = os.path.join(B, "partition_table", "partition-table.bin")
app = os.path.join(B, "faraway.bin")

cmd = [sys.executable, "-m", "esptool", "--chip", "esp32c3", "merge_bin",
       "-o", full, "--fill-flash-size", "8MB", "--flash_size", "8MB",
       "0x0", bl, "0x8000", pt, "0x10000", app]
r = subprocess.run(cmd, cwd=B, capture_output=True, text=True, encoding="utf-8", errors="replace")
print("MERGE_RC =", r.returncode)
print((r.stdout or "")[-600:])
if r.returncode != 0:
    print((r.stderr or "")[-1200:])
    sys.exit(1)

img = open(full, "rb").read()
print("SIZE       =", len(img))
print("SHA256     =", hashlib.sha256(img).hexdigest())
print("byte3      = 0x%02X  (r56/r53 都是 0x3F)" % img[3])

ok = True
for off, p in ((0x0, bl), (0x8000, pt), (0x10000, app)):
    d = open(p, "rb").read()
    same = img[off:off + len(d)] == d
    ok = ok and same
    print("seg @0x%05X  %-22s %9d B  %s" % (off, os.path.basename(p), len(d), "OK" if same else "**MISMATCH**"))

tail = img[0x10000 + os.path.getsize(app):]
tail_ff = all(x == 0xFF for x in tail)
print("tail fill  = %s (%d B)" % ("all 0xFF" if tail_ff else "**NOT 0xFF**", len(tail)))

good = ok and len(img) == 8388608 and img[3] == 0x3F and tail_ff
print("VERDICT    =", "ALL PASS" if good else "FAIL")
sys.exit(0 if good else 2)
