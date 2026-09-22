idf_component_register(
    SRCS "main.c"
         "fa_game.c"          # 玩法逻辑（纯数据，不依赖 LVGL）
         "fa_view.c"          # 全部 LVGL 对象
         "fa_sprites.c"       # 由 tools/gen_fa_sprites.py 生成
         "tb_clock.c"         # SNTP 校时 + NVS 时间缓存
         "tb_prefs.c"         # 屏幕方向/亮度等持久化偏好
         "fa_talk.c"          # ★ 第十八轮：猫咪对我说的话（290 句，_dl/_talk18.py 产出）
         # fa_music.c 已于 2026-09-19 下线（背景音乐去掉，audio 分区一并取消）
         # ★ 字库走显式列举（不是 GLOB）。gen_fa_fonts.py 新增一支字体后，
         #   必须在这里补一行，否则 .c 会静静地不进构建图，
         #   报错形态是链接期 "undefined reference to `fa_font_xxx'"。
         "fonts/fa_font_title.c"   # 以下五支由 tools/gen_fa_fonts.py 生成
         "fonts/fa_font_ui.c"
         "fonts/fa_font_tiny.c"
         "fonts/fa_font_game.c"    # 思源黑 Bold 24px，字面见 gen_fa_fonts.py 的 GAME_TEXT
         "fonts/fa_font_small.c"   # ★ 第四十六轮：思源黑 Medium 12px，设置页那三块按键表专用
    INCLUDE_DIRS "." "fonts"
    REQUIRES bsp esp_wifi esp_netif nvs_flash lwip esp_timer
    # ------------------------------------------------------------------
    # ★ 精灵数据走 EMBED_FILES，不生成 .c 字节数组。
    #   展开成 "0x00, 0x00, ..." 源码约 8MB，GCC 编这一个 TU 要吃掉大量内存
    #   和时间（在 Windows 上尤其明显）。EMBED_FILES 走汇编 .incbin，
    #   链接期直接拼进去，编译时间可以忽略。
    #   对应的符号是 _binary_fa_sprites_bin_start（见 fa_sprites.c）。
    #   ★ 2026-09-19：4 位访客（蜗牛/蜜蜂/蝴蝶/乌龟，8 条动画）已从图集剔除。
    #     剔除方式后来从命令行 --skip 改成**写死在 gen_fa_sprites.py 的 OFFLINE
    #     名单里**（命令行参数靠人记得带，忘了就是固件悄悄长一兆）。当前常驻
    #     下线 39 条；想让某条回来，把它从 OFFLINE 挪走再重跑脚本。
    #   ⚠ 这里的体积随素材变，别写死数字 —— 现测 **6,649,968 B (6.34 MB)**，
    #     大头是 240x320 全幅底图（nocat_*，每张 153,600 B）。
    #     权威数字看 tools/_gen_fa_sprites_report.txt（每次打包自动写）。
    #     这个体积直接决定 app 分区够不够，见工程根的 partitions.csv。
    # ------------------------------------------------------------------
    EMBED_FILES "fa_sprites.bin"
)
