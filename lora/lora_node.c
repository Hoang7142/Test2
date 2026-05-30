/**
 * @file lora_node.c
 * @brief Non-blocking slave response state machine.
 */
#include "lora_node.h"

#include <string.h>

static bool send_packet(const lora_node_t* node, const lora_packet_t* pkt) {
  uint8_t wire[LORA_PACKET_MAX_SIZE];
  const size_t wire_len = lora_packet_encode(pkt, wire, sizeof(wire));

  if (wire_len == 0 || node->radio.send == NULL) {
    return false;
  }

  return node->radio.send(wire, wire_len);
}

static bool handle_read_sensor(lora_node_t* node, const lora_packet_t* request) {
  uint8_t sensor_payload[LORA_MAX_PAYLOAD];
  uint8_t payload_len = 0;

  if (node->radio.read_sensor != NULL) {
    payload_len = node->radio.read_sensor(sensor_payload, LORA_MAX_PAYLOAD);
    if (payload_len > LORA_MAX_PAYLOAD) {
      payload_len = LORA_MAX_PAYLOAD;
    }
  }

  lora_packet_t response;
  if (!lora_packet_build(&response, node->config.gateway_id, node->config.node_id,
                         CMD_SENSOR_DATA, request->seq, sensor_payload,
                         payload_len)) {
    return false;
  }

  return send_packet(node, &response);
}

static bool handle_ping(lora_node_t* node, const lora_packet_t* request) {
  lora_packet_t response;
  if (!lora_packet_build(&response, node->config.gateway_id, node->config.node_id,
                         CMD_ACK, request->seq, NULL, 0)) {
    return false;
  }

  return send_packet(node, &response);
}

static void process_request(lora_node_t* node, const lora_packet_t* request) {
  switch (request->cmd) {
    case CMD_READ_SENSOR:
      handle_read_sensor(node, request);
      break;
    case CMD_PING:
      handle_ping(node, request);
      break;
    default:
      break;
  }
}

void lora_node_init(lora_node_t* node, const lora_node_config_t* config,
                    const lora_node_radio_t* radio) {
  if (node == NULL || config == NULL || radio == NULL) {
    return;
  }

  memset(node, 0, sizeof(*node));
  node->config = *config;
  node->radio = *radio;
  node->state = LORA_NODE_STATE_RX_WAIT;
}

void lora_node_poll(lora_node_t* node) {
  if (node == NULL || node->radio.rx_pending == NULL ||
      node->radio.receive == NULL) {
    return;
  }

  if (!node->radio.rx_pending()) {
    return;
  }

  uint8_t raw[LORA_PACKET_MAX_SIZE];
  int16_t rssi = 0;
  const int rx_len = node->radio.receive(raw, sizeof(raw), &rssi);
  if (rx_len <= 0) {
    return;
  }

  lora_packet_t pkt;
  if (!lora_packet_decode(raw, (size_t)rx_len, &pkt)) {
    return;
  }

  if (pkt.dst != node->config.node_id) {
    return;
  }

  node->pending_request = pkt;
  node->state = LORA_NODE_STATE_PROCESS_REQUEST;
  process_request(node, &pkt);
  node->state = LORA_NODE_STATE_RX_WAIT;
}

lora_node_state_t lora_node_get_state(const lora_node_t* node) {
  return node != NULL ? node->state : LORA_NODE_STATE_RX_WAIT;
}
