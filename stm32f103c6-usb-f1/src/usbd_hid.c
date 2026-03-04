#include "usbd_hid.h"
#include "usbd_ctlreq.h"

#define HID_REQ_SET_PROTOCOL    0x0BU
#define HID_REQ_GET_PROTOCOL    0x03U
#define HID_REQ_SET_IDLE        0x0AU
#define HID_REQ_GET_IDLE        0x02U

static uint8_t USBD_HID_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t USBD_HID_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t USBD_HID_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static uint8_t USBD_HID_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t *USBD_HID_GetHSCfgDesc(uint16_t *length);
static uint8_t *USBD_HID_GetFSCfgDesc(uint16_t *length);
static uint8_t *USBD_HID_GetOtherSpeedCfgDesc(uint16_t *length);
static uint8_t *USBD_HID_GetDeviceQualifierDesc(uint16_t *length);

__ALIGN_BEGIN static uint8_t HID_KeyboardReportDesc[HID_KEYBOARD_REPORT_DESC_SIZE] __ALIGN_END = {
  0x05U, 0x01U,
  0x09U, 0x06U,
  0xA1U, 0x01U,
  0x05U, 0x07U,
  0x19U, 0xE0U,
  0x29U, 0xE7U,
  0x15U, 0x00U,
  0x25U, 0x01U,
  0x75U, 0x01U,
  0x95U, 0x08U,
  0x81U, 0x02U,
  0x95U, 0x01U,
  0x75U, 0x08U,
  0x81U, 0x01U,
  0x95U, 0x05U,
  0x75U, 0x01U,
  0x05U, 0x08U,
  0x19U, 0x01U,
  0x29U, 0x05U,
  0x91U, 0x02U,
  0x95U, 0x01U,
  0x75U, 0x03U,
  0x91U, 0x01U,
  0x95U, 0x06U,
  0x75U, 0x08U,
  0x15U, 0x00U,
  0x25U, 0x65U,
  0x05U, 0x07U,
  0x19U, 0x00U,
  0x29U, 0x65U,
  0x81U, 0x00U,
  0xC0U
};

__ALIGN_BEGIN static uint8_t USBD_HID_CfgDesc[USB_HID_CONFIG_DESC_SIZ] __ALIGN_END = {
  0x09U,
  USB_DESC_TYPE_CONFIGURATION,
  LOBYTE(USB_HID_CONFIG_DESC_SIZ),
  HIBYTE(USB_HID_CONFIG_DESC_SIZ),
  0x01U,
  0x01U,
  0x00U,
  0x80U,
  0x32U,

  0x09U,
  USB_DESC_TYPE_INTERFACE,
  0x00U,
  0x00U,
  0x01U,
  0x03U,
  0x01U,
  0x01U,
  0x00U,

  0x09U,
  HID_DESCRIPTOR_TYPE,
  0x11U,
  0x01U,
  0x00U,
  0x01U,
  HID_REPORT_DESC,
  LOBYTE(HID_KEYBOARD_REPORT_DESC_SIZE),
  HIBYTE(HID_KEYBOARD_REPORT_DESC_SIZE),

  0x07U,
  USB_DESC_TYPE_ENDPOINT,
  HID_EPIN_ADDR,
  0x03U,
  LOBYTE(HID_EPIN_SIZE),
  HIBYTE(HID_EPIN_SIZE),
  HID_FS_BINTERVAL
};

__ALIGN_BEGIN static uint8_t USBD_HID_DeviceQualifierDesc[0x0AU] __ALIGN_END = {
  0x0AU,
  USB_DESC_TYPE_DEVICE_QUALIFIER,
  0x00U,
  0x02U,
  0x00U,
  0x00U,
  0x00U,
  0x40U,
  0x01U,
  0x00U
};

USBD_ClassTypeDef USBD_HID = {
  .Init = USBD_HID_Init,
  .DeInit = USBD_HID_DeInit,
  .Setup = USBD_HID_Setup,
  .EP0_TxSent = NULL,
  .EP0_RxReady = NULL,
  .DataIn = USBD_HID_DataIn,
  .DataOut = NULL,
  .SOF = NULL,
  .IsoINIncomplete = NULL,
  .IsoOUTIncomplete = NULL,
  .GetHSConfigDescriptor = USBD_HID_GetHSCfgDesc,
  .GetFSConfigDescriptor = USBD_HID_GetFSCfgDesc,
  .GetOtherSpeedConfigDescriptor = USBD_HID_GetOtherSpeedCfgDesc,
  .GetDeviceQualifierDescriptor = USBD_HID_GetDeviceQualifierDesc
};

static uint8_t USBD_HID_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
  USBD_HID_HandleTypeDef *hhid;
  (void)cfgidx;

  if (USBD_LL_OpenEP(pdev, HID_EPIN_ADDR, USBD_EP_TYPE_INTR, HID_EPIN_SIZE) != USBD_OK)
  {
    return USBD_FAIL;
  }

  pdev->pClassData = USBD_malloc(sizeof(USBD_HID_HandleTypeDef));
  if (pdev->pClassData == NULL)
  {
    (void)USBD_LL_CloseEP(pdev, HID_EPIN_ADDR);
    return USBD_FAIL;
  }

  hhid = (USBD_HID_HandleTypeDef *)pdev->pClassData;
  hhid->Protocol = 0U;
  hhid->IdleState = 0U;
  hhid->AltSetting = 0U;
  hhid->state = HID_IDLE;

  return USBD_OK;
}

static uint8_t USBD_HID_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
  (void)cfgidx;

  (void)USBD_LL_CloseEP(pdev, HID_EPIN_ADDR);

  if (pdev->pClassData != NULL)
  {
    USBD_free(pdev->pClassData);
    pdev->pClassData = NULL;
  }

  return USBD_OK;
}

static uint8_t USBD_HID_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
  uint16_t len;
  uint8_t *pbuf;
  USBD_HID_HandleTypeDef *hhid;
  uint8_t ifalt;
  uint16_t status_info;

  hhid = (USBD_HID_HandleTypeDef *)pdev->pClassData;
  len = 0U;
  pbuf = NULL;

  switch (req->bmRequest & USB_REQ_TYPE_MASK)
  {
    case USB_REQ_TYPE_CLASS:
      switch (req->bRequest)
      {
        case HID_REQ_SET_PROTOCOL:
          if (hhid != NULL)
          {
            hhid->Protocol = (uint8_t)(req->wValue);
          }
          break;

        case HID_REQ_GET_PROTOCOL:
          if (hhid != NULL)
          {
            USBD_CtlSendData(pdev, (uint8_t *)&hhid->Protocol, 1U);
          }
          break;

        case HID_REQ_SET_IDLE:
          if (hhid != NULL)
          {
            hhid->IdleState = (uint8_t)(req->wValue >> 8);
          }
          break;

        case HID_REQ_GET_IDLE:
          if (hhid != NULL)
          {
            USBD_CtlSendData(pdev, (uint8_t *)&hhid->IdleState, 1U);
          }
          break;

        default:
          USBD_CtlError(pdev, req);
          return USBD_FAIL;
      }
      break;

    case USB_REQ_TYPE_STANDARD:
      switch (req->bRequest)
      {
        case USB_REQ_GET_STATUS:
          if (pdev->dev_state == USBD_STATE_CONFIGURED)
          {
            status_info = 0U;
            USBD_CtlSendData(pdev, (uint8_t *)&status_info, 2U);
          }
          else
          {
            USBD_CtlError(pdev, req);
            return USBD_FAIL;
          }
          break;

        case USB_REQ_GET_DESCRIPTOR:
          if ((req->wValue >> 8) == HID_REPORT_DESC)
          {
            len = MIN(HID_KEYBOARD_REPORT_DESC_SIZE, req->wLength);
            pbuf = HID_KeyboardReportDesc;
          }
          else if ((req->wValue >> 8) == HID_DESCRIPTOR_TYPE)
          {
            len = MIN(USB_HID_DESC_SIZ, req->wLength);
            pbuf = USBD_HID_CfgDesc + 18U;
          }
          else
          {
            USBD_CtlError(pdev, req);
            return USBD_FAIL;
          }

          USBD_CtlSendData(pdev, pbuf, len);
          break;

        case USB_REQ_GET_INTERFACE:
          if (hhid != NULL)
          {
            ifalt = (uint8_t)hhid->AltSetting;
            USBD_CtlSendData(pdev, &ifalt, 1U);
          }
          break;

        case USB_REQ_SET_INTERFACE:
          if (hhid != NULL)
          {
            hhid->AltSetting = (uint8_t)(req->wValue);
          }
          break;

        default:
          break;
      }
      break;

    default:
      USBD_CtlError(pdev, req);
      return USBD_FAIL;
  }

  return USBD_OK;
}

static uint8_t USBD_HID_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
  USBD_HID_HandleTypeDef *hhid;
  (void)epnum;

  hhid = (USBD_HID_HandleTypeDef *)pdev->pClassData;
  if (hhid != NULL)
  {
    hhid->state = HID_IDLE;
  }

  return USBD_OK;
}

uint8_t USBD_HID_SendReport(USBD_HandleTypeDef *pdev, uint8_t *report, uint16_t len)
{
  USBD_HID_HandleTypeDef *hhid;

  if ((pdev == NULL) || (report == NULL) || (len != USBD_HID_REPORT_LENGTH))
  {
    return USBD_FAIL;
  }

  hhid = (USBD_HID_HandleTypeDef *)pdev->pClassData;
  if (hhid == NULL)
  {
    return USBD_FAIL;
  }

  if (pdev->dev_state != USBD_STATE_CONFIGURED)
  {
    return USBD_FAIL;
  }

  if (hhid->state != HID_IDLE)
  {
    return USBD_BUSY;
  }

  hhid->state = HID_BUSY;
  return USBD_LL_Transmit(pdev, HID_EPIN_ADDR, report, USBD_HID_REPORT_LENGTH);
}

uint32_t USBD_HID_GetPollingInterval(USBD_HandleTypeDef *pdev)
{
  (void)pdev;
  return HID_FS_BINTERVAL;
}

static uint8_t *USBD_HID_GetHSCfgDesc(uint16_t *length)
{
  *length = sizeof(USBD_HID_CfgDesc);
  return USBD_HID_CfgDesc;
}

static uint8_t *USBD_HID_GetFSCfgDesc(uint16_t *length)
{
  *length = sizeof(USBD_HID_CfgDesc);
  return USBD_HID_CfgDesc;
}

static uint8_t *USBD_HID_GetOtherSpeedCfgDesc(uint16_t *length)
{
  *length = sizeof(USBD_HID_CfgDesc);
  return USBD_HID_CfgDesc;
}

static uint8_t *USBD_HID_GetDeviceQualifierDesc(uint16_t *length)
{
  *length = sizeof(USBD_HID_DeviceQualifierDesc);
  return USBD_HID_DeviceQualifierDesc;
}
