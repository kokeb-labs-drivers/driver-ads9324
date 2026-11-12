/**
 * @file test_types.cpp
 * @brief The pure part of the driver: errors, enumerations, Validate(), frame
 * decoding and the helper functions, checked against the model's own tables
 * (T25, T27, T29).
 */
#include <array>
#include <string_view>

#include "ads9324_encode.hpp"
#include "mock_ads9324.hpp"
#include "test_harness.hpp"

using namespace ads9324;
using ads9324_test::Ads9324Model;

// NOLINTBEGIN(clang-analyzer-optin.core.EnumCastOutOfRange): out-of-range enums
// test Validate() and IsValid()

static_assert(static_cast<int>(Error::Unsupported) == 9);
static_assert(static_cast<int>(Error::UnexpectedReset) == 16);
static_assert(static_cast<int>(Error::DataMismatch) == 19);

TEST_CASE("T25 error names") {
  CHECK(std::string_view(ToString(Error::Ok)) == std::string_view("Ok"));
  CHECK(std::string_view(ToString(Error::DataMismatch)) == std::string_view("DataMismatch"));
  CHECK(std::string_view(ToString(Error::Crc)) == std::string_view("Crc"));
  CHECK(std::string_view(ToString(static_cast<Error>(200))) == std::string_view("Unknown"));
  for (uint32_t v = 0; v < 20; ++v) {
    const auto e = static_cast<Error>(v);
    const bool named = std::string_view(ToString(e)) != std::string_view("Unknown");
    CHECK_EQ(named, (v <= 9U) || (v >= 16U));
  }
}

TEST_CASE("T25 Result carries a value or an error") {
  const auto ok = Result<int>::Success(5);
  CHECK(ok.ok());
  CHECK(static_cast<bool>(ok));
  const auto bad = Result<int>::Failure(Error::Bus);
  CHECK(!bad.ok());
  CHECK_EQ(bad.value, 0);
  CHECK(Result<void>::Success().ok());
  CHECK(!static_cast<bool>(Result<void>::Failure(Error::Timeout)));
}

TEST_CASE("T25 enumerations accept their enumerators and nothing past them") {
  CHECK(detail::IsEnumerator(Lanes::One));
  CHECK(!detail::IsEnumerator(static_cast<Lanes>(4)));
  CHECK(detail::IsEnumerator(AverageLength::X128));
  CHECK(!detail::IsEnumerator(static_cast<AverageLength>(12)));
  CHECK(!detail::IsEnumerator(static_cast<ReferenceSource>(3)));
  CHECK(!detail::IsEnumerator(static_cast<ChannelCount>(4)));
  CHECK(!detail::IsEnumerator(static_cast<DriveStrength>(4)));
  CHECK(!detail::IsEnumerator(static_cast<FilterKind>(4)));
  CHECK(!detail::IsEnumerator(static_cast<Bandwidth>(2)));
  CHECK(!detail::IsEnumerator(static_cast<DataFormat>(2)));
  CHECK(!detail::IsEnumerator(static_cast<ResetKind>(2)));
  CHECK(!detail::IsEnumerator(static_cast<Verify>(2)));
  CHECK(!detail::IsEnumerator(static_cast<ReadySource>(2)));
  CHECK(!detail::IsEnumerator(static_cast<CalibrationTrigger>(2)));
  CHECK(!detail::IsEnumerator(static_cast<reg::Bank>(3)));
}

TEST_CASE("T25 documented codes only: reserved codes are not valid") {
  CHECK(IsValid(InputType::SingleEndedOpenWireSafe));
  CHECK(!IsValid(static_cast<InputType>(1)));
  CHECK(!IsValid(static_cast<InputType>(7)));
  CHECK(!IsValid(static_cast<InputRange>(1)));
  CHECK(!IsValid(static_cast<InputRange>(6)));
  CHECK(IsValid(WordLength::Bits24));
  CHECK(!IsValid(static_cast<WordLength>(1)));
  CHECK(!IsValid(static_cast<AlarmFunction>(2)));
  CHECK(!IsValid(static_cast<AlarmFunction>(7)));
  CHECK(IsValid(AlarmFunction::OrMasked));
  CHECK(!IsValid(static_cast<FirFilter>(0)));
  CHECK(!IsValid(static_cast<FirFilter>(6)));
  CHECK(IsValid(FirFilter::Fir7));
  CHECK(!IsValid(static_cast<PatternKind>(1)));
  CHECK(!IsValid(static_cast<CalibrationMode>(0)));
  CHECK(IsValid(CalibrationMode::OffsetAndGain));
}

TEST_CASE("T27 Validate accepts the defaults and every documented combination "
          "it should") {
  CHECK_EQ(Validate(Config{}), Error::Ok);
  CHECK_EQ(Validate(ChipResetConfig()), Error::Ok);
  Config c{};
  c.supply = Supply{4750, 1700, 3600};
  CHECK_EQ(Validate(c), Error::Ok);
  for (auto& in : c.inputs) {
    in.type = InputType::SingleEnded;
    in.range = InputRange::V12_5;
  }
  CHECK_EQ(Validate(c), Error::Ok);
  c.inputs[3].type = InputType::SingleEndedOpenWireSafe; // single-ended class too
  CHECK_EQ(Validate(c), Error::Ok);
}

TEST_CASE("T27 Validate rejects each out-of-range value") {
  const auto bad = [](auto edit) {
    Config c{};
    edit(c);
    return Validate(c) == Error::InvalidArgument;
  };
  CHECK(bad([](Config& c) { c.sampling.convst_hz = 99'999; }));
  CHECK(bad([](Config& c) { c.sampling.convst_hz = 1'000'001; }));
  CHECK(bad([](Config& c) { c.sampling.ready_timeout_us = 0; }));
  CHECK(bad([](Config& c) { c.sampling.ready_timeout_us = kReadyTimeoutMaxUs + 1U; }));
  CHECK(bad([](Config& c) { c.supply.avdd_5v_mv = 4000; }));
  CHECK(bad([](Config& c) { c.supply.avdd_1v8_mv = 2000; }));
  CHECK(bad([](Config& c) { c.supply.iovdd_mv = 4000; }));
  CHECK(bad([](Config& c) { c.window_glitch[1] = 16; }));
  CHECK(bad([](Config& c) { c.interface.delay_ns[8] = 6; }));
  CHECK(bad([](Config& c) { c.inputs[15].offset_quarter_lsb = -513; }));
  CHECK(bad([](Config& c) { c.inputs[15].gain = -8193; }));
  CHECK(bad([](Config& c) { c.channels = {ChannelCount::Four, 4}; }));
  CHECK(bad([](Config& c) { c.channels = {ChannelCount::Two, 8}; }));
  CHECK(bad([](Config& c) { c.channels = {ChannelCount::All16, 1}; }));
  CHECK(bad([](Config& c) { c.filter.kind = FilterKind::MovingAverage; }));
  CHECK(bad(
      [](Config& c) { c.filter = {FilterKind::Fir, AverageLength::None, FirFilter::Fir1, true}; }));
  CHECK(bad([](Config& c) { c.inputs[8].type = InputType::SingleEnded; }));
  CHECK(bad([](Config& c) { c.interface.lanes = Lanes::Eight; }));
  CHECK(bad([](Config& c) { c.calibration.mode = static_cast<CalibrationMode>(0); }));
  CHECK(bad([](Config& c) { c.test_pattern[1].kind = static_cast<PatternKind>(1); }));
  CHECK(bad([](Config& c) { c.reference = static_cast<ReferenceSource>(7); }));
}

TEST_CASE("T25 the Validate result is Unsupported only for the daisy chain") {
  Config c{};
  c.daisy = DaisyChain{true, 1, 2};
  CHECK_EQ(Validate(c), Error::Unsupported);
}

TEST_CASE("T11 the slot to channel mapping equals the model's Table 7-12 for "
          "all 15 modes") {
  const struct {
    ChannelCount count;
    uint32_t groups;
    uint32_t shift;
  } rows[] = {{ChannelCount::All16, 1, 0},
              {ChannelCount::Eight, 2, 2},
              {ChannelCount::Four, 4, 1},
              {ChannelCount::Two, 8, 0}};
  for (const auto& row : rows) {
    for (uint32_t g = 0; g < row.groups; ++g) {
      const ChannelSelect select{row.count, static_cast<uint8_t>(g)};
      const auto expect =
          Ads9324Model::SlotChannels(static_cast<uint32_t>(row.count), g << row.shift);
      CHECK_EQ(FrameChannels(select), expect.size());
      for (uint32_t slot = 0; slot < expect.size(); ++slot) {
        CHECK_EQ(static_cast<int>(FrameSlotChannel(select, slot)), expect[slot]);
      }
      CHECK_EQ(detail::ChannelSelCode(select), g << row.shift);
    }
  }
}

TEST_CASE("T10 randomizer mask, coding and frame sizes") {
  CHECK_EQ(DerandomizeMask(0x1F), 0x1FFFU);
  CHECK_EQ(DerandomizeMask(0x00), 0x0000U);
  CHECK_EQ(DerandomizeMask(0xFE), 0xFE00U);
  CHECK_EQ(DerandomizeMask(0xFF), 0xFFFFU);
  CHECK_EQ(WireToCode(0x8000, DataFormat::OffsetBinary), 0);
  CHECK_EQ(WireToCode(0x0000, DataFormat::OffsetBinary), -32768);
  CHECK_EQ(WireToCode(0xFFFF, DataFormat::OffsetBinary), 32767);
  CHECK_EQ(WireToCode(0xFFFF, DataFormat::TwosComplement), -1);
  Interface i{};
  CHECK_EQ(FrameBytes(i, ChannelSelect{}), 32U);
  i.length = WordLength::Bits24;
  CHECK_EQ(FrameBytes(i, ChannelSelect{}), 48U);
  i.append_diag = true;
  CHECK_EQ(FrameBytes(i, ChannelSelect{}), 49U);
  i.length = WordLength::Bits16;
  i.randomize = true;
  CHECK_EQ(FrameBytes(i, ChannelSelect{}), 34U);
  CHECK_EQ(FrameBytes(Interface{}, ChannelSelect{ChannelCount::Two, 0}), 4U);
  CHECK(FrameBytes(Interface{.length = WordLength::Bits24, .append_diag = true, .randomize = false},
                   ChannelSelect{}) <= kMaxFrameBytes);
}

TEST_CASE("T10 DecodeFrame refuses a frame of the wrong length") {
  const std::array<uint8_t, 5> bytes{};
  CHECK_EQ(detail::DecodeFrame(Config{}, bytes).error, Error::InvalidArgument);
  const std::array<uint8_t, 32> good{};
  CHECK(detail::DecodeFrame(Config{}, good).ok());
}

TEST_CASE("T09 microvolts round half away from zero") {
  CHECK_EQ(CodeToMicrovolts(0, InputRange::V5), 0);
  CHECK_EQ(CodeToMicrovolts(32767, InputRange::V5), 4'999'847);
  CHECK_EQ(CodeToMicrovolts(-32768, InputRange::V5), -5'000'000);
  CHECK_EQ(CodeToMicrovolts(16384, InputRange::V10), 5'000'000);
  CHECK_EQ(CodeToMicrovolts(-1, InputRange::V12_5), -381);
  CHECK_EQ(CodeToMicrovolts(1, InputRange::V12_5), 381);
  CHECK_EQ(CodeToMicrovolts(1, InputRange::V2_5), 76);
  CHECK_EQ(CodeToMicrovolts(1, InputRange::V6_25), 191);
  CHECK_EQ(RangeMillivolts(static_cast<InputRange>(1)), 5000);
}

TEST_CASE("T13 conversion time, output rate and averaging helpers") {
  Config c{};
  CHECK_EQ(ConversionTimeUs(c), 2U);
  c.filter = {FilterKind::BlockAverage, AverageLength::X128, FirFilter::Fir1, false};
  CHECK_EQ(ConversionTimeUs(c), 1272U); // section 5.7: 127 x 10 us + 1.25 us at 100 kHz
  c.filter.internal_clock = true;
  CHECK_EQ(ConversionTimeUs(c), 1272U);
  c.sampling.convst_hz = 1'000'000;
  c.filter.length = AverageLength::X2;
  CHECK_EQ(ConversionTimeUs(c), 3U); // 2.25 us rounded up
  c.filter = {FilterKind::Fir, AverageLength::None, FirFilter::Fir7, false};
  CHECK_EQ(ConversionTimeUs(c), 9U); // 7 x 1 us + 1.25 us, rounded up
  c.sampling.convst_hz = 100'000;
  c.filter = {FilterKind::BlockAverage, AverageLength::X128, FirFilter::Fir1, true};
  CHECK_EQ(OutputRateHz(c), 781U);
  c.filter = {FilterKind::Fir, AverageLength::None, FirFilter::Fir7, false};
  CHECK_EQ(OutputRateHz(c), 12'500U);
  CHECK_EQ(FirDecimation(FirFilter::Fir3), 4U);
  CHECK_EQ(FirDecimation(FirFilter::Fir4), 2U);
  const uint32_t factors[] = {1, 2, 4, 6, 8, 10, 12, 16, 20, 32, 64, 128};
  for (uint32_t i = 0; i < 12; ++i) {
    CHECK_EQ(AverageFactor(static_cast<AverageLength>(i)), factors[i]);
  }
  CHECK_EQ(AverageFactor(static_cast<AverageLength>(40)), 1U);
}

TEST_CASE("T17 offset and gain helpers follow equations 1 to 4") {
  CHECK_EQ(OffsetCorrectionFor(100, 90), 40);
  CHECK_EQ(OffsetCorrectionFor(0, 1000), -512);
  CHECK_EQ(OffsetCorrectionFor(1000, 0), 511);
  CHECK_EQ(GainCorrectionFor(32768, 32000), 1573);
  CHECK_EQ(GainCorrectionFor(32000, 32768), -1536);
  CHECK_EQ(GainCorrectionFor(1, 0), 0);
  CHECK_EQ(GainCorrectionFor(32767, 1), 8191);
  CHECK_EQ(GainCorrectionFor(-32767, 1), -8192);
  CHECK_EQ(GainCorrectionFor(100, -50), -8192);
}

TEST_CASE("T14 a stuck frame is four or more identical 0x00 or 0xFF bytes") {
  const std::array<uint8_t, 4> zeros{};
  const std::array<uint8_t, 4> ones{0xFF, 0xFF, 0xFF, 0xFF};
  const std::array<uint8_t, 4> mixed{0xFF, 0xFF, 0x00, 0xFF};
  const std::array<uint8_t, 4> same{0x12, 0x12, 0x12, 0x12};
  CHECK(IsStuckFrame(zeros));
  CHECK(IsStuckFrame(ones));
  CHECK(!IsStuckFrame(mixed));
  CHECK(!IsStuckFrame(same));
  CHECK(!IsStuckFrame(std::span<const uint8_t>{zeros}.first(3)));
}

TEST_CASE("T16 the encoder agrees with the model's own register layout for "
          "every channel") {
  Ads9324Model model;
  for (uint8_t ch = 0; ch < 16; ++ch) {
    Config c{};
    c.inputs[ch].range = InputRange::V6_25;
    c.inputs[ch].offset_quarter_lsb = -2;
    c.inputs[ch].gain = 5;
    c.inputs[ch].window = Window{true, 9, -9, 4};
    const auto img = detail::Encode(c, 0U);
    const auto local = static_cast<uint32_t>(model.LocalIndex(ch));
    const uint8_t bank_code = Ads9324Model::ChannelBank(ch);
    const reg::Bank bank = (bank_code == 2U) ? reg::Bank::Ain1To8 : reg::Bank::Ain9To16;
    CHECK_EQ(detail::Get(img, bank, static_cast<uint8_t>(0x11U + local)), 0x03FEU);
    CHECK_EQ(detail::Get(img, bank, static_cast<uint8_t>(0x19U + local)), 5U);
    CHECK_EQ(detail::Get(img, bank, static_cast<uint8_t>(0x22U + local)), 0x09F7U);
    CHECK_EQ(detail::Get(img, bank, 0x21) & (1U << local), 1U << local);
  }
}
// NOLINTEND(clang-analyzer-optin.core.EnumCastOutOfRange)
