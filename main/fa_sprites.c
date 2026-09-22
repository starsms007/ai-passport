// 自动生成，勿手改 —— 由 tools/gen_fa_sprites.py 产出（manifest 驱动）。
// 数据来自 <art>/out/*.i8（精灵）与 <art>/out/*.rgb565（满幅素材），已按 manifest 逐 job 转码：
//   agent ∈ FULLBLEED_AGENTS -> RGB565 2B/px（直取 .rgb565，不经 256 色板）
//   其余                      -> RGB565A8（RGB565 平面在前、alpha 平面在后）
//   不认 I8 索引色：LVGL 9 的软件渲染 blend_to_* 表里只有 I1，没有 I8，
//   递 I8 进去会落到 default 分支 —— 一个像素都不画、且不报任何错。
#include "fa_sprites.h"

// ★ EMBED_FILES 生成的符号：fa_sprites.bin 的起始地址（见 main/CMakeLists.txt）。
extern const uint8_t _binary_fa_sprites_bin_start[] asm("_binary_fa_sprites_bin_start");

#define FA_FRAME_AT(off) (const uint8_t *)(_binary_fa_sprites_bin_start + (off))

// ---- cat_idle  64x64 x8  rgb565a8 ----
static const lv_image_dsc_t cat_idle_dsc[8] = {
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 64, .h = 64, .stride = 128 },
      .data_size = 12288, .data = FA_FRAME_AT(0) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 64, .h = 64, .stride = 128 },
      .data_size = 12288, .data = FA_FRAME_AT(12288) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 64, .h = 64, .stride = 128 },
      .data_size = 12288, .data = FA_FRAME_AT(24576) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 64, .h = 64, .stride = 128 },
      .data_size = 12288, .data = FA_FRAME_AT(36864) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 64, .h = 64, .stride = 128 },
      .data_size = 12288, .data = FA_FRAME_AT(49152) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 64, .h = 64, .stride = 128 },
      .data_size = 12288, .data = FA_FRAME_AT(61440) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 64, .h = 64, .stride = 128 },
      .data_size = 12288, .data = FA_FRAME_AT(73728) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 64, .h = 64, .stride = 128 },
      .data_size = 12288, .data = FA_FRAME_AT(86016) },
};

// ---- cat_walk  64x64 x8  rgb565a8 ----
static const lv_image_dsc_t cat_walk_dsc[8] = {
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 64, .h = 64, .stride = 128 },
      .data_size = 12288, .data = FA_FRAME_AT(98304) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 64, .h = 64, .stride = 128 },
      .data_size = 12288, .data = FA_FRAME_AT(110592) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 64, .h = 64, .stride = 128 },
      .data_size = 12288, .data = FA_FRAME_AT(122880) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 64, .h = 64, .stride = 128 },
      .data_size = 12288, .data = FA_FRAME_AT(135168) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 64, .h = 64, .stride = 128 },
      .data_size = 12288, .data = FA_FRAME_AT(147456) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 64, .h = 64, .stride = 128 },
      .data_size = 12288, .data = FA_FRAME_AT(159744) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 64, .h = 64, .stride = 128 },
      .data_size = 12288, .data = FA_FRAME_AT(172032) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 64, .h = 64, .stride = 128 },
      .data_size = 12288, .data = FA_FRAME_AT(184320) },
};

// ---- cat_eat  64x64 x8  rgb565a8 ----
static const lv_image_dsc_t cat_eat_dsc[8] = {
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 64, .h = 64, .stride = 128 },
      .data_size = 12288, .data = FA_FRAME_AT(196608) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 64, .h = 64, .stride = 128 },
      .data_size = 12288, .data = FA_FRAME_AT(208896) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 64, .h = 64, .stride = 128 },
      .data_size = 12288, .data = FA_FRAME_AT(221184) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 64, .h = 64, .stride = 128 },
      .data_size = 12288, .data = FA_FRAME_AT(233472) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 64, .h = 64, .stride = 128 },
      .data_size = 12288, .data = FA_FRAME_AT(245760) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 64, .h = 64, .stride = 128 },
      .data_size = 12288, .data = FA_FRAME_AT(258048) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 64, .h = 64, .stride = 128 },
      .data_size = 12288, .data = FA_FRAME_AT(270336) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 64, .h = 64, .stride = 128 },
      .data_size = 12288, .data = FA_FRAME_AT(282624) },
};

// ---- cat_sleep  64x64 x8  rgb565a8 ----
static const lv_image_dsc_t cat_sleep_dsc[8] = {
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 64, .h = 64, .stride = 128 },
      .data_size = 12288, .data = FA_FRAME_AT(294912) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 64, .h = 64, .stride = 128 },
      .data_size = 12288, .data = FA_FRAME_AT(307200) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 64, .h = 64, .stride = 128 },
      .data_size = 12288, .data = FA_FRAME_AT(319488) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 64, .h = 64, .stride = 128 },
      .data_size = 12288, .data = FA_FRAME_AT(331776) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 64, .h = 64, .stride = 128 },
      .data_size = 12288, .data = FA_FRAME_AT(344064) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 64, .h = 64, .stride = 128 },
      .data_size = 12288, .data = FA_FRAME_AT(356352) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 64, .h = 64, .stride = 128 },
      .data_size = 12288, .data = FA_FRAME_AT(368640) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 64, .h = 64, .stride = 128 },
      .data_size = 12288, .data = FA_FRAME_AT(380928) },
};

// ---- cat_read  64x64 x8  rgb565a8 ----
static const lv_image_dsc_t cat_read_dsc[8] = {
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 64, .h = 64, .stride = 128 },
      .data_size = 12288, .data = FA_FRAME_AT(393216) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 64, .h = 64, .stride = 128 },
      .data_size = 12288, .data = FA_FRAME_AT(405504) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 64, .h = 64, .stride = 128 },
      .data_size = 12288, .data = FA_FRAME_AT(417792) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 64, .h = 64, .stride = 128 },
      .data_size = 12288, .data = FA_FRAME_AT(430080) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 64, .h = 64, .stride = 128 },
      .data_size = 12288, .data = FA_FRAME_AT(442368) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 64, .h = 64, .stride = 128 },
      .data_size = 12288, .data = FA_FRAME_AT(454656) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 64, .h = 64, .stride = 128 },
      .data_size = 12288, .data = FA_FRAME_AT(466944) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 64, .h = 64, .stride = 128 },
      .data_size = 12288, .data = FA_FRAME_AT(479232) },
};

// ---- cat_write  64x64 x8  rgb565a8 ----
static const lv_image_dsc_t cat_write_dsc[8] = {
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 64, .h = 64, .stride = 128 },
      .data_size = 12288, .data = FA_FRAME_AT(491520) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 64, .h = 64, .stride = 128 },
      .data_size = 12288, .data = FA_FRAME_AT(503808) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 64, .h = 64, .stride = 128 },
      .data_size = 12288, .data = FA_FRAME_AT(516096) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 64, .h = 64, .stride = 128 },
      .data_size = 12288, .data = FA_FRAME_AT(528384) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 64, .h = 64, .stride = 128 },
      .data_size = 12288, .data = FA_FRAME_AT(540672) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 64, .h = 64, .stride = 128 },
      .data_size = 12288, .data = FA_FRAME_AT(552960) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 64, .h = 64, .stride = 128 },
      .data_size = 12288, .data = FA_FRAME_AT(565248) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 64, .h = 64, .stride = 128 },
      .data_size = 12288, .data = FA_FRAME_AT(577536) },
};

// ---- cat_pack  64x64 x8  rgb565a8 ----
static const lv_image_dsc_t cat_pack_dsc[8] = {
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 64, .h = 64, .stride = 128 },
      .data_size = 12288, .data = FA_FRAME_AT(589824) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 64, .h = 64, .stride = 128 },
      .data_size = 12288, .data = FA_FRAME_AT(602112) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 64, .h = 64, .stride = 128 },
      .data_size = 12288, .data = FA_FRAME_AT(614400) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 64, .h = 64, .stride = 128 },
      .data_size = 12288, .data = FA_FRAME_AT(626688) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 64, .h = 64, .stride = 128 },
      .data_size = 12288, .data = FA_FRAME_AT(638976) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 64, .h = 64, .stride = 128 },
      .data_size = 12288, .data = FA_FRAME_AT(651264) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 64, .h = 64, .stride = 128 },
      .data_size = 12288, .data = FA_FRAME_AT(663552) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 64, .h = 64, .stride = 128 },
      .data_size = 12288, .data = FA_FRAME_AT(675840) },
};

// ---- cat_wave  64x64 x8  rgb565a8 ----
static const lv_image_dsc_t cat_wave_dsc[8] = {
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 64, .h = 64, .stride = 128 },
      .data_size = 12288, .data = FA_FRAME_AT(688128) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 64, .h = 64, .stride = 128 },
      .data_size = 12288, .data = FA_FRAME_AT(700416) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 64, .h = 64, .stride = 128 },
      .data_size = 12288, .data = FA_FRAME_AT(712704) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 64, .h = 64, .stride = 128 },
      .data_size = 12288, .data = FA_FRAME_AT(724992) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 64, .h = 64, .stride = 128 },
      .data_size = 12288, .data = FA_FRAME_AT(737280) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 64, .h = 64, .stride = 128 },
      .data_size = 12288, .data = FA_FRAME_AT(749568) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 64, .h = 64, .stride = 128 },
      .data_size = 12288, .data = FA_FRAME_AT(761856) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 64, .h = 64, .stride = 128 },
      .data_size = 12288, .data = FA_FRAME_AT(774144) },
};

// ---- cat_carry  64x64 x8  rgb565a8 ----
static const lv_image_dsc_t cat_carry_dsc[8] = {
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 64, .h = 64, .stride = 128 },
      .data_size = 12288, .data = FA_FRAME_AT(786432) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 64, .h = 64, .stride = 128 },
      .data_size = 12288, .data = FA_FRAME_AT(798720) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 64, .h = 64, .stride = 128 },
      .data_size = 12288, .data = FA_FRAME_AT(811008) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 64, .h = 64, .stride = 128 },
      .data_size = 12288, .data = FA_FRAME_AT(823296) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 64, .h = 64, .stride = 128 },
      .data_size = 12288, .data = FA_FRAME_AT(835584) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 64, .h = 64, .stride = 128 },
      .data_size = 12288, .data = FA_FRAME_AT(847872) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 64, .h = 64, .stride = 128 },
      .data_size = 12288, .data = FA_FRAME_AT(860160) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 64, .h = 64, .stride = 128 },
      .data_size = 12288, .data = FA_FRAME_AT(872448) },
};

// ---- snail_b  48x48 x4  rgb565a8 ----
static const lv_image_dsc_t snail_b_dsc[4] = {
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 48, .h = 48, .stride = 96 },
      .data_size = 6912, .data = FA_FRAME_AT(884736) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 48, .h = 48, .stride = 96 },
      .data_size = 6912, .data = FA_FRAME_AT(891648) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 48, .h = 48, .stride = 96 },
      .data_size = 6912, .data = FA_FRAME_AT(898560) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 48, .h = 48, .stride = 96 },
      .data_size = 6912, .data = FA_FRAME_AT(905472) },
};

// ---- bee_a  48x48 x4  rgb565a8 ----
static const lv_image_dsc_t bee_a_dsc[4] = {
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 48, .h = 48, .stride = 96 },
      .data_size = 6912, .data = FA_FRAME_AT(912384) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 48, .h = 48, .stride = 96 },
      .data_size = 6912, .data = FA_FRAME_AT(919296) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 48, .h = 48, .stride = 96 },
      .data_size = 6912, .data = FA_FRAME_AT(926208) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 48, .h = 48, .stride = 96 },
      .data_size = 6912, .data = FA_FRAME_AT(933120) },
};

// ---- butterfly_a  48x48 x4  rgb565a8 ----
static const lv_image_dsc_t butterfly_a_dsc[4] = {
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 48, .h = 48, .stride = 96 },
      .data_size = 6912, .data = FA_FRAME_AT(940032) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 48, .h = 48, .stride = 96 },
      .data_size = 6912, .data = FA_FRAME_AT(946944) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 48, .h = 48, .stride = 96 },
      .data_size = 6912, .data = FA_FRAME_AT(953856) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 48, .h = 48, .stride = 96 },
      .data_size = 6912, .data = FA_FRAME_AT(960768) },
};

// ---- turtle_b  48x48 x4  rgb565a8 ----
static const lv_image_dsc_t turtle_b_dsc[4] = {
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 48, .h = 48, .stride = 96 },
      .data_size = 6912, .data = FA_FRAME_AT(967680) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 48, .h = 48, .stride = 96 },
      .data_size = 6912, .data = FA_FRAME_AT(974592) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 48, .h = 48, .stride = 96 },
      .data_size = 6912, .data = FA_FRAME_AT(981504) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 48, .h = 48, .stride = 96 },
      .data_size = 6912, .data = FA_FRAME_AT(988416) },
};

// ---- fx_snow  24x24 x4  rgb565a8 ----
static const lv_image_dsc_t fx_snow_dsc[4] = {
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 24, .h = 24, .stride = 48 },
      .data_size = 1728, .data = FA_FRAME_AT(995328) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 24, .h = 24, .stride = 48 },
      .data_size = 1728, .data = FA_FRAME_AT(997056) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 24, .h = 24, .stride = 48 },
      .data_size = 1728, .data = FA_FRAME_AT(998784) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 24, .h = 24, .stride = 48 },
      .data_size = 1728, .data = FA_FRAME_AT(1000512) },
};

// ---- souvenir_a  44x44 x12  rgb565a8 ----
static const lv_image_dsc_t souvenir_a_dsc[12] = {
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 44, .h = 44, .stride = 88 },
      .data_size = 5808, .data = FA_FRAME_AT(1002240) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 44, .h = 44, .stride = 88 },
      .data_size = 5808, .data = FA_FRAME_AT(1008048) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 44, .h = 44, .stride = 88 },
      .data_size = 5808, .data = FA_FRAME_AT(1013856) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 44, .h = 44, .stride = 88 },
      .data_size = 5808, .data = FA_FRAME_AT(1019664) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 44, .h = 44, .stride = 88 },
      .data_size = 5808, .data = FA_FRAME_AT(1025472) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 44, .h = 44, .stride = 88 },
      .data_size = 5808, .data = FA_FRAME_AT(1031280) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 44, .h = 44, .stride = 88 },
      .data_size = 5808, .data = FA_FRAME_AT(1037088) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 44, .h = 44, .stride = 88 },
      .data_size = 5808, .data = FA_FRAME_AT(1042896) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 44, .h = 44, .stride = 88 },
      .data_size = 5808, .data = FA_FRAME_AT(1048704) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 44, .h = 44, .stride = 88 },
      .data_size = 5808, .data = FA_FRAME_AT(1054512) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 44, .h = 44, .stride = 88 },
      .data_size = 5808, .data = FA_FRAME_AT(1060320) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 44, .h = 44, .stride = 88 },
      .data_size = 5808, .data = FA_FRAME_AT(1066128) },
};

// ---- souvenir_b  44x44 x12  rgb565a8 ----
static const lv_image_dsc_t souvenir_b_dsc[12] = {
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 44, .h = 44, .stride = 88 },
      .data_size = 5808, .data = FA_FRAME_AT(1071936) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 44, .h = 44, .stride = 88 },
      .data_size = 5808, .data = FA_FRAME_AT(1077744) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 44, .h = 44, .stride = 88 },
      .data_size = 5808, .data = FA_FRAME_AT(1083552) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 44, .h = 44, .stride = 88 },
      .data_size = 5808, .data = FA_FRAME_AT(1089360) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 44, .h = 44, .stride = 88 },
      .data_size = 5808, .data = FA_FRAME_AT(1095168) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 44, .h = 44, .stride = 88 },
      .data_size = 5808, .data = FA_FRAME_AT(1100976) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 44, .h = 44, .stride = 88 },
      .data_size = 5808, .data = FA_FRAME_AT(1106784) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 44, .h = 44, .stride = 88 },
      .data_size = 5808, .data = FA_FRAME_AT(1112592) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 44, .h = 44, .stride = 88 },
      .data_size = 5808, .data = FA_FRAME_AT(1118400) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 44, .h = 44, .stride = 88 },
      .data_size = 5808, .data = FA_FRAME_AT(1124208) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 44, .h = 44, .stride = 88 },
      .data_size = 5808, .data = FA_FRAME_AT(1130016) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 44, .h = 44, .stride = 88 },
      .data_size = 5808, .data = FA_FRAME_AT(1135824) },
};

// ---- fx_drop  24x24 x4  rgb565a8 ----
static const lv_image_dsc_t fx_drop_dsc[4] = {
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 24, .h = 24, .stride = 48 },
      .data_size = 1728, .data = FA_FRAME_AT(6906864) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 24, .h = 24, .stride = 48 },
      .data_size = 1728, .data = FA_FRAME_AT(6908592) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 24, .h = 24, .stride = 48 },
      .data_size = 1728, .data = FA_FRAME_AT(6910320) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 24, .h = 24, .stride = 48 },
      .data_size = 1728, .data = FA_FRAME_AT(6912048) },
};

// ---- fx_firefly  24x24 x4  rgb565a8 ----
static const lv_image_dsc_t fx_firefly_dsc[4] = {
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 24, .h = 24, .stride = 48 },
      .data_size = 1728, .data = FA_FRAME_AT(6913776) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 24, .h = 24, .stride = 48 },
      .data_size = 1728, .data = FA_FRAME_AT(6915504) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 24, .h = 24, .stride = 48 },
      .data_size = 1728, .data = FA_FRAME_AT(6917232) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 24, .h = 24, .stride = 48 },
      .data_size = 1728, .data = FA_FRAME_AT(6918960) },
};

// ---- fx_plane  24x24 x2  rgb565a8 ----
static const lv_image_dsc_t fx_plane_dsc[2] = {
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 24, .h = 24, .stride = 48 },
      .data_size = 1728, .data = FA_FRAME_AT(6920688) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 24, .h = 24, .stride = 48 },
      .data_size = 1728, .data = FA_FRAME_AT(6922416) },
};

// ---- fx_balloon  24x24 x4  rgb565a8 ----
static const lv_image_dsc_t fx_balloon_dsc[4] = {
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 24, .h = 24, .stride = 48 },
      .data_size = 1728, .data = FA_FRAME_AT(6924144) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 24, .h = 24, .stride = 48 },
      .data_size = 1728, .data = FA_FRAME_AT(6925872) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 24, .h = 24, .stride = 48 },
      .data_size = 1728, .data = FA_FRAME_AT(6927600) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 24, .h = 24, .stride = 48 },
      .data_size = 1728, .data = FA_FRAME_AT(6929328) },
};

// ---- fx_soap  24x24 x4  rgb565a8 ----
static const lv_image_dsc_t fx_soap_dsc[4] = {
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 24, .h = 24, .stride = 48 },
      .data_size = 1728, .data = FA_FRAME_AT(6931056) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 24, .h = 24, .stride = 48 },
      .data_size = 1728, .data = FA_FRAME_AT(6932784) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 24, .h = 24, .stride = 48 },
      .data_size = 1728, .data = FA_FRAME_AT(6934512) },
    { .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 24, .h = 24, .stride = 48 },
      .data_size = 1728, .data = FA_FRAME_AT(6936240) },
};

// ---- 单张资源（kind == single）—— 导出 fa_res_<id> ----
// city_canola  176x220  rgb565
const lv_image_dsc_t fa_res_city_canola = {
    .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565,
                  .flags = 0, .w = 176, .h = 220, .stride = 352 },
      .data_size = 77440, .data = FA_FRAME_AT(1141632) };


// city_bridge  176x220  rgb565
const lv_image_dsc_t fa_res_city_bridge = {
    .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565,
                  .flags = 0, .w = 176, .h = 220, .stride = 352 },
      .data_size = 77440, .data = FA_FRAME_AT(1219072) };


// city_bamboo  176x220  rgb565
const lv_image_dsc_t fa_res_city_bamboo = {
    .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565,
                  .flags = 0, .w = 176, .h = 220, .stride = 352 },
      .data_size = 77440, .data = FA_FRAME_AT(1296512) };


// city_willow  176x220  rgb565
const lv_image_dsc_t fa_res_city_willow = {
    .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565,
                  .flags = 0, .w = 176, .h = 220, .stride = 352 },
      .data_size = 77440, .data = FA_FRAME_AT(1373952) };


// town_alley  176x220  rgb565
const lv_image_dsc_t fa_res_town_alley = {
    .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565,
                  .flags = 0, .w = 176, .h = 220, .stride = 352 },
      .data_size = 77440, .data = FA_FRAME_AT(1451392) };


// town_lake  176x220  rgb565
const lv_image_dsc_t fa_res_town_lake = {
    .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565,
                  .flags = 0, .w = 176, .h = 220, .stride = 352 },
      .data_size = 77440, .data = FA_FRAME_AT(1528832) };


// town_terrace  176x220  rgb565
const lv_image_dsc_t fa_res_town_terrace = {
    .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565,
                  .flags = 0, .w = 176, .h = 220, .stride = 352 },
      .data_size = 77440, .data = FA_FRAME_AT(1606272) };


// town_mudflat  176x220  rgb565
const lv_image_dsc_t fa_res_town_mudflat = {
    .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565,
                  .flags = 0, .w = 176, .h = 220, .stride = 352 },
      .data_size = 77440, .data = FA_FRAME_AT(1683712) };


// prov_huangshan  176x220  rgb565
const lv_image_dsc_t fa_res_prov_huangshan = {
    .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565,
                  .flags = 0, .w = 176, .h = 220, .stride = 352 },
      .data_size = 77440, .data = FA_FRAME_AT(1761152) };


// prov_lijiang  176x220  rgb565
const lv_image_dsc_t fa_res_prov_lijiang = {
    .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565,
                  .flags = 0, .w = 176, .h = 220, .stride = 352 },
      .data_size = 77440, .data = FA_FRAME_AT(1838592) };


// prov_taishan  176x220  rgb565
const lv_image_dsc_t fa_res_prov_taishan = {
    .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565,
                  .flags = 0, .w = 176, .h = 220, .stride = 352 },
      .data_size = 77440, .data = FA_FRAME_AT(1916032) };


// prov_wall  176x220  rgb565
const lv_image_dsc_t fa_res_prov_wall = {
    .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565,
                  .flags = 0, .w = 176, .h = 220, .stride = 352 },
      .data_size = 77440, .data = FA_FRAME_AT(1993472) };


// far_danxia  176x220  rgb565
const lv_image_dsc_t fa_res_far_danxia = {
    .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565,
                  .flags = 0, .w = 176, .h = 220, .stride = 352 },
      .data_size = 77440, .data = FA_FRAME_AT(2070912) };


// far_poplar  176x220  rgb565
const lv_image_dsc_t fa_res_far_poplar = {
    .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565,
                  .flags = 0, .w = 176, .h = 220, .stride = 352 },
      .data_size = 77440, .data = FA_FRAME_AT(2148352) };


// far_luguhu  176x220  rgb565
const lv_image_dsc_t fa_res_far_luguhu = {
    .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565,
                  .flags = 0, .w = 176, .h = 220, .stride = 352 },
      .data_size = 77440, .data = FA_FRAME_AT(2225792) };


// far_kanas  176x220  rgb565
const lv_image_dsc_t fa_res_far_kanas = {
    .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565,
                  .flags = 0, .w = 176, .h = 220, .stride = 352 },
      .data_size = 77440, .data = FA_FRAME_AT(2303232) };


// far_daocheng  176x220  rgb565
const lv_image_dsc_t fa_res_far_daocheng = {
    .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565,
                  .flags = 0, .w = 176, .h = 220, .stride = 352 },
      .data_size = 77440, .data = FA_FRAME_AT(2380672) };


// edge_potala  176x220  rgb565
const lv_image_dsc_t fa_res_edge_potala = {
    .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565,
                  .flags = 0, .w = 176, .h = 220, .stride = 352 },
      .data_size = 77440, .data = FA_FRAME_AT(2458112) };


// edge_star  176x220  rgb565
const lv_image_dsc_t fa_res_edge_star = {
    .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565,
                  .flags = 0, .w = 176, .h = 220, .stride = 352 },
      .data_size = 77440, .data = FA_FRAME_AT(2535552) };


// edge_aurora  176x220  rgb565
const lv_image_dsc_t fa_res_edge_aurora = {
    .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565,
                  .flags = 0, .w = 176, .h = 220, .stride = 352 },
      .data_size = 77440, .data = FA_FRAME_AT(2612992) };


// edge_kekexili  176x220  rgb565
const lv_image_dsc_t fa_res_edge_kekexili = {
    .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565,
                  .flags = 0, .w = 176, .h = 220, .stride = 352 },
      .data_size = 77440, .data = FA_FRAME_AT(2690432) };


// edge_grassland  176x220  rgb565
const lv_image_dsc_t fa_res_edge_grassland = {
    .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565,
                  .flags = 0, .w = 176, .h = 220, .stride = 352 },
      .data_size = 77440, .data = FA_FRAME_AT(2767872) };


// edge_snowpeak  176x220  rgb565
const lv_image_dsc_t fa_res_edge_snowpeak = {
    .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565,
                  .flags = 0, .w = 176, .h = 220, .stride = 352 },
      .data_size = 77440, .data = FA_FRAME_AT(2845312) };


// edge_taklimakan  176x220  rgb565
const lv_image_dsc_t fa_res_edge_taklimakan = {
    .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565,
                  .flags = 0, .w = 176, .h = 220, .stride = 352 },
      .data_size = 77440, .data = FA_FRAME_AT(2922752) };


// home  240x320  rgb565
const lv_image_dsc_t fa_res_home = {
    .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565,
                  .flags = 0, .w = 240, .h = 320, .stride = 480 },
      .data_size = 153600, .data = FA_FRAME_AT(3000192) };


// nocat_city_canola  240x320  rgb565
const lv_image_dsc_t fa_res_nocat_city_canola = {
    .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565,
                  .flags = 0, .w = 240, .h = 320, .stride = 480 },
      .data_size = 153600, .data = FA_FRAME_AT(3153792) };


// nocat_city_bridge  240x320  rgb565
const lv_image_dsc_t fa_res_nocat_city_bridge = {
    .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565,
                  .flags = 0, .w = 240, .h = 320, .stride = 480 },
      .data_size = 153600, .data = FA_FRAME_AT(3307392) };


// nocat_city_bamboo  240x320  rgb565
const lv_image_dsc_t fa_res_nocat_city_bamboo = {
    .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565,
                  .flags = 0, .w = 240, .h = 320, .stride = 480 },
      .data_size = 153600, .data = FA_FRAME_AT(3460992) };


// nocat_city_willow  240x320  rgb565
const lv_image_dsc_t fa_res_nocat_city_willow = {
    .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565,
                  .flags = 0, .w = 240, .h = 320, .stride = 480 },
      .data_size = 153600, .data = FA_FRAME_AT(3614592) };


// nocat_town_alley  240x320  rgb565
const lv_image_dsc_t fa_res_nocat_town_alley = {
    .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565,
                  .flags = 0, .w = 240, .h = 320, .stride = 480 },
      .data_size = 153600, .data = FA_FRAME_AT(3768192) };


// nocat_town_lake  240x320  rgb565
const lv_image_dsc_t fa_res_nocat_town_lake = {
    .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565,
                  .flags = 0, .w = 240, .h = 320, .stride = 480 },
      .data_size = 153600, .data = FA_FRAME_AT(3921792) };


// nocat_town_terrace  240x320  rgb565
const lv_image_dsc_t fa_res_nocat_town_terrace = {
    .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565,
                  .flags = 0, .w = 240, .h = 320, .stride = 480 },
      .data_size = 153600, .data = FA_FRAME_AT(4075392) };


// nocat_town_mudflat  240x320  rgb565
const lv_image_dsc_t fa_res_nocat_town_mudflat = {
    .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565,
                  .flags = 0, .w = 240, .h = 320, .stride = 480 },
      .data_size = 153600, .data = FA_FRAME_AT(4228992) };


// nocat_prov_huangshan  240x320  rgb565
const lv_image_dsc_t fa_res_nocat_prov_huangshan = {
    .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565,
                  .flags = 0, .w = 240, .h = 320, .stride = 480 },
      .data_size = 153600, .data = FA_FRAME_AT(4382592) };


// nocat_prov_lijiang  240x320  rgb565
const lv_image_dsc_t fa_res_nocat_prov_lijiang = {
    .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565,
                  .flags = 0, .w = 240, .h = 320, .stride = 480 },
      .data_size = 153600, .data = FA_FRAME_AT(4536192) };


// nocat_prov_taishan  240x320  rgb565
const lv_image_dsc_t fa_res_nocat_prov_taishan = {
    .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565,
                  .flags = 0, .w = 240, .h = 320, .stride = 480 },
      .data_size = 153600, .data = FA_FRAME_AT(4689792) };


// nocat_prov_wall  240x320  rgb565
const lv_image_dsc_t fa_res_nocat_prov_wall = {
    .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565,
                  .flags = 0, .w = 240, .h = 320, .stride = 480 },
      .data_size = 153600, .data = FA_FRAME_AT(4843392) };


// nocat_far_danxia  240x320  rgb565
const lv_image_dsc_t fa_res_nocat_far_danxia = {
    .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565,
                  .flags = 0, .w = 240, .h = 320, .stride = 480 },
      .data_size = 153600, .data = FA_FRAME_AT(4996992) };


// nocat_far_poplar  240x320  rgb565
const lv_image_dsc_t fa_res_nocat_far_poplar = {
    .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565,
                  .flags = 0, .w = 240, .h = 320, .stride = 480 },
      .data_size = 153600, .data = FA_FRAME_AT(5150592) };


// nocat_far_luguhu  240x320  rgb565
const lv_image_dsc_t fa_res_nocat_far_luguhu = {
    .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565,
                  .flags = 0, .w = 240, .h = 320, .stride = 480 },
      .data_size = 153600, .data = FA_FRAME_AT(5304192) };


// nocat_far_kanas  240x320  rgb565
const lv_image_dsc_t fa_res_nocat_far_kanas = {
    .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565,
                  .flags = 0, .w = 240, .h = 320, .stride = 480 },
      .data_size = 153600, .data = FA_FRAME_AT(5457792) };


// nocat_far_daocheng  240x320  rgb565
const lv_image_dsc_t fa_res_nocat_far_daocheng = {
    .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565,
                  .flags = 0, .w = 240, .h = 320, .stride = 480 },
      .data_size = 153600, .data = FA_FRAME_AT(5611392) };


// nocat_edge_potala  240x320  rgb565
const lv_image_dsc_t fa_res_nocat_edge_potala = {
    .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565,
                  .flags = 0, .w = 240, .h = 320, .stride = 480 },
      .data_size = 153600, .data = FA_FRAME_AT(5764992) };


// nocat_edge_star  240x320  rgb565
const lv_image_dsc_t fa_res_nocat_edge_star = {
    .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565,
                  .flags = 0, .w = 240, .h = 320, .stride = 480 },
      .data_size = 153600, .data = FA_FRAME_AT(5918592) };


// nocat_edge_aurora  240x320  rgb565
const lv_image_dsc_t fa_res_nocat_edge_aurora = {
    .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565,
                  .flags = 0, .w = 240, .h = 320, .stride = 480 },
      .data_size = 153600, .data = FA_FRAME_AT(6072192) };


// nocat_edge_kekexili  240x320  rgb565
const lv_image_dsc_t fa_res_nocat_edge_kekexili = {
    .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565,
                  .flags = 0, .w = 240, .h = 320, .stride = 480 },
      .data_size = 153600, .data = FA_FRAME_AT(6225792) };


// nocat_edge_grassland  240x320  rgb565
const lv_image_dsc_t fa_res_nocat_edge_grassland = {
    .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565,
                  .flags = 0, .w = 240, .h = 320, .stride = 480 },
      .data_size = 153600, .data = FA_FRAME_AT(6379392) };


// nocat_edge_snowpeak  240x320  rgb565
const lv_image_dsc_t fa_res_nocat_edge_snowpeak = {
    .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565,
                  .flags = 0, .w = 240, .h = 320, .stride = 480 },
      .data_size = 153600, .data = FA_FRAME_AT(6532992) };


// nocat_edge_taklimakan  240x320  rgb565
const lv_image_dsc_t fa_res_nocat_edge_taklimakan = {
    .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565,
                  .flags = 0, .w = 240, .h = 320, .stride = 480 },
      .data_size = 153600, .data = FA_FRAME_AT(6686592) };


// batt_full  26x14  rgb565a8
const lv_image_dsc_t fa_res_batt_full = {
    .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 26, .h = 14, .stride = 52 },
      .data_size = 1092, .data = FA_FRAME_AT(6840192) };


// batt_half  26x14  rgb565a8
const lv_image_dsc_t fa_res_batt_half = {
    .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 26, .h = 14, .stride = 52 },
      .data_size = 1092, .data = FA_FRAME_AT(6841296) };


// batt_low  26x14  rgb565a8
const lv_image_dsc_t fa_res_batt_low = {
    .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 26, .h = 14, .stride = 52 },
      .data_size = 1092, .data = FA_FRAME_AT(6842400) };


// mini_cover  176x120  rgb565a8
const lv_image_dsc_t fa_res_mini_cover = {
    .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565A8,
                  .flags = 0, .w = 176, .h = 120, .stride = 352 },
      .data_size = 63360, .data = FA_FRAME_AT(6843504) };


// ---- 动画总表（只含多帧 job）----
const fa_anim_t fa_anims[FA_ANIM_COUNT] = {
    { "cat_idle",  64,  64,  8, cat_idle_dsc },
    { "cat_walk",  64,  64,  8, cat_walk_dsc },
    { "cat_eat",  64,  64,  8, cat_eat_dsc },
    { "cat_sleep",  64,  64,  8, cat_sleep_dsc },
    { "cat_read",  64,  64,  8, cat_read_dsc },
    { "cat_write",  64,  64,  8, cat_write_dsc },
    { "cat_pack",  64,  64,  8, cat_pack_dsc },
    { "cat_wave",  64,  64,  8, cat_wave_dsc },
    { "cat_carry",  64,  64,  8, cat_carry_dsc },
    { "snail_b",  48,  48,  4, snail_b_dsc },
    { "bee_a",  48,  48,  4, bee_a_dsc },
    { "butterfly_a",  48,  48,  4, butterfly_a_dsc },
    { "turtle_b",  48,  48,  4, turtle_b_dsc },
    { "fx_snow",  24,  24,  4, fx_snow_dsc },
    { "souvenir_a",  44,  44, 12, souvenir_a_dsc },
    { "souvenir_b",  44,  44, 12, souvenir_b_dsc },
    { "fx_drop",  24,  24,  4, fx_drop_dsc },
    { "fx_firefly",  24,  24,  4, fx_firefly_dsc },
    { "fx_plane",  24,  24,  2, fx_plane_dsc },
    { "fx_balloon",  24,  24,  4, fx_balloon_dsc },
    { "fx_soap",  24,  24,  4, fx_soap_dsc },
};

// ---- 查找 ----
const fa_anim_t *fa_anim_find(const char *id)
{
    if (!id) return NULL;
    for (int i = 0; i < FA_ANIM_COUNT; i++) {
        const char *a = fa_anims[i].id;
        const char *b = id;
        while (*a && *a == *b) { a++; b++; }
        if (*a == 0 && *b == 0) return &fa_anims[i];
    }
    return NULL;
}
