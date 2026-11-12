/**
 * @file test_kconfig.cpp
 * @brief KconfigDefaults(): the defaults without Kconfig, and menuconfig values
 * when they are set.
 *
 * Built twice: without CONFIG_ macros (KCONFIG_SET undefined) and with the
 * values an sdkconfig.h would define (KCONFIG_SET=1, see tests/CMakeLists.txt).
 */
#include "ads9324_kconfig.hpp"
#include "test_harness.hpp"

#ifdef KCONFIG_SET
TEST_CASE("Kconfig values reach the Config") {
  constexpr ads9324::Config kC = ads9324::KconfigDefaults();
  CHECK_EQ(kC.sampling.convst_hz, 500'000U);
  CHECK_EQ(kC.sampling.ready_timeout_us, 2500U);
  CHECK_EQ(kC.interface.length, ads9324::WordLength::Bits24);
  CHECK(kC.interface.append_diag);
  CHECK_EQ(kC.sampling.ready, ads9324::ReadySource::Pin);
  CHECK_EQ(ads9324::Validate(kC), ads9324::Error::Ok);
}
#else
TEST_CASE("Without Kconfig, KconfigDefaults() equals Config{}") {
  constexpr ads9324::Config kC = ads9324::KconfigDefaults();
  constexpr ads9324::Config kD{};
  CHECK_EQ(kC.sampling.convst_hz, kD.sampling.convst_hz);
  CHECK_EQ(kC.sampling.ready_timeout_us, kD.sampling.ready_timeout_us);
  CHECK_EQ(kC.interface.length, kD.interface.length);
  CHECK_EQ(kC.interface.append_diag, kD.interface.append_diag);
  CHECK_EQ(kC.sampling.ready, kD.sampling.ready);
}
#endif
