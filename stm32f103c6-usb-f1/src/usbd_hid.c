#include "usbd_hid.h"
#include "usbd_ctlreq.h"

static uint8_t USBD_HID_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t USBD_HID_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t USBD_HID_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static uint8_t *USBD_HID_GetFSCfgDesc(uint16_t *length);
static uint8_t *USBD_HID_GetDeviceQualifierDesc(uint16_t *length);

__ALIGN_BEGIN static uint8_t HID_KEYBOARD_ReportDesc[HID_KEYBOARD_REPORT_DESC_SIZE] __ALIGN_END =
{
  0x05, 0x01,
  0x09, 0x06,
  0xA1, 0x01,
  0x05, 0x07,
  0x19, 0xE0,
  0x29, 0xE7,
  0x15, 0x00,
  0x25, 0x01,
  0x75, 0x01,
  0x95, 0x08,
  0x81, 0x02,
  0x95, 0x01,
  0x75, 0x08,
  0x81, 0x01,
  0x95, 0x06,
  0x75, 0x08,
  0x15, 0x00,
  0x25, 0x65,
  0x05, 0x07,
  0x19, 0x00,
  0x29, 0x65,
  0x81, 0x00,
  0xC0
};

__ALIGN_BEGIN static uint8_t USBD_HID_CfgFSDesc[USB_HID_CONFIG_DESC_SIZ] __ALIGN_END =
{
  0x09, USB_DESC_TYPE_CONFIGURATION, USB_HID_CONFIG_DESC_SIZ, 0x00,
  0x01, 0x01, 0x00, 0xA0, 0x32,

  0x09, USB_DESC_TYPE_INTERFACE, 0x00, 0x00, 0x01,
  0x03, 0x01, 0x01, 0x00,

  0x09, HID_DESCRIPTOR_TYPE, 0x11, 0x01, 0x00, 0x01,
  0x22, HID_KEYBOARD_REPORT_DESC_SIZE, 0x00,

  0x07, USB_DESC_TYPE_ENDPOINT, HID_EPIN_ADDR, 0x03,
  HID_EPIN_SIZE, 0x00, HID_FS_BINTERVAL,
};

__ALIGN_BEGIN static uint8_t USBD_HID_DeviceQualifierDesc[0x0A] __ALIGN_END =
{
  0x0A, USB_DESC_TYPE_DEVICE_QUALIFIER, 0x00, 0x02,
  0x00, 0x00, 0x00, 0x40, 0x01, 0x00,
};

USBD_ClassTypeDef USBD_HID =
{
  USBD_HID_Init,
  USBD_HID_DeInit,
  USBD_HID_Setup,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  USBD_HID_GetFSCfgDesc,
  USBD_HID_GetFSCfgDesc,
  USBD_HID_GetFSCfgDesc,
  USBD_HID_GetDeviceQualifierDesc,
};

static uint8_t USBD_HID_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
  (void)cfgidx;
  USBD_LL_OpenEP(pdev, HID_EPIN_ADDR, USBD_EP_TYPE_INTR, HID_EPIN_SIZE);
  pdev->pClassData = USBD_malloc(sizeof(USBD_HID_HandleTypeDef));
  if (pdev->pClassData == NULL) return USBD_FAIL;
  ((USBD_HID_HandleTypeDef *)pdev->pClassData)->state = HID_IDLE;
  return USBD_OK;
}

static uint8_t USBD_HID_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
  (void)cfgidx;
  USBD_LL_CloseEP(pdev, HID_EPIN_ADDR);
  if (pdev->pClassData != NULL) {
    USBD_free(pdev->pClassData);
    pdev->pClassData = NULL;
  }
  return USBD_OK;
}

static uint8_t USBD_HID_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
  uint16_t len = 0;
  uint8_t *pbuf = NULL;
  USBD_HID_HandleTypeDef *hhid = (USBD_HID_HandleTypeDef *)pdev->pClassData;

  switch (req->bmRequest & USB_REQ_TYPE_MASK)
  {
    case USB_REQ_TYPE_CLASS:
      switch (req->bRequest)
      {
        case 0x0B:
          hhid->Protocol = (uint8_t)(req->wValue);
          break;
        case 0x03:
          hhid->Protocol = (uint8_t)(req->wValue);
          break;
        case 0x0A:
          hhid->IdleState = (uint8_t)(req->wValue >> 8);
          break;
        case 0x02:
          USBD_CtlSendData(pdev, (uint8_t *)&hhid->IdleState, 1);
          break;
        default:
          USBD_CtlError(pdev, req);
          return USBD_FAIL;
      }
      break;

    case USB_REQ_TYPE_STANDARD:
      switch (req->bRequest)
      {
        case USB_REQ_GET_DESCRIPTOR:
          if ((req->wValue >> 8) == HID_REPORT_DESC)
          {
            len = MIN(HID_KEYBOARD_REPORT_DESC_SIZE, req->wLength);
            pbuf = HID_KEYBOARD_ReportDesc;
          }
          else if ((req->wValue >> 8) == HID_DESCRIPTOR_TYPE)
          {
            pbuf = USBD_HID_CfgFSDesc + 0x12;
            len = MIN(USB_HID_DESC_SIZ, req->wLength);
          }
          USBD_CtlSendData(pdev, pbuf, len);
          break;

        case USB_REQ_GET_INTERFACE:
          USBD_CtlSendData(pdev, (uint8_t *)&hhid->AltSetting, 1);
          break;

        case USB_REQ_SET_INTERFACE:
          hhid->AltSetting = (uint8_t)(req->wValue);
          break;
      }
      break;
  }

  return USBD_OK;
}

uint8_t USBD_HID_SendReport(USBD_HandleTypeDef *pdev, uint8_t *report, uint16_t len)
{
  USBD_HID_HandleTypeDef *hhid = (USBD_HID_HandleTypeDef *)pdev->pClassData;
  if (pdev->dev_state == USBD_STATE_CONFIGURED)
  {
    if (hhid->state == HID_IDLE)
    {
      hhid->state = HID_BUSY;
      USBD_LL_Transmit(pdev, HID_EPIN_ADDR, report, len);
    }
  }
  return USBD_OK;
}

uint32_t USBD_HID_GetPollingInterval(USBD_HandleTypeDef *pdev)
{
  (void)pdev;
  return HID_FS_BINTERVAL;
}

static uint8_t *USBD_HID_GetFSCfgDesc(uint16_t *length)
{
  *length = sizeof(USBD_HID_CfgFSDesc);
  return USBD_HID_CfgFSDesc;
}

static uint8_t *USBD_HID_GetDeviceQualifierDesc(uint16_t *length)
{
  *length = sizeof(USBD_HID_DeviceQualifierDesc);
  return USBD_HID_DeviceQualifierDesc;
}

void USBD_HID_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
  (void)epnum;
  ((USBD_HID_HandleTypeDef *)pdev->pClassData)->state = HID_IDLE;
}
