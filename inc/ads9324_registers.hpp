/**
 * @file ads9324_registers.hpp
 * @brief ADS9324 register map: banks, addresses, bit fields, reset values and
 * the channel layout (datasheet section 8).
 * @copyright Copyright (c) 2026 The driver-ads9324 authors.
 * SPDX-License-Identifier: MIT
 *
 * @details
 * The chip has 116 documented 16-bit registers in three banks selected through
 * BANK_SEL: the common bank (18 registers) and two channel banks of 49
 * registers each, one for AIN1 to AIN8 and one for AIN9 to AIN16. Every
 * register is addressed with 8 bits and moves as the 16-bit data of a 24-bit
 * frame (section 7.5).
 *
 * **The AIN9 to AIN16 bank is laid out in reverse order** (Table 8-72,
 * section 8.3): its first channel register is AIN16 and its last is AIN9. Both
 * channel banks follow one rule once a channel is expressed as a bank and a
 * bank-local index, `Locate()`: local index `ch` in the AIN1 to AIN8 bank,
 * local index `15 - ch` in the AIN9 to AIN16 bank, `ch` counted from 0 for
 * AIN1. The even local index is the low half of a register that holds two
 * channels, the odd one the high half.
 */
#ifndef ADS9324_REGISTERS_HPP
#define ADS9324_REGISTERS_HPP

#include <array>
#include <cstddef>
#include <cstdint>

#include "ads9324_detail.hpp"

/** @brief The ADS9324 register map. */
namespace ads9324::reg {

using detail::Field;
using detail::operator""_u16;
using detail::operator""_u8;

/**
 * @defgroup ads9324_registers Register map
 * @brief Banks, addresses, fields and reset values.
 * @{
 */

/** @brief A register bank. The value is the index of the bank in the driver's
 * tables. */
enum class Bank : uint8_t {
  Common = 0,   ///< The common registers, BANK_SEL code 001b.
  Ain1To8 = 1,  ///< Channel registers AIN1 to AIN8, BANK_SEL code 010b.
  Ain9To16 = 2, ///< Channel registers AIN9 to AIN16 (reversed order), BANK_SEL
                ///< code 100b.
};

/** @brief Number of register banks. */
inline constexpr uint8_t kBankCount = 3;
/** @brief Number of channels. */
inline constexpr uint8_t kChannelCount = 16;
/** @brief Channels per channel bank. */
inline constexpr uint8_t kChannelsPerBank = 8;
/** @brief Addresses per bank table: the highest documented address is 0x3E. */
inline constexpr uint8_t kAddressSpace = 64;

/**
 * @brief The BANK_SEL code of a bank (section 8.1.2).
 * @param bank The bank.
 * @return 1, 2 or 4.
 */
constexpr uint16_t BankSelCode(Bank bank) noexcept {
  switch (bank) {
  case Bank::Common:
    return 1_u16;
  case Bank::Ain1To8:
    return 2_u16;
  case Bank::Ain9To16:
    return 4_u16;
  default:
    break;
  }
  return 1_u16;
}

/// @name Common bank addresses (section 8.1)
/// @{
inline constexpr uint8_t kGenCfg1 = 0x01;      ///< GEN_CFG1: read command, software reset.
inline constexpr uint8_t kBankSel = 0x02;      ///< BANK_SEL: always accessible.
inline constexpr uint8_t kDiagCtrl = 0x07;     ///< DIAG_CTRL: RESET_DETECT_FLAG.
inline constexpr uint8_t kPdnCtl = 0x08;       ///< PDN_CTL: standby.
inline constexpr uint8_t kGenCfg2 = 0x09;      ///< GEN_CFG2: daisy chain (never written).
inline constexpr uint8_t kGenCfg3 = 0x0A;      ///< GEN_CFG3: data format and lanes.
inline constexpr uint8_t kXorBitsCtl = 0x0B;   ///< XOR_BITS_CTL: output randomizer.
inline constexpr uint8_t kDrdyAlarmSel = 0x0C; ///< DRDY_ALARM_SEL: pin function.
inline constexpr uint8_t kGenCfg4 = 0x0D;      ///< GEN_CFG4: drive strength, delay enable.
inline constexpr uint8_t kDigDelayCfg1 = 0x0E; ///< DIG_DELAY_CFG1: SDOUT and D3 to D0 delay.
inline constexpr uint8_t kDigDelayCfg2 = 0x0F; ///< DIG_DELAY_CFG2: D7 to D4 delay.
inline constexpr uint8_t kAnaCfg1 = 0x10;      ///< ANA_CFG1: reference source.
inline constexpr uint8_t kAnaCfg2 = 0x11;      ///< ANA_CFG2: channel mode, low-speed crosstalk.
inline constexpr uint8_t kAdcCal = 0x12;       ///< ADC_CAL: ADC calibration module.
inline constexpr uint8_t kDigFilter = 0x14;    ///< DIG_FILTER: digital filters and SYSREF.
inline constexpr uint8_t kGenCfg5 = 0x15;      ///< GEN_CFG5: AVG_MODE gate.
inline constexpr uint8_t kDeviceStatus = 0x1D; ///< DEVICE_STATUS (R): CALIB_BUSY_FLAG.
inline constexpr uint8_t kDeviceId = 0x21;     ///< DEVICE_ID (R): 0x0004.
/** @} */

/// @name Channel bank addresses (sections 8.2 and 8.3)
/// @{
inline constexpr uint8_t kPgaConfig = 0x08;  ///< PGA_CONFIG, 4 registers, two channels each.
inline constexpr uint8_t kPgaBwSel = 0x0C;   ///< PGA_BW_SEL, 2 bits per channel.
inline constexpr uint8_t kPhaseDelay = 0x0D; ///< PHASE_DELAY, 4 registers, 8 bits per channel.
inline constexpr uint8_t kOffset = 0x11;     ///< OFS, 8 registers, 10 bits.
inline constexpr uint8_t kGain = 0x19;       ///< GAN, 8 registers, 14 bits.
inline constexpr uint8_t kDwcCfg = 0x21;     ///< DWC_CFG: enables, glitch filter, flag reset.
inline constexpr uint8_t kDwcTh = 0x22;      ///< DWC_TH, 8 registers: high byte, low byte.
inline constexpr uint8_t kDwcHys = 0x2A;     ///< DWC_HYS, 4 registers, 8 bits per channel.
inline constexpr uint8_t kTpCfg = 0x2E;      ///< TP_CFG: test pattern control.
inline constexpr uint8_t kTp = 0x2F;         ///< TP_AIN, 8 registers, 16 bits.
inline constexpr uint8_t kCorrection = 0x37; ///< GEN_CFG5 of a channel bank: correction bypass.
inline constexpr uint8_t kDwcFlag = 0x3E;    ///< DWC_FLAG (R): latched high and low flags.
/** @} */

/// @name GEN_CFG1 (section 8.1.1)
/// @{
using GenCfg1ReadAddress = Field<uint16_t, 8U, 8U>; ///< REG_RD_ADD[7:0].
inline constexpr uint16_t kSwRst = 0x0002;          ///< SW_RST.
inline constexpr uint16_t kRegRdEn = 0x0001;        ///< REG_RD_EN.
/** @} */

inline constexpr uint16_t kResetDetectFlag = 0x0100; ///< DIAG_CTRL.RESET_DETECT_FLAG (8.1.3).
inline constexpr uint16_t kDevicePdn = 0x0001;       ///< PDN_CTL.DEVICE_PDN (8.1.4).

/// @name GEN_CFG2 daisy chain fields (section 8.1.5), defined for the seam,
/// never written
/// @{
using DaisyLocation = Field<uint16_t, 8U, 4U>;   ///< DAISY_CHN_LOC[3:0].
using DaisyDevices = Field<uint16_t, 4U, 4U>;    ///< DAISY_CHN_NUM_DEV[3:0].
inline constexpr uint16_t kDaisyEnable = 0x0001; ///< DAISY_CHN_EN.
/** @} */

/// @name GEN_CFG3 (section 8.1.6)
/// @{
inline constexpr uint16_t kEnOfsBinary = 0x1000;    ///< EN_OFS_BINARY: 1 = offset binary.
inline constexpr uint16_t kEnXorPatt = 0x0200;      ///< EN_XOR_PATT: randomizer.
inline constexpr uint16_t kEnDiagFlag = 0x0100;     ///< EN_DIAG_FLAG: DIAG byte after the data.
using DoutLaneSel = Field<uint16_t, 4U, 2U>;        ///< DOUT_LANE_SEL[1:0].
using DoutLength = Field<uint16_t, 2U, 2U>;         ///< DOUT_LENGTH[1:0].
inline constexpr uint16_t kAdcDataSdoutEn = 0x0002; ///< ADC_DATA_SDOUT_EN.
/** @} */

/// XOR_BITS_CTL for the randomizer: XOR_MODE 11b, NUM_XOR_BITS 11b, XOR_BIT_SEL
/// 1 (section 7.3.14.4).
inline constexpr uint16_t kXorBitsRandomizer = 0x00F1;

/// @name DRDY_ALARM_SEL (section 8.1.8)
/// @{
using AlarmMask = Field<uint16_t, 8U, 8U>;    ///< ALRM_MASK[7:0].
inline constexpr uint16_t kAlrmType = 0x0020; ///< ALRM_TYPE: 1 = pulse (window alert only).
inline constexpr uint16_t kAlrmPol = 0x0010;  ///< ALRM_POL: 1 = active low.
using DrdyAlrmSel = Field<uint16_t, 0U, 4U>;  ///< DRDY_ALRM_SEL[3:0].
/** @} */

/// @name GEN_CFG4 (section 8.1.9)
/// @{
inline constexpr uint16_t kAlrmDis = 0x0020;        ///< ALRM_DIS: 1 = alarm function disabled.
inline constexpr uint16_t kDigDelayEn = 0x0008;     ///< DIG_DELAY_EN.
using DriveStrengthField = Field<uint16_t, 0U, 2U>; ///< DRIVE_STRENGTH[1:0].
/** @} */

/// @name DIG_DELAY_CFG1 and DIG_DELAY_CFG2 (sections 8.1.10, 8.1.11), 3 bits
/// per output
/// @{
using DelaySdout = Field<uint16_t, 12U, 3U>; ///< DIG_DELAY_SDOUT, in CFG1.
using DelayD3 = Field<uint16_t, 9U, 3U>;     ///< DIG_DELAY_D3, in CFG1.
using DelayD2 = Field<uint16_t, 6U, 3U>;     ///< DIG_DELAY_D2, in CFG1.
using DelayD1 = Field<uint16_t, 3U, 3U>;     ///< DIG_DELAY_D1, in CFG1.
using DelayD0 = Field<uint16_t, 0U, 3U>;     ///< DIG_DELAY_D0, in CFG1.
using DelayD7 = Field<uint16_t, 9U, 3U>;     ///< DIG_DELAY_D7, in CFG2.
using DelayD6 = Field<uint16_t, 6U, 3U>;     ///< DIG_DELAY_D6, in CFG2.
using DelayD5 = Field<uint16_t, 3U, 3U>;     ///< DIG_DELAY_D5, in CFG2.
using DelayD4 = Field<uint16_t, 0U, 3U>;     ///< DIG_DELAY_D4, in CFG2.
/** @} */

/// @name ANA_CFG1 and ANA_CFG2 (sections 8.1.12, 8.1.13)
/// @{
inline constexpr uint16_t kRefselCtrlDis = 0x0002; ///< REFSEL_CTRL_DIS: ignore the REFSEL pin.
inline constexpr uint16_t kExtRefEn = 0x0001;      ///< EXT_REF_EN: external reference.
inline constexpr uint16_t kXtalkLowSpeed = 0x8000; ///< CH_XTALK_LOW_SPEED.
using AdcChSel = Field<uint16_t, 4U, 3U>;          ///< ADC_CH_SEL[2:0].
using AdcNumSel = Field<uint16_t, 2U, 2U>;         ///< ADC_NUM_SEL[1:0].
/** @} */

/// @name ADC_CAL (section 8.1.14)
/// @{
inline constexpr uint16_t kSeDiffAin1To8 = 0x2000;  ///< SE_DIFF_MODE_AIN1_8: 1 = differential.
inline constexpr uint16_t kSeDiffAin9To16 = 0x1000; ///< SE_DIFF_MODE_AIN9_16: 1 = differential.
inline constexpr uint16_t kAdcCalTrig = 0x0100;     ///< ADC_CAL_TRIG: written 1, then 0.
using AdcCalMode = Field<uint16_t, 0U, 2U>;         ///< ADC_CAL_MODE[1:0].
/** @} */

/// @name DIG_FILTER and GEN_CFG5 (sections 8.1.15, 8.1.16)
/// @{
inline constexpr uint16_t kPhaseDelayEn = 0x8000;  ///< PHASE_DELAY_EN.
using FirFiltSel = Field<uint16_t, 12U, 3U>;       ///< FIR_FILT_SEL[2:0].
using MvgAvgLen = Field<uint16_t, 8U, 4U>;         ///< MVG_AVG_LEN[3:0].
using BlkAvgOsr = Field<uint16_t, 4U, 4U>;         ///< BLK_AVG_OSR[3:0].
inline constexpr uint16_t kIntTrigMode = 0x0002;   ///< INT_TRIG_MODE: internal oversampling clock.
inline constexpr uint16_t kDigFiltSysref = 0x0001; ///< DIG_FILT_SYSREF: written 1, then 0.
inline constexpr uint16_t kAvgModeOvrEn = 0x0002;  ///< AVG_MODE_OVR_EN.
inline constexpr uint16_t kAvgMode = 0x0001;       ///< AVG_MODE.
/** @} */

inline constexpr uint16_t kCalibBusyFlag = 0x0008; ///< DEVICE_STATUS.CALIB_BUSY_FLAG (8.1.17).
inline constexpr uint16_t kDeviceIdValue = 0x0004; ///< DEVICE_ID after reset, read only (8.1.18).

/// @name Channel register fields, bank-local layout (sections 8.2, 8.3)
/// @{
using CmeCorrEn = Field<uint8_t, 7U, 1U>;       ///< CME_CORR_EN_AINn, in one byte of PGA_CONFIG.
using CmRange = Field<uint8_t, 4U, 3U>;         ///< CM_RANGE_AINn, in one byte of PGA_CONFIG.
using InputRangeField = Field<uint8_t, 0U, 3U>; ///< INPUT_RANGE_AINn, in one byte of PGA_CONFIG.
using PgaBw = Field<uint16_t, 0U, 2U>;          ///< PGA_BW_SEL_AINn, 2 bits at `2 * local`.
using OffsetField = Field<uint16_t, 0U, 10U>;   ///< OFS_AINn[9:0], two's complement.
using GainField = Field<uint16_t, 0U, 14U>;     ///< GAN_AINn[13:0], two's complement.
inline constexpr uint16_t kDwcStatRst = 0x8000; ///< DWC_STAT_RST: written 1, then 0.
using DwcGlitchFilt = Field<uint16_t, 8U, 4U>;  ///< DWC_GLITCH_FILT[3:0].
using DwcHighTh = Field<uint16_t, 8U, 8U>;      ///< HIGH_TH_AINn[7:0].
using DwcLowTh = Field<uint16_t, 0U, 8U>;       ///< LOW_TH_AINn[7:0].
using TpMode = Field<uint16_t, 4U, 3U>;         ///< TP_MODE[2:0].
inline constexpr uint16_t kTpDisIdx = 0x0004;   ///< TP_DIS_IDX: 1 = no channel index insertion.
inline constexpr uint16_t kTpUpdMode = 0x0002;  ///< TP_UPD_MODE: 1 = ramp steps at every CONVST.
inline constexpr uint16_t kTpEn = 0x0001;       ///< TP_EN.
inline constexpr uint16_t kOfsCorrDis = 0x0002; ///< OFS_CORR_DIS.
inline constexpr uint16_t kGanCorrDis = 0x0001; ///< GAN_CORR_DIS.
/** @} */

/** @brief Where a channel's registers are: the bank and the bank-local index.
 */
struct ChannelLocation {
  Bank bank{Bank::Ain1To8}; ///< The channel bank.
  uint8_t local{0};         ///< The bank-local index, 0 to 7.
};

/**
 * @brief The bank and local index of a channel (the AIN9 to AIN16 bank is
 * reversed).
 * @param channel The channel counted from 0 (AIN1 is 0, AIN16 is 15); 0 to 15.
 * @return `{Ain1To8, channel}` for 0 to 7, `{Ain9To16, 15 - channel}` for 8
 * to 15.
 */
constexpr ChannelLocation Locate(uint8_t channel) noexcept {
  if (channel < kChannelsPerBank) {
    return {.bank = Bank::Ain1To8, .local = channel};
  }
  return {.bank = Bank::Ain9To16, .local = static_cast<uint8_t>(kChannelCount - 1U - channel)};
}

/**
 * @brief The channel at a bank-local index; the inverse of `Locate()`.
 * @param bank A channel bank.
 * @param local The bank-local index, 0 to 7.
 * @return The channel counted from 0.
 */
constexpr uint8_t ChannelAt(Bank bank, uint8_t local) noexcept {
  if (bank == Bank::Ain9To16) {
    return static_cast<uint8_t>(kChannelCount - 1U - local);
  }
  return local;
}

/**
 * @brief The address of the register that holds the PGA_CONFIG of a local
 * index.
 * @param local The bank-local channel index, 0 to 7.
 * @return The register address in the bank.
 */
constexpr uint8_t PgaConfigAddress(uint8_t local) noexcept {
  return static_cast<uint8_t>(kPgaConfig + (local >> 1U));
}
/**
 * @brief The address of the PHASE_DELAY register of a local index.
 * @param local The bank-local channel index, 0 to 7.
 * @return The register address in the bank.
 */
constexpr uint8_t PhaseDelayAddress(uint8_t local) noexcept {
  return static_cast<uint8_t>(kPhaseDelay + (local >> 1U));
}
/**
 * @brief The address of the OFS register of a local index.
 * @param local The bank-local channel index, 0 to 7.
 * @return The register address in the bank.
 */
constexpr uint8_t OffsetAddress(uint8_t local) noexcept {
  return static_cast<uint8_t>(kOffset + local);
}
/**
 * @brief The address of the GAN register of a local index.
 * @param local The bank-local channel index, 0 to 7.
 * @return The register address in the bank.
 */
constexpr uint8_t GainAddress(uint8_t local) noexcept {
  return static_cast<uint8_t>(kGain + local);
}
/**
 * @brief The address of the DWC_TH register of a local index.
 * @param local The bank-local channel index, 0 to 7.
 * @return The register address in the bank.
 */
constexpr uint8_t DwcThAddress(uint8_t local) noexcept {
  return static_cast<uint8_t>(kDwcTh + local);
}
/**
 * @brief The address of the DWC_HYS register of a local index.
 * @param local The bank-local channel index, 0 to 7.
 * @return The register address in the bank.
 */
constexpr uint8_t DwcHysAddress(uint8_t local) noexcept {
  return static_cast<uint8_t>(kDwcHys + (local >> 1U));
}
/**
 * @brief The address of the TP_AIN register of a local index.
 * @param local The bank-local channel index, 0 to 7.
 * @return The register address in the bank.
 */
constexpr uint8_t TpAddress(uint8_t local) noexcept {
  return static_cast<uint8_t>(kTp + local);
}
/**
 * @brief The bit position of the odd (high) or even (low) half of a two-channel
 * register.
 * @param local The bank-local channel index.
 * @return 8 for an odd index, 0 for an even one.
 */
constexpr uint32_t HalfShift(uint8_t local) noexcept {
  return ((static_cast<uint32_t>(local) & 1U) != 0U) ? 8U : 0U;
}

namespace detail {

/// Whether `address` is a documented common-bank register.
constexpr bool IsCommon(uint8_t address) noexcept {
  switch (address) {
  case kGenCfg1:
  case kBankSel:
  case kDiagCtrl:
  case kPdnCtl:
  case kGenCfg2:
  case kGenCfg3:
  case kXorBitsCtl:
  case kDrdyAlarmSel:
  case kGenCfg4:
  case kDigDelayCfg1:
  case kDigDelayCfg2:
  case kAnaCfg1:
  case kAnaCfg2:
  case kAdcCal:
  case kDigFilter:
  case kGenCfg5:
  case kDeviceStatus:
  case kDeviceId:
    return true;
  default:
    break;
  }
  return false;
}

/// Whether `address` is a documented channel-bank register.
constexpr bool IsChannel(uint8_t address) noexcept {
  // PGA_CONFIG (0x08) to TP_AIN8 (0x36) is one run of registers, then GEN_CFG5
  // and DWC_FLAG.
  return ((address >= kPgaConfig) && (address < kTp + 8U)) || (address == kCorrection) ||
         (address == kDwcFlag);
}

} // namespace detail

/**
 * @brief Whether a register is documented in a bank (section 8, Tables 8-1,
 * 8-21, 8-72).
 * @param bank The bank.
 * @param address The address.
 * @return true for the 116 documented registers; every other address is
 * reserved and never accessed.
 */
constexpr bool IsDocumented(Bank bank, uint8_t address) noexcept {
  return (bank == Bank::Common) ? detail::IsCommon(address) : detail::IsChannel(address);
}

/**
 * @brief Whether a register is read only (DEVICE_STATUS, DEVICE_ID and
 * DWC_FLAG).
 * @param bank The bank.
 * @param address The address.
 * @return true for a register the driver never writes.
 */
constexpr bool IsReadOnly(Bank bank, uint8_t address) noexcept {
  if (bank == Bank::Common) {
    return (address == kDeviceStatus) || (address == kDeviceId);
  }
  return address == kDwcFlag;
}

/**
 * @brief Whether the driver keeps a write-through shadow of a register:
 * documented, writable and not one of the two frame-control registers GEN_CFG1
 * and BANK_SEL.
 * @param bank The bank.
 * @param address The address.
 * @return true when the register is shadowed.
 */
constexpr bool IsShadowed(Bank bank, uint8_t address) noexcept {
  if (!IsDocumented(bank, address) || IsReadOnly(bank, address)) {
    return false;
  }
  const bool command_register =
      (bank == Bank::Common) && ((address == kGenCfg1) || (address == kBankSel));
  return !command_register;
}

/**
 * @brief The value of a register after reset (section 8).
 * @param bank The bank.
 * @param address The address.
 * @return 0x0004 for DEVICE_ID, 1 for BANK_SEL, 0xFF00 for DWC_TH and the HYS
 * pairs 0x2B to 0x2D, 0 otherwise.
 */
constexpr uint16_t ResetValue(Bank bank, uint8_t address) noexcept {
  if (bank == Bank::Common) {
    if (address == kDeviceId) {
      return kDeviceIdValue;
    }
    return (address == kBankSel) ? 1_u16 : 0_u16;
  }
  const bool threshold = (address >= kDwcTh) && (address < kDwcTh + 8U);
  const bool hysteresis = (address > kDwcHys) && (address < kDwcHys + 4U);
  return (threshold || hysteresis) ? 0xFF00_u16 : 0_u16;
}

/**
 * @brief The number of documented registers over the three banks.
 * @return The count (116).
 */
constexpr uint32_t CountDocumented() noexcept {
  uint32_t count = 0U;
  for (uint32_t b = 0U; b < kBankCount; ++b) {
    for (uint32_t a = 0U; a < kAddressSpace; ++a) {
      if (IsDocumented(static_cast<Bank>(b), static_cast<uint8_t>(a))) {
        ++count;
      }
    }
  }
  return count;
}

// The register map is the datasheet's: 18 common registers and 49 in each
// channel bank.
static_assert(CountDocumented() == 116U);
static_assert(IsDocumented(Bank::Common, kDeviceId) && !IsDocumented(Bank::Common, 0x00));
static_assert(IsDocumented(Bank::Ain1To8, kDwcFlag) && !IsDocumented(Bank::Ain1To8, 0x38));
static_assert(!IsDocumented(Bank::Ain9To16, 0x13 + 0x40));
// Reset values (Tables 8-1 to 8-71).
static_assert(ResetValue(Bank::Common, kBankSel) == 1U);
static_assert(ResetValue(Bank::Common, kDeviceId) == 4U);
static_assert(ResetValue(Bank::Ain1To8, kDwcTh) == 0xFF00U);
static_assert(ResetValue(Bank::Ain9To16, kDwcHys) == 0U);
static_assert(ResetValue(Bank::Ain9To16, kDwcHys + 1U) == 0xFF00U);
// The reversed bank (section 8.3): AIN16 is local 0, AIN9 is local 7; both
// banks round trip.
static_assert(Locate(0).bank == Bank::Ain1To8 && Locate(0).local == 0U);
static_assert(Locate(7).bank == Bank::Ain1To8 && Locate(7).local == 7U);
static_assert(Locate(15).bank == Bank::Ain9To16 && Locate(15).local == 0U);
static_assert(Locate(8).bank == Bank::Ain9To16 && Locate(8).local == 7U);
static_assert(ChannelAt(Locate(11).bank, Locate(11).local) == 11U);
// AIN15 (channel 14) is local 1: the high half of PGA_CONFIG at 0x08 (Table
// 8-74).
static_assert(PgaConfigAddress(Locate(14).local) == 0x08U && HalfShift(Locate(14).local) == 8U);
// AIN9 (channel 8) is local 7: OFS at 0x18, TP at 0x36 (Tables 8-72).
static_assert(OffsetAddress(Locate(8).local) == 0x18U && TpAddress(Locate(8).local) == 0x36U);
static_assert(DwcHysAddress(Locate(8).local) == 0x2DU);

/** @} */

} // namespace ads9324::reg

namespace ads9324::detail {

/// Last enumerator of `reg::Bank` (detail::IsEnumerator).
template <>
struct EnumLast<reg::Bank> {
  static constexpr reg::Bank kValue = reg::Bank::Ain9To16; ///< The last enumerator.
};

} // namespace ads9324::detail

#endif // ADS9324_REGISTERS_HPP
