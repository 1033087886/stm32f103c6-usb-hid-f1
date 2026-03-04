#include "main.h"
#include "usb_device.h"
#include "usbd_hid.h"

static void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void USB_ReEnumerate(void);
static HAL_StatusTypeDef HID_SendReportWithTimeout(const uint8_t report[USBD_HID_REPORT_LENGTH], uint32_t timeout_ms);
static HAL_StatusTypeDef HID_SendSingleKey(uint8_t keycode);

extern USBD_HandleTypeDef hUsbDeviceFS;

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();

  USB_ReEnumerate();
  MX_USB_DEVICE_Init();

  /* Requirement: delay 8 seconds after power-up, then send one F1 keystroke. */
  HAL_Delay(8000U);
  (void)HID_SendSingleKey(HID_KEY_F1);

  while (1)
  {
    HAL_Delay(1000U);
  }
}

static HAL_StatusTypeDef HID_SendReportWithTimeout(const uint8_t report[USBD_HID_REPORT_LENGTH], uint32_t timeout_ms)
{
  uint32_t start = HAL_GetTick();

  while ((HAL_GetTick() - start) < timeout_ms)
  {
    if (hUsbDeviceFS.dev_state == USBD_STATE_CONFIGURED)
    {
      if (USBD_HID_SendReport(&hUsbDeviceFS, (uint8_t *)report, USBD_HID_REPORT_LENGTH) == USBD_OK)
      {
        return HAL_OK;
      }
    }

    HAL_Delay(1U);
  }

  return HAL_TIMEOUT;
}

static HAL_StatusTypeDef HID_SendSingleKey(uint8_t keycode)
{
  uint8_t key_press[USBD_HID_REPORT_LENGTH] = {0U};
  uint8_t key_release[USBD_HID_REPORT_LENGTH] = {0U};

  key_press[2] = keycode;

  if (HID_SendReportWithTimeout(key_press, 2000U) != HAL_OK)
  {
    return HAL_ERROR;
  }

  HAL_Delay(20U);

  if (HID_SendReportWithTimeout(key_release, 2000U) != HAL_OK)
  {
    return HAL_ERROR;
  }

  HAL_Delay(20U);
  return HAL_OK;
}

static void USB_ReEnumerate(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Force D+ low briefly so the host sees a disconnect before USB init. */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  HAL_GPIO_WritePin(USB_REENUM_GPIO_PORT, USB_REENUM_GPIO_PIN, GPIO_PIN_RESET);

  GPIO_InitStruct.Pin = USB_REENUM_GPIO_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(USB_REENUM_GPIO_PORT, &GPIO_InitStruct);

  HAL_Delay(40U);

  GPIO_InitStruct.Pin = GPIO_PIN_11 | GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  HAL_Delay(10U);
}

static void MX_GPIO_Init(void)
{
  __HAL_RCC_AFIO_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
}

static void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}

#if (USE_FULL_ASSERT == 1U)
void assert_failed(uint8_t *file, uint32_t line)
{
  (void)file;
  (void)line;
  Error_Handler();
}
#endif
