# Stella Smart ESPHome Component

ESPHome external component for **Stella Smart BLE perfume diffuser**.

## Usage

```yaml
external_components:
  - source:
      type: git
      url: https://github.com/rcayadi/esphome-stella-smart.git
      ref: main
    components: [stella_smart]

ble_client:
  - mac_address: "AA:BB:CC:DD:EE:FF"
    id: stella_ble

stella_smart:
  - ble_client_id: stella_ble
    mac_address: "AA:BB:CC:DD:EE:FF"
    name: "Stella"
    battery:
      name: "Stella Battery"
    spray_count:
      name: "Stella Spray Count"
    power:
      name: "Stella Power"
    trigger_spray:
      name: "Stella Trigger Spray"
    reset_spray:
      name: "Stella Reset Spray Count"
    spray_duration:
      name: "Stella Spray Duration"
    timer_mode:
      name: "Stella Timer Mode"
    firmware_version:
      name: "Stella Firmware Version"
```

## Entities

| Entity | Type | Description |
|--------|------|-------------|
| Battery | Sensor | Battery level (0-100%) |
| Spray Count | Sensor | Total spray count |
| Power | Switch | Turn on/off the device |
| Trigger Spray | Button | Trigger a single spray |
| Reset Spray Count | Button | Reset spray counter |
| Spray Duration | Select | 10 min / 20 min / 40 min |
| Timer Mode | Select | X / 6h / 8h / 12h |
| Firmware Version | Text Sensor | Device firmware version |

## License

MIT
