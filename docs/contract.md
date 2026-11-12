# Contract

What the ADS9324 driver promises, what it needs, and the rules a caller follows.

<!-- generated: datasheet-refs -->
??? abstract "Datasheet sections this page relies on"

    From Texas Instruments SBASB22 -:

    - Electrical characteristics, power-up and reset times: 5.5 Electrical Characteristics, [p. 7](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=7)
    - Timing diagrams, DRDY and the data frame: 5.8 Timing Diagrams, [p. 12](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=12)
    - ADC gain and offset error calibration: 7.3.10 ADC Gain and Offset Error Calibration, [p. 32](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=32)
    - Diagnostic byte and the randomizer: 7.3.14.3 Diagnostic Flags; 7.3.14.4 ADC Output Data Randomizer, [p. 44](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=44)
    - Normal operation and standby: 7.4.2 Normal Operation; 7.4.3 Standby Mode, [p. 49](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=49)

    All of them, with the driver API for each: [Datasheet map](datasheet.md).
<!-- /generated -->

The driver inherits the [Kokeb Labs driver contract](https://github.com/kokeb-labs/.github/blob/main/standards/driver.md);
this page states only what is specific to it. A change to anything here is a breaking change, listed under
**Breaking** in the [changelog](changelog.md).

## Promises

- The constructor is `noexcept`, stores the configuration, does no I/O and cannot fail.
- `EnsureInitialized()` is the only way to start the driver. Its first call checks the stored configuration
  (`InvalidArgument` or `Unsupported`, nothing sent), brings the bus up, waits the power-up time, resets the chip
  (RESET when `Config::hardware_reset` is set, otherwise SW_RST), checks DEVICE_ID and BANK_SEL, waits for the
  calibration flag to clear and only then writes the configuration. On any failure the driver stays uninitialised
  and the next call starts again from the beginning; once it has succeeded, later calls return `Ok` with no I/O.
- Every call that touches the chip returns `Error::NotInitialized` before `EnsureInitialized()` succeeds. No call
  initialises or resets the chip as a side effect. `ApplyConfig()` before the start only stores the configuration.
- Every port failure propagates as `Error::Bus`; no getter returns a stale or zero value after a failed read.
- A setter changes the stored configuration only after the chip accepted every write; after a failure the driver
  keeps the old configuration. The driver's shadow of the registers equals the chip after every setter that
  returns `Ok`.
- A setter writes only the registers that change, in the order the chip needs: the window thresholds before the
  window is enabled, and a SYSREF pulse after a filter change.
- Every wait is bounded: the power-up and reset waits are constants, the DRDY wait by `Sampling::ready_timeout_us`,
  the ADC calibration by 125 ms plus 25 ms.
- A frame that cannot be a conversion (all bytes `0x00` or all `0xFF`, which is what a stuck or floating SDOUT
  reads) is `DataMismatch`, not data. `Sampling::reject_stuck_frames` turns this off for a test pattern.
- A DIAG byte that reports a reset makes the driver uninitialised and returns `UnexpectedReset`: the configuration
  is gone, and the next `EnsureInitialized()` writes it again. The DIAG byte is off in `Config{}`, so every
  configuration call (`ApplyConfig()` and the setters, `ClearWindowFlags()`, `ReadWindowFlags()`, the calibration
  calls, `RunDataPathSelfTest()`, `PowerDown()`) first re-sends BANK_SEL, reads DEVICE_ID and DIAG_CTRL, and returns
  `UnexpectedReset` with nothing else written when RESET_DETECT_FLAG is set. A reset between two frames that no such
  call has looked at is seen only by the next data frame with the DIAG byte, the next configuration call or `Wake()`.
- A register reply is data only when its last byte (SDOUT[7:0]) is `0x00`; any other tail is `Bus`. A stuck-low
  line reads as zeros and passes that check, so every status read that would accept "idle" or "no reset"
  (`IsCalibrationBusy()`, the end of a calibration wait, `Wake()`, the reset probe) also reads DEVICE_ID and returns
  `Bus` when it is not `0x0004`.
- After a configuration call fails part-way the layout registers (lanes, word length, channel selection) are
  unknown: `ReadFrame()` and `MeasureOnce()` return `Refused` until a configuration call succeeds.
- `MeasureOnce()` sends as many CONVST pulses as one result needs (`PulsesPerResult()`: the FIR decimation, or the
  block-average length with the external clock) and waits `ConversionTimeUs()`, the datasheet's
  `(N - 1) x tCYC + 1.25 us`. `Convert()` leaves CONVST high, as Figure 5-7 requires for the internal oversampling
  clock.
- `Config{}` is the datasheet's initialisation example (Table 7-18), which reads all 16 channels as 16-bit words on
  SDOUT; `ChipResetConfig()` is the chip's own reset state (eight lanes on D7 to D0).
- A missing `DelayUs()` is a compile error; a missing `GpioSet()` gives `Error::Unsupported` from the calls that
  need CONVST or RESET; a missing `GpioGet()` makes the timed wait the only ready source; a missing `Log()` logs
  nothing. A misdeclared hook is a compile error.
- `Deinitialize()` puts the chip in standby, its lowest-current state, even if that write failed.

## Depends on

| Dependency | Version | Why |
| --- | --- | --- |
| A C++20 compiler | GCC 10, Clang 13 or later | Concepts constrain the port |
| CMake | 3.16 or later | Generates `ads9324_version.h` |
| Your port | | The bus, within the bus requirements; the delay; optionally CONVST, RESET and DRDY, each at its idle level before the first transfer; lane capture for 2, 4 or 8 lanes; and logging ([Porting](porting.md)) |

Nothing else: no RTOS, no SDK, no other repository of the org.

## Threading

The driver is not thread-safe and takes no lock. Use each driver object from one task at a time, or guard every
call on it with one mutex; the layer that owns the driver owns that lock, and the bus lock when several chips
share a bus. Nothing may run in an ISR: every call can block for a bus transfer or a delay. Separate objects
share no state.

## Errors

`ads9324::Error` inside `ads9324::Result<T>`. Values 0 to 9 are the shared base set; chip-specific values start at
16. Every value is returned by some call, except the three the chip cannot produce, which a test asserts are never
returned.

| `Error` | Value | Returned when |
| --- | --- | --- |
| `Ok` | 0 | The call succeeded |
| `NotInitialized` | 1 | `EnsureInitialized()` has not succeeded, or `Deinitialize()` ran since |
| `InvalidArgument` | 2 | A `Config` field or argument is out of range (`Validate()`); nothing was sent |
| `Bus` | 3 | A port hook returned `false` |
| `Timeout` | 4 | The calibration flag, the DRDY pin or a reset did not finish in time |
| `DeviceIdMismatch` | 5 | DEVICE_ID is not `0x0004` after a reset |
| `Crc` | 6 | Never returned: the chip has no CRC on this interface |
| `Busy` | 7 | Never returned: the chip has no busy reply |
| `DeviceFault` | 8 | Never returned: the chip has no fault flag; kept for the shared set |
| `Unsupported` | 9 | A port hook the call needs is absent, or the daisy chain was requested |
| `UnexpectedReset` | 16 | The DIAG byte or RESET_DETECT_FLAG reports a reset: the chip lost its configuration. The driver is uninitialised afterwards |
| `VerifyFailed` | 17 | A read-back (`Verify::ReadBack`) or the register check after standby differs from what was written |
| `Refused` | 18 | The call is not allowed in this state (standby, or a calibration with a digital filter on); nothing was sent |
| `DataMismatch` | 19 | A frame is all zeros or all ones, or the data-path self-test read another channel's value |

The [API guide](api-guide.md#handle-errors) says what to do about each.

## Memory

No heap, no exceptions, no RTTI, no `std::function`, no `std::string`, no `printf`, no floating point and no
static mutable state. A driver object holds a reference to the port, a `Config` (340 bytes) and a shadow of the
116 registers: 792 bytes in all. The largest stack buffers are two arrays of 50 bytes (the longest data frame).

## Platform

Platform-free: the same code runs on any target with a C++20 compiler, including the host tests. The ESP-IDF port
in [`examples/esp32`](examples.md) is an example, not part of the driver.

## Deviations from the standard

None.
