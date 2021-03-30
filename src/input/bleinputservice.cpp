#include "common.hpp"
#include "controllerstate.hpp"
#include "bleinputservice.hpp"
#include "bleinputsource.hpp"
#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gattc_api.h"
#include "esp_gatt_defs.h"
#include "esp_bt_main.h"

BleInputService::BleInputService(uint16_t u, BleInputSource *s) : source(s), uuid(u) {
  registerService(this);
}

void BleInputService::openedConnection(esp_ble_gattc_cb_param_t::gattc_open_evt_param &openEvent) {
  source->connectionId = openEvent.conn_id;
  memcpy(&(source->address), openEvent.remote_bda, sizeof(esp_bd_addr_t));
  uint16_t serviceNum = requestedServiceUUID();
  esp_bt_uuid_t uuid;
  uuid.len = ESP_UUID_LEN_16;
  uuid.uuid.uuid16 = serviceNum;
  esp_ble_gattc_search_service(
      clientNumber,
      source->connectionId,
      serviceNum ? &uuid : NULL);
}

uint16_t BleInputService::requestedServiceUUID() { return uuid; }
void BleInputService::foundService(esp_ble_gattc_cb_param_t::gattc_search_res_evt_param &service) {
  if (shouldUseService(service)) {
    startHandle = service.start_handle;
    endHandle = service.end_handle;
  }
}
uint8_t BleInputService::shouldUseService(esp_ble_gattc_cb_param_t::gattc_search_res_evt_param &service) {
  return (service.srvc_id.uuid.len == ESP_UUID_LEN_16) &&
          (service.srvc_id.uuid.uuid.uuid16 == requestedServiceUUID());
}
uint8_t BleInputService::wantNotifications(esp_gattc_char_elem_t &element) {
  return 1;
}
void BleInputService::searchComplete(esp_ble_gattc_cb_param_t::gattc_search_cmpl_evt_param &completion) {
  /// Start looking for characteristics
  esp_gattc_char_elem_t *charElements;
  uint16_t count = 0;

  esp_ble_gattc_get_attr_count(
    clientNumber,
    source->connectionId,
    ESP_GATT_DB_CHARACTERISTIC,
    startHandle, endHandle,
    ESP_GATT_INVALID_HANDLE,
    &count);

  if (count <= 0) {
    return; //We're done here
  }
  charElements = (esp_gattc_char_elem_t *)malloc(sizeof(esp_gattc_char_elem_t) * count);
  esp_ble_gattc_get_all_char(
      clientNumber,
      source->connectionId,
      startHandle, endHandle,
      charElements, &count, 0);
  for (uint16_t i = 0; i < count; i++)
  {
    if (charElements[i].properties & ESP_GATT_CHAR_PROP_BIT_NOTIFY &&
        wantNotifications(charElements[i])){
      esp_ble_gattc_register_for_notify(
          clientNumber,
          source->address,
          charElements[i].char_handle);
    }
  }
  free(charElements);
}

void BleInputService::reset() {
  startHandle = 0;
  endHandle = 0;
  clientNumber = 0;
}

void BleInputService::disconnected(esp_ble_gattc_cb_param_t::gattc_disconnect_evt_param &disconnect) {}

void BleInputService::notification(esp_ble_gattc_cb_param_t::gattc_notify_evt_param &notify) {
  Serial.printf("NOTIFICATION %d\n", notify.conn_id);
  Serial.printf("%d bytes: ", notify.value_len);
  for (int i = 0; i < notify.value_len; i++)
    Serial.printf("%02X ", notify.value[i]);
  Serial.println();
}
