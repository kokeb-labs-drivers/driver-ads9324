/**
 * @file ads9324_kconfig.hpp
 * @brief Optional ESP-IDF layer: a `Config` built from the `CONFIG_ADS9324_*`
 * Kconfig options.
 * @copyright Copyright (c) 2026 The driver-ads9324 authors.
 * SPDX-License-Identifier: MIT
 *
 * @details
 * Include this header only where you want menuconfig to choose the defaults;
 * the driver never includes it and no driver signature depends on it. Under
 * ESP-IDF the values come from `sdkconfig.h`. Without Kconfig every option
 * falls back to the `Config{}` default, so `KconfigDefaults()` equals
 * `Config{}`.
 *
 * Kconfig writes a bool option set to "n" as an absent macro, so an absent
 * option means off in both cases.
 *
 * One configuration per program: the inline function below depends on the
 * `CONFIG_` macros, so every translation unit of one program must see the same
 * values (ESP-IDF has one `sdkconfig.h` per build; each host test is its own
 * program). No driver or library header includes this one (handbook
 * KB-CPP20-018).
 */
#ifndef ADS9324_KCONFIG_HPP
#define ADS9324_KCONFIG_HPP

#ifdef __has_include
#if __has_include("sdkconfig.h")
#include "sdkconfig.h"
#endif
#endif

#include "ads9324_types.hpp"

#ifndef CONFIG_ADS9324_CONVST_HZ
/** CONVST frequency when Kconfig does not set it: 100 kHz, the lowest the chip
 * allows. */
#define CONFIG_ADS9324_CONVST_HZ 100000
#endif
#ifndef CONFIG_ADS9324_READY_TIMEOUT_US
/** DRDY wait when Kconfig does not set it:
 * `Config{}.sampling.ready_timeout_us`. */
#define CONFIG_ADS9324_READY_TIMEOUT_US 10000
#endif

namespace ads9324 {

/**
 * @brief The configuration menuconfig selected, on top of the `Config{}`
 * defaults.
 * @return `Config{}` with the CONVST frequency, word length, DIAG byte, ready
 * source and DRDY timeout taken from Kconfig.
 */
constexpr Config KconfigDefaults() noexcept {
  Config c{};
  c.sampling.convst_hz = static_cast<uint32_t>(CONFIG_ADS9324_CONVST_HZ);
  c.sampling.ready_timeout_us = static_cast<uint32_t>(CONFIG_ADS9324_READY_TIMEOUT_US);
#if defined(CONFIG_ADS9324_WORD_24_BIT) && CONFIG_ADS9324_WORD_24_BIT
  c.interface.length = WordLength::Bits24;
#endif
#if defined(CONFIG_ADS9324_APPEND_DIAG) && CONFIG_ADS9324_APPEND_DIAG
  c.interface.append_diag = true;
#endif
#if defined(CONFIG_ADS9324_READY_ON_PIN) && CONFIG_ADS9324_READY_ON_PIN
  c.sampling.ready = ReadySource::Pin;
#endif
  return c;
}

} // namespace ads9324

#endif // ADS9324_KCONFIG_HPP
