#include "common.hpp"
#include "controllerstate.hpp"
#include "bleinputservice.hpp"
#include "bleinputsource.hpp"
#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gattc_api.h"
#include "esp_gatt_defs.h"
#include "esp_bt_main.h"

BleInputService *services[10];
uint8_t serviceCount = 0;

void registerService(BleInputService *s)
{
  services[serviceCount] = s;
  s->serviceIndex = serviceCount;
  serviceCount++;
}

void setScanning(uint8_t on)
{
  if (on)
  {
    esp_ble_gap_start_scanning(50);
  }
  else
  {
    esp_ble_gap_stop_scanning();
  }
}

class MagicSeeHid : public BleInputService
{
  ControllerState currentState;

public:
  MagicSeeHid(uint16_t u, BleInputSource *s) : BleInputService(u, s)
  {
    memset(&currentState, 0, sizeof(ControllerState));
  }
  void notification(esp_ble_gattc_cb_param_t::gattc_notify_evt_param &notify)
  {
    if (notify.value_len != 2)
    {
      Serial.printf("NOTIFICATION %d\n", notify.conn_id);
      Serial.printf("%d bytes: ", notify.value_len);
      for (int i = 0; i < notify.value_len; i++)
        Serial.printf("%02X ", notify.value[i]);
      Serial.println();
      return;
    }
    ControllerState newState = currentState;
    uint32_t time = millis();
    memcpy(&newState.currentButtons, notify.value, 2);
    newState.changedButtons = static_cast<ControllerButtons>
      (newState.currentButtons ^ currentState.currentButtons);
    currentState = newState;
    controllerState.poke(newState);
  }
};

class MagicSeeSource : public BleInputSource
{
  MagicSeeHid hid;
  BleInputService batt;

public:
  uint8_t shouldConnect(
      std::string deviceName,
      esp_ble_gap_cb_param_t::ble_scan_result_evt_param &device)
  {
    const std::string controllerName = "Magicsee R1       ";
    return controllerName == deviceName;
  }
  MagicSeeSource() : BleInputSource(),
                     hid(ESP_GATT_UUID_HID_SVC, this),
                     batt(ESP_GATT_UUID_BATTERY_SERVICE_SVC, this)
  {
  }
};

void gapEvent(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);
void gattcEvent(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param);

class BleInputManager : public Manager
{
  void *pBLEScan;
  MagicSeeSource controller;

public:
  const char *name()
  {
    return "InputManager";
  }
  BleInputManager() : pBLEScan(NULL)
  {
  }
  void setup()
  {
    btStart();
    esp_bluedroid_init();
    esp_bluedroid_enable();
    esp_ble_gap_register_callback(gapEvent);
    esp_ble_gattc_register_callback(gattcEvent);
    for (uint8_t i = 0; i < serviceCount; i++)
    {
      esp_ble_gattc_app_register(i);
    }

    esp_ble_scan_params_t scanParams = {
        .scan_type = BLE_SCAN_TYPE_ACTIVE,
        .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
        .scan_filter_policy = BLE_SCAN_FILTER_ALLOW_ALL,
        .scan_interval = 0x50,
        .scan_window = 0x30};
    esp_ble_gap_set_scan_params(&scanParams);
  }
  void foundDevice(esp_ble_gap_cb_param_t::ble_scan_result_evt_param &device)
  {
    uint8_t *deviceName = NULL;
    uint8_t deviceNameLength = 0;
    deviceName = esp_ble_resolve_adv_data(
        device.ble_adv,
        ESP_BLE_AD_TYPE_NAME_CMPL,
        &deviceNameLength);
    std::string name((char *)deviceName, deviceNameLength);
    if (controller.shouldConnect(name, device))
    {
      setScanning(false);
      controller.connect(device);
    }
  }
  void loop(){}
};

// Services Available on VR Box
#define GATT_UUID_GENERIC_ACCESS 0x1800    // device name, appearance, peripheral preferred connection parameters
#define GATT_UUID_GENERIC_ATTRIBUTE 0x1801 // generic attribute

/*** Mode 3:
 * First byte: Buttons
 * Triggers top: 80 bottom: 40
 * A: 02 B: 08 C: 01 D: 10
 * Second byte: Cursor position:
 * 00 10 20
 * 40 50 60
 * 80 90 A0
 */

/***  In short, keep mode B and C.
 *          | Mode A | Mode B | Mode C | Mode D
 * ---------+--------+--------+--------+------
 * Top tgr  |  02 50 | 80 00  | 80 00  |
 * Bot tgr  |  01 50 | 40 00  | 40 00  |
 * tgr up   |  00 50 | (n/a)) | (n/a)
 * Btn A    |  04 00 | 10 02  | 02 00  |
 * Btn B    |    XX  | 01 00  | 08 00  |
 * Btn C    |  02 00 | 08 01  | 01 00  |
 * Btn D    |  01 00 | 02 00  | 10 00  |
 * Up+Left  |    ?   | 00 00  |  <-    |
 * Up       |  01 00 | 00 10  |  <-    |
 * Up+Right |    ?   | 00 20  |  <-    |
 * Right    |  02 00 | 00 40  |  <-    |
 * Center   |  00 00 | 00 50  |  <-    |
 * Left     |  20 00 | 00 60  |  <-    |
 * Dn+left  |    ?   | 00 80  |  <-    |
 * Down     |  02 00 | 00 90  |  <-    |
 * Dn+right |    ?   | 00 A0  |  <-    |
 So the existance of 0400 or 2000 means mode A.
*/

BleInputManager inputManager;

void scanNextService(BleInputSource *source, uint8_t startingIndex)
{
  uint8_t index = startingIndex;
  while (index < serviceCount)
  {
    if (services[index]->source == source)
    {
      esp_ble_gattc_open(services[index]->clientNumber, source->address, BLE_ADDR_TYPE_PUBLIC, true);
      return;
    }
    index++;
  }
}

BleInputService *serviceFor(uint16_t s)
{
  for (uint8_t i = 0; i < serviceCount; i++)
  {
    BleInputService *service = services[i];
    if (service->clientNumber == s)
    {
      return service;
    }
  }
  return NULL;
}

void gapEvent(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
  switch (event)
  {
  case ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT:
    //Parameters set. Start scanning
    setScanning(true);
    break;

  case ESP_GAP_BLE_SCAN_START_COMPLETE_EVT:
    //Starting up scan
    break;

  case ESP_GAP_BLE_SCAN_RESULT_EVT:
  {
    //Scan found something
    esp_ble_gap_cb_param_t *scan_result = (esp_ble_gap_cb_param_t *)param;
    switch (scan_result->scan_rst.search_evt)
    {
    case ESP_GAP_SEARCH_INQ_RES_EVT:
    {
      inputManager.foundDevice(scan_result->scan_rst);
      break;
    }

    case ESP_GAP_SEARCH_INQ_CMPL_EVT:
      // Scanning is off
      break;

    default:
      break;
    }
    break;
  }

  case ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT:
  {
    //Scan complete
    break;
  }

  case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
    //We finished advertising?
    break;
  default:
    break;
  }
}

void gattcEvent(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param)
{
  esp_ble_gattc_cb_param_t *data = (esp_ble_gattc_cb_param_t *)param;

  if (event == ESP_GATTC_REG_EVT)
  {
    services[param->reg.app_id]->clientNumber = gattc_if;
  }

  BleInputService *service = serviceFor(gattc_if);
  if (service == NULL)
  {
    return;
  }

  switch (event)
  {
    // this event occurs when the connection is set up
  case ESP_GATTC_OPEN_EVT:
  {
    service->openedConnection(data->open);
    break;
  }

  case ESP_GATTC_SEARCH_RES_EVT:
    service->foundService(data->search_res);
    break;

  case ESP_GATTC_SEARCH_CMPL_EVT:
    service->searchComplete(data->search_cmpl);
    scanNextService(service->source, service->serviceIndex + 1);
    break;

  case ESP_GATTC_DISCONNECT_EVT: //Now what?
    service->disconnected(data->disconnect);
    // service->reset();
    // service->source->reset();
    break;

  case ESP_GATTC_NOTIFY_EVT:
    service->notification(data->notify);
    break;

  default:
    break;
  }
}
