# API guide

The driver's API by task, with code that compiles: initialise, measure, set a range, watch a window, calibrate,
handle errors and go to standby.

<!-- generated: datasheet-refs -->
??? abstract "Datasheet sections this page relies on"

    From Texas Instruments SBASB22 -:

    - ADC transfer function and output codes: 7.3.5 ADC Transfer Function, [p. 27](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=27)
    - ADC gain and offset error calibration: 7.3.10 ADC Gain and Offset Error Calibration, [p. 32](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=32)
    - Digital filters and the system phase calibration: 7.3.11 Digital Filter; 7.3.11.1 System Phase Calibration, [p. 34](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=34)
    - Digital window comparator: 7.3.12 Digital Window Comparator, [p. 39](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=39)
    - Alarm modes and the DRDY/ALARM pin: 7.3.13 Alarm Modes, Table 7-10, [p. 41](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=41)
    - Test patterns: 7.3.14.5 Test Patterns for Data Interface, Table 7-14, [p. 47](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=47)
    - Normal operation and standby: 7.4.2 Normal Operation; 7.4.3 Standby Mode, [p. 49](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=49)

    All of them, with the driver API for each: [Datasheet map](datasheet.md).
<!-- /generated -->

## The shape of the API

| Group | Calls |
| --- | --- |
| Lifecycle | `EnsureInitialized()`, `IsInitialized()`, `Deinitialize()`, `Reset()`, `Identify()` |
| Configuration | `ApplyConfig()`, `GetConfig()`, `SetInput()`, `SetInterface()`, `SetFilter()`, `SetChannels()`, `SetAlarm()`, `SetReference()`, `SetTestPattern()` |
| Measurement | `Convert()`, `WaitDataReady()`, `ReadFrame()`, `MeasureOnce()`, `ReadAlarmPin()` |
| Window comparator | `ReadWindowFlags()`, `ClearWindowFlags()` |
| Calibration | `RunAdcCalibration()`, `DisableAdcCalibration()`, `IsCalibrationBusy()` |
| Power | `PowerDown()`, `Wake()`, `IsPoweredDown()` |
| Diagnostics | `ReadRegister()`, `GetShadow()`, `RunDataPathSelfTest()` |
| Pure helpers | `Validate()`, `RangeMillivolts()`, `CodeToMicrovolts()`, `OutputRateHz()`, `ads9324::GetDriverVersion()` |

Every call that touches the chip returns `Result<T>`: check `ok()` (or the `bool` conversion) and read `value`, or
read `error`. `Result` is `[[nodiscard]]`, so an unchecked call is a compiler warning. Channels are counted from 0:
channel 0 is AIN1 and channel 15 is AIN16.

## Initialise

```cpp
--8<-- "tests/docs/api_tasks.cpp:lifecycle"
```

The constructor stores the port and the configuration (`Config{}` unless you pass one) and does no I/O.
`EnsureInitialized()` is the one way to start the chip. Its first call brings the bus up, waits the power-up time,
resets the chip (RESET when `Config::hardware_reset` is set, otherwise SW_RST), checks DEVICE_ID, clears the reset
flag and writes the configuration.

Once it has succeeded, `EnsureInitialized()` returns `Ok` at once with no I/O, so a caller (or a handler that owns
the driver) can call it before every use. After a failure the driver stays uninitialised and the next call starts
again from the beginning. No other call starts the chip: before `EnsureInitialized()` succeeds every read and write
returns `NotInitialized` and sends nothing. `ApplyConfig()` before the start only stores the configuration for the
first `EnsureInitialized()`.

## Measure

```cpp
--8<-- "tests/docs/api_tasks.cpp:measure"
```

`MeasureOnce()` is `Convert()`, `WaitDataReady()` and `ReadFrame()` in one call. `Convert()` pulses CONVST (high,
then low, then high again: the falling edge samples all 16 channels together, and the line is high when the result
arrives). `MeasureOnce()` repeats the pulse as often as one result needs (`PulsesPerResult()`).
`WaitDataReady()` waits for the DRDY pin when `Sampling::ready` is `ReadySource::Pin` and the port has
`GpioGet()`, otherwise the conversion time. `ReadFrame()` clocks the frame out and decodes it into a `Frame`:

| `Frame` field | Meaning |
| --- | --- |
| `codes[n]` | The signed 16-bit code of channel n (0 for a channel that is not in the frame) |
| `microvolts[n]` | The input voltage of channel n, from the code and the channel's range, in integer microvolts |
| `raw[n]`, `low_byte[n]` | The wire value after derandomizing, and the extra byte of a 24-bit word |
| `present_mask`, `clipped_mask` | Bit n: channel n is in the frame, or sits at a code limit |
| `diag`, `window_alert`, `reset_detected` | The DIAG byte, when `Interface::append_diag` or `randomize` adds it |

### Drive CONVST yourself

A port without `GpioSet()` for CONVST, or a design where a timer or a PWM channel clocks CONVST, calls `ReadFrame()`
after each conversion instead of `MeasureOnce()`; the driver then never touches CONVST, except that
`RunAdcCalibration()` and a filter change need pulses and return `Unsupported` without the hook. Set
`Sampling::convst_hz` to the rate your timer runs, so the timed waits and the output-rate helpers match.

With a block-average filter on the external clock, or a FIR filter, one `Convert()` is one CONVST pulse, but the
chip produces a result only after the block (`PulsesPerResult(Filter)` pulses); pulse CONVST that many times (a
free-running timer does this) before `ReadFrame()`. `MeasureOnce()` does it for you. [Limitations](limitations.md)
lists this.

## Set a range

```cpp
--8<-- "tests/docs/api_tasks.cpp:input"
```

`SetInput()` takes a channel and an `Input`: range (±2.5 V, ±5 V, ±6.25 V, ±10 V, ±12.5 V), type (differential,
single-ended, single-ended open-wire safe), bandwidth, common-mode correction, offset, gain, phase delay and the
window. The registers of the AIN9 to AIN16 bank are in reverse order; `SetInput(8, ...)` writes AIN9's registers.
A call writes only what changed and stores the input only after the chip accepted it. The offset and gain
correction helpers `OffsetCorrectionFor()` and `GainCorrectionFor()` turn a measured error into the register
values.

## Watch a window

```cpp
--8<-- "tests/docs/api_tasks.cpp:window"
```

The window comparator compares the top byte of each code with a high and a low threshold, with hysteresis. The
driver writes the thresholds before it enables the window, so no false alert appears between the two writes. With
`AlarmFunction::WindowAlert` the DRDY/ALARM pin shows the alert (`ReadAlarmPin()` reads it, with `active_low`
applied); the flags per channel are latched until `ClearWindowFlags()` clears a group. `Alarm::pulse` makes the pin
a pulse instead of a level (window alert only); `AlarmFunction::OrMasked` needs `or_mask_acknowledged`, because the
sense of the source mask is not verified ([Limitations](limitations.md)).

## Choose a filter

```cpp
--8<-- "tests/docs/api_tasks.cpp:filter"
```

One digital filter at a time: block average, moving average or a FIR filter (FIR1 to FIR5 and FIR7). The driver
pulses SYSREF (CONVST low, high, low) after a filter write, as the chip requires; that needs `GpioSet()` for
CONVST. A filter and the ADC calibration exclude each other: switch the filter off first.

## Calibrate

```cpp
--8<-- "tests/docs/api_tasks.cpp:calibrate"
```

`RunAdcCalibration()` removes the converter's own offset and gain error (`Config::calibration` chooses which, and
whether CONVST runs single-shot or free). It waits for CALIB_BUSY with a bound of 150 ms. The chip keeps the
result as long as ADC_CAL_MODE stays set, so the driver leaves it set; `DisableAdcCalibration()` removes the
correction.

## Check the data path

`RunDataPathSelfTest()` writes a different constant test pattern to every channel, reads a frame and compares
each channel with its own pattern, then restores the configuration. It proves the data lanes, the word format and
the channel-to-register mapping of both banks on your board, and returns `DataMismatch` when a channel returns
another's value. Run it once at bring-up.

## Handle errors

```cpp
--8<-- "tests/docs/api_tasks.cpp:errors"
```

| Signal | Meaning | What to do |
| --- | --- | --- |
| `Error::Bus` | The port failed a transfer or the bus bring-up | Check the port and the wiring; retry, or `Reset(Software)`, or `Deinitialize()` and `EnsureInitialized()` |
| `Error::Timeout` | A reset, the DRDY pin or a calibration did not finish | Check the supplies, the reference and CONVST; raise `Sampling::ready_timeout_us` |
| `Error::DataMismatch` | The frame is all zeros or all ones | Check SDOUT and the supplies; `RunDataPathSelfTest()` |
| `Error::UnexpectedReset` | The chip reset behind the driver's back | `EnsureInitialized()` writes the configuration again |
| `Error::VerifyFailed` | A read-back differs from what was written | Lower the clock; check the supplies |
| `Error::Refused` | Standby, or a calibration with a filter on | `Wake()`, or switch the filter off |
| `Error::Unsupported` | A hook the call needs is absent, or the daisy chain was requested | Add the hook; the daisy chain is [out of scope](limitations.md) |

Every value, with the calls that return it, is on the [Contract](contract.md#errors) page.
`ads9324::ToString()` names an error for a log.

## Diagnostics

For bring-up and fault finding, the driver keeps what it believes the chip holds and offers one raw access
(handbook KB-CPP-119, KB-CPP-129):

| Call | I/O | What it gives |
| --- | --- | --- |
| `ReadRegister(bank, address)` | bank select, command and data frame, no retry | The raw 16 bits of one documented register |
| `GetLastRegisterReply()` | none | The raw value the last `ReadRegister` returned |
| `GetShadow(bank, address)` | none | The value the driver believes the chip holds |
| `Verify::ReadBack` (a trailing argument of every setter) | one two-frame read per register written | `VerifyFailed` when a register differs |

## Standby

```cpp
--8<-- "tests/docs/api_tasks.cpp:power"
```

`PowerDown()` writes the standby bit; until `Wake()` every other call returns `Refused`. `Wake()` waits 30 ms and
checks that the chip kept its registers (GEN_CFG3 against the shadow and the reset flag). `Deinitialize()` is the
standby write plus `NotInitialized` afterwards: `Deinitialize()` then `EnsureInitialized()` is also the recovery
after a fault.

Next: [Configuration](configuration.md).
