# Exlink

Exlink is an ESP32-S3 multi-tool project with an LVGL UI, including DC POWER, PWM, UART Helper, I2C Scan, DSO, BLE UART, and Frequency Counter.

## GitHub Setup Guide

### 1. Install Git
Windows:
1. Download and install Git: https://git-scm.com/download/win
2. Verify in PowerShell:

```powershell
git --version
```

### 2. Clone this repository

```powershell
git clone https://github.com/mgcyoulinjx/exlink.git
cd exlink
```

### 3. Install PlatformIO
Option A (recommended, VS Code):
1. Install VS Code: https://code.visualstudio.com/
2. Install extension: `PlatformIO IDE`
3. Open this project folder

Option B (CLI):

```powershell
python -m pip install -U platformio
platformio --version
```

## Usage

### 1. Build firmware

```powershell
platformio run
```

Build outputs:
- `.pio/build/esp32s3/firmware.bin`
- `.pio/build/esp32s3/bootloader.bin`
- `.pio/build/esp32s3/partitions.bin`

### 2. Flash firmware

```powershell
platformio run -t upload
```

Specify serial port if needed:

```powershell
platformio run -t upload --upload-port COMx
```

### 3. Download prebuilt firmware from GitHub Releases
Releases page:
- https://github.com/mgcyoulinjx/exlink/releases

After extracting the release zip, flash with `esptool.py`:

```powershell
esptool.py --chip esp32s3 --port COMx --baud 921600 --before default_reset --after hard_reset write_flash -z ^
  0x0 bootloader.bin ^
  0x8000 partitions.bin ^
  0x10000 firmware.bin
```

### 4. Serial monitor

```powershell
platformio device monitor -b 115200
```

## Development Notes

- Default environment: `esp32s3` (`platformio.ini`)
- Main source files:
  - `src/main.cpp`
  - `src/ui.c`
  - `src/event.c`

## Contributing

1. Fork this repo
2. Create a branch (for example `feat/xxx`)
3. Commit and push changes
4. Open a Pull Request
