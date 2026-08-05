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
		case CMD_WRITE_CONTROL:
			return "WRITE_CONTROL"; // ?? Th?m d?ng n?y
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
    node_log(node, "[ERR] LoRa TX fail (lost pkt?)\r\n");
    return false;
  }

  node_log(node, "TX OK\r\n");
  return true;
}

static bool handle_read_sensor(lora_node_t *node, const lora_packet_t *request) {// ham xu ly doc cam bien
  uint8_t sensor_payload[LORA_MAX_PAYLOAD];
  uint8_t payload_len = 0;

  node_log(node, "REQ READ_SENSOR: sampling...\r\n");
//goi phan cung do dac du lieu
  if (node->radio.read_sensor != NULL) {
    payload_len = node->radio.read_sensor(sensor_payload, LORA_MAX_PAYLOAD);// doc du lieu tu cam bien va dua vao mang
    if (payload_len > LORA_MAX_PAYLOAD) {
      payload_len = LORA_MAX_PAYLOAD;// khong che du lieu tranh mang bi tran
    }
  }
  if (payload_len == 0) {
    node_log(node, "[ERR] Sensor payload empty\r\n");
  }

  node_log(node, "Sensor payload %u bytes\r\n", (unsigned)payload_len);

  lora_packet_t response;
  if (!lora_packet_build(&response, node->config.gateway_id, node->config.node_id,
                         CMD_SENSOR_DATA, request->seq, sensor_payload,
                         payload_len)) {// goi ham dong goi du lieu, cmd ->CMD_SENSOR_DATA bao day la du lieu cam bien
    node_log(node, "[ERR] Build SENSOR_DATA fail\r\n");
    return false;
  }

  return send_packet(node, &response);// phat song ban ve cho getway , ma hoa encoder
}

static bool handle_ping(lora_node_t *node, const lora_packet_t *request) {// ham xu ly yeu cau cua getway la kiem tra xem node con song hay k
  lora_packet_t response;

  node_log(node, "REQ PING: sending ACK\r\n");

  if (!lora_packet_build(&response, node->config.gateway_id, node->config.node_id,
                         CMD_ACK, request->seq, NULL, 0)) {
    node_log(node, "[ERR] Build PING ACK fail\r\n");
    return false;
  }

  return send_packet(node, &response);
}

extern volatile lora_control_payload_t g_remote_control;
extern volatile uint8_t g_control_updated;
extern uint8_t g_soil_on_threshold;
extern uint8_t g_soil_off_threshold;
volatile uint8_t g_ack_pending = 0; // C? b?o: c? ACK dang ch? g?i (sau khi main.c x? l? xong)
// ?? H?M M?I: B?c t?ch d? li?u n?t b?m t? ESP32 g?i xu?ng v? d?y qua cho main x? l? ph?n c?ng
/** @brief H?ng l?nh ghi di?u khi?n t? Web xu?ng v? ??NG G?I TR?NG TH?I TH?C T? v?o g?i ACK g?i ngu?c l?n */
static bool handle_write_control(lora_node_t *node, const lora_packet_t *request) {
  if (node->config.log != NULL) {
    node->config.log("[LoRa Node] Nhan lenh nut nhan tu Web GUI do xuong\r\n");
  }

  if (request->payload_len == sizeof(lora_control_payload_t)) {
    memcpy((void*)&g_remote_control, request->payload, sizeof(lora_control_payload_t));
    #if CONTROL_TEST_DEBUG
  if (node->config.log != NULL) {
    node->config.log("[LoRa] CMD_WRITE_CONTROL payload OK\r\n");
  }
#endif
    g_control_updated = 1;
  }

  // KH?NG g?i ACK ngay ? d?y n?a. Ch? d?nh d?u "c? l?nh c?n ph?n h?i".
  // main.c s? g?i ACK TH?T sau khi ?p d?ng xong logic AUTO/an to?n,
  // d?ng node->pending_request (d? luu s?n src/dst/seq c?a l?nh n?y).
  g_ack_pending = 1;

  if (node->config.log != NULL) {
    node->config.log("[LoRa Node] Da xep hang cho ACK, cho main.c xu ly xong AUTO...\r\n");
  }

  return true;
}

static bool handle_set_thresholds(lora_node_t *node, const lora_packet_t *request) {
  if (request->payload_len == sizeof(lora_threshold_payload_t)) {
    lora_threshold_payload_t th;
    memcpy(&th, request->payload, sizeof(th));
    if (th.soil_on < th.soil_off && th.soil_on <= 100 && th.soil_off <= 100) {
      g_soil_on_threshold = th.soil_on;
      g_soil_off_threshold = th.soil_off;
      if (node->config.log != NULL) {
        node->config.log("[LoRa] Thresholds updated: ON<%u OFF>%u\r\n",
                         (unsigned)th.soil_on, (unsigned)th.soil_off);
      }
    }
  }
  return true;
}

bool lora_node_send_ack(lora_node_t *node, const lora_control_payload_t *state) {
  lora_packet_t response;
  // D?ng l?i src/dst/seq c?a l?nh WRITE_CONTROL d? luu trong node->pending_request
  if (!lora_packet_build(&response, node->pending_request.src, node->pending_request.dst,
                         CMD_ACK, node->pending_request.seq,
                         (const uint8_t*)state, sizeof(*state))) {
    if (node->config.log != NULL) {
      node->config.log("[ERR] Build control ACK fail\r\n");
    }
    return false;
  }

  if (node->config.log != NULL) {
    node->config.log("[LoRa Node] Dang phat CMD_ACK mang TRANG THAI THAT (sau AUTO) len ESP32...\r\n");
  }

  return send_packet(node, &response);
}


static void process_request(lora_node_t *node, const lora_packet_t *request) {// ham xu ly yeu cau cua getway
  node_log(node, "Process %s seq=%u\r\n", cmd_name(request->cmd),
           (unsigned)request->seq);

  switch (request->cmd) {// ma lenh
    case CMD_READ_SENSOR:
      handle_read_sensor(node, request);// doc cam bien
      break;
    case CMD_PING:
      handle_ping(node, request);// phan hoi ping
      break;
		case CMD_WRITE_CONTROL:
			handle_write_control(node, request);
      break;
    case CMD_SET_THRESHOLDS:
      handle_set_thresholds(node, request);
      break;
    default:// lenh la thi in log canh bao va bo qua
      node_log(node, "Ignored cmd 0x%02X\r\n", (unsigned)request->cmd);
      break;
  }
}

void lora_node_init(lora_node_t* node, const lora_node_config_t* config,
                    const lora_node_radio_t* radio) {//ham de cau hinh cho node
  if (node == NULL || config == NULL || radio == NULL) {
    return;
  }

  memset(node, 0, sizeof(*node));//xoa du lieu cua node trong vung nho ram de k bi du lieu rac
  node->config = *config;
  node->radio = *radio;
  node->state = LORA_NODE_STATE_RX_WAIT;// thiet lap trang thai ban dau la trang thai cho
  node_log(node, "Node init OK, state=RX_WAIT\r\n");
}

void lora_node_poll(lora_node_t* node) {
  if (node == NULL || node->radio.rx_pending == NULL ||
      node->radio.receive == NULL) {// kiem tra ham ngat va ham nhan du lieu da dc set up day du chua
    return;
  }

  if (!node->radio.rx_pending()) {// kiem tra xem chan DIO c? dc keo len k tuc la nhan dc song k, neu k co thoat de MCU lam nhung viec khac
    return;
  }

  uint8_t raw[LORA_PACKET_MAX_SIZE];     // loi mang byte du lieu tho ra                        
  int16_t rssi = 0;     // bien do do manh yeu cua song                                           
  const int rx_len = node->radio.receive(raw, sizeof(raw), &rssi); // do byte du lieu tho vao mang raw
  if (rx_len <= 0) {                                               
    if (rx_len < 0) {
      node_log(node, "[ERR] LoRa RX CRC (noise/lost)\r\n");
    }
    return;
  }

  node_log(node, "RX raw %d bytes rssi=%d\r\n", rx_len, (int)rssi);
// 3. Giai ma va kiem tra tem chong gia
  lora_packet_t pkt;
  if (!lora_packet_decode(raw, (size_t)rx_len, &pkt)) {
    node_log(node, "[ERR] LoRa decode/CRC bad\r\n");
    return;// thay raw vao ham decorder de kiem tra crc va boc tach du lieu ptk, neu giai ma loi huy goi tinh luon
  }

  node_log(node, "RX pkt %s src=0x%02X dst=0x%02X seq=%u plen=%u\r\n",
           cmd_name(pkt.cmd), (unsigned)pkt.src, (unsigned)pkt.dst,
           (unsigned)pkt.seq, (unsigned)pkt.payload_len);

  if (pkt.dst != node->config.node_id) {// kiem tra xem node nao dung id thi moi nhan goi tin
    // char buf[48];
    // snprintf(buf, sizeof(buf), "RX drop: not for me (want 0x%02X)\r\n",
    //          (unsigned)node->config.node_id);
    // node_log(node, buf);
    return;
  }
// neu dung la getway goi minh th? node do phai thuc thi menh lenh tu getway
  node->pending_request = pkt;// luu goi tin vao bien node->pending_request de node ghi nho
  node->state = LORA_NODE_STATE_PROCESS_REQUEST;// cap nhat trang thai sang trang thai (dang xu ly yeu cau)
  process_request(node, &pkt);//di doc cam bien va tra du lieu theo yeu cau, hoac phan hoi ACK neu la lenh PING
  node->state = LORA_NODE_STATE_RX_WAIT;// sau khi xu ly lenh cua getway xong thi node tro ve trang thai cho
}

lora_node_state_t lora_node_get_state(const lora_node_t* node) {// ham debug kiem tra trang thai cua node
  return node != NULL ? node->state : LORA_NODE_STATE_RX_WAIT;
}
