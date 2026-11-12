/**
 * @file fuzz_ads9324.cpp
 * @brief Fuzz target: the driver over a bus whose behaviour the fuzzer
 * controls.
 *
 * What the driver reads comes from the chip over the bus, so read data is the
 * untrusted input. The fuzzer's bytes choose a configuration, then a sequence
 * of driver calls, and between the calls they change the chip: a stuck or
 * floating data line, a flipped reply bit, a failing transfer, a failing pin, a
 * brown-out, a reset flagged in the DIAG byte, a slow calibration, other
 * readings of the datasheet's open questions.
 *
 * After every call the target checks the driver's promises (docs/contract.md),
 * and aborts, naming the line, when one breaks:
 *
 * - F1 a failed call returns an `Error` the enumeration defines, and a zero
 *   value;
 * - F2 while the driver is not initialised, only EnsureInitialized(), Reset(),
 *   PowerDown() and Deinitialize() make a transfer; PowerDown() on a healthy chip always
 *   reaches standby (this covers "no register write between a seen
 *   reset and the next successful start");
 * - F3 only documented registers are written, only with documented codes:
 *   BANK_SEL in {1, 2, 4}, XOR_BITS_CTL in {0, 0x00F1}, CM_RANGE in {0, 5, 6},
 *   INPUT_RANGE in {0, 2, 3, 4, 5}, PGA_BW in {0, 1}, DOUT_LENGTH not 01b, FIR
 *   not 110b, average codes up to 11, delays up to 5, the alarm function in {0,
 *   1, 6, 8}, TP_MODE in {0, 2}, and no write to a read-only register;
 * - F4 every readout frame sends zeros on SDI;
 * - F5 every readout frame has the length the configuration implies;
 * - F6 no calibration runs while a digital filter is on, and
 *   RunAdcCalibration() with a filter is refused;
 * - F7 a successful change of the digital filter ends with a SYSREF pulse (the
 *   register's bit 0 back at 0);
 * - F8 with the randomizer on, MeasureOnce() returns the words the chip converted;
 * - F9 a reset flag in a data frame leaves the driver uninitialised;
 * - F10 every call ends: the time the port is asked to wait is bounded;
 * - F11 every log line is a non-empty plain-text message;
 * - F12 while no fault is injected, the shadow equals the chip's registers
 *   after every call, so every register write went to the bank the chip had
 *   selected;
 * - F13 while no fault is injected, each channel's offset and gain sit in the
 *   chip's register for that channel (the bank 2 mapping);
 * - F14 the daisy chain register GEN_CFG2 is never written.
 *
 * Built two ways (tests/fuzz/CMakeLists.txt): with libFuzzer (Clang,
 * `-DADS9324_BUILD_FUZZERS=ON`), and as a plain executable that replays the
 * corpus in tests/fuzz/corpus/ as a CTest on any compiler.
 */
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

#include "ads9324.hpp"
#include "mock_ads9324.hpp"

namespace {

using ads9324::Config;
using ads9324::Error;
using ads9324_test::FullPort;

#define FUZZ_CHECK(cond)                                                                           \
  do {                                                                                             \
    const bool fuzz_ok = (cond);                                                                   \
    if (!fuzz_ok) {                                                                                \
      (void)std::fprintf(stderr, "fuzz invariant broken at %s:%d: %s\n", __FILE__, __LINE__,       \
                         #cond);                                                                   \
      std::abort();                                                                                \
    }                                                                                              \
  } while (0)

/// The fuzzer's bytes, read front to back; zeros once they run out.
class Input {
public:
  Input(const uint8_t* data, std::size_t size) noexcept : data_(data), size_(size) {}
  [[nodiscard]] bool Empty() const noexcept {
    return pos_ >= size_;
  }
  uint8_t Byte() noexcept {
    return pos_ < size_ ? data_[pos_++] : uint8_t{0};
  }
  /// A value 0 to `n - 1`.
  uint32_t Below(uint32_t n) noexcept {
    return static_cast<uint32_t>(Byte()) % n;
  }

private:
  const uint8_t* data_;
  std::size_t size_;
  std::size_t pos_ = 0;
};

bool ErrorDefined(Error e) {
  const auto v = static_cast<uint32_t>(e);
  return (v <= 9U) || ((v >= 16U) && (v <= 19U));
}

/// One in eight values is out of range on purpose, so Validate() has work.
int32_t Pick(Input& in, int32_t lo, int32_t hi) {
  const int32_t span = hi - lo + 1;
  const int32_t v = lo + static_cast<int32_t>(in.Below(static_cast<uint32_t>(span)));
  return (in.Below(8) == 0U) ? (hi + 1 + static_cast<int32_t>(in.Below(3))) : v;
}

ads9324::Input FuzzInput(Input& in) {
  ads9324::Input i{};
  uint32_t type_code = 0U;
  if (in.Below(8) == 0U) {
    type_code = 5U + in.Below(3);
  } else if (in.Below(3) == 0U) {
    type_code = 5U;
  }
  i.type = static_cast<ads9324::InputType>(type_code);
  i.range = static_cast<ads9324::InputRange>(in.Below(7));
  i.bandwidth = static_cast<ads9324::Bandwidth>(in.Below(3));
  i.cm_correction = in.Below(2) == 1U;
  i.offset_quarter_lsb = static_cast<int16_t>(Pick(in, -512, 511));
  i.gain = static_cast<int16_t>(Pick(in, -8192, 8191));
  i.phase_delay = in.Byte();
  i.window = ads9324::Window{in.Below(4) == 0U, static_cast<int8_t>(in.Byte()),
                             static_cast<int8_t>(in.Byte()), in.Byte()};
  return i;
}

Config FuzzConfig(Input& in) {
  Config c{};
  c.interface.lanes = static_cast<ads9324::Lanes>(in.Below(5));
  c.interface.data_on_sdout = in.Below(4) != 0U;
  c.interface.length = static_cast<ads9324::WordLength>(in.Below(4));
  c.interface.format = static_cast<ads9324::DataFormat>(in.Below(3));
  c.interface.append_diag = in.Below(2) == 1U;
  c.interface.randomize = in.Below(4) == 0U;
  if (c.interface.randomize && (in.Below(4) != 0U)) {
    c.interface.length =
        ads9324::WordLength::Bits16; // the randomizer is specified for 16 bits only
  }
  c.interface.drive = static_cast<ads9324::DriveStrength>(in.Below(5));
  for (auto& d : c.interface.delay_ns) {
    d = static_cast<uint8_t>(in.Below(8));
  }
  c.reference = static_cast<ads9324::ReferenceSource>(in.Below(4));
  c.filter.kind = static_cast<ads9324::FilterKind>(in.Below(5));
  c.filter.length = static_cast<ads9324::AverageLength>(in.Below(13));
  // NOLINTNEXTLINE(clang-analyzer-optin.core.EnumCastOutOfRange): the fuzzer feeds every code
  c.filter.fir = static_cast<ads9324::FirFilter>(in.Below(8));
  c.filter.internal_clock = in.Below(2) == 1U;
  c.channels = {static_cast<ads9324::ChannelCount>(in.Below(4)), static_cast<uint8_t>(in.Below(9))};
  c.alarm.function = static_cast<ads9324::AlarmFunction>(in.Below(10));
  c.alarm.active_low = in.Below(2) == 1U;
  c.alarm.pulse = in.Below(4) == 0U;
  c.alarm.sources = in.Byte();
  c.alarm.or_mask_acknowledged = in.Below(2) == 1U;
  c.sampling.convst_hz = static_cast<uint32_t>(Pick(in, 100'000, 1'000'000));
  if (in.Below(16U) == 0U) {
    c.sampling.convst_hz = 0U; // invalid on purpose: no call may divide by it before the start
  }
  c.sampling.ready = static_cast<ads9324::ReadySource>(in.Below(2));
  c.sampling.ready_timeout_us = static_cast<uint32_t>(Pick(in, 1, 5000));
  c.calibration.run_at_start = in.Below(4) == 0U;
  // NOLINTNEXTLINE(clang-analyzer-optin.core.EnumCastOutOfRange): the fuzzer feeds every code
  c.calibration.mode = static_cast<ads9324::CalibrationMode>(in.Below(4));
  c.calibration.trigger = static_cast<ads9324::CalibrationTrigger>(in.Below(2));
  c.hardware_reset = in.Below(4) == 0U;
  c.daisy.enabled = in.Below(32) == 0U;
  for (auto& t : c.test_pattern) {
    t.enabled = in.Below(4) == 0U;
    t.kind = static_cast<ads9324::PatternKind>(in.Below(4));
    t.value[0] = static_cast<uint16_t>(in.Byte());
  }
  for (std::size_t i = 0; i < 16; i += 1) {
    if (in.Below(4) == 0U) {
      c.inputs[i] = FuzzInput(in);
    }
  }
  return c;
}

using Driver = ads9324::ADS9324<FullPort>;

/// Replace each part of a random configuration that Validate() would refuse with the default, so
/// most random configurations reach the chip with their other parts intact.
void Repair(Config& c) {
  const Config ok{};
  namespace d = ads9324::detail;
  c.daisy.enabled = false;
  if (!c.interface.randomize || (c.interface.length == ads9324::WordLength::Bits16)) {
    c.interface.data_on_sdout =
        c.interface.data_on_sdout && (c.interface.lanes == ads9324::Lanes::One);
  } else {
    c.interface.length = ads9324::WordLength::Bits16;
  }
  if (d::ValidateInterface(c.interface) != Error::Ok) {
    c.interface = ok.interface;
  }
  for (std::size_t i = 0; i < c.inputs.size(); ++i) {
    if ((d::ValidateInput(c.inputs[i]) != Error::Ok) ||
        (d::ValidateWindow(c.inputs[i].window) != Error::Ok)) {
      c.inputs[i] = ok.inputs[i];
    }
  }
  if (d::ValidateFilter(c.filter) != Error::Ok) {
    c.filter = ok.filter;
  }
  if (d::ValidateAlarm(c.alarm) != Error::Ok) {
    c.alarm = ok.alarm;
  }
  if (d::ValidateSampling(c.sampling) != Error::Ok) {
    c.sampling = ok.sampling;
  }
  if (d::ValidateExtras(c) != Error::Ok) {
    c.reference = ok.reference;
    c.channels = ok.channels;
    c.calibration = ok.calibration;
    c.test_pattern = ok.test_pattern;
    c.window_glitch = ok.window_glitch;
  }
  if (d::WindowWithOffsetBinary(c)) {
    c.interface.format = ok.interface.format;
  }
  if ((c.filter.kind != ads9324::FilterKind::None) && c.calibration.run_at_start) {
    c.calibration.run_at_start = false;
  }
}

/// Three in four configurations are valid, so the setters reach the chip; the rest are not, so
/// Validate() has work.
Config NextConfig(Input& in) {
  Config c = FuzzConfig(in);
  if (in.Below(4U) != 0U) {
    Repair(c);
    if (ads9324::Validate(c) != Error::Ok) {
      return Config{};
    }
  }
  return c;
}

/// F3: the registers of the chip hold only documented codes.
void CheckChipRegisters(const ads9324_test::Ads9324Model& chip) {
  const uint32_t bank = chip.Reg(1, 0x02);
  FUZZ_CHECK((bank == 1U) || (bank == 2U) || (bank == 4U));
  FUZZ_CHECK((chip.Reg(1, 0x0B) == 0U) || (chip.Reg(1, 0x0B) == 0x00F1U));
  FUZZ_CHECK(((chip.Reg(1, 0x0A) >> 2U) & 3U) != 1U);
  const uint32_t sel = chip.Reg(1, 0x0C) & 0xFU;
  FUZZ_CHECK((sel == 0U) || (sel == 1U) || (sel == 6U) || (sel == 8U));
  FUZZ_CHECK(((chip.Reg(1, 0x14) >> 12U) & 7U) != 6U);
  FUZZ_CHECK(((chip.Reg(1, 0x14) >> 8U) & 0xFU) <= 11U);
  FUZZ_CHECK(((chip.Reg(1, 0x14) >> 4U) & 0xFU) <= 11U);
  FUZZ_CHECK(chip.Reg(1, 0x09) == 0U); // F14
  for (uint32_t d = 0; d < 5; ++d) {
    FUZZ_CHECK(((chip.Reg(1, 0x0E) >> (3U * d)) & 7U) <= 5U);
  }
  for (const uint8_t code : {uint8_t{2}, uint8_t{4}}) {
    const uint32_t tp = chip.Reg(code, 0x2E);
    FUZZ_CHECK((((tp >> 4U) & 7U) == 0U) || (((tp >> 4U) & 7U) == 2U));
    for (uint8_t a = 0x08; a < 0x0C; ++a) {
      for (const uint32_t half : {0U, 8U}) {
        const uint32_t cm = (chip.Reg(code, a) >> (half + 4U)) & 7U;
        const uint32_t range = (chip.Reg(code, a) >> half) & 7U;
        FUZZ_CHECK((cm == 0U) || (cm == 5U) || (cm == 6U));
        FUZZ_CHECK((range <= 5U) && (range != 1U));
      }
    }
    for (uint32_t l = 0; l < 8; ++l) {
      FUZZ_CHECK(((chip.Reg(code, 0x0C) >> (2U * l)) & 3U) <= 1U);
    }
  }
  FUZZ_CHECK(chip.bad_writes.empty());
}

/// F4: every readout frame sent zeros. F5: when `exact` is set, every readout frame has the length
/// the configuration implies.
void CheckLog(const ads9324_test::Ads9324Model& chip, std::size_t from, const Config& config,
              bool exact) {
  const std::size_t want = ads9324::FrameBytes(config.interface, config.channels);
  for (std::size_t i = from; i < chip.log.size(); ++i) {
    const auto& f = chip.log[i];
    if (!f.is_register_frame) {
      FUZZ_CHECK(f.all_zero_tx);
      FUZZ_CHECK(f.len <= ads9324::kMaxFrameBytes);
      FUZZ_CHECK(!exact || (f.len == want));
    }
  }
}

/// The calls that may transfer while the driver is not initialised: the start, and the switch-off
/// calls PowerDown() and Deinitialize() (they put a chip that was live into standby, whatever the
/// driver believes: a switch-off is never refused for doubt).
bool MayStart(uint32_t op) {
  return (op == 0U) || (op == 17U) || (op == 19U) || (op == 23U);
}

/// F12: every shadowed register of the driver equals the chip's.
void CheckShadow(const Driver& d, const ads9324_test::Ads9324Model& chip) {
  const uint8_t codes[3] = {1, 2, 4};
  for (uint8_t b = 0; b < 3U; ++b) {
    const auto bank = static_cast<ads9324::reg::Bank>(b);
    for (uint32_t a = 0; a < 64U; ++a) {
      if (ads9324::reg::IsShadowed(bank, static_cast<uint8_t>(a))) {
        FUZZ_CHECK(d.GetShadow(bank, static_cast<uint8_t>(a)) ==
                   chip.Reg(codes[b], static_cast<uint8_t>(a)));
      }
    }
  }
}

/// F13: each channel's offset and gain sit where the chip keeps them for that channel.
void CheckChannelMap(const Driver& d, const ads9324_test::Ads9324Model& chip) {
  const Config& c = d.GetConfig();
  for (int ch = 0; ch < 16; ++ch) {
    const auto& in = c.inputs[static_cast<std::size_t>(ch)];
    const uint8_t bank = ads9324_test::Ads9324Model::ChannelBank(ch);
    FUZZ_CHECK(chip.Reg(bank, chip.ChannelAddr(ch, 0x11U, 1)) ==
               (static_cast<uint32_t>(static_cast<uint16_t>(in.offset_quarter_lsb)) & 0x3FFU));
    FUZZ_CHECK(chip.Reg(bank, chip.ChannelAddr(ch, 0x19U, 1)) ==
               (static_cast<uint32_t>(static_cast<uint16_t>(in.gain)) & 0x3FFFU));
  }
}

template <typename T>
Error CheckResult(const ads9324::Result<T>& r) {
  FUZZ_CHECK(ErrorDefined(r.error));
  if (!r.ok()) {
    FUZZ_CHECK(r.value == T{});
  }
  return r.error;
}

Error CheckVoid(const ads9324::Result<void>& r) {
  FUZZ_CHECK(ErrorDefined(r.error));
  return r.error;
}

Error CheckFrame(const ads9324::Result<ads9324::Frame>& r, ads9324_test::Ads9324Model& chip,
                 bool derandomized_must_match) {
  FUZZ_CHECK(ErrorDefined(r.error));
  if (!r.ok()) {
    FUZZ_CHECK(r.value.present_mask == 0U);
  } else if (derandomized_must_match) {
    for (std::size_t ch = 0; ch < 16; ++ch) { // F8
      if (((static_cast<uint32_t>(r.value.present_mask) >> ch) & 1U) != 0U) {
        FUZZ_CHECK(r.value.raw[ch] == chip.sent_raw[ch]);
      }
    }
  }
  return r.error;
}

/// Change the chip between calls.
void Fault(FullPort& port, Input& in) {
  auto& k = port.state.chip.knobs;
  switch (in.Below(11)) {
  case 0:
    k.stuck_sdout = !k.stuck_sdout;
    break;
  case 1:
    k.float_sdout = !k.float_sdout;
    break;
  case 2:
    k.flip_reply_bit = (in.Below(2) == 0U) ? -1 : static_cast<int>(in.Below(16));
    break;
  case 3:
    k.fail_transfer = port.state.chip.transfers + static_cast<int>(in.Below(8));
    break;
  case 4:
    port.state.gpio_fail = port.state.gpio_calls + static_cast<int>(in.Below(4));
    break;
  case 5:
    port.state.chip.BrownOut();
    break;
  case 6:
    k.reset_during_run = true;
    break;
  case 7:
    k.cal_busy_us = in.Below(2) == 0U ? 125'000U : 500'000U;
    break;
  case 8:
    k.bank2_mirrored = !k.bank2_mirrored;
    break;
  case 9:
    k.reply_low_byte = in.Byte();
    break;
  default:
    k.alrm_mask_one_masks = !k.alrm_mask_one_masks;
    break;
  }
}

void Call(Driver& d, FullPort& port, Input& in, bool& faults) {
  auto& chip = port.state.chip;
  const std::size_t log_from = chip.log.size();
  const uint64_t delays = port.state.delays_us;
  const bool started = d.IsInitialized();
  Config c = d.GetConfig();
  const Config before = c;
  const uint32_t calibrations = chip.calibrations;
  const bool cal_in_flight = chip.Calibrating(); // one started earlier may end in this call
  const uint32_t sysref = chip.sysref_edges;
  const bool derandomize = !faults && before.interface.randomize;
  Error err = Error::Ok;
  const uint32_t op = in.Below(24);
  switch (op) {
  case 0:
    err = CheckVoid(d.EnsureInitialized());
    break;
  case 1:
    err = CheckVoid(d.Convert());
    break;
  case 2:
    err = CheckVoid(d.WaitDataReady());
    break;
  case 3:
    err = CheckFrame(d.ReadFrame(), chip, false); // no wait: the result may not be ready
    break;
  case 4:
    err = CheckFrame(d.MeasureOnce(), chip, derandomize);
    break;
  case 5:
    err = CheckVoid(
        d.SetInput(static_cast<uint8_t>(in.Below(18)), FuzzInput(in), ads9324::Verify::ReadBack));
    break;
  case 6:
    err = CheckVoid(d.ApplyConfig(NextConfig(in)));
    break;
  case 7:
    c.interface = NextConfig(in).interface;
    err = CheckVoid(d.SetInterface(c.interface));
    break;
  case 8:
    err = CheckVoid(d.SetFilter(NextConfig(in).filter));
    break;
  case 9:
    err = CheckVoid(d.SetChannels(NextConfig(in).channels));
    break;
  case 10:
    err = CheckVoid(d.SetAlarm(NextConfig(in).alarm));
    break;
  case 11:
    err = CheckVoid(d.SetReference(NextConfig(in).reference));
    break;
  case 12:
    err = CheckVoid(
        d.SetTestPattern(static_cast<uint8_t>(in.Below(3)), NextConfig(in).test_pattern[0]));
    break;
  case 13:
    err = CheckResult(d.ReadWindowFlags());
    break;
  case 14:
    err = CheckVoid(d.ClearWindowFlags(static_cast<uint8_t>(in.Below(3))));
    break;
  case 15:
    err = CheckVoid(d.RunAdcCalibration());
    break;
  case 16:
    err = CheckVoid(d.DisableAdcCalibration());
    break;
  case 17:
    err = CheckVoid(d.PowerDown());
    break;
  case 18:
    err = CheckVoid(d.Wake());
    break;
  case 19:
    err = CheckVoid(d.Reset(static_cast<ads9324::ResetKind>(in.Below(3))));
    break;
  case 20:
    err = CheckResult(d.ReadRegister(static_cast<ads9324::reg::Bank>(in.Below(4)), in.Byte()));
    break;
  case 21:
    err = CheckVoid(d.RunDataPathSelfTest());
    break;
  case 22:
    err = CheckResult(d.ReadAlarmPin());
    break;
  default:
    err = CheckVoid(d.Deinitialize());
    break;
  }
  const bool initialized = d.IsInitialized();
  if (!started && !initialized && !MayStart(op)) {
    FUZZ_CHECK(chip.log.size() == log_from); // F2
  }
  if ((op == 17U) && started && !faults) {
    FUZZ_CHECK(chip.Standby()); // a switch-off always reaches the chip
  }
  if (started && (err == Error::UnexpectedReset)) {
    FUZZ_CHECK(!initialized); // F9
  }
  if ((op == 15U) && started && !faults && (before.filter.kind != ads9324::FilterKind::None)) {
    FUZZ_CHECK(err == Error::Refused); // F6
  }
  if ((before.filter.kind != ads9324::FilterKind::None) &&
      (d.GetConfig().filter.kind != ads9324::FilterKind::None)) {
    FUZZ_CHECK(cal_in_flight || (chip.calibrations == calibrations)); // F6
  }
  // Only the fields that reach DIG_FILTER count: the length is for the averages, `fir` for the FIR.
  const ads9324::Filter& now = d.GetConfig().filter;
  const bool uses_length = (now.kind == ads9324::FilterKind::BlockAverage) ||
                           (now.kind == ads9324::FilterKind::MovingAverage);
  const bool filter_changed =
      (now.kind != before.filter.kind) || (uses_length && (now.length != before.filter.length)) ||
      ((now.kind == ads9324::FilterKind::Fir) && (now.fir != before.filter.fir));
  if (!faults && started && (err == Error::Ok) && filter_changed &&
      (d.GetConfig().filter.kind != ads9324::FilterKind::None)) {
    FUZZ_CHECK(chip.sysref_edges > sysref); // F7
    FUZZ_CHECK((chip.Reg(1, 0x14) & 1U) == 0U);
  }
  FUZZ_CHECK(port.state.delays_us - delays <= 5'000'000U); // F10
  CheckChipRegisters(chip);
  CheckLog(chip, log_from, d.GetConfig(), !faults && ((op == 3U) || (op == 4U)));
  if (!faults && initialized && (err == Error::Ok) && (op >= 5U) && (op <= 12U)) {
    CheckShadow(d, chip);     // F12
    CheckChannelMap(d, chip); // F13
  }
  for (const char* line : port.state.logs) {
    FUZZ_CHECK((line != nullptr) && (line[0] != '\0')); // F11
  }
}

} // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, std::size_t size) {
  Input in(data, size);
  FullPort port;
  Driver driver(port, NextConfig(in));
  // Half of the inputs never inject a fault, so the invariants that only hold on a healthy chip
  // (F7, F8, F12, F13) are exercised; most inputs bring the driver up first so the setters reach
  // the chip.
  const bool faulty = in.Below(2U) == 0U;
  bool faults = false;
  if (in.Below(4U) != 0U) {
    (void)CheckVoid(driver.EnsureInitialized());
  }
  for (int calls = 0; (calls < 48) && !in.Empty(); ++calls) {
    if (faulty && (in.Below(5U) == 0U)) {
      Fault(port, in);
      faults = true;
    }
    Call(driver, port, in, faults);
  }
  return 0;
}
