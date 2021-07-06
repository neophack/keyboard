#include <nvs_flash.h>

#include <cstring>

#include "esp_if/esp_if.hpp"
#include "ible.hpp"
#include "ihid.hpp"
#include "keyboard.hpp"
#include "matrix.hpp"
#include "object.hpp"
#include "supporting/mapping.hpp"
#include "transports.hpp"

using namespace kopinions;
using namespace kopinions::logging;

void hid_demo_task(void *pvParameters) {
  auto ios = new gpios_if;
  auto clk = new clock_if;
  auto mtx = new matrix(*ios, *clk);
  auto lay = new layout(layout_mapping);
  auto cfg = new config();
  auto kbd = new keyboard(*lay, *mtx, *cfg);
  auto sink = new esp_log_sink();
  auto lg = new logger(level::INFO, sink);
  auto trans = new transports();

  static const uint16_t s_primary_service_uuid = ESP_GATT_UUID_PRI_SERVICE;
  static const uint16_t s_include_service_uuid = ESP_GATT_UUID_INCLUDE_SERVICE;
  static const uint16_t s_character_declaration_uuid = ESP_GATT_UUID_CHAR_DECLARE;
  static const uint16_t s_character_client_config_uuid = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;

  static const uint16_t s_bat_level_uuid = ESP_GATT_UUID_BATTERY_LEVEL;
  static const uint16_t s_bat_char_pres_format_uuid = ESP_GATT_UUID_CHAR_PRESENT_FORMAT;
  static const uint8_t s_char_prop_read_notify = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_NOTIFY;
  static uint8_t bat_level = 1;
  auto b = new bt::ble("Chaos", bt::appearance_t::KEYBOARD, *lg);

  const bt::application_t &bat = bt::application_builder_t::name("kbd")
                                     ->id(0x180f)
                                     ->profile([](bt::profile_builder_t *p) {
                                       p->service([](bt::service_builder_t *s) {
                                         s->id(ESP_GATT_UUID_BATTERY_SERVICE_SVC);
                                         s->characteristic([](bt::characteristic_builder_t *c) {
                                           c->id(s_bat_level_uuid);
                                           c->automated(false);
                                           c->property(bt::characteristic_t::property_t::READ);
                                           c->property(bt::characteristic_t::property_t::NOTIFY);
                                           c->permission(bt::characteristic_t::permission_t::READ);
                                           c->value(&bat_level, 1, 1);
                                         });
                                       });
                                     })
                                     ->build();

  static const uint16_t primary_service_uuid = ESP_GATT_UUID_PRI_SERVICE;
  static const uint16_t include_service_uuid = ESP_GATT_UUID_INCLUDE_SERVICE;
  static const uint16_t character_declaration_uuid = ESP_GATT_UUID_CHAR_DECLARE;
  static const uint16_t character_client_config_uuid = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;
  static const uint16_t hid_info_char_uuid = ESP_GATT_UUID_HID_INFORMATION;
  static const uint16_t hid_report_map_uuid = ESP_GATT_UUID_HID_REPORT_MAP;
  static const uint16_t hid_control_point_uuid = ESP_GATT_UUID_HID_CONTROL_POINT;
  static const uint16_t hid_report_uuid = ESP_GATT_UUID_HID_REPORT;
  static const uint16_t hid_proto_mode_uuid = ESP_GATT_UUID_HID_PROTO_MODE;
  static const uint16_t hid_kb_input_uuid = ESP_GATT_UUID_HID_BT_KB_INPUT;
  static const uint16_t hid_kb_output_uuid = ESP_GATT_UUID_HID_BT_KB_OUTPUT;
  static const uint16_t hid_mouse_input_uuid = ESP_GATT_UUID_HID_BT_MOUSE_INPUT;
  static const uint16_t hid_repot_map_ext_desc_uuid = ESP_GATT_UUID_EXT_RPT_REF_DESCR;
  static const uint16_t hid_report_ref_descr_uuid = ESP_GATT_UUID_RPT_REF_DESCR;
  /// the propoty definition
  static const uint8_t char_prop_notify = ESP_GATT_CHAR_PROP_BIT_NOTIFY;
  static const uint8_t char_prop_read = ESP_GATT_CHAR_PROP_BIT_READ;
  static const uint8_t char_prop_write_nr = ESP_GATT_CHAR_PROP_BIT_WRITE_NR;
  static const uint8_t char_prop_read_write = ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_READ;
  static const uint8_t char_prop_read_notify = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_NOTIFY;
  static const uint8_t char_prop_read_write_notify =
      ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_NOTIFY;

  /// battary Service
  static const uint16_t battary_svc = ESP_GATT_UUID_BATTERY_SERVICE_SVC;

  static const uint16_t bat_lev_uuid = ESP_GATT_UUID_BATTERY_LEVEL;
  static const uint8_t bat_lev_ccc[2] = {0x00, 0x00};
  static const uint16_t char_format_uuid = ESP_GATT_UUID_CHAR_PRESENT_FORMAT;
#define ATT_SVC_HID 0x1812
#define ESP_HID_FLAGS_REMOTE_WAKE 0x01           // RemoteWake
#define ESP_HID_FLAGS_NORMALLY_CONNECTABLE 0x02  // NormallyConnectable
  static uint16_t hid_le_svc = ATT_SVC_HID;
  static const uint8_t hidInfo[4] = {
      0x11, 0x01,                                                     // bcdHID (USB HID version)
      0x00,                                                           // bCountryCode
      ESP_HID_FLAGS_REMOTE_WAKE | ESP_HID_FLAGS_NORMALLY_CONNECTABLE  // Flags
  };
  /// HID Information structure
  typedef struct {
    /// bcdHID
    uint16_t bcdHID;
    /// bCountryCode
    uint8_t bCountryCode;
    /// Flags
    uint8_t flags;
  } hids_hid_info_t;

  static const unsigned char hidReportMap[] = {
      0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
      0x09, 0x02,        // Usage (Mouse)
      0xA1, 0x01,        // Collection (Application)
      0x85, 0x01,        //   Report ID (1)
      0x09, 0x01,        //   Usage (Pointer)
      0xA1, 0x00,        //   Collection (Physical)
      0x05, 0x09,        //     Usage Page (Button)
      0x19, 0x01,        //     Usage Minimum (0x01)
      0x29, 0x03,        //     Usage Maximum (0x03)
      0x15, 0x00,        //     Logical Minimum (0)
      0x25, 0x01,        //     Logical Maximum (1)
      0x75, 0x01,        //     Report Size (1)
      0x95, 0x03,        //     Report Count (3)
      0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
      0x75, 0x05,        //     Report Size (5)
      0x95, 0x01,        //     Report Count (1)
      0x81, 0x01,        //     Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
      0x05, 0x01,        //     Usage Page (Generic Desktop Ctrls)
      0x09, 0x30,        //     Usage (X)
      0x09, 0x31,        //     Usage (Y)
      0x09, 0x38,        //     Usage (Wheel)
      0x15, 0x81,        //     Logical Minimum (-127)
      0x25, 0x7F,        //     Logical Maximum (127)
      0x75, 0x08,        //     Report Size (8)
      0x95, 0x03,        //     Report Count (3)
      0x81, 0x06,        //     Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position)
      0xC0,              //   End Collection
      0xC0,              // End Collection

      0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
      0x09, 0x06,        // Usage (Keyboard)
      0xA1, 0x01,        // Collection (Application)
      0x85, 0x02,        //   Report ID (2)
      0x05, 0x07,        //   Usage Page (Kbrd/Keypad)
      0x19, 0xE0,        //   Usage Minimum (0xE0)
      0x29, 0xE7,        //   Usage Maximum (0xE7)
      0x15, 0x00,        //   Logical Minimum (0)
      0x25, 0x01,        //   Logical Maximum (1)
      0x75, 0x01,        //   Report Size (1)
      0x95, 0x08,        //   Report Count (8)
      0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
      0x95, 0x01,        //   Report Count (1)
      0x75, 0x08,        //   Report Size (8)
      0x81, 0x01,        //   Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
      0x95, 0x05,        //   Report Count (5)
      0x75, 0x01,        //   Report Size (1)
      0x05, 0x08,        //   Usage Page (LEDs)
      0x19, 0x01,        //   Usage Minimum (Num Lock)
      0x29, 0x05,        //   Usage Maximum (Kana)
      0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
      0x95, 0x01,        //   Report Count (1)
      0x75, 0x03,        //   Report Size (3)
      0x91, 0x01,        //   Output (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
      0x95, 0x06,        //   Report Count (6)
      0x75, 0x08,        //   Report Size (8)
      0x15, 0x00,        //   Logical Minimum (0)
      0x25, 0x65,        //   Logical Maximum (101)
      0x05, 0x07,        //   Usage Page (Kbrd/Keypad)
      0x19, 0x00,        //   Usage Minimum (0x00)
      0x29, 0x65,        //   Usage Maximum (0x65)
      0x81, 0x00,        //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
      0xC0,              // End Collection

      0x05, 0x0C,        // Usage Page (Consumer)
      0x09, 0x01,        // Usage (Consumer Control)
      0xA1, 0x01,        // Collection (Application)
      0x85, 0x03,        //   Report ID (3)
      0x09, 0x02,        //   Usage (Numeric Key Pad)
      0xA1, 0x02,        //   Collection (Logical)
      0x05, 0x09,        //     Usage Page (Button)
      0x19, 0x01,        //     Usage Minimum (0x01)
      0x29, 0x0A,        //     Usage Maximum (0x0A)
      0x15, 0x01,        //     Logical Minimum (1)
      0x25, 0x0A,        //     Logical Maximum (10)
      0x75, 0x04,        //     Report Size (4)
      0x95, 0x01,        //     Report Count (1)
      0x81, 0x00,        //     Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
      0xC0,              //   End Collection
      0x05, 0x0C,        //   Usage Page (Consumer)
      0x09, 0x86,        //   Usage (Channel)
      0x15, 0xFF,        //   Logical Minimum (-1)
      0x25, 0x01,        //   Logical Maximum (1)
      0x75, 0x02,        //   Report Size (2)
      0x95, 0x01,        //   Report Count (1)
      0x81, 0x46,        //   Input (Data,Var,Rel,No Wrap,Linear,Preferred State,Null State)
      0x09, 0xE9,        //   Usage (Volume Increment)
      0x09, 0xEA,        //   Usage (Volume Decrement)
      0x15, 0x00,        //   Logical Minimum (0)
      0x75, 0x01,        //   Report Size (1)
      0x95, 0x02,        //   Report Count (2)
      0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
      0x09, 0xE2,        //   Usage (Mute)
      0x09, 0x30,        //   Usage (Power)
      0x09, 0x83,        //   Usage (Recall Last)
      0x09, 0x81,        //   Usage (Assign Selection)
      0x09, 0xB0,        //   Usage (Play)
      0x09, 0xB1,        //   Usage (Pause)
      0x09, 0xB2,        //   Usage (Record)
      0x09, 0xB3,        //   Usage (Fast Forward)
      0x09, 0xB4,        //   Usage (Rewind)
      0x09, 0xB5,        //   Usage (Scan Next Track)
      0x09, 0xB6,        //   Usage (Scan Previous Track)
      0x09, 0xB7,        //   Usage (Stop)
      0x15, 0x01,        //   Logical Minimum (1)
      0x25, 0x0C,        //   Logical Maximum (12)
      0x75, 0x04,        //   Report Size (4)
      0x95, 0x01,        //   Report Count (1)
      0x81, 0x00,        //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
      0x09, 0x80,        //   Usage (Selection)
      0xA1, 0x02,        //   Collection (Logical)
      0x05, 0x09,        //     Usage Page (Button)
      0x19, 0x01,        //     Usage Minimum (0x01)
      0x29, 0x03,        //     Usage Maximum (0x03)
      0x15, 0x01,        //     Logical Minimum (1)
      0x25, 0x03,        //     Logical Maximum (3)
      0x75, 0x02,        //     Report Size (2)
      0x81, 0x00,        //     Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
      0xC0,              //   End Collection
      0x81, 0x03,        //   Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
      0xC0,              // End Collection

      0x06, 0xFF, 0xFF,  // Usage Page (Vendor Defined 0xFFFF)
      0x09, 0xA5,        // Usage (0xA5)
      0xA1, 0x01,        // Collection (Application)
      0x85, 0x04,        //   Report ID (4)
      0x09, 0xA6,        //   Usage (0xA6)
      0x09, 0xA9,        //   Usage (0xA9)
      0x75, 0x08,        //   Report Size (8)
      0x95, 0x7F,        //   Report Count (127)
      0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
      0xC0,              // End Collection

      // 250 bytes
  };
  static uint16_t hidExtReportRefDesc = ESP_GATT_UUID_BATTERY_LEVEL;

  static esp_gatts_incl_svc_desc_t incl_svc = {0};
#define HID_PROTOCOL_MODE_REPORT        0x01      // Report Protocol Mode
  static uint8_t hidProtocolMode = HID_PROTOCOL_MODE_REPORT;
  const bt::application_t &hid =
      bt::application_builder_t::name("hid")
          ->id(0x1812)
          ->profile([](bt::profile_builder_t *p) {
            p->service([](bt::service_builder_t *s) {
              s->id(ESP_GATT_UUID_BATTERY_SERVICE_SVC);
              s->characteristic([](bt::characteristic_builder_t *c) {
                c->id(include_service_uuid);
                c->automated(true);
                c->property(bt::characteristic_t::property_t::READ);
                c->permission(bt::characteristic_t::permission_t::READ_ENCRYPTED);
                c->value(reinterpret_cast<uint8_t *>(&incl_svc), sizeof(esp_gatts_incl_svc_desc_t),
                         sizeof(esp_gatts_incl_svc_desc_t));
              });
              s->characteristic([](bt::characteristic_builder_t *c) {
                c->id(hid_info_char_uuid);
                c->automated(true);
                c->property(bt::characteristic_t::property_t::READ);
                c->property(bt::characteristic_t::property_t::NOTIFY);
                c->permission(bt::characteristic_t::permission_t::READ);
                c->value(const_cast<uint8_t *>(hidInfo), sizeof(hidInfo), sizeof(hids_hid_info_t));
              });
              s->characteristic([](bt::characteristic_builder_t *c) {
                c->id(hid_control_point_uuid);
                c->automated(true);
                c->property(bt::characteristic_t::property_t::READ);
                c->property(bt::characteristic_t::property_t::NOTIFY);
                c->permission(bt::characteristic_t::permission_t::WRITE);
                c->value(nullptr, 0, sizeof(uint8_t));
              });
              s->characteristic([](bt::characteristic_builder_t *c) {
                c->id(hid_report_map_uuid);
                c->automated(true);
                c->property(bt::characteristic_t::property_t::READ);
                c->property(bt::characteristic_t::property_t::NOTIFY);
                c->permission(bt::characteristic_t::permission_t::READ);
                c->value(const_cast<uint8_t *>(hidReportMap), sizeof(hidReportMap), HIDD_LE_REPORT_MAP_MAX_LEN);
              });
              s->characteristic([](bt::characteristic_builder_t *c) {
                c->id(hid_repot_map_ext_desc_uuid);
                c->automated(true);
                c->property(bt::characteristic_t::property_t::READ);
                c->property(bt::characteristic_t::property_t::NOTIFY);
                c->permission(bt::characteristic_t::permission_t::READ);
                c->value(reinterpret_cast<uint8_t *>(&hidExtReportRefDesc), sizeof(uint16_t), sizeof(uint16_t));
              });
              s->characteristic([](bt::characteristic_builder_t *c) {
                c->id(hid_proto_mode_uuid);
                c->automated(true);
                c->property(bt::characteristic_t::property_t::READ);
                c->property(bt::characteristic_t::property_t::NOTIFY);
                c->permission(bt::characteristic_t::permission_t::READ);
                c->value(reinterpret_cast<uint8_t *>(&hidProtocolMode), sizeof(hidProtocolMode), sizeof(uint8_t));
              });
              s->characteristic([](bt::characteristic_builder_t *c) {
                c->id(hid_report_uuid);
                c->automated(true);
                c->property(bt::characteristic_t::property_t::READ);
                c->property(bt::characteristic_t::property_t::NOTIFY);
                c->permission(bt::characteristic_t::permission_t::READ);
                c->value(nullptr, 0, HIDD_LE_REPORT_MAX_LEN);
              });
              // TODO client config descriptor should not add the character declaration
              s->characteristic([](bt::characteristic_builder_t *c) {
                c->id(character_client_config_uuid);
                c->automated(true);
                c->property(bt::characteristic_t::property_t::READ);
                c->property(bt::characteristic_t::property_t::WRITE);
                c->permission(bt::characteristic_t::permission_t::READ);
                c->value(nullptr, 0, sizeof(uint16_t));
              });
            });
          })
          ->build();

  //  // Report KEY INPUT Characteristic - Client Characteristic Configuration Descriptor
  //      [HIDD_LE_IDX_REPORT_KEY_IN_CCC] = {{ESP_GATT_AUTO_RSP},
  //                                         {ESP_UUID_LEN_16, (uint8_t *)&character_client_config_uuid,
  //                                          (ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE), sizeof(uint16_t), 0, NULL}},
  //  // Report Characteristic - Report Reference Descriptor
  //      [HIDD_LE_IDX_REPORT_KEY_IN_REP_REF] = {{ESP_GATT_AUTO_RSP},
  //                                             {ESP_UUID_LEN_16, (uint8_t *)&hid_report_ref_descr_uuid,
  //                                              ESP_GATT_PERM_READ, sizeof(hidReportRefKeyIn),
  //                                              sizeof(hidReportRefKeyIn), hidReportRefKeyIn}},
  //
  //  // Report Characteristic Declaration
  //      [HIDD_LE_IDX_REPORT_LED_OUT_CHAR] = {{ESP_GATT_AUTO_RSP},
  //                                           {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid,
  //                                           ESP_GATT_PERM_READ,
  //                                            CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE,
  //                                            (uint8_t *)&char_prop_read_write}},
  //
  //  [HIDD_LE_IDX_REPORT_LED_OUT_VAL] = {{ESP_GATT_AUTO_RSP},
  //                                      {ESP_UUID_LEN_16, (uint8_t *)&hid_report_uuid,
  //                                       ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE, HIDD_LE_REPORT_MAX_LEN, 0, NULL}},
  //  [HIDD_LE_IDX_REPORT_LED_OUT_REP_REF] = {{ESP_GATT_AUTO_RSP},
  //                                          {ESP_UUID_LEN_16, (uint8_t *)&hid_report_ref_descr_uuid,
  //                                          ESP_GATT_PERM_READ,
  //                                           sizeof(hidReportRefLedOut), sizeof(hidReportRefLedOut),
  //                                           hidReportRefLedOut}},

  //  // Report Characteristic Declaration
  //      [HIDD_LE_IDX_REPORT_CC_IN_CHAR] = {{ESP_GATT_AUTO_RSP},
  //                                         {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid,
  //                                         ESP_GATT_PERM_READ,
  //                                          CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE,
  //                                          (uint8_t *)&char_prop_read_notify}},
  //  // Report Characteristic Value
  //      [HIDD_LE_IDX_REPORT_CC_IN_VAL] = {{ESP_GATT_AUTO_RSP},
  //                                        {ESP_UUID_LEN_16, (uint8_t *)&hid_report_uuid, ESP_GATT_PERM_READ,
  //                                         HIDD_LE_REPORT_MAX_LEN, 0, NULL}},
  //  // Report KEY INPUT Characteristic - Client Characteristic Configuration Descriptor
  //      [HIDD_LE_IDX_REPORT_CC_IN_CCC] = {{ESP_GATT_AUTO_RSP},
  //                                        {ESP_UUID_LEN_16, (uint8_t *)&character_client_config_uuid,
  //                                         (ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE_ENCRYPTED), sizeof(uint16_t), 0,
  //                                         NULL}},
  //  // Report Characteristic - Report Reference Descriptor
  //      [HIDD_LE_IDX_REPORT_CC_IN_REP_REF] = {{ESP_GATT_AUTO_RSP},
  //                                            {ESP_UUID_LEN_16, (uint8_t *)&hid_report_ref_descr_uuid,
  //                                            ESP_GATT_PERM_READ,
  //                                             sizeof(hidReportRefCCIn), sizeof(hidReportRefCCIn), hidReportRefCCIn}},
  //
  //  // Boot Keyboard Input Report Characteristic Declaration
  //      [HIDD_LE_IDX_BOOT_KB_IN_REPORT_CHAR] = {{ESP_GATT_AUTO_RSP},
  //                                              {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid,
  //                                               ESP_GATT_PERM_READ, CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE,
  //                                               (uint8_t *)&char_prop_read_notify}},
  //  // Boot Keyboard Input Report Characteristic Value
  //      [HIDD_LE_IDX_BOOT_KB_IN_REPORT_VAL] = {{ESP_GATT_AUTO_RSP},
  //                                             {ESP_UUID_LEN_16, (uint8_t *)&hid_kb_input_uuid, ESP_GATT_PERM_READ,
  //                                              HIDD_LE_BOOT_REPORT_MAX_LEN, 0, NULL}},
  //  // Boot Keyboard Input Report Characteristic - Client Characteristic Configuration Descriptor
  //      [HIDD_LE_IDX_BOOT_KB_IN_REPORT_NTF_CFG] = {{ESP_GATT_AUTO_RSP},
  //                                                 {ESP_UUID_LEN_16, (uint8_t *)&character_client_config_uuid,
  //                                                  (ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE), sizeof(uint16_t), 0,
  //                                                  NULL}},
  //
  //  // Boot Keyboard Output Report Characteristic Declaration
  //      [HIDD_LE_IDX_BOOT_KB_OUT_REPORT_CHAR] = {{ESP_GATT_AUTO_RSP},
  //                                               {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid,
  //                                                ESP_GATT_PERM_READ, CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE,
  //                                                (uint8_t *)&char_prop_read_write}},
  //  // Boot Keyboard Output Report Characteristic Value
  //      [HIDD_LE_IDX_BOOT_KB_OUT_REPORT_VAL] = {{ESP_GATT_AUTO_RSP},
  //                                              {ESP_UUID_LEN_16, (uint8_t *)&hid_kb_output_uuid,
  //                                               (ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE),
  //                                               HIDD_LE_BOOT_REPORT_MAX_LEN, 0, NULL}},

  //
  //  // Report Characteristic Declaration
  //      [HIDD_LE_IDX_REPORT_CHAR] = {{ESP_GATT_AUTO_RSP},
  //                                   {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
  //                                    CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t
  //                                    *)&char_prop_read_write}},
  //  // Report Characteristic Value
  //      [HIDD_LE_IDX_REPORT_VAL] = {{ESP_GATT_AUTO_RSP},
  //                                  {ESP_UUID_LEN_16, (uint8_t *)&hid_report_uuid, ESP_GATT_PERM_READ,
  //                                   HIDD_LE_REPORT_MAX_LEN, 0, NULL}},
  //  // Report Characteristic - Report Reference Descriptor
  //      [HIDD_LE_IDX_REPORT_REP_REF] = {{ESP_GATT_AUTO_RSP},
  //                                      {ESP_UUID_LEN_16, (uint8_t *)&hid_report_ref_descr_uuid, ESP_GATT_PERM_READ,
  //                                       sizeof(hidReportRefFeature), sizeof(hidReportRefFeature),
  //                                       hidReportRefFeature}},
  std::cout << bat.stringify();
  std::cout << hid.stringify();
  b->enroll(bat);
  b->enroll(hid);

  while (true) {
    lg->log(level::INFO, "%s %d", "xxx1111", 222);

    auto &&res = kbd->scan();
    auto tx = trans->select();

    for (auto b : res) {
      auto status = b.sts;
      lg->log(level::DEBUG, "%d", status);
    }

    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

extern "C" void app_main() {
  auto ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  xTaskCreate(&hid_demo_task, "hid_task", 3500, NULL, 5, NULL);

  //  auto sche = new scheduler_if<>;
  //
  //  sche->schedule("test", []() -> void {
  //    auto ios = std::make_shared<gpios_if>();
  //    auto clk = std::make_shared<clock_if>();
  //    auto mtx = std::make_shared<matrix>(ios, clk);
  //    auto lay = std::make_shared<layout>(layout_mapping);
  //    auto cfg = std::make_shared<config>();
  //    auto kbd = std::make_shared<keyboard>(lay, mtx, cfg);
  //    auto lg = std::make_shared<logger>(level::INFO, std::make_shared<esp_log_sink>());
  //    auto trans = std::make_shared<transports>();
  //    //    const std::shared_ptr<endpoint>& ep = std::make_shared<endpoint>();
  //    const std::shared_ptr<bt::ble>& b = std::make_shared<bt::ble>("Chaos", bt::appearance_t::KEYBOARD, lg);
  //
  //    static const uint16_t s_primary_service_uuid = ESP_GATT_UUID_PRI_SERVICE;
  //    static const uint16_t s_include_service_uuid = ESP_GATT_UUID_INCLUDE_SERVICE;
  //    static const uint16_t s_character_declaration_uuid = ESP_GATT_UUID_CHAR_DECLARE;
  //    static const uint16_t s_character_client_config_uuid = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;
  //
  //    static const uint16_t s_bat_level_uuid = ESP_GATT_UUID_BATTERY_LEVEL;
  //    static const uint16_t s_bat_char_pres_format_uuid = ESP_GATT_UUID_CHAR_PRESENT_FORMAT;
  //    static const uint8_t s_char_prop_read_notify = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_NOTIFY;
  //    static uint8_t bat_level = 1;
  //
  //    //    const bt::application_t& app = bt::application_builder_t::name("kbd")
  //    //                                       ->id(0x0001)
  //    //                                       ->profile([](bt::profile_builder_t* p) {
  //    //                                         p->service([](bt::service_builder_t* s) {
  //    //                                           s->id(ESP_GATT_UUID_BATTERY_SERVICE_SVC);
  //    //                                           s->characteristic([](bt::characteristic_builder_t* c) {
  //    //                                             c->id(s_bat_level_uuid);
  //    //                                             c->property(bt::characteristic_t::property_t::READ);
  //    //                                             c->property(bt::characteristic_t::property_t::NOTIFY);
  //    //                                             c->permission(bt::characteristic_t::permission_t::READ);
  //    //                                             c->value(&bat_level, 1, 1);
  //    //                                           });
  //    //                                         });
  //    //                                       })
  //    //                                       ->build();
  //    //    b->enroll(app);
  //
  //    while (true) {
  //      lg->log(level::DEBUG, "%s %d", "xxx1111", 222);
  //
  //      auto&& res = kbd->scan();
  //      trans->select();
  //
  //      for (auto b : res) {
  //        auto status = b.sts;
  //        lg->log(level::DEBUG, "%d", status);
  //      }
  //
  //      vTaskDelay(100);
  //    }
  //  });
  //  while (true) {
  //    vTaskDelay(1000 / portTICK_PERIOD_MS);
  //  }
}