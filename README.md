# STM32F103 LoRa Slave Sensor Node

Poll-only LoRa slave for **STM32F103 + SX1278**, designed to work with the
[ESP32_LORA](https://github.com/Hoang7142/ESP32_LORA) gateway.

The node stays in continuous RX and **only transmits when the gateway sends
`CMD_READ_SENSOR` or `CMD_PING`** addressed to this board.

## Network IDs

| Device   | ID   |
|----------|------|
| Gateway  | 0x01 |
| Node 1   | 0x11 |
| Node 2   | 0x12 |
| Node 3   | 0x13 |

Set your board ID in `lora/lora_network_config.h`:

```c
#define MY_NODE_ID  0x11   /* change to 0x12 or 0x13 per board */
```

Or pass a Keil/PlatformIO define: `-DMY_NODE_ID=0x12`.

## Wiring (SX1278 ↔ STM32F103)

| SX1278 | STM32F103 |
|--------|-----------|
| VCC    | 3.3 V     |
| GND    | GND       |
| NSS/CS | PA4       |
| RST    | PA3       |
| DIO0   | PB5 (EXTI)|
| SCK    | PA5 (SPI1)|
| MISO   | PA6       |
| MOSI   | PA7       |

### Sensors (this project)

| Sensor        | Pin  | Notes                          |
|---------------|------|--------------------------------|
| DHT11         | PB12 | Temperature / humidity         |
| Soil moisture | PA0  | ADC (via `adc.c`)              |
| Water level   | ADC  | See note below                 |

**Pin conflict:** default `water_level.c` uses **PA7** for ADC, which is also
LoRa MOSI. Move the water-level sensor to another ADC pin (e.g. PA1) and update
`water_level.c`, or use a board where water level is not on PA7.

## Modem settings (must match ESP32)

| Parameter   | Value        |
|-------------|--------------|
| Frequency   | 433 MHz      |
| SF          | 7            |
| BW          | 125 kHz      |
| CR          | 4/5          |
| TX power    | 17 dBm       |
| Preamble    | 8            |
| Sync word   | 0xF1         |
| HW CRC      | ON           |

## Build (Keil uVision)

1. Open `TEST2.uvprojx`.
2. Set `MY_NODE_ID` for this board.
3. Build and flash to STM32F103C8.

Source layout:

```
main.c                      — init, sensors, lora_node_poll() loop
lora/
  lora_radio.c/h            — SX1278 SPI driver
  lora_protocol.c/h         — packet encode/decode (from ESP32_LORA)
  lora_node.c/h             — slave state machine (from ESP32_LORA)
  lora_network_config.h     — gateway/node IDs
```

## UART debug

USART1 @ **115200** baud (PA9 TX, PA10 RX). On boot:

```
===== STM32 LoRa Slave Node =====
Radio OK, node ID 0x11, gateway 0x01
Listening (continuous RX)...
```

## Test with ESP32 gateway

1. Flash **ESP32_LORA** on the gateway (LoRa enabled in `main.cpp`).
2. Flash this STM32 firmware with `MY_NODE_ID = 0x11`.
3. Open ESP32 Serial Monitor @ 115200.
4. Every ~5 s you should see:

```
TX OK (7 bytes)
SENSOR_DATA from 0x11, RSSI -xx dBm, len 8
  temp=28.5 C  humi=62.0 %  soil=512  water=780
Poll round complete:
  Node 0x11 ONLINE
  Node 0x12 OFFLINE
  Node 0x13 OFFLINE
```

5. Repeat with boards at 0x12 and 0x13.
6. Confirm the STM32 never transmits unless polled (gateway must initiate).

## Protocol summary

- `CMD_READ_SENSOR (0x01)` → reply `CMD_SENSOR_DATA (0x02)`, same `seq`
- `CMD_PING (0x04)` → reply `CMD_ACK (0x03)`, same `seq`
- Packets with bad application CRC are ignored
- Only packets with `dst == MY_NODE_ID` are handled

Sensor payload (`lora_sensor_payload_t`, 8 bytes):

| Field             | Type    | Example   |
|-------------------|---------|-----------|
| temperature_c10   | int16   | 285 = 28.5 °C |
| humidity_pct10    | uint16  | 620 = 62.0 %  |
| soil_moisture     | uint16  | 0–100 or ADC  |
| water_level       | uint16  | 0–100 or ADC  |

## Reference

Integration template and full spec live in the ESP32_LORA repo:

- `examples/stm32_node/main_example.c`
- `examples/stm32_node/IMPLEMENTATION_PROMPT.md`
