/**
 * @file mock_ads9324.hpp
 * @brief An independent model of the ADS9324 and the mock ports that put it on
 * a bus.
 *
 * The model is written from the datasheet, not from the driver: it uses its own
 * numeric constants and its own channel tables, so a mistake in the driver's
 * register map or frame decoding shows up as a difference instead of being
 * mirrored. It implements the SPI frame decoder (24-bit frames, bank select,
 * two-frame reads), all 116 registers with their reset values and read-only
 * behaviour, a conversion engine with coding, ranges, offset, gain, test
 * patterns, block average, the digital window comparator, the DRDY/ALARM pin,
 * the data-frame packer (lanes, 16 or 24 bits, DIAG, PRBS randomizer), the ADC
 * calibration state machine, standby and reset detection.
 *
 * Every ambiguity of the datasheet that the driver resolves by a design
 * decision has a knob in `Knobs`; the tests flip the knob and check the driver
 * still behaves (docs/datasheet-ambiguities).
 */
#ifndef ADS9324_MOCK_ADS9324_HPP
#define ADS9324_MOCK_ADS9324_HPP

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

#include "ads9324_spi_interface.hpp"
#include "ads9324_types.hpp"

namespace ads9324_test {

/// How the second channel bank is laid out (assumption A19).
enum class Bank2Order : uint8_t { Mirrored, Ascending };

/// The model's switches for the datasheet's open questions (A1 to A48) and for
/// faults.
struct Knobs {
  uint8_t spi_mode{0};              ///< A1: the mode the chip decodes; other modes give garbage.
  uint32_t reset_settle_us{30'000}; ///< A3: access before this time after a reset is ignored.
  uint32_t reset_min_us{1};         ///< A4: shortest RESET pulse the chip sees.
  bool rdcmd_any_bank{true};        ///< A6: GEN_CFG1 is reachable while a channel bank is selected.
  bool sdout_after_write_zero{true}; ///< A7: reply to a non-read frame (else the last reply).
  bool decode_data_frames{false};    ///< A9: a data readout frame is also decoded as a command.
  bool reset_detect_init{true};      ///< A11: RESET_DETECT_FLAG reads 1 after a reset.
  bool xor_with_24bit{false};        ///< A13: the randomizer also works on 24-bit words.
  bool diag_before_prbs{true};       ///< A14: DIAG byte first, then PRBS (else reversed).
  bool low_byte_extra{false};        ///< A15: the 24-bit extra byte holds noise bits (else 0).
  bool dwc_reset_text{false};        ///< A16: DWC_TH resets to 0x7F80 (text) instead of 0xFF00.
  bool bank2_mirrored{true};         ///< A19/A21: bank 2 is reversed (the datasheet's layout).
  bool tp_mode1_accepted{false};     ///< A22: TP_MODE 1 gives a pattern too.
  bool tp_index_or{false};           ///< A24: the channel index is ORed instead of added.
  bool offset_subtracted{false};     ///< A26: OFS is subtracted instead of added.
  bool avg_gate_required{true};      ///< A31: filters work only with AVG_MODE_OVR_EN and AVG_MODE.
  bool int_trig_shared{true};        ///< A35: INT_TRIG_MODE is also the oversampling clock.
  uint32_t cal_busy_us{125'000};     ///< A36: single-shot calibration time.
  bool cal_at_power_up{false};       ///< A38: a calibration is running after power-up.
  bool cal_needs_high_end{true};     ///< A39: single-shot calibration needs CONVST to end high.
  bool standby_retains{true};        ///< A40: registers survive standby.
  bool alrm_mask_one_masks{true};    ///< A41: a set ALRM_MASK bit leaves the source out.
  uint32_t ref_settle_us{30'000};    ///< A44: reference switch settle time.
  uint32_t data_valid_us{0};         ///< A48: a result is lost after this long (0: never).
  uint32_t convst_min_ns{50};        ///< A46: shortest CONVST pulse (informational).
  // Faults.
  int fail_transfer{-1};           ///< Fail the transfer with this index; -1 never.
  bool fail_after_delivery{false}; ///< The failed transfer reached the chip first (frame decoded).
  uint16_t bank_sel_reset{1};      ///< BANK_SEL after a reset (the datasheet says 0x0001).
  bool stuck_sdout{false};         ///< Data and register replies are all zeros.
  bool float_sdout{false};         ///< Data and register replies are all ones.
  int flip_reply_bit{-1};         ///< Flip this bit (0 to 15) of the register value in every reply.
  bool reply_shifted_by_8{false}; ///< Fault: the register value sits in SDOUT[15:0], not [23:8].
  bool flip_in_channel_banks_only{false}; ///< `flip_reply_bit` only hits channel-bank reads.
  uint16_t device_id{4};            ///< DEVICE_ID the chip reports (the datasheet says 0x0004).
  uint8_t reply_low_byte{0};        ///< SDOUT[7:0] of a register reply; the datasheet says 0x00.
  bool stale_when_not_ready{false}; ///< A readout without a new result returns the last frame.
  uint32_t convst_hz{1'000'000};    ///< The CONVST rate the chip sees (tCYC = 1 / convst_hz).
  bool reset_during_run{false};     ///< The next conversion frame reports a reset in DIAG.
};

/// One logged SPI frame.
struct FrameLog {
  uint8_t addr{0};  ///< SDI[23:16].
  uint16_t data{0}; ///< SDI[15:0].
  bool is_register_frame{true};
  std::size_t len{0};
  uint8_t bank{1}; ///< The bank code selected when the frame arrived.
  bool all_zero_tx{false};
};

/// The ADS9324 as the datasheet describes it.
class Ads9324Model { // NOLINT(clang-analyzer-optin.performance.Padding): a test model, its size
                     // does not matter
public:
  Knobs knobs{};
  std::array<int32_t, 16> input_uv{}; ///< Input voltages, microvolts.
  int32_t analog_offset_lsb{20};      ///< Offset error that the calibration removes.
  std::vector<FrameLog> log;
  std::vector<uint16_t> bad_writes; ///< `(bank << 8) | addr` of writes to
                                    ///< undocumented/RO registers.
  std::array<uint16_t, 16>
      sent_raw{}; ///< Per channel: the word of the latest conversion before the randomizer.
  uint64_t now_us{0};
  uint32_t sysref_edges{0};
  uint32_t resets{0};
  uint32_t conversions{0};
  uint32_t calibrations{0};
  int transfers{0};
  bool convst{false};
  std::vector<std::pair<uint64_t, bool>> convst_edges; ///< (time in us, new level).

  Ads9324Model() {
    powerOn();
  }

  /// Register value as stored (test access).
  /// A common-bank register as a wide unsigned value.
  [[nodiscard]] uint32_t Common(uint8_t addr) const {
    return regs_[0][addr];
  }
  [[nodiscard]] uint32_t Reg(uint8_t bank_code, uint8_t addr) const {
    return regs_[bankIndex(bank_code)][addr & 63U];
  }
  /// Write a stored register directly (test setup).
  void Poke(uint8_t bank_code, uint8_t addr, uint16_t v) {
    regs_[bankIndex(bank_code)][addr & 63U] = v;
  }
  [[nodiscard]] uint8_t BankCode() const {
    return bank_;
  }
  [[nodiscard]] bool Standby() const {
    return standby_;
  }
  [[nodiscard]] bool Calibrating() const {
    return cal_running_;
  }
  [[nodiscard]] uint16_t CalMode() const {
    return cal_mode_applied_;
  }

  /// Make the chip lose its registers as a brown-out would.
  void BrownOut() {
    hardReset();
  }

  /// A hardware RESET pin change.
  void SetResetPin(bool asserted) {
    if (asserted && !reset_pin_) {
      reset_since_us_ = now_us;
    }
    if (!asserted && reset_pin_ && ((now_us - reset_since_us_) >= knobs.reset_min_us)) {
      hardReset();
    }
    reset_pin_ = asserted;
  }

  /// A CONVST level change; a falling edge starts a conversion.
  void SetConvst(bool high) {
    const bool fall = convst && !high;
    if (high && !convst && data_ready_ && (now_us <= ready_at_us_)) {
      convst_high_before_ready_ = true; // Figure 5-7, note 1
    }
    if (high != convst) {
      convst_edges.emplace_back(now_us, high);
    }
    convst = high;
    if (fall && !standby_ && accessible()) {
      onConvstFall();
    }
  }

  /// Time passes.
  void Delay(uint32_t us) {
    now_us += us;
    if (cal_running_ && (now_us >= cal_done_us_) && convstEndOk()) {
      cal_running_ = false;
      cal_mode_applied_ = cal_mode_;
      ++calibrations;
    }
  }

  /// The DRDY/ALARM pin level (the physical level).
  [[nodiscard]] bool AlarmPin() const {
    const uint32_t sel = regs_[0][0x0C];
    const bool active_low = (sel & 0x10U) != 0U;
    const bool asserted = alarmAsserted(sel & 0xFU, (sel >> 8U) & 0xFFU);
    return asserted != active_low;
  }

  /// One SPI transfer (a register frame is 3 bytes; anything else is a data
  /// readout).
  bool Transfer(const uint8_t* tx, uint8_t* rx, std::size_t len) {
    const int index = transfers++;
    const bool doomed = index == knobs.fail_transfer;
    if ((doomed && !knobs.fail_after_delivery) || (len == 0U) || (tx == nullptr)) {
      return false;
    }
    FrameLog entry{};
    entry.len = len;
    entry.bank = bank_;
    entry.is_register_frame = (len == 3U);
    entry.all_zero_tx = true;
    for (std::size_t i = 0; i < len; ++i) {
      entry.all_zero_tx = entry.all_zero_tx && (tx[i] == 0U);
    }
    std::vector<uint8_t> reply(len, 0U);
    if (len == 3U) {
      entry.addr = tx[0];
      entry.data = static_cast<uint16_t>((static_cast<uint32_t>(tx[1]) << 8U) |
                                         static_cast<uint32_t>(tx[2]));
      log.push_back(entry);
      registerFrame(entry, reply);
    } else {
      log.push_back(entry);
      // Data comes out on SDOUT only when GEN_CFG3 bit 1 (ADC_DATA_SDOUT_EN) is set; after a reset
      // the data is on D7 to D0 and SDOUT idles at zero (Table 7-11).
      if ((Common(0x0A) & 0x0002U) != 0U) {
        readout(reply);
      }
    }
    if (knobs.stuck_sdout) {
      reply.assign(len, 0x00U);
    } else if (knobs.float_sdout) {
      reply.assign(len, 0xFFU);
    }
    if ((rx != nullptr) && (knobs.spi_mode == 0U)) {
      for (std::size_t i = 0; i < len; ++i) {
        rx[i] = reply[i];
      }
    } else if (rx != nullptr) {
      for (std::size_t i = 0; i < len; ++i) {
        rx[i] = 0x55U; // wrong mode: garbage
      }
    }
    return !doomed;
  }

  /// Lane capture: the same byte stream, only when the chip is set to the same
  /// lane count.
  bool CaptureLanes(uint8_t lanes, uint8_t* rx, std::size_t len) {
    if ((len == 0U) || (rx == nullptr)) {
      return false;
    }
    FrameLog entry{};
    entry.len = len;
    entry.is_register_frame = false;
    entry.bank = bank_;
    entry.all_zero_tx = true;
    log.push_back(entry);
    std::vector<uint8_t> reply(len, 0U);
    if (lanes == LaneCount()) {
      readout(reply);
    }
    for (std::size_t i = 0; i < len; ++i) {
      rx[i] = reply[i];
    }
    return true;
  }

  /// The number of data lanes the chip is configured for.
  [[nodiscard]] uint8_t LaneCount() const {
    const uint32_t sel = (Common(0x0A) >> 4U) & 3U;
    return static_cast<uint8_t>(8U >> sel);
  }

  /// Number of logged register frames that wrote `addr` (bank independent), or
  /// read-command frames.
  [[nodiscard]] int CountWrites(uint8_t addr) const {
    int n = 0;
    for (const FrameLog& f : log) {
      n += (f.is_register_frame && (f.addr == addr)) ? 1 : 0;
    }
    return n;
  }

  /// The model's own channel table for a frame (Table 7-12).
  static std::vector<int> SlotChannels(uint32_t num_sel, uint32_t ch_sel) {
    std::vector<int> out;
    if (num_sel == 0U) {
      for (int i = 0; i < 16; ++i) {
        out.push_back(i);
      }
    } else if (num_sel == 1U) {
      const int g = static_cast<int>(ch_sel >> 2U);
      const int first[2][8] = {{0, 1, 2, 3, 12, 13, 14, 15}, {4, 5, 6, 7, 8, 9, 10, 11}};
      for (int i = 0; i < 8; ++i) {
        out.push_back(first[g][i]);
      }
    } else if (num_sel == 2U) {
      const int g = static_cast<int>(ch_sel >> 1U);
      out = {2 * g, (2 * g) + 1, 15 - (2 * g) - 1, 15 - (2 * g)};
    } else {
      const int g = static_cast<int>(ch_sel);
      out = {g, 15 - g};
    }
    return out;
  }

  /// Which bank-local index holds a channel's registers under the current
  /// bank-2 layout.
  [[nodiscard]] int LocalIndex(int channel) const {
    if (channel < 8) {
      return channel;
    }
    return knobs.bank2_mirrored ? (15 - channel) : (channel - 8);
  }

  /// Register address of a channel's field: `base + local / per`.
  [[nodiscard]] uint8_t ChannelAddr(int channel, uint8_t base, int per) const {
    return static_cast<uint8_t>(base + (LocalIndex(channel) / per));
  }
  /// Bank code of a channel.
  static uint8_t ChannelBank(int channel) {
    return (channel < 8) ? 2U : 4U;
  }

private:
  static std::size_t bankIndex(uint8_t code) {
    if (code == 2U) {
      return 1U;
    }
    return (code == 4U) ? 2U : 0U;
  }

  void powerOn() {
    hardReset();
    if (knobs.cal_at_power_up) {
      cal_running_ = true;
      cal_done_us_ = now_us + 100'000U;
    }
  }

  [[nodiscard]] bool accessible() const {
    return (now_us - reset_at_us_) >= knobs.reset_settle_us;
  }

  [[nodiscard]] bool convstEndOk() const {
    return !knobs.cal_needs_high_end || convst;
  }

  void hardReset() {
    for (auto& bank : regs_) {
      bank.fill(0U);
    }
    for (std::size_t b = 1; b <= 2; ++b) {
      for (std::size_t i = 0; i < 8; ++i) {
        regs_[b][0x22U + i] = knobs.dwc_reset_text ? 0x7F80U : 0xFF00U;
      }
      for (std::size_t i = 1; i < 4; ++i) {
        regs_[b][0x2AU + i] = 0xFF00U;
      }
    }
    regs_[0][0x02] = knobs.bank_sel_reset;
    regs_[0][0x21] = knobs.device_id;
    regs_[0][0x07] = knobs.reset_detect_init ? 0x0100U : 0U;
    bank_ = 1U;
    read_pending_ = false;
    standby_ = false;
    cal_running_ = false;
    cal_armed_ = false;
    cal_mode_applied_ = 0U;
    data_ready_ = false;
    reset_at_us_ = now_us;
    flags_hi_ = {0, 0};
    flags_lo_ = {0, 0};
    ++resets;
  }

  [[nodiscard]] bool documented(uint8_t bank, uint8_t a) const {
    if (bank == 1U) {
      return ((a >= 1U) && (a <= 2U)) || (a == 7U) || (a == 8U) || (a == 9U) ||
             ((a >= 0x0AU) && (a <= 0x12U)) || (a == 0x14U) || (a == 0x15U) || (a == 0x1DU) ||
             (a == 0x21U);
    }
    return ((a >= 8U) && (a <= 0x36U)) || (a == 0x37U) || (a == 0x3EU);
  }

  [[nodiscard]] bool readOnly(uint8_t bank, uint8_t a) const {
    return (bank == 1U) ? ((a == 0x1DU) || (a == 0x21U)) : (a == 0x3EU);
  }

  [[nodiscard]] uint16_t readValue(uint8_t bank, uint8_t a) const {
    const uint16_t stored = regs_[bankIndex(bank)][a];
    if ((bank == 1U) && (a == 0x1DU)) {
      return cal_running_ ? 0x0008U : 0U;
    }
    if ((bank != 1U) && (a == 0x3EU)) {
      const std::size_t g = (bank == 2U) ? 0U : 1U;
      return static_cast<uint16_t>((static_cast<uint32_t>(flags_hi_[g]) << 8U) | flags_lo_[g]);
    }
    return stored;
  }

  void registerFrame(const FrameLog& f, std::vector<uint8_t>& reply) {
    if (!accessible()) {
      read_pending_ = false;
      return; // too early after a reset: the chip does not answer
    }
    if (read_pending_) {
      // Table 7-17: SDOUT[23:8] = register data, SDOUT[7:0] = 0x00 (Figure 5-1: DO15 first).
      const std::size_t hi = knobs.reply_shifted_by_8 ? 1U : 0U;
      reply[hi] = static_cast<uint8_t>(read_value_ >> 8U);
      reply[hi + 1U] = static_cast<uint8_t>(read_value_ & 0xFFU);
      if (!knobs.reply_shifted_by_8) {
        reply[2] = knobs.reply_low_byte;
      }
    } else if (!knobs.sdout_after_write_zero) {
      reply = last_reply_;
      reply.resize(3U, 0U);
    }
    if ((knobs.flip_reply_bit >= 0) && (!knobs.flip_in_channel_banks_only || (read_bank_ != 1U))) {
      const std::size_t byte = 1U - (static_cast<std::size_t>(knobs.flip_reply_bit) / 8U);
      reply[byte] = static_cast<uint8_t>(
          reply[byte] ^ (1U << (static_cast<unsigned>(knobs.flip_reply_bit) % 8U)));
    }
    last_reply_ = reply;
    const bool was_read = read_pending_;
    read_pending_ = false;
    const bool standby_ok = (f.addr == 0x02U) || ((f.addr == 0x08U) && (f.bank == 1U));
    if (standby_ && !standby_ok) {
      // In standby only BANK_SEL and PDN_CTL are written.
      if (f.addr != 0x01U) {
        return;
      }
    }
    apply(f, was_read);
  }

  void apply(const FrameLog& f, bool /*was_read*/) {
    const uint8_t a = f.addr;
    const uint16_t d = f.data;
    if ((a == 0x00U) && (d == 0U)) {
      return; // the NOP frame that ends a read sequence (Table 7-17)
    }
    if (a == 0x02U) {
      if ((d == 1U) || (d == 2U) || (d == 4U)) {
        bank_ = static_cast<uint8_t>(d);
        regs_[0][0x02] = d;
      }
      return;
    }
    if (a == 0x01U) {
      if (!knobs.rdcmd_any_bank && (bank_ != 1U)) {
        return;
      }
      if ((d & 0x0002U) != 0U) {
        hardReset();
        return;
      }
      if ((d & 0x0001U) != 0U) {
        const auto target = static_cast<uint8_t>(d >> 8U);
        read_value_ = documented(bank_, target) ? readValue(bank_, target) : 0U;
        read_pending_ = true;
        read_bank_ = bank_;
      }
      return;
    }
    if (!documented(bank_, a) || readOnly(bank_, a)) {
      bad_writes.push_back(static_cast<uint16_t>((static_cast<uint32_t>(bank_) << 8U) | a));
      return;
    }
    store(a, d);
  }

  void store(uint8_t a, uint16_t d) {
    std::uint16_t& r = regs_[bankIndex(bank_)][a];
    const uint16_t old = r;
    r = d;
    if (bank_ == 1U) {
      if (a == 0x08U) {
        standby_ = (d & 1U) != 0U;
        if (!standby_ && (old & 1U) != 0U && !knobs.standby_retains) {
          hardReset();
        }
      } else if (a == 0x14U) {
        if (((d & 1U) != 0U) && ((old & 1U) == 0U)) {
          ++sysref_edges;
          block_count_ = 0U;
        }
      } else if (a == 0x12U) {
        adcCal(old, d);
      } else if (a == 0x07U) {
        // The user clears RESET_DETECT_FLAG by writing 0.
      } else if (a == 0x11U) {
        // The reference switch needs time to settle: the next access is not
        // delayed here.
      }
    } else if (a == 0x21U) {
      const std::size_t g = (bank_ == 2U) ? 0U : 1U;
      if (((d & 0x8000U) != 0U) && ((old & 0x8000U) == 0U)) {
        flags_hi_[g] = 0U;
        flags_lo_[g] = 0U;
      }
    }
  }

  void adcCal(uint16_t old, uint16_t d) {
    const bool rise = ((d & 0x0100U) != 0U) && ((old & 0x0100U) == 0U);
    const bool fall = ((d & 0x0100U) == 0U) && ((old & 0x0100U) != 0U);
    if (rise && ((d & 3U) != 0U)) {
      cal_armed_ = true;
      cal_arm_time_ = now_us;
    } else if (fall) {
      cal_trig_released_ = cal_armed_;
      if (cal_armed_ && ((now_us - cal_arm_time_) < 10U)) {
        cal_armed_ = false; // TRIG must stay high at least 10 us
        cal_trig_released_ = false;
      }
    }
    cal_mode_ = static_cast<uint16_t>(d & 3U);
    if (cal_mode_ == 0U) {
      cal_mode_applied_ = 0U;
    }
  }

  void onConvstFall() {
    if (cal_armed_ && cal_trig_released_ && !cal_running_) {
      cal_running_ = true;
      cal_armed_ = false;
      cal_trig_released_ = false;
      cal_done_us_ = now_us + knobs.cal_busy_us;
      return;
    }
    ++conversions;
    if (cal_running_) {
      return;
    }
    const uint32_t pulses = pulsesPerResult();
    if (pulses > 1U) {
      ++block_count_;
      if (block_count_ < pulses) {
        return;
      }
      block_count_ = 0U;
    }
    convert();
    data_ready_ = true;
    convst_high_before_ready_ = false;
    ready_at_us_ = now_us + convTimeUs();
    ready_expires_us_ =
        knobs.data_valid_us == 0U ? UINT64_MAX : (ready_at_us_ + knobs.data_valid_us);
  }

  [[nodiscard]] uint32_t blockOsr() const {
    const bool gate = !knobs.avg_gate_required || ((Common(0x15) & 3U) == 3U);
    const uint32_t code = (Common(0x14) >> 4U) & 0xFU;
    static constexpr std::array<uint32_t, 12> kOsr{1, 2, 4, 6, 8, 10, 12, 16, 20, 32, 64, 128};
    return (gate && (code < 12U)) ? kOsr[code] : 1U;
  }

  /// FIR decimation from FIR_FILT_SEL (Table 7-9); 1 when no FIR is selected.
  [[nodiscard]] uint32_t firDecimation() const {
    switch ((Common(0x14) >> 12U) & 7U) {
    case 1:
    case 2:
    case 4:
      return 2U;
    case 3:
    case 5:
      return 4U;
    case 7:
      return 8U;
    default:
      return 1U;
    }
  }

  [[nodiscard]] bool internalOversampling() const {
    return (Common(0x14) & 2U) != 0U;
  }

  /// CONVST falling edges that make one result: N for an external block average, the decimation
  /// for a FIR, one for the internal oversampling clock.
  [[nodiscard]] uint32_t pulsesPerResult() const {
    const uint32_t fir = firDecimation();
    if (fir > 1U) {
      return fir;
    }
    return internalOversampling() ? 1U : blockOsr();
  }

  /// Section 5.7: tCONV = (N - 1) * tCYC + 1.25 us for the internal oversampling clock; the
  /// external clock modes deliver the result 1.25 us after the last falling edge. At least 2 us.
  [[nodiscard]] uint32_t convTimeUs() const {
    const uint32_t osr = blockOsr();
    uint64_t ns = 1250U;
    if ((osr > 1U) && internalOversampling()) {
      ns += static_cast<uint64_t>(osr - 1U) * (1'000'000'000ULL / knobs.convst_hz);
    }
    const auto us = static_cast<uint32_t>((ns + 999U) / 1000U);
    return (us < 2U) ? 2U : us;
  }

  static int32_t rangeUv(uint32_t code) {
    switch (code) {
    case 0:
      return 5'000'000;
    case 2:
      return 2'500'000;
    case 3:
      return 6'250'000;
    case 4:
      return 10'000'000;
    default:
      return 12'500'000;
    }
  }

  static int32_t sext(uint32_t v, unsigned bits) {
    const uint32_t m = 1U << (bits - 1U);
    return static_cast<int32_t>((v ^ m) - m);
  }

  int32_t channelCode(int ch) {
    const uint8_t bank = ChannelBank(ch);
    const uint16_t pga = readHalf(bank, ChannelAddr(ch, 0x08U, 2), LocalIndex(ch));
    const int64_t full = rangeUv(pga & 7U);
    int64_t code = (static_cast<int64_t>(input_uv[static_cast<std::size_t>(ch)]) * 32768 +
                    (input_uv[static_cast<std::size_t>(ch)] >= 0 ? full / 2 : -full / 2)) /
                   full;
    const bool offset_cal = (cal_mode_applied_ & 1U) != 0U;
    code += offset_cal ? 0 : analog_offset_lsb;
    const int32_t ofs = sext(regs_[bankIndex(bank)][ChannelAddr(ch, 0x11U, 1)] & 0x3FFU, 10);
    const uint16_t corr = regs_[bankIndex(bank)][0x37];
    if ((corr & 2U) == 0U) {
      code += knobs.offset_subtracted ? -(ofs / 4) : (ofs / 4);
    }
    const int32_t gan = sext(regs_[bankIndex(bank)][ChannelAddr(ch, 0x19U, 1)] & 0x3FFFU, 14);
    if ((corr & 1U) == 0U) {
      code = code + ((code * gan) / 65536);
    }
    code = std::min<int64_t>(code, 32767);
    code = std::max<int64_t>(code, -32768);
    return static_cast<int32_t>(code);
  }

  [[nodiscard]] uint16_t readHalf(uint8_t bank, uint8_t addr, int local) const {
    const uint32_t v = regs_[bankIndex(bank)][addr];
    return static_cast<uint16_t>(((static_cast<uint32_t>(local) & 1U) != 0U) ? (v >> 8U)
                                                                             : (v & 0xFFU));
  }

  [[nodiscard]] bool patternOn(uint8_t bank, uint16_t* value, int ch) const {
    const uint32_t cfg = regs_[bankIndex(bank)][0x2E];
    if ((cfg & 1U) == 0U) {
      return false;
    }
    const uint32_t mode = (cfg >> 4U) & 7U;
    const bool known = (mode == 0U) || (mode == 2U) || ((mode == 1U) && knobs.tp_mode1_accepted);
    if (!known) {
      return false;
    }
    uint32_t v = regs_[bankIndex(bank)][ChannelAddr(ch, 0x2FU, 1)];
    if (mode == 2U) {
      v = (regs_[bankIndex(bank)][0x2F] + 1U) * (conversions & 0xFFFFU);
    }
    if ((cfg & 4U) == 0U) {
      const auto idx = static_cast<uint32_t>(LocalIndex(ch));
      v = knobs.tp_index_or ? (v | (idx << 12U)) : (v + (idx << 12U));
    }
    *value = static_cast<uint16_t>(v);
    return true;
  }

  void convert() {
    const uint32_t num = (Common(0x11) >> 2U) & 3U;
    const uint32_t sel = (Common(0x11) >> 4U) & 7U;
    const std::vector<int> slots = SlotChannels(num, sel);
    const bool bits24 = ((Common(0x0A) >> 2U) & 3U) == 2U;
    const bool offset_bin = (Common(0x0A) & 0x1000U) != 0U;
    const bool diag_en = (Common(0x0A) & 0x0100U) != 0U;
    const bool xor_on = (Common(0x0A) & 0x0200U) != 0U && (!bits24 || knobs.xor_with_24bit);
    prbs_ = static_cast<uint8_t>((prbs_ * 29U + 7U) & 0xFFU);
    const uint32_t prbs = prbs_;
    const uint32_t mask = xor_on ? ((prbs << 8U) + ((prbs & 1U) * 255U)) : 0U;
    frame_.clear();
    bool any_alert = false;
    for (const int ch : slots) {
      int32_t code = channelCode(ch);
      uint16_t pattern = 0;
      if (patternOn(ChannelBank(ch), &pattern, ch)) {
        code = sext(pattern, 16);
      }
      any_alert = dwc(ch, code) || any_alert;
      uint32_t wire = static_cast<uint32_t>(code) & 0xFFFFU;
      wire = offset_bin ? (wire ^ 0x8000U) : wire;
      sent_raw[static_cast<std::size_t>(ch)] = static_cast<uint16_t>(wire);
      wire ^= mask;
      frame_.push_back(static_cast<uint8_t>(wire >> 8U));
      frame_.push_back(static_cast<uint8_t>(wire & 0xFFU));
      if (bits24) {
        frame_.push_back(knobs.low_byte_extra ? 0x5AU : 0U);
      }
    }
    appendTrailer(diag_en || xor_on, xor_on, any_alert);
  }

  void appendTrailer(bool diag_en, bool xor_on, bool any_alert) {
    auto diag = static_cast<uint8_t>(
        (any_alert || anyFlag() ? 0x40U : 0U) |
        (((Common(0x07) & 0x0100U) != 0U) || knobs.reset_during_run ? 1U : 0U));
    knobs.reset_during_run = false;
    if (diag_en && xor_on && !knobs.diag_before_prbs) {
      frame_.push_back(prbs_);
      frame_.push_back(diag);
    } else {
      if (diag_en) {
        frame_.push_back(diag);
      }
      if (xor_on) {
        frame_.push_back(prbs_);
      }
    }
  }

  [[nodiscard]] bool anyFlag() const {
    const uint32_t all = static_cast<uint32_t>(flags_hi_[0]) | static_cast<uint32_t>(flags_hi_[1]) |
                         static_cast<uint32_t>(flags_lo_[0]) | static_cast<uint32_t>(flags_lo_[1]);
    return all != 0U;
  }

  /// The window comparator of one channel; the thresholds compare the top byte
  /// (signed).
  bool dwc(int ch, int32_t code) {
    const uint8_t bank = ChannelBank(ch);
    const int local = LocalIndex(ch);
    const std::size_t g = (bank == 2U) ? 0U : 1U;
    if ((regs_[bankIndex(bank)][0x21] & (1U << static_cast<unsigned>(local))) == 0U) {
      return false;
    }
    const uint32_t th = regs_[bankIndex(bank)][0x22U + static_cast<std::size_t>(local)];
    const int32_t high = sext(th >> 8U, 8);
    const int32_t low = sext(th & 0xFFU, 8);
    const int32_t top = sext((static_cast<uint32_t>(code) >> 8U) & 0xFFU, 8);
    const bool h = top > high;
    const bool l = top < low;
    const uint32_t glitch = (static_cast<uint32_t>(regs_[bankIndex(bank)][0x21]) >> 8U) & 0xFU;
    uint32_t& count = glitch_count_[g][static_cast<std::size_t>(local)];
    count = (h || l) ? (count + 1U) : 0U;
    if (count > glitch) {
      flags_hi_[g] =
          static_cast<uint8_t>(flags_hi_[g] | (h ? (1U << static_cast<uint32_t>(local)) : 0U));
      flags_lo_[g] =
          static_cast<uint8_t>(flags_lo_[g] | (l ? (1U << static_cast<uint32_t>(local)) : 0U));
    }
    return count > glitch;
  }

  void readout(std::vector<uint8_t>& reply) {
    const bool lost = internalOversampling() && (blockOsr() > 1U) && !convst_high_before_ready_;
    if (!data_ready_ || (now_us < ready_at_us_) || (now_us > ready_expires_us_) || standby_ ||
        lost) {
      if (knobs.stale_when_not_ready) {
        for (std::size_t i = 0; (i < reply.size()) && (i < frame_.size()); ++i) {
          reply[i] = frame_[i];
        }
      }
      return; // no result: the lines idle at zero
    }
    for (std::size_t i = 0; (i < reply.size()) && (i < frame_.size()); ++i) {
      reply[i] = frame_[i];
    }
    data_ready_ = false;
  }

  [[nodiscard]] bool alarmAsserted(uint32_t sel, uint32_t mask) const {
    const bool drdy = data_ready_ && (now_us >= ready_at_us_);
    const bool win = anyFlag();
    const bool cal_done = !cal_running_ && (cal_mode_applied_ != 0U);
    switch (sel) {
    case 0:
      return drdy;
    case 1:
      return win;
    case 6:
      return cal_done;
    case 8: {
      const bool sources[8] = {drdy, win, false, false, false, false, cal_done, false};
      bool any = false;
      for (unsigned i = 0; i < 8U; ++i) {
        const bool unmasked =
            knobs.alrm_mask_one_masks ? (((mask >> i) & 1U) == 0U) : (((mask >> i) & 1U) != 0U);
        any = any || (sources[i] && unmasked);
      }
      return any;
    }
    default:
      return false;
    }
  }

  std::array<std::array<uint16_t, 64>, 3> regs_{};
  uint8_t bank_{1};
  bool read_pending_{false};
  uint8_t read_bank_{1};
  uint16_t read_value_{0};
  std::vector<uint8_t> last_reply_{0, 0, 0};
  bool standby_{false};
  bool reset_pin_{false};
  uint64_t reset_since_us_{0};
  uint64_t reset_at_us_{0};
  bool cal_armed_{false};
  bool cal_trig_released_{false};
  uint64_t cal_arm_time_{0};
  bool cal_running_{false};
  uint64_t cal_done_us_{0};
  uint16_t cal_mode_{0};
  uint16_t cal_mode_applied_{0};
  uint32_t block_count_{0};
  bool convst_high_before_ready_{false};
  bool data_ready_{false};
  uint64_t ready_at_us_{0};
  uint64_t ready_expires_us_{UINT64_MAX};
  std::vector<uint8_t> frame_;
  uint8_t prbs_{0x3B};
  std::array<uint8_t, 2> flags_hi_{};
  std::array<uint8_t, 2> flags_lo_{};
  std::array<std::array<uint32_t, 8>, 2> glitch_count_{};
};

/// Shared state of the mock ports: the chip and the call counters.
struct MockBusState {
  Ads9324Model chip;
  bool bus_up_ok = true;
  int bus_up_calls = 0;
  int gpio_fail = -1; ///< Fail the GpioSet call with this index; -1 never.
  int gpio_calls = 0;
  std::vector<std::pair<uint8_t, uint8_t>> gpio_log; ///< (pin, signal).
  std::vector<const char*> logs;
  uint64_t delays_us = 0;
};

/// The required hooks and `DelayUs()`; derive to add optional hooks.
template <typename Self>
class MockPortBase : public ads9324::SpiInterface<Self> {
public:
  MockBusState state;

  bool EnsureInitialized() noexcept {
    ++state.bus_up_calls;
    return state.bus_up_ok;
  }
  bool Transfer(const uint8_t* tx, uint8_t* rx, std::size_t len) noexcept {
    return state.chip.Transfer(tx, rx, len);
  }
};

/// The required hooks plus `DelayUs()`.
class MinimalPort : public MockPortBase<MinimalPort> {
public:
  void DelayUs(uint32_t us) noexcept {
    state.delays_us += us;
    state.chip.Delay(us);
  }
};

/// Every optional hook: GpioSet (RESET, CONVST), GpioGet (DRDY/ALARM),
/// CaptureLanes and Log.
class FullPort : public MockPortBase<FullPort> {
public:
  void DelayUs(uint32_t us) noexcept {
    state.delays_us += us;
    state.chip.Delay(us);
  }
  bool GpioSet(ads9324::CtrlPin pin, ads9324::GpioSignal level) noexcept {
    const int index = state.gpio_calls++;
    if (index == state.gpio_fail) {
      return false;
    }
    state.gpio_log.emplace_back(static_cast<uint8_t>(pin), static_cast<uint8_t>(level));
    const bool active = level == ads9324::GpioSignal::ACTIVE;
    if (pin == ads9324::CtrlPin::Reset) {
      state.chip.SetResetPin(active);
    } else if (pin == ads9324::CtrlPin::Convst) {
      state.chip.SetConvst(active);
    }
    return true;
  }
  bool GpioGet(ads9324::CtrlPin pin, bool& high) noexcept {
    if (pin != ads9324::CtrlPin::DrdyAlarm) {
      return false;
    }
    high = state.chip.AlarmPin();
    return true;
  }
  bool CaptureLanes(uint8_t lanes, uint8_t* rx, std::size_t len) noexcept {
    return state.chip.CaptureLanes(lanes, rx, len);
  }
  void Log(ads9324::LogLevel /*level*/, const char* message) noexcept {
    state.logs.push_back(message);
  }
};

} // namespace ads9324_test

#endif // ADS9324_MOCK_ADS9324_HPP
