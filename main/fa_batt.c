// main/fa_batt.c —— CW2017 电量计的自愈看护。来龙去脉见 fa_batt.h。
#include "fa_batt.h"

#include "bsp_battery.h"
#include "bsp_i2c.h"

#include "driver/i2c_master.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

static const char *TAG = "fa_batt";

// 两次自愈之间的最小间隔。取 60 秒的理由：
//   bsp_battery_init() 在「芯片在、但 SOC 一直算不出来」那一路会**阻塞 5 秒**
//   （cw_wait_soc_ready 里 50 × 100ms）才返回失败。若不限频，一次真硬件故障
//   就变成「每 10 秒占住 I2C 总线与 CPU 5 秒」，而且日志刷屏。
#define RECOVER_MIN_GAP_MS 60000u

// ★★ 2026-09-29（r56）判据换成**时间维度**，不再依赖电压。为什么要换：
//
//   r55 的判据是「SOC ≤ 3% 且电压 ≥ 3.6V」—— 靠「SOC 说没电、电压却还有」
//   这对矛盾来识别占位值。但它有一条天然边界：**隔夜 + 低温时电压本来就低**，
//   判据认为「SOC 低、电压也低，自洽」⇒ 照样放行一个占位 0。
//   用户 09-29 早上真机漏出来的正是这一条（开机 0%，充几分钟跳到 34%）。
//
//   物理事实（CW2017 官方 datasheet：No Sense Resistor Required）：
//   这颗芯片**没有采样电阻、根本不是库仑计** —— 它只测电压，再拿一份电池
//   模型反推 SOC。芯片被（重新）上电后，SOC 寄存器本来就停在 0，要用当前
//   电压重新收敛 —— 这段时间的读数与电池实际余量无关，**一律不可信**。
//   ⇒ 判据只看「离最近一次上电过了多久」：窗口内 SOC ≤ 3 一律 WARMING。
//
//   ⚠ 不掩盖真低电：窗口一过照常显示，真没电照样红 0% + 低电告警。
//   ⚠ 不再需要「轮数兜底」—— 时间窗口本身就是兜底（90 秒后必然放行）。
//     这同时消掉了 r55 的一个瑕疵：原来的 6 轮兜底是按 10 秒档写的
//     （注释说 ≈60 秒），可开机实际走 3 秒档 ⇒ 真实保护窗口只有 ≈18 秒。
//     改成绝对时间后，跟轮询节奏解耦，不会再被档位变化悄悄缩短。
#define WARMING_WINDOW_MS 90000
#define WARMING_SOC_MAX   3

// ★★ 2026-10-04（r57，候选 1）**绝对上限**：从**开机**起算，最多只压 180 秒。
//
//   09-30 只读诊断 §3 漏洞 1 实查出来的病灶：
//     note_reinit() 在看护任务里是**无条件**调用的；而 bsp_battery_init() 的
//     第一行就是 `if (s_dev) return ESP_OK;` —— 句柄还在时它是**空操作**，
//     芯片根本没重新上电。可窗口照样被重置成「现在」。
//     偏偏 WARMING 分支还会把 60 秒限频清零 ⇒ **一次瞬时 NACK 就能换来
//     又 90 秒空窗**；反复发生，窗口永远出不去，数字也就永远不出现。
//   ⇒ 两条一起上：
//     ① 上电探针（见 guard_task）：只有 init **可能真的重建过设备**时才续期；
//     ② 这条绝对上限：无论怎么续期，开机 180 秒后**必然放行**真实读数。
//   ⚠ 180 秒是给"芯片重新收敛"留的余量，不是给"无限重试"留的口子。
#define WARMING_HARD_CAP_MS 180000

// ★★ 2026-10-04（r57，候选 3）自愈升级：init 超时不再干等 60 秒。
//   诊断 §3 漏洞 2：init 返回 ESP_ERR_TIMEOUT（SOC 5 秒没就绪）时，原逻辑
//   把这轮记为失败，然后**干等 60 秒**才轮到下一次。同一轮内立即重试几次
//   几乎不花代价（芯片就在总线上），却能把"偶发一次没就绪"直接吃掉。
#define HEAL_INIT_RETRY   3
#define HEAL_RETRY_GAP_MS 300

// ★★ 2026-10-04（r57，候选 2）认账阈值：连续这么多轮自愈都**没成功**
//   ⇒ 界面把那一格显示成 "--%" 而不是空白（见 fa_batt_unknown）。
#define UNKNOWN_HEAL_FAILS 2

static SemaphoreHandle_t s_lock;
static volatile bool     s_req;
static bool              s_started;

// 芯片最近一次（真正）被重新初始化的时刻（微秒）。0 = 还没记过。
//   ★ 开机那次（main.c 调 bsp_battery_init）与**真正重建成功**的自愈那次都算。
//   ⚠ 由 LVGL 任务（fa_batt_read）和自愈任务（guard_task）共同读写，
//     所以**必须只在 fa_batt_try_lock() 的锁内动它**。
static int64_t s_win_start_us;
// ★ r57：开机时刻（首次 fa_batt_read）。绝对上限的唯一基准。
static int64_t s_boot_us;
// 「窗口已过、放行了一个低电读数」是否已经报过 —— 只影响日志（避免每 3 秒
// 刷一行），不影响保护语义。
static bool s_low_warned;
// ★ r57：本轮自愈是否**真的续期过**窗口。用于决定要不要清零 60 秒限频
//   （没续期却清零 = 诊断漏洞 1 的那条链子）。
static bool s_win_renewed;

// ★ r57 候选 2 的状态：连续「自愈未成功」的轮数，以及对外那个"认账"标志。
static int  s_heal_fails;
static bool s_unknown;

// 上一次的读数状态 —— 只用来「状态翻转时打一条日志」，避免每 10 秒刷一行。
static fa_batt_state_t s_last_state = FA_BATT_OK;
static bool            s_state_valid;

static const char *state_name(fa_batt_state_t s)
{
    switch (s) {
        case FA_BATT_OK:      return "OK";
        case FA_BATT_WARMING: return "WARMING";
        default:              return "FAIL";
    }
}

// ★ 记一次「芯片刚被（重新）上电」，收敛窗口从这里重新计时。
//   ⚠ 调用方**必须**持锁（fa_batt_read 读 s_win_start_us 也在锁内）。
//   ⚠ r57 起**不是无条件调**了：只有"init 前探到芯片不应答、且 init 成功"
//     才算真的重新上电（见 guard_task 里的注释）。
static void note_reinit(void)
{
    s_win_start_us = esp_timer_get_time();
    s_low_warned   = false;
    s_win_renewed  = true;
}

bool fa_batt_unknown(void)
{
    return s_unknown;
}

fa_batt_state_t fa_batt_read(int *soc_out, int *mv_out)
{
    int soc = bsp_battery_soc();
    int mv  = bsp_battery_mv();
    if (soc_out) *soc_out = soc;
    if (mv_out)  *mv_out  = mv;

    // ★★ 时间判据（r56）：从最近一次上电算起的一段「收敛窗口」内，
    //   SOC 读数不可信 —— 芯片这时正拿当前电压重算，跟电池实际余量无关。
    int64_t now = esp_timer_get_time();
    if (s_boot_us == 0)      s_boot_us      = now;   // 首次调用＝开机
    if (s_win_start_us == 0) s_win_start_us = now;   // 首次调用＝开机
    // ★ r57 候选 1：两个条件**都要**满足才压读数 —— 窗口内 **且** 没到开机绝对上限。
    //   第二个条件就是"窗口不可被无限续期"的硬保证。
    const bool within_win = (now - s_win_start_us) < ((int64_t)WARMING_WINDOW_MS    * 1000);
    const bool within_cap = (now - s_boot_us)      < ((int64_t)WARMING_HARD_CAP_MS * 1000);
    bool warm_win = within_win && within_cap;

    fa_batt_state_t st;
    if (soc < 0) {
        // I2C 失败 / 芯片不应答 / 未就绪时读到 0xFF（驱动已把 >100 归成 -1）。
        st = FA_BATT_FAIL;
    } else if (soc <= WARMING_SOC_MAX && warm_win) {
        // ★ 芯片刚上电、收敛窗口内读到「几乎没电」—— 这是占位值，不是故障。
        //   等它算完即可；**不看电压**（电压在低温/隔夜时本来就低，靠不住）。
        st = FA_BATT_WARMING;
    } else {
        if (soc <= WARMING_SOC_MAX && !s_low_warned) {
            // ★★ 兜底放行：窗口已过还 ≤3% ⇒ 认账，当**真实低电**处理
            //   （低电告警继续工作）。这条日志格式与上面那条不同，
            //   接串口时一眼能分清「正常读数」和「窗口过期硬放行」。
            //   ★ r57：把"为什么窗口结束了"也写清楚 —— 是 90 秒到了，
            //     还是被 180 秒的绝对上限截断的（后者说明窗口一直在被续期）。
            ESP_LOGW(TAG, "收敛窗口已过（%s），放行低电读数 SOC=%d%% mV=%d —— 按真实低电处理",
                     within_cap ? "窗口 90 秒到期" : "撞到 180 秒绝对上限", soc, mv);
            s_low_warned = true;
        }
        st = FA_BATT_OK;
    }
    // 读到明显健康的电量就解除「已报过」标记，下次真放行还能再报一条。
    if (soc > WARMING_SOC_MAX) s_low_warned = false;

    if (st != s_last_state || !s_state_valid) {
        ESP_LOGI(TAG, "电量读数 SOC=%d%% mV=%d -> %s（上电后 %d 秒）",
                 soc, mv, state_name(st), (int)((now - s_win_start_us) / 1000000));
        s_last_state  = st;
        s_state_valid = true;
    }
    // ★ r57 候选 2：读到有效值就把"认账"撤掉（界面会自己把数字换回来）。
    if (st == FA_BATT_OK) s_unknown = false;
    return st;
}

bool fa_batt_try_lock(void)
{
    if (!s_lock) return true;        // 锁与任务一起建，这里只是兜底
    return xSemaphoreTake(s_lock, 0) == pdTRUE;
}

void fa_batt_unlock(void)
{
    if (s_lock) xSemaphoreGive(s_lock);
}

void fa_batt_request_recover(void)
{
    s_req = true;
}

static void guard_task(void *arg)
{
    (void)arg;
    uint32_t last_ms = 0;

    for (;;) {
        // 500ms 醒一次看一眼标志 —— 不用事件组/通知，只是为了省掉一个
        // 跨任务句柄；这点空转开销可以忽略。
        vTaskDelay(pdMS_TO_TICKS(500));
        if (!s_req) continue;
        s_req = false;

        uint32_t now = (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);
        if (last_ms && (now - last_ms) < RECOVER_MIN_GAP_MS) continue;
        last_ms = now;

        // ★ 这里可以用**阻塞式**取锁：LVGL 侧用的是 try-lock，最长只持有
        //   一次 I2C 读的时间（毫秒级），等一下就好 —— 反过来把 UI 卡住
        //   是绝不允许的，所以方向必须是「自愈等 UI」，不能是「UI 等自愈」。
        if (s_lock) xSemaphoreTake(s_lock, portMAX_DELAY);

        s_win_renewed = false;

        // ★★ r57 候选 1 的探针：**在 init 之前**问一次芯片。
        //   bsp_battery.h 没有暴露"设备是否已就绪"，而 components/bsp/ 不许改
        //   （与官方逐字节一致的约束）⇒ 只能用公开 API 探。
        //   · soc() ≥ 0 ⇒ 句柄还在、I2C 通 ⇒ 紧接着的 bsp_battery_init()
        //     会走 `if (s_dev) return ESP_OK;` ＝**空操作**，芯片没重新上电
        //     ⇒ **绝不续期窗口**（续期就是凭空送 90 秒空窗）。
        //   · soc() < 0 ⇒ 句柄可能已空/芯片不应答 ⇒ init 真的会重建。
        //   ⚠ 残留不精确：若这次 NACK 是瞬时的、句柄其实还在，会误判成
        //     "可能重建"而续期一次 —— 所以还有 180 秒绝对上限兜底。
        int probe = bsp_battery_soc();

        // ★★ r57 候选 3①：复位总线**提前**到重跑 init 之前。
        //   诊断 §3 漏洞 2：复位排在 init 之后时，若 init 已把 s_dev 置空，
        //   复位后那次读必然还是 -1（cw_read 见 !s_dev 直接报错）⇒ 复位白做。
        //   先复位控制器时序，再让 init 去重挂设备，才是有效顺序。
        if (probe < 0) {
            i2c_master_bus_handle_t bus = bsp_i2c_bus();
            if (bus) {
                esp_err_t r = i2c_master_bus_reset(bus);
                ESP_LOGW(TAG, "自愈：先复位 I2C 总线 -> %s", esp_err_to_name(r));
            }
        }

        ESP_LOGW(TAG, "电量读取连续失败，开始自愈（init 前探针 soc=%d）", probe);

        // ★★ r57 候选 3②：init 返回 ESP_ERR_TIMEOUT 时**同一轮内立即重试**，
        //   不再干等 60 秒（芯片就在总线上，重试几乎不花代价）。
        //   ① 句柄为空（说明初始化曾失败）⇒ 这一句就是**完整重跑**：
        //      唤醒 -> profile 校验/重写 -> 等 SOC 就绪。
        //      ★「芯片被留在睡眠态」那一类故障正是在这一步被救回来 ——
        //        cw_update_profile() 会先把芯片写进睡眠（0x30→0xF0），之后
        //        任何一步失败都直接 return，**走不到 enter_active()**，
        //        芯片就停在那儿了。
        esp_err_t e = ESP_FAIL;
        for (int t = 1; t <= HEAL_INIT_RETRY; t++) {
            e = bsp_battery_init();
            if (e != ESP_ERR_TIMEOUT) break;
            ESP_LOGW(TAG, "自愈：init 超时（SOC 未就绪），第 %d/%d 次立即重试",
                     t, HEAL_INIT_RETRY);
            vTaskDelay(pdMS_TO_TICKS(HEAL_RETRY_GAP_MS));
        }

        // ★★ r57 候选 1：只有「init 前探到不应答」且「init 成功」才续期。
        //   少了这一句，窗口还是开机那一刻的 ⇒ 自愈后立刻拿到占位 0 并被
        //   当成 OK 报成「自愈成功 SOC=0%」；但**无条件**调就是原来的漏洞。
        if (probe < 0 && e == ESP_OK) note_reinit();

        int soc = -1, mv = -1;
        fa_batt_state_t st = fa_batt_read(&soc, &mv);

        // ★ 最后手段：句柄在、但 I2C 事务持续失败 ⇒ 复位总线再读一次。
        //   （r57 起这是**第二道** —— 第一道已经提到 init 之前了。）
        if (st == FA_BATT_FAIL) {
            i2c_master_bus_handle_t bus = bsp_i2c_bus();
            if (bus) {
                esp_err_t r = i2c_master_bus_reset(bus);
                ESP_LOGW(TAG, "自愈：复位 I2C 总线 -> %s", esp_err_to_name(r));
                st = fa_batt_read(&soc, &mv);
            }
        }

        if (st == FA_BATT_OK) {
            s_heal_fails = 0;
            s_unknown    = false;
            ESP_LOGW(TAG, "自愈成功（init=%s, SOC=%d%% mV=%d）",
                     esp_err_to_name(e), soc, mv);
            last_ms = 0;             // 好了就清零限频：下次真坏了能立刻救
        } else if (st == FA_BATT_WARMING) {
            // ★★ 重跑 init 会把芯片重启一次 ⇒ SOC 从 0 重新算。
            //   这不是失败，**绝不能**当成「自愈成功 SOC=0%」报上去，
            //   更不能立刻再救一次 —— 那会重启芯片 → 又 WARMING → 死循环。
            s_heal_fails = 0;        // WARMING 不是失败
            s_unknown    = false;
            ESP_LOGW(TAG, "自愈已跑完，芯片在重新计算电量（SOC 暂不可信）");
            // ★★ r57 候选 1：**只有真的续期过**才清限频。
            //   原来无条件清零，配合"空操作也续期"就成了一条无限循环的链子
            //   （诊断 §3 漏洞 1）：一次瞬时 NACK → 自愈 → WARMING → 又能立刻自愈。
            if (s_win_renewed) last_ms = 0;
        } else {
            s_heal_fails++;
            ESP_LOGE(TAG, "自愈未成功（init=%s, SOC 仍读不到）—— "
                          "若反复出现，多半是电量计芯片或 I2C 走线的硬件问题",
                     esp_err_to_name(e));
            // ★★ r57 候选 2：连续两轮都没救回来 ⇒ 认账"读不到"，
            //   界面把那一格显示成 "--%"（见 fa_batt_unknown 的说明）。
            if (s_heal_fails >= UNKNOWN_HEAL_FAILS && !s_unknown) {
                s_unknown = true;
                ESP_LOGE(TAG, "连续 %d 轮自愈未成功 ⇒ 那一格改显示 --%%（认账读不到）",
                         s_heal_fails);
            }
        }

        fa_batt_unlock();
    }
}

void fa_batt_guard_start(void)
{
    if (s_started) return;
    s_started = true;

    if (!s_lock) s_lock = xSemaphoreCreateMutex();
    if (!s_lock) {
        ESP_LOGE(TAG, "互斥量创建失败 —— 电量将不会自愈");
        return;
    }

    // 优先级 2：低于 LVGL 任务。栈 4096：bsp_battery_init 里要跑 ESP_LOG
    // 的格式化，别抠。创建失败不致命 —— 最坏就是退化回「数字消失不自愈」，
    // 也就是现在这个样子。
    if (xTaskCreate(guard_task, "fa_batt", 4096, NULL, 2, NULL) != pdPASS) {
        ESP_LOGE(TAG, "看护任务创建失败 —— 电量将不会自愈");
    }
}
