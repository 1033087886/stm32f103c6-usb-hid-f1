# stm32f103c6-usb-f1

STM32F103C6T6 + PlatformIO + STM32Cube HAL 的 USB HID 键盘示例。

## 功能

- 系统时钟 `72MHz`（HSE 8MHz -> PLL x9）
- USB 时钟 `48MHz`（PLL / 1.5）
- 启动时执行 USB D+ 重枚举控制（PA12 拉低后释放）
- 设备枚举为标准 HID Keyboard
- 上电后延时 8 秒，自动发送一次 `F1 (0x3A)` 报告并发送释放报告

## 目录

- `platformio.ini`
- `include/main.h`
- `include/stm32f1xx_hal_conf.h`
- `include/usbd_desc.h`
- `include/usbd_hid.h`
- `include/usbd_conf.h`
- `include/usb_device.h`
- `src/main.c`
- `src/usbd_desc.c`
- `src/usbd_hid.c`
- `src/usbd_conf.c`
- `src/usb_device.c`
- `src/stm32f1xx_it.c`

## HID 报告格式

采用标准 8 字节键盘输入报告：

- Byte0: Modifier
- Byte1: Reserved
- Byte2~Byte7: 最多 6 个按键码

端点发送长度固定为 `8` 字节（`HID_EPIN_SIZE = 0x08`，`USBD_HID_REPORT_LENGTH = 0x08`）。

## 构建与下载

```bash
pio run
pio run -t upload
```
