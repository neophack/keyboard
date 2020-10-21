#include "keyboard.hpp"

#include "common/matchers.hpp"
#include "common/mocks_provider.hpp"
#include "common/test.hpp"
#include "gpios.hpp"

namespace di = boost::di;

int main() {
  "dummy"_test = [] {
    auto injector = di::make_injector<mocks_provider>();
    auto&& gpio_stub = mock<gpio>();
    auto&& gpios_stub = mock<gpios>();

    auto kbd = injector.create<keyboard>();

    expect_that<int>(kbd.scan().size(), matchers::eq(0));
  };
}