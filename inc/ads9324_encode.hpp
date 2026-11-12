/**
 * @file ads9324_encode.hpp
 * @brief Pure register encoding: a `Config` becomes the register image the
 * driver writes, and a data frame becomes a `Frame`.
 * @copyright Copyright (c) 2026 The driver-ads9324 authors.
 * SPDX-License-Identifier: MIT
 *
 * @details
 * Everything here is `constexpr` and does no I/O, so every encoding is proved
 * with `static_assert` and tested without a bus. The image holds a value for
 * each register the configuration writes; the driver writes the ones that
 * differ from its shadow.
 */
#ifndef ADS9324_ENCODE_HPP
#define ADS9324_ENCODE_HPP

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

#include "ads9324_detail.hpp"
#include "ads9324_registers.hpp"
#include "ads9324_types.hpp"

namespace ads9324::detail {

/// The registers a configuration writes: a value per address and bank, and
/// which are written.
struct RegisterImage {
  std::array<std::array<uint16_t, reg::kAddressSpace>, reg::kBankCount> value{}; ///< Values.
  std::array<uint64_t, reg::kBankCount> touched{}; ///< Bit `a` of bank `b`: address `a` is written.
};

/// Index of a bank in the image tables.
constexpr std::size_t BankIndex(reg::Bank bank) noexcept {
  return static_cast<std::size_t>(ToUnderlying(bank));
}

/// The bit of an address in a touched mask.
constexpr uint64_t AddressBit(uint8_t address) noexcept {
  return uint64_t{1} << (static_cast<uint32_t>(address) & 63U);
}

/// `a | b` as `uint16_t`, on unsigned operands.
constexpr uint16_t Or(uint16_t a, uint16_t b) noexcept {
  return static_cast<uint16_t>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

/// `mask` when `condition` holds, else 0.
constexpr uint16_t Bit(bool condition, uint16_t mask) noexcept {
  return condition ? mask : static_cast<uint16_t>(0U);
}

/// Set a whole register of the image and mark it written.
constexpr void Put(RegisterImage& img, reg::Bank bank, uint8_t address, uint16_t value) noexcept {
  if (address >= reg::kAddressSpace) {
    return;
  }
  img.value[BankIndex(bank)][address] = value;
  img.touched[BankIndex(bank)] |= AddressBit(address);
}

/// The value of a register of the image.
constexpr uint16_t Get(const RegisterImage& img, reg::Bank bank, uint8_t address) noexcept {
  return (address < reg::kAddressSpace) ? img.value[BankIndex(bank)][address]
                                        : static_cast<uint16_t>(0U);
}

/// Whether the image writes a register.
constexpr bool IsTouched(const RegisterImage& img, reg::Bank bank, uint8_t address) noexcept {
  return (img.touched[BankIndex(bank)] & AddressBit(address)) != 0U;
}

/// Replace the bits of `mask` in a register of the image by `bits` and mark it
/// written.
constexpr void Merge(RegisterImage& img, reg::Bank bank, uint8_t address, uint16_t mask,
                     uint16_t bits) noexcept {
  const uint32_t kept =
      static_cast<uint32_t>(Get(img, bank, address)) & ~static_cast<uint32_t>(mask);
  Put(img, bank, address, static_cast<uint16_t>(kept | (static_cast<uint32_t>(bits) & mask)));
}

/// The code of an enumerator in a register field.
template <Enumeration E>
constexpr uint32_t Code(E e) noexcept {
  return static_cast<uint32_t>(ToUnderlying(e));
}

constexpr void EncodeInterface(RegisterImage& img, const Interface& i) noexcept {
  using reg::Bank;
  uint16_t gc3 = reg::DoutLaneSel::Insert(0_u16, Code(i.lanes));
  gc3 = reg::DoutLength::Insert(gc3, Code(i.length));
  gc3 = Or(gc3, Bit(i.data_on_sdout, reg::kAdcDataSdoutEn));
  gc3 = Or(gc3, Bit(i.format == DataFormat::OffsetBinary, reg::kEnOfsBinary));
  gc3 = Or(gc3, Bit(i.randomize, reg::kEnXorPatt));
  gc3 = Or(gc3, Bit(HasDiagByte(i), reg::kEnDiagFlag));
  Put(img, Bank::Common, reg::kGenCfg3, gc3);
  Put(img, Bank::Common, reg::kXorBitsCtl, i.randomize ? reg::kXorBitsRandomizer : 0_u16);
  const auto& d = i.delay_ns;
  uint16_t d1 = reg::DelaySdout::Insert(0_u16, d[0]);
  d1 = reg::DelayD3::Insert(d1, d[4]);
  d1 = reg::DelayD2::Insert(d1, d[3]);
  d1 = reg::DelayD1::Insert(d1, d[2]);
  d1 = reg::DelayD0::Insert(d1, d[1]);
  uint16_t d2 = reg::DelayD7::Insert(0_u16, d[8]);
  d2 = reg::DelayD6::Insert(d2, d[7]);
  d2 = reg::DelayD5::Insert(d2, d[6]);
  d2 = reg::DelayD4::Insert(d2, d[5]);
  Put(img, Bank::Common, reg::kDigDelayCfg1, d1);
  Put(img, Bank::Common, reg::kDigDelayCfg2, d2);
  bool any_delay = false;
  for (const uint8_t v : d) {
    any_delay = any_delay || (v != 0U);
  }
  uint16_t gc4 = reg::DriveStrengthField::Insert(0_u16, Code(i.drive));
  gc4 = Or(gc4, Bit(any_delay, reg::kDigDelayEn));
  Put(img, Bank::Common, reg::kGenCfg4, gc4);
}

constexpr void EncodeAlarm(RegisterImage& img, const Alarm& a) noexcept {
  using reg::Bank;
  const bool or_masked = a.function == AlarmFunction::OrMasked;
  // Assumption A41: a set mask bit leaves the source out of the OR.
  const uint32_t mask = or_masked ? (~static_cast<uint32_t>(a.sources) & 0xFFU) : 0U;
  uint16_t v = reg::DrdyAlrmSel::Insert(0_u16, Code(a.function));
  v = reg::AlarmMask::Insert(v, mask);
  v = Or(v, Bit(a.active_low, reg::kAlrmPol));
  v = Or(v, Bit(a.pulse, reg::kAlrmType));
  Put(img, Bank::Common, reg::kDrdyAlarmSel, v);
  // GEN_CFG4 is shared with the interface encoder: merge only ALRM_DIS.
  Merge(img, Bank::Common, reg::kGenCfg4, reg::kAlrmDis, Bit(a.disabled, reg::kAlrmDis));
}

constexpr uint16_t ReferenceBits(ReferenceSource source) noexcept {
  switch (source) {
  case ReferenceSource::Internal:
    return reg::kRefselCtrlDis;
  case ReferenceSource::External:
    return Or(reg::kRefselCtrlDis, reg::kExtRefEn);
  case ReferenceSource::Pin:
  default:
    break;
  }
  return 0_u16;
}

/// ADC_CH_SEL: the group number in the top bits of the 3-bit field (Table 7-12,
/// `xx` bits unused).
constexpr uint32_t ChannelSelCode(const ChannelSelect& select) noexcept {
  const auto group = static_cast<uint32_t>(select.group);
  switch (select.count) {
  case ChannelCount::Eight:
    return group << 2U;
  case ChannelCount::Four:
    return group << 1U;
  case ChannelCount::Two:
  case ChannelCount::All16:
  default:
    break;
  }
  return group;
}

constexpr void EncodeAnalog(RegisterImage& img, const Config& c, uint16_t cal_mode) noexcept {
  using reg::Bank;
  Put(img, Bank::Common, reg::kAnaCfg1, ReferenceBits(c.reference));
  uint16_t a2 = reg::AdcNumSel::Insert(0_u16, Code(c.channels.count));
  a2 = reg::AdcChSel::Insert(a2, ChannelSelCode(c.channels));
  a2 = Or(a2, Bit(c.low_speed_crosstalk, reg::kXtalkLowSpeed));
  Put(img, Bank::Common, reg::kAnaCfg2, a2);
  uint16_t cal = reg::AdcCalMode::Insert(0_u16, cal_mode);
  cal = Or(cal, Bit(IsDifferential(c.inputs[0]), reg::kSeDiffAin1To8));
  cal = Or(cal, Bit(IsDifferential(c.inputs[8]), reg::kSeDiffAin9To16));
  Put(img, Bank::Common, reg::kAdcCal, cal);
}

constexpr bool AnyPhaseDelay(const Config& c) noexcept {
  return std::ranges::any_of(c.inputs, [](const Input& in) { return in.phase_delay != 0U; });
}

constexpr void EncodeFilter(RegisterImage& img, const Config& c) noexcept {
  using reg::Bank;
  const Filter& f = c.filter;
  uint16_t v = Bit(AnyPhaseDelay(c), reg::kPhaseDelayEn);
  if (f.kind == FilterKind::Fir) {
    v = reg::FirFiltSel::Insert(v, Code(f.fir));
  } else if (f.kind == FilterKind::MovingAverage) {
    v = reg::MvgAvgLen::Insert(v, Code(f.length));
  } else if (f.kind == FilterKind::BlockAverage) {
    v = reg::BlkAvgOsr::Insert(v, Code(f.length));
    v = Or(v, Bit(f.internal_clock, reg::kIntTrigMode));
  } else {
    v = Or(v, 0_u16);
  }
  Put(img, Bank::Common, reg::kDigFilter, v);
  // One gate for every digital filter (section 7.3.11.2, assumption A31).
  const bool gate = f.kind != FilterKind::None;
  Put(img, Bank::Common, reg::kGenCfg5, Bit(gate, Or(reg::kAvgModeOvrEn, reg::kAvgMode)));
}

/// The PGA_CONFIG byte of a channel: CME_CORR_EN, CM_RANGE, INPUT_RANGE.
constexpr uint8_t PgaByte(const Input& in) noexcept {
  uint8_t b = reg::InputRangeField::Insert(0_u8, Code(in.range));
  b = reg::CmRange::Insert(b, Code(in.type));
  return reg::CmeCorrEn::Insert(b, in.cm_correction ? 1U : 0U);
}

constexpr uint16_t HalfMask(uint8_t local, uint16_t mask) noexcept {
  return static_cast<uint16_t>(static_cast<uint32_t>(mask) << reg::HalfShift(local));
}

constexpr uint16_t Twos(int32_t value, uint32_t bits) noexcept {
  return static_cast<uint16_t>(static_cast<uint32_t>(value) & ((uint32_t{1} << bits) - 1U));
}

constexpr void EncodeChannel(RegisterImage& img, uint8_t channel, const Input& in) noexcept {
  const reg::ChannelLocation at = reg::Locate(channel);
  const uint8_t l = at.local;
  const uint32_t half = reg::HalfShift(l);
  const auto pga = static_cast<uint16_t>(static_cast<uint32_t>(PgaByte(in)) << half);
  Merge(img, at.bank, reg::PgaConfigAddress(l), HalfMask(l, 0x00FF_u16), pga);
  const uint32_t bw_shift = 2U * static_cast<uint32_t>(l);
  Merge(img, at.bank, reg::kPgaBwSel, static_cast<uint16_t>(3U << bw_shift),
        static_cast<uint16_t>(Code(in.bandwidth) << bw_shift));
  const auto delay = static_cast<uint16_t>(static_cast<uint32_t>(in.phase_delay) << half);
  Merge(img, at.bank, reg::PhaseDelayAddress(l), HalfMask(l, 0x00FF_u16), delay);
  Put(img, at.bank, reg::OffsetAddress(l), Twos(in.offset_quarter_lsb, 10U));
  Put(img, at.bank, reg::GainAddress(l), Twos(in.gain, 14U));
  const auto th = static_cast<uint16_t>(
      (static_cast<uint32_t>(static_cast<uint8_t>(in.window.high_code)) << 8U) |
      static_cast<uint32_t>(static_cast<uint8_t>(in.window.low_code)));
  Put(img, at.bank, reg::DwcThAddress(l), th);
  const auto hys = static_cast<uint16_t>(static_cast<uint32_t>(in.window.hysteresis) << half);
  Merge(img, at.bank, reg::DwcHysAddress(l), HalfMask(l, 0x00FF_u16), hys);
  const auto en = static_cast<uint16_t>(in.window.enabled ? (1U << l) : 0U);
  Merge(img, at.bank, reg::kDwcCfg, static_cast<uint16_t>(1U << l), en);
}

constexpr void EncodeBankSettings(RegisterImage& img, const Config& c) noexcept {
  for (uint32_t g = 0U; g < 2U; ++g) {
    const reg::Bank bank = (g == 0U) ? reg::Bank::Ain1To8 : reg::Bank::Ain9To16;
    Merge(img, bank, reg::kDwcCfg, 0x0F00_u16,
          reg::DwcGlitchFilt::Insert(0_u16, c.window_glitch[g]));
    uint16_t corr = Bit(!c.correction.offset_enabled, reg::kOfsCorrDis);
    corr = Or(corr, Bit(!c.correction.gain_enabled, reg::kGanCorrDis));
    Put(img, bank, reg::kCorrection, corr);
    const TestPattern& t = c.test_pattern[g];
    uint16_t cfg = reg::TpMode::Insert(0_u16, Code(t.kind));
    cfg = Or(cfg, Bit(t.enabled, reg::kTpEn));
    cfg = Or(cfg, Bit(!t.index_insertion, reg::kTpDisIdx));
    cfg = Or(cfg, Bit(t.step_every_convst, reg::kTpUpdMode));
    Put(img, bank, reg::kTpCfg, cfg);
    for (uint32_t i = 0U; i < 8U; ++i) {
      const reg::ChannelLocation at = reg::Locate(static_cast<uint8_t>((8U * g) + i));
      Put(img, at.bank, reg::TpAddress(at.local), t.value[i]);
    }
  }
}

/**
 * @brief Every register a configuration writes.
 * @param c A valid configuration.
 * @param cal_mode The ADC_CAL_MODE bits to keep (0 until a calibration has
 * run).
 * @return The image.
 */
constexpr RegisterImage Encode(const Config& c, uint16_t cal_mode) noexcept {
  RegisterImage img{};
  EncodeInterface(img, c.interface);
  EncodeAlarm(img, c.alarm);
  EncodeAnalog(img, c, cal_mode);
  EncodeFilter(img, c);
  for (uint8_t ch = 0U; ch < reg::kChannelCount; ++ch) {
    EncodeChannel(img, ch, c.inputs[ch]);
  }
  EncodeBankSettings(img, c);
  return img;
}

// Table 7-18: 16-bit words, one lane on SDOUT is 0x0032 in GEN_CFG3.
static_assert(Get(Encode(Config{}, 0U), reg::Bank::Common, reg::kGenCfg3) == 0x0032U);
// The chip's reset state encodes to the reset values of the interface
// registers.
static_assert(Get(Encode(ChipResetConfig(), 0U), reg::Bank::Common, reg::kGenCfg3) == 0x0000U);
static_assert(Get(Encode(Config{}, 0U), reg::Bank::Common, reg::kGenCfg4) == 0x0000U);
// A randomizer asks for the DIAG byte, which carries the PRBS byte, and sets
// XOR_BITS_CTL.
static_assert(Get(Encode(
                      [] {
                        Config c{};
                        c.interface.randomize = true;
                        return c;
                      }(),
                      0U),
                  reg::Bank::Common, reg::kXorBitsCtl) == 0x00F1U);
// AIN16 is local index 0 of the second bank, the low byte of its first
// PGA_CONFIG register.
static_assert(Get(Encode(
                      [] {
                        Config c{};
                        c.inputs[15].range = InputRange::V10;
                        return c;
                      }(),
                      0U),
                  reg::Bank::Ain9To16, reg::kPgaConfig) == 0x0004U);

/// The signed code is at a limit of the 16-bit range.
constexpr bool IsClipped(int16_t code) noexcept {
  return (code == 32767) || (code == -32768);
}

/// Decode the words of a frame into `frame` (the trailer is read by
/// `DecodeFrame`).
constexpr void DecodeWords(const Config& c, std::span<const uint8_t> bytes, uint16_t mask,
                           Frame& frame) noexcept {
  const uint32_t per = (c.interface.length == WordLength::Bits24) ? 3U : 2U;
  const uint32_t n = FrameChannels(c.channels);
  for (uint32_t slot = 0U; slot < n; ++slot) {
    const uint32_t at = slot * per;
    const uint8_t ch = FrameSlotChannel(c.channels, slot);
    const uint32_t word =
        (static_cast<uint32_t>(bytes[at]) << 8U) | static_cast<uint32_t>(bytes[at + 1U]);
    const auto raw = static_cast<uint16_t>(word ^ static_cast<uint32_t>(mask));
    const int16_t code = WireToCode(raw, c.interface.format);
    frame.raw[ch] = raw;
    frame.codes[ch] = code;
    frame.low_byte[ch] = (per == 3U) ? bytes[at + 2U] : static_cast<uint8_t>(0U);
    frame.microvolts[ch] = CodeToMicrovolts(code, c.inputs[ch].range);
    frame.present_mask =
        static_cast<uint16_t>(static_cast<uint32_t>(frame.present_mask) | (1U << ch));
    if (IsClipped(code)) {
      frame.clipped_mask =
          static_cast<uint16_t>(static_cast<uint32_t>(frame.clipped_mask) | (1U << ch));
    }
  }
}

/**
 * @brief Decode one data frame as the interface in `c` sends it
 * (section 7.3.14).
 * @param c The configuration the chip runs with.
 * @param bytes The frame, exactly `FrameBytes()` long.
 * @return The frame, or `InvalidArgument` for a wrong length.
 */
constexpr Result<Frame> DecodeFrame(const Config& c, std::span<const uint8_t> bytes) noexcept {
  if (bytes.size() != FrameBytes(c.interface, c.channels)) {
    return Result<Frame>::Failure(Error::InvalidArgument);
  }
  Frame frame{};
  const uint32_t per = (c.interface.length == WordLength::Bits24) ? 3U : 2U;
  const uint32_t trailer = FrameChannels(c.channels) * per;
  const bool diag = HasDiagByte(c.interface);
  frame.diag = diag ? bytes[trailer] : static_cast<uint8_t>(0U);
  frame.prbs = c.interface.randomize ? bytes[trailer + 1U] : static_cast<uint8_t>(0U);
  const uint16_t mask =
      c.interface.randomize ? DerandomizeMask(frame.prbs) : static_cast<uint16_t>(0U);
  DecodeWords(c, bytes, mask, frame);
  frame.window_alert = (static_cast<uint32_t>(frame.diag) & kDiagWindowAlert) != 0U;
  frame.reset_detected = (static_cast<uint32_t>(frame.diag) & kDiagResetDetect) != 0U;
  return Result<Frame>::Success(frame);
}

// Table 7-12, 8 channels, group 0: AIN1 to AIN4 and AIN13 to AIN16.
static_assert(FrameSlotChannel({.count = ChannelCount::Eight, .group = 0}, 4) == 12U);
static_assert(FrameSlotChannel({.count = ChannelCount::Eight, .group = 1}, 0) == 4U);
static_assert((ChannelSelCode({.count = ChannelCount::Four, .group = 1}) == 2U) &&
              (ChannelSelCode({.count = ChannelCount::Eight, .group = 1}) == 4U));
// Equation 7: PRBS 0x1F gives the mask 0x1FFF.
static_assert(DerandomizeMask(0x1F) == 0x1FFFU);

/// The code (0 to 7) of a range for the register field, kept as an inline
/// helper for tests.
constexpr uint32_t RangeCode(InputRange r) noexcept {
  return Code(r);
}

} // namespace ads9324::detail

#endif // ADS9324_ENCODE_HPP
