#ifndef __bleinputservice_hpp
#define __bleinputservice_hpp
#include <stdint.h>
#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gattc_api.h"
#include "esp_gatt_defs.h"
#include "esp_bt_main.h"

class BleInputSource;

class BleInputService{
  BleInputSource *source;
  uint16_t startHandle;
  uint16_t endHandle;
  uint16_t uuid;
  uint8_t serviceIndex;

public:
  uint16_t clientNumber;
  BleInputService(uint16_t u, BleInputSource *s = NULL);
  void openedConnection(esp_ble_gattc_cb_param_t::gattc_open_evt_param &openEvent);
  virtual uint16_t requestedServiceUUID();
  void foundService(esp_ble_gattc_cb_param_t::gattc_search_res_evt_param &service);
  virtual uint8_t shouldUseService(esp_ble_gattc_cb_param_t::gattc_search_res_evt_param &service);
  virtual uint8_t wantNotifications(esp_gattc_char_elem_t &element);
  void searchComplete(esp_ble_gattc_cb_param_t::gattc_search_cmpl_evt_param &completion);
  void reset();
  virtual void disconnected(esp_ble_gattc_cb_param_t::gattc_disconnect_evt_param &disconnect);
  virtual void notification(esp_ble_gattc_cb_param_t::gattc_notify_evt_param &notify);

  friend class BleInputSource;
  friend void scanNextService(BleInputSource *source, uint8_t startingIndex);
  friend void gattcEvent(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param);
  friend void registerService(BleInputService *s);
};

#endif