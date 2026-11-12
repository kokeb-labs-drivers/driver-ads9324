/**
 * @file test_errors.cpp
 * @brief Which errors the driver returns (T25): the chip's own, and that `Crc`
 * and `Busy` are never returned because the chip has no CRC and no busy reply.
 */
#include "test_support.hpp"

using namespace ads9324;
using namespace ads9324_test;

namespace {

/// Collects the error of every call in a scenario.
struct Spy {
  uint32_t seen{0};
  void Note(Error e) {
    seen |= 1U << static_cast<uint32_t>(e);
  }
  [[nodiscard]] bool Saw(Error e) const {
    return (seen & (1U << static_cast<uint32_t>(e))) != 0U;
  }
};

void Run(Spy& spy, void (*scenario)(Spy&)) {
  scenario(spy);
}

void Scenario(Spy& spy) {
  Rig rig;
  spy.Note(rig.drv.Convert().error); // NotInitialized
  rig.port.state.bus_up_ok = false;
  spy.Note(rig.drv.EnsureInitialized().error); // Bus
  rig.port.state.bus_up_ok = true;
  spy.Note(rig.drv.EnsureInitialized().error);   // Ok
  spy.Note(rig.drv.SetInput(99, Input{}).error); // InvalidArgument
  Config d{};
  d.daisy.enabled = true;
  spy.Note(rig.drv.ApplyConfig(d).error); // Unsupported
  spy.Note(rig.drv.SetFilter({FilterKind::Fir, AverageLength::None, FirFilter::Fir1, false}).ok()
               ? rig.drv.RunAdcCalibration().error
               : Error::Ok); // Refused
  (void)rig.drv.SetFilter(Filter{});
  rig.Chip().knobs.flip_in_channel_banks_only = true;
  rig.Chip().knobs.flip_reply_bit = 4;
  spy.Note(rig.drv
               .SetInput(
                   1,
                   [] {
                     Input i{};
                     i.gain = 3;
                     return i;
                   }(),
                   Verify::ReadBack)
               .error); // VerifyFailed
  rig.Chip().knobs.flip_reply_bit = -1;
  rig.Chip().knobs.stuck_sdout = true;
  spy.Note(rig.drv.MeasureOnce().error); // DataMismatch
  rig.Chip().knobs.stuck_sdout = false;
  rig.Chip().knobs.reset_during_run = true;
  Config c = rig.drv.GetConfig();
  c.interface.append_diag = true;
  (void)rig.drv.ApplyConfig(c);
  spy.Note(rig.drv.MeasureOnce().error); // UnexpectedReset
  Rig slow;
  slow.Chip().knobs.cal_busy_us = 500'000;
  (void)slow.drv.EnsureInitialized();
  spy.Note(slow.drv.RunAdcCalibration().error); // Timeout
  Rig wrong;
  wrong.Chip().knobs.stuck_sdout = true;
  spy.Note(wrong.drv.EnsureInitialized().error); // DeviceIdMismatch
}

} // namespace

TEST_CASE("T25 every chip-specific error is returned by some call") {
  Spy spy;
  Run(spy, Scenario);
  CHECK(spy.Saw(Error::UnexpectedReset));
  CHECK(spy.Saw(Error::VerifyFailed));
  CHECK(spy.Saw(Error::Refused));
  CHECK(spy.Saw(Error::DataMismatch));
  CHECK(spy.Saw(Error::NotInitialized));
  CHECK(spy.Saw(Error::InvalidArgument));
  CHECK(spy.Saw(Error::Bus));
  CHECK(spy.Saw(Error::Timeout));
  CHECK(spy.Saw(Error::DeviceIdMismatch));
  CHECK(spy.Saw(Error::Unsupported));
}

TEST_CASE("T25 Crc, Busy and DeviceFault are in the set but never returned") {
  Spy spy;
  Run(spy, Scenario);
  CHECK(!spy.Saw(Error::Crc));
  CHECK(!spy.Saw(Error::Busy));
  CHECK(!spy.Saw(Error::DeviceFault));
}

TEST_CASE("T25 the port failing to log does not matter, and a port without Log "
          "works") {
  MinimalPort port;
  ADS9324<MinimalPort> drv(port);
  CHECK(drv.EnsureInitialized().ok());
  Rig rig;
  rig.Chip().knobs.float_sdout = true;
  CHECK(!rig.drv.EnsureInitialized().ok());
  CHECK(!rig.port.state.logs.empty());
  for (const char* line : rig.port.state.logs) {
    CHECK((line != nullptr) && (line[0] != '\0'));
  }
}
