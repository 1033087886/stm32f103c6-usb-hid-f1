#ifndef __USBD_CONF_H
#define __USBD_CONF_H

#include "stm32f1xx.h"

#define USBD_MAX_NUM_INTERFACES               1U
#define USBD_MAX_NUM_CONFIGURATION             1U
#define USBD_MAX_STR_DESC_SIZ                 64U
#define USBD_SUPPORT_USER_STRING               0U
#define USBD_SELF_POWERED                      0U
#define USBD_DEBUG_LEVEL                       0U

#define DEVICE_FS                              0U

#define USBD_malloc         malloc
#define USBD_free           free
#define USBD_memset         memset
#define USBD_memcpy         memcpy
#define USBD_Delay          HAL_Delay

#endif
