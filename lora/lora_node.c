/**
 * @file lora_node.c
 * @brief Non-blocking slave response state machine.
 */
#include "lora_node.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static void node_log(lora_node_t *node, const char *fmt, ...) {
  char buf[128];
  va_list ap;

  if (node == NULL || node->config.log == NULL || fmt == NULL) {
    return;
  }

  va_start(ap, fmt);
  vsnprintf(buf, sizeof(buf), fmt, ap);
  va_end(ap);

  node->config.log("[LoRa] %s", buf);
}

static const char *cmd_name(uint8_t cmd) {
  switch (cmd) {
    case CMD_READ_SENSOR:
      return "READ_SENSOR";
    case CMD_SENSOR_DATA:
      return "SENSOR_DATA";
    case CMD_ACK:
      return "ACK";
    case CMD_PING:
      return "PING";
    default:
      return "UNKNOWN";
  }
}

static bool send_packet(lora_node_t *node, const lora_packet_t *pkt) {
  uint8_t wire[LORA_PACKET_MAX_SIZE];
  const size_t wire_len = lora_packet_encode(pkt, wire, sizeof(wire));

  if (wire_len == 0 || node->radio.send == NULL) {
    node_log(node, "TX encode/send unavailable\r\n");
    return false;
  }

  node_log(node, "TX %s -> gw 0x%02X seq=%u len=%u\r\n",
           cmd_name(pkt->cmd), (unsigned)pkt->dst, (unsigned)pkt->seq,
           (unsigned)wire_len);

  if (!node->radio.send(wire, wire_len)) {
    node_log(node, "TX radio send failed\r\n");
    return false;
  }

  node_log(node, "TX OK\r\n");
  return true;
}

static bool handle_read_sensor(lora_node_t *node, const lora_packet_t *request) {
  uint8_t sensor_payload[LORA_MAX_PAYLOAD];
  uint8_t payload_len = 0;

  node_log(node, "REQ READ_SENSOR: sampling...\r\n");

  if (node->radio.read_sensor != NULL) {
    payload_len = node->radio.read_sensor(sensor_payload, LORA_MAX_PAYLOAD);
    if (payload_len > LORA_MAX_PAYLOAD) {
      payload_len = LORA_MAX_PAYLOAD;
    }
  }

  node_log(node, "Sensor payload %u bytes\r\n", (unsigned)payload_len);

  lora_packet_t response;
  if (!lora_packet_build(&response, node->config.gateway_id, node->config.node_id,
                         CMD_SENSOR_DATA, request->seq, sensor_payload,
                         payload_len)) {
    node_log(node, "Build SENSOR_DATA response failed\r\n");
    return false;
  }

  return send_packet(node, &response);
}

static bool handle_ping(lora_node_t *node, const lora_packet_t *request) {
  lora_packet_t response;

  node_log(node, "REQ PING: sending ACK\r\n");

  if (!lora_packet_build(&response, node->config.gateway_id, node->config.node_id,
                         CMD_ACK, request->seq, NULL, 0)) {
    node_log(node, "Build ACK response failed\r\n");
    return false;
  }

  return send_packet(node, &response);
}

static void process_request(lora_node_t *node, const lora_packet_t *request) {
  node_log(node, "Process %s seq=%u\r\n", cmd_name(request->cmd),
           (unsigned)request->seq);

  switch (request->cmd) {
    case CMD_READ_SENSOR:
      handle_read_sensor(node, request);
      break;
    case CMD_PING:
      handle_ping(node, request);
      break;
    default:
      node_log(node, "Ignored cmd 0x%02X\r\n", (unsigned)request->cmd);
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
  node_log(node, "Node init OK, state=RX_WAIT\r\n");
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
    if (rx_len < 0) {
      node_log(node, "RX CRC/error\r\n");
    }
    return;
  }

  node_log(node, "RX raw %d bytes rssi=%d\r\n", rx_len, (int)rssi);

  lora_packet_t pkt;
  if (!lora_packet_decode(raw, (size_t)rx_len, &pkt)) {
    node_log(node, "RX decode failed\r\n");
    return;
  }

  node_log(node, "RX pkt %s src=0x%02X dst=0x%02X seq=%u plen=%u\r\n",
           cmd_name(pkt.cmd), (unsigned)pkt.src, (unsigned)pkt.dst,
           (unsigned)pkt.seq, (unsigned)pkt.payload_len);

  if (pkt.dst != node->config.node_id) {
    // char buf[48];
    // snprintf(buf, sizeof(buf), "RX drop: not for me (want 0x%02X)\r\n",
    //          (unsigned)node->config.node_id);
    // node_log(node, buf);
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
