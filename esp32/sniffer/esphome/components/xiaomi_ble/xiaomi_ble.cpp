#include "xiaomi_ble.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"

#ifdef ARDUINO_ARCH_ESP32

#include <vector>
#include "mbedtls/ccm.h"

namespace esphome {
namespace xiaomi_ble {

static const char *TAG = "xiaomi_ble";

bool parse_xiaomi_message(const std::vector<uint8_t> &message, XiaomiParseResult &result) {
  result.has_encryption = (message[0] & 0x08) ? true : false;  // update encryption status
  if (result.has_encryption) {
    ESP_LOGVV(TAG, "parse_xiaomi_message(): payload is encrypted, stop reading message.");
    return false;
  }

  // Data point specs
  // Byte 0: type
  // Byte 1: fixed 0x10
  // Byte 2: length
  // Byte 3..3+len-1: data point value

  const uint8_t *raw = message.data() + result.raw_offset;
  const uint8_t *data = raw + 3;
  const uint8_t data_length = raw[2];

  if ((data_length < 1) || (data_length > 4)) {
    ESP_LOGVV(TAG, "parse_xiaomi_message(): payload has wrong size (%d)!", data_length);
    return false;
  }

  // motion detection, 1 byte, 8-bit unsigned integer
  if ((raw[0] == 0x03) && (data_length == 1)) {
    result.has_motion = (data[0]) ? true : false;
  }
  // temperature, 2 bytes, 16-bit signed integer (LE), 0.1 °C
  else if ((raw[0] == 0x04) && (data_length == 2)) {
    const int16_t temperature = uint16_t(data[0]) | (uint16_t(data[1]) << 8);
    result.temperature = temperature / 10.0f;
  }
  // humidity, 2 bytes, 16-bit signed integer (LE), 0.1 %
  else if ((raw[0] == 0x06) && (data_length == 2)) {
    const int16_t humidity = uint16_t(data[0]) | (uint16_t(data[1]) << 8);
    result.humidity = humidity / 10.0f;
  }
  // illuminance (+ motion), 3 bytes, 24-bit unsigned integer (LE), 1 lx
  else if (((raw[0] == 0x07) || (raw[0] == 0x0F)) && (data_length == 3)) {
    const uint32_t illuminance = uint32_t(data[0]) | (uint32_t(data[1]) << 8) | (uint32_t(data[2]) << 16);
    result.illuminance = illuminance;
    result.is_light = (illuminance == 100) ? true : false;
    if (raw[0] == 0x0F)
      result.has_motion = true;
  }
  // soil moisture, 1 byte, 8-bit unsigned integer, 1 %
  else if ((raw[0] == 0x08) && (data_length == 1)) {
    result.moisture = data[0];
  }
  // conductivity, 2 bytes, 16-bit unsigned integer (LE), 1 µS/cm
  else if ((raw[0] == 0x09) && (data_length == 2)) {
    const uint16_t conductivity = uint16_t(data[0]) | (uint16_t(data[1]) << 8);
    result.conductivity = conductivity;
  }
  // battery, 1 byte, 8-bit unsigned integer, 1 %
  else if ((raw[0] == 0x0A) && (data_length == 1)) {
    result.battery_level = data[0];
  }
  // temperature + humidity, 4 bytes, 16-bit signed integer (LE) each, 0.1 °C, 0.1 %
  else if ((raw[0] == 0x0D) && (data_length == 4)) {
    const int16_t temperature = uint16_t(data[0]) | (uint16_t(data[1]) << 8);
    const int16_t humidity = uint16_t(data[2]) | (uint16_t(data[3]) << 8);
    result.temperature = temperature / 10.0f;
    result.humidity = humidity / 10.0f;
  }
  // formaldehyde, 2 bytes, 16-bit unsigned integer (LE), 0.01 mg / m3
  else if ((raw[0] == 0x10) && (data_length == 2)) {
    const uint16_t formaldehyde = uint16_t(data[0]) | (uint16_t(data[1]) << 8);
    result.formaldehyde = formaldehyde / 100.0f;
  }
  // on/off state, 1 byte, 8-bit unsigned integer
  else if ((raw[0] == 0x12) && (data_length == 1)) {
    result.is_active = (data[0]) ? true : false;
  }
  // mosquito tablet, 1 byte, 8-bit unsigned integer, 1 %
  else if ((raw[0] == 0x13) && (data_length == 1)) {
    result.tablet = data[0];
  }
  // idle time since last motion, 4 byte, 32-bit unsigned integer, 1 min
  else if ((raw[0] == 0x17) && (data_length == 4)) {
    const uint32_t idle_time =
        uint32_t(data[0]) | (uint32_t(data[1]) << 8) | (uint32_t(data[2]) << 16) | (uint32_t(data[2]) << 24);
    result.idle_time = idle_time / 60.0f;
    result.has_motion = (idle_time) ? false : true;
  } else {
    return false;
  }

  return true;
}

optional<XiaomiParseResult> parse_xiaomi_header(const esp32_ble_tracker::ServiceData &service_data) {
  XiaomiParseResult result;
  if (!service_data.uuid.contains(0x95, 0xFE)) {
    ESP_LOGVV(TAG, "parse_xiaomi_header(): no service data UUID magic bytes.");
    return {};
  }

  auto raw = service_data.data;
  result.has_data = (raw[0] & 0x40) ? true : false;
  result.has_capability = (raw[0] & 0x20) ? true : false;
  result.has_encryption = (raw[0] & 0x08) ? true : false;

  if (!result.has_data) {
    ESP_LOGVV(TAG, "parse_xiaomi_header(): service data has no DATA flag.");
    return {};
  }

  static uint8_t last_frame_count = 0;
  if (last_frame_count == raw[4]) {
    ESP_LOGVV(TAG, "parse_xiaomi_header(): duplicate data packet received (%d).", static_cast<int>(last_frame_count));
    result.is_duplicate = true;
    return {};
  }
  last_frame_count = raw[4];
  result.is_duplicate = false;
  result.raw_offset = result.has_capability ? 12 : 11;

  if ((raw[2] == 0x98) && (raw[3] == 0x00)) {  // MiFlora
    result.type = XiaomiParseResult::TYPE_HHCCJCY01;
    result.name = "HHCCJCY01";
  } else if ((raw[2] == 0xaa) && (raw[3] == 0x01)) {  // round body, segment LCD
    result.type = XiaomiParseResult::TYPE_LYWSDCGQ;
    result.name = "LYWSDCGQ";
  } else if ((raw[2] == 0x5d) && (raw[3] == 0x01)) {  // FlowerPot, RoPot
    result.type = XiaomiParseResult::TYPE_HHCCPOT002;
    result.name = "HHCCPOT002";
  } else if ((raw[2] == 0xdf) && (raw[3] == 0x02)) {  // Xiaomi (Honeywell) formaldehyde sensor, OLED display
    result.type = XiaomiParseResult::TYPE_JQJCY01YM;
    result.name = "JQJCY01YM";
  } else if ((raw[2] == 0xdd) && (raw[3] == 0x03)) {  // Philips/Xiaomi BLE nightlight
    result.type = XiaomiParseResult::TYPE_MUE4094RT;
    result.name = "MUE4094RT";
    result.raw_offset -= 6;
  } else if ((raw[2] == 0x47) && (raw[3] == 0x03)) {  // round body, e-ink display
    result.type = XiaomiParseResult::TYPE_CGG1;
    result.name = "CGG1";
  } else if ((raw[2] == 0xbc) && (raw[3] == 0x03)) {  // VegTrug Grow Care Garden
    result.type = XiaomiParseResult::TYPE_GCLS002;
    result.name = "GCLS002";
  } else if ((raw[2] == 0x5b) && (raw[3] == 0x04)) {  // rectangular body, e-ink display
    result.type = XiaomiParseResult::TYPE_LYWSD02;
    result.name = "LYWSD02";
  } else if ((raw[2] == 0x0a) && (raw[3] == 0x04)) {  // Mosquito Repellent Smart Version
    result.type = XiaomiParseResult::TYPE_WX08ZM;
    result.name = "WX08ZM";
  } else if ((raw[2] == 0x76) && (raw[3] == 0x05)) {  // Cleargrass (Qingping) alarm clock, segment LCD
    result.type = XiaomiParseResult::TYPE_CGD1;
    result.name = "CGD1";
  } else if ((raw[2] == 0x5b) && (raw[3] == 0x05)) {  // small square body, segment LCD, encrypted
    result.type = XiaomiParseResult::TYPE_LYWSD03MMC;
    result.name = "LYWSD03MMC";
  } else if ((raw[2] == 0xf6) && (raw[3] == 0x07)) {  // Xiaomi-Yeelight BLE nightlight
    result.type = XiaomiParseResult::TYPE_MJYD02YLA;
    result.name = "MJYD02YLA";
    if (raw.size() == 19)
      result.raw_offset -= 6;
  } else {
    ESP_LOGVV(TAG, "parse_xiaomi_header(): unknown device, no magic bytes.");
    return {};
  }

  return result;
}

bool decrypt_xiaomi_payload(std::vector<uint8_t> &raw, const uint8_t *bindkey, const uint64_t &address) {
  if (!((raw.size() == 19) || ((raw.size() >= 22) && (raw.size() <= 24)))) {
    ESP_LOGVV(TAG, "decrypt_xiaomi_payload(): data packet has wrong size (%d)!", raw.size());
    ESP_LOGVV(TAG, "  Packet : %s", hexencode(raw.data(), raw.size()).c_str());
    return false;
  }

  uint8_t mac_reverse[6] = {0};
  mac_reverse[5] = (uint8_t)(address >> 40);
  mac_reverse[4] = (uint8_t)(address >> 32);
  mac_reverse[3] = (uint8_t)(address >> 24);
  mac_reverse[2] = (uint8_t)(address >> 16);
  mac_reverse[1] = (uint8_t)(address >> 8);
  mac_reverse[0] = (uint8_t)(address >> 0);

  XiaomiAESVector vector{.key = {0},
                         .plaintext = {0},
                         .ciphertext = {0},
                         .authdata = {0x11},
                         .iv = {0},
                         .tag = {0},
                         .keysize = 16,
                         .authsize = 1,
                         .datasize = 0,
                         .tagsize = 4,
                         .ivsize = 12};

  vector.datasize = (raw.size() == 19) ? raw.size() - 12 : raw.size() - 18;
  int cipher_pos = (raw.size() == 19) ? 5 : 11;

  const uint8_t *v = raw.data();

  memcpy(vector.key, bindkey, vector.keysize);
  memcpy(vector.ciphertext, v + cipher_pos, vector.datasize);
  memcpy(vector.tag, v + raw.size() - vector.tagsize, vector.tagsize);
  memcpy(vector.iv, mac_reverse, 6);             // MAC address reverse
  memcpy(vector.iv + 6, v + 2, 3);               // sensor type (2) + packet id (1)
  memcpy(vector.iv + 9, v + raw.size() - 7, 3);  // payload counter

  mbedtls_ccm_context ctx;
  mbedtls_ccm_init(&ctx);

  int ret = mbedtls_ccm_setkey(&ctx, MBEDTLS_CIPHER_ID_AES, vector.key, vector.keysize * 8);
  if (ret) {
    ESP_LOGVV(TAG, "decrypt_xiaomi_payload(): mbedtls_ccm_setkey() failed.");
    mbedtls_ccm_free(&ctx);
    return false;
  }

  ret = mbedtls_ccm_auth_decrypt(&ctx, vector.datasize, vector.iv, vector.ivsize, vector.authdata, vector.authsize,
                                 vector.ciphertext, vector.plaintext, vector.tag, vector.tagsize);
  if (ret) {
    uint8_t mac_address[6] = {0};
    memcpy(mac_address, mac_reverse + 5, 1);
    memcpy(mac_address + 1, mac_reverse + 4, 1);
    memcpy(mac_address + 2, mac_reverse + 3, 1);
    memcpy(mac_address + 3, mac_reverse + 2, 1);
    memcpy(mac_address + 4, mac_reverse + 1, 1);
    memcpy(mac_address + 5, mac_reverse, 1);
    ESP_LOGVV(TAG, "decrypt_xiaomi_payload(): authenticated decryption failed.");
    ESP_LOGVV(TAG, "  MAC address : %s", hexencode(mac_address, 6).c_str());
    ESP_LOGVV(TAG, "       Packet : %s", hexencode(raw.data(), raw.size()).c_str());
    ESP_LOGVV(TAG, "          Key : %s", hexencode(vector.key, vector.keysize).c_str());
    ESP_LOGVV(TAG, "           Iv : %s", hexencode(vector.iv, vector.ivsize).c_str());
    ESP_LOGVV(TAG, "       Cipher : %s", hexencode(vector.ciphertext, vector.datasize).c_str());
    ESP_LOGVV(TAG, "          Tag : %s", hexencode(vector.tag, vector.tagsize).c_str());
    mbedtls_ccm_free(&ctx);
    return false;
  }

  // replace encrypted payload with plaintext
  uint8_t *p = vector.plaintext;
  for (std::vector<uint8_t>::iterator it = raw.begin() + cipher_pos; it != raw.begin() + cipher_pos + vector.datasize;
       ++it) {
    *it = *(p++);
  }

  // clear encrypted flag
  raw[0] &= ~0x08;

  ESP_LOGVV(TAG, "decrypt_xiaomi_payload(): authenticated decryption passed.");
  ESP_LOGVV(TAG, "  Plaintext : %s, Packet : %d", hexencode(raw.data() + cipher_pos, vector.datasize).c_str(),
            static_cast<int>(raw[4]));

  mbedtls_ccm_free(&ctx);
  return true;
}

bool report_xiaomi_results(const optional<XiaomiParseResult> &result, const std::string &address) {
  if (!result.has_value()) {
    ESP_LOGVV(TAG, "report_xiaomi_results(): no results available.");
    return false;
  }

  ESP_LOGD(TAG, "Got Xiaomi %s (%s):", result->name.c_str(), address.c_str());

  if (result->temperature.has_value()) {
    ESP_LOGD(TAG, "  Temperature: %.1f°C", *result->temperature);
  }
  if (result->humidity.has_value()) {
    ESP_LOGD(TAG, "  Humidity: %.1f%%", *result->humidity);
  }
  if (result->battery_level.has_value()) {
    ESP_LOGD(TAG, "  Battery Level: %.0f%%", *result->battery_level);
  }
  if (result->conductivity.has_value()) {
    ESP_LOGD(TAG, "  Conductivity: %.0fµS/cm", *result->conductivity);
  }
  if (result->illuminance.has_value()) {
    ESP_LOGD(TAG, "  Illuminance: %.0flx", *result->illuminance);
  }
  if (result->moisture.has_value()) {
    ESP_LOGD(TAG, "  Moisture: %.0f%%", *result->moisture);
  }
  if (result->tablet.has_value()) {
    ESP_LOGD(TAG, "  Mosquito tablet: %.0f%%", *result->tablet);
  }
  if (result->is_active.has_value()) {
    ESP_LOGD(TAG, "  Repellent: %s", (*result->is_active) ? "on" : "off");
  }
  if (result->has_motion.has_value()) {
    ESP_LOGD(TAG, "  Motion: %s", (*result->has_motion) ? "yes" : "no");
  }
  if (result->is_light.has_value()) {
    ESP_LOGD(TAG, "  Light: %s", (*result->is_light) ? "on" : "off");
  }

  return true;
}

bool XiaomiListener::parse_device(const esp32_ble_tracker::ESPBTDevice &device) {
  // Previously the message was parsed twice per packet, once by XiaomiListener::parse_device()
  // and then again by the respective device class's parse_device() function. Parsing the header
  // here and then for each device seems to be unneccessary and complicates the duplicate packet filtering.
  // Hence I disabled the call to parse_xiaomi_header() here and the message parsing is done entirely
  // in the respecive device instance. The XiaomiListener class is defined in __init__.py and I was not
  // able to remove it entirely.

  return false;  // with true it's not showing device scans
}

}  // namespace xiaomi_ble
}  // namespace esphome

#endif
