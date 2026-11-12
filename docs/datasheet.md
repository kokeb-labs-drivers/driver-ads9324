# Datasheet map

The driver follows the ADS9324 datasheet. This page says which revision, where to get it, and where each part of
the driver is described in it, so you can check any behaviour against the source. Section names and page numbers
are the datasheet's own (the printed page numbers).


<!-- generated: datasheet-map -->
- **Datasheet: ADS9324 16-Channel, 16-Bit, 1MSPS, Simultaneous-Sampling SAR ADC With Integrated Analog Front-End**,
  Texas Instruments SBASB22, revision - (2025-12). Get it from the [vendor page](https://www.ti.com/product/ADS9324) or as
  a [PDF](https://www.ti.com/lit/ds/symlink/ads9324.pdf).
  SHA-256 of the file used: `bf4ce5824931912b9d3b2a5450907f8ec445aa3ec1af4418acc0c233d98b8d4c`.

| Topic | Datasheet section | Page | In the driver | Explained on |
| --- | --- | --- | --- | --- |
| Features, channel count and the 16 simultaneous samples | 1 Features; 3 Description | [p. 1](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=1) | `Config`, `reg::kChannelCount` | [ADS9324 Driver](index.md), [Hardware](hardware.md) |
| Pins, package and the data lanes | 4 Pin Configuration and Functions | [p. 3](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=3) | `CtrlPin`, `CaptureLanes()`, `GpioSet()`, `GpioGet()` | [Hardware](hardware.md), [Porting](porting.md), [Quickstart](quickstart.md) |
| Absolute maximum ratings | 5.1 Absolute Maximum Ratings | [p. 5](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=5) | `Supply` (documented limits only, no register) | [Hardware](hardware.md) |
| Supplies, reference and input ranges | 5.3 Recommended Operating Conditions | [p. 6](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=6) | `Supply`, `Validate()`, `RangeMillivolts()` | [Hardware](hardware.md), [Configuration](configuration.md) |
| Electrical characteristics, power-up and reset times | 5.5 Electrical Characteristics | [p. 7](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=7) | `kPowerUpMs`, `kResetMs`, `kInternalVrefMv` | [Hardware](hardware.md), [Contract](contract.md) |
| SPI and CONVST timing | 5.6 Timing Requirements; 5.7 Switching Characteristics | [p. 11](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=11) | `kMaxClockHz`, `kMinCsHighNs`, `kConvstMinHz`, `kConvstMaxHz` | [Porting](porting.md), [Hardware](hardware.md) |
| Timing diagrams, DRDY and the data frame | 5.8 Timing Diagrams | [p. 12](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=12) | `WaitDataReady()`, `ReadFrame()`, `ConversionTimeUs()` | [Porting](porting.md), [Contract](contract.md) |
| Analog inputs and the input clamps | 7.3.1 Analog Inputs; 7.3.2 Input Clamp Protection Circuit | [p. 25](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=25) | `Input`, `Input::type` | [Configuration](configuration.md), [Hardware](hardware.md) |
| PGA, input ranges, common-mode range and bandwidth | 7.3.4 Programmable Gain Amplifier (PGA), Tables 7-1 and 7-2 | [p. 26](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=26) | `Input::range`, `Input::bandwidth`, `RangeMillivolts()` | [Configuration](configuration.md) |
| ADC transfer function and output codes | 7.3.5 ADC Transfer Function | [p. 27](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=27) | `WireToCode()`, `CodeToMicrovolts()`, `Interface::format` | [Configuration](configuration.md), [API guide](api-guide.md) |
| Reference selection | 7.3.6 Reference, Table 7-3 | [p. 28](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=28) | `ReferenceSource`, `SetReference()` | [Configuration](configuration.md), [Hardware](hardware.md) |
| Open-wire safe mode | 7.3.7 Open Wire Safe Mode | [p. 30](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=30) | `InputType::SingleEndedOpenWireSafe` | [Configuration](configuration.md), [Limitations](limitations.md) |
| System offset and gain calibration | 7.3.8 System Offset Calibration; 7.3.9 System Gain Calibration | [p. 31](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=31) | `Correction`, `OffsetCorrectionFor()`, `GainCorrectionFor()` | [Configuration](configuration.md) |
| ADC gain and offset error calibration | 7.3.10 ADC Gain and Offset Error Calibration | [p. 32](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=32) | `RunAdcCalibration()`, `DisableAdcCalibration()`, `IsCalibrationBusy()` | [API guide](api-guide.md), [Contract](contract.md) |
| Digital filters and the system phase calibration | 7.3.11 Digital Filter; 7.3.11.1 System Phase Calibration | [p. 34](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=34) | `Filter`, `Input::phase_delay`, `SetFilter()` | [Configuration](configuration.md), [API guide](api-guide.md) |
| Block average, moving average and FIR filters | 7.3.11.2 to 7.3.11.4 | [p. 35](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=35) | `Filter`, `AverageFactor()`, `FirDecimation()`, `ResultsPerOutput()` | [Configuration](configuration.md), [Limitations](limitations.md) |
| Digital window comparator | 7.3.12 Digital Window Comparator | [p. 39](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=39) | `Window`, `ReadWindowFlags()`, `ClearWindowFlags()` | [API guide](api-guide.md), [Configuration](configuration.md) |
| Alarm modes and the DRDY/ALARM pin | 7.3.13 Alarm Modes, Table 7-10 | [p. 41](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=41) | `Alarm`, `SetAlarm()`, `ReadAlarmPin()` | [API guide](api-guide.md), [Configuration](configuration.md), [Hardware](hardware.md) |
| Data interface: lanes, word length and channel modes | 7.3.14 Data Interface; 7.3.14.1 ADC Channel Modes, Table 7-12 | [p. 42](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=42) | `Interface`, `ChannelSelect`, `FrameChannels()`, `FrameBytes()`, `SetChannels()` | [Configuration](configuration.md), [Architecture](architecture.md), [Porting](porting.md) |
| Daisy chain (not supported in v1) | 7.3.14.2 Daisy Chain | [p. 43](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=43) | `DaisyChain`, `Error::Unsupported` | [Limitations](limitations.md) |
| Diagnostic byte and the randomizer | 7.3.14.3 Diagnostic Flags; 7.3.14.4 ADC Output Data Randomizer | [p. 44](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=44) | `Interface::append_diag`, `Interface::randomize`, `DerandomizeMask()` | [Configuration](configuration.md), [Contract](contract.md) |
| Test patterns | 7.3.14.5 Test Patterns for Data Interface, Table 7-14 | [p. 47](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=47) | `TestPattern`, `SetTestPattern()`, `RunDataPathSelfTest()` | [API guide](api-guide.md), [Testing and CI](testing.md) |
| Output drive strength and output delay | 7.3.14.6 Digital Output Drive Strength Control; 7.3.14.7 Digital Output Delay Adjustment | [p. 48](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=48) | `Interface::delay_ns`, `Interface::drive` | [Configuration](configuration.md), [Porting](porting.md) |
| Reset | 7.4.1 Reset | [p. 49](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=49) | `Reset()`, `CtrlPin::Reset`, `kResetPulseUs` | [Hardware](hardware.md), [Troubleshooting](troubleshooting.md) |
| Normal operation and standby | 7.4.2 Normal Operation; 7.4.3 Standby Mode | [p. 49](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=49) | `PowerDown()`, `Wake()`, `IsPoweredDown()` | [API guide](api-guide.md), [Contract](contract.md) |
| Register write and read frames | 7.5 Programming; 7.5.1 Register Write Operation; 7.5.2 Register Read Operation | [p. 50](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=50) | `SpiInterface`, `detail::RegisterBus`, `ReadRegister()` | [Architecture](architecture.md), [Porting](porting.md) |
| Initialization example, single lane on SDOUT | 7.5.3 Initialization Example - Single Lane Mode on SDOUT | [p. 51](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=51) | `EnsureInitialized()`, `Config{}` | [Quickstart](quickstart.md), [Architecture](architecture.md) |
| Common registers | 8.1 ADS93xx Common Registers | [p. 53](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=53) | `inc/ads9324_registers.hpp`, `Identify()`, `Error::DeviceIdMismatch` | [Architecture](architecture.md), [Configuration](configuration.md) |
| Channel registers AIN1 to AIN8 | 8.2 AIN1 - AIN8 Channel Registers | [p. 64](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=64) | `reg::Locate()`, `SetInput()` | [Architecture](architecture.md), [Configuration](configuration.md) |
| Channel registers AIN9 to AIN16 (reversed order) | 8.3 AIN9 - AIN16 Channel Registers | [p. 86](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=86) | `reg::Locate()`, `reg::ChannelAt()` | [Architecture](architecture.md), [Limitations](limitations.md) |
| Typical application and input filter | 9.2 Typical Application, Figures 9-1 and 9-2 | [p. 106](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=106) | - | [Hardware](hardware.md) |
| Power-supply decoupling | 9.3 Power Supply Recommendations, Figure 9-4 | [p. 109](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=109) | `Supply` | [Hardware](hardware.md) |
| Layout | 9.4 Layout | [p. 109](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=109) | - | [Hardware](hardware.md) |
| Package outline and ordering | 12 Mechanical, Packaging, and Orderable Information | [p. 112](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=112) | - | [Hardware](hardware.md) |
<!-- /generated -->

## When the datasheet and the driver differ

The driver is written against the revision above. If you have a newer revision, compare its revision history with
the topics above; a change to a pin, a register, a timing or a value the driver uses is a bug report for this
repository. Where the driver deliberately goes beyond the datasheet, the [Contract](contract.md) says so.

The repository never contains the PDF itself: vendors own their documents. Download it from the vendor, and compare
its SHA-256 with the one above to confirm you have the same file.
