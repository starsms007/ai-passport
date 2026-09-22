// 自动生成，勿手改 —— 由 _dl/_talk45.py 产出。
// 猫咪对我说的话：50 句通用 + 24 处风景各 10 句 + 7 种天气各 10 句 = 360 句。
#pragma once

#include "fa_game.h"      // FA_DEST_COUNT

#define FA_TALK_BASE_N      50
#define FA_TALK_SCENE_N     10
// ★ 天气那一段的两个维度。**注意命名沿用了既有的惯例**：
//   _N 是「每一组几句」（和 FA_TALK_SCENE_N 一样），组数另起一个名字。
//   ★★ 第四十五轮：GROUPS 4 -> **7**（第 4 组改写成萤火，新增飞机/热气球/泡泡）。
//     ⚠ 这个数必须和 fa_view.c 的 **WX_KIND_N** 相等 ——
//       天气号直接当行号用（见 fa_view.c 的 s_wx_talk_kind），
//       这里比那边小的话是**越界读**（读到 BASE 的字符串，屏幕上出现
//       一句牛头不对马嘴的通用台词，而且不报错）。
#define FA_TALK_WX_N        10
#define FA_TALK_WX_GROUPS    7

extern const char *const FA_TALK_BASE [FA_TALK_BASE_N];
extern const char *const FA_TALK_SCENE[FA_DEST_COUNT][FA_TALK_SCENE_N];
extern const char *const FA_TALK_WX   [FA_TALK_WX_GROUPS][FA_TALK_WX_N];
