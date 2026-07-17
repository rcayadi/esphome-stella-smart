#pragma once

#include "esphome/core/component.h"
#include "esphome/core/log.h"

#include "esphome/components/ble_client/ble_client.h"
#include "esphome/components/esp32_ble_tracker/esp32_ble_tracker.h"

namespace esphome {
namespace stella_ble {

class StellaBLE : public ble_client::BLEClientNode,
                  public Component {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;

  // BLEClientNode
  void gattc_event_handler(
      esp_gattc_cb_event_t event,
      esp_gatt_if_t gattc_if,
      esp_ble_gattc_cb_param_t *param) override;

  void gap_event_handler(
      esp_gap_ble_cb_event_t event,
      esp_ble_gap_cb_param_t *param) override;

  bool send_packet(const uint8_t *data, size_t len);

 protected:
  bool ready_{false};

  uint16_t service_start_{0};
  uint16_t service_end_{0};

  uint16_t write_handle_{0};
  uint16_t notify_handle_{0};

  bool notify_enabled_{false};
};

}  // namespace stella_ble
}  // namespace esphome
