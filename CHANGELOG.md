# Changelog

All notable changes to this repository are listed here. Versions follow [Semantic Versioning](https://semver.org/).
Entries under **Breaking** need source changes in code that used the affected API; any of them calls for a major
version bump (a minor bump before 1.0).

## [Unreleased]

### Added

- ADS9324 driver, first version: start-up and reset with an identity check, the three register banks (the AIN9 to
  AIN16 bank is laid out in reverse order and is mapped per channel), per-channel range, input type, bandwidth,
  offset, gain, phase delay and window, data readout on 1, 2, 4 or 8 lanes with 16- or 24-bit words, the DIAG
  byte, the randomizer and all 15 channel modes, block-average, moving-average and FIR filters, the window
  comparator and the alarm pin, the ADC calibration, reference selection, test patterns, a data-path self-test,
  standby and wake, and a compile-time checked configuration.
- A host example, an ESP-IDF example with GPIO hooks for CONVST, RESET and DRDY, and an independent chip model with
  a knob for every assumption the datasheet leaves open.
- The daisy chain is out of scope: `Validate()` returns `Unsupported` for it (see the Limitations page).
