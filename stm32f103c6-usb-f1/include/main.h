#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"

#define HID_KEY_F1            0x3AU
#define USB_REENUM_GPIO_PORT  GPIOA
#define USB_REENUM_GPIO_PIN   GPIO_PIN_12

void Error_Handler(void);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
