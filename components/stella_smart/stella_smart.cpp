#include "stella_smart.h"

namespace esphome {
namespace stella_smart {

static const char *const TAG = "stella_smart";

static const char *const SERVICE_UUID = "0000fff0-0000-1000-8000-00805f9b34fb";
static const char *const CHAR_NOTIFY_UUID = "0000fff1-0000-1000-8000-00805f9b34fb";
static const char *const CHAR_WRITE_UUID = "0000fff2-0000-1000-8000-00805f9b34fb";

static const uint16_t SPRAY_EMPTY = 2600;
static const uint16_t SPRAY_LOW = 1950;

// -----------------------------------------------------------------------
// Entity forwarders
// -----------------------------------------------------------------------

#ifdef USE_SWITCH
void StellaPowerSwitch::write_state(bool state) {
  this->get_parent()->power_command(state);
}
#endif

#ifdef USE_BUTTON
void StellaSprayButton::press_action() { this->get_parent()->spray_command(); }
void StellaResetButton::press_action() { this->get_parent()->reset_command(); }
#endif

#ifdef USE_SELECT
void StellaDurationSelect::control(const std::string &value) {
  int idx = 0;
  if (value == "20 min") idx = 1;
  else if (value == "40 min") idx = 2;
  this->get_parent()->duration_command(idx);
}
void StellaTimerSelect::control(const std::string &value) {
  this->get_parent()->timer_command(value);
}
#endif

// -----------------------------------------------------------------------
// Setup / lifecycle
// -----------------------------------------------------------------------

void StellaSmartComponent::setup() {
  ESP_LOGI(TAG, "Setting up Stella Smart");

#ifdef USE_SWITCH
  if (!this->power_switch_) {
    this->power_switch_ = new StellaPowerSwitch();
    this->power_switch_->set_parent(this);
    this->power_switch_->set_name((this->device_name_ + " Power").c_str());
    this->power_switch_->set_device_class("switch");
    App.register_switch(this->power_switch_);
  }
#endif

#ifdef USE_SENSOR
  if (!this->battery_sensor_) {
    this->battery_sensor_ = new sensor::Sensor();
    this->battery_sensor_->set_name((this->device_name_ + " Battery").c_str());
    this->battery_sensor_->set_unit_of_measurement("%");
    this->battery_sensor_->set_device_class("battery");
    this->battery_sensor_->set_state_class(sensor::STATE_CLASS_MEASUREMENT);
    this->battery_sensor_->set_accuracy_decimals(0);
    App.register_sensor(this->battery_sensor_);
  }

  if (!this->spray_count_sensor_) {
    this->spray_count_sensor_ = new sensor::Sensor();
    this->spray_count_sensor_->set_name((this->device_name_ + " Spray Count").c_str());
    this->spray_count_sensor_->set_unit_of_measurement("sprays");
    this->spray_count_sensor_->set_icon("mdi:spray");
    this->spray_count_sensor_->set_state_class(sensor::STATE_CLASS_MEASUREMENT);
    this->spray_count_sensor_->set_accuracy_decimals(0);
    App.register_sensor(this->spray_count_sensor_);
  }
#endif

#ifdef USE_BUTTON
  if (!this->spray_button_) {
    this->spray_button_ = new StellaSprayButton();
    this->spray_button_->set_parent(this);
    this->spray_button_->set_name((this->device_name_ + " Trigger Spray").c_str());
    this->spray_button_->set_icon("mdi:spray");
    App.register_button(this->spray_button_);
  }

  if (!this->reset_button_) {
    this->reset_button_ = new StellaResetButton();
    this->reset_button_->set_parent(this);
    this->reset_button_->set_name((this->device_name_ + " Reset Spray Count").c_str());
    this->reset_button_->set_icon("mdi:restore");
    App.register_button(this->reset_button_);
  }
#endif

#ifdef USE_SELECT
  if (!this->duration_select_) {
    this->duration_select_ = new StellaDurationSelect();
    this->duration_select_->set_parent(this);
    this->duration_select_->set_name((this->device_name_ + " Spray Duration").c_str());
    this->duration_select_->set_icon("mdi:timer-outline");
    this->duration_select_->traits.set_options({"10 min", "20 min", "40 min"});
    App.register_select(this->duration_select_);
  }

  if (!this->timer_select_) {
    this->timer_select_ = new StellaTimerSelect();
    this->timer_select_->set_parent(this);
    this->timer_select_->set_name((this->device_name_ + " Timer Mode").c_str());
    this->timer_select_->set_icon("mdi:calendar-clock");
    this->timer_select_->traits.set_options({"X", "6h", "8h", "12h"});
    App.register_select(this->timer_select_);
  }
#endif

#ifdef USE_TEXT_SENSOR
  if (!this->version_sensor_) {
    this->version_sensor_ = new text_sensor::TextSensor();
    this->version_sensor_->set_name((this->device_name_ + " Firmware Version").c_str());
    this->version_sensor_->set_icon("mdi:chip");
    this->version_sensor_->set_entity_category(EntityCategory::ENTITY_CATEGORY_DIAGNOSTIC);
    App.register_text_sensor(this->version_sensor_);
  }
#endif
}

void StellaSmartComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "Stella Smart:");
  ESP_LOGCONFIG(TAG, "  MAC: %02X:%02X:%02X:%02X:%02X:%02X",
                (uint8_t)(this->mac_address_ >> 40),
                (uint8_t)(this->mac_address_ >> 32),
                (uint8_t)(this->mac_address_ >> 24),
                (uint8_t)(this->mac_address_ >> 16),
                (uint8_t)(this->mac_address_ >> 8),
                (uint8_t)(this->mac_address_));
}

void StellaSmartComponent::loop() {
  if (!this->gattc_ready_ || !this->notifications_enabled_) return;

  if (!this->ready_) {
    this->on_ready();
    return;
  }

  uint32_t now = millis();
  if (now - this->last_keepalive_ > 15000) {
    this->last_keepalive_ = now;
    this->set_device_time();
  }
  if (now - this->last_poll_ > 60000) {
    this->last_poll_ = now;
    this->query_all();
  }
}

// -----------------------------------------------------------------------
// BLE GATT events
// -----------------------------------------------------------------------

void StellaSmartComponent::gattc_event_handler(
    esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
    esp_ble_gattc_cb_param_t *param) {
  switch (event) {
    case ESP_GATTC_OPEN_EVT:
      if (param->open.status != ESP_GATT_OK) {
        ESP_LOGE(TAG, "Connection failed: %d", param->open.status);
        this->ready_ = false;
        this->gattc_ready_ = false;
      } else {
        ESP_LOGI(TAG, "Connected");
      }
      break;

    case ESP_GATTC_DISCONNECT_EVT:
      ESP_LOGI(TAG, "Disconnected");
      this->ready_ = false;
      this->gattc_ready_ = false;
      this->notifications_enabled_ = false;
      this->char_write_handle_ = 0;
      this->char_notify_handle_ = 0;
      break;

    case ESP_GATTC_SEARCH_CMPL_EVT: {
      auto *service = this->parent_->get_service(
          esp32_ble_tracker::ESPBTUUID::from_raw(SERVICE_UUID));
      if (!service) { ESP_LOGE(TAG, "Service FFF0 not found"); break; }

      auto *w = service->get_characteristic(
          esp32_ble_tracker::ESPBTUUID::from_raw(CHAR_WRITE_UUID));
      if (!w) { ESP_LOGE(TAG, "Char FFF2 not found"); break; }
      this->char_write_handle_ = w->handle;

      auto *n = service->get_characteristic(
          esp32_ble_tracker::ESPBTUUID::from_raw(CHAR_NOTIFY_UUID));
      if (!n) { ESP_LOGE(TAG, "Char FFF1 not found"); break; }
      this->char_notify_handle_ = n->handle;

      esp_ble_gattc_register_for_notify(
          this->parent_->get_gattc_if(), this->parent_->get_remote_bda(),
          n->handle);
      break;
    }

    case ESP_GATTC_REG_FOR_NOTIFY_EVT: {
      if (param->reg_for_notify.status != ESP_GATT_OK) break;
      uint8_t cccd[] = {0x01, 0x00};
      esp_ble_gattc_write_char_descr(
          this->parent_->get_gattc_if(), this->parent_->get_conn_id(),
          param->reg_for_notify.handle + 1,
          sizeof(cccd), cccd,
          ESP_GATT_WRITE_TYPE_RSP, ESP_GATT_AUTH_REQ_NONE);
      break;
    }

    case ESP_GATTC_NOTIFY_EVT:
      if (param->notify.handle == this->char_notify_handle_)
        this->handle_response(param->notify.value, param->notify.value_len);
      break;

    case ESP_GATTC_WRITE_DESCR_EVT:
      if (!this->notifications_enabled_) {
        this->notifications_enabled_ = true;
        this->gattc_ready_ = true;
        ESP_LOGI(TAG, "Notifications enabled");
      }
      break;

    default:
      break;
  }
}

void StellaSmartComponent::on_ready() {
  this->ready_ = true;
  ESP_LOGI(TAG, "Device ready");
  delay(100);
  this->set_device_time();
  delay(50);
  this->query_all();
  this->last_keepalive_ = millis();
  this->last_poll_ = millis();
}

// -----------------------------------------------------------------------
// BLE packet I/O
// -----------------------------------------------------------------------

void StellaSmartComponent::send_bytes(const uint8_t *data, size_t len) {
  if (!this->gattc_ready_ || this->char_write_handle_ == 0) {
    ESP_LOGW(TAG, "BLE not ready, dropping %d bytes", (int)len);
    return;
  }
  esp_ble_gattc_write_char(
      this->parent_->get_gattc_if(), this->parent_->get_conn_id(),
      this->char_write_handle_, len,
      const_cast<uint8_t *>(data),
      ESP_GATT_WRITE_TYPE_NO_RSP, ESP_GATT_AUTH_REQ_NONE);
}

// 10-byte packet [AA CMD 07 00 00 00 00 00 00 CK]
void StellaSmartComponent::send_std_cmd(uint8_t cmd) {
  uint8_t pkt[10] = {0xAA, cmd, 7, 0, 0, 0, 0, 0, 0, 0};
  uint8_t ck = 0;
  for (int i = 3; i < 9; i++) ck ^= pkt[i];
  pkt[9] = ck;
  this->send_bytes(pkt, 10);
  ESP_LOGD(TAG, "TX 0x%02X (std)", cmd);
}

// 10-byte packet [AA CMD 07 b3 00 00 00 00 00 CK]
void StellaSmartComponent::send_std_cmd(uint8_t cmd, uint8_t b3) {
  uint8_t pkt[10] = {0xAA, cmd, 7, b3, 0, 0, 0, 0, 0, 0};
  uint8_t ck = 0;
  for (int i = 3; i < 9; i++) ck ^= pkt[i];
  pkt[9] = ck;
  this->send_bytes(pkt, 10);
  ESP_LOGD(TAG, "TX 0x%02X [%02X]", cmd, b3);
}

// 10-byte packet [AA CMD 07 b3 b4 00 00 00 00 CK]
void StellaSmartComponent::send_std_cmd(uint8_t cmd, uint8_t b3, uint8_t b4) {
  uint8_t pkt[10] = {0xAA, cmd, 7, b3, b4, 0, 0, 0, 0, 0};
  uint8_t ck = 0;
  for (int i = 3; i < 9; i++) ck ^= pkt[i];
  pkt[9] = ck;
  this->send_bytes(pkt, 10);
  ESP_LOGD(TAG, "TX 0x%02X [%02X %02X]", cmd, b3, b4);
}

// 11-byte packet [AA CMD 08 pay0..pay6 CK] — payload = 7 bytes
void StellaSmartComponent::send_long_cmd(uint8_t cmd, const uint8_t *payload) {
  uint8_t pkt[11];
  pkt[0] = 0xAA;
  pkt[1] = cmd;
  pkt[2] = 8;
  memcpy(pkt + 3, payload, 7);
  uint8_t ck = 0;
  for (int i = 3; i < 10; i++) ck ^= pkt[i];
  pkt[10] = ck;
  this->send_bytes(pkt, 11);
  ESP_LOGD(TAG, "TX 0x%02X (long)", cmd);
}

void StellaSmartComponent::send_ack(const uint8_t *data, size_t len) {
  this->send_bytes(data, len);
}

// -----------------------------------------------------------------------
// Commands
// -----------------------------------------------------------------------

void StellaSmartComponent::set_device_time() {
  struct tm t;
  time_t now_ts = ::time(nullptr);
  localtime_r(&now_ts, &t);

  // Per APK: 10-byte, LEN=7, payload = year,month,day,hour,min,sec
  uint8_t pkt[10] = {
      0xAA, 0x24, 7,
      (uint8_t)(t.tm_year + 1900 - 2000),
      (uint8_t)(t.tm_mon + 1),
      (uint8_t)(t.tm_mday),
      (uint8_t)(t.tm_hour),
      (uint8_t)(t.tm_min),
      (uint8_t)(t.tm_sec),
      0  // checksum
  };
  uint8_t ck = 0;
  for (int i = 3; i < 9; i++) ck ^= pkt[i];
  pkt[9] = ck;
  this->send_bytes(pkt, 10);
  ESP_LOGD(TAG, "TX setTime");
}

void StellaSmartComponent::query_all() {
  this->send_std_cmd(0x22);
  delay(50);
  this->send_std_cmd(0x09);
  delay(50);
  this->send_std_cmd(0x10);
  delay(50);
  this->send_std_cmd(0x04);
}

void StellaSmartComponent::power_command(bool on) {
  this->send_std_cmd(0x0D, on ? 1 : 0);
  ESP_LOGI(TAG, "Power %s", on ? "ON" : "OFF");
  delay(50);
  this->send_std_cmd(0x22);
  delay(50);
  this->send_std_cmd(0x09);
}

void StellaSmartComponent::spray_command() {
  this->send_std_cmd(0x08);
  ESP_LOGI(TAG, "Spray");
}

void StellaSmartComponent::reset_command() {
  this->send_std_cmd(0x0B);
  ESP_LOGI(TAG, "Reset spray count");
}

void StellaSmartComponent::duration_command(uint8_t idx) {
  if (idx > 3) idx = 0;
  this->send_std_cmd(0x0F, idx);
  this->spray_duration_ = idx;
}

void StellaSmartComponent::timer_command(const std::string &mode) {
  this->timer_mode_ = mode;
  this->send_std_cmd(0x06, 0);
}

// -----------------------------------------------------------------------
// Response handler
// -----------------------------------------------------------------------

void StellaSmartComponent::handle_response(const uint8_t *data, size_t len) {
  if (len < 4 || data[0] != 0xAA) return;

  uint8_t cmd = data[1];
  uint8_t b3 = data[3];

  ESP_LOGD(TAG, "RX 0x%02X [%s]", cmd,
           esphome::format_hex_pretty(data, len).c_str());

  switch (cmd) {
    case 0x04: case 0x05:
      this->spray_duration_ = b3;
      break;

    case 0x08:
      if (b3 == 1) ESP_LOGW(TAG, "Spray failed");
      break;

    case 0x09: case 0x0A:
      if (len >= 5) {
        uint16_t cnt = (uint16_t)(data[4]) | ((uint16_t)(data[3]) << 8);
        this->spray_count_ = cnt;
        if (cnt >= SPRAY_EMPTY || cnt == 0)
          ESP_LOGW(TAG, "Perfume empty (%d)", cnt);
        else if (cnt >= SPRAY_LOW)
          ESP_LOGW(TAG, "Perfume low (%d)", cnt);
      }
      break;

    case 0x0B:
      delay(50);
      this->send_std_cmd(0x09);
      return;

    case 0x0C:
    case 0x10:
      this->power_state_ = (b3 != 0);
      break;

    case 0x0E:
      if (b3 == 0) {
        if      (this->timer_mode_ == "X")  this->timer_mode_ = "6";
        else if (this->timer_mode_ == "6")  this->timer_mode_ = "8";
        else if (this->timer_mode_ == "8")  this->timer_mode_ = "12";
        else if (this->timer_mode_ == "12") this->timer_mode_ = "X";
      }
      break;

    case 0x0F:
      this->spray_duration_ = b3;
      break;

    case 0x22: case 0x23:
      this->battery_raw_ = b3;
      break;

    case 0x24:
      delay(50);
      this->send_std_cmd(0x10);
      return;

    case 0x28:
      delay(50);
      this->send_std_cmd(0x28);
      return;

    case 0x2A:
      if (len >= 8) {
        this->firmware_version_ =
            std::to_string(data[5]) + '.' +
            std::to_string(data[6]) + '.' +
            std::to_string(data[7]);
      }
      break;

    case 0x2B:
      if (len >= 3) this->send_ack(data, 3);
      return;

    case 0x2C:
      if (len >= 5 && data[4] != 0) this->send_ack(data, 4);
      return;

    default:
      return;
  }

  this->publish_state();
}

void StellaSmartComponent::publish_state() {
#ifdef USE_SWITCH
  if (this->power_switch_)
    this->power_switch_->publish_state(this->power_state_);
#endif
#ifdef USE_SENSOR
  if (this->battery_sensor_)
    this->battery_sensor_->publish_state(this->battery_raw_);
  if (this->spray_count_sensor_)
    this->spray_count_sensor_->publish_state(this->spray_count_);
#endif
#ifdef USE_SELECT
  if (this->duration_select_ && this->spray_duration_ < 3) {
    static const char *opts[] = {"10 min", "20 min", "40 min"};
    this->duration_select_->publish_state(opts[this->spray_duration_]);
  }
  if (this->timer_select_)
    this->timer_select_->publish_state(this->timer_mode_);
#endif
#ifdef USE_TEXT_SENSOR
  if (this->version_sensor_ && !this->firmware_version_.empty())
    this->version_sensor_->publish_state(this->firmware_version_);
#endif
}

}  // namespace stella_smart
}  // namespace esphome
