// main/tb_prefs.c
// 少量持久化偏好：屏幕方向 + 亮度。
//
// ★ 改了默认值：雷霆战机是横版（90），《去远方》是竖版（0/180）。
//   180 与 0 的区别只是上下颠倒 —— 屏幕装反了的批次靠它救回来，
//   不用重新刷固件（长按 ↓ 切）。
#include "tb_prefs.h"

#include "esp_log.h"
#include "nvs.h"
#include "lvgl.h"

#include "bsp_display.h"
#include "bsp_pins.h"

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

// ---------------------------------------------------------------------------
// 显示方向：两个就地实现的小助手
//
// 本工程原先的 components/bsp 自带 bsp_lvgl_set_rotation() / bsp_lvgl_get_res()，
// 上游 BSP 没有这两个函数。这里改用 LVGL 的公开 API 就地实现，**不动 components/bsp**
// —— 保持那份与上游逐字节一致，日后同步上游才不会每次都撞在同一堆文件上。
//
// 语义与旧实现一致：拿不到 display 时静默跳过（旋转）、退回编译期的 BSP_LCD_W/H（打印）。
// ---------------------------------------------------------------------------
static void fa_lvgl_set_rotation(int deg) {
    lv_display_t *disp = lv_display_get_default();
    if (!disp) return;                     // LVGL 还没起来，保持当前方向

    lv_display_rotation_t r;
    switch (deg) {
    case 90:  r = LV_DISPLAY_ROTATION_90;  break;
    case 180: r = LV_DISPLAY_ROTATION_180; break;
    case 270: r = LV_DISPLAY_ROTATION_270; break;
    default:  r = LV_DISPLAY_ROTATION_0;   break;
    }

    // lv_display_set_rotation 会发 LV_EVENT_SIZE_CHANGED，esp_lvgl_port 的
    // size_update 回调据此把 swap_xy / mirror 重新下发给面板。
    if (bsp_lvgl_lock(1000)) {
        lv_display_set_rotation(disp, r);
        bsp_lvgl_unlock();
    }
}

static void fa_lvgl_get_res(int *w, int *h) {
    lv_display_t *disp = lv_display_get_default();
    if (!disp) {
        if (w) *w = BSP_LCD_W;
        if (h) *h = BSP_LCD_H;
        return;
    }
    if (w) *w = (int)lv_display_get_horizontal_resolution(disp);
    if (h) *h = (int)lv_display_get_vertical_resolution(disp);
}

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
    fa_lvgl_set_rotation(deg);

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
    fa_lvgl_set_rotation(deg);

    // ★ 亮度要在开机时补一次:main.c 里那句 bsp_display_backlight(100) 跑在
    //   本函数之前,不覆盖的话用户调过的亮度每次开机都会被打回 100。
    if (br < BR_MIN) br = BR_MIN;
    if (br > BR_MAX) br = BR_MAX;
    s_br = br;
    bsp_display_backlight((uint8_t)br);

    int w = 0, hgt = 0;
    fa_lvgl_get_res(&w, &hgt);
    ESP_LOGI(TAG, "rotation %d deg, brightness %d%%, logical resolution %dx%d",
             deg, br, w, hgt);
}
