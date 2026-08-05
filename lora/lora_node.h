#pragma once

/**
 * @file lora_node.h
 * @brief Slave node request handler and response state machine API.
 */

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "lora_protocol.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Optional debug hook (same style as debug_log). NULL = silent. */
typedef void (*lora_node_log_fn)(const char *fmt, ...);

typedef struct {
  uint8_t node_id;
  uint8_t gateway_id;
  lora_node_log_fn log;
} lora_node_config_t;

typedef bool (*lora_node_send_fn)(const uint8_t* data, size_t len);
typedef bool (*lora_node_rx_pending_fn)(void);
typedef int (*lora_node_receive_fn)(uint8_t* data, size_t max_len,
                                    int16_t* rssi_out);
typedef uint8_t (*lora_node_read_sensor_fn)(uint8_t* payload, uint8_t max_len);

typedef struct {
  lora_node_send_fn send;
  lora_node_rx_pending_fn rx_pending;
  lora_node_receive_fn receive;
  lora_node_read_sensor_fn read_sensor;
} lora_node_radio_t;

typedef enum {
  LORA_NODE_STATE_RX_WAIT = 0,
  LORA_NODE_STATE_PROCESS_REQUEST,
  LORA_NODE_STATE_READ_SENSOR,
  LORA_NODE_STATE_SEND_RESPONSE,
} lora_node_state_t;

typedef struct {
  lora_node_config_t config;
  lora_node_radio_t radio;
  lora_node_state_t state;
  lora_packet_t pending_request;
} lora_node_t;

void lora_node_init(lora_node_t* node, const lora_node_config_t* config,
                    const lora_node_radio_t* radio);

void lora_node_poll(lora_node_t* node);

lora_node_state_t lora_node_get_state(const lora_node_t* node);
/**
 * @brief G?i CMD_ACK ph?n h?i l?nh WRITE_CONTROL, dùng TR?NG THÁI TH?T
 *        (sau khi dã áp d?ng AUTO/an toàn), không ph?i l?nh v?a nh?n.
 * @param node   Node instance.
 * @param state  Tr?ng thái th?t cu?i cùng d? dóng gói g?i lên Gateway.
 * @return true n?u g?i thành công.
 */
bool lora_node_send_ack(lora_node_t* node, const lora_control_payload_t* state);

#ifdef __cplusplus
}
#endif
