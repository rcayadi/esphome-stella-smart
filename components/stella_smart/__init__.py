import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import ble_client, sensor, switch, button, select, text_sensor
from esphome.const import (
    CONF_ID,
    CONF_MAC_ADDRESS,
    CONF_NAME,
    DEVICE_CLASS_BATTERY,
    DEVICE_CLASS_SWITCH,
    STATE_CLASS_MEASUREMENT,
    UNIT_PERCENT,
    ENTITY_CATEGORY_DIAGNOSTIC,
)

DEPENDENCIES = ["ble_client"]
AUTO_LOAD = ["sensor", "switch", "button", "select", "text_sensor"]
CODEOWNERS = ["@opencode"]

CONF_POWER = "power"
CONF_BATTERY = "battery"
CONF_SPRAY_COUNT = "spray_count"
CONF_SPRAY_DURATION = "spray_duration"
CONF_TIMER_MODE = "timer_mode"
CONF_TRIGGER_SPRAY = "trigger_spray"
CONF_RESET_SPRAY = "reset_spray"
CONF_FIRMWARE_VERSION = "firmware_version"

stella_smart_ns = cg.esphome_ns.namespace("stella_smart")
StellaSmartComponent = stella_smart_ns.class_(
    "StellaSmartComponent", cg.Component, ble_client.BLEClientNode
)
StellaPowerSwitch = stella_smart_ns.class_("StellaPowerSwitch", switch.Switch)
StellaSprayButton = stella_smart_ns.class_("StellaSprayButton", button.Button)
StellaResetButton = stella_smart_ns.class_("StellaResetButton", button.Button)
StellaDurationSelect = stella_smart_ns.class_("StellaDurationSelect", select.Select)
StellaTimerSelect = stella_smart_ns.class_("StellaTimerSelect", select.Select)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(StellaSmartComponent),
            cv.Required(CONF_MAC_ADDRESS): cv.mac_address,
            cv.Optional(CONF_NAME, default="Stella Smart"): cv.string,
            cv.Optional(CONF_BATTERY): sensor.sensor_schema(
                unit_of_measurement=UNIT_PERCENT,
                device_class=DEVICE_CLASS_BATTERY,
                state_class=STATE_CLASS_MEASUREMENT,
                accuracy_decimals=0,
            ),
            cv.Optional(CONF_SPRAY_COUNT): sensor.sensor_schema(
                unit_of_measurement="sprays",
                icon="mdi:spray",
                state_class=STATE_CLASS_MEASUREMENT,
                accuracy_decimals=0,
            ),
            cv.Optional(CONF_POWER): switch.switch_schema(
                StellaPowerSwitch,
                device_class=DEVICE_CLASS_SWITCH,
            ),
            cv.Optional(CONF_TRIGGER_SPRAY): button.button_schema(
                StellaSprayButton,
                icon="mdi:spray",
            ),
            cv.Optional(CONF_RESET_SPRAY): button.button_schema(
                StellaResetButton,
                icon="mdi:restore",
            ),
            cv.Optional(CONF_SPRAY_DURATION): select.select_schema(
                StellaDurationSelect,
                icon="mdi:timer-outline",
            ),
            cv.Optional(CONF_TIMER_MODE): select.select_schema(
                StellaTimerSelect,
                icon="mdi:calendar-clock",
            ),
            cv.Optional(CONF_FIRMWARE_VERSION): text_sensor.text_sensor_schema(
                icon="mdi:chip",
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
            ),
        }
    )
    .extend(ble_client.BLE_CLIENT_SCHEMA)
    .extend(cv.COMPONENT_SCHEMA)
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await ble_client.register_ble_node(var, config)

    cg.add(var.set_mac_address(config[CONF_MAC_ADDRESS].as_hex))
    cg.add(var.set_device_name(config[CONF_NAME]))

    if CONF_BATTERY in config:
        sens = await sensor.new_sensor(config[CONF_BATTERY])
        cg.add(var.set_battery_sensor(sens))
    if CONF_SPRAY_COUNT in config:
        sens = await sensor.new_sensor(config[CONF_SPRAY_COUNT])
        cg.add(var.set_spray_count_sensor(sens))
    if CONF_POWER in config:
        sw = await switch.new_switch(config[CONF_POWER])
        cg.add(sw.set_parent(var))
        cg.add(var.set_power_switch(sw))
    if CONF_TRIGGER_SPRAY in config:
        btn = await button.new_button(config[CONF_TRIGGER_SPRAY])
        cg.add(btn.set_parent(var))
        cg.add(var.set_spray_button(btn))
    if CONF_RESET_SPRAY in config:
        btn = await button.new_button(config[CONF_RESET_SPRAY])
        cg.add(btn.set_parent(var))
        cg.add(var.set_reset_button(btn))
    if CONF_SPRAY_DURATION in config:
        sel = await select.new_select(
            config[CONF_SPRAY_DURATION],
            options=["10 min", "20 min", "40 min"],
        )
        cg.add(sel.set_parent(var))
        cg.add(var.set_duration_select(sel))
    if CONF_TIMER_MODE in config:
        sel = await select.new_select(
            config[CONF_TIMER_MODE],
            options=["X", "6h", "8h", "12h"],
        )
        cg.add(sel.set_parent(var))
        cg.add(var.set_timer_select(sel))
    if CONF_FIRMWARE_VERSION in config:
        ts = await text_sensor.new_text_sensor(config[CONF_FIRMWARE_VERSION])
        cg.add(var.set_version_sensor(ts))
