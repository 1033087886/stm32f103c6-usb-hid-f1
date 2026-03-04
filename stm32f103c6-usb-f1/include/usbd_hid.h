#ifndef __USBD_HID_H
#define __USBD_HID_H

#include "usbd_ioreq.h"

#define HID_EPIN_ADDR                 0x81U
#define HID_EPIN_SIZE                 0x08U

#define USB_HID_CONFIG_DESC_SIZ       34U
#define USB_HID_DESC_SIZ              9U
#define HID_KEYBOARD_REPORT_DESC_SIZE 63U

#define HID_DESCRIPTOR_TYPE           0x21U
#define HID_REPORT_DESC               0x22U

#define HID_FS_BINTERVAL              0x0AU

typedef enum
{
  HID_IDLE = 0,
  HID_BUSY,
} HID_StateTypeDef;

typedef struct _USBD_HID_HandleTypeDef
{
  uint32_t Protocol;
  uint32_t IdleState;
  uint32_t AltSetting;
  HID_StateTypeDef state;
} USBD_HID_HandleTypeDef;

extern USBD_ClassTypeDef USBD_HID;
#define USBD_HID_CLASS &USBD_HID

uint8_t USBD_HID_SendReport(USBD_HandleTypeDef *pdev, uint8_t *report, uint16_t len);
uint32_t USBD_HID_GetPollingInterval(USBD_HandleTypeDef *pdev);

#endif
