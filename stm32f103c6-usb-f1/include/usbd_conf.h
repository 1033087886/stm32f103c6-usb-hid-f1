#ifndef __USBD_CONF_H
#define __USBD_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <string.h>

#include "stm32f1xx.h"
#include "stm32f1xx_hal.h"

#define USBD_MAX_NUM_INTERFACES         1U
#define USBD_MAX_NUM_CONFIGURATION       1U
#define USBD_MAX_STR_DESC_SIZ            64U
#define USBD_SUPPORT_USER_STRING         0U
#define USBD_SELF_POWERED                0U
#define USBD_DEBUG_LEVEL                 0U

#define DEVICE_FS                        0U

#define USBD_malloc                      USBD_static_malloc
#define USBD_free                        USBD_static_free
#define USBD_memset                      memset
#define USBD_memcpy                      memcpy
#define USBD_Delay                       HAL_Delay

void *USBD_static_malloc(uint32_t size);
void USBD_static_free(void *p);

#if (USBD_DEBUG_LEVEL > 0U)
#define USBD_UsrLog(...)                 do { } while (0)
#define USBD_ErrLog(...)                 do { } while (0)
#define USBD_DbgLog(...)                 do { } while (0)
#else
#define USBD_UsrLog(...)                 do { } while (0)
#define USBD_ErrLog(...)                 do { } while (0)
#define USBD_DbgLog(...)                 do { } while (0)
#endif

#ifdef __cplusplus
}
#endif

#endif /* __USBD_CONF_H */
