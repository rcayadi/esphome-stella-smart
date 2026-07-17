import esphome.codegen as cg
import esphome.config_validation as cv

from esphome.components import ble_client
from esphome.const import CONF_ID

DEPENDENCIES = ["ble_client"]
AUTO_LOAD = ["esp32_ble_client"]

stella_ble_ns = cg.esphome_ns.namespace("stella_ble")

StellaBLE = stella_ble_ns.class_(
    "StellaBLE",
    ble_client.BLEClientNode,
    cg.Component,
)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(StellaBLE),
        }
    )
    .extend(ble_client.BLE_CLIENT_SCHEMA)
    .extend(cv.COMPONENT_SCHEMA)
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])

    await cg.register_component(var, config)

    await ble_client.register_ble_node(var, config)
