#pragma once

#include "esphome/core/component.h"
#include "esphome/core/application.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"
#include "esphome/components/ble_client/ble_client.h"
#include "esphome/components/esp32_ble_tracker/esp32_ble_tracker.h"

#ifdef USE_SENSOR
#include "esphome/components/sensor/sensor.h"
#endif
#ifdef USE_SWITCH
#include "esphome/components/switch/switch.h"
#endif
#ifdef USE_BUTTON
#include "esphome/components/button/button.h"
#endif
#ifdef USE_SELECT
#include "esphome/components/select/select.h"
#endif
#ifdef USE_TEXT_SENSOR
#include "esphome/components/text_sensor/text_sensor.h"
#endif

#include <vector>
#include <string>
#include <cstdint>

namespace esphome {
namespace stella_smart {

class StellaSmartComponent;

#ifdef USE_SWITCH
class StellaPowerSwitch : public switch_::Switch, public Parented<StellaSmartComponent> {
  void write_state(bool state) override;
};
#endif

#ifdef USE_BUTTON
class StellaSprayButton : public button::Button, public Parented<StellaSmartComponent> {
  void press_action() override;
};
class StellaResetButton : public button::Button, public Parented<StellaSmartComponent> {
  void press_action() override;
};
#endif

#ifdef USE_SELECT
class StellaDurationSelect : public select::Select, public Parented<StellaSmartComponent> {
  void control(const std::string &value) override;
};
class StellaTimerSelect : public select::Select, public Parented<StellaSmartComponent> {
  void control(const std::string &value) override;
};
#endif

class StellaSmartComponent : public Component, public ble_client::BLEClientNode {
  friend class StellaPowerSwitch;
  friend class StellaSprayButton;
  friend class StellaResetButton;
  friend class StellaDurationSelect;
  friend class StellaTimerSelect;

 public:
  void setup() override;
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::AFTER_BLUETOOTH; }
  void gattc_event_handler(esp_gattc_cb_event_t event,
                           esp_gatt_if_t gattc_if,
                           esp_ble_gattc_cb_param_t *param) override;

  void set_mac_address(uint64_t mac) { mac_address_ = mac; }
  void set_device_name(const std::string &name) { device_name_ = name; }

#ifdef USE_SENSOR
  void set_battery_sensor(sensor::Sensor *s) { battery_sensor_ = s; }
  void set_spray_count_sensor(sensor::Sensor *s) { spray_count_sensor_ = s; }
#endif
#ifdef USE_SWITCH
  void set_power_switch(StellaPowerSwitch *s) { power_switch_ = s; }
#endif
#ifdef USE_BUTTON
  void set_spray_button(StellaSprayButton *b) { spray_button_ = b; }
  void set_reset_button(StellaResetButton *b) { reset_button_ = b; }
#endif
#ifdef USE_SELECT
  void set_duration_select(StellaDurationSelect *s) { duration_select_ = s; }
  void set_timer_select(StellaTimerSelect *s) { timer_select_ = s; }
#endif
#ifdef USE_TEXT_SENSOR
  void set_version_sensor(text_sensor::TextSensor *s) { version_sensor_ = s; }
#endif

 protected:
  void send_bytes(const uint8_t *data, size_t len);
  void send_std_cmd(uint8_t cmd);
  void send_std_cmd(uint8_t cmd, uint8_t b3);
  void send_std_cmd(uint8_t cmd, uint8_t b3, uint8_t b4);
  void send_long_cmd(uint8_t cmd, const uint8_t *payload);  // 7-byte payload
  void send_ack(const uint8_t *data, size_t len);
  void handle_response(const uint8_t *data, size_t len);
  void set_device_time();
  void on_ready();
  void query_all();
  void publish_state();

  void power_command(bool on);
  void spray_command();
  void reset_command();
  void duration_command(uint8_t idx);
  void timer_command(const std::string &mode);

  uint64_t mac_address_{0};
  std::string device_name_;

  bool ready_{false};
  bool gattc_ready_{false};
  bool notifications_enabled_{false};

  uint16_t char_write_handle_{0};
  uint16_t char_notify_handle_{0};

  uint32_t last_poll_{0};
  uint32_t last_keepalive_{0};

  bool power_state_{false};
  uint8_t battery_raw_{0};
  uint16_t spray_count_{0};
  uint8_t spray_duration_{0};
  std::string timer_mode_{"X"};
  std::string firmware_version_{};

#ifdef USE_SWITCH
  StellaPowerSwitch *power_switch_{nullptr};
#endif
#ifdef USE_SENSOR
  sensor::Sensor *battery_sensor_{nullptr};
  sensor::Sensor *spray_count_sensor_{nullptr};
#endif
#ifdef USE_BUTTON
  StellaSprayButton *spray_button_{nullptr};
  StellaResetButton *reset_button_{nullptr};
#endif
#ifdef USE_SELECT
  StellaDurationSelect *duration_select_{nullptr};
  StellaTimerSelect *timer_select_{nullptr};
#endif
#ifdef USE_TEXT_SENSOR
  text_sensor::TextSensor *version_sensor_{nullptr};
#endif
};

}  // namespace stella_smart
}  // namespace esphome
