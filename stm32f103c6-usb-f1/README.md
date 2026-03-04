# STM32F103C6T6 USB HID Keyboard (PlatformIO + STM32Cube HAL)

本工程目标：
- 芯片：STM32F103C6T6
- 框架：STM32Cube HAL
- 功能：上电后等待 8 秒，自动发送一次 F1（HID KeyCode `0x3A`），随后释放按键

## 工程结构

```
stm32f103c6-usb-f1/
├── platformio.ini
├── include/
│   ├── main.h
│   ├── stm32f1xx_hal_conf.h
│   ├── usbd_desc.h
│   ├── usbd_hid.h
│   └── usb_device.h
└── src/
    ├── main.c
    ├── stm32f1xx_it.c
    ├── usbd_conf.c
    ├── usbd_desc.c
    ├── usbd_hid.c
    └── usb_device.c
```

## 说明

- 使用了标准 8 字节键盘输入报告：`[modifier][reserved][key1..key6]`
- `main.c` 中在 `MX_USB_DEVICE_Init()` 后 `HAL_Delay(8000)`，发送 F1 后立即发送全零报告释放。
- USB 时钟配置为 `PLL/1.5 = 48MHz`。

## 硬件提示

- 默认 USB FS 使用 `PA11(D-)` / `PA12(D+)`
- 若需强制重枚举，`main.c` 中已提供 `USB_ReEnumerate()` 逻辑（短暂拉低 D+）

## 编译

```bash
pio run
```

## 烧录

```bash
pio run -t upload
```
