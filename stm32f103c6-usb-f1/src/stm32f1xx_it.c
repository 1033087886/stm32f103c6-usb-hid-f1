#include "main.h"
#include "stm32f1xx_hal.h"

extern PCD_HandleTypeDef hpcd_USB_FS;

void SysTick_Handler(void)
{
  /* HAL time base. */
  HAL_IncTick();
}

void USB_LP_CAN1_RX0_IRQHandler(void)
{
  /* USB low-priority interrupt entry. */
  HAL_PCD_IRQHandler(&hpcd_USB_FS);
}
