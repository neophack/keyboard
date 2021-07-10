#include "ible/visitors.hpp"

static const uint16_t s_primary_service_uuid = ESP_GATT_UUID_PRI_SERVICE;
static const uint16_t s_include_service_uuid = ESP_GATT_UUID_INCLUDE_SERVICE;
static const uint16_t CHARACTERISTIC_DECLARE = ESP_GATT_UUID_CHAR_DECLARE;
static const uint16_t s_character_client_config_uuid = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;

static const uint16_t s_bat_level_uuid = ESP_GATT_UUID_BATTERY_LEVEL;
static const uint16_t s_bat_char_pres_format_uuid = ESP_GATT_UUID_CHAR_PRESENT_FORMAT;
static const uint8_t s_char_prop_read_notify = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_NOTIFY;

static const uint16_t s_bat_svc = ESP_GATT_UUID_BATTERY_SERVICE_SVC;
enum {
  BAS_IDX_SVC,

  BAS_IDX_BATT_LVL_CHAR,
  BAS_IDX_BATT_LVL_VAL,
  BAS_IDX_BATT_LVL_CCC,
  BAS_IDX_BATT_LVL_PRES_FMT,

  BAS_IDX_NB,
};

void bt::attribute_visitor::visit(bt::profile_t *t) {
  std::cout << "visit profile" << std::endl;
  for (auto srv : t->services()) {
    auto *service_visitor = new attribute_visitor(m_gatt_if);
    srv.accept(dynamic_cast<visitor_t<std::remove_pointer_t<decltype(srv)>> *>(service_visitor));
    esp_err_t err = m_gatt_if->create_attr_tab(service_visitor->m_attributes.data(), BAS_IDX_NB, 0);
    if (err) {
      std::cout << "error while attribute sevice visitor" << std::endl;
    }
    delete service_visitor;
  }
}

void bt::attribute_visitor::visit(bt::service_t *t) {
  std::cout << "visit service" << std::endl;
  m_attributes.push_back(esp_gatts_attr_db_t{.attr_control = {.auto_rsp = ESP_GATT_AUTO_RSP},
                                             .att_desc = {.uuid_length = ESP_UUID_LEN_16,
                                                          .uuid_p = (uint8_t *)&s_primary_service_uuid,
                                                          .perm = ESP_GATT_PERM_READ,
                                                          .max_length = 2,
                                                          .length = 2,
                                                          .value = (uint8_t *)&s_bat_svc}});

  for (auto c : t->characteristics()) {
    c.accept(dynamic_cast<visitor_t<std::remove_pointer_t<decltype(c)>> *>(this));
  }
  std::cout << "character_size" << m_attributes.size() << std::endl;
}

void bt::attribute_visitor::visit(bt::characteristic_t *t) {
  std::cout << "visit characteristic" << std::endl;
  for (auto *attr : t->attributes()) {
    attr->accept(dynamic_cast<visitor_t<std::remove_pointer_t<decltype(attr)>> *>(this));
  }
}

void bt::attribute_visitor::visit(bt::attribute_t *t) {
  // TODO push attributes here
  m_attributes.push_back(
      esp_gatts_attr_db_t{.attr_control = {.auto_rsp = static_cast<uint8_t>(t->m_automated ? 1u : 0u)},
                          .att_desc = {.uuid_length = ESP_UUID_LEN_16,
                                       .uuid_p = (uint8_t *)(&t->m_uuid),
                                       .perm = static_cast<uint16_t>(t->m_permission),
                                       .max_length = t->m_max_length,
                                       .length = t->m_length,
                                       .value = t->m_value}});
}
