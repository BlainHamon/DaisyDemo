#include "common.hpp"
#include "controllerstate.hpp"
#include "bleinputservice.hpp"
#include "bleinputsource.hpp"
#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gattc_api.h"
#include "esp_gatt_defs.h"
#include "esp_bt_main.h"


uint8_t BleInputSource::shouldConnect(
  std::string deviceName, 
  esp_ble_gap_cb_param_t::ble_scan_result_evt_param &device
) { return 0; };

void BleInputSource::connect(
      esp_ble_gap_cb_param_t::ble_scan_result_evt_param &device
) {
  memcpy(address, device.bda, sizeof(esp_bd_addr_t));
  scanNextService(this, 0);
}

void BleInputSource::reset() {
  if (connectionId != 0) {
    memset(address, 0, sizeof(esp_bd_addr_t));
    connectionId = 0;
    esp_ble_gap_start_scanning(50);
  }
}
