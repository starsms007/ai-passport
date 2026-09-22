// main/fa_game.c —— 《去远方》玩法逻辑实现
//
// ⚠ 本文件里的中文都会进字库（tools/gen_fa_fonts.py 扫 main/*.c 的非 ASCII 字符）。
//   所以 ESP_LOG 一律用英文 —— 日志文案没人看，塞进字库就是白占 flash。
//   注释里可以随便写中文，扫描前会剥掉。

#include "fa_game.h"

#include <string.h>
#include <time.h>

#include "esp_log.h"
#include "esp_random.h"
#include "esp_timer.h"
#include "nvs.h"

static const char *TAG = "fa_game";

// NVS 命名空间。跟 tb_clock 的 "tbc"、tb_prefs 的 "tbp" 并列，互不干扰 ——
// 分开存的价值是：擦掉游戏存档不会顺带把屏幕方向和时钟一起清掉。
#define NVS_NS "fa"

fa_save_t g_fa;

// ------------------------------------------------------------------ 数据表

// ★★ 2026-09-19 重排：这 24 条**按「离家远近」从近到远排**，而且每一档的
//   起止下标是连续的 —— 见下面 DEST_SLOT / DEST_SLOT_FIRST。
//
//   为什么重排：原来这 24 个地名与 24 张明信片素材**对不上号**。
//   素材是按 city_ / town_ / prov_ / far_ / edge_ 五组画的，每一组画的是什么，
//   存在 03-素材流水线/art/manifest.json 的 subject 字段里 —— 那才是唯一真相。
//   旧表把「青岛」配到可可西里的荒原、把「武夷山」配到漠河的极光，
//   玩家拿到手就会看出「说好的青岛怎么是戈壁」。
//   现在每个地名都**照着它自己那张素材画的什么**来命名，一一对应，不再凑合。
//
//   ⚠ 顺序即语义：改这里的顺序 = 改档位分组，必须同步改 fa_view.c 的 DEST_CARD
//     （两张表按下标一一对应），否则又会出现「名字是西湖、图是黄山」。
static const char *const DEST_NAME[FA_DEST_COUNT] = {
    // ---- 第 0 档「试玩 15 秒」：家门口的近郊田园（素材 city_*）----
    "婺源", "宏村", "蜀南竹海", "扬州",
    // ---- 第 1 档「5 分」：小城古镇（素材 town_*）----
    "西塘", "西湖", "元阳", "霞浦",
    // ---- 第 2 档「1 时」：名山大川（素材 prov_*）----
    "黄山", "桂林", "泰山", "长城",
    // ---- 第 3 档「4 时」：远方（素材 far_*）----
    "张掖", "额济纳", "泸沽湖", "喀纳斯", "稻城",
    // ---- 第 4 档「12 时」：极边（素材 edge_*）----
    "布达拉宫", "敦煌", "漠河", "可可西里", "呼伦贝尔", "雨崩", "塔克拉玛干",
};

// 题词也按同一顺序重写 —— 每一句都对应那张明信片画的东西。
static const char *const DEST_VERSE[FA_DEST_COUNT] = {
    "油菜花海，开到天边", "村口石桥，溪水绕村", "竹影成排，光斑筛下", "河堤垂柳，拂过水面",
    "青石窄巷，一盏红灯", "断桥残雪，一湖烟雨", "层叠水田，映着天光", "退潮滩涂，夕照成金",
    "云海翻涌，松涛入梦", "一江碧水，两岸青峰", "日出东方，群山皆小", "城墙蜿蜒，落日照砖",
    "彩色岩层，层层叠叠", "胡杨金黄，老树虬枝", "湖面如镜，猪槽船静", "白桦金黄，河湾碧绿",
    "雪山草甸，牛奶海蓝",
    "红墙白殿，日照金顶", "风过鸣沙，月在泉心", "绿色极光，横过雪夜", "荒原风里，羚羊成群",
    "天似穹庐，草浪起伏", "冰川垂顶，经幡猎猎", "沙脊如波，霞满长天",
};

// 画明信片小风景时的配色档（0..5）。真明信片一显示就把这套盖住了，
// 所以它现在的用处是「还没去过时的那块抽象色块」和纪念品/设置页的占位，
// 只要同档之内别太单调就行。
static const uint8_t DEST_HUE[FA_DEST_COUNT] = {
    3, 2, 3, 3,
    4, 3, 0, 1,
    0, 2, 5, 1,
    1, 1, 2, 3, 2,
    5, 4, 5, 0, 0, 2, 1,
};

// ★ 每个景点属于哪一档（下标 = 景点号，值 = 档位号 0..4）。
//   为什么写成显式表而不是「算下标区间」：两张表（这张和上面的名字）一旦
//   有一个人改了顺序、另一个没跟上，算区间就会静默错位 ——
//   表现是「选 5 分钟却给你一张布达拉宫」，而且不会报任何错。
//   写死在这，改了名字表就**必须**改它，编译器帮不了忙，但至少是一处。
static const uint8_t DEST_SLOT[FA_DEST_COUNT] = {
    0, 0, 0, 0,
    1, 1, 1, 1,
    2, 2, 2, 2,
    3, 3, 3, 3, 3,
    4, 4, 4, 4, 4, 4, 4,
};


// ★★ 2026-09-19（第三次）：这一张表**照素材改过名**。
//   原来那 12 个名字（草帽/风铃/陶笛/绣帕/松果/贝壳/竹伞/铜铃/书签/木雕/邮票/香囊）
//   是当初凭空起的，画面上一件都对不上 —— 收集册里的纪念品就一直只能显示一块
//   抽象色块（实机反馈「收藏册里面纪念品显示不出来」）。
//   美术管线里现成有两套各 12 件的纪念品图标：
//     souvenir_a = 地方特产（桂花糕、酥油茶……）
//     souvenir_b = 自然拾遗（海边贝壳、松果……）
//   ★★ 2026-09-21（第四十七轮）：**两套都采用**（第三轮只用了 a）——
//      收藏册新增「纪念品」图鉴，两页 3x4 = 24 件。
//   ⚠ 顺序必须与 manifest 里两条 job 的 note 一字不差：
//      第 0 件是桂花糕、第 12 件是海边贝壳，册子第 0 / 12 张图就必须是它们。
//      （两套的帧号 = k % 12，见 fa_view.c 的 souv_refresh。）
//   ⚠ 名字是纯显示串，不参与存档编码 —— 改这里**不需要**升 FA_SAVE_VER。
//   改完必须重跑 tools/gen_fa_fonts.py（白话：屏幕上要出现的字得先烤进字库）。
static const char *const SOUV_NAME[FA_SOUV_COUNT] = {
    "桂花糕", "酥油茶", "葡萄干", "烤馕", "竹筒饭", "青稞饼",
    "竹编小篮", "陶哨", "扎染方巾", "木刻小猫", "手工草鞋", "漆器小碗",
    "海边贝壳", "火山石", "花瓣书签", "松果", "彩色砂瓶", "干枯胡杨枝",
    "会响的石头", "半张旧地图", "生锈的钥匙", "褪色车票", "空玻璃瓶", "一枚老铜钱",
};

// 时长档位。第 0 档是试玩档，见 fa_game.h 的说明。
// ★ 四张表必须同步增删 —— 索引就是档位号，错一格就会出现「选 5 分、跑 1 小时」。
//   「10 分」已于 2026-09-17 删除（理由见 fa_game.h 的 FA_SLOT_COUNT）。
static const char *const SLOT_LABEL[FA_SLOT_COUNT] = { "试玩", "5分", "1时", "4时", "12时" };
static const char *const SLOT_LONG [FA_SLOT_COUNT] = { "试玩 15 秒", "5 分钟",
                                                       "1 小时", "4 小时", "12 小时" };
static const uint32_t    SLOT_SECS [FA_SLOT_COUNT] = { 15, 300, 3600, 14400, 43200 };
// ★ 2026-09-19 改成用户逐档点名的数字（原来是 {1,1,3,5,8}，还带 0~1 随机浮动）。
//   试玩档是 0 —— 它同时是下面所有「折价补偿」的闸门，见 fa_finish 的说明。
static const uint16_t    SLOT_CLOVER[FA_SLOT_COUNT] = { 0, 1, 2, 3, 5 };

// ------------------------------------------------------------------ 取表

static int clamp_i(int v, int lo, int hi)
{
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

const char *fa_slot_label(int slot) { return SLOT_LABEL[clamp_i(slot, 0, FA_SLOT_COUNT - 1)]; }
const char *fa_slot_long (int slot) { return SLOT_LONG [clamp_i(slot, 0, FA_SLOT_COUNT - 1)]; }
uint32_t    fa_slot_secs (int slot) { return SLOT_SECS [clamp_i(slot, 0, FA_SLOT_COUNT - 1)]; }
// 只有第 0 档是「试玩」。写成函数而不是散落的 `slot == 0`：
// 万一以后试玩档换位置，只要改这里一处。
bool        fa_slot_is_trial(int slot) { return slot == 0; }

const char *fa_dest_name (int i) { return DEST_NAME [clamp_i(i, 0, FA_DEST_COUNT - 1)]; }
const char *fa_dest_verse(int i) { return DEST_VERSE[clamp_i(i, 0, FA_DEST_COUNT - 1)]; }
uint8_t     fa_dest_hue  (int i) { return DEST_HUE  [clamp_i(i, 0, FA_DEST_COUNT - 1)]; }
const char *fa_souv_name (int i) { return SOUV_NAME [clamp_i(i, 0, FA_SOUV_COUNT - 1)]; }

// ---- 景点 <-> 档位（2026-09-19）----
// 走多远，见多远：短途只到近处，长途才到远方。理由和分组依据见 DEST_NAME 的说明。
bool fa_dest_in_slot(int dest, int slot)
{
    if (dest < 0 || dest >= FA_DEST_COUNT) return false;
    if (slot < 0 || slot >= FA_SLOT_COUNT) return false;
    return DEST_SLOT[dest] == (uint8_t)slot;
}

int fa_slot_dest_first(int slot)
{
    for (int i = 0; i < FA_DEST_COUNT; i++)
        if (DEST_SLOT[i] == (uint8_t)slot) return i;
    return 0;
}

int fa_slot_dest_count(int slot)
{
    int n = 0;
    for (int i = 0; i < FA_DEST_COUNT; i++)
        if (DEST_SLOT[i] == (uint8_t)slot) n++;
    return n;
}

// ------------------------------------------------------------------ 时间

// tb_clock 把系统时间设成 1600000000 之后的值才算数（2020-09 之前一律当没时间）。
// 没有 wall clock 时退回到「开机秒数」—— 本次开机内足够用，跨重启的行程在
// fa_init() 里会被取消。
#define FA_EPOCH_MIN 1600000000

uint32_t fa_now(uint8_t *base_out)
{
    time_t t = time(NULL);
    if (t >= (time_t)FA_EPOCH_MIN) {
        if (base_out) *base_out = 1;
        return (uint32_t)t;
    }
    if (base_out) *base_out = 2;
    return (uint32_t)(esp_timer_get_time() / 1000000LL);
}

// ------------------------------------------------------------------ 存档

void fa_save_now(void)
{
    nvs_handle_t h;
    if (nvs_open(NVS_NS, NVS_READWRITE, &h) != ESP_OK) {
        ESP_LOGW(TAG, "nvs open for write failed; progress not saved");
        return;
    }
    nvs_set_blob(h, "s", &g_fa, sizeof(g_fa));
    nvs_commit(h);
    nvs_close(h);
}

// ------------------------------------------------------------------ 开场剧情

#define NVS_KEY_INTRO "intro"

// ★ 用独立键，不塞进 fa_save_t。
//   塞进去就要动结构体，动了就得升 FA_SAVE_VER，升了老存档就读不出来 ——
//   为了「开场只看一次」这点事，把玩家的三叶草和明信片清掉，不值。
bool fa_intro_done(void)
{
    nvs_handle_t h;
    if (nvs_open(NVS_NS, NVS_READONLY, &h) != ESP_OK) return false;
    uint8_t v = 0;
    esp_err_t e = nvs_get_u8(h, NVS_KEY_INTRO, &v);
    nvs_close(h);
    return (e == ESP_OK && v == 1);
}

void fa_intro_mark(void)
{
    nvs_handle_t h;
    if (nvs_open(NVS_NS, NVS_READWRITE, &h) != ESP_OK) return;
    nvs_set_u8(h, NVS_KEY_INTRO, 1);
    nvs_commit(h);
    nvs_close(h);
}

// ------------------------------------------------------------------ 主页面风景

// ★ 和「开场看过没有」一样用独立键，**不塞进 fa_save_t**。
//   理由一模一样：往结构体里加字段就要升 FA_SAVE_VER，升了老存档读不出来，
//   玩家已经攒下的明信片和纪念品会一起清零 —— 为一个背景图不值当。
#define NVS_KEY_SCENE "scene"

uint8_t fa_scene(void)
{
    nvs_handle_t h;
    if (nvs_open(NVS_NS, NVS_READONLY, &h) != ESP_OK) return FA_SCENE_NONE;

    uint8_t v = FA_SCENE_NONE;
    esp_err_t e = nvs_get_u8(h, NVS_KEY_SCENE, &v);
    nvs_close(h);

    if (e != ESP_OK || v >= FA_DEST_COUNT) return FA_SCENE_NONE;
    // ★★ 必须再拿 postcards 位图验一次。存档 blob 和 scene 是两个独立 NVS 键，
    //   擦掉游戏存档（或升级 FA_SAVE_VER）不会连 scene 一起擦 ——
    //   于是可能出现「挂着一个从没去过的地方的风景」。
    //   在这里拦掉，界面就永远不用去想这种情况。
    if (((g_fa.postcards >> v) & 1u) == 0) return FA_SCENE_NONE;
    return v;
}

bool fa_scene_set(uint8_t d)
{
    // 只能挂**去过**的地方 —— 没去过的连图都没见过（见 fa_game.h 的说明）
    if (d >= FA_DEST_COUNT)                return false;
    if (((g_fa.postcards >> d) & 1u) == 0) return false;

    nvs_handle_t h;
    if (nvs_open(NVS_NS, NVS_READWRITE, &h) != ESP_OK) return false;
    nvs_set_u8(h, NVS_KEY_SCENE, d);
    nvs_commit(h);
    nvs_close(h);

    ESP_LOGI(TAG, "scene set: dest=%u (%s)", (unsigned)d, DEST_NAME[d]);
    return true;
}

void fa_scene_clear(void)
{
    nvs_handle_t h;
    if (nvs_open(NVS_NS, NVS_READWRITE, &h) != ESP_OK) return;
    nvs_set_u8(h, NVS_KEY_SCENE, FA_SCENE_NONE);
    nvs_commit(h);
    nvs_close(h);
    ESP_LOGI(TAG, "scene back to default");
}

// --------------------------------------------------------- 小院道具（已删除）
//
// ★★ 2026-09-19（第五次）整节删除，对应 fa_game.h 里同名的那段说明。
//   原来这里住着 PROP_NAME[13] / PROP_NEED[13]（解锁门槛 1,2,4,…,24）、
//   fa_prop_name / need / have / have_count 四个查询、NVS 键 "yard"、
//   以及 fa_yard_get / fa_yard_set 两个槽位读写（u16 打包 4 个 4bit 槽）。
//
//   ⚠ 顺带说明为什么删得掉：这 13 件的**解锁门槛全挂在明信片数量上**
//     （拿 1/2/4/…/24 张各解锁一件），所以删掉之后没有任何「收集进度」丢数据 ——
//     明信片位图本身还在，玩家的进度一个 bit 都没少。
//
//   ⚠ 不要为了「删干净」去把 NVS 里的 "yard" 也擦掉：那要 open/erase 一次命名空间
//     或者按 key 删，多一处擦写就多一次写坏存档的机会，而它只是一个没人读的 u16。

// ★ 第十四轮：这条前置声明**必须留在 fa_init 之前** —— fa_init 现在会调它
//   （关机自动结算，见下面那段）。原来那份声明在 429 行的定义上方，位置在
//   fa_init **后面**，在这里调用会直接编译不过（C 不允许先用了再声明）。
static void fa_finish(bool recalled);

void fa_init(void)
{
    memset(&g_fa, 0, sizeof(g_fa));
    g_fa.magic    = FA_SAVE_MAGIC;
    g_fa.ver      = FA_SAVE_VER;
    g_fa.slot     = 1;          // 默认停在「5 分」，别停在试玩档
    g_fa.clover   = 3;          // 开局给 3 棵，第一趟不用等
    g_fa.got_souv = 255;

    nvs_handle_t h;
    if (nvs_open(NVS_NS, NVS_READONLY, &h) == ESP_OK) {
        fa_save_t tmp;
        size_t n = sizeof(tmp);
        esp_err_t e = nvs_get_blob(h, "s", &tmp, &n);
        nvs_close(h);
        if (e == ESP_OK && n == sizeof(tmp) &&
            tmp.magic == FA_SAVE_MAGIC && tmp.ver == FA_SAVE_VER) {
            g_fa = tmp;
            ESP_LOGI(TAG, "save loaded: trips=%u clover=%u postcards=0x%06x",
                     (unsigned)g_fa.trips, (unsigned)g_fa.clover,
                     (unsigned)(g_fa.postcards & 0xFFFFFF));
        } else if (e == ESP_OK) {
            ESP_LOGW(TAG, "save blob mismatch (size=%u magic=%04x ver=%u); starting fresh",
                     (unsigned)n, (unsigned)tmp.magic, (unsigned)tmp.ver);
        }
    }

    // ★ 夹到**最后一档**而不是第 1 档。
    //   档位表删过项（2026-09-17 删「10 分」），老存档里的 slot 可能越界；
    //   越界说明玩家当初选的是最长的档，夹回最大档才不冤枉他。
    if (g_fa.slot >= FA_SLOT_COUNT) g_fa.slot = FA_SLOT_COUNT - 1;
    if (g_fa.state > FA_BACK)       g_fa.state = FA_HOME;
    g_fa.got_souv = 255;

    // ★ 在途行程没有可用的时间基准就取消。
    //   刚出厂、从没对过时的机器上没有 wall clock，出发时刻记的是「开机秒数」；
    //   重启后秒数归零，再拿新秒数去减旧出发时刻会算出荒唐的剩余时间。
    //   这里直接收摊 —— 不假装，也不让界面显示一个假倒计时。
    //
    // ★★ 2026-09-19（第十三轮）**判据收紧了**（原来是 `g_fa.tbase != b`）。
    //   旧判据漏了一种情况：tbase 与 b **同为 2** —— 也就是「出发时没有 wall clock、
    //   现在也还没有」。重启后开机秒数从 0 重新数，可两个 2 被判成「同类」，
    //   于是这趟行程**活了下来**，拿上一开机的 depart/arrive 去减这一开机的秒数
    //   算剩余时间。上面那句注释写着「这里直接收摊」，但那个条件收不了这个摊。
    //
    //   根子在于 **tbase 存的是「基准的种类」（1 = wall clock，2 = 开机秒数），
    //   而不是某一开机的身份** —— 所以「同类」推不出「同一个基准」。
    //   开机秒数永远跨不过一次重启，**只有 1 是能活下来的那个基准**。
    //   ⇒ 判据写成「出发时是 1 且现在还是 1」，其余一律收摊。
    //   ⚠ 这条对常见路径（对过时的机器：1 -> 1）没有任何行为改变，只是补上漏洞。
    uint8_t b = 0;
    fa_now(&b);

    // ★★★ 2026-09-19（第十四轮）**在途行程不再丢弃，改成「已归来」结算。**
    //   用户原话：「如果关机了，就自动完成本次旅行。」
    //
    //   第十三轮那版是「时间基准对不上就直接收摊」—— 诚实，但后果是这趟白跑：
    //   没有明信片、没有三叶草、连归来动画都没有，只剩一行 log。
    //   现在改成：**只要开机看到 FA_AWAY，就当这趟已经走完了。**
    //
    //   ⚠ 为什么可以不判「够没够时长」：这块板子**没有 RTC 备份电池**
    //     （见 tb_clock.h 第一行），断电期间到底过了多久**物理上就无从得知**。
    //     既然判不了，就按用户要的口径算 —— 完成。
    //   ⚠ 副作用说在明处：**每次断电重启都会结算一趟**（三叶草 + 一张明信片）。
    //     这不是 bug，是无 RTC 硬件上这个设计的必然结果；想堵这个口子就得
    //     先有可信的离线时间，而那恰恰是缺的东西。
    //
    //   ★ 走 fa_finish(false) 而不是自己写一套：false = **正常到点**，
    //     于是三叶草按档位照发、明信片照抽、纪念品照掷 —— 和真跑到点回来
    //     完全是同一条路径，不会出现「关机回来得到的东西和正常回来不一样」。
    //   ★ 结算只改状态与存档，**不碰 UI**。亮那张到达卡是 fa_view_init() 的活儿：
    //     它开头就有 `if (g_fa.state == FA_BACK)` 那一支（早就有，当时是给
    //     「重启前没来得及收下」兜底的），这里正好复用 —— 开机第一眼就是
    //     那张明信片，不用重播走路动画（重播会让人以为又出了一趟）。
    if (g_fa.state == FA_AWAY) {
        ESP_LOGW(TAG, "trip interrupted by power-off (tbase %u -> %u); settling as arrived",
                 (unsigned)g_fa.tbase, (unsigned)b);
        fa_finish(false);
    }
}

// ------------------------------------------------------------------ 归来结算

static void fa_finish(bool recalled);

void fa_set_slot(int slot)
{
    if (g_fa.state != FA_HOME) return;
    g_fa.slot = (uint8_t)clamp_i(slot, 0, FA_SLOT_COUNT - 1);
    fa_save_now();
}

void fa_depart(void)
{
    if (g_fa.state != FA_HOME) return;

    uint8_t b = 0;
    uint32_t now = fa_now(&b);
    g_fa.depart = now;
    g_fa.arrive = now + fa_slot_secs(g_fa.slot);
    g_fa.tbase  = b;
    g_fa.state  = FA_AWAY;
    g_fa.trips++;
    fa_save_now();

    ESP_LOGI(TAG, "depart: slot=%u secs=%u base=%u", (unsigned)g_fa.slot,
             (unsigned)fa_slot_secs(g_fa.slot), (unsigned)b);
}

// 在 [0,n) 里随机挑一个「位图里还空着」的位，返回其下标；没有空位返回 -1
static int pick_free(uint32_t mask, int count)
{
    uint32_t free_bits = (~mask) & ((count >= 32) ? 0xFFFFFFFFu : ((1u << count) - 1u));
    if (!free_bits) return -1;

    int n = 0;
    for (int i = 0; i < count; i++) if (free_bits & (1u << i)) n++;

    int pick = (int)(esp_random() % (uint32_t)n);
    for (int i = 0; i < count; i++) {
        if (free_bits & (1u << i)) {
            if (pick == 0) return i;
            pick--;
        }
    }
    return -1;
}

// 在**第 slot 档**里随机挑一个「还没拿到」的景点；这一档全拿完了返回 -1。
// ★ 为什么要按档位抽而不是全池抽：见 DEST_NAME 上面那段 —— 「走多远，见多远」。
//   全池抽的话，选「试玩 15 秒」也可能直接给你一张塔克拉玛干，
//   那五个档位就只剩下等待时长这一个区别了。
static int pick_free_in_slot(uint32_t mask, int slot)
{
    int pool[FA_DEST_COUNT];
    int n = 0;
    for (int i = 0; i < FA_DEST_COUNT; i++)
        if (DEST_SLOT[i] == (uint8_t)slot && ((mask >> i) & 1u) == 0) pool[n++] = i;
    if (n == 0) return -1;
    return pool[esp_random() % (uint32_t)n];
}

// ★★ 2026-09-19（第五次）新增：在**第 slot 档**里随机挑一个「已经拿到过」的景点。
//
//   为什么需要它（用户原话）：
//     「如果旅行结束，碰到与之前相同的明信片的时候，同样要显示出来，
//       不要只说一句相同。」
//   旧口径是「本档全拿到 -> 只折 2 棵三叶草，got_dest 留 0」，
//   到收获卡上就是一块**没有画面的兜底色块** —— 跑了五个小时回来，
//   看着像出了故障。现在照样抽一张、照样把图贴出来。
//
//   ★ 和 pick_free_in_slot 只差一个条件：这里要 `& 1u` 为真（已经有了）。
//     两份代码看起来像，但不能合并成一个带 flag 的函数 ——
//     那种写法在调用点上完全看不出「这一趟到底要新的还是要旧的」，
//     而这两件事的**后果**完全不同（一个改 postcards 位图，一个不改）。
static int pick_owned_in_slot(uint32_t mask, int slot)
{
    int pool[FA_DEST_COUNT];
    int n = 0;
    for (int i = 0; i < FA_DEST_COUNT; i++)
        if (DEST_SLOT[i] == (uint8_t)slot && ((mask >> i) & 1u) != 0) pool[n++] = i;
    if (n == 0) return -1;
    return pool[esp_random() % (uint32_t)n];
}

static void fa_finish(bool recalled)
{
    g_fa.got_flags = 0;
    g_fa.got_dest  = 0;
    g_fa.got_souv  = 255;

    const int      slot = clamp_i(g_fa.slot, 0, FA_SLOT_COUNT - 1);
    const uint16_t base = SLOT_CLOVER[slot];

    // ★★ 2026-09-19：召回这趟**照给明信片和纪念品，但不给三叶草**。
    //   用户口径：「用三叶草返回，可以给明信片和纪念品，但是不给三叶草，
    //   相当于用三叶草买的道具。」—— 花掉的那几棵就是这笔道具的价钱，
    //   所以这里的现象是：余额的净变化 = −报价，收获照常入袋。
    //
    //   ★ base 是「本档一趟给几棵」，试玩档是 0。它同时是下面两条折价补偿的**闸门**：
    //     本档明信片集满时折 +2 棵、纪念品集满时折 +1 棵 —— 这两条要是也跟着
    //     试玩档生效，15 秒一趟能刷出 2~3 棵，整个经济当场崩掉。
    //     所以「base == 0」= 这一档不给任何三叶草，补偿也不例外。
    //
    //   ★ 顺便去掉了原来 0~1 的随机浮动：用户是逐档报的数字，留着浮动就会
    //     出现「说好 1 棵给了 2 棵」—— 那不是彩蛋，是没兑现。
    uint16_t gain = recalled ? 0 : base;

    {
        // ★ 召回也照常抽明信片 —— 这就是「花三叶草买的道具」。
        //   抽的是**本档**里还没拿到的那几张（走多远、见多远，见 DEST_NAME 的说明）。
        int idx = pick_free_in_slot(g_fa.postcards, slot);
        if (idx >= 0) {
            g_fa.postcards |= (1u << idx);
            g_fa.got_dest   = (uint8_t)idx;
            g_fa.got_flags |= 1;
        } else {
            // ★★ 2026-09-19（第五次）本档全拿到之后，**照样抽一张已有的**。
            //   用户原话：「碰到与之前相同的明信片的时候，同样要显示出来，
            //   不要只说一句相同。」—— 见 pick_owned_in_slot 上面那段说明。
            //
            //   ⚠ 这里必须**只置 got_dest，不动 postcards 位图**：
            //     位图是「去过哪儿」的事实记录，重复抽到不该改它，
            //     否则「明信片 x / 24」那个计数会虚高。
            //   ⚠ 也**不用**判 recalled：召回的收获照给明信片（用户口径
            //     「相当于用三叶草买的道具」），重复的这一张同样照给画面。
            const int idx2 = pick_owned_in_slot(g_fa.postcards, slot);
            if (idx2 >= 0) {
                g_fa.got_dest   = (uint8_t)idx2;
                g_fa.got_flags |= 1;    // 有明信片可显示 —— 和「新到手」共用同一套画法
                g_fa.got_flags |= 4;    // ★ bit2：这一张是**早就有了**的
            }

            // 折价补偿照旧：本档跑完就该换更远的档，这一点没变。
            // ★ 仍然只在**不是召回**这一趟发 —— 召回那趟 base 只当闸门用。
            if (!recalled && base > 0) gain = (uint16_t)(gain + 2);
        }
    }

    // 纪念品 25% 概率
    if ((esp_random() % 100u) < 25u) {
        int si = pick_free(g_fa.souvenirs, FA_SOUV_COUNT);
        if (si >= 0) {
            g_fa.souvenirs |= (1u << si);
            g_fa.got_souv   = (uint8_t)si;
        } else if (!recalled && base > 0) {
            gain = (uint16_t)(gain + 1);
        }
    }

    // bit1：这趟是花三叶草提前回来的。收获卡据此换一行文案
    // （不是「没有收获」—— 上面这段照样发东西了，别把卡片写成自相矛盾）。
    if (recalled) g_fa.got_flags |= 2;

    g_fa.got_clover = gain;
    uint32_t c = (uint32_t)g_fa.clover + gain;
    g_fa.clover  = (uint16_t)(c > 999 ? 999 : c);
    g_fa.state   = FA_BACK;
    fa_save_now();

    // ★ 第五次加了 dup 一位：newcard=1 既可能是新到手、也可能是重复的，
    //   自检和实机排障时这两个数必须能分开看（否则「怎么又出了一张」无法定位）。
    ESP_LOGI(TAG, "back: clover +%u (have %u) dest=%u card=%u dup=%u souv=%u recalled=%d",
             (unsigned)gain, (unsigned)g_fa.clover, (unsigned)g_fa.got_dest,
             (unsigned)((g_fa.got_flags & 1u) ? 1 : 0),
             (unsigned)((g_fa.got_flags & 4u) ? 1 : 0),
             (unsigned)g_fa.got_souv, (int)recalled);
}

bool fa_poll(void)
{
    if (g_fa.state != FA_AWAY) return false;

    uint8_t b = 0;
    uint32_t now = fa_now(&b);

    // 时间源换了（例如行程中途 WiFi 对上时）—— 旧基准算出来的差值没有意义，收摊。
    if (b != g_fa.tbase) {
        ESP_LOGW(TAG, "time base changed mid-trip, finishing now");
        fa_finish(true);
        return true;
    }

    if ((int32_t)(now - g_fa.arrive) >= 0) {
        fa_finish(false);
        return true;
    }
    return false;
}

void fa_recall(void)
{
    if (g_fa.state != FA_AWAY) return;
    fa_finish(true);
}

// ------------------------------------------------------------------ 三叶草快速返回

// 剩余时长 -> 花费。表按上限升序，取第一个装得下的档。
// ⚠ 这张表必须与 fa_game.h 里那段说明保持一致 —— 改了一边忘了另一边，
//   玩家看到的价钱和扣掉的就不一样了。
//
// ★★ 2026-09-19（第十一轮）用户点名改价：
//     「4小时要用10个猫罐头，12小时要用30个猫罐头。」
//   所以这两档是**用户给定的锚点**，不是估出来的：
//     ≤4 小时 = 10、≤12 小时 = 30（正好都是每小时 2.5 个）。
//   低三档没有新指示，保持原值 1/2/3 —— 它们与锚点连起来是一条
//   「每小时单价随时间下降」的曲线（12/h -> 4/h -> 3/h -> 2.5/h -> 2.5/h），
//   也就是量越大越便宜。**别再按比例去"修"低档**：5 分钟档收 1 个是
//   上手价的定位（而且 1 是能收到的最小正整数）。
//   ⚠ 12 时档是长度的上限（SLOT_SECS 最大 43200），所以 remain 不可能超过它；
//     最后那行 0x7FFFFFFF 是兜底，正常永远不触发，留着防「时间基准错乱」。
static const struct { int32_t max_remain; uint8_t cost; } RECALL_TIER[] = {
    {      300,  1 },       // ≤ 5 分钟
    {     1800,  2 },       // ≤ 30 分钟
    {     3600,  3 },       // ≤ 1 小时
    {    14400, 10 },       // ≤ 4 小时     ← 第十一轮：4 -> 10
    {    43200, 30 },       // ≤ 12 小时    ← 第十一轮：新增这一档（原来并进「更长」）
    { 0x7FFFFFFF, 30 },     // 更长（兜底，正常到不了）
};

int fa_recall_cost(void)
{
    if (g_fa.state != FA_AWAY) return -1;

    // 试玩档免费。15 秒的行程，为它收三叶草既不合算也不像个欢迎动作。
    if (fa_slot_is_trial(g_fa.slot)) return 0;

    int remain = fa_remain_secs();
    // remain == 0：要么马上就到点（再等一帧 fa_poll() 自己就结算了），
    // 要么时间基准丢了（那这趟本来就白跑）。两种情况都不该再收钱。
    if (remain <= 0) return 0;

    for (unsigned i = 0; i < sizeof(RECALL_TIER) / sizeof(RECALL_TIER[0]); i++)
        if (remain <= RECALL_TIER[i].max_remain) return (int)RECALL_TIER[i].cost;

    // 到不了：最后一行是 0x7FFFFFFF。留着是为了「万一 remain 是个天文数字」时
    // 返回一个和末档一致的价，而不是回到旧版那个 5（那会让报价莫名其妙变便宜）。
    return 30;
}

bool fa_recall_paid(void)
{
    int cost = fa_recall_cost();
    if (cost < 0) return false;
    if (cost > (int)g_fa.clover) return false;   // 不够就不动 —— 别扣一半

    // ★ 先扣再结算。fa_finish() 里会 fa_save_now()，把扣完的余额一起落盘；
    //   反过来的话（先结算后扣）中途掉电就会白送一趟。
    g_fa.clover = (uint16_t)(g_fa.clover - (uint16_t)cost);
    fa_finish(true);                             // true = 提前召回，不给新明信片

    ESP_LOGI(TAG, "paid recall: cost=%d clover_left=%u", cost, (unsigned)g_fa.clover);
    return true;
}

void fa_claim(void)
{
    if (g_fa.state != FA_BACK) return;
    g_fa.state = FA_HOME;
    g_fa.depart = g_fa.arrive = 0;
    fa_save_now();
}

// ------------------------------------------------------------------ 小游戏奖励

// ★★ 2026-09-19（第十轮）新增。这是**唯一**一条不经过行程结算就发猫罐头的通路 ——
//   收藏册里那个「接猫罐头」小游戏接满一定数量就调它。
//
//   为什么不让界面层直接写 g_fa.clover：
//     ① 999 上限的夹取只应该写在一处（fa_finish 里那份是行程结算专用的），
//        这个数不该散到界面层去；
//     ② 它自己 fa_save_now() —— 掉电不会把刚赢来的那几个丢掉。
//
//   ⚠ 它**不碰** got_clover / got_flags / state。那几个是「本趟归来收获」的字段，
//     收获卡正在读；这里插一脚会让卡片上那行数字变成小游戏赢来的，
//     玩家看到的就是「卡还没收下、罐头怎么先多了」。
bool fa_award_clover(int n)
{
    if (n <= 0) return false;

    uint32_t c = (uint32_t)g_fa.clover + (uint32_t)n;
    g_fa.clover = (uint16_t)(c > 999 ? 999 : c);
    fa_save_now();

    ESP_LOGI(TAG, "mini award: +%d clover -> %u", n, (unsigned)g_fa.clover);
    return true;
}

// ------------------------------------------------------------------ 查询

int fa_remain_secs(void)
{
    if (g_fa.state != FA_AWAY) return 0;
    uint8_t b = 0;
    uint32_t now = fa_now(&b);
    if (b != g_fa.tbase) return 0;
    int32_t d = (int32_t)(g_fa.arrive - now);
    return d > 0 ? (int)d : 0;
}

int fa_slot_from_secs(uint32_t s)
{
    for (int i = 0; i < FA_SLOT_COUNT; i++)
        if (s <= SLOT_SECS[i]) return i;
    return FA_SLOT_COUNT - 1;
}

static int popcount32(uint32_t v)
{
    int n = 0;
    while (v) { n += (int)(v & 1u); v >>= 1; }
    return n;
}

int fa_postcards_have(void) { return popcount32(g_fa.postcards & 0x00FFFFFFu); }
int fa_souvenirs_have(void) { return popcount32(g_fa.souvenirs & 0x00FFFFFFu); }
