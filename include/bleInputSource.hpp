#ifndef __bleinputsource_hpp
#define __bleinputsource_hpp
#include <Arduino.h>
#include <stdint.h>
#include <string>
#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gattc_api.h"
#include "esp_gatt_defs.h"
#include "esp_bt_main.h"

class BleInputService;

class BleInputSource {
  esp_bd_addr_t address;
  uint16_t connectionId;
public:
  BleInputSource() {}
  virtual uint8_t shouldConnect(
      std::string deviceName,
      esp_ble_gap_cb_param_t::ble_scan_result_evt_param &device);
  void connect(
      esp_ble_gap_cb_param_t::ble_scan_result_evt_param &device);
  void reset();
  virtual void disconnected(){};
  friend class BleInputService;
  friend void scanNextService(BleInputSource *source, uint8_t startingIndex);
};

#endif