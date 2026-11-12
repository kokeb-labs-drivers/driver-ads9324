/**
 * @file test_config.cpp
 * @brief Configuration writes: the bank layout, the shadow, minimal traffic,
 * failure paths, read-back (T16 to T22, T32).
 */
#include "test_support.hpp"

using namespace ads9324;
using namespace ads9324_test;

namespace {

/// Every documented register of the driver's shadow equals the model's register
/// (T32, F12).
bool ShadowMatchesChip(Rig& rig) {
  const uint8_t codes[3] = {1, 2, 4};
  for (uint8_t b = 0; b < 3; ++b) {
    for (uint32_t a = 0; a < 64; ++a) {
      const auto bank = static_cast<reg::Bank>(b);
      if (!reg::IsShadowed(bank, static_cast<uint8_t>(a))) {
        continue;
      }
      if (rig.drv.GetShadow(bank, static_cast<uint8_t>(a)) !=
          rig.Chip().Reg(codes[b], static_cast<uint8_t>(a))) {
        return false;
      }
    }
  }
  return true;
}

uint32_t Half(uint32_t v, int local) {
  return ((static_cast<uint32_t>(local) & 1U) != 0U) ? (v >> 8U) : (v & 0xFFU);
}

} // namespace

TEST_CASE("T16 each channel's range lands in the model's register for that "
          "channel (A19, F13)") {
  for (uint8_t ch = 0; ch < 16; ++ch) {
    Rig rig;
    REQUIRE(rig.Init());
    Input in{};
    in.range = InputRange::V10;
    in.type = InputType::Differential;
    in.cm_correction = true;
    REQUIRE(rig.drv.SetInput(ch, in).ok());
    for (uint8_t other = 0; other < 16; ++other) {
      const uint32_t v =
          rig.Chip().Reg(Ads9324Model::ChannelBank(other), rig.Chip().ChannelAddr(other, 0x08U, 2));
      const uint32_t byte = Half(v, rig.Chip().LocalIndex(other));
      CHECK_EQ(byte, (other == ch) ? 0x84U : 0x00U);
    }
    CHECK(ShadowMatchesChip(rig));
  }
}

TEST_CASE("T16 the second bank is mirrored: AIN16 is the first register, AIN9 "
          "the last") {
  Rig rig;
  REQUIRE(rig.Init());
  Input in{};
  in.offset_quarter_lsb = 77;
  REQUIRE(rig.drv.SetInput(15, in).ok());
  CHECK_EQ(rig.Chip().Reg(4, 0x11), 77U);
  in.offset_quarter_lsb = 78;
  REQUIRE(rig.drv.SetInput(8, in).ok());
  CHECK_EQ(rig.Chip().Reg(4, 0x18), 78U);
  in.offset_quarter_lsb = 5;
  REQUIRE(rig.drv.SetInput(0, in).ok());
  CHECK_EQ(rig.Chip().Reg(2, 0x11), 5U);
}

TEST_CASE("T17 offset, gain and phase delay are written as two's complement fields") {
  Rig rig;
  REQUIRE(rig.Init());
  Input in{};
  in.offset_quarter_lsb = -3;
  in.gain = -8192;
  in.phase_delay = 200;
  in.bandwidth = Bandwidth::Wide;
  REQUIRE(rig.drv.SetInput(1, in).ok());
  CHECK_EQ(rig.Chip().Reg(2, 0x12), 0x03FDU);
  CHECK_EQ(rig.Chip().Reg(2, 0x1A), 0x2000U);
  CHECK_EQ(rig.Chip().Reg(2, 0x0D) >> 8U, 200U);
  CHECK_EQ(rig.Chip().Reg(2, 0x0C), 0x0004U);
  CHECK_EQ(rig.Chip().Reg(1, 0x14) & 0x8000U, 0x8000U); // PHASE_DELAY_EN
  CHECK_EQ(rig.Chip().sysref_edges, 1U);
}

TEST_CASE("T17 the offset removes an offset error and the gain scales") {
  Rig rig;
  rig.Chip().analog_offset_lsb = 20;
  rig.Chip().input_uv[0] = 0;
  REQUIRE(rig.Init());
  CHECK_EQ(rig.drv.MeasureOnce().value.codes[0], 20);
  Input in{};
  in.offset_quarter_lsb = OffsetCorrectionFor(0, 20);
  REQUIRE(rig.drv.SetInput(0, in).ok());
  CHECK_EQ(rig.drv.MeasureOnce().value.codes[0], 0);
  rig.Chip().input_uv[1] = 2'500'000;
  in = Input{};
  in.gain = GainCorrectionFor(16384, 16404);
  REQUIRE(rig.drv.SetInput(1, in).ok());
  const auto f = rig.drv.MeasureOnce();
  REQUIRE(f.ok());
  CHECK(f.value.codes[1] <= 16384 + 21);
}

TEST_CASE("T18 an identical configuration writes nothing") {
  Rig rig;
  REQUIRE(rig.Init());
  rig.Chip().log.clear();
  REQUIRE(rig.drv.ApplyConfig(rig.drv.GetConfig()).ok());
  // Only the reset probe (BANK_SEL, two reads) goes out; no configuration register is written.
  for (const auto& f : rig.Chip().log) {
    CHECK((f.addr == 0x00U) || (f.addr == 0x01U) || (f.addr == 0x02U));
  }
  CHECK(rig.Chip().bad_writes.empty());
}

TEST_CASE("T18 a setter writes only the registers it changes") {
  Rig rig;
  REQUIRE(rig.Init());
  rig.Chip().log.clear();
  Input in{};
  in.range = InputRange::V2_5;
  REQUIRE(rig.drv.SetInput(5, in).ok());
  int writes = 0;
  for (const FrameLog& f : rig.Chip().log) {
    writes += (f.is_register_frame && (f.addr != 0x00U) && (f.addr != 0x01U) && (f.addr != 0x02U))
                  ? 1
                  : 0;
  }
  CHECK_EQ(writes, 1);
}

TEST_CASE("T19 a failed setter leaves the stored configuration alone, and a "
          "retry repairs the chip") {
  Rig rig;
  REQUIRE(rig.Init());
  Input in{};
  in.range = InputRange::V12_5;
  in.offset_quarter_lsb = 9;
  rig.Chip().knobs.fail_transfer = rig.Chip().transfers + 1;
  CHECK_EQ(rig.drv.SetInput(12, in).error, Error::Bus);
  CHECK_EQ(rig.drv.GetConfig().inputs[12].range, InputRange::V5);
  rig.Chip().knobs.fail_transfer = -1;
  CHECK(rig.drv.SetInput(12, in).ok());
  CHECK_EQ(rig.drv.GetConfig().inputs[12].range, InputRange::V12_5);
  CHECK(ShadowMatchesChip(rig));
}

TEST_CASE("T19 a failure at any transfer of a large change is Bus and the "
          "shadow stays true") {
  for (int at = 0; at < 50; ++at) {
    Rig rig;
    REQUIRE(rig.Init());
    Config next = rig.drv.GetConfig();
    next.inputs[2].range = InputRange::V2_5;
    next.inputs[9].offset_quarter_lsb = 33;
    next.interface.append_diag = true;
    next.alarm.active_low = true;
    rig.Chip().knobs.fail_transfer = rig.Chip().transfers + at;
    const auto r = rig.drv.ApplyConfig(next);
    rig.Chip().knobs.fail_transfer = -1;
    if (r.ok()) {
      CHECK(ShadowMatchesChip(rig));
      continue;
    }
    CHECK_EQ(r.error, Error::Bus);
    CHECK(rig.drv.ApplyConfig(next).ok());
    CHECK(ShadowMatchesChip(rig));
    CHECK(rig.Chip().bad_writes.empty());
  }
}

TEST_CASE("T20 Verify::ReadBack reads every written register and finds a "
          "difference") {
  Rig rig;
  REQUIRE(rig.Init());
  Input in{};
  in.gain = 1234;
  rig.Chip().log.clear();
  CHECK(rig.drv.SetInput(7, in, Verify::ReadBack).ok());
  // The reset probe reads DEVICE_ID and DIAG_CTRL (two read commands), the verify one more.
  CHECK_EQ(rig.Chip().CountWrites(0x01), 3);
  in.gain = 1235;
  rig.Chip().knobs.flip_in_channel_banks_only = true;
  rig.Chip().knobs.flip_reply_bit = 8;
  CHECK_EQ(rig.drv.SetInput(7, in, Verify::ReadBack).error, Error::VerifyFailed);
  rig.Chip().knobs.flip_reply_bit = -1;
  // NOLINTNEXTLINE(clang-analyzer-optin.core.EnumCastOutOfRange): deliberate
  const auto bad_verify = static_cast<Verify>(5);
  CHECK_EQ(rig.drv.SetInput(7, in, bad_verify).error, Error::InvalidArgument);
}

TEST_CASE("T21 before the start a setter only stores the configuration") {
  Rig rig;
  Input in{};
  in.range = InputRange::V10;
  CHECK(rig.drv.SetInput(4, in).ok());
  CHECK(rig.Chip().log.empty());
  CHECK_EQ(rig.drv.GetConfig().inputs[4].range, InputRange::V10);
  REQUIRE(rig.Init());
  CHECK_EQ(Half(rig.Chip().Reg(2, 0x0A), 4), 0x04U);
  CHECK_EQ(rig.drv.SetInput(16, in).error, Error::InvalidArgument);
}

TEST_CASE("T21 invalid arguments are refused with no traffic") {
  Rig rig;
  REQUIRE(rig.Init());
  const std::size_t frames = rig.Chip().log.size();
  Input bad{};
  bad.offset_quarter_lsb = 600;
  CHECK_EQ(rig.drv.SetInput(0, bad).error, Error::InvalidArgument);
  bad = Input{};
  bad.gain = 9000;
  CHECK_EQ(rig.drv.SetInput(0, bad).error, Error::InvalidArgument);
  bad = Input{};
  // NOLINTNEXTLINE(clang-analyzer-optin.core.EnumCastOutOfRange): deliberate
  bad.range = static_cast<InputRange>(1);
  CHECK_EQ(rig.drv.SetInput(0, bad).error, Error::InvalidArgument);
  bad = Input{};
  bad.type = InputType::SingleEnded; // mixes the bank's differential and
                                     // single-ended classes
  CHECK_EQ(rig.drv.SetInput(0, bad).error, Error::InvalidArgument);
  Interface ifc{};
  ifc.delay_ns[3] = 6;
  CHECK_EQ(rig.drv.SetInterface(ifc).error, Error::InvalidArgument);
  ifc = Interface{};
  ifc.lanes = Lanes::Four; // data_on_sdout is still set
  CHECK_EQ(rig.drv.SetInterface(ifc).error, Error::InvalidArgument);
  CHECK_EQ(rig.drv.SetChannels({ChannelCount::Eight, 2}).error, Error::InvalidArgument);
  CHECK_EQ(
      rig.drv.SetFilter({FilterKind::BlockAverage, AverageLength::None, FirFilter::Fir1, false})
          .error,
      Error::InvalidArgument);
  CHECK_EQ(rig.Chip().log.size(), frames);
}

TEST_CASE("T22 the interface: lanes, delays, drive strength and the format "
          "registers") {
  Rig rig;
  REQUIRE(rig.Init());
  Interface ifc{};
  ifc.lanes = Lanes::Two;
  ifc.data_on_sdout = false;
  ifc.drive = DriveStrength::Double;
  ifc.format = DataFormat::OffsetBinary;
  ifc.delay_ns = {5, 1, 2, 3, 4, 5, 4, 3, 2};
  REQUIRE(rig.drv.SetInterface(ifc).ok());
  CHECK_EQ(rig.Chip().Reg(1, 0x0A), 0x1020U);
  CHECK_EQ(rig.Chip().Reg(1, 0x0D), 0x000AU);
  CHECK_EQ(rig.Chip().Reg(1, 0x0E), (5U << 12U) | (4U << 9U) | (3U << 6U) | (2U << 3U) | 1U);
  CHECK_EQ(rig.Chip().Reg(1, 0x0F), (2U << 9U) | (3U << 6U) | (4U << 3U) | 5U);
  CHECK(ShadowMatchesChip(rig));
}

TEST_CASE("T22 low-speed crosstalk, correction bypass and glitch filter land "
          "in their registers") {
  Config c{};
  c.low_speed_crosstalk = true;
  c.correction = Correction{false, true};
  c.window_glitch = {3, 9};
  Rig rig(c);
  REQUIRE(rig.Init());
  CHECK_EQ(rig.Chip().Reg(1, 0x11) & 0x8000U, 0x8000U);
  CHECK_EQ(rig.Chip().Reg(2, 0x37), 0x0002U);
  CHECK_EQ(rig.Chip().Reg(4, 0x37), 0x0002U);
  CHECK_EQ((rig.Chip().Reg(2, 0x21) >> 8U) & 0xFU, 3U);
  CHECK_EQ((rig.Chip().Reg(4, 0x21) >> 8U) & 0xFU, 9U);
}

TEST_CASE("T32 the shadow equals the chip after every kind of change") {
  Rig rig;
  REQUIRE(rig.Init());
  Config c = rig.drv.GetConfig();
  for (std::size_t i = 0; i < 16; ++i) {
    c.inputs[i].range = (i % 2U == 0U) ? InputRange::V10 : InputRange::V2_5;
    c.inputs[i].offset_quarter_lsb = static_cast<int16_t>((static_cast<int>(i) * 7) - 50);
    c.inputs[i].gain = static_cast<int16_t>((static_cast<int>(i) * 100) - 800);
    c.inputs[i].phase_delay = static_cast<uint8_t>(i * 3U);
    c.inputs[i].window =
        Window{(i % 3U) == 0U, static_cast<int8_t>(60 + static_cast<int>(i)),
               static_cast<int8_t>(-60 - static_cast<int>(i)), static_cast<uint8_t>(i)};
    c.inputs[i].bandwidth = (i % 4U == 0U) ? Bandwidth::Wide : Bandwidth::Low;
  }
  c.filter = Filter{FilterKind::MovingAverage, AverageLength::X16, FirFilter::Fir1, false};
  c.alarm.function = AlarmFunction::WindowAlert;
  c.reference = ReferenceSource::Internal;
  c.window_glitch = {5, 6};
  REQUIRE(rig.drv.ApplyConfig(c).ok());
  CHECK(ShadowMatchesChip(rig));
  CHECK(rig.Chip().bad_writes.empty());
  REQUIRE(rig.drv.ApplyConfig(ChipResetConfig()).ok());
  CHECK(ShadowMatchesChip(rig));
}

TEST_CASE("T32 no write ever goes to a reserved or read-only register") {
  Rig rig;
  REQUIRE(rig.Init());
  Config c = rig.drv.GetConfig();
  c.interface.randomize = true;
  c.filter = Filter{FilterKind::Fir, AverageLength::None, FirFilter::Fir7, false};
  c.test_pattern[1].enabled = true;
  REQUIRE(rig.drv.ApplyConfig(c).ok());
  CHECK(rig.Chip().bad_writes.empty());
  for (const FrameLog& f : rig.Chip().log) {
    if (f.is_register_frame && (f.addr == 0x02U)) {
      CHECK(f.data == 1U || f.data == 2U || f.data == 4U);
    }
  }
}

TEST_CASE("T22 the reference: pin, internal and external, with the settle time") {
  Rig rig;
  REQUIRE(rig.Init());
  const uint64_t before = rig.port.state.delays_us;
  REQUIRE(rig.drv.SetReference(ReferenceSource::Internal).ok());
  CHECK_EQ(rig.Chip().Reg(1, 0x10), 0x0002U);
  CHECK(rig.port.state.delays_us - before >= 30'000U);
  REQUIRE(rig.drv.SetReference(ReferenceSource::External).ok());
  CHECK_EQ(rig.Chip().Reg(1, 0x10), 0x0003U);
  REQUIRE(rig.drv.SetReference(ReferenceSource::Pin).ok());
  CHECK_EQ(rig.Chip().Reg(1, 0x10), 0x0000U);
  // NOLINTNEXTLINE(clang-analyzer-optin.core.EnumCastOutOfRange): deliberate
  const auto bad_ref = static_cast<ReferenceSource>(3);
  CHECK_EQ(rig.drv.SetReference(bad_ref).error, Error::InvalidArgument);
}
