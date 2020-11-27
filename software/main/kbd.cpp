#include "esp_if.hpp"
#include "keyboard.hpp"
#include "matrix.hpp"
#include "supporting/mapping.hpp"

using namespace kopinions;
using namespace kopinions::logging;

extern "C" void app_main() {
  auto sche = std::make_shared<scheduler_if<>>();

  sche->schedule("test", []() -> void {
    auto ios = std::make_shared<gpios_if>();
    auto clk = std::make_shared<clock_if>();
    auto mtx = std::make_shared<matrix>(ios, clk);
    auto lay = std::make_shared<layout>(layout_mapping);
    auto cfg = std::make_shared<config>();
    auto kbd = std::make_shared<keyboard>(lay, mtx, cfg);
    auto lg = std::make_shared<logger>(level::INFO, std::make_shared<esp_log_sink>());
    while (true) {
      lg->log(level::DEBUG, "%s", "xxx1111");

      auto&& res = kbd->scan();
      for (auto b : res) {
        auto status = b.sts;
        lg->log(level::DEBUG, "%s", status);
      }
      vTaskDelay(100);
    }
  });
}