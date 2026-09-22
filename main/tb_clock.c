// main/tb_clock.c —— 时间来源：NVS 恢复 + WiFi/SNTP + 手动设时
//
// 设计取舍（重要）
// ----------------
// 这台机器没有 RTC 备份电池，也没有配网入口（官方主分支的 WiFi 只能扫 AP）。
// 所以时间做成「主路 + 兜底」：
//
//   * 主路：连 WiFi → SNTP 对时 → **立刻断开 WiFi**。
//     保持关联会让射频常开（哪怕 modem sleep 也要 ~1mA），对一个挂件是浪费。
//     对完之后内部 RTC 自己走，6 小时后再连一次重新校。
//   * 兜底：设置页手动设时。
//
// 两条路都会写 NVS，所以重启能立刻有可显示的时间，只是标为「上次校时」。
//
// 另一个坑：IDF 的 CONFIG_LWIP_SNTP_STARTUP_DELAY 默认是 y —— SNTP 会把首次
// 请求随机推迟 1~5 分钟。对开机就想起课的设备完全不可接受，已在
// sdkconfig.defaults 里关掉。

#include "tb_clock.h"
#include "secrets_wifi.h"

#include <string.h>
#include <sys/time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_sntp.h"
#include "esp_timer.h"
#include "nvs_flash.h"

static const char *TAG = "tb_clock";

#define NVS_NS      "tbc"
#define NVS_KEY_EP  "epoch"
#define TZ_STR      "CST-8"          // 中国标准时间 UTC+8

#define WIFI_TRY_TIMEOUT_MS  30000   // 等拿到 IP 的上限
#define SNTP_WAIT_TIMEOUT_MS 45000   // 等 SNTP 回包的上限
#define RESYNC_PERIOD_MS     (6 * 3600 * 1000)

static const char *NTP_SERVERS[] = {
    "ntp.aliyun.com",     // 国内，快
    "cn.pool.ntp.org",
    "pool.ntp.org",
};
#define NTP_SERVER_COUNT  (sizeof(NTP_SERVERS) / sizeof(NTP_SERVERS[0]))
_Static_assert(NTP_SERVER_COUNT <= CONFIG_LWIP_SNTP_MAX_SERVERS,
               "too many NTP servers; anything past CONFIG_LWIP_SNTP_MAX_SERVERS is dropped");

static volatile tb_clk_state_t s_state = TB_CLK_NONE;
static volatile bool s_wifi_ok  = false;
static volatile bool s_synced   = false;
static SemaphoreHandle_t s_ip_sem;
static bool s_sntp_started = false;
static bool s_want_conn    = false;
static esp_netif_t *s_netif;
static esp_timer_handle_t s_persist_timer;

static bool s_creds_ok(void)
{
    return strcmp(TB_WIFI_SSID, "PUT_YOUR_SSID_HERE") != 0 &&
           strlen(TB_WIFI_SSID) > 0;
}

bool tb_clock_wifi_enabled(void) { return s_creds_ok(); }

// ------------------------------------------------------------------ NVS

static void nvs_boot(void)
{
    esp_err_t e = nvs_flash_init();
    if (e == ESP_ERR_NVS_NO_FREE_PAGES || e == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        e = nvs_flash_init();
    }
    if (e != ESP_OK) ESP_LOGW(TAG, "NVS init failed (%s); clock will not persist", esp_err_to_name(e));
}

void tb_clock_persist(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    if (tv.tv_sec < 1600000000) return;         // 2020-09 之前的时间视为无效，不存

    nvs_handle_t h;
    if (nvs_open(NVS_NS, NVS_READWRITE, &h) != ESP_OK) return;
    nvs_set_i64(h, NVS_KEY_EP, (int64_t)tv.tv_sec);
    nvs_commit(h);
    nvs_close(h);
}

static bool nvs_restore(void)
{
    nvs_handle_t h;
    if (nvs_open(NVS_NS, NVS_READONLY, &h) != ESP_OK) return false;
    int64_t ep = 0;
    esp_err_t e = nvs_get_i64(h, NVS_KEY_EP, &ep);
    nvs_close(h);
    if (e != ESP_OK || ep < 1600000000) return false;

    struct timeval tv = { .tv_sec = (time_t)ep, .tv_usec = 0 };
    settimeofday(&tv, NULL);
    ESP_LOGI(TAG, "restored clock from NVS: %lld", (long long)ep);
    return true;
}

// ------------------------------------------------------------------ SNTP

static void sntp_sync_cb(struct timeval *tv)
{
    (void)tv;
    s_synced = true;
    s_state  = TB_CLK_SYNCED;
    tb_clock_persist();
    ESP_LOGI(TAG, "SNTP sync OK");
}

static void sntp_start_once(void)
{
    if (s_sntp_started) return;
    esp_sntp_setoperatingmode(ESP_SNTP_OPMODE_POLL);
    for (size_t i = 0; i < NTP_SERVER_COUNT; i++)
        esp_sntp_setservername((uint8_t)i, NTP_SERVERS[i]);
    esp_sntp_set_time_sync_notification_cb(sntp_sync_cb);
    esp_sntp_init();
    s_sntp_started = true;
    ESP_LOGI(TAG, "SNTP started, %d servers", (int)NTP_SERVER_COUNT);
}

// ------------------------------------------------------------------ WiFi 事件

static void on_wifi_event(void *arg, esp_event_base_t base, int32_t id, void *data)
{
    (void)arg; (void)base; (void)data;
    if (id == WIFI_EVENT_STA_START) {
        if (s_want_conn) esp_wifi_connect();
    } else if (id == WIFI_EVENT_STA_DISCONNECTED) {
        s_wifi_ok = false;
        if (s_want_conn) esp_wifi_connect();     // 连接阶段内自动重试
    }
}

static void on_ip_event(void *arg, esp_event_base_t base, int32_t id, void *data)
{
    (void)arg; (void)base; (void)data;
    if (id == IP_EVENT_STA_GOT_IP) {
        s_wifi_ok = true;
        if (s_ip_sem) xSemaphoreGive(s_ip_sem);
        ESP_LOGI(TAG, "got IP");
    }
}

// ------------------------------------------------------------------ 后台联网任务

static bool wait_ms(volatile bool *flag, int timeout_ms)
{
    for (int t = 0; t < timeout_ms; t += 100) {
        if (*flag) return true;
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    return *flag;
}

static void net_task(void *arg)
{
    (void)arg;

    wifi_config_t wc = { 0 };
    strlcpy((char *)wc.sta.ssid,     TB_WIFI_SSID, sizeof(wc.sta.ssid));
    strlcpy((char *)wc.sta.password, TB_WIFI_PASS, sizeof(wc.sta.password));
    wc.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    if (strlen(TB_WIFI_PASS) == 0) wc.sta.threshold.authmode = WIFI_AUTH_OPEN;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wc));
    esp_wifi_set_ps(WIFI_PS_MIN_MODEM);          // 省电：射频按 DTIM 周期打盹
    if (s_netif) esp_netif_set_hostname(s_netif, TB_WIFI_HOSTNAME);
    ESP_ERROR_CHECK(esp_wifi_start());

    while (1) {
        s_want_conn = true;
        s_synced    = false;
        xSemaphoreTake(s_ip_sem, 0);             // 清掉上一轮的信号
        esp_wifi_connect();

        if (wait_ms(&s_wifi_ok, WIFI_TRY_TIMEOUT_MS)) {
            s_state = TB_CLK_SYNCING;
            sntp_start_once();
            if (wait_ms(&s_synced, SNTP_WAIT_TIMEOUT_MS))
                ESP_LOGI(TAG, "sync done, WiFi disconnected to save power");
            else
                ESP_LOGW(TAG, "SNTP timeout, no sync this round");
        } else {
            ESP_LOGW(TAG, "WiFi not connected within %d s (SSID=%s)", WIFI_TRY_TIMEOUT_MS / 1000, TB_WIFI_SSID);
        }

        s_want_conn = false;
        esp_wifi_disconnect();                   // 对完就走，别让射频常开
        vTaskDelay(pdMS_TO_TICKS(RESYNC_PERIOD_MS));
    }
}

// ------------------------------------------------------------------ 初始化

void tb_clock_init(void)
{
    setenv("TZ", TZ_STR, 1);
    tzset();

    nvs_boot();
    if (nvs_restore()) s_state = TB_CLK_RESTORED;

    // 每 10 分钟把当前时间落一次盘，这样突然断电也不会退回到上一次对时
    const esp_timer_create_args_t pt = {
        .callback = (void (*)(void *))tb_clock_persist,
        .name = "persist",
    };
    if (esp_timer_create(&pt, &s_persist_timer) == ESP_OK)
        esp_timer_start_periodic(s_persist_timer, 10LL * 60 * 1000000);

    if (!s_creds_ok()) {
        ESP_LOGW(TAG, "no WiFi credentials in main/secrets_wifi.h; "
                      "skipping network, set the clock manually in settings");
        return;
    }

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    s_netif = esp_netif_create_default_wifi_sta();
    s_ip_sem = xSemaphoreCreateBinary();

    wifi_init_config_t ic = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&ic));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, on_wifi_event, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP, on_ip_event, NULL, NULL));

    xTaskCreate(net_task, "tb_net", 4096, NULL, 4, NULL);
}

// ------------------------------------------------------------------ 取值

bool tb_clock_valid(void) { return s_state != TB_CLK_NONE; }

tb_clk_state_t tb_clock_state(void)  { return s_state; }
bool           tb_clock_wifi_ok(void) { return s_wifi_ok; }

bool tb_clock_local(struct tm *out)
{
    if (s_state == TB_CLK_NONE) return false;
    time_t now = time(NULL);
    if (now < 1600000000) return false;
    struct tm tmp;
    if (!localtime_r(&now, &tmp)) return false;
    *out = tmp;
    return true;
}

void tb_clock_set_manual(int year, int mon, int day, int hour, int min)
{
    struct tm t = { 0 };
    t.tm_year = year - 1900;
    t.tm_mon  = mon - 1;
    t.tm_mday = day;
    t.tm_hour = hour;
    t.tm_min  = min;
    t.tm_isdst = -1;
    time_t ep = mktime(&t);                 // mktime 按本地时区解释，与 TZ 一致
    if (ep < 1600000000) return;
    struct timeval tv = { .tv_sec = ep, .tv_usec = 0 };
    settimeofday(&tv, NULL);
    s_state = TB_CLK_MANUAL;
    tb_clock_persist();
    ESP_LOGI(TAG, "manual time set: %04d-%02d-%02d %02d:%02d", year, mon, day, hour, min);
}

// ---------------------------------------------------------------------------
// 雷霆战机侧追加的两个查询接口
// ---------------------------------------------------------------------------
int tb_clock_seconds(void)
{
    struct tm t;
    if (!tb_clock_local(&t)) return -1;
    return t.tm_hour * 3600 + t.tm_min * 60 + t.tm_sec;
}

bool tb_clock_hm(int *hour, int *min)
{
    struct tm t;
    if (!tb_clock_local(&t)) return false;
    if (hour) *hour = t.tm_hour;
    if (min)  *min  = t.tm_min;
    return true;
}
