#include "stm32f1xx_hal.h"

extern PCD_HandleTypeDef hpcd_USB_FS;

void SysTick_Handler(void)
{
  HAL_IncTick();
}

void USB_LP_CAN1_RX0_IRQHandler(void)
{
  HAL_PCD_IRQHandler(&hpcd_USB_FS);
}
