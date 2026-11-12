/**
 * @file ads9324_basic_example.cpp
 * @brief Initialise the ADS9324 from the menuconfig values and log the 16 channels twice a second.
 *
 * Wiring: examples/esp32/README.md. Expected output: one "initialised" line, then one line per
 * second with the first four channels in millivolts. Every Result is checked and logged by name.
 */
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "ads9324.hpp"
#include "ads9324_kconfig.hpp"
#include "esp32_ads9324_port.hpp"

namespace {
constexpr const char* kTag = "ads9324_example";
}

extern "C" void app_main(void) {
  static Esp32Ads9324Port port; // static: the driver keeps a reference
  static ads9324::ADS9324<Esp32Ads9324Port> adc(port, ads9324::KconfigDefaults()); // no I/O

  ESP_LOGI(kTag, "ADS9324 driver %s", ads9324::GetDriverVersion());
  const auto init = adc.EnsureInitialized();
  if (!init) {
    ESP_LOGE(kTag, "EnsureInitialized failed: %s", ads9324::ToString(init.error));
    return;
  }
  ESP_LOGI(kTag, "initialised");

  while (true) {
    const auto frame = adc.MeasureOnce();
    if (frame) {
      ESP_LOGI(kTag, "AIN1 %d mV, AIN2 %d mV, AIN3 %d mV, AIN4 %d mV",
               static_cast<int>(frame.value.microvolts[0] / 1000),
               static_cast<int>(frame.value.microvolts[1] / 1000),
               static_cast<int>(frame.value.microvolts[2] / 1000),
               static_cast<int>(frame.value.microvolts[3] / 1000));
    } else {
      ESP_LOGW(kTag, "read failed: %s", ads9324::ToString(frame.error));
      if (frame.error == ads9324::Error::UnexpectedReset) {
        (void)adc.EnsureInitialized(); // the chip lost its configuration: start it again
      } else if (frame.error == ads9324::Error::Bus) {
        (void)adc.Reset(ads9324::ResetKind::Software); // recover after a bus glitch
      } else {
        // Any other error: the next read reports again.
      }
    }
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}
