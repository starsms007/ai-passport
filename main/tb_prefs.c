// main/tb_prefs.c
// 少量持久化偏好。目前只有一项：屏幕方向。
//
// ★ 改了默认值：雷霆战机是横版（90），《去远方》是竖版（0/180）。
//   180 与 0 的区别只是上下颠倒 —— 屏幕装反了的批次靠它救回来，
//   不用重新刷固件（长按 ↓ 切）。
#include "tb_prefs.h"

#include "esp_log.h"
#include "nvs.h"

#include "bsp_display.h"

static const char *TAG = "prefs";

#define NVS_NS  "tbp"
#define NVS_KEY "rot"
#define DEG_DEF 0

// 亮度。★ 下限给 10 不是 0:背光关到 0 等于把人锁在门外 ——
//   屏全黑、又看不见当前调到了几,只能靠再刷一次机救回来。
//   10% 在室内还是能看清字的,足够当「最暗」。
#define BR_KEY  "bl"
#define BR_MIN  10
#define BR_MAX  100
#define BR_DEF  100

static int s_deg  = DEG_DEF;
static int s_br   = BR_DEF;

int tb_screen_deg(void) { return s_deg; }
int tb_bright(void)     { return s_br;  }

int tb_bright_set(int percent) {
    if (percent < BR_MIN) percent = BR_MIN;
    if (percent > BR_MAX) percent = BR_MAX;

    s_br = percent;
    bsp_display_backlight((uint8_t)percent);

    nvs_handle_t h;
    if (nvs_open(NVS_NS, NVS_READWRITE, &h) == ESP_OK) {
        nvs_set_i32(h, BR_KEY, (int32_t)percent);
        nvs_commit(h);
        nvs_close(h);
    } else {
        ESP_LOGW(TAG, "NVS open failed; brightness applies to this boot only");
    }
    return percent;
}

int tb_screen_deg_set(int deg) {
    // 只认竖屏的两个朝向。90/270 会让 240x320 的布局整体错位 ——
    // 本固件是按竖版排的，横屏没有对应布局，直接挡在门外。
    if (deg != 0 && deg != 180) deg = DEG_DEF;

    s_deg = deg;
    bsp_lvgl_set_rotation(deg);

    nvs_handle_t h;
    if (nvs_open(NVS_NS, NVS_READWRITE, &h) == ESP_OK) {
        nvs_set_i32(h, NVS_KEY, (int32_t)deg);
        nvs_commit(h);
        nvs_close(h);
    } else {
        ESP_LOGW(TAG, "NVS open failed; rotation applies to this boot only");
    }
    return deg;
}

void tb_prefs_apply(void) {
    int deg = DEG_DEF;
    int br  = BR_DEF;

    nvs_handle_t h;
    if (nvs_open(NVS_NS, NVS_READONLY, &h) == ESP_OK) {
        int32_t v = DEG_DEF;
        if (nvs_get_i32(h, NVS_KEY, &v) == ESP_OK) deg = (int)v;

        // 亮度顺手一起读 —— 少开一次 NVS。
        int32_t b = BR_DEF;
        if (nvs_get_i32(h, BR_KEY, &b) == ESP_OK) br = (int)b;

        nvs_close(h);
    }
    // ★ 这里要挡住老固件留下的 90/270 —— NVS 是跨固件共享的，
    //   雷霆战机存过 90，直接用会让竖版布局整体错位。
    if (deg != 0 && deg != 180) deg = DEG_DEF;

    s_deg = deg;
    bsp_lvgl_set_rotation(deg);

    // ★ 亮度要在开机时补一次:main.c 里那句 bsp_display_backlight(100) 跑在
    //   本函数之前,不覆盖的话用户调过的亮度每次开机都会被打回 100。
    if (br < BR_MIN) br = BR_MIN;
    if (br > BR_MAX) br = BR_MAX;
    s_br = br;
    bsp_display_backlight((uint8_t)br);

    int w = 0, hgt = 0;
    bsp_lvgl_get_res(&w, &hgt);
    ESP_LOGI(TAG, "rotation %d deg, brightness %d%%, logical resolution %dx%d",
             deg, br, w, hgt);
}
