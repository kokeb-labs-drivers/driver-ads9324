# Configuration

Every field of `ads9324::Config`, how a configuration is checked, the data interface, and the optional Kconfig
layer.

<!-- generated: datasheet-refs -->
??? abstract "Datasheet sections this page relies on"

    From Texas Instruments SBASB22 -:

    - Supplies, reference and input ranges: 5.3 Recommended Operating Conditions, [p. 6](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=6)
    - Analog inputs and the input clamps: 7.3.1 Analog Inputs; 7.3.2 Input Clamp Protection Circuit, [p. 25](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=25)
    - PGA, input ranges, common-mode range and bandwidth: 7.3.4 Programmable Gain Amplifier (PGA), Tables 7-1 and 7-2, [p. 26](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=26)
    - ADC transfer function and output codes: 7.3.5 ADC Transfer Function, [p. 27](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=27)
    - Reference selection: 7.3.6 Reference, Table 7-3, [p. 28](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=28)
    - Open-wire safe mode: 7.3.7 Open Wire Safe Mode, [p. 30](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=30)
    - System offset and gain calibration: 7.3.8 System Offset Calibration; 7.3.9 System Gain Calibration, [p. 31](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=31)
    - Digital filters and the system phase calibration: 7.3.11 Digital Filter; 7.3.11.1 System Phase Calibration, [p. 34](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=34)
    - Block average, moving average and FIR filters: 7.3.11.2 to 7.3.11.4, [p. 35](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=35)
    - Digital window comparator: 7.3.12 Digital Window Comparator, [p. 39](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=39)
    - Alarm modes and the DRDY/ALARM pin: 7.3.13 Alarm Modes, Table 7-10, [p. 41](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=41)
    - Data interface: lanes, word length and channel modes: 7.3.14 Data Interface; 7.3.14.1 ADC Channel Modes, Table 7-12, [p. 42](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=42)
    - Diagnostic byte and the randomizer: 7.3.14.3 Diagnostic Flags; 7.3.14.4 ADC Output Data Randomizer, [p. 44](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=44)
    - Output drive strength and output delay: 7.3.14.6 Digital Output Drive Strength Control; 7.3.14.7 Digital Output Delay Adjustment, [p. 48](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=48)
    - Common registers: 8.1 ADS93xx Common Registers, [p. 53](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=53)
    - Channel registers AIN1 to AIN8: 8.2 AIN1 - AIN8 Channel Registers, [p. 64](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=64)

    All of them, with the driver API for each: [Datasheet map](datasheet.md).
<!-- /generated -->

## Fields

`Config{}` is the datasheet's initialisation example (Table 7-18) laid over the reset values of every other
register: it reads all 16 channels as 16-bit words in one lane on SDOUT, with the ±5 V differential range on every
channel. `ChipResetConfig()` is the chip's own reset state (eight lanes on D7 to D0) for code that needs it.
Fields with a unit carry it in the name; voltages are integer microvolts.

| Field | Default | Range | Registers |
| --- | --- | --- | --- |
| `interface` | One lane on SDOUT, 16-bit, two's complement | [Data interface](#data-interface) | GEN_CFG3, XOR_BITS_CTL, GEN_CFG4, DIG_DELAY_CFG |
| `reference` | `Pin` | `Pin`, `Internal`, `External` | ANA_CFG1 |
| `inputs[16]` | Differential, ±5 V, low bandwidth, no correction, window off | [Per channel](#per-channel) | Both channel banks |
| `filter` | `None` | [Filters](#filters) | DIG_FILTER, GEN_CFG5 |
| `channels` | All 16 | 16, 8 (2 groups), 4 (4 groups), 2 (8 groups) | ANA_CFG2 |
| `low_speed_crosstalk` | `false` | | CH_XTALK_LOW_SPEED; for under 100 kSPS per channel |
| `alarm` | DRDY, active high, level | [Alarm](#alarm) | DRDY_ALARM_SEL, GEN_CFG4 |
| `sampling.convst_hz` | 100,000 | 100,000 to 1,000,000 | driver only: the timed waits |
| `sampling.ready` | `Time` | `Time`, `Pin` | driver only |
| `sampling.ready_timeout_us` | 10,000 | 1 to 200,000 | driver only |
| `sampling.reject_stuck_frames` | `true` | | driver only: all-0x00 or all-0xFF is `DataMismatch` |
| `correction` | Offset and gain stages on | | GEN_CFG5 of the channel banks |
| `calibration` | Not at start, offset and gain, single shot | | ADC_CAL_CTL |
| `test_pattern[2]` | Off | One per group of eight channels | TP_CFG, TP_AIN |
| `window_glitch[2]` | 0 | 0 to 15 per group | DWC glitch filter |
| `daisy` | Off | `Validate()` refuses `enabled` | [Limitations](limitations.md) |
| `supply` | 0 (not checked) | AVDD_5V 4750 to 5250, AVDD_1V8 1700 to 1900, IOVDD 1700 to 3600 mV | driver only |
| `hardware_reset` | `false` | Needs `GpioSet()` for RESET | driver only |

Start from `Config{}` and change fields:

```cpp
--8<-- "tests/docs/configuration.cpp:custom"
```

## Per channel

`Input` holds everything the chip stores for one channel: `type` (differential, single-ended, single-ended
open-wire safe), `range` (±2.5, ±5, ±6.25, ±10 or ±12.5 V), `bandwidth` (25.5 kHz, or 280 kHz to 350 kHz by
range), `cm_correction`, `offset_quarter_lsb` (-512 to 511), `gain` (-8192 to 8191, the result is multiplied by
`1 + gain / 65536`), `phase_delay` (0 to 255 CONVST periods) and the window (`enabled`, `high_code`, `low_code`,
`hysteresis`). The channels of one bank share a type class: a bank cannot mix differential and single-ended inputs
(`Validate()` returns `InvalidArgument`), because the datasheet sets the common-mode range per bank.

## Data interface

`Interface` chooses how a conversion leaves the chip:

| Field | Values | Notes |
| --- | --- | --- |
| `lanes` | `One`, `Two`, `Four`, `Eight` | Lanes other than `One` cannot use SDOUT: the data is on D7 to D0 and needs `CaptureLanes()` |
| `data_on_sdout` | `true` | One lane on SDOUT (`true`) or on D7 (`false`) |
| `length` | `Bits16`, `Bits24` | 24 bits adds an extra byte per channel |
| `format` | `TwosComplement`, `OffsetBinary` | Offset binary cannot be combined with a window |
| `append_diag` | `false` | One DIAG byte at the end of the frame: window alert and reset flags |
| `randomize` | `false` | XORs the data with a PRBS and appends the PRBS byte; needs 16-bit words |
| `drive` | `Normal` | 0.5x, 1x, 1.5x or 2x output drive |
| `delay_ns[9]` | 0 | Output delay 0 to 5 ns for SDOUT and D0 to D7 |

`ChannelSelect` picks the channels in the frame: all 16, or a group of 8, 4 or 2 (Table 7-12). `FrameBytes()` says
how long the frame is, `FrameChannels()` which channels it carries, and `detail::DecodeFrame()` is the pure
function that turns bytes into codes (host-testable on its own).

## Filters

One filter at a time (`FilterKind`): `BlockAverage` and `MovingAverage` with a `length` of 2 to 128, or `Fir` with
FIR1 to FIR5 or FIR7 (FIR6 is reserved and absent). A block average can use the chip's internal oversampling clock
(`internal_clock`); the average types need a length, and `internal_clock` applies only to the block average.
`ResultsPerOutput()`, `FirDecimation()` and `OutputRateHz()` compute the output rate from the filter and
`convst_hz`.

## Alarm

`Alarm::function` is what DRDY/ALARM shows: `Drdy`, `WindowAlert`, `CalDone`, or `OrMasked` (the OR of
`sources`, needs `or_mask_acknowledged`). `active_low` inverts the pin (for every function), `pulse` makes the
window alert a pulse, and `disabled` gates the pin off (refused while the ready source is the pin).

## Validation

`ads9324::Validate(config)` checks a configuration without touching the chip, and `EnsureInitialized()`,
`ApplyConfig()` and every setter call it before any I/O. It returns `Error::InvalidArgument` when an enum holds a
value that is not an enumerator, a field is out of its range, or the combination is impossible: lanes other than
`One` with SDOUT, the randomizer with 24-bit words, a bank that mixes differential and single-ended inputs, an
average filter with no length, a pulse alarm for anything but the window alert, an `OrMasked` mask that is empty or
not acknowledged, a window with offset binary, a calibration at start with a filter on, or a channel count with a
group number outside its range. A requested daisy chain returns `Error::Unsupported`.

A configuration known at build time can be proved valid at build time:

```cpp
--8<-- "tests/docs/configuration.cpp:checked"
```

An invalid `CheckedConfig` does not compile; a compile-fail test checks that.

## Kconfig

Under ESP-IDF, `idf.py menuconfig` shows **ADS9324 16-channel ADC** when the driver is a component of the project
([Installation](installation.md)); the menu is [`_config/Kconfig`](../_config/Kconfig). The options reach the code
only through [`inc/ads9324_kconfig.hpp`](../inc/ads9324_kconfig.hpp), never through a driver signature:

```cpp
--8<-- "tests/docs/configuration.cpp:kconfig"
```

| Option | Default | `Config` field |
| --- | --- | --- |
| `CONFIG_ADS9324_CONVST_HZ` | 100000 | `sampling.convst_hz` |
| `CONFIG_ADS9324_WORD_24_BIT` | n | `interface.length` |
| `CONFIG_ADS9324_APPEND_DIAG` | n | `interface.append_diag` |
| `CONFIG_ADS9324_READY_ON_PIN` | n | `sampling.ready` |
| `CONFIG_ADS9324_READY_TIMEOUT_US` | 10000 | `sampling.ready_timeout_us` |

Without Kconfig (plain CMake, other platforms) `KconfigDefaults()` returns `Config{}`. Both cases are host
tests.

Next: [Porting](porting.md).
