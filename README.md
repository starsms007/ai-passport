# AI Passport · 个人 Fork

[FoloToy AI Passport](https://github.com/FoloToy/ai-passport) 的个人 fork（ESP32-C3 卡片机固件）。

按官方 fork 约定，这个仓库的 `main` 分支**只用来跟上游同步**，与上游逐字节一致 ——
唯一例外是这份根目录 README（上游故意把仓库根目录让给 fork 开发者）。
每个应用各占一条 `feature/*` 分支，从上游最新 `main` 出发。

## 应用目录

### 去远方 · Faraway

> 在家上班的人，也想去看看远方。于是橘猫背上小书包，替你去。

一只橘猫的旅行手账 —— 跑在 240×320 竖屏上的收集小游戏。
在主页挑一趟行程（试玩 15 秒 / 5 分钟 / 1 小时 / 4 小时 / 12 小时），走得越久去的地方越远；
时间到了它会带回一张当地明信片和一件小纪念品。

**24 个目的地 · 24 件纪念品 · 4 个小游戏 · 天气图层 · 猫咪台词。离线可玩，不用配网。**

| | |
|---|---|
| 分支 | [`feature/faraway`](https://github.com/starsms007/ai-passport/tree/feature/faraway) |
| 分支说明 | [README](https://github.com/starsms007/ai-passport/blob/feature/faraway/README.md) |
| 基线 | 上游 `main` @ `1051209`（2026-09-20） |
| 改动 | 30 个文件、零删除；`components/bsp/` 未改 |

## 分支一览

| 分支 | 用途 | 与上游的关系 |
|---|---|---|
| `main` | 同步上游，外加这份 README | 逐字节一致 |
| `feature/faraway` | 《去远方》的全部改动 | 从 `main` 的 `1051209` 分叉 |

> 新项目请以**上游最新 `main`** 作为分叉点，而不要基于本 fork 的 `main` ——
> 它随上游滚动，随时可能与你本地不一致。

## 构建

与上游要求一致：**ESP-IDF v5.5.3**、目标 `esp32c3`、**8 MB Flash** 的板子（4 MB 板会无限重启）。

```bash
cp main/secrets_wifi.example.h main/secrets_wifi.h   # WiFi 校时可选，不填也能玩
idf.py set-target esp32c3
idf.py build
```

各应用的具体说明见对应分支的 README。

## 上游文档

- 上游仓库 <https://github.com/FoloToy/ai-passport>
- 上游中文说明 [`docs/README.zh_CN.md`](docs/README.zh_CN.md)
- fork 工作流约定 [`docs/fork-guide.zh_CN.md`](docs/fork-guide.zh_CN.md)（[English](docs/fork-guide.md)）
- 构建与测试 [`docs/development/engineering/build-and-test.zh_CN.md`](docs/development/engineering/build-and-test.zh_CN.md)
- 项目收尾清单 [`docs/development/release/project-completion.zh_CN.md`](docs/development/release/project-completion.zh_CN.md)

## 许可

跟随上游 [`LICENSE`](LICENSE)。
