# -*- coding: utf-8 -*-
"""校验 fa_sprites.bin 是否被原样嵌进 faraway.bin。

EMBED_FILES 走汇编 .incbin，链接器把整个 blob 原样放进 .rodata。
这里在 app 镜像里定位这个 blob 并逐字节比对。

★ 探针的选法很关键：素材段开头是透明填充（成片相同字节），
  拿文件头当探针会命中上千处假阳性。改用「数据密集区 + 512 字节」，
  512 字节随机内容出现第二次的概率可以忽略。
"""
import hashlib
import io
import sys

APP = r'D:\Espressif\faraway-firmware\build\faraway.bin'
SPR = r'D:\Espressif\faraway-firmware\main\fa_sprites.bin'

app = io.open(APP, 'rb').read()
spr = io.open(SPR, 'rb').read()

print('app = %d B' % len(app))
print('  sha256 %s' % hashlib.sha256(app).hexdigest())
print('spr = %d B' % len(spr))
print('  sha256 %s' % hashlib.sha256(spr).hexdigest())
print()

# ---- 挑一个「字节种类多」的位置做探针 ----
PROBE_LEN = 512
best_off, best_div = 0, -1
for off in range(0, len(spr) - PROBE_LEN, 4096):
    div = len(set(spr[off:off + PROBE_LEN]))
    if div > best_div:
        best_off, best_div = off, div

probe = spr[best_off:best_off + PROBE_LEN]
print('探针: blob 偏移 0x%X，%d B，含 %d 种不同字节' % (best_off, PROBE_LEN, best_div))

hits, off = [], app.find(probe)
while off >= 0:
    hits.append(off)
    off = app.find(probe, off + 1)

print('app 内命中 %d 处' % len(hits))
if not hits:
    print('\n端到端校验: FAIL  (探针未找到)')
    sys.exit(1)
if len(hits) > 1:
    print('  候选: %s' % ['0x%X' % h for h in hits[:8]])

ok = False
for h in hits:
    blob_off = h - best_off                    # 由探针位置反推 blob 起点
    if blob_off < 0:
        continue
    seg = app[blob_off:blob_off + len(spr)]
    if len(seg) == len(spr) and seg == spr:
        print('\nblob 起点 = 0x%X (app 文件偏移)' % blob_off)
        print('  %d B 逐字节一致' % len(spr))
        ok = True

print()
if ok:
    print('端到端校验: PASS')
    sys.exit(0)
print('端到端校验: FAIL  (命中位置对不上，blob 未被原样嵌入)')
sys.exit(1)
