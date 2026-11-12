// clang-format off: the table below must keep one row per line.
/**
 * @file ads9324_spi_interface.hpp
 * @brief The port of the ADS9324 driver: the CRTP base, its pin enums, the `SpiPort` concept and
 * the 24-bit register framing.
 * @copyright Copyright (c) 2026 The driver-ads9324 authors. SPDX-License-Identifier: MIT
 *
 * @details
 * The driver never calls an SDK. It calls hooks on a class you derive from
 * `ads9324::SpiInterface<Self>`:
 *
 * - `bool EnsureInitialized() noexcept` brings the bus and pins up on the first call; `true` at once afterwards.
 * - `bool Transfer(const uint8_t* tx, uint8_t* rx, std::size_t len) noexcept` is one full-duplex transfer with
 *   CS low for all `len` bytes; `rx` may be `nullptr`.
 * - `void DelayUs(uint32_t us) noexcept` waits at least `us` microseconds.
 *
 * The port moves raw bytes: it never retries, never interprets a frame and never configures the
 * chip. What the chip needs from the bus is below as `constexpr` values (`kSpiMode` to `kFrameBits`,
 * datasheet sections 5.6, 5.7 and 7.5); a port checks its settings against them with `static_assert`.
 * The optional hooks (`GpioSet`, `GpioGet`, `CaptureLanes`, `Log`) are described in `ads9324.hpp`.
 */
// clang-format on
#ifndef ADS9324_SPI_INTERFACE_HPP
#define ADS9324_SPI_INTERFACE_HPP

#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <span>

#include "ads9324_detail.hpp"
#include "ads9324_registers.hpp"
#include "ads9324_types.hpp"

namespace ads9324 {

/**
 * @name Bus requirements
 * What the ADS9324 needs from the SPI bus (datasheet sections 5.6, 5.7
 * and 7.5). A port and a board configuration check their settings against these
 * with `static_assert` (KB-CPP-123).
 * @{
 */
/// SPI mode the driver is tested with: CPOL 0, CPHA 0. The datasheet names none
/// (assumption A1, derived from the timing diagram: SDI is sampled on the
/// rising edge).
inline constexpr uint8_t kSpiMode = 0;
/// Each byte is clocked most significant bit first.
inline constexpr bool kMsbFirst = true;
/// Highest SCLK frequency, f_SCLK (section 5.6), for frames that read nothing back.
inline constexpr uint32_t kMaxClockHz = kSclkMaxHz;
/// Highest SCLK frequency when SDOUT is read in mode 0: the 17 ns td_CKDO must fit half a period
/// (assumption A2). A port that reads registers or data checks against this one.
inline constexpr uint32_t kMaxSdoutClockHz = 29'000'000;
/// Shortest time CS stays high between two frames, thi_CS (section 5.6).
inline constexpr uint32_t kMinCsHighNs = 16;
/// CS low to the first SCLK edge, tsu_CSCK.
inline constexpr uint32_t kCsSetupNs = 16;
/// Last SCLK edge to CS high, tht_CSCK.
inline constexpr uint32_t kCsHoldNs = 16;
/// `true`: CS stays low for a whole frame; the chip decodes a frame when CS
/// rises (section 7.5).
inline constexpr bool kFrameIsAtomic = true;
/// `false`: SCLK may pause between bytes of a frame, so a port need not send a
/// whole frame without gaps (the chip decodes on the rising edge of CS, not on
/// a byte count).
inline constexpr bool kContinuousWord = false;
/// Bits in a register frame: 8 address bits and 16 data bits.
inline constexpr uint32_t kFrameBits = 24;
/** @} */

/** @brief Control pins the port may drive or read through the optional hooks.
 */
enum class CtrlPin : uint8_t {
  Reset = 0,  ///< RESET, active low on the package pin; idles high (INACTIVE).
  Convst = 1, ///< CONVST: ACTIVE is a high level, and the line idles high (ACTIVE) between pulses.
  DrdyAlarm = 2, ///< DRDY/ALARM: an input, read with `GpioGet()`.
};

/** @brief Logical level of a control pin; the port maps it to the physical
 * polarity. */
enum class GpioSignal : uint8_t {
  INACTIVE = 0, ///< The pin function is deasserted.
  ACTIVE = 1,   ///< The pin function is asserted.
};

/**
 * @brief CRTP base of the platform port.
 *
 * Derive `class MyPort : public ads9324::SpiInterface<MyPort>` and implement
 * the hooks in this file's description. The base declares no hook itself (a
 * forwarding hook would turn a missing override into infinite recursion) and no
 * data; the driver checks your class with the `SpiPort` concept, so a missing
 * or misdeclared hook is a compile error that names it.
 *
 * @tparam Derived Your port class.
 */
template <typename Derived>
class SpiInterface {
public:
  /** @brief Ports are not copied: the driver holds a reference. */
  SpiInterface(const SpiInterface&) = delete;
  /** @brief Ports are not copied: the driver holds a reference. */
  SpiInterface& operator=(const SpiInterface&) = delete;

protected:
  /** @brief Only the derived port is constructed. */
  SpiInterface() noexcept = default;
  /** @brief Destroyed through the derived port only. */
  ~SpiInterface() noexcept = default;
};

/**
 * @brief Satisfied by a class that derives from `SpiInterface<Port>` and
 * implements the required hooks with exactly the types above, `noexcept` and
 * returning `bool`.
 *
 * Each hook's address is matched with `detail::ExactHook`, which takes a member
 * function of exactly the signature and converts no value: a hook declared with
 * a narrower length (`uint16_t len`) or without `noexcept` does not satisfy the
 * concept, instead of truncating every transfer silently (KB-CPP20-006).
 *
 * @tparam Port The port class.
 */
template <typename Port>
concept SpiPort = std::derived_from<Port, SpiInterface<Port>> && requires {
  detail::ExactHook<Port, bool() noexcept>::Take(&Port::EnsureInitialized);
  detail::ExactHook<Port, bool(const uint8_t*, uint8_t*, std::size_t) noexcept>::Take(
      &Port::Transfer);
};

namespace detail {

/// Bytes in one register frame.
inline constexpr std::size_t kFrameBytes = 3;

/// The GEN_CFG1 value that asks for the register at `address` (section 7.5.2).
constexpr uint16_t ReadCommand(uint8_t address) noexcept {
  return reg::GenCfg1ReadAddress::Insert(reg::kRegRdEn, address);
}

/**
 * Register access over the 24-bit frames of datasheet section 7.5, one transfer
 * per frame. A read is two frames: the read command, then a frame of zeros
 * whose reply carries the data.
 */
template <SpiPort Port>
class RegisterBus {
public:
  explicit RegisterBus(Port& port) noexcept : port_(port) {}

  Port& GetPort() & noexcept {
    return port_;
  }

  /// One write frame: `address` and 16 data bits. `Bus` on a failed transfer.
  Error Write(uint8_t address, uint16_t value) noexcept {
    tx_[0] = address;
    tx_[1] = static_cast<uint8_t>(value >> 8U);
    tx_[2] = static_cast<uint8_t>(value & 0xFFU);
    return port_.Transfer(tx_.data(), nullptr, kFrameBytes) ? Error::Ok : Error::Bus;
  }

  /// A read of the register at `address` of the selected bank: command frame,
  /// then a zero frame. The reply carries the data in SDOUT[23:8] and 0x00 in
  /// SDOUT[7:0] (Table 7-17, Figure 5-1); a reply whose last byte is not 0x00
  /// is a line fault, never data (`Bus`).
  Result<uint16_t> Read(uint8_t address) noexcept {
    if (const Error e = Write(reg::kGenCfg1, ReadCommand(address)); e != Error::Ok) {
      return Result<uint16_t>::Failure(e);
    }
    tx_.fill(0U);
    rx_.fill(0U);
    if (!port_.Transfer(tx_.data(), rx_.data(), kFrameBytes)) {
      return Result<uint16_t>::Failure(Error::Bus);
    }
    if (rx_[2] != 0U) {
      return Result<uint16_t>::Failure(Error::Bus);
    }
    return Result<uint16_t>::Success(static_cast<uint16_t>((static_cast<uint32_t>(rx_[0]) << 8U) |
                                                           static_cast<uint32_t>(rx_[1])));
  }

  /// Clock `out.size()` bytes of zeros and capture the reply (data readout on
  /// SDOUT). `Bus` on a failed transfer, `InvalidArgument` for an empty or
  /// oversized buffer (nothing sent).
  Error ReadStream(std::span<uint8_t> out) noexcept {
    if (out.empty() || (out.size() > kMaxFrameBytes)) {
      return Error::InvalidArgument;
    }
    zeros_.fill(0U);
    return port_.Transfer(zeros_.data(), out.data(), out.size()) ? Error::Ok : Error::Bus;
  }

private:
  Port& port_;
  // The transfer buffers belong to the driver object, never to a caller's stack, so a port whose
  // bus reads them by DMA reaches them (KB-HW-019).
  std::array<uint8_t, kFrameBytes> tx_{};
  std::array<uint8_t, kFrameBytes> rx_{};
  std::array<uint8_t, kMaxFrameBytes> zeros_{};
};

} // namespace detail

} // namespace ads9324

#endif // ADS9324_SPI_INTERFACE_HPP
