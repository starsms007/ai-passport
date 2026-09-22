// main/main.c —— 《去远方》固件入口（FoloToy AI Passport / ESP32-C3）
//
// 主循环每 50ms 做三件事：
//   1. 读三键（ADC 分压）→ 折成 press / release / long 三种事件
//   2. fa_poll() 推进玩法状态机（到点了没有）
//   3. 进 LVGL 锁 → 派发按键事件 + fa_view_tick() 刷界面
//
// ★ 按键事件为什么要先排队、再进锁派发：
//   事件处理里会弹提示气泡（碰 LVGL 对象），而按键轮询跑在 main 任务里。
//   LVGL 不是线程安全的 —— 直接在轮询处调界面函数会随机踩坏 LVGL 内部状态，
//   表现是刷着刷着花屏或者直接重启，且极难复现。所以收集与派发分成两步。
//
// 按键语义（2026-09-19 第五次改版）
//   这一次是**减法**：布置小院连同 13 件道具整个下线，收集册只剩「明信片 + 家」。
//
//   开场      ● 单击  翻页（共 3 页，最后一页进游戏）
//   在家      ↑/↓ 单击 移动选择器（试玩 / 5分 / 1时 / 4时 / 12时 / 收集）
//             ● 单击  选中时长 -> 出发；选中「收集」-> 开收集册
//             ● 长按  **关屏**。关的只是背光，CPU 和系统时钟一秒都没睡
//                     ⇒ **倒计时照走**（用户口径「要是关闭屏幕，还是正常计时」）。
//                     唤醒 = 任意键，且那一下只负责亮屏、不会触发按键本身的功能
//                     （否则关着屏按一下 ● 会把猫直接送出门）。见 keys_poll 开头。
//             ↑ 长按  看校时状态        ↓ 长按  屏幕翻转 180°（刷完机若画面是倒的，按这个）
//                     ★ 本机不联网、不校时，所以长按 ↑ 在未校时的时候**不弹任何话**
//                       （用户口径「不用联网，不用报错」，见 toast_clock_state）。
//   旅行中    ● 长按  花三叶草快速返回（试玩档免费，其余按剩余时长 1~5 棵）
//                     ★ 这趟**照给明信片和纪念品，但不给三叶草** ——
//                       花掉的那几棵就是这张明信片的价钱（用户口径：
//                       「相当于用三叶草买的道具」）。所以余额的净变化 = −报价。
//                     短按 ● 只报价钱、不召回（防手滑）
//   归来      ● 单击  收下
//             ★ 本档明信片全拿到之后再出行，收获卡**照样把那张图显示出来**
//               （随机一张本档已有的，见 fa_game.c 的 pick_owned_in_slot），
//               只是按钮那行改成「● 收下　这张已经有了」。
//   收集册    ↑/↓ 单击 翻上一条 / 下一条（「家」→ 各张明信片，翻到头绕回）
//             ● 单击  出册（全册只有这一种页面）
//             ● 长按  **把这一条设成主页的风景**：
//                     翻到「家」= 回家（小猫恢复动态）；翻到明信片 = 挂上它
//                     ★ 已经是当前风景时不折腾，只弹一句、留在册里继续翻
//             序列：**「家」永远排第一**，后面跟着已经到手的明信片。
//             家被当成「第一张明信片」—— 用户原话：
//             「收藏里面就是明信片和家，把家当成一种一张明信片」。
//
// ★ 猫在家会自己溜达、坐下歇着（fa_view.c 的 SEQ_STROLL/SEQ_SIT）。
//   那两个状态**不算「忙」** —— fa_view_busy() 只看出发/归来那四个序列。
//   把溜达也算进去的后果是：猫一走开，手柄三个键全被吞掉，界面看着却一切正常。
// ★ 猫还有一个不上屏的理由：风景不是「家」时猫根本不露面（fa_view.c 的 cat_sync）。
//   那时溜达逻辑照跑（省一个分支），但玩家看不见 —— 这是有意的，不是没画出来。
//   ★ 第五次把「小屋 + 13 件道具」删掉之后，这条判据下**只剩猫**了。
//
// 已下线（别再加回来）：
//   · 昼夜系统（apply_night + 14 个 N_* 夜色常量）—— 旅行中改成压一层遮罩；
//   · **「布置小院」与 13 件道具**（第五次）—— 模式、收集册入口、NVS 键 "yard"、
//     fa_prop_* / fa_yard_* / fa_view_yard_* 一起下线；素材侧 house 与那 13 件
//     scene-prop 进了 tools/gen_fa_sprites.py 的 OFFLINE，图集里已经没有它们。
//     ★ 恢复要点：得先把那 14 个 id 从 OFFLINE 里挪走再重跑图集，光抄代码没用。
//   · 收藏册里的「纪念品合集页」与那五块程序化兜底小风景（第五次）——
//     纪念品**照常发**（收获卡那行「带回桂花糕」还在，fa_souv_name 仍在用），
//     只是不再有展示页；souvenir_a 那 12 帧素材没删，将来要加回来还拿得到。
//   · 顶栏那条不透明状态条（第五次）—— 三叶草 / 电量 / 校时点直接压在底图天空上，
//     旅行遮罩压上去时自动换暖白字（见 fa_view.c 的 hud_paint）。
//   · 「更换风景」选择页（s_scene_pick / scene_pick_build 整套）—— 换到明信片页的长按上；
//   · 纪念品的逐件浏览 —— 曾改成一页合集，第五次连那一页也删了（见上）；
//   · 屏幕亮度调整格（连同上上版的音量格）—— 收集册现在没有任何设置项；
//   · 「花三叶草指定下一站」—— 整套 fa_target_* / DEX_ENTRY_GO 已删；
//   · 状态栏的 hh:mm 时钟（校时本身还在，fa_game 靠它算行程）；
//   · 背景音乐（fa_music / audio 分区）。
//
// ★ 猫在家会自己溜达、坐下歇着（fa_view.c 的 SEQ_STROLL/SEQ_SIT）。
//   那两个状态**不算「忙」** —— fa_view_busy() 只看出发/归来那四个序列。
//   把溜达也算进去的后果是：猫一走开，手柄三个键全被吞掉，界面看着却一切正常。
// ★ 猫还有一个不上屏的理由：风景不是「家」时猫根本不露面（fa_view.c 的 cat_sync）。
//   那时溜达逻辑照跑（省一个分支），但玩家看不见 —— 这是有意的，不是没画出来。

#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "nvs_flash.h"

#include "bsp_battery.h"
#include "bsp_button.h"
#include "bsp_display.h"

#include "fa_game.h"
#include "fa_view.h"
#include "tb_clock.h"
#include "tb_prefs.h"
// ★ 2026-09-19：fa_music.h 已下线（背景音乐整条线去掉，用户反馈「不好听」）。

static const char *TAG = "faraway";

// 动态提示文案要落在静态缓冲里 —— fa_view_msg() 不拷贝字符串。
// ★ 给 64 而不是 48：最长的两条是「长按 ● 花 %d 个猫罐头回来」和
//   「花 %d 个猫罐头回来了」，编译器按 %d 最坏 11 字符算下来约 46 字节，
//   IDF 又把 -Wformat-truncation 当错误，48 会直接编不过。
static char s_msg_buf[64];

#define FRAME_MS     50          // 20fps
#define KEY_LONG_MS  550

// 按键索引，与 bsp_pins.h 的 BSP_BTN_MV_TABLE 顺序一致
#define K_UP   0
#define K_DOWN 1
#define K_OK   2
#define K_NONE (-1)

// ---------------------------------------------------------------------------
// 按键：ADC 轮询 + 自建状态机
//
// 为什么不用 BSP 的 iot_button 回调：它给的是 PRESS/CLICK/DOUBLE/LONG，
// 没有「松开」。这里要区分「单击」和「长按之后抬手」，所以自己读分压。
// bsp_button_init(NULL, NULL) 仍要调 —— ADC 单元与校准句柄是它建的。
// ---------------------------------------------------------------------------

static int   s_key = K_NONE;
static float s_hold_ms = 0.0f;
static bool  s_long_fired = false;

// 事件队列（只被 main 任务读写，不需要锁）
enum { EV_PRESS = 0, EV_RELEASE, EV_LONG };
typedef struct { int k; int kind; bool was_long; } key_ev_t;
static key_ev_t s_q[8];
static int      s_qn;

// ---------------------------------------------------------------------------
// 关屏 / 亮屏
//
// ★★★ 2026-09-19（第十四轮）用户原话：「要是关闭屏幕，还是正常计时。」
//   这台机器在这一轮之前**根本没有「关屏」这个动作** —— 背光下限只给到 10%
//   （见 tb_prefs.c 的 BR_MIN，理由是「关到 0 等于把人锁在门外」）。
//   用户这句话里其实藏着一个诉求：**要能关屏，而且关屏不能停表**。
//   所以这里把「关屏」补上（入口见 on_long 的长按 ●），
//   并把「为什么它一定不影响计时」写在下面，免得以后有人好心改成 light_sleep。
//
// ★ 为什么关屏一定不影响计时：关掉的只是**背光这一路**，CPU 一秒都没睡 ——
//   `fa_now()` 取的 `esp_timer_get_time()` 与 `time()` 照常走。
//   换句话说，在这台机器上「屏幕」和「计时」本来就是两件互不相干的事。
// ★ 为什么不顺手做 light_sleep 去省那几十毫安：那会把 esp_timer 的连续性
//   和 WiFi 唤醒源一起搅进来 —— 为了省电去动计时的地基，而用户要的恰恰是
//   「关屏也要正常计时」。不值当。
// ★ 唤醒 = **任意键**，且按下的那一整次按压全部作废（见 keys_poll 开头）：
//   否则关屏状态下按一下 ● 会把猫直接送出门。
// ---------------------------------------------------------------------------

static bool s_screen_off;
static int  s_swallow = K_NONE;    // 亮屏那一下要作废的键，吞到它抬起为止

static void screen_off_set(bool off)
{
    s_screen_off = off;
    // ★★★ 2026-09-21（第四十四轮，实机反馈）：**关屏那一按要一起吞掉。**
    //   用户原话：「旅行页面的上下键，无法正常关闭屏幕，点击之后，屏幕只会闪一下。
    //             主界面长按 ok 键也是这样，无法关闭屏幕。」
    //   根因（两条入口是同一个病）：
    //     关屏这一下是**由按着的键**触发的 ——
    //       主界面：长按 ● 满 550ms 发 EV_LONG，此刻手指还在键上；
    //       旅行页：EV_PRESS（按下即关），手指当然还在键上。
    //     而 keys_poll 开头的关屏拦截写的是「now != K_NONE 就亮屏」——
    //     ⇒ **下一帧**（约 33ms 后）它看到手指还压着，当成「任意键唤醒」，
    //       立刻把背光打回原值。表现就是**屏幕闪一下**，然后什么都没发生。
    //   ⇒ 关屏的同时把这一按记进 s_swallow（它本来就是「这一按要作废的键」，
    //     语义完全对得上），拦截段就会一直等它**抬起**，才恢复「任意键唤醒」。
    //   ⚠ 只在 off==true 时写：亮屏那条路自己会设 s_swallow（见 keys_poll），
    //     这里再多写一次会把它的值覆盖成上一次的旧键。
    if (off) s_swallow = s_key;
    // ⚠ 直接调 bsp，**绕开 tb_bright_set 的 10% 下限**：
    //   那个下限是给「亮度设置」定的（调太暗会看不见自己调到了几，
    //   只能靠重刷一次固件救回来）；关屏是另一回事 —— 它有一个写在按键
    //   语义里的、明确的唤醒方式（任意键），不存在「把自己锁在门外」。
    //   所以这里用 tb_bright() 把用户设定过的亮度取回来，不做钳位。
    bsp_display_backlight(off ? 0 : (uint8_t)tb_bright());
}

static void q_push(int k, int kind, bool was_long)
{
    if (s_qn >= (int)(sizeof(s_q) / sizeof(s_q[0]))) return;
    s_q[s_qn].k        = k;
    s_q[s_qn].kind     = kind;
    s_q[s_qn].was_long = was_long;
    s_qn++;
}

// 读当前按的是哪个键。窗口与 bsp_pins.h 的 BSP_BTN_MV_TABLE 一致：
//   上 < 150mV    下 150..447mV    确定 447..1900mV    >= 1900mV 视为松开
static int key_read(void)
{
    int mv = bsp_button_read_mv();
    if (mv < 0)      return K_NONE;
    if (mv < 150)    return K_UP;
    if (mv < 447)    return K_DOWN;
    if (mv < 1900)   return K_OK;
    return K_NONE;
}

static void keys_poll(float dt_ms)
{
    int now = key_read();

    // ---- 关屏拦截（2026-09-19 第十四轮）----
    // ★ 关屏时，**任意键都只负责亮屏**：这一整次按压（press / long / release）
    //   一个事件都不产生 —— 否则关着屏按一下 ●，猫就被送出门了。
    // ★ `s_swallow` 记的是「这一按要作废的键」，一直吞到它**抬起**为止。
    //   抬起时把 s_key 归回 K_NONE 是必须的：下面那段「换键」逻辑靠
    //   `now != s_key` 判断，s_key 留着旧值的话，下一次真按键会先补一个假的
    //   EV_RELEASE 出去。
    if (s_screen_off) {
        // ★★★ 2026-09-21（第四十四轮，实机反馈）：**先看「关屏那一按」抬起没有。**
        //   这一小节必须排在下面「任意键唤醒」**之前**，否则完全失效。
        //   为什么需要它：关屏是由**按着的键**触发的（旅行页按下 ↑/↓、
        //   主界面长按 ● 满 550ms），手指在关屏那一刻**还压着**。
        //   没有这一小节的话，紧接着的下一帧就会走进下面那段
        //   「now != K_NONE ⇒ 亮屏」，把刚关掉的屏幕重新点亮 ——
        //   用户看到的正是「点击之后，屏幕只会闪一下」。
        //   s_swallow 在 screen_off_set(true) 里被设成那一刻的 s_key，
        //   所以这里只要等它归 K_NONE（手指抬起）就算这一次按压结束。
        //   ⚠ s_key 必须一起清：下面「换键」逻辑靠 `now != s_key` 判断，
        //     留着旧值的话，下一次真按键会先补一个假的 EV_RELEASE 出去。
        if (s_swallow != K_NONE) {
            if (now == K_NONE) {        // 抬起了：解除吞键，重新允许「任意键唤醒」
                s_swallow = K_NONE;
                s_key     = K_NONE;
            }
            return;                     // 还没抬起就继续等，绝不亮屏
        }
        if (now != K_NONE) {
            screen_off_set(false);      // 亮屏
            s_swallow    = now;         // 这一按作废
            s_key        = now;
            s_hold_ms    = 0.0f;
            s_long_fired = false;
        }
        return;
    }
    if (s_swallow != K_NONE) {
        if (now == K_NONE) {            // 抬起了，解除吞键
            s_swallow = K_NONE;
            s_key     = K_NONE;
        }
        return;                         // 还没抬起就继续吞
    }

    if (now != s_key) {
        int  prev     = s_key;
        bool was_long = s_long_fired;   // ★ 必须在清零之前存下来，
        s_key         = now;            //   否则长按抬手会顺带触发一次点按
        s_hold_ms     = 0.0f;
        s_long_fired  = false;
        if (prev != K_NONE) q_push(prev, EV_RELEASE, was_long);
        if (now  != K_NONE) q_push(now,  EV_PRESS,   false);
        return;
    }

    if (now != K_NONE) {
        s_hold_ms += dt_ms;
        if (!s_long_fired && s_hold_ms >= (float)KEY_LONG_MS) {
            s_long_fired = true;
            q_push(now, EV_LONG, false);
        }
    }
}

// ---------------------------------------------------------------------------
// 事件派发（必须在 LVGL 锁内调用）
// ---------------------------------------------------------------------------

static void toast_clock_state(void)
{
    switch (tb_clock_state()) {
    case TB_CLK_SYNCED:   fa_view_msg("已校时");   break;
    case TB_CLK_SYNCING:  fa_view_msg("正在校时"); break;
    case TB_CLK_RESTORED: fa_view_msg("上次校时"); break;
    case TB_CLK_MANUAL:   fa_view_msg("手动设时"); break;
    // ★★★ 2026-09-19（第十四轮）**未校时不再弹任何提示。**
    //   用户原话（回答第十三轮抛出的「时钟怎么校」那个三选一）：
    //   「不用联网，不用报错。」
    //   ⇒ 这台机器就是**不联网、不校时**地跑：行程倒计时一律走「开机秒数」
    //     这个基准（见 fa_game.c 的 fa_now），功能上完全自洽 ——
    //     没有任何东西是坏的，也没有任何东西需要用户去处理。
    //   ⇒ 那就不该在屏幕上说「未校时」：它读起来像一条故障提示，而实际上
    //     这是**这台机器的正常工作方式**。所以这一支什么都不做。
    //   ⚠ 「长按 ↑ 看校时状态」这个入口**保留**（真校过时，上面四支照旧弹）；
    //     只是没校时的时候按下去不再有反应 —— 在这里「什么都没发生」
    //     才是正确的表达，比一句听起来像报错的话好。
    //   ⚠ 别顺手把上面四支也删了：一旦哪天填了 WiFi 凭据，它们就是唯一的回显。
    default:              break;
    }
}

static void on_press(int k)
{
    // ★★ 小游戏**必须排在最前面**：它是盖在收藏册上面的一层，
    //   而下面那句 `if (fa_view_dex_open()) return;` 会把所有按键一起吞掉 ——
    //   册子确实还开着（我们没关它，只是用游戏那一层盖住），所以顺序反了就完全没法玩。
    //   ↑ = 往左、↓ = 往右：板子上没有左右键，理由见 fa_view.h 那段。
    if (fa_view_mini_open()) {
        if      (k == K_UP)   fa_view_mini_hold(-1, true);
        else if (k == K_DOWN) fa_view_mini_hold(+1, true);
        return;                     // ● 按下不做任何事（短按的意义在抬手那一刻）
    }

    // ★★ 2026-09-19（第十六轮）：选择页也要排在那句 `fa_view_dex_open()` 之前 ——
    //   和上面小游戏同一条理由（它是盖在册子上面的一层，册子确实还开着）。
    //   ↑↓ 按下即移游标：换选项没有破坏性，不必等抬手（和主页选择器同一条纪律）。
    //   ⚠ 上面那一支（mini）和这一支**互斥**：进游戏前 menu_close() 已经把它收了，
    //     所以两者不会同时开着，先后顺序只是为了不让人误以为可以省。
    if (fa_view_menu_open()) {
        if      (k == K_UP)   fa_view_menu_move(-1);
        else if (k == K_DOWN) fa_view_menu_move(+1);
        return;                     // ● 按下不做任何事（同小游戏：意义在抬手）
    }

    // ★★ 2026-09-21（第四十五轮）：设置页同样排在那句 `fa_view_dex_open()` 之前。
    //   它和上面两支是**不同的一条链**（册子 → 设置页），不会与选择页同时开着，
    //   但「盖在册子上面的层必须先判」这条纪律一样适用。
    //   ↑ = 亮一格 / ↓ = 暗一格：按下即调（改亮度没有破坏性，和主页选择器同一条纪律）。
    //   ⚠ 别把 ● 也接到这儿 —— 短按的意义在抬手（on_release），
    //     按下就返回会变成「按下去就退」，长按便再也无从分辨。
    if (fa_view_set_open()) {
        if      (k == K_UP)   fa_view_set_move(+1);
        else if (k == K_DOWN) fa_view_set_move(-1);
        return;
    }

    if (fa_view_intro_active()) return;     // 开场期间只有 ● 短按有效，走 on_release
    if (fa_view_busy()) return;             // 出发/归来动画播放中，吞掉一切
    if (fa_view_dex_open()) return;
    if (fa_view_card_visible()) return;

    if (g_fa.state == FA_HOME) {
        // ↑/↓ 移动选择器。按下即生效 —— 换选项没有破坏性，不需要等抬手。
        if (k == K_UP)        fa_view_sel_move(-1);
        else if (k == K_DOWN) fa_view_sel_move(+1);
    } else if (g_fa.state == FA_AWAY) {
        // ★★ 2026-09-20（第二十轮）：旅行界面 **按 ↑ 或 ↓ 即关屏**，长按短按都算。
        //   用户原话：「旅行界面关屏 = 按上/下键，不管长按还是短按，上下键不需要猫。」
        //   ⇒ 按下这一下就关，不必等 550ms 的长按 —— 关屏是「这会儿不想看了」，
        //     这时候还要人多按半秒没有任何道理。
        //   ⇒ 「上下键不需要猫」是说**别为了给猫让路而保留 ↑↓**：
        //     旅行界面里猫是自己走的，按键从来就没管过它，这里可以放心占。
        //   ⚠ 别再加一个 on_long 分支：长按会先走 on_press（这一下已经关屏了），
        //     再走 on_long —— 而 on_long 的 FA_AWAY 那支只管 ●，↑↓ 落下去
        //     会被那句 `if (g_fa.state != FA_HOME) return;` 挡掉。重复写反而容易失配。
        //   ⚠ 关屏后**再按 ↑↓ 是亮屏**，不是又关一次 —— 关屏拦截在 keys_poll 开头，
        //     那一整次按压会被 s_swallow 作废（见那段注释），到不了这里。
        if (k == K_UP || k == K_DOWN) screen_off_set(true);
    }
}

static void on_release(int k, bool was_long)
{
    // ★★ 小游戏必须处理在下一句 `if (was_long) return;` **之前** —— 这是一个真陷阱：
    //   长按 ↑ 或 ↓ 会先触发一次 EV_LONG（550ms），抬手时 was_long 为真；
    //   要是走常规流程在这里直接 return，「松开方向键」这件事就丢了，
    //   fa_view_mini_hold() 里的 s_mini_dir 永远清不掉，
    //   猫会朝那个方向**一路走到局终**，而且玩家怎么松手都没用。
    //   （↑↓ 的长按本身没有别的含义，所以这里只管把方向收回来。）
    if (fa_view_mini_open()) {
        if      (k == K_UP)   fa_view_mini_hold(-1, false);
        else if (k == K_DOWN) fa_view_mini_hold(+1, false);
        else if (k == K_OK && !was_long) fa_view_mini_ok();   // 结算面板亮着时 = 回收藏册
        return;
    }

    // ★★ 2026-09-21（第四十五轮）：设置页的抬手事件。
    //   ① 放在 `if (was_long) return;` **之前** —— 和小游戏那一支同一个理由：
    //      ● 长按会先触发 EV_LONG（550ms），抬手时 was_long 为真，
    //      走常规流程就会被那一句挡掉，**长按 ● 在设置页会变成死键**
    //      （而 on_long 那边我们故意不处理它，免得一个动作两处写）。
    //      ⇒ 这一支把「短按/长按都算退回去」揽下来，两个键就都活了。
    //   ② 必须排在册子那一支**之前**：册子还在底下开着，
    //      顺序反了 ● 会漏到 `fa_view_dex_ok()` 去 —— 那一下会**出册**，
    //      而人只是想从设置页退回册子（表现：按一下直接到主页，回不去了）。
    //   ⚠ ↑↓ 的抬手在这里是**空转**（亮度在按下那一刻就调完了），
    //     所以不判方向，直接 return —— 和选择页那一支的写法一致。
    if (fa_view_set_open()) {
        if (k == K_OK) fa_view_set_close();
        return;
    }

    if (was_long) return;                   // 长按已经处理过了，抬手不再触发点按

    if (fa_view_intro_active()) {
        if (k == K_OK) fa_view_intro_next();
        return;
    }

    if (fa_view_busy()) return;

    // ★★ 2026-09-19（第十六轮）：选择页的抬手事件。
    //   放在 `if (was_long) return;` **之后**（和小游戏那一支不同）——
    //   理由：选择页上 ↑↓ 的长按没有任何含义（方向在按下那一刻就已经用掉了），
    //   所以长按抬手直接丢掉是正确的；而 ● 长按也不该被当成「进入」
    //   （下面 on_long 里明确不处理，正好被这一句挡住）。
    //   ⚠ 必须排在册子那一支**之前**：册子还开着（被盖住），
    //     顺序反了 ● 会漏到 `fa_view_dex_ok()` 去 —— 那一下会把册子关掉、
    //     同时选择页还在，变成两个页面打架。
    if (fa_view_menu_open()) {
        if (k == K_OK) fa_view_menu_ok();
        return;
    }

    // ★ 2026-09-19（第五次）：这里原来有一段「布置小院」的分支（● 换道具 /
    //   ↑↓ 换位置），整块删掉了 —— 那个模式和收集册互斥，所以当年要判在册子之前。
    //   现在册子前面只剩「开场」和「忙」两个判断。

    if (fa_view_dex_open()) {
        // ★ 返回改成 ● 短按。用户明确说不喜欢长按返回。
        // ↑ / ↓ 从「切页」改成「翻上一条 / 下一条」—— 一次只显示一件。
        // ★ 第四次把 ● 短按改成「按当前页分派」，第五次**又改回不需要分派**：
        //   册里只剩一种页面（家 / 明信片共用同一套白卡），
        //   所以 fa_view_dex_ok 就是一句话「出册」。分派仍然留在界面层，
        //   这里只转发 —— 将来再加页面时不用动 main.c。
        if (k == K_OK)        fa_view_dex_ok();
        else if (k == K_UP)   fa_view_dex_move(-1);
        else if (k == K_DOWN) fa_view_dex_move(+1);
        return;
    }

    if (fa_view_card_visible()) {
        if (k == K_OK) fa_view_card_claim();
        return;
    }

    if (g_fa.state == FA_HOME && k == K_OK) {
        // 选择器停在「收集」上就开册，否则出发。
        if (fa_view_sel_is_dex()) fa_view_dex_show();
        else                      fa_view_start_depart();
        return;
    }

    if (g_fa.state == FA_AWAY && k == K_OK) {
        // ★ 2026-09-19（第九轮）：短按 ● 从「报价钱」改成**进收藏册**。
        //   用户原话：「短按进入收藏，此时收藏里面设置一个回到旅行界面」。
        //   报价钱这件事已经常驻写在旅行面板上了（每帧刷新的那两行），
        //   短按再说一遍等于什么都没干，还白占掉唯一一颗确定键；
        //   而现在这一次按键有了去处 —— 册子第一条就是「旅行中」，按 ● 就回来。
        fa_view_dex_show();
        return;
    }
}

static void on_long(int k)
{
    // ★ 小游戏：● 长按 = 不想玩了，**按当前分数结算**（奖励照给，不是「放弃」）。
    //   15 秒不长，但不给一条早退的路，人被困在里面就只能干等。
    //   ⚠ ↑↓ 的长按在这里**故意不处理**：那只是「按得久了一点」，
    //     方向由 on_release 收尾（也在小游戏那一支里）。
    if (fa_view_mini_open()) {
        if (k == K_OK) fa_view_mini_end_now();
        return;
    }

    if (fa_view_intro_active()) return;
    if (fa_view_busy()) return;

    // ★★ 2026-09-19（第十六轮）：选择页开着时长按**要在这里挡住**，
    //   不能漏到下面 `fa_view_dex_ok_long()` 那一支 ——
    //   册子还在底下开着（我们只是盖住它），漏下去会在人看着选择页的时候
    //   把**主页背景**给换了（静默、还退不回来）。
    //   ★★★ 第十七轮：这一支从「什么都不做」改成 **● 长按 = 退回收藏册**。
    //     用户原话：「小游戏主界面要写"长按返回"。」
    //     这一页的 ● 短按已经被「开始游戏」占满，返回只剩长按这一条路。
    //   ⚠ ↑↓ 的长按这里**故意不处理**（方向在按下那一刻就用掉了），
    //     抬手由 on_release 收尾 —— 和上面小游戏那一支同一个道理。
    //   ⚠ `return` 无论如何都要执行（挡住下面那些支），别写成只在 K_OK 时 return。
    if (fa_view_menu_open()) {
        if (k == K_OK) fa_view_menu_back();
        return;
    }

    // ★★ 2026-09-21（第四十五轮）：设置页开着时长按**必须在这里挡住**，
    //   和上面选择页那一支是同一条理由 —— 册子还在底下开着（我们只是盖住它），
    //   漏下去会在人看着设置页的时候把**主页背景**给换了（静默、还退不回来）。
    //   ★ 为什么这里**什么都不做**（而不是像选择页那样接一个「退回」）：
    //     设置页的 ● 在 on_release 那一支里已经**长短按都算退回**了
    //     （见那边 ① 的说明）。两处都写就是同一个动作两个真相，
    //     将来改一处忘一处 —— 表现是「长按退两次」或「短按不退」，都不报错。
    //   ⚠ `return` 无论如何都要执行（挡住下面那些支），别写成只在 K_OK 时 return。
    if (fa_view_set_open()) return;

    // ★ 2026-09-19（第五次）：这里原来有一段「布置模式：长按 ● = 直接收工」，
    //   整块删掉了（模式本身已下线）。

    if (fa_view_dex_open()) {
        // ★ 2026-09-19（第四次）册内的长按不再是「什么都不做」——
        //   长按 ● = **把这一条换成主页的风景**（用户原话：
        //   「短按返回，长按更换为主页风景」）。
        // ★ 第五次扩到全册：翻到「家」长按就是回家（fa_scene_clear），
        //   翻到某张明信片长按就是挂上它。分派整个留在界面层的
        //   fa_view_dex_ok_long 里，这里只转发，和短按那条一个写法。
        if (k == K_OK)        fa_view_dex_ok_long();
        return;
    }

    if (fa_view_card_visible()) {
        // ★ 2026-09-19（第九轮）：收获卡上的长按**不再是「无效」** ——
        //   ● 长按 = 收下这张，并立刻把它设成主页背景。
        //   用户原话：「要加入长按直接设为背景。可以让收到之后立马设置为背景。」
        //   分派留在界面层（fa_view_card_claim_bg），这里只转发 ——
        //   和短按那条、册内那条一个写法。
        if (k == K_OK) fa_view_card_claim_bg();
        return;
    }

    if (g_fa.state == FA_AWAY && k == K_OK) {
        // ★ 2026-09-19：长按 ● 不再是「免费提前召回」，改成花三叶草的**快速返回**。
        //   原来的免费召回没有任何代价，三叶草对玩家的决策毫无影响，
        //   实机反馈「三叶草有什么用」有一半是这里来的。
        //   三叶草不够时必须说清差多少 —— 一句「没反应」会让人以为按键坏了。
        int cost = fa_recall_cost();
        if (cost < 0) return;
        if (cost == 0) {
            fa_recall_paid();
            fa_view_msg("回来了（这一档免费）");
        } else if (fa_recall_paid()) {
            snprintf(s_msg_buf, sizeof(s_msg_buf), "花 %d 个猫罐头回来了", cost);
            fa_view_msg(s_msg_buf);
        } else {
            snprintf(s_msg_buf, sizeof(s_msg_buf), "猫罐头不够，要 %d 个", cost);
            fa_view_msg(s_msg_buf);
        }
        return;
    }

    if (g_fa.state != FA_HOME) return;

    // ★★★ 2026-09-19（第十四轮）长按 ● = **关屏**（唤醒是任意键，见 keys_poll）。
    //   为什么放在这个键上：① 长按 ● 在「在家」这一支本来就是空的 ——
    //   原来只有长按 ↑ 看校时、长按 ↓ 翻屏，● 是唯一闲着的；
    //   ② 关屏是**不需要看屏幕**就能完成的动作，交给最主要、最好按的那个键最顺。
    //   ⚠ 只有「关」这一个方向在这里 —— 亮屏走的是 keys_poll 开头的拦截，
    //     根本到不了 on_long。所以关屏状态下不存在「长按 ● 又把它打开」这条路径。
    //   ⚠ 别把它改成「长按 ● 进收集册」：收集册已经是选择器里的一格，
    //     再留一个长按入口只会让人分不清长按 ● 到底是出发还是开册。
    if (k == K_OK) {
        screen_off_set(true);
        return;
    }

    if (k == K_UP) {
        toast_clock_state();
    } else if (k == K_DOWN) {
        // ★ 刷完机第一次上电，如果画面是倒的，长按 ↓ 就能翻过来（存进 NVS）。
        //   这是「编译期验不了真机朝向」的兜底 —— 总比让人重新刷一遍强。
        int d = (tb_screen_deg() == 180) ? 0 : 180;
        tb_screen_deg_set(d);
        fa_view_msg(d == 180 ? "屏幕转 180°" : "屏幕转 0°");
    }
}

static void keys_dispatch(void)
{
    for (int i = 0; i < s_qn; i++) {
        key_ev_t *e = &s_q[i];
        switch (e->kind) {
        case EV_PRESS:   on_press(e->k);                  break;
        case EV_RELEASE: on_release(e->k, e->was_long);   break;
        case EV_LONG:    on_long(e->k);                   break;
        default: break;
        }
    }
    s_qn = 0;
}

// ---------------------------------------------------------------------------
// 启动自检
//
// 真机上跑不了单元测试，就把校验搬进固件：用「立刻到期」的假行程把玩法跑 40 轮，
// 确认状态机确实会结算、明信片池确实会被填满、集满之后确实会折成三叶草。
// 这一步的价值在于：逻辑被改坏时，界面看着完全正常，只是永远收集不到明信片。
//
// ★ 快照用 static（虽然 fa_save_t 只有几十字节，但栈上放结构体快照是老毛病了）。
// ★ 自检过程中 fa_finish() 会写 NVS —— 跑完必须把快照落回去，否则真存档被假数据覆盖。
// ---------------------------------------------------------------------------
static void self_check(void)
{
    static fa_save_t snap;
    snap = g_fa;

    uint8_t  b = 0;
    uint32_t now = fa_now(&b);

    int settled = 0, newcards = 0, recall_ok = 0;

    // ★ 每档「第一趟」实得几棵。初值 0xFFFF 是「还没跑到这一档」的哨兵 ——
    //   用 0 当哨兵不行，试玩档本来就该是 0。
    //   记第一趟而不是任意一趟：本档明信片集满后会折价 +2，后面几趟的数字会飘。
    //   ★ 第五次起「集满」那一条还多带一件事：这一趟会抽到一张**已有的**明信片
    //     （bit2），所以从第二趟起 got_dest 也会被写上 —— 不影响那 +2，只是别误会。
    static uint16_t first_gain[FA_SLOT_COUNT];
    for (int i = 0; i < FA_SLOT_COUNT; i++) first_gain[i] = 0xFFFF;

    // ★ 2026-09-19（第五次）新加的一位：本档全拿到之后抽到的**重复**明信片。
    //   它和 newcards 共用 bit0（都是「有画面可显示」），靠 bit2 区分。
    //   自检跑完如果 dup > 0，说明第五次那条新分支真的走通了 ——
    //   这比「代码里看着对」可靠，因为它跑的是真随机、真结算。
    int dupcards = 0;

    for (int i = 0; i < 40; i++) {
        const int slot = i % FA_SLOT_COUNT;
        g_fa.state  = FA_AWAY;
        g_fa.slot   = (uint8_t)slot;
        g_fa.tbase  = b;
        g_fa.depart = now;
        g_fa.arrive = now - 1;              // 已经过期 —— fa_poll 应当立刻结算

        if (fa_poll()) {
            settled++;
            // ⚠ 顺序不能调：先数 dup 再数 newcards 是对的，但两个都必须在
            //   fa_claim() 之前读 —— claim 会把 state 推回 HOME，
            //   下一轮循环开头又会覆写 got_flags。
            if (g_fa.got_flags & 4u) dupcards++;
            if (g_fa.got_flags & 1u) newcards++;
            if (first_gain[slot] == 0xFFFF) first_gain[slot] = g_fa.got_clover;
        }
        fa_claim();
    }

    // ★ 快速返回这条路也走一遍（2026-09-19 改口径后的正确行为）：
    //   应当结算成 FA_BACK，明信片照抽，但**一棵三叶草也不给** ——
    //   花掉的那几棵就是买这张明信片的价钱。
    //   特意挑 slot=2（1 时档，正常一趟该给 2 棵）：base 非 0 还给出 0，
    //   才能证明是召回这条分支压掉的，而不是本来就该给 0。
    g_fa.state  = FA_AWAY;
    g_fa.slot   = 2;
    g_fa.tbase  = b;
    g_fa.depart = now;
    g_fa.arrive = now + 9999;
    fa_recall();
    if (g_fa.state == FA_BACK && g_fa.got_clover == 0u) recall_ok = 1;
    fa_claim();

    // ★ dup 那一格在 40 趟里**应当 > 0**：每档只有 4~7 个景点，40 趟必然把
    //   好几档跑满，跑满之后再抽到的就是重复的（第五次新分支）。
    //   实测若恒为 0，说明 pick_owned_in_slot 那条路没被走到。
    ESP_LOGI(TAG, "self-check: %d/40 settled, %d cards (%d dup), pool %d/%d, souv %d/%d, recall %s",
             settled, newcards, dupcards, fa_postcards_have(), FA_DEST_COUNT,
             fa_souvenirs_have(), FA_SOUV_COUNT, recall_ok ? "OK" : "BAD");
    // 逐档奖励单独打一行：改 SLOT_CLOVER 数字却忘了改别处时，这行一眼就能对上。
    ESP_LOGI(TAG, "slot clover: trial=%u 5min=%u 1h=%u 4h=%u 12h=%u (expect 0 1 2 3 5)",
             (unsigned)first_gain[0], (unsigned)first_gain[1], (unsigned)first_gain[2],
             (unsigned)first_gain[3], (unsigned)first_gain[4]);

    g_fa = snap;
    fa_save_now();                          // 把真存档落回去
}

// ---------------------------------------------------------------------------

void app_main(void)
{
    esp_err_t nv = nvs_flash_init();
    if (nv == ESP_ERR_NVS_NO_FREE_PAGES || nv == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    ESP_ERROR_CHECK(bsp_display_init());
    bsp_display_backlight(100);

    if (!bsp_lvgl_init()) {
        ESP_LOGE(TAG, "LVGL init failed, halting");
        return;
    }

    // ★ 顺序要紧：方向必须在建任何界面之前定下来，
    //   否则布局会按 240x320 算完再被旋转，位置全错。
    tb_prefs_apply();

    if (bsp_battery_init() != ESP_OK)
        ESP_LOGW(TAG, "battery gauge not found; HUD will skip the battery field");

    bsp_button_init(NULL, NULL);      // 只为拿到 ADC 句柄；事件走自己的轮询
    tb_clock_init();

    fa_init();
    self_check();                     // ★ 必须在 fa_view_init 之前 —— 它会改 g_fa

    if (bsp_lvgl_lock(2000)) {
        fa_view_init();
        bsp_lvgl_unlock();
    }

    ESP_LOGI(TAG, "faraway ready (state=%u slot=%u clover=%u)",
             (unsigned)g_fa.state, (unsigned)g_fa.slot, (unsigned)g_fa.clover);

    TickType_t last_wake = xTaskGetTickCount();
    TickType_t prev      = last_wake;

    while (1) {
        TickType_t now = xTaskGetTickCount();
        uint32_t dt_ms = (uint32_t)(now - prev) * (uint32_t)portTICK_PERIOD_MS;
        prev = now;
        if (dt_ms < 1)   dt_ms = 1;
        if (dt_ms > 120) dt_ms = 120;   // 别让一次卡顿把动画按秒推进

        keys_poll((float)dt_ms);
        fa_poll();

        if (bsp_lvgl_lock(200)) {
            keys_dispatch();
            fa_view_tick(dt_ms);
            bsp_lvgl_unlock();
        }

        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(FRAME_MS));
    }
}
