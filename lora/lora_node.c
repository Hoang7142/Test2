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
			return "WRITE_CONTROL"; // ?? Thêm dòng này
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

  node_log(node, "Sensor payload %u bytes\r\n", (unsigned)payload_len);

  lora_packet_t response;
  if (!lora_packet_build(&response, node->config.gateway_id, node->config.node_id,
                         CMD_SENSOR_DATA, request->seq, sensor_payload,
                         payload_len)) {// goi ham dong goi du lieu, cmd ->CMD_SENSOR_DATA bao day la du lieu cam bien
    node_log(node, "Build SENSOR_DATA response failed\r\n");
    return false;
  }

  return send_packet(node, &response);// phat song ban ve cho getway , ma hoa encoder
}

static bool handle_ping(lora_node_t *node, const lora_packet_t *request) {// ham xu ly yeu cau cua getway la kiem tra xem node con song hay k
  lora_packet_t response;

  node_log(node, "REQ PING: sending ACK\r\n");

  if (!lora_packet_build(&response, node->config.gateway_id, node->config.node_id,
                         CMD_ACK, request->seq, NULL, 0)) {
    node_log(node, "Build ACK response failed\r\n");
    return false;
  }

  return send_packet(node, &response);
}

extern volatile lora_control_payload_t g_remote_control;
extern volatile uint8_t g_control_updated;
// ?? HÀM M?I: Bóc tách d? li?u nút b?m t? ESP32 g?i xu?ng và d?y qua cho main x? lý ph?n c?ng
/** @brief H?ng l?nh ghi di?u khi?n t? Web xu?ng và ÐÓNG GÓI TR?NG THÁI TH?C T? vào gói ACK g?i ngu?c lên */
static bool handle_write_control(lora_node_t *node, const lora_packet_t *request) {
  if (node->config.log != NULL) { // ?? Ð?i sang NULL
    node->config.log("[LoRa Node] Nhan lenh nut nhan tu Web GUI do xuong\r\n");
  }

  if (request->payload_len == sizeof(lora_control_payload_t)) {
    // 1. Nap thang trang thai nut nhan vao vung nho chia se de main.c thuc thi phan cung that
    memcpy((void*)&g_remote_control, request->payload, sizeof(lora_control_payload_t));
    g_control_updated = 1; // Kich co bao cho main.c
  }

  // 2. THU HIEN BUOC 1 THEO DUNG LOGIC TRONG ANH: Dong goi trang thai nut bam hien tai vao struct
  lora_control_payload_t ack_payload;
  ack_payload.pump_status = g_remote_control.pump_status;
  ack_payload.pump_pwm    = g_remote_control.pump_pwm;
  ack_payload.roof_status = g_remote_control.roof_status;
  ack_payload.roof_pwm    = g_remote_control.roof_pwm;
  ack_payload.system_mode = g_remote_control.system_mode;

  // 3. Xay dung goi tin phan hoi mang ma lenh CMD_ACK (0x03) chua kem payload trang thai thuc te
  lora_packet_t response;
  if (!lora_packet_build(&response, request->src, request->dst,
                         CMD_ACK, request->seq, (const uint8_t*)&ack_payload, sizeof(ack_payload))) {
    if (node->config.log != NULL) { // ?? Ð?i sang NULL
      node->config.log("Build ACK response failed\r\n");
    }
    return false;
  }

  if (node->config.log != NULL) { // ?? Ð?i sang NULL
    node->config.log("[LoRa Node] Dang phat song nguoc goi CMD_ACK mang trang thai thuc te len ESP32...\r\n");
  }

  // 4. Phat song nguoc len cho ESP32 Gateway thong qua ham goc cua dong nghiep
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
		case CMD_WRITE_CONTROL:// them case de bat lenh dieu khien nut nhan
			handle_write_control(node, request);
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

  if (!node->radio.rx_pending()) {// kiem tra xem chan DIO có dc keo len k tuc la nhan dc song k, neu k co thoat de MCU lam nhung viec khac
    return;
  }

  uint8_t raw[LORA_PACKET_MAX_SIZE];     // loi mang byte du lieu tho ra                        
  int16_t rssi = 0;     // bien do do manh yeu cua song                                           
  const int rx_len = node->radio.receive(raw, sizeof(raw), &rssi); // do byte du lieu tho vao mang raw
  if (rx_len <= 0) {                                               
    if (rx_len < 0) {
      node_log(node, "RX CRC/error\r\n");// kiem tra coi co bi loi k
    }
    return;
  }

  node_log(node, "RX raw %d bytes rssi=%d\r\n", rx_len, (int)rssi);
// 3. Giai ma va kiem tra tem chong gia
  lora_packet_t pkt;
  if (!lora_packet_decode(raw, (size_t)rx_len, &pkt)) {
    node_log(node, "RX decode failed\r\n");
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
// neu dung la getway goi minh thì node do phai thuc thi menh lenh tu getway
  node->pending_request = pkt;// luu goi tin vao bien node->pending_request de node ghi nho
  node->state = LORA_NODE_STATE_PROCESS_REQUEST;// cap nhat trang thai sang trang thai (dang xu ly yeu cau)
  process_request(node, &pkt);//di doc cam bien va tra du lieu theo yeu cau, hoac phan hoi ACK neu la lenh PING
  node->state = LORA_NODE_STATE_RX_WAIT;// sau khi xu ly lenh cua getway xong thi node tro ve trang thai cho
}

lora_node_state_t lora_node_get_state(const lora_node_t* node) {// ham debug kiem tra trang thai cua node
  return node != NULL ? node->state : LORA_NODE_STATE_RX_WAIT;
}
