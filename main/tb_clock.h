// main/tb_clock.h —— 时间来源管理
//
// 这台机器**没有 RTC 备份电池**，断电时间归零。所以时间有两条来路：
//
//   1. 主路：开机连 WiFi → SNTP 对时 → 之后靠芯片内部 RTC 走
//   2. 兜底：三键手动设时（设置页）
//
// 两者都会把时刻写进 NVS，所以重启后能立刻恢复上次的时间（标为「上次校时」，
// 不是「已校时」—— 因为关机期间是不知道过了多久的）。
//
// 断网也能走：内部 RTC 一直在跑，只是不再被校正。
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <time.h>

typedef enum {
    TB_CLK_NONE = 0,   // 从未有过时间 —— 显示 --:--
    TB_CLK_RESTORED,   // 从 NVS 恢复的上次时间，未校时
    TB_CLK_SYNCING,    // WiFi 已连上，正在等 SNTP 回包
    TB_CLK_SYNCED,     // 本次开机已成功对时
    TB_CLK_MANUAL,     // 本次开机由用户手动设定
} tb_clk_state_t;

// 初始化：NVS + 时区 + （若有凭据）WiFi + SNTP。非阻塞，联网在后台任务里做。
void tb_clock_init(void);

// 现在是否有一个可显示的时间（NONE 状态下没有）
bool tb_clock_valid(void);

// 取本地时间（含年月日时分秒）；无效时返回 false 且不改 *out
bool tb_clock_local(struct tm *out);

// 手动设时（会立刻生效并写入 NVS）
void tb_clock_set_manual(int year, int mon, int day, int hour, int min);

// 把「当前时间」存进 NVS（内部用；外部一般不用调）
void tb_clock_persist(void);

tb_clk_state_t tb_clock_state(void);
bool           tb_clock_wifi_ok(void);
bool           tb_clock_wifi_enabled(void);   // 凭据填了才 true

// ---- 雷霆战机侧追加:状态栏每秒刷一次,只需"时分"和"秒序号" ----

// 当前本地时刻的秒序号(hh*3600 + mm*60 + ss)。无有效时间返回 -1。
// 状态栏用它判断"秒变了没有",避免每帧都去写 label。
int tb_clock_seconds(void);

// 取当前时分的数值。无有效时间返回 false。
bool tb_clock_hm(int *hour, int *min);
