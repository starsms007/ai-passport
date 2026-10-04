#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""只写 app 段升级 r58，并在烧写前后**对比 NVS 分区**，用字节证明存档没被动。

为什么需要它：r58 的「整片」镜像在 NVS 分区（0x9000..0xF000）里全是 0xFF，
整片写会把玩家的存档一起抹掉（NVS 的擦除态就是"空"）。
只写 app（偏移 0x10000）不碰 NVS ⇒ 进度、三叶草、明信片、纪念品全部保留。

判据（脚本自己会打印）：
  NVS 前  = read_flash 0x9000 0x6000 的 sha256
  NVS 后  = 同上，再读一次
  ⇒ 两者相同 = **存档分区一个字节都没动**（这就是"不影响存档"的证据）

用法：
  python _flash_app_only_and_verify.py --port COM5
  python _flash_app_only_and_verify.py            # 自动找端口
"""
import argparse
import hashlib
import os
import subprocess
import sys

PY = sys.executable
APP_ONLY = r"D:\Games\_firmware\faraway-r58-app-only-0x10000.bin"
NVS_OFF, NVS_LEN = 0x9000, 0x6000          # 规范 1.1 分区表：nvs, data, nvs, 0x9000, 0x6000
APP_OFF = 0x10000


def esp(*args):
    """跑一条 esptool 命令（v4 用下划线子命令）。"""
    cmd = [PY, "-m", "esptool", "--chip", "esp32c3"] + list(args)
    return subprocess.run(cmd, capture_output=True, text=True, encoding="utf-8", errors="replace")


def find_port():
    try:
        import serial.tools.list_ports as lp
    except Exception:
        return None
    for p in lp.comports():
        # Espressif 原生 USB Serial/JTAG
        if (p.vid, p.pid) == (0x303A, 0x1001):
            return p.device
    ports = [p.device for p in lp.comports()]
    return ports[0] if ports else None


def read_region(port, off, ln, out):
    r = esp("-p", port, "-b", "460800", "read_flash", hex(off), hex(ln), out)
    if r.returncode != 0 or not os.path.exists(out):
        return None, (r.stderr or r.stdout or "")[-400:]
    return hashlib.sha256(open(out, "rb").read()).hexdigest(), None


def app_desc(buf: bytes):
    """在 app 镜像头部里找 esp_app_desc 的 magic，解出 project_name / version / date / time。"""
    magic = (0xABCD5432).to_bytes(4, "little")
    i = buf.find(magic)
    if i < 0:
        return None
    p = i + 16                      # magic(4)+secure_version(4)+reserv(8) 之后是 version[32]
    def s(o, n):
        raw = buf[p + o: p + o + n]
        return raw.split(b"\x00", 1)[0].decode("utf-8", "replace")
    return {"version": s(0, 32), "project": s(32, 32), "time": s(64, 16), "date": s(80, 16)}


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--port", default=None)
    ap.add_argument("--tmp", default=r"D:\Tools\_dl")
    ap.add_argument("--skip-preflight", action="store_true",
                    help="跳过上机前检查（不建议）")
    a = ap.parse_args()
    os.makedirs(a.tmp, exist_ok=True)

    if not os.path.exists(APP_ONLY):
        print("!! 找不到纯 app 镜像:", APP_ONLY)
        return 2
    print("app-only 镜像 :", APP_ONLY)
    print("            sha256:", hashlib.sha256(open(APP_ONLY, "rb").read()).hexdigest())
    print("            大小  :", os.path.getsize(APP_ONLY), "B  →  写偏移", hex(APP_OFF))

    port = a.port or find_port()
    if not port:
        print("\n!! 没有检测到串口（板子未插 / 驱动未装 / pyserial 缺失）。")
        print("   插上板子后重跑，或显式指定：--port COM5")
        return 3
    print("端口 :", port)

    nvs_before = os.path.join(a.tmp, "_nvs_before.bin")
    nvs_after = os.path.join(a.tmp, "_nvs_after.bin")
    pt_dev = os.path.join(a.tmp, "_pt_dev.bin")
    app_dev = os.path.join(a.tmp, "_app_dev.bin")

    # ---- [0] 上机前检查：设备上是不是**我们这套分区表**（只写 app 的前提）----
    if not a.skip_preflight:
        print("\n[0/4] 上机前检查 …")
        full = r"D:\Games\_firmware\faraway-r58-8MB.bin"
        h_pt, err = read_region(port, 0x8000, 0x1000, pt_dev)
        if h_pt is None:
            print("  读分区表失败:", err)
            return 4
        ours = hashlib.sha256(open(full, "rb").read()[0x8000:0x9000]).hexdigest()
        print("      设备分区表 sha256 =", h_pt[:32])
        print("      我们分区表 sha256 =", ours[:32])
        print("      一致 =", h_pt == ours)
        if h_pt != ours:
            print("\n  !! 设备上不是我们的分区表（很可能还是官方固件）。")
            print("     这时**只写 app 会砸掉 cardid/recovery** ⇒ 必须整片写 0x0。")
            print("     整片写会把存档清掉（NVS 全 0xFF），但官方固件的机器上本来就没有我们的进度。")
            print("     已停止，未写入任何东西。")
            return 8
        r = esp("-p", port, "-b", "460800", "read_flash", "0x10000", "0x1000", app_dev)
        if r.returncode == 0 and os.path.exists(app_dev):
            d = app_desc(open(app_dev, "rb").read())
            print("      设备当前 app:", d if d else "（没解出描述符）")
            if d and d.get("project") and d["project"] != "faraway":
                print("  !! 设备上的 app 不是《去远方》(faraway) —— 请确认你要覆盖的是哪一台。")
                print("     已停止，未写入任何东西。")
                return 9


    print("\n[1/4] 读 NVS 分区（存档就在这儿）…")
    h1, err = read_region(port, NVS_OFF, NVS_LEN, nvs_before)
    if h1 is None:
        print("  读取失败:", err)
        return 4
    print("      NVS 前  sha256 =", h1[:32])

    print("\n[2/4] 只写 app 段（0x10000）…")
    r = esp("-p", port, "-b", "460800", "write_flash", hex(APP_OFF), APP_ONLY)
    print((r.stdout or "")[-500:])
    if r.returncode != 0:
        print("!! 写失败:", (r.stderr or "")[-500:])
        return 5
    print("      写完成，rc =", r.returncode)

    print("\n[3/4] 回读 NVS 分区 …")
    h2, err = read_region(port, NVS_OFF, NVS_LEN, nvs_after)
    if h2 is None:
        print("  读取失败:", err)
        return 6
    print("      NVS 后  sha256 =", h2[:32])

    print("\n[4/4] 判据")
    same = (h1 == h2)
    print("      NVS 前后逐字节相同 =", same)
    print("      ⇒", "存档分区**没有被动过** —— 玩家进度保留（这是可复算的证据）"
          if same else "★ NVS 变了，请立刻停下并把两份 _nvs_*.bin 交给我核对")
    return 0 if same else 7


if __name__ == "__main__":
    sys.exit(main())
