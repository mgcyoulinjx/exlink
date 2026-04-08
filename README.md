# Exlink

Exlink 是一个基于 ESP32-S3 的多功能工具项目（LVGL 界面），包含 DC POWER、PWM、UART、I2C Scan、DSO、BLE UART、频率计等功能。

## GitHub 安装教程

### 1. 安装 Git
Windows:
1. 下载并安装 Git: https://git-scm.com/download/win
2. 安装完成后打开 PowerShell，执行:

```powershell
git --version
```

### 2. 克隆项目

```powershell
git clone https://github.com/mgcyoulinjx/exlink.git
cd exlink
```

### 3. 安装 PlatformIO
方式一（推荐，使用 VS Code）:
1. 安装 VS Code: https://code.visualstudio.com/
2. 在扩展市场安装 `PlatformIO IDE`
3. 打开本项目目录

方式二（命令行）:

```powershell
python -m pip install -U platformio
platformio --version
```

## 使用说明

### 1. 编译固件

```powershell
platformio run
```

编译产物在:
- `.pio/build/esp32s3/firmware.bin`
- `.pio/build/esp32s3/bootloader.bin`
- `.pio/build/esp32s3/partitions.bin`

### 2. 烧录固件（命令行）

```powershell
platformio run -t upload
```

如需手动指定串口:

```powershell
platformio run -t upload --upload-port COMx
```

### 3. 通过 GitHub Releases 下载现成固件
发布页:
- https://github.com/mgcyoulinjx/exlink/releases

下载 zip 后，解压得到 `firmware.bin / bootloader.bin / partitions.bin`，可使用 `esptool.py` 烧录:

```powershell
esptool.py --chip esp32s3 --port COMx --baud 921600 --before default_reset --after hard_reset write_flash -z ^
  0x0 bootloader.bin ^
  0x8000 partitions.bin ^
  0x10000 firmware.bin
```

### 4. 监视串口日志

```powershell
platformio device monitor -b 115200
```

## 开发说明

- 默认开发板环境: `esp32s3`（见 `platformio.ini`）
- 核心代码:
  - `src/main.cpp`
  - `src/ui.c`
  - `src/event.c`

## 贡献

1. Fork 本仓库
2. 新建分支（例如 `feat/xxx`）
3. 提交并推送代码
4. 提交 Pull Request
