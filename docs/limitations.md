# Limitations

What the driver does not do, what it assumes where the datasheet is silent or contradicts itself, and what is
not verified on hardware.

<!-- generated: datasheet-refs -->
??? abstract "Datasheet sections this page relies on"

    From Texas Instruments SBASB22 -:

    - Open-wire safe mode: 7.3.7 Open Wire Safe Mode, [p. 30](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=30)
    - Block average, moving average and FIR filters: 7.3.11.2 to 7.3.11.4, [p. 35](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=35)
    - Daisy chain (not supported in v1): 7.3.14.2 Daisy Chain, [p. 43](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=43)
    - Channel registers AIN9 to AIN16 (reversed order): 8.3 AIN9 - AIN16 Channel Registers, [p. 86](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=86)

    All of them, with the driver API for each: [Datasheet map](datasheet.md).
<!-- /generated -->

## Out of scope: the daisy chain

The datasheet's daisy chain (section 7.3.14.2, Figure 7-29) shares CS and SCLK between several ADS9324 and passes
each device's SDOUT to the next one's SDI. The driver leaves it out of version 1 and keeps the seam:

- **Why.** In a chain a register read or write takes 32 SCLK per device instead of 24, so a single driver object
  cannot send one chip's frame; the frame belongs to the whole chain, with the position (`DAISY_CHN_LOC`) and the
  size (`DAISY_CHN_NUM`) programmed in every device. The chain works only with 1, 2 or 4 data lanes on D0 to D7,
  never with the data on SDOUT, which is the driver's default path. Doing it right needs a chain-level object
  and a host model of several chips, and neither exists yet.
- **What happens if you ask.** `Config::daisy` exists, so the layout does not change when the support arrives;
  `Validate()` returns `Error::Unsupported` when `daisy.enabled` is set, `EnsureInitialized()` returns it before any
  I/O, the daisy-chain registers are never written, and `CheckedConfig` does not compile (a compile-fail test
  checks it).
- **What to do instead.** Use one driver object per chip, each with its own chip select; the chips may share SCLK
  and SDI.

## Behaviours to know

- **The randomizer's XOR_BITS_CTL code.** The register table gives 0x00F1; a note in the datasheet says 0x00C1. The
  driver writes 0x00F1 and only that value or 0x0000. If a bench read shows a chip that wants the other
  value, change `reg::kXorBitsRandomizer` and nothing else.
- **Coverage of the datasheet.** [Coverage matrix](coverage-matrix.md) lists every parameter the datasheet specifies
  with the symbol that covers it; rows marked `GAP` are values the driver has no symbol for (limits and delays that
  are documented in [Hardware](hardware.md) only), and `tools/check_matrix.py` fails when a cited symbol is missing.

- **Block average, FIR and `Convert()`.** `Convert()` is one CONVST pulse. With a block-average filter on the
  external clock, or a FIR filter, the chip delivers a result only after the block: `MeasureOnce()` pulses CONVST
  `PulsesPerResult(Filter)` times; with your own timer, pulse that many times and then call `ReadFrame()`.
- **Stuck frames are errors.** A frame of all `0x00` or all `0xFF` is `DataMismatch` by default
  (`Sampling::reject_stuck_frames`): it is what a floating SDOUT reads. Turn it off for a test pattern that really
  produces it.
- **One calibration at a time with a filter off.** `RunAdcCalibration()` refuses to run with a digital filter on.
- **`OrMasked` alarm.** The sense of the ALRM_MASK bits is not verified (A41); the call needs
  `Alarm::or_mask_acknowledged`.
- **Only documented codes.** The reserved register codes are unreachable: the enumerations hold the documented
  values only (for example TP_MODE 1 is not offered, A22).

## Assumptions where the datasheet is silent or contradicts itself

Each row has a knob in the host-test chip model (`tests/mock_ads9324.hpp`, `Knobs`), so the alternatives are
testable. The first column is the number used in the source comments.

| # | Topic | What the driver does |
| --- | --- | --- |
| A1 | SPI mode | Mode 0, derived from the timing diagram: SDI is sampled on the rising edge |
| A2 | td_CKDO of 17 ns against 60 MHz | `kMaxSdoutClockHz` (29 MHz) is the limit for any port that reads SDOUT in mode 0; `kMaxClockHz` is the 60 MHz of the table; examples run at 10 MHz |
| A3 | Settle after reset: 30 ms or 1 ms | Waits 30 ms (the safe one) |
| A4 | RESET pulse width is not given | 1 ms, a design choice |
| A5 | Table 7-18 has no step 5 | Treated as a numbering slip |
| A6 | GEN_CFG1 reachability in the channel banks | Always select the common bank for the command, then the target bank |
| A7 | SDOUT after a non-read frame | The driver never relies on it |
| A8 | Register access during a conversion | Allowed only outside the readout window |
| A9 | Does SDI decode on a data frame | The driver sends zeros while it reads data |
| A10 | Frames shorter or longer than 24 bits | The driver always sends 24 bits |
| A11 | RESET_DETECT reset value: Table 7-13 says 1, Table 8-5 says 0 | Set after a reset, cleared at start-up, either value tolerated |
| A12 | EN_DIAG_FLAG or EN_STATUS_BITS | The register bit name, EN_DIAG_FLAG |
| A13 | XOR "TBD" with 24-bit words | The randomizer is refused with 24-bit words |
| A14 | Order of the PRBS and DIAG bytes | DIAG first, then PRBS |
| A15 | Meaning of the 24-bit low byte | Exposed raw in `Frame::low_byte` |
| A16 | DWC threshold reset values: register 0xFF00 against text 127 and -128 | Thresholds and hysteresis are written before the window is enabled |
| A17 | Glitch filter units | A count of consecutive conversions |
| A18 | DWC_RST edge or level | Write 1, then 0 |
| A19 | Order of the AIN9 to AIN16 bank | Mirrored: local index 15 - channel, from the register table |
| A20 | Flag latch and clear | Cleared with the group reset before use |
| A21 | DWC_EN bit mapping | Follows the bank mapping |
| A22 | TP_MODE codes | Only 0 and 2 are reachable |
| A23 | TP_UPD_MODE | Exposed as the ramp update point |
| A24 | Test-pattern index add or OR | Add, as Table 7-14 shows |
| A25 | Gain equations | `g / 65536`, clamped |
| A26 | Offset sign | Equation 1 (added), over the text "subtracted" |
| A27 | Moving average per channel or common | Common, one value |
| A28 | DOUT_LENGTH 01 | Reserved, unreachable |
| A29 | Phase delay range and unit | 0 to 255 CONVST periods |
| A30 | SYSREF semantics | 0, 1, 0 with at least one CONVST period |
| A31 | AVG_MODE gate for FIR and moving average | The OVR_EN and AVG_MODE pair is always written |
| A32 | DRDY cadence with decimation | One DRDY every N conversions |
| A33 | FIR7 -3 dB printed 0.540 | A documentation slip; no code depends on it |
| A34 | Block-average clock default | The configuration chooses explicitly |
| A35 | INT_TRIG_MODE shared with the single-shot calibration | Restored after the calibration |
| A36 | CALIB_BUSY timing | Wait 125 ms, then poll with a 25 ms margin |
| A37 | Calibration with common-mode correction | Mixed bank types are refused |
| A38 | ADC calibration at power-up | Optional, `Calibration::run_at_start` |
| A39 | CONVST level at the end of the single shot | High |
| A40 | Standby wake time and register retention | Wake waits 30 ms and re-verifies the registers |
| A41 | ALRM_MASK bit map | `OrMasked` needs the acknowledgement |
| A42 | ALRM_DIS and the pulse width | The pulse width is not given |
| A43 | ALRM_POL on DRDY | The driver applies the polarity to every source |
| A44 | Reference switch settle | Waits 30 ms |
| A45 | td_CVDRDY_lo and CH_XTALK_LOW_SPEED | Waits 350 ns; the low-speed bit is a `Config` field |
| A46 | tPL_CV and tPH_CV names swapped | Uses the figures: 50 ns high, 50 ns low at least |
| A47 | Switching characteristics only at IOVDD 1.7 V to 1.9 V | `Validate()` accepts up to 3.6 V |
| A48 | Data-valid window and readout deadline | Read within one conversion period |

Some of the knobs describe the chip's behaviour only in the model (A9, A10, A17, A18, A20, A23, A25, A27, A29, A30,
A32 to A34, A37, A42, A43, A45, A47): they record the assumption and no test depends on the alternative. A real
chip that behaves differently shows up on the bench steps below, not in the host tests.

## Not verified on hardware

Nothing in this driver is verified on a real ADS9324: the host tests run against a model written from the
datasheet. The bench steps:

1. `EnsureInitialized()` returns `Ok` (DEVICE_ID `0x0004`) at your SPI clock; try 1 MHz first. This confirms
   mode 0 (A1) and the reset time (A3).
2. `RunDataPathSelfTest()` passes with the inputs unconnected: the data format, the lanes and the mirrored
   channel mapping.
3. Apply a known voltage to AIN9 and AIN16 and check `microvolts` on the right channel: the mirrored bank (A19).
4. Run `RunAdcCalibration()` and check the code of a grounded input: the calibration timing (A36, A39).
5. With 2, 4 or 8 lanes, the packing of your `CaptureLanes()` against SDOUT mode (the bit order is assumed).
6. The window alert with a pulse alarm and the `OrMasked` source mask (A41).

MISRA C++:2023 CodeQL scanning and the ESP32 builds run in CI only; the repository's local checks run the other
gates ([Testing and CI](testing.md)).
