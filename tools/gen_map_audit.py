# -*- coding: utf-8 -*-
"""从源码 + 素材清单生成《明信片 ↔ 景点名 映射审计》HTML。

为什么要生成而不是手写：
    DEST_NAME / DEST_ART[].card / DEST_SLOT 三张表分处两个 .c 文件，
    改一处忘一处不会报错，只会在设备上表现成「选 5 分钟给你一张布达拉宫」。
    这张审计表的价值就在于它**不是人写的** —— 它直接读那三张表和
    art/manifest.json 的 subject，任何人改完代码重跑一遍就能看出有没有错位。

三处取数：
    1. main/fa_game.c    → DEST_NAME / DEST_VERSE / DEST_SLOT
    2. main/fa_view.c    → DEST_ART[].card（素材 id；第九轮起由 DEST_CARD[] 改名而来）
    3. art/manifest.json → subject（美术侧对这张图的文字描述）

交叉验证：
    subject 的第一句是画面描述（"西湖断桥，湖面雾气未散…"）。
    拿它去和景点名对，命中算过、不命中标红 —— 这是「明信片与地名一一对应」
    这条需求的机器可判定形式。
"""
import html
import io
import json
import os
import re
import sys

# ---- 路径：默认按脚本位置推导，可用命令行参数覆盖 ----
#
# 脚本落在 02-固件源码/tools/ 时，向上两级就是包根：
#     02-固件源码/tools/gen_map_audit.py
#     ├── main/fa_game.c            ← 读
#     ├── main/fa_view.c            ← 读
#     └── ../03-素材流水线/art/manifest.json   ← 读
#          ../05-交付物/明信片映射审计.html     ← 写
#
# 这么写是为了「搬哪都能跑」—— 工程工作副本（D:\Espressif\faraway-firmware）
# 和交接包是两个不同的目录树，靠绝对路径写死迟早失效。
HERE = os.path.dirname(os.path.abspath(__file__))


def _opt(name):
    """取 `--name value` 形式的参数，没有就返回 None。"""
    argv = sys.argv[1:]
    for i, a in enumerate(argv):
        if a == name and i + 1 < len(argv):
            return argv[i + 1]
    return None


# 先收参数，再统一推导 —— 不能边解析边算，否则给了 --src 但 PKG 还是老值，
# MANIFEST 会指到 D:\Espressif\03-素材流水线 这种不存在的地方。
# （2026-09-19 修的：原来 --src 只改 SRC 不改 PKG，工作副本下一跑就 FileNotFoundError。）
SRC = _opt('--src') or os.path.dirname(HERE)      # 02-固件源码 / faraway-firmware
PKG = _opt('--pkg') or os.path.dirname(SRC)       # 包根（工作副本时不存在也无所谓）

FA_GAME = os.path.join(SRC, 'main', 'fa_game.c')
FA_VIEW = os.path.join(SRC, 'main', 'fa_view.c')
MANIFEST = _opt('--manifest') or os.path.join(PKG, '03-素材流水线', 'art', 'manifest.json')
OUT = _opt('--out') or os.path.join(PKG, '05-交付物', '明信片映射审计.html')


SLOT_LABEL = ['试玩 15 秒', '5 分钟', '1 小时', '4 小时', '12 小时']
# ★ 2026-09-19 第十四轮修：原来写 [1,1,3,5,8]，与 fa_game.c:114 的
#   `SLOT_CLOVER = {0,1,2,3,5}` 不符（试玩档必须是 0 —— 它是折价补偿的闸门）。
#   这两个数只用于页面展示，不会静默改任何行为，但展示错了照样误导人。
#   ⚠ 量词也一并改：第九轮起「三叶草」对外叫**猫罐头**，单位是「个」。
SLOT_CLOVER = [0, 1, 2, 3, 5]

# 地名 → 在 subject 画面描述里应当出现的关键词（正则片段）。
# 这张表是「断言」：命中说明美术画的确实是这个地儿。
# 找不到就标红，逼人去核对到底是名字错了还是图错了。
EXPECT = {
    '婺源':       r'油菜花',
    '宏村':       r'青石拱桥|村口',
    '蜀南竹海':   r'竹林|毛竹',
    '扬州':       r'垂柳',
    '西塘':       r'古镇|窄巷',
    '西湖':       r'西湖',
    '元阳':       r'元阳',
    '霞浦':       r'霞浦',
    '黄山':       r'黄山',
    '桂林':       r'漓江',
    '泰山':       r'泰山',
    '长城':       r'长城',
    '张掖':       r'张掖|丹霞',
    '额济纳':     r'额济纳',
    '泸沽湖':     r'泸沽湖',
    '喀纳斯':     r'喀纳斯',
    '稻城':       r'稻城',
    '布达拉宫':   r'布达拉宫',
    '敦煌':       r'敦煌',
    '漠河':       r'漠河',
    '可可西里':   r'可可西里',
    '呼伦贝尔':   r'呼伦贝尔',
    '雨崩':       r'雨崩',
    '塔克拉玛干': r'塔克拉玛干',
}


def read(p):
    return io.open(p, 'r', encoding='utf-8').read()


def block(src, name):
    i = src.find(name + '[')
    if i < 0:
        i = src.find(name + ' [')
    j = src.find('{', i)
    k = src.find('};', j)
    return src[j:k]


def dest_art_cards(src):
    """从 `DEST_ART[]` 取 24 个**明信片**素材 id。

    ★ 2026-09-19 第十四轮修：这张表原来叫 `DEST_CARD[]`（一张指针数组），
      第九轮改成了 `static const dest_art_t DEST_ART[]` —— **一行一个景点、
      card / nocat 两个字段并排**。脚本没跟着改，于是 `block(v,'DEST_CARD')`
      找不到名字、`src.find` 返回 -1，切出来的是**整个文件** ⇒ 断言
      「四表长度不一致」直接抛错（不静默，算是运气好）。

    ⚠ 不能靠 `src.find('DEST_ART[')` 定位 —— 源码里 `DEST_ART[]` 在**注释中**
      先出现过两次（第 420 / 2406 行），会锚到注释上。必须锚**声明那一行**。
    ⚠ 只能用「行首 `{` + 第一个指针 + 逗号」这个形状取，取到的是 `card` 字段；
      第 2 个（`nocat`）是底图，明信片审计不要它。
    """
    i = src.find('static const dest_art_t DEST_ART[')
    assert i >= 0, 'fa_view.c 里找不到 DEST_ART[] 声明 —— 表又改名了？'
    j = src.find('{', i)
    k = src.find('};', j)
    assert j > 0 and k > j, 'DEST_ART[] 的花括号没配对'
    return re.findall(r'\{\s*&fa_res_([a-z0-9_]+)\s*,', src[j:k])


def main():
    g = read(FA_GAME)
    v = read(FA_VIEW)
    mf = json.load(io.open(MANIFEST, encoding='utf-8'))

    names = re.findall(r'"([^"]+)"', block(g, 'DEST_NAME'))
    verses = re.findall(r'"([^"]+)"', block(g, 'DEST_VERSE'))
    slots = [int(x) for x in re.findall(r'\b(\d+)\b', block(g, 'DEST_SLOT'))]
    cards = dest_art_cards(v)
    souv = re.findall(r'"([^"]+)"', block(g, 'SOUV_NAME'))

    n = len(names)
    assert len(verses) == n and len(slots) == n and len(cards) == n, '四表长度不一致'

    # 素材 id → subject 首句
    subj = {}
    for j in mf['jobs']:
        if j.get('agent') == 'postcard':
            subj[j['id']] = (j.get('subject') or '').split('画面里的小猫')[0].strip(' 。')

    rows = []
    bad = []
    for i in range(n):
        nm, cd = names[i], cards[i]
        scene = subj.get(cd, '（清单里找不到这张素材）')
        pat = EXPECT.get(nm)
        hit = bool(pat and re.search(pat, scene))
        if not hit:
            bad.append(nm)
        rows.append(dict(i=i, name=nm, verse=verses[i], slot=slots[i],
                         card=cd, scene=scene, hit=hit,
                         pat=pat or '(未定义)'))

    # ---- 每档统计 ----
    from collections import Counter
    cnt = Counter(slots)

    def esc(s):
        return html.escape(s)

    P = []
    a = P.append
    a('<!DOCTYPE html>')
    a('<html lang="zh-CN">')
    a('<head>')
    a('<meta charset="utf-8">')
    a('<title>明信片 ↔ 景点名 映射审计</title>')
    a('<style>')
    a(':root{--ink:#2C2C2A;--ink2:#57534E;--ink3:#8A8580;--line:#E7E5E1;'
      '--bg:#FFFFFF;--card:#FAFAF8;--ok-bg:#F0FDFA;--ok-line:#99F6E4;--ok-ink:#0F766E;'
      '--bad-bg:#FFF1F2;--bad-line:#FECDD3;--bad-ink:#9F1239;}')
    a('*{box-sizing:border-box}')
    a('body{margin:0;background:var(--bg);color:var(--ink);'
      'font:400 15px/1.7 "PingFang SC","Microsoft YaHei",system-ui,sans-serif;'
      '-webkit-font-smoothing:antialiased}')
    a('.wrap{max-width:1000px;margin:0 auto;padding:44px 28px 72px}')
    a('h1{font-size:26px;font-weight:500;letter-spacing:.3px;margin:0 0 6px}')
    a('.sub{color:var(--ink3);font-size:13px;margin:0 0 30px}')
    a('h2{font-size:16px;font-weight:500;margin:38px 0 14px;padding-left:11px;'
      'border-left:3px solid var(--ink);line-height:1.2}')
    a('h2 .n{color:var(--ink3);font-size:13px;font-weight:400;margin-right:8px}')
    a('p{margin:0 0 12px}')
    a('.note{background:var(--card);border:0.5px solid var(--line);border-radius:10px;'
      'padding:16px 18px;margin:0 0 8px;font-size:14px;color:var(--ink2)}')
    a('.pill{display:flex;gap:10px;flex-wrap:wrap;margin:18px 0 6px}')
    a('.pill div{flex:1 1 0;min-width:140px;border-radius:9px;padding:13px 15px;'
      'border:0.5px solid var(--line);background:var(--card)}')
    a('.pill .k{font-size:12px;color:var(--ink3);margin-bottom:3px}')
    a('.pill .v{font-size:23px;font-weight:500;line-height:1.1}')
    a('.pill .u{font-size:12px;color:var(--ink3);margin-left:3px;font-weight:400}')
    a('.pill .ok{color:var(--ok-ink)} .pill .bd{color:var(--bad-ink)}')
    a('table{width:100%;border-collapse:collapse;font-size:13px;margin:4px 0 6px}')
    a('th{text-align:left;font-weight:500;font-size:12px;color:var(--ink3);'
      'padding:8px 9px;border-bottom:0.5px solid var(--line);white-space:nowrap}')
    a('td{padding:7px 9px;border-bottom:0.5px solid var(--line);vertical-align:top}')
    a('tr:last-child td{border-bottom:none}')
    a('.idx{color:var(--ink3);font-variant-numeric:tabular-nums;width:30px}')
    a('.name{font-weight:500;white-space:nowrap}')
    a('.sym{font-family:ui-monospace,Consolas,monospace;font-size:11.5px;'
      'color:var(--ink2);white-space:nowrap}')
    a('.scene{color:var(--ink2);font-size:12.5px}')
    a('.tag{display:inline-block;font-size:11px;padding:1px 7px;border-radius:20px;'
      'border:0.5px solid;white-space:nowrap;line-height:1.6}')
    a('.tag.ok{background:var(--ok-bg);border-color:var(--ok-line);color:var(--ok-ink)}')
    a('.tag.bd{background:var(--bad-bg);border-color:var(--bad-line);color:var(--bad-ink)}')
    a('.slot{font-size:12px;color:var(--ink3);background:var(--card);'
      'border:0.5px solid var(--line);border-radius:20px;padding:2px 10px;'
      'display:inline-block;margin:0 0 6px}')
    a('.foot{color:var(--ink3);font-size:12.5px;margin-top:36px;'
      'border-top:0.5px solid var(--line);padding-top:14px}')
    a('</style>')
    a('</head>')
    a('<body><div class="wrap">')

    a('<h1>明信片 ↔ 景点名 映射审计</h1>')
    a('<p class="sub">本页由 <code>tools/gen_map_audit.py</code> 从 '
      '<code>main/fa_game.c</code> 的 DEST_NAME/DEST_VERSE/DEST_SLOT、'
      '<code>main/fa_view.c</code> 的 DEST_ART[].card、以及 '
      '<code>03-素材流水线/art/manifest.json</code> 的 subject 三处自动汇总。'
      '—— 2026-09-19</p>')

    a('<div class="note">')
    a('<b>怎么读这张表。</b>「画面描述」一栏是美术清单里对这张图的原文（取 '
      '<code>subject</code> 的首句，即画面场景那段）。「对应」一栏是拿这句去对'
      '景点名的机器判定 —— 命中说明美术画的确实是这个地儿。<br>')
    a('三张表分处两个 .c 文件，改一处忘一处<b>不会报错</b>，只会在设备上表现成'
      '「选 5 分钟却给了一张布达拉宫」。所以这张表和代码是同一个真相来源：'
      '改完代码重跑生成脚本，错位会立刻显形。')
    a('</div>')

    a('<div class="pill">')
    a('<div><div class="k">景点总数</div><div class="v">%d</div></div>' % n)
    a('<div><div class="k">档位</div><div class="v">%d</div></div>' % len(cnt))
    a('<div><div class="k">画面描述命中</div><div class="v ok">%d</div></div>'
      % (n - len(bad)))
    a('<div><div class="k">未命中</div><div class="v%s">%d</div></div>'
      % (' bd' if bad else '', len(bad)))
    a('</div>')

    if bad:
        a('<div class="note" style="border-color:var(--bad-line);'
          'background:var(--bad-bg);color:var(--bad-ink)">'
          '<b>未命中的景点：</b>%s —— 需要人工核对是名字错了还是素材画错了。</div>'
          % esc('、'.join(bad)))

    # ---- 按档分组 ----
    for s in range(len(SLOT_LABEL)):
        grp = [r for r in rows if r['slot'] == s]
        if not grp:
            continue
        a('<h2><span class="n">第 %d 档</span>%s'
          % (s, esc(SLOT_LABEL[s])))
        a('</h2>')
        a('<div class="slot">%d 个景点 · 抵达可得 %d 个猫罐头'
          % (len(grp), SLOT_CLOVER[s]))
        if s == 0:
            a(' · 试玩档')
        a('</div>')
        a('<table>')
        a('<tr><th>#</th><th>景点名</th><th>题词</th><th>素材 id</th>'
          '<th>画面描述（美术清单原文）</th><th>对应</th></tr>')
        for r in grp:
            a('<tr>')
            a('<td class="idx">%d</td>' % r['i'])
            a('<td class="name">%s</td>' % esc(r['name']))
            a('<td class="scene">%s</td>' % esc(r['verse']))
            a('<td class="sym">%s</td>' % esc(r['card']))
            a('<td class="scene">%s</td>' % esc(r['scene']))
            a('<td><span class="tag %s">%s</span></td>'
              % ('ok' if r['hit'] else 'bd', '命中' if r['hit'] else '未命中'))
            a('</tr>')
        a('</table>')

    # ---- 纪念品 ----
    a('<h2><span class="n">附录</span>纪念品（%d 件）</h2>' % len(souv))
    a('<div class="note">纪念品<b>不与地点绑定</b>：它们按趟次随机掉落，'
      '任何一档都有可能拿到，所以不存在「哪件对应哪个地方」的对应关系需要核对。'
      '这里只作清单存档。</div>')
    a('<table><tr><th>#</th><th>名称</th></tr>')
    for i, sname in enumerate(souv):
        a('<tr><td class="idx">%d</td><td class="name">%s</td></tr>'
          % (i, esc(sname)))
    a('</table>')

    a('<div class="foot">')
    a('生成源：<code>main/fa_game.c</code> / <code>main/fa_view.c</code> / '
      '<code>art/manifest.json</code>　·　'
      '改完上述任一文件请重跑生成脚本，不要手改本页。')
    a('</div>')

    a('</div></body></html>')

    io.open(OUT, 'w', encoding='utf-8', newline='\n').write('\n'.join(P))

    print('读: %s' % FA_GAME)
    print('读: %s' % FA_VIEW)
    print('读: %s' % MANIFEST)
    print('写出: %s' % OUT)
    print('景点 %d 个 / 档位 %d 个 / 命中 %d / 未命中 %d'
          % (n, len(cnt), n - len(bad), len(bad)))
    for s in range(len(SLOT_LABEL)):
        print('  第 %d 档 %-10s %d 个' % (s, SLOT_LABEL[s], cnt.get(s, 0)))
    if bad:
        print('!! 未命中: %s' % '、'.join(bad))
        return 1
    return 0


if __name__ == '__main__':
    sys.exit(main())
