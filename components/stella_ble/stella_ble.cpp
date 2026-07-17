#include "stella_ble.h"

namespace esphome {
namespace stella_ble {

static const char *const TAG = "stella_ble";

void StellaBLE::setup() {
  ESP_LOGI(TAG, "Setup");
}

void StellaBLE::loop() {
}

void StellaBLE::dump_config() {
  ESP_LOGCONFIG(TAG, "Stella BLE");
}

void StellaBLE::gattc_event_handler(
    esp_gattc_cb_event_t event,
    esp_gatt_if_t gattc_if,
    esp_ble_gattc_cb_param_t *param) {

  switch (event) {

    case ESP_GATTC_OPEN_EVT:
      ESP_LOGI(TAG, "OPEN");
      break;

    case ESP_GATTC_CONNECT_EVT:
      ESP_LOGI(TAG, "CONNECT");
      break;

    case ESP_GATTC_CFG_MTU_EVT:
      ESP_LOGI(TAG, "MTU");
      break;

    case ESP_GATTC_SEARCH_RES_EVT:
      ESP_LOGI(TAG, "SERVICE FOUND");
      break;

    case ESP_GATTC_SEARCH_CMPL_EVT:
      ESP_LOGI(TAG, "SEARCH COMPLETE");
      break;

    case ESP_GATTC_REG_FOR_NOTIFY_EVT:
      ESP_LOGI(TAG, "REGISTER NOTIFY");
      break;

    case ESP_GATTC_NOTIFY_EVT:
      ESP_LOGI(TAG, "NOTIFY len=%d", param->notify.value_len);
      break;

    case ESP_GATTC_WRITE_DESCR_EVT:
      ESP_LOGI(TAG, "WRITE DESCRIPTOR");
      break;

    case ESP_GATTC_WRITE_CHAR_EVT:
      ESP_LOGI(TAG, "WRITE CHAR");
      break;

    case ESP_GATTC_DISCONNECT_EVT:
      ESP_LOGI(TAG, "DISCONNECT");
      break;

    default:
      break;
  }
}

void StellaBLE::gap_event_handler(
    esp_gap_ble_cb_event_t event,
    esp_ble_gap_cb_param_t *param) {
}

bool StellaBLE::send_packet(const uint8_t *data, size_t len) {
  return false;
}

}  // namespace stella_ble
}  // namespace esphome
