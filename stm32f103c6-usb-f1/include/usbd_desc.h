#ifndef __USBD_DESC_H
#define __USBD_DESC_H

#include "usbd_def.h"

#define USBD_VID                     0x1209
#define USBD_PID_FS                  0x0001
#define USBD_LANGID_STRING           0x0409
#define USBD_MANUFACTURER_STRING     "OpenClaw"
#define USBD_PRODUCT_STRING_FS       "STM32 HID Keyboard"
#define USBD_CONFIGURATION_STRING_FS "HID Config"
#define USBD_INTERFACE_STRING_FS     "HID Interface"

extern USBD_DescriptorsTypeDef FS_Desc;

uint8_t *USBD_FS_DeviceDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
uint8_t *USBD_FS_LangIDStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
uint8_t *USBD_FS_ManufacturerStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
uint8_t *USBD_FS_ProductStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
uint8_t *USBD_FS_SerialStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
uint8_t *USBD_FS_ConfigStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
uint8_t *USBD_FS_InterfaceStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);

#endif
