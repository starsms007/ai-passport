// 自动生成，勿手改 —— 由 tools/gen_fa_sprites.py 产出（manifest 驱动）。
#pragma once

#include "lvgl.h"

// 多帧动画条数（= manifest 里 kind==sheet 的 job 数）。单张资源不在这个表里，
// 它们各自导出 fa_res_<id>，直接取地址用。
#define FA_ANIM_COUNT 21

// 一条动画。帧图像全在 flash 里（EMBED 的 .bin，走 .rodata），不占 RAM。
typedef struct {
    const char *id;                 // 与 manifest 的 job id 同名
    uint8_t w, h;                   // 单帧像素尺寸
    uint8_t frames;                 // 帧数
    const lv_image_dsc_t *dsc;      // frames 个描述符，连续排布
} fa_anim_t;

extern const fa_anim_t fa_anims[FA_ANIM_COUNT];

// 按 id 找动画；找不到返回 NULL。
const fa_anim_t *fa_anim_find(const char *id);

// ---- 单张资源（kind == single）：53 条 ----
extern const lv_image_dsc_t fa_res_city_canola;   // 176x220 rgb565
extern const lv_image_dsc_t fa_res_city_bridge;   // 176x220 rgb565
extern const lv_image_dsc_t fa_res_city_bamboo;   // 176x220 rgb565
extern const lv_image_dsc_t fa_res_city_willow;   // 176x220 rgb565
extern const lv_image_dsc_t fa_res_town_alley;   // 176x220 rgb565
extern const lv_image_dsc_t fa_res_town_lake;   // 176x220 rgb565
extern const lv_image_dsc_t fa_res_town_terrace;   // 176x220 rgb565
extern const lv_image_dsc_t fa_res_town_mudflat;   // 176x220 rgb565
extern const lv_image_dsc_t fa_res_prov_huangshan;   // 176x220 rgb565
extern const lv_image_dsc_t fa_res_prov_lijiang;   // 176x220 rgb565
extern const lv_image_dsc_t fa_res_prov_taishan;   // 176x220 rgb565
extern const lv_image_dsc_t fa_res_prov_wall;   // 176x220 rgb565
extern const lv_image_dsc_t fa_res_far_danxia;   // 176x220 rgb565
extern const lv_image_dsc_t fa_res_far_poplar;   // 176x220 rgb565
extern const lv_image_dsc_t fa_res_far_luguhu;   // 176x220 rgb565
extern const lv_image_dsc_t fa_res_far_kanas;   // 176x220 rgb565
extern const lv_image_dsc_t fa_res_far_daocheng;   // 176x220 rgb565
extern const lv_image_dsc_t fa_res_edge_potala;   // 176x220 rgb565
extern const lv_image_dsc_t fa_res_edge_star;   // 176x220 rgb565
extern const lv_image_dsc_t fa_res_edge_aurora;   // 176x220 rgb565
extern const lv_image_dsc_t fa_res_edge_kekexili;   // 176x220 rgb565
extern const lv_image_dsc_t fa_res_edge_grassland;   // 176x220 rgb565
extern const lv_image_dsc_t fa_res_edge_snowpeak;   // 176x220 rgb565
extern const lv_image_dsc_t fa_res_edge_taklimakan;   // 176x220 rgb565
extern const lv_image_dsc_t fa_res_home;   // 240x320 rgb565
extern const lv_image_dsc_t fa_res_nocat_city_canola;   // 240x320 rgb565
extern const lv_image_dsc_t fa_res_nocat_city_bridge;   // 240x320 rgb565
extern const lv_image_dsc_t fa_res_nocat_city_bamboo;   // 240x320 rgb565
extern const lv_image_dsc_t fa_res_nocat_city_willow;   // 240x320 rgb565
extern const lv_image_dsc_t fa_res_nocat_town_alley;   // 240x320 rgb565
extern const lv_image_dsc_t fa_res_nocat_town_lake;   // 240x320 rgb565
extern const lv_image_dsc_t fa_res_nocat_town_terrace;   // 240x320 rgb565
extern const lv_image_dsc_t fa_res_nocat_town_mudflat;   // 240x320 rgb565
extern const lv_image_dsc_t fa_res_nocat_prov_huangshan;   // 240x320 rgb565
extern const lv_image_dsc_t fa_res_nocat_prov_lijiang;   // 240x320 rgb565
extern const lv_image_dsc_t fa_res_nocat_prov_taishan;   // 240x320 rgb565
extern const lv_image_dsc_t fa_res_nocat_prov_wall;   // 240x320 rgb565
extern const lv_image_dsc_t fa_res_nocat_far_danxia;   // 240x320 rgb565
extern const lv_image_dsc_t fa_res_nocat_far_poplar;   // 240x320 rgb565
extern const lv_image_dsc_t fa_res_nocat_far_luguhu;   // 240x320 rgb565
extern const lv_image_dsc_t fa_res_nocat_far_kanas;   // 240x320 rgb565
extern const lv_image_dsc_t fa_res_nocat_far_daocheng;   // 240x320 rgb565
extern const lv_image_dsc_t fa_res_nocat_edge_potala;   // 240x320 rgb565
extern const lv_image_dsc_t fa_res_nocat_edge_star;   // 240x320 rgb565
extern const lv_image_dsc_t fa_res_nocat_edge_aurora;   // 240x320 rgb565
extern const lv_image_dsc_t fa_res_nocat_edge_kekexili;   // 240x320 rgb565
extern const lv_image_dsc_t fa_res_nocat_edge_grassland;   // 240x320 rgb565
extern const lv_image_dsc_t fa_res_nocat_edge_snowpeak;   // 240x320 rgb565
extern const lv_image_dsc_t fa_res_nocat_edge_taklimakan;   // 240x320 rgb565
extern const lv_image_dsc_t fa_res_batt_full;   // 26x14 rgb565a8
extern const lv_image_dsc_t fa_res_batt_half;   // 26x14 rgb565a8
extern const lv_image_dsc_t fa_res_batt_low;   // 26x14 rgb565a8
extern const lv_image_dsc_t fa_res_mini_cover;   // 176x120 rgb565a8
