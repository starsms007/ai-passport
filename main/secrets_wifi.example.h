// main/secrets_wifi.example.h —— WiFi 凭据模板
//
// 用法：复制成 secrets_wifi.h，把两个占位串换成你自己的，再编译。
//   cp main/secrets_wifi.example.h main/secrets_wifi.h
//
// ⚠ secrets_wifi.h 已在 .gitignore 里，不会被提交。
//   如果两个值保持占位串，固件会自动跳过联网、时间走手动设置那条路
//   —— 不卡住、不反复重连、不刷错误日志。
#pragma once

#define TB_WIFI_SSID      "PUT_YOUR_SSID_HERE"
#define TB_WIFI_PASS      "PUT_YOUR_PASSWORD_HERE"
#define TB_WIFI_HOSTNAME  "ai-passport"
