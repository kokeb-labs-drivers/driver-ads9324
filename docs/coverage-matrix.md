# Coverage matrix

One row for each parameter, register field, mode, pin and timing the data sheet specifies. The **API** column names
the symbol of this driver that covers the row. `tools/check_matrix.py` (part of `tools/preflight.sh`) fails when a
symbol cited here is not in `inc/`, `src/` or `docs/hardware.yml` (a symbol that appears only in a comment does not
count), when a test id does not start the name of a `TEST_CASE` in `tests/`, when a row is malformed, or when the
counts below are wrong. The Test column names a topic: every test case whose name starts with that id belongs to
it, so the table shows where a row is tested, not which assertion proves it. A row the driver does not cover has
the status `GAP` and says so in the API column.

Status: `OK` covered and tested, `GUARD` the driver restricts the values to the documented ones, `TODO` the data sheet
leaves it open or the feature is out of scope for this version, `INFO` a fact with no driver behaviour, `GAP` the
data sheet value has no symbol in the driver.

Total rows 873: OK 675, GUARD 82, TODO 10, INFO 80, GAP 26.

## Registers (116 rows)

| Id | Item | Reference | API | Test | Status |
| --- | --- | --- | --- | --- | --- |
| REG-1-01 | common bank 0x01 GEN_CFG1 [reset 0x0000] | S8.1.1 p53 | internal: read command (`reg::kRegRdEn`, `GenCfg1ReadAddress`) and `Reset(Software)`; never in the shadow | T04,T06 | OK |
| REG-1-02 | common bank 0x02 BANK_SEL [reset 0x0001] | S8.1.2 p53 | internal bank selector, values 1, 2, 4 only; always accessible | T04,T05 | OK |
| REG-1-07 | common bank 0x07 DIAG_CTRL [reset 0x0000] | S8.1.3 p54 | start-up clears the flag; `Frame::reset_detected`, `Error::UnexpectedReset` | T06,T24 | OK |
| REG-1-08 | common bank 0x08 PDN_CTL [reset 0x0000] | S8.1.4 p54 | `PowerDown()`, `Wake()` | T22 | OK |
| REG-1-09 | common bank 0x09 GEN_CFG2 [reset 0x0000] | S8.1.5 p54 | daisy chain fields: out of scope in v1, register never written (stays 0) | T02,T23 | TODO |
| REG-1-0A | common bank 0x0A GEN_CFG3 [reset 0x0000] | S8.1.6 p55 | `Interface` (`format`, `randomize`, `append_diag`, `lanes`, `length`, `data_on_sdout`) | T10,T12 | OK |
| REG-1-0B | common bank 0x0B XOR_BITS_CTL [reset 0x0000] | S8.1.7 p55 | `Interface::randomize` writes 0x00F1 (the register table) or 0x0000 only; a data sheet note says 0x00C1 (docs/limitations.md) | T10 | GUARD |
| REG-1-0C | common bank 0x0C DRDY_ALARM_SEL [reset 0x0000] | S8.1.8 p56 | `Alarm` (`function`, `active_low`, `pulse`, `sources` guarded by `or_mask_acknowledged`) | T19 | OK |
| REG-1-0D | common bank 0x0D GEN_CFG4 [reset 0x0000] | S8.1.9 p56 | `Alarm.disabled` (guarded), `Interface.drive`, `Interface.delay_ns` | T19,T21 | OK |
| REG-1-0E | common bank 0x0E DIG_DELAY_CFG1 [reset 0x0000] | S8.1.10 p57 | `Interface.delay_ns` (SDOUT, D3..D0), codes 0..5 | T21 | OK |
| REG-1-0F | common bank 0x0F DIG_DELAY_CFG2 [reset 0x0000] | S8.1.11 p58 | `Interface.delay_ns` (D7..D4), codes 0..5 | T21 | OK |
| REG-1-10 | common bank 0x10 ANA_CFG1 [reset 0x0000] | S8.1.12 p58 | `Reference` (Table 7-3) | T07 | OK |
| REG-1-11 | common bank 0x11 ANA_CFG2 [reset 0x0000] | S8.1.13 p59 | `ChannelSelect`, `Config.low_speed_crosstalk` | T11 | OK |
| REG-1-12 | common bank 0x12 ADC_CAL [reset 0x0000] | S8.1.14 p59 | `RunAdcCalibration`, `DisableAdcCalibration` | T15 | OK |
| REG-1-14 | common bank 0x14 DIG_FILTER [reset 0x0000] | S8.1.15 p59 | `Filter` (`SetFilter`), `SetInput`, `Input::phase_delay` enable, internal SYSREF pulse | T16,T17 | OK |
| REG-1-15 | common bank 0x15 GEN_CFG5 [reset 0x0000] | S8.1.16 p60 | common: internal AVG_MODE order for filters; the T_MODE bits are never written | T16,T03 | OK |
| REG-1-1D | common bank 0x1D DEVICE_STATUS [reset 0x0000] | S8.1.17 p61 | `IsCalibrationBusy` | T15 | OK |
| REG-1-21 | common bank 0x21 DEVICE_ID [reset 0x0004] | S8.1.18 p61 | `Identify()`: must read 0x0004 | T06 | OK |
| REG-2-08 | AIN1-8 bank 0x08 PGA_CONFIG_AIN1_2 [reset 0x0000] | S8.2.1 p64 | `Input` per channel (`type`, `range`, `cm_correction`) | T08 | OK |
| REG-2-09 | AIN1-8 bank 0x09 PGA_CONFIG_AIN3_4 [reset 0x0000] | S8.2.2 p65 | `Input` per channel (`type`, `range`, `cm_correction`) | T08 | OK |
| REG-2-0A | AIN1-8 bank 0x0A PGA_CONFIG_AIN5_6 [reset 0x0000] | S8.2.3 p65 | `Input` per channel (`type`, `range`, `cm_correction`) | T08 | OK |
| REG-2-0B | AIN1-8 bank 0x0B PGA_CONFIG_AIN7_8 [reset 0x0000] | S8.2.4 p66 | `Input` per channel (`type`, `range`, `cm_correction`) | T08 | OK |
| REG-2-0C | AIN1-8 bank 0x0C PGA_BW_SEL_AIN1_8 [reset 0x0000] | S8.2.5 p67 | `Input.bandwidth` per channel | T08 | OK |
| REG-2-0D | AIN1-8 bank 0x0D PHASE_DELAY_AIN1_2 [reset 0x0000] | S8.2.6 p68 | `SetInput`, `Input::phase_delay` | T17 | OK |
| REG-2-0E | AIN1-8 bank 0x0E PHASE_DELAY_AIN3_4 [reset 0x0000] | S8.2.7 p68 | `SetInput`, `Input::phase_delay` | T17 | OK |
| REG-2-0F | AIN1-8 bank 0x0F PHASE_DELAY_AIN5_6 [reset 0x0000] | S8.2.8 p68 | `SetInput`, `Input::phase_delay` | T17 | OK |
| REG-2-10 | AIN1-8 bank 0x10 PHASE_DELAY_AIN7_8 [reset 0x0000] | S8.2.9 p69 | `SetInput`, `Input::phase_delay` | T17 | OK |
| REG-2-11 | AIN1-8 bank 0x11 OFS_AIN1 [reset 0x0000] | S8.2.10 p69 | `SetInput`, `Input::offset_quarter_lsb` | T14 | OK |
| REG-2-12 | AIN1-8 bank 0x12 OFS_AIN2 [reset 0x0000] | S8.2.11 p69 | `SetInput`, `Input::offset_quarter_lsb` | T14 | OK |
| REG-2-13 | AIN1-8 bank 0x13 OFS_AIN3 [reset 0x0000] | S8.2.12 p70 | `SetInput`, `Input::offset_quarter_lsb` | T14 | OK |
| REG-2-14 | AIN1-8 bank 0x14 OFS_AIN4 [reset 0x0000] | S8.2.13 p70 | `SetInput`, `Input::offset_quarter_lsb` | T14 | OK |
| REG-2-15 | AIN1-8 bank 0x15 OFS_AIN5 [reset 0x0000] | S8.2.14 p70 | `SetInput`, `Input::offset_quarter_lsb` | T14 | OK |
| REG-2-16 | AIN1-8 bank 0x16 OFS_AIN6 [reset 0x0000] | S8.2.15 p71 | `SetInput`, `Input::offset_quarter_lsb` | T14 | OK |
| REG-2-17 | AIN1-8 bank 0x17 OFS_AIN7 [reset 0x0000] | S8.2.16 p71 | `SetInput`, `Input::offset_quarter_lsb` | T14 | OK |
| REG-2-18 | AIN1-8 bank 0x18 OFS_AIN8 [reset 0x0000] | S8.2.17 p71 | `SetInput`, `Input::offset_quarter_lsb` | T14 | OK |
| REG-2-19 | AIN1-8 bank 0x19 GAN_AIN1 [reset 0x0000] | S8.2.18 p72 | `SetInput`, `Input::gain` | T14 | OK |
| REG-2-1A | AIN1-8 bank 0x1A GAN_AIN2 [reset 0x0000] | S8.2.19 p72 | `SetInput`, `Input::gain` | T14 | OK |
| REG-2-1B | AIN1-8 bank 0x1B GAN_AIN3 [reset 0x0000] | S8.2.20 p72 | `SetInput`, `Input::gain` | T14 | OK |
| REG-2-1C | AIN1-8 bank 0x1C GAN_AIN4 [reset 0x0000] | S8.2.21 p73 | `SetInput`, `Input::gain` | T14 | OK |
| REG-2-1D | AIN1-8 bank 0x1D GAN_AIN5 [reset 0x0000] | S8.2.22 p73 | `SetInput`, `Input::gain` | T14 | OK |
| REG-2-1E | AIN1-8 bank 0x1E GAN_AIN6 [reset 0x0000] | S8.2.23 p74 | `SetInput`, `Input::gain` | T14 | OK |
| REG-2-1F | AIN1-8 bank 0x1F GAN_AIN7 [reset 0x0000] | S8.2.24 p74 | `SetInput`, `Input::gain` | T14 | OK |
| REG-2-20 | AIN1-8 bank 0x20 GAN_AIN8 [reset 0x0000] | S8.2.25 p74 | `SetInput`, `Input::gain` | T14 | OK |
| REG-2-21 | AIN1-8 bank 0x21 DWC_CFG [reset 0x0000] | S8.2.26 p75 | `Window.enabled`, `Config::window_glitch`, `ClearWindowFlags` | T18 | OK |
| REG-2-22 | AIN1-8 bank 0x22 DWC_TH_AIN1 [reset 0xFF00] | S8.2.27 p75 | `Window.high_code`, `Window.low_code` | T18 | OK |
| REG-2-23 | AIN1-8 bank 0x23 DWC_TH_AIN2 [reset 0xFF00] | S8.2.28 p76 | `Window.high_code`, `Window.low_code` | T18 | OK |
| REG-2-24 | AIN1-8 bank 0x24 DWC_TH_AIN3 [reset 0xFF00] | S8.2.29 p76 | `Window.high_code`, `Window.low_code` | T18 | OK |
| REG-2-25 | AIN1-8 bank 0x25 DWC_TH_AIN4 [reset 0xFF00] | S8.2.30 p76 | `Window.high_code`, `Window.low_code` | T18 | OK |
| REG-2-26 | AIN1-8 bank 0x26 DWC_TH_AIN5 [reset 0xFF00] | S8.2.31 p77 | `Window.high_code`, `Window.low_code` | T18 | OK |
| REG-2-27 | AIN1-8 bank 0x27 DWC_TH_AIN6 [reset 0xFF00] | S8.2.32 p77 | `Window.high_code`, `Window.low_code` | T18 | OK |
| REG-2-28 | AIN1-8 bank 0x28 DWC_TH_AIN7 [reset 0xFF00] | S8.2.33 p77 | `Window.high_code`, `Window.low_code` | T18 | OK |
| REG-2-29 | AIN1-8 bank 0x29 DWC_TH_AIN8 [reset 0xFF00] | S8.2.34 p78 | `Window.high_code`, `Window.low_code` | T18 | OK |
| REG-2-2A | AIN1-8 bank 0x2A DWC_HYS_AIN1_2 [reset 0x0000] | S8.2.35 p78 | `Window.hysteresis` | T18 | OK |
| REG-2-2B | AIN1-8 bank 0x2B DWC_HYS_AIN3_4 [reset 0xFF00] | S8.2.36 p78 | `Window.hysteresis` | T18 | OK |
| REG-2-2C | AIN1-8 bank 0x2C DWC_HYS_AIN5_6 [reset 0xFF00] | S8.2.37 p79 | `Window.hysteresis` | T18 | OK |
| REG-2-2D | AIN1-8 bank 0x2D DWC_HYS_AIN7_8 [reset 0xFF00] | S8.2.38 p79 | `Window.hysteresis` | T18 | OK |
| REG-2-2E | AIN1-8 bank 0x2E TP_CFG [reset 0x0000] | S8.2.39 p79 | `SetTestPattern` | T20 | OK |
| REG-2-2F | AIN1-8 bank 0x2F TP_AIN1 [reset 0x0000] | S8.2.40 p80 | `TestPattern::value` (the first value is the ramp step) | T20 | OK |
| REG-2-30 | AIN1-8 bank 0x30 TP_AIN2 [reset 0x0000] | S8.2.41 p80 | `TestPattern::value` (the first value is the ramp step) | T20 | OK |
| REG-2-31 | AIN1-8 bank 0x31 TP_AIN3 [reset 0x0000] | S8.2.42 p80 | `TestPattern::value` (the first value is the ramp step) | T20 | OK |
| REG-2-32 | AIN1-8 bank 0x32 TP_AIN4 [reset 0x0000] | S8.2.43 p81 | `TestPattern::value` (the first value is the ramp step) | T20 | OK |
| REG-2-33 | AIN1-8 bank 0x33 TP_AIN5 [reset 0x0000] | S8.2.44 p81 | `TestPattern::value` (the first value is the ramp step) | T20 | OK |
| REG-2-34 | AIN1-8 bank 0x34 TP_AIN6 [reset 0x0000] | S8.2.45 p81 | `TestPattern::value` (the first value is the ramp step) | T20 | OK |
| REG-2-35 | AIN1-8 bank 0x35 TP_AIN7 [reset 0x0000] | S8.2.46 p82 | `TestPattern::value` (the first value is the ramp step) | T20 | OK |
| REG-2-36 | AIN1-8 bank 0x36 TP_AIN8 [reset 0x0000] | S8.2.47 p82 | `TestPattern::value` (the first value is the ramp step) | T20 | OK |
| REG-2-37 | AIN1-8 bank 0x37 GEN_CFG5 [reset 0x0000] | S8.2.48 p82 | `Correction` (offset, gain enable per group) | T14 | OK |
| REG-2-3E | AIN1-8 bank 0x3E DWC_FLAG_AIN1_8 [reset 0x0000] | S8.2.49 p83 | `ReadWindowFlags()` | T18 | OK |
| REG-3-08 | AIN9-16 bank 0x08 PGA_CONFIG_AIN15_16 [reset 0x0000] | S8.3.1 p86 | `Input` per channel (`type`, `range`, `cm_correction`) | T08 | OK |
| REG-3-09 | AIN9-16 bank 0x09 PGA_CONFIG_AIN13_14 [reset 0x0000] | S8.3.2 p87 | `Input` per channel (`type`, `range`, `cm_correction`) | T08 | OK |
| REG-3-0A | AIN9-16 bank 0x0A PGA_CONFIG_AIN11_12 [reset 0x0000] | S8.3.3 p88 | `Input` per channel (`type`, `range`, `cm_correction`) | T08 | OK |
| REG-3-0B | AIN9-16 bank 0x0B PGA_CONFIG_AIN9_10 [reset 0x0000] | S8.3.4 p88 | `Input` per channel (`type`, `range`, `cm_correction`) | T08 | OK |
| REG-3-0C | AIN9-16 bank 0x0C PGA_BW_SEL_AIN9_16 [reset 0x0000] | S8.3.5 p89 | `Input.bandwidth` per channel | T08 | OK |
| REG-3-0D | AIN9-16 bank 0x0D PHASE_DELAY_AIN15_16 [reset 0x0000] | S8.3.6 p90 | `SetInput`, `Input::phase_delay` | T17 | OK |
| REG-3-0E | AIN9-16 bank 0x0E PHASE_DELAY_AIN13_14 [reset 0x0000] | S8.3.7 p90 | `SetInput`, `Input::phase_delay` | T17 | OK |
| REG-3-0F | AIN9-16 bank 0x0F PHASE_DELAY_AIN11_12 [reset 0x0000] | S8.3.8 p90 | `SetInput`, `Input::phase_delay` | T17 | OK |
| REG-3-10 | AIN9-16 bank 0x10 PHASE_DELAY_AIN9_10 [reset 0x0000] | S8.3.9 p91 | `SetInput`, `Input::phase_delay` | T17 | OK |
| REG-3-11 | AIN9-16 bank 0x11 OFS_AIN16 [reset 0x0000] | S8.3.10 p91 | `SetInput`, `Input::offset_quarter_lsb` | T14 | OK |
| REG-3-12 | AIN9-16 bank 0x12 OFS_AIN15 [reset 0x0000] | S8.3.11 p91 | `SetInput`, `Input::offset_quarter_lsb` | T14 | OK |
| REG-3-13 | AIN9-16 bank 0x13 OFS_AIN14 [reset 0x0000] | S8.3.12 p92 | `SetInput`, `Input::offset_quarter_lsb` | T14 | OK |
| REG-3-14 | AIN9-16 bank 0x14 OFS_AIN13 [reset 0x0000] | S8.3.13 p92 | `SetInput`, `Input::offset_quarter_lsb` | T14 | OK |
| REG-3-15 | AIN9-16 bank 0x15 OFS_AIN12 [reset 0x0000] | S8.3.14 p92 | `SetInput`, `Input::offset_quarter_lsb` | T14 | OK |
| REG-3-16 | AIN9-16 bank 0x16 OFS_AIN11 [reset 0x0000] | S8.3.15 p93 | `SetInput`, `Input::offset_quarter_lsb` | T14 | OK |
| REG-3-17 | AIN9-16 bank 0x17 OFS_AIN10 [reset 0x0000] | S8.3.16 p93 | `SetInput`, `Input::offset_quarter_lsb` | T14 | OK |
| REG-3-18 | AIN9-16 bank 0x18 OFS_AIN9 [reset 0x0000] | S8.3.17 p93 | `SetInput`, `Input::offset_quarter_lsb` | T14 | OK |
| REG-3-19 | AIN9-16 bank 0x19 GAN_AIN16 [reset 0x0000] | S8.3.18 p94 | `SetInput`, `Input::gain` | T14 | OK |
| REG-3-1A | AIN9-16 bank 0x1A GAN_AIN15 [reset 0x0000] | S8.3.19 p94 | `SetInput`, `Input::gain` | T14 | OK |
| REG-3-1B | AIN9-16 bank 0x1B GAN_AIN14 [reset 0x0000] | S8.3.20 p95 | `SetInput`, `Input::gain` | T14 | OK |
| REG-3-1C | AIN9-16 bank 0x1C GAN_AIN13 [reset 0x0000] | S8.3.21 p95 | `SetInput`, `Input::gain` | T14 | OK |
| REG-3-1D | AIN9-16 bank 0x1D GAN_AIN12 [reset 0x0000] | S8.3.22 p95 | `SetInput`, `Input::gain` | T14 | OK |
| REG-3-1E | AIN9-16 bank 0x1E GAN_AIN11 [reset 0x0000] | S8.3.23 p96 | `SetInput`, `Input::gain` | T14 | OK |
| REG-3-1F | AIN9-16 bank 0x1F GAN_AIN10 [reset 0x0000] | S8.3.24 p96 | `SetInput`, `Input::gain` | T14 | OK |
| REG-3-20 | AIN9-16 bank 0x20 GAN_AIN9 [reset 0x0000] | S8.3.25 p96 | `SetInput`, `Input::gain` | T14 | OK |
| REG-3-21 | AIN9-16 bank 0x21 DWC_CFG [reset 0x0000] | S8.3.26 p97 | `Window.enabled`, `Config::window_glitch`, `ClearWindowFlags` | T18 | OK |
| REG-3-22 | AIN9-16 bank 0x22 DWC_TH_AIN16 [reset 0xFF00] | S8.3.27 p97 | `Window.high_code`, `Window.low_code` | T18 | OK |
| REG-3-23 | AIN9-16 bank 0x23 DWC_TH_AIN15 [reset 0xFF00] | S8.3.28 p98 | `Window.high_code`, `Window.low_code` | T18 | OK |
| REG-3-24 | AIN9-16 bank 0x24 DWC_TH_AIN14 [reset 0xFF00] | S8.3.29 p98 | `Window.high_code`, `Window.low_code` | T18 | OK |
| REG-3-25 | AIN9-16 bank 0x25 DWC_TH_AIN13 [reset 0xFF00] | S8.3.30 p98 | `Window.high_code`, `Window.low_code` | T18 | OK |
| REG-3-26 | AIN9-16 bank 0x26 DWC_TH_AIN12 [reset 0xFF00] | S8.3.31 p99 | `Window.high_code`, `Window.low_code` | T18 | OK |
| REG-3-27 | AIN9-16 bank 0x27 DWC_TH_AIN11 [reset 0xFF00] | S8.3.32 p99 | `Window.high_code`, `Window.low_code` | T18 | OK |
| REG-3-28 | AIN9-16 bank 0x28 DWC_TH_AIN10 [reset 0xFF00] | S8.3.33 p99 | `Window.high_code`, `Window.low_code` | T18 | OK |
| REG-3-29 | AIN9-16 bank 0x29 DWC_TH_AIN9 [reset 0xFF00] | S8.3.34 p100 | `Window.high_code`, `Window.low_code` | T18 | OK |
| REG-3-2A | AIN9-16 bank 0x2A DWC_HYS_AIN15_16 [reset 0x0000] | S8.3.35 p100 | `Window.hysteresis` | T18 | OK |
| REG-3-2B | AIN9-16 bank 0x2B DWC_HYS_AIN13_14 [reset 0xFF00] | S8.3.36 p100 | `Window.hysteresis` | T18 | OK |
| REG-3-2C | AIN9-16 bank 0x2C DWC_HYS_AIN11_12 [reset 0xFF00] | S8.3.37 p101 | `Window.hysteresis` | T18 | OK |
| REG-3-2D | AIN9-16 bank 0x2D DWC_HYS_AIN9_10 [reset 0xFF00] | S8.3.38 p101 | `Window.hysteresis` | T18 | OK |
| REG-3-2E | AIN9-16 bank 0x2E TP_CFG [reset 0x0000] | S8.3.39 p101 | `SetTestPattern` | T20 | OK |
| REG-3-2F | AIN9-16 bank 0x2F TP_AIN16 [reset 0x0000] | S8.3.40 p102 | `TestPattern::value` (the first value is the ramp step) | T20 | OK |
| REG-3-30 | AIN9-16 bank 0x30 TP_AIN15 [reset 0x0000] | S8.3.41 p102 | `TestPattern::value` (the first value is the ramp step) | T20 | OK |
| REG-3-31 | AIN9-16 bank 0x31 TP_AIN14 [reset 0x0000] | S8.3.42 p103 | `TestPattern::value` (the first value is the ramp step) | T20 | OK |
| REG-3-32 | AIN9-16 bank 0x32 TP_AIN13 [reset 0x0000] | S8.3.43 p103 | `TestPattern::value` (the first value is the ramp step) | T20 | OK |
| REG-3-33 | AIN9-16 bank 0x33 TP_AIN12 [reset 0x0000] | S8.3.44 p103 | `TestPattern::value` (the first value is the ramp step) | T20 | OK |
| REG-3-34 | AIN9-16 bank 0x34 TP_AIN11 [reset 0x0000] | S8.3.45 p103 | `TestPattern::value` (the first value is the ramp step) | T20 | OK |
| REG-3-35 | AIN9-16 bank 0x35 TP_AIN10 [reset 0x0000] | S8.3.46 p104 | `TestPattern::value` (the first value is the ramp step) | T20 | OK |
| REG-3-36 | AIN9-16 bank 0x36 TP_AIN9 [reset 0x0000] | S8.3.47 p104 | `TestPattern::value` (the first value is the ramp step) | T20 | OK |
| REG-3-37 | AIN9-16 bank 0x37 GEN_CFG5 [reset 0x0000] | S8.3.48 p104 | `Correction` (offset, gain enable per group) | T14 | OK |
| REG-3-3E | AIN9-16 bank 0x3E DWC_FLAG_AIN9_16 [reset 0x0000] | S8.3.49 p105 | `ReadWindowFlags()` | T18 | OK |

## Register bit fields (385 rows)

| Id | Item | Reference | API | Test | Status |
| --- | --- | --- | --- | --- | --- |
| FLD-1-01-15_8 | common 0x01 GEN_CFG1 [15:8] REG_RD_ADD[7:0] (R/W) | S8.1.1 p53 | internal: set by every register read; not user-visible | T04 | OK |
| FLD-1-01-7_2 | common 0x01 GEN_CFG1 [7:2] RESERVED (R/W) | S8.1.1 p53 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-1-01-1 | common 0x01 GEN_CFG1 [1] SW_RST (R/W) | S8.1.1 p53 | `Reset(ResetKind::Software)`; always written 0 otherwise | T06 | OK |
| FLD-1-01-0 | common 0x01 GEN_CFG1 [0] REG_RD_EN (R/W) | S8.1.1 p53 | internal: 1 in a read command, 0 in every other write | T04 | OK |
| FLD-1-02-15_3 | common 0x02 BANK_SEL [15:3] RESERVED (R/W) | S8.1.2 p53 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-1-02-2_0 | common 0x02 BANK_SEL [2:0] BANK_SEL[2:0] (R/W) | S8.1.2 p53 | internal: 1, 2 or 4 only; re-selected after any bus error | T04,T05 | OK |
| FLD-1-07-15_9 | common 0x07 DIAG_CTRL [15:9] RESERVED (R) | S8.1.3 p54 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-1-07-8 | common 0x07 DIAG_CTRL [8] RESET_DETECT_FLAG (R/W) | S8.1.3 p54 | start-up clears the flag; `Frame::reset_detected`; `Frame.diag` bit 0 gives `UnexpectedReset` | T06,T24 | OK |
| FLD-1-07-7_0 | common 0x07 DIAG_CTRL [7:0] RESERVED (R) | S8.1.3 p54 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-1-08-15_1 | common 0x08 PDN_CTL [15:1] RESERVED (R/W) | S8.1.4 p54 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-1-08-0 | common 0x08 PDN_CTL [0] DEVICE_PDN (R/W) | S8.1.4 p54 | `PowerDown()` writes 1, `Wake()` writes 0 | T22 | OK |
| FLD-1-09-15_12 | common 0x09 GEN_CFG2 [15:12] RESERVED (R/W) | S8.1.5 p54 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-1-09-11_8 | common 0x09 GEN_CFG2 [11:8] DAISY_CHN_LOC[3:0] (R/W) | S8.1.5 p54 | out of scope in v1: never written (0) | T23 | TODO |
| FLD-1-09-7_4 | common 0x09 GEN_CFG2 [7:4] DAISY_CHN_NUM_DEV[3:0] (R/W) | S8.1.5 p54 | out of scope in v1: never written (0) | T23 | TODO |
| FLD-1-09-3_1 | common 0x09 GEN_CFG2 [3:1] RESERVED (R/W) | S8.1.5 p54 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-1-09-0 | common 0x09 GEN_CFG2 [0] DAISY_CHN_EN (R/W) | S8.1.5 p54 | out of scope in v1: never written (0); `Config.daisy` refused by `Validate` | T23 | TODO |
| FLD-1-0A-15_13 | common 0x0A GEN_CFG3 [15:13] RESERVED (R/W) | S8.1.6 p55 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-1-0A-12 | common 0x0A GEN_CFG3 [12] EN_OFS_BINARY (R/W) | S8.1.6 p55 | `Interface.format` (`TwosComplement`, `OffsetBinary`) | T10 | OK |
| FLD-1-0A-11_10 | common 0x0A GEN_CFG3 [11:10] RESERVED (R/W) | S8.1.6 p55 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-1-0A-9 | common 0x0A GEN_CFG3 [9] EN_XOR_PATT (R/W) | S8.1.6 p55 | `Interface.randomize` (with XOR_BITS_CTL); refused with 24-bit words | T10 | GUARD |
| FLD-1-0A-8 | common 0x0A GEN_CFG3 [8] EN_DIAG_FLAG (R/W) | S8.1.6 p55 | `Interface.append_diag` | T10 | OK |
| FLD-1-0A-7_6 | common 0x0A GEN_CFG3 [7:6] RESERVED (R/W) | S8.1.6 p55 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-1-0A-5_4 | common 0x0A GEN_CFG3 [5:4] DOUT_LANE_SEL[1:0] (R/W) | S8.1.6 p55 | `Interface.lanes` (One, Two, Four, Eight) | T12 | OK |
| FLD-1-0A-3_2 | common 0x0A GEN_CFG3 [3:2] DOUT_LENGTH[1:0] (R/W) | S8.1.6 p55 | `Interface.length` (Bits16, Bits24); 01b unreachable | T10 | GUARD |
| FLD-1-0A-1 | common 0x0A GEN_CFG3 [1] ADC_DATA_SDOUT_EN (R/W) | S8.1.6 p55 | `Interface.data_on_sdout` (1-lane only) | T12 | OK |
| FLD-1-0A-0 | common 0x0A GEN_CFG3 [0] RESERVED (R/W) | S8.1.6 p55 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-1-0B-15_8 | common 0x0B XOR_BITS_CTL [15:8] RESERVED (R/W) | S8.1.7 p55 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-1-0B-7_6 | common 0x0B XOR_BITS_CTL [7:6] XOR_MODE[1:0] (R/W) | S8.1.7 p55 | fixed 3 by `Interface.randomize`; no other value reachable | T10 | GUARD |
| FLD-1-0B-5_4 | common 0x0B XOR_BITS_CTL [5:4] NUM_XOR_BITS[1:0] (R/W) | S8.1.7 p55 | fixed 3 by `Interface.randomize`; no other value reachable | T10 | GUARD |
| FLD-1-0B-3_1 | common 0x0B XOR_BITS_CTL [3:1] RESERVED (R/W) | S8.1.7 p55 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-1-0B-0 | common 0x0B XOR_BITS_CTL [0] XOR_BIT_SEL (R/W) | S8.1.7 p55 | fixed 1 by `Interface.randomize`; no other value reachable | T10 | GUARD |
| FLD-1-0C-15_8 | common 0x0C DRDY_ALARM_SEL [15:8] ALRM_MASK[7:0] (R/W) | S8.1.8 p56 | `Alarm::sources`, only with `or_mask_acknowledged`; bit meaning undocumented (A41) | T19 | TODO |
| FLD-1-0C-7_6 | common 0x0C DRDY_ALARM_SEL [7:6] RESERVED (R/W) | S8.1.8 p56 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-1-0C-5 | common 0x0C DRDY_ALARM_SEL [5] ALRM_TYPE (R/W) | S8.1.8 p56 | `Alarm.pulse` (window alert only) | T19 | OK |
| FLD-1-0C-4 | common 0x0C DRDY_ALARM_SEL [4] ALRM_POL (R/W) | S8.1.8 p56 | `Alarm.active_low`; driver applies it when reading the pin | T19 | OK |
| FLD-1-0C-3_0 | common 0x0C DRDY_ALARM_SEL [3:0] DRDY_ALRM_SEL[3:0] (R/W) | S8.1.8 p56 | `Alarm.function` (Drdy 0, WindowAlert 1, CalDone 6, OrMasked 8); other codes unreachable | T19 | GUARD |
| FLD-1-0D-15_6 | common 0x0D GEN_CFG4 [15:6] RESERVED (R/W) | S8.1.9 p56 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-1-0D-5 | common 0x0D GEN_CFG4 [5] ALRM_DIS (R/W) | S8.1.9 p56 | `Alarm.disabled`, refused while the ready source is the pin | T19 | GUARD |
| FLD-1-0D-4 | common 0x0D GEN_CFG4 [4] RESERVED (R/W) | S8.1.9 p56 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-1-0D-3 | common 0x0D GEN_CFG4 [3] DIG_DELAY_EN (R/W) | S8.1.9 p56 | set when any `Interface.delay_ns` is non-zero | T21 | OK |
| FLD-1-0D-2 | common 0x0D GEN_CFG4 [2] RESERVED (R/W) | S8.1.9 p56 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-1-0D-1_0 | common 0x0D GEN_CFG4 [1:0] DRIVE_STRENGTH[1:0] (R/W) | S8.1.9 p56 | `Interface.drive` (Normal, Half, Double, OneAndHalf) | T21 | OK |
| FLD-1-0E-15 | common 0x0E DIG_DELAY_CFG1 [15] RESERVED (R/W) | S8.1.10 p57 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-1-0E-14_12 | common 0x0E DIG_DELAY_CFG1 [14:12] DIG_DELAY_SDOUT[2:0] (R/W) | S8.1.10 p57 | `Interface.delay_ns` for SDOUT, 0..5 ns; 6 and 7 refused | T21 | OK |
| FLD-1-0E-11_9 | common 0x0E DIG_DELAY_CFG1 [11:9] DIG_DELAY_D3[2:0] (R/W) | S8.1.10 p57 | `Interface.delay_ns` for D3, 0..5 ns; 6 and 7 refused | T21 | OK |
| FLD-1-0E-8_6 | common 0x0E DIG_DELAY_CFG1 [8:6] DIG_DELAY_D2[2:0] (R/W) | S8.1.10 p57 | `Interface.delay_ns` for D2, 0..5 ns; 6 and 7 refused | T21 | OK |
| FLD-1-0E-5_3 | common 0x0E DIG_DELAY_CFG1 [5:3] DIG_DELAY_D1[2:0] (R/W) | S8.1.10 p57 | `Interface.delay_ns` for D1, 0..5 ns; 6 and 7 refused | T21 | OK |
| FLD-1-0E-2_0 | common 0x0E DIG_DELAY_CFG1 [2:0] DIG_DELAY_D0[2:0] (R/W) | S8.1.10 p57 | `Interface.delay_ns` for D0, 0..5 ns; 6 and 7 refused | T21 | OK |
| FLD-1-0F-15_12 | common 0x0F DIG_DELAY_CFG2 [15:12] RESERVED (R/W) | S8.1.11 p58 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-1-0F-11_9 | common 0x0F DIG_DELAY_CFG2 [11:9] DIG_DELAY_D7[2:0] (R/W) | S8.1.11 p58 | `Interface.delay_ns` for D7, 0..5 ns; 6 and 7 refused | T21 | OK |
| FLD-1-0F-8_6 | common 0x0F DIG_DELAY_CFG2 [8:6] DIG_DELAY_D6[2:0] (R/W) | S8.1.11 p58 | `Interface.delay_ns` for D6, 0..5 ns; 6 and 7 refused | T21 | OK |
| FLD-1-0F-5_3 | common 0x0F DIG_DELAY_CFG2 [5:3] DIG_DELAY_D5[2:0] (R/W) | S8.1.11 p58 | `Interface.delay_ns` for D5, 0..5 ns; 6 and 7 refused | T21 | OK |
| FLD-1-0F-2_0 | common 0x0F DIG_DELAY_CFG2 [2:0] DIG_DELAY_D4[2:0] (R/W) | S8.1.11 p58 | `Interface.delay_ns` for D4, 0..5 ns; 6 and 7 refused | T21 | OK |
| FLD-1-10-15_2 | common 0x10 ANA_CFG1 [15:2] RESERVED (R/W) | S8.1.12 p58 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-1-10-1 | common 0x10 ANA_CFG1 [1] REFSEL_CTRL_DIS (R/W) | S8.1.12 p58 | `Reference` (Internal and External set it, RefselPin clears it) | T07 | OK |
| FLD-1-10-0 | common 0x10 ANA_CFG1 [0] EXT_REF_EN (R/W) | S8.1.12 p58 | `Reference::External` | T07 | OK |
| FLD-1-11-15 | common 0x11 ANA_CFG2 [15] CH_XTALK_LOW_SPEED (R/W) | S8.1.13 p59 | `Config.low_speed_crosstalk` (explicit, A45) | T11 | OK |
| FLD-1-11-14_7 | common 0x11 ANA_CFG2 [14:7] RESERVED (R/W) | S8.1.13 p59 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-1-11-6_4 | common 0x11 ANA_CFG2 [6:4] ADC_CH_SEL[2:0] (R/W) | S8.1.13 p59 | `ChannelSelect.group` | T11 | OK |
| FLD-1-11-3_2 | common 0x11 ANA_CFG2 [3:2] ADC_NUM_SEL[1:0] (R/W) | S8.1.13 p59 | `ChannelSelect.count` (16, 8, 4, 2) | T11 | OK |
| FLD-1-11-1_0 | common 0x11 ANA_CFG2 [1:0] RESERVED (R/W) | S8.1.13 p59 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-1-12-15_14 | common 0x12 ADC_CAL [15:14] RESERVED (R/W) | S8.1.14 p59 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-1-12-13 | common 0x12 ADC_CAL [13] SE_DIFF_MODE_AIN1_8 (R/W) | S8.1.14 p59 | derived per bank from `Input.type` by `RunAdcCalibration`; mixed bank refused | T15 | GUARD |
| FLD-1-12-12 | common 0x12 ADC_CAL [12] SE_DIFF_MODE_AIN9_16 (R/W) | S8.1.14 p59 | derived per bank from `Input.type` by `RunAdcCalibration`; mixed bank refused | T15 | GUARD |
| FLD-1-12-11_9 | common 0x12 ADC_CAL [11:9] RESERVED (R/W) | S8.1.14 p59 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-1-12-8 | common 0x12 ADC_CAL [8] ADC_CAL_TRIG (R/W) | S8.1.14 p59 | internal 0, 1, 0 sequence of `RunAdcCalibration` | T15 | OK |
| FLD-1-12-7_2 | common 0x12 ADC_CAL [7:2] RESERVED (R/W) | S8.1.14 p59 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-1-12-1_0 | common 0x12 ADC_CAL [1:0] ADC_CAL_MODE[1:0] (R/W) | S8.1.14 p59 | `RunAdcCalibration(mode)`; kept afterwards; `DisableAdcCalibration()` writes 00b | T15 | OK |
| FLD-1-14-15 | common 0x14 DIG_FILTER [15] PHASE_DELAY_EN (R/W) | S8.1.15 p59 | `SetInput`, `Input::phase_delay` (a non-zero delay enables the stage) | T17 | OK |
| FLD-1-14-14_12 | common 0x14 DIG_FILTER [14:12] FIR_FILT_SEL[2:0] (R/W) | S8.1.15 p59 | `Filter::Fir` (Fir1..Fir5, Fir7); 110b unreachable | T16 | GUARD |
| FLD-1-14-11_8 | common 0x14 DIG_FILTER [11:8] MVG_AVG_LEN[3:0] (R/W) | S8.1.15 p59 | `Filter::MovingAverage` (2..128); 12..15 unreachable | T16 | GUARD |
| FLD-1-14-7_4 | common 0x14 DIG_FILTER [7:4] BLK_AVG_OSR[3:0] (R/W) | S8.1.15 p59 | `Filter::BlockAverage` (2..128); 12..15 unreachable | T16 | GUARD |
| FLD-1-14-3_2 | common 0x14 DIG_FILTER [3:2] RESERVED (R/W) | S8.1.15 p59 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-1-14-1 | common 0x14 DIG_FILTER [1] INT_TRIG_MODE (R/W) | S8.1.15 p59 | `Filter::internal_clock`; set by single-shot calibration, restored after | T15,T16 | OK |
| FLD-1-14-0 | common 0x14 DIG_FILTER [0] DIG_FILT_SYSREF (R/W) | S8.1.15 p59 | internal 0, 1, wait >= tCONVST, 0 after every filter or phase change | T16,T17 | OK |
| FLD-1-15-15_4 | common 0x15 GEN_CFG5 [15:4] RESERVED (R/W) | S8.1.16 p60 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-1-15-3 | common 0x15 GEN_CFG5 [3] T_MODE_OVR_EN (R/W) | S8.1.16 p60 | never written: undocumented (empty description, Table 8-18) | T03 | GUARD |
| FLD-1-15-2 | common 0x15 GEN_CFG5 [2] T_MODE (R/W) | S8.1.16 p60 | never written: undocumented (empty description, Table 8-18) | T03 | GUARD |
| FLD-1-15-1 | common 0x15 GEN_CFG5 [1] AVG_MODE_OVR_EN (R/W) | S8.1.16 p60 | internal: set 1 before AVG_MODE when any filter is active (A31) | T16 | OK |
| FLD-1-15-0 | common 0x15 GEN_CFG5 [0] AVG_MODE (R/W) | S8.1.16 p60 | internal: set 1 after AVG_MODE_OVR_EN when any filter is active (A31) | T16 | OK |
| FLD-1-1D-15_4 | common 0x1D DEVICE_STATUS [15:4] RESERVED (R) | S8.1.17 p61 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-1-1D-3 | common 0x1D DEVICE_STATUS [3] CALIB_BUSY_FLAG (R) | S8.1.17 p61 | `IsCalibrationBusy`; confirms the end of a calibration | T15 | OK |
| FLD-1-1D-2_0 | common 0x1D DEVICE_STATUS [2:0] RESERVED (R) | S8.1.17 p61 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-1-21-15_8 | common 0x21 DEVICE_ID [15:8] RESERVED (R) | S8.1.18 p61 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-1-21-7_0 | common 0x21 DEVICE_ID [7:0] DEVICE_ID[7:0] (R) | S8.1.18 p61 | `Identify()`: 0x04 or `DeviceIdMismatch` | T06 | OK |
| FLD-2-08-15 | AIN1-8 0x08 PGA_CONFIG_AIN1_2 [15] CME_CORR_EN_AIN2 (AIN2, local 1) (R/W) | S8.2.1 p64 | `Input::cm_correction` | T08 | OK |
| FLD-2-08-14_12 | AIN1-8 0x08 PGA_CONFIG_AIN1_2 [14:12] CM_RANGE_AIN2[2:0] (AIN2, local 1) (R/W) | S8.2.1 p64 | `Input.type` (Differential 0, SingleEnded 5, SingleEndedOpenWireSafe 6); others unreachable | T08,T09 | GUARD |
| FLD-2-08-11 | AIN1-8 0x08 PGA_CONFIG_AIN1_2 [11] RESERVED (R/W) | S8.2.1 p64 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-2-08-10_8 | AIN1-8 0x08 PGA_CONFIG_AIN1_2 [10:8] INPUT_RANGE_AIN2[2:0] (AIN2, local 1) (R/W) | S8.2.1 p64 | `Input.range` (V5 0, V2_5 2, V6_25 3, V10 4, V12_5 5); 1, 6, 7 unreachable | T08 | GUARD |
| FLD-2-08-7 | AIN1-8 0x08 PGA_CONFIG_AIN1_2 [7] CME_CORR_EN_AIN1 (AIN1, local 0) (R/W) | S8.2.1 p64 | `Input::cm_correction` | T08 | OK |
| FLD-2-08-6_4 | AIN1-8 0x08 PGA_CONFIG_AIN1_2 [6:4] CM_RANGE_AIN1[2:0] (AIN1, local 0) (R/W) | S8.2.1 p64 | `Input.type` (Differential 0, SingleEnded 5, SingleEndedOpenWireSafe 6); others unreachable | T08,T09 | GUARD |
| FLD-2-08-3 | AIN1-8 0x08 PGA_CONFIG_AIN1_2 [3] RESERVED (R/W) | S8.2.1 p64 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-2-08-2_0 | AIN1-8 0x08 PGA_CONFIG_AIN1_2 [2:0] INPUT_RANGE_AIN1[2:0] (AIN1, local 0) (R/W) | S8.2.1 p64 | `Input.range` (V5 0, V2_5 2, V6_25 3, V10 4, V12_5 5); 1, 6, 7 unreachable | T08 | GUARD |
| FLD-2-09-15 | AIN1-8 0x09 PGA_CONFIG_AIN3_4 [15] CME_CORR_EN_AIN4 (AIN4, local 3) (R/W) | S8.2.2 p65 | `Input::cm_correction` | T08 | OK |
| FLD-2-09-14_12 | AIN1-8 0x09 PGA_CONFIG_AIN3_4 [14:12] CM_RANGE_AIN4[2:0] (AIN4, local 3) (R/W) | S8.2.2 p65 | `Input.type` (Differential 0, SingleEnded 5, SingleEndedOpenWireSafe 6); others unreachable | T08,T09 | GUARD |
| FLD-2-09-11 | AIN1-8 0x09 PGA_CONFIG_AIN3_4 [11] RESERVED (R/W) | S8.2.2 p65 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-2-09-10_8 | AIN1-8 0x09 PGA_CONFIG_AIN3_4 [10:8] INPUT_RANGE_AIN4[2:0] (AIN4, local 3) (R/W) | S8.2.2 p65 | `Input.range` (V5 0, V2_5 2, V6_25 3, V10 4, V12_5 5); 1, 6, 7 unreachable | T08 | GUARD |
| FLD-2-09-7 | AIN1-8 0x09 PGA_CONFIG_AIN3_4 [7] CME_CORR_EN_AIN3 (AIN3, local 2) (R/W) | S8.2.2 p65 | `Input::cm_correction` | T08 | OK |
| FLD-2-09-6_4 | AIN1-8 0x09 PGA_CONFIG_AIN3_4 [6:4] CM_RANGE_AIN3[2:0] (AIN3, local 2) (R/W) | S8.2.2 p65 | `Input.type` (Differential 0, SingleEnded 5, SingleEndedOpenWireSafe 6); others unreachable | T08,T09 | GUARD |
| FLD-2-09-3 | AIN1-8 0x09 PGA_CONFIG_AIN3_4 [3] RESERVED (R/W) | S8.2.2 p65 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-2-09-2_0 | AIN1-8 0x09 PGA_CONFIG_AIN3_4 [2:0] INPUT_RANGE_AIN3[2:0] (AIN3, local 2) (R/W) | S8.2.2 p65 | `Input.range` (V5 0, V2_5 2, V6_25 3, V10 4, V12_5 5); 1, 6, 7 unreachable | T08 | GUARD |
| FLD-2-0A-15 | AIN1-8 0x0A PGA_CONFIG_AIN5_6 [15] CME_CORR_EN_AIN6 (AIN6, local 5) (R/W) | S8.2.3 p65 | `Input::cm_correction` | T08 | OK |
| FLD-2-0A-14_12 | AIN1-8 0x0A PGA_CONFIG_AIN5_6 [14:12] CM_RANGE_AIN6[2:0] (AIN6, local 5) (R/W) | S8.2.3 p65 | `Input.type` (Differential 0, SingleEnded 5, SingleEndedOpenWireSafe 6); others unreachable | T08,T09 | GUARD |
| FLD-2-0A-11 | AIN1-8 0x0A PGA_CONFIG_AIN5_6 [11] RESERVED (R/W) | S8.2.3 p65 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-2-0A-10_8 | AIN1-8 0x0A PGA_CONFIG_AIN5_6 [10:8] INPUT_RANGE_AIN6[2:0] (AIN6, local 5) (R/W) | S8.2.3 p65 | `Input.range` (V5 0, V2_5 2, V6_25 3, V10 4, V12_5 5); 1, 6, 7 unreachable | T08 | GUARD |
| FLD-2-0A-7 | AIN1-8 0x0A PGA_CONFIG_AIN5_6 [7] CME_CORR_EN_AIN5 (AIN5, local 4) (R/W) | S8.2.3 p65 | `Input::cm_correction` | T08 | OK |
| FLD-2-0A-6_4 | AIN1-8 0x0A PGA_CONFIG_AIN5_6 [6:4] CM_RANGE_AIN5[2:0] (AIN5, local 4) (R/W) | S8.2.3 p65 | `Input.type` (Differential 0, SingleEnded 5, SingleEndedOpenWireSafe 6); others unreachable | T08,T09 | GUARD |
| FLD-2-0A-3 | AIN1-8 0x0A PGA_CONFIG_AIN5_6 [3] RESERVED (R/W) | S8.2.3 p65 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-2-0A-2_0 | AIN1-8 0x0A PGA_CONFIG_AIN5_6 [2:0] INPUT_RANGE_AIN5[2:0] (AIN5, local 4) (R/W) | S8.2.3 p65 | `Input.range` (V5 0, V2_5 2, V6_25 3, V10 4, V12_5 5); 1, 6, 7 unreachable | T08 | GUARD |
| FLD-2-0B-15 | AIN1-8 0x0B PGA_CONFIG_AIN7_8 [15] CME_CORR_EN_AIN8 (AIN8, local 7) (R/W) | S8.2.4 p66 | `Input::cm_correction` | T08 | OK |
| FLD-2-0B-14_12 | AIN1-8 0x0B PGA_CONFIG_AIN7_8 [14:12] CM_RANGE_AIN8[2:0] (AIN8, local 7) (R/W) | S8.2.4 p66 | `Input.type` (Differential 0, SingleEnded 5, SingleEndedOpenWireSafe 6); others unreachable | T08,T09 | GUARD |
| FLD-2-0B-11 | AIN1-8 0x0B PGA_CONFIG_AIN7_8 [11] RESERVED (R/W) | S8.2.4 p66 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-2-0B-10_8 | AIN1-8 0x0B PGA_CONFIG_AIN7_8 [10:8] INPUT_RANGE_AIN8[2:0] (AIN8, local 7) (R/W) | S8.2.4 p66 | `Input.range` (V5 0, V2_5 2, V6_25 3, V10 4, V12_5 5); 1, 6, 7 unreachable | T08 | GUARD |
| FLD-2-0B-7 | AIN1-8 0x0B PGA_CONFIG_AIN7_8 [7] CME_CORR_EN_AIN7 (AIN7, local 6) (R/W) | S8.2.4 p66 | `Input::cm_correction` | T08 | OK |
| FLD-2-0B-6_4 | AIN1-8 0x0B PGA_CONFIG_AIN7_8 [6:4] CM_RANGE_AIN7[2:0] (AIN7, local 6) (R/W) | S8.2.4 p66 | `Input.type` (Differential 0, SingleEnded 5, SingleEndedOpenWireSafe 6); others unreachable | T08,T09 | GUARD |
| FLD-2-0B-3 | AIN1-8 0x0B PGA_CONFIG_AIN7_8 [3] RESERVED (R/W) | S8.2.4 p66 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-2-0B-2_0 | AIN1-8 0x0B PGA_CONFIG_AIN7_8 [2:0] INPUT_RANGE_AIN7[2:0] (AIN7, local 6) (R/W) | S8.2.4 p66 | `Input.range` (V5 0, V2_5 2, V6_25 3, V10 4, V12_5 5); 1, 6, 7 unreachable | T08 | GUARD |
| FLD-2-0C-15_14 | AIN1-8 0x0C PGA_BW_SEL_AIN1_8 [15:14] PGA_BW_SEL_AIN8[1:0] (AIN8, local 7) (R/W) | S8.2.5 p67 | `Input.bandwidth` (Low 0, Wide 1); 2, 3 unreachable | T08 | GUARD |
| FLD-2-0C-13_12 | AIN1-8 0x0C PGA_BW_SEL_AIN1_8 [13:12] PGA_BW_SEL_AIN7[1:0] (AIN7, local 6) (R/W) | S8.2.5 p67 | `Input.bandwidth` (Low 0, Wide 1); 2, 3 unreachable | T08 | GUARD |
| FLD-2-0C-11_10 | AIN1-8 0x0C PGA_BW_SEL_AIN1_8 [11:10] PGA_BW_SEL_AIN6[1:0] (AIN6, local 5) (R/W) | S8.2.5 p67 | `Input.bandwidth` (Low 0, Wide 1); 2, 3 unreachable | T08 | GUARD |
| FLD-2-0C-9_8 | AIN1-8 0x0C PGA_BW_SEL_AIN1_8 [9:8] PGA_BW_SEL_AIN5[1:0] (AIN5, local 4) (R/W) | S8.2.5 p67 | `Input.bandwidth` (Low 0, Wide 1); 2, 3 unreachable | T08 | GUARD |
| FLD-2-0C-7_6 | AIN1-8 0x0C PGA_BW_SEL_AIN1_8 [7:6] PGA_BW_SEL_AIN4[1:0] (AIN4, local 3) (R/W) | S8.2.5 p67 | `Input.bandwidth` (Low 0, Wide 1); 2, 3 unreachable | T08 | GUARD |
| FLD-2-0C-5_4 | AIN1-8 0x0C PGA_BW_SEL_AIN1_8 [5:4] PGA_BW_SEL_AIN3[1:0] (AIN3, local 2) (R/W) | S8.2.5 p67 | `Input.bandwidth` (Low 0, Wide 1); 2, 3 unreachable | T08 | GUARD |
| FLD-2-0C-3_2 | AIN1-8 0x0C PGA_BW_SEL_AIN1_8 [3:2] PGA_BW_SEL_AIN2[1:0] (AIN2, local 1) (R/W) | S8.2.5 p67 | `Input.bandwidth` (Low 0, Wide 1); 2, 3 unreachable | T08 | GUARD |
| FLD-2-0C-1_0 | AIN1-8 0x0C PGA_BW_SEL_AIN1_8 [1:0] PGA_BW_SEL_AIN1[1:0] (AIN1, local 0) (R/W) | S8.2.5 p67 | `Input.bandwidth` (Low 0, Wide 1); 2, 3 unreachable | T08 | GUARD |
| FLD-2-0D-15_8 | AIN1-8 0x0D PHASE_DELAY_AIN1_2 [15:8] PHASE_DELAY_AIN2[7:0] (AIN2, local 1) (R/W) | S8.2.6 p68 | `SetInput`, `Input::phase_delay` | T17 | OK |
| FLD-2-0D-7_0 | AIN1-8 0x0D PHASE_DELAY_AIN1_2 [7:0] PHASE_DELAY_AIN1[7:0] (AIN1, local 0) (R/W) | S8.2.6 p68 | `SetInput`, `Input::phase_delay` | T17 | OK |
| FLD-2-0E-15_8 | AIN1-8 0x0E PHASE_DELAY_AIN3_4 [15:8] PHASE_DELAY_AIN4[7:0] (AIN4, local 3) (R/W) | S8.2.7 p68 | `SetInput`, `Input::phase_delay` | T17 | OK |
| FLD-2-0E-7_0 | AIN1-8 0x0E PHASE_DELAY_AIN3_4 [7:0] PHASE_DELAY_AIN3[7:0] (AIN3, local 2) (R/W) | S8.2.7 p68 | `SetInput`, `Input::phase_delay` | T17 | OK |
| FLD-2-0F-15_8 | AIN1-8 0x0F PHASE_DELAY_AIN5_6 [15:8] PHASE_DELAY_AIN6[7:0] (AIN6, local 5) (R/W) | S8.2.8 p68 | `SetInput`, `Input::phase_delay` | T17 | OK |
| FLD-2-0F-7_0 | AIN1-8 0x0F PHASE_DELAY_AIN5_6 [7:0] PHASE_DELAY_AIN5[7:0] (AIN5, local 4) (R/W) | S8.2.8 p68 | `SetInput`, `Input::phase_delay` | T17 | OK |
| FLD-2-10-15_8 | AIN1-8 0x10 PHASE_DELAY_AIN7_8 [15:8] PHASE_DELAY_AIN8[7:0] (AIN8, local 7) (R/W) | S8.2.9 p69 | `SetInput`, `Input::phase_delay` | T17 | OK |
| FLD-2-10-7_0 | AIN1-8 0x10 PHASE_DELAY_AIN7_8 [7:0] PHASE_DELAY_AIN7[7:0] (AIN7, local 6) (R/W) | S8.2.9 p69 | `SetInput`, `Input::phase_delay` | T17 | OK |
| FLD-2-11-15_10 | AIN1-8 0x11 OFS_AIN1 [15:10] RESERVED (R/W) | S8.2.10 p69 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-2-11-9_0 | AIN1-8 0x11 OFS_AIN1 [9:0] OFS_AIN1[9:0] (AIN1, local 0) (R/W) | S8.2.10 p69 | `SetInput`, `Input::offset_quarter_lsb` quarter-LSB16; helper `OffsetCorrectionFor` | T14 | OK |
| FLD-2-12-15_10 | AIN1-8 0x12 OFS_AIN2 [15:10] RESERVED (R/W) | S8.2.11 p69 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-2-12-9_0 | AIN1-8 0x12 OFS_AIN2 [9:0] OFS_AIN2[9:0] (AIN2, local 1) (R/W) | S8.2.11 p69 | `SetInput`, `Input::offset_quarter_lsb` quarter-LSB16; helper `OffsetCorrectionFor` | T14 | OK |
| FLD-2-13-15_10 | AIN1-8 0x13 OFS_AIN3 [15:10] RESERVED (R/W) | S8.2.12 p70 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-2-13-9_0 | AIN1-8 0x13 OFS_AIN3 [9:0] OFS_AIN3[9:0] (AIN3, local 2) (R/W) | S8.2.12 p70 | `SetInput`, `Input::offset_quarter_lsb` quarter-LSB16; helper `OffsetCorrectionFor` | T14 | OK |
| FLD-2-14-15_10 | AIN1-8 0x14 OFS_AIN4 [15:10] RESERVED (R/W) | S8.2.13 p70 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-2-14-9_0 | AIN1-8 0x14 OFS_AIN4 [9:0] OFS_AIN4[9:0] (AIN4, local 3) (R/W) | S8.2.13 p70 | `SetInput`, `Input::offset_quarter_lsb` quarter-LSB16; helper `OffsetCorrectionFor` | T14 | OK |
| FLD-2-15-15_10 | AIN1-8 0x15 OFS_AIN5 [15:10] RESERVED (R/W) | S8.2.14 p70 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-2-15-9_0 | AIN1-8 0x15 OFS_AIN5 [9:0] OFS_AIN5[9:0] (AIN5, local 4) (R/W) | S8.2.14 p70 | `SetInput`, `Input::offset_quarter_lsb` quarter-LSB16; helper `OffsetCorrectionFor` | T14 | OK |
| FLD-2-16-15_10 | AIN1-8 0x16 OFS_AIN6 [15:10] RESERVED (R/W) | S8.2.15 p71 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-2-16-9_0 | AIN1-8 0x16 OFS_AIN6 [9:0] OFS_AIN6[9:0] (AIN6, local 5) (R/W) | S8.2.15 p71 | `SetInput`, `Input::offset_quarter_lsb` quarter-LSB16; helper `OffsetCorrectionFor` | T14 | OK |
| FLD-2-17-15_10 | AIN1-8 0x17 OFS_AIN7 [15:10] RESERVED (R/W) | S8.2.16 p71 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-2-17-9_0 | AIN1-8 0x17 OFS_AIN7 [9:0] OFS_AIN7[9:0] (AIN7, local 6) (R/W) | S8.2.16 p71 | `SetInput`, `Input::offset_quarter_lsb` quarter-LSB16; helper `OffsetCorrectionFor` | T14 | OK |
| FLD-2-18-15_10 | AIN1-8 0x18 OFS_AIN8 [15:10] RESERVED (R/W) | S8.2.17 p71 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-2-18-9_0 | AIN1-8 0x18 OFS_AIN8 [9:0] OFS_AIN8[9:0] (AIN8, local 7) (R/W) | S8.2.17 p71 | `SetInput`, `Input::offset_quarter_lsb` quarter-LSB16; helper `OffsetCorrectionFor` | T14 | OK |
| FLD-2-19-15_14 | AIN1-8 0x19 GAN_AIN1 [15:14] RESERVED (R/W) | S8.2.18 p72 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-2-19-13_0 | AIN1-8 0x19 GAN_AIN1 [13:0] GAN_AIN1[13:0] (AIN1, local 0) (R/W) | S8.2.18 p72 | `SetInput`, `Input::gain`; helper `GainCorrectionFor` | T14 | OK |
| FLD-2-1A-15_14 | AIN1-8 0x1A GAN_AIN2 [15:14] RESERVED (R/W) | S8.2.19 p72 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-2-1A-13_0 | AIN1-8 0x1A GAN_AIN2 [13:0] GAN_AIN2[13:0] (AIN2, local 1) (R/W) | S8.2.19 p72 | `SetInput`, `Input::gain`; helper `GainCorrectionFor` | T14 | OK |
| FLD-2-1B-15_14 | AIN1-8 0x1B GAN_AIN3 [15:14] RESERVED (R/W) | S8.2.20 p72 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-2-1B-13_0 | AIN1-8 0x1B GAN_AIN3 [13:0] GAN_AIN3[13:0] (AIN3, local 2) (R/W) | S8.2.20 p72 | `SetInput`, `Input::gain`; helper `GainCorrectionFor` | T14 | OK |
| FLD-2-1C-15_14 | AIN1-8 0x1C GAN_AIN4 [15:14] RESERVED (R/W) | S8.2.21 p73 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-2-1C-13_0 | AIN1-8 0x1C GAN_AIN4 [13:0] GAN_AIN4[13:0] (AIN4, local 3) (R/W) | S8.2.21 p73 | `SetInput`, `Input::gain`; helper `GainCorrectionFor` | T14 | OK |
| FLD-2-1D-15_14 | AIN1-8 0x1D GAN_AIN5 [15:14] RESERVED (R/W) | S8.2.22 p73 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-2-1D-13_0 | AIN1-8 0x1D GAN_AIN5 [13:0] GAN_AIN5[13:0] (AIN5, local 4) (R/W) | S8.2.22 p73 | `SetInput`, `Input::gain`; helper `GainCorrectionFor` | T14 | OK |
| FLD-2-1E-15_14 | AIN1-8 0x1E GAN_AIN6 [15:14] RESERVED (R/W) | S8.2.23 p74 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-2-1E-13_0 | AIN1-8 0x1E GAN_AIN6 [13:0] GAN_AIN6[13:0] (AIN6, local 5) (R/W) | S8.2.23 p74 | `SetInput`, `Input::gain`; helper `GainCorrectionFor` | T14 | OK |
| FLD-2-1F-15_14 | AIN1-8 0x1F GAN_AIN7 [15:14] RESERVED (R/W) | S8.2.24 p74 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-2-1F-13_0 | AIN1-8 0x1F GAN_AIN7 [13:0] GAN_AIN7[13:0] (AIN7, local 6) (R/W) | S8.2.24 p74 | `SetInput`, `Input::gain`; helper `GainCorrectionFor` | T14 | OK |
| FLD-2-20-15_14 | AIN1-8 0x20 GAN_AIN8 [15:14] RESERVED (R/W) | S8.2.25 p74 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-2-20-13_0 | AIN1-8 0x20 GAN_AIN8 [13:0] GAN_AIN8[13:0] (AIN8, local 7) (R/W) | S8.2.25 p74 | `SetInput`, `Input::gain`; helper `GainCorrectionFor` | T14 | OK |
| FLD-2-21-15 | AIN1-8 0x21 DWC_CFG [15] DWC_STAT_RST (R/W) | S8.2.26 p75 | `ClearWindowFlags(group)` writes 1 then 0 (edge) | T18 | OK |
| FLD-2-21-14_12 | AIN1-8 0x21 DWC_CFG [14:12] RESERVED (R/W) | S8.2.26 p75 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-2-21-11_8 | AIN1-8 0x21 DWC_CFG [11:8] DWC_GLITCH_FILT[3:0] (R/W) | S8.2.26 p75 | `Config::window_glitch` | T18 | OK |
| FLD-2-21-7 | AIN1-8 0x21 DWC_CFG [7] DWC_EN_AIN8 (AIN8, local 7) (R/W) | S8.2.26 p75 | `Window.enabled`; thresholds written before the enable bit | T18 | OK |
| FLD-2-21-6 | AIN1-8 0x21 DWC_CFG [6] DWC_EN_AIN7 (AIN7, local 6) (R/W) | S8.2.26 p75 | `Window.enabled`; thresholds written before the enable bit | T18 | OK |
| FLD-2-21-5 | AIN1-8 0x21 DWC_CFG [5] DWC_EN_AIN6 (AIN6, local 5) (R/W) | S8.2.26 p75 | `Window.enabled`; thresholds written before the enable bit | T18 | OK |
| FLD-2-21-4 | AIN1-8 0x21 DWC_CFG [4] DWC_EN_AIN5 (AIN5, local 4) (R/W) | S8.2.26 p75 | `Window.enabled`; thresholds written before the enable bit | T18 | OK |
| FLD-2-21-3 | AIN1-8 0x21 DWC_CFG [3] DWC_EN_AIN4 (AIN4, local 3) (R/W) | S8.2.26 p75 | `Window.enabled`; thresholds written before the enable bit | T18 | OK |
| FLD-2-21-2 | AIN1-8 0x21 DWC_CFG [2] DWC_EN_AIN3 (AIN3, local 2) (R/W) | S8.2.26 p75 | `Window.enabled`; thresholds written before the enable bit | T18 | OK |
| FLD-2-21-1 | AIN1-8 0x21 DWC_CFG [1] DWC_EN_AIN2 (AIN2, local 1) (R/W) | S8.2.26 p75 | `Window.enabled`; thresholds written before the enable bit | T18 | OK |
| FLD-2-21-0 | AIN1-8 0x21 DWC_CFG [0] DWC_EN_AIN1 (AIN1, local 0) (R/W) | S8.2.26 p75 | `Window.enabled`; thresholds written before the enable bit | T18 | OK |
| FLD-2-22-15_8 | AIN1-8 0x22 DWC_TH_AIN1 [15:8] HIGH_TH_AIN1[7:0] (AIN1, local 0) (R/W) | S8.2.27 p75 | `Window.high_code` -> top byte in the output format (A16) | T18 | OK |
| FLD-2-22-7_0 | AIN1-8 0x22 DWC_TH_AIN1 [7:0] LOW_TH_AIN1[7:0] (AIN1, local 0) (R/W) | S8.2.27 p75 | `Window.low_code` -> top byte in the output format (A16) | T18 | OK |
| FLD-2-23-15_8 | AIN1-8 0x23 DWC_TH_AIN2 [15:8] HIGH_TH_AIN2[7:0] (AIN2, local 1) (R/W) | S8.2.28 p76 | `Window.high_code` -> top byte in the output format (A16) | T18 | OK |
| FLD-2-23-7_0 | AIN1-8 0x23 DWC_TH_AIN2 [7:0] LOW_TH_AIN2[7:0] (AIN2, local 1) (R/W) | S8.2.28 p76 | `Window.low_code` -> top byte in the output format (A16) | T18 | OK |
| FLD-2-24-15_8 | AIN1-8 0x24 DWC_TH_AIN3 [15:8] HIGH_TH_AIN3[7:0] (AIN3, local 2) (R/W) | S8.2.29 p76 | `Window.high_code` -> top byte in the output format (A16) | T18 | OK |
| FLD-2-24-7_0 | AIN1-8 0x24 DWC_TH_AIN3 [7:0] LOW_TH_AIN3[7:0] (AIN3, local 2) (R/W) | S8.2.29 p76 | `Window.low_code` -> top byte in the output format (A16) | T18 | OK |
| FLD-2-25-15_8 | AIN1-8 0x25 DWC_TH_AIN4 [15:8] HIGH_TH_AIN4[7:0] (AIN4, local 3) (R/W) | S8.2.30 p76 | `Window.high_code` -> top byte in the output format (A16) | T18 | OK |
| FLD-2-25-7_0 | AIN1-8 0x25 DWC_TH_AIN4 [7:0] LOW_TH_AIN4[7:0] (AIN4, local 3) (R/W) | S8.2.30 p76 | `Window.low_code` -> top byte in the output format (A16) | T18 | OK |
| FLD-2-26-15_8 | AIN1-8 0x26 DWC_TH_AIN5 [15:8] HIGH_TH_AIN5[7:0] (AIN5, local 4) (R/W) | S8.2.31 p77 | `Window.high_code` -> top byte in the output format (A16) | T18 | OK |
| FLD-2-26-7_0 | AIN1-8 0x26 DWC_TH_AIN5 [7:0] LOW_TH_AIN5[7:0] (AIN5, local 4) (R/W) | S8.2.31 p77 | `Window.low_code` -> top byte in the output format (A16) | T18 | OK |
| FLD-2-27-15_8 | AIN1-8 0x27 DWC_TH_AIN6 [15:8] HIGH_TH_AIN6[7:0] (AIN6, local 5) (R/W) | S8.2.32 p77 | `Window.high_code` -> top byte in the output format (A16) | T18 | OK |
| FLD-2-27-7_0 | AIN1-8 0x27 DWC_TH_AIN6 [7:0] LOW_TH_AIN6[7:0] (AIN6, local 5) (R/W) | S8.2.32 p77 | `Window.low_code` -> top byte in the output format (A16) | T18 | OK |
| FLD-2-28-15_8 | AIN1-8 0x28 DWC_TH_AIN7 [15:8] HIGH_TH_AIN7[7:0] (AIN7, local 6) (R/W) | S8.2.33 p77 | `Window.high_code` -> top byte in the output format (A16) | T18 | OK |
| FLD-2-28-7_0 | AIN1-8 0x28 DWC_TH_AIN7 [7:0] LOW_TH_AIN7[7:0] (AIN7, local 6) (R/W) | S8.2.33 p77 | `Window.low_code` -> top byte in the output format (A16) | T18 | OK |
| FLD-2-29-15_8 | AIN1-8 0x29 DWC_TH_AIN8 [15:8] HIGH_TH_AIN8[7:0] (AIN8, local 7) (R/W) | S8.2.34 p78 | `Window.high_code` -> top byte in the output format (A16) | T18 | OK |
| FLD-2-29-7_0 | AIN1-8 0x29 DWC_TH_AIN8 [7:0] LOW_TH_AIN8[7:0] (AIN8, local 7) (R/W) | S8.2.34 p78 | `Window.low_code` -> top byte in the output format (A16) | T18 | OK |
| FLD-2-2A-15_8 | AIN1-8 0x2A DWC_HYS_AIN1_2 [15:8] HYS_AIN2[7:0] (AIN2, local 1) (R/W) | S8.2.35 p78 | `Window.hysteresis` 0..255 (in top-byte LSBs) | T18 | OK |
| FLD-2-2A-7_0 | AIN1-8 0x2A DWC_HYS_AIN1_2 [7:0] HYS_AIN1[7:0] (AIN1, local 0) (R/W) | S8.2.35 p78 | `Window.hysteresis` 0..255 (in top-byte LSBs) | T18 | OK |
| FLD-2-2B-15_8 | AIN1-8 0x2B DWC_HYS_AIN3_4 [15:8] HYS_AIN4[7:0] (AIN4, local 3) (R/W) | S8.2.36 p78 | `Window.hysteresis` 0..255 (in top-byte LSBs) | T18 | OK |
| FLD-2-2B-7_0 | AIN1-8 0x2B DWC_HYS_AIN3_4 [7:0] HYS_AIN3[7:0] (AIN3, local 2) (R/W) | S8.2.36 p78 | `Window.hysteresis` 0..255 (in top-byte LSBs) | T18 | OK |
| FLD-2-2C-15_8 | AIN1-8 0x2C DWC_HYS_AIN5_6 [15:8] HYS_AIN6[7:0] (AIN6, local 5) (R/W) | S8.2.37 p79 | `Window.hysteresis` 0..255 (in top-byte LSBs) | T18 | OK |
| FLD-2-2C-7_0 | AIN1-8 0x2C DWC_HYS_AIN5_6 [7:0] HYS_AIN5[7:0] (AIN5, local 4) (R/W) | S8.2.37 p79 | `Window.hysteresis` 0..255 (in top-byte LSBs) | T18 | OK |
| FLD-2-2D-15_8 | AIN1-8 0x2D DWC_HYS_AIN7_8 [15:8] HYS_AIN8[7:0] (AIN8, local 7) (R/W) | S8.2.38 p79 | `Window.hysteresis` 0..255 (in top-byte LSBs) | T18 | OK |
| FLD-2-2D-7_0 | AIN1-8 0x2D DWC_HYS_AIN7_8 [7:0] HYS_AIN7[7:0] (AIN7, local 6) (R/W) | S8.2.38 p79 | `Window.hysteresis` 0..255 (in top-byte LSBs) | T18 | OK |
| FLD-2-2E-15_7 | AIN1-8 0x2E TP_CFG [15:7] RESERVED (R/W) | S8.2.39 p79 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-2-2E-6_4 | AIN1-8 0x2E TP_CFG [6:4] TP_MODE[2:0] (R/W) | S8.2.39 p79 | `TestPattern.kind` (Constant 0, Ramp 2); 1 and 3 to 5 unreachable | T20 | GUARD |
| FLD-2-2E-3 | AIN1-8 0x2E TP_CFG [3] RESERVED (R/W) | S8.2.39 p79 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-2-2E-2 | AIN1-8 0x2E TP_CFG [2] TP_DIS_IDX (R/W) | S8.2.39 p79 | `TestPattern::index_insertion` (inverted) | T20 | OK |
| FLD-2-2E-1 | AIN1-8 0x2E TP_CFG [1] TP_UPD_MODE (R/W) | S8.2.39 p79 | `TestPattern::step_every_convst` | T20 | OK |
| FLD-2-2E-0 | AIN1-8 0x2E TP_CFG [0] TP_EN (R/W) | S8.2.39 p79 | `TestPattern.enabled` per group | T20 | OK |
| FLD-2-2F-15_0 | AIN1-8 0x2F TP_AIN1 [15:0] TP_AIN1[15:0] (AIN1, local 0) (R/W) | S8.2.40 p80 | `TestPattern.value[ch]`; in ramp mode the first channel's value is step-1 | T20 | OK |
| FLD-2-30-15_0 | AIN1-8 0x30 TP_AIN2 [15:0] TP_AIN2[15:0] (AIN2, local 1) (R/W) | S8.2.41 p80 | `TestPattern.value[ch]`; in ramp mode the first channel's value is step-1 | T20 | OK |
| FLD-2-31-15_0 | AIN1-8 0x31 TP_AIN3 [15:0] TP_AIN3[15:0] (AIN3, local 2) (R/W) | S8.2.42 p80 | `TestPattern.value[ch]`; in ramp mode the first channel's value is step-1 | T20 | OK |
| FLD-2-32-15_0 | AIN1-8 0x32 TP_AIN4 [15:0] TP_AIN4[15:0] (AIN4, local 3) (R/W) | S8.2.43 p81 | `TestPattern.value[ch]`; in ramp mode the first channel's value is step-1 | T20 | OK |
| FLD-2-33-15_0 | AIN1-8 0x33 TP_AIN5 [15:0] TP_AIN5[15:0] (AIN5, local 4) (R/W) | S8.2.44 p81 | `TestPattern.value[ch]`; in ramp mode the first channel's value is step-1 | T20 | OK |
| FLD-2-34-15_0 | AIN1-8 0x34 TP_AIN6 [15:0] TP_AIN6[15:0] (AIN6, local 5) (R/W) | S8.2.45 p81 | `TestPattern.value[ch]`; in ramp mode the first channel's value is step-1 | T20 | OK |
| FLD-2-35-15_0 | AIN1-8 0x35 TP_AIN7 [15:0] TP_AIN7[15:0] (AIN7, local 6) (R/W) | S8.2.46 p82 | `TestPattern.value[ch]`; in ramp mode the first channel's value is step-1 | T20 | OK |
| FLD-2-36-15_0 | AIN1-8 0x36 TP_AIN8 [15:0] TP_AIN8[15:0] (AIN8, local 7) (R/W) | S8.2.47 p82 | `TestPattern.value[ch]`; in ramp mode the first channel's value is step-1 | T20 | OK |
| FLD-2-37-15_5 | AIN1-8 0x37 GEN_CFG5 [15:5] RESERVED (R/W) | S8.2.48 p82 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-2-37-4 | AIN1-8 0x37 GEN_CFG5 [4] RESERVED (R/W) | S8.2.48 p82 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-2-37-3_2 | AIN1-8 0x37 GEN_CFG5 [3:2] RESERVED (R/W) | S8.2.48 p82 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-2-37-1 | AIN1-8 0x37 GEN_CFG5 [1] OFS_CORR_DIS (R/W) | S8.2.48 p82 | `Correction.offset_enabled` per group (inverted) | T14 | OK |
| FLD-2-37-0 | AIN1-8 0x37 GEN_CFG5 [0] GAN_CORR_DIS (R/W) | S8.2.48 p82 | `Correction.gain_enabled` per group (inverted) | T14 | OK |
| FLD-2-3E-15 | AIN1-8 0x3E DWC_FLAG_AIN1_8 [15] HIGH_FLAG_AIN8 (AIN8, local 7) (R) | S8.2.49 p83 | `ReadWindowFlags()` high mask | T18 | OK |
| FLD-2-3E-14 | AIN1-8 0x3E DWC_FLAG_AIN1_8 [14] HIGH_FLAG_AIN7 (AIN7, local 6) (R) | S8.2.49 p83 | `ReadWindowFlags()` high mask | T18 | OK |
| FLD-2-3E-13 | AIN1-8 0x3E DWC_FLAG_AIN1_8 [13] HIGH_FLAG_AIN6 (AIN6, local 5) (R) | S8.2.49 p83 | `ReadWindowFlags()` high mask | T18 | OK |
| FLD-2-3E-12 | AIN1-8 0x3E DWC_FLAG_AIN1_8 [12] HIGH_FLAG_AIN5 (AIN5, local 4) (R) | S8.2.49 p83 | `ReadWindowFlags()` high mask | T18 | OK |
| FLD-2-3E-11 | AIN1-8 0x3E DWC_FLAG_AIN1_8 [11] HIGH_FLAG_AIN4 (AIN4, local 3) (R) | S8.2.49 p83 | `ReadWindowFlags()` high mask | T18 | OK |
| FLD-2-3E-10 | AIN1-8 0x3E DWC_FLAG_AIN1_8 [10] HIGH_FLAG_AIN3 (AIN3, local 2) (R) | S8.2.49 p83 | `ReadWindowFlags()` high mask | T18 | OK |
| FLD-2-3E-9 | AIN1-8 0x3E DWC_FLAG_AIN1_8 [9] HIGH_FLAG_AIN2 (AIN2, local 1) (R) | S8.2.49 p83 | `ReadWindowFlags()` high mask | T18 | OK |
| FLD-2-3E-8 | AIN1-8 0x3E DWC_FLAG_AIN1_8 [8] HIGH_FLAG_AIN1 (AIN1, local 0) (R) | S8.2.49 p83 | `ReadWindowFlags()` high mask | T18 | OK |
| FLD-2-3E-7 | AIN1-8 0x3E DWC_FLAG_AIN1_8 [7] LOW_FLAG_AIN8 (AIN8, local 7) (R) | S8.2.49 p83 | `ReadWindowFlags()` low mask | T18 | OK |
| FLD-2-3E-6 | AIN1-8 0x3E DWC_FLAG_AIN1_8 [6] LOW_FLAG_AIN7 (AIN7, local 6) (R) | S8.2.49 p83 | `ReadWindowFlags()` low mask | T18 | OK |
| FLD-2-3E-5 | AIN1-8 0x3E DWC_FLAG_AIN1_8 [5] LOW_FLAG_AIN6 (AIN6, local 5) (R) | S8.2.49 p83 | `ReadWindowFlags()` low mask | T18 | OK |
| FLD-2-3E-4 | AIN1-8 0x3E DWC_FLAG_AIN1_8 [4] LOW_FLAG_AIN5 (AIN5, local 4) (R) | S8.2.49 p83 | `ReadWindowFlags()` low mask | T18 | OK |
| FLD-2-3E-3 | AIN1-8 0x3E DWC_FLAG_AIN1_8 [3] LOW_FLAG_AIN4 (AIN4, local 3) (R) | S8.2.49 p83 | `ReadWindowFlags()` low mask | T18 | OK |
| FLD-2-3E-2 | AIN1-8 0x3E DWC_FLAG_AIN1_8 [2] LOW_FLAG_AIN3 (AIN3, local 2) (R) | S8.2.49 p83 | `ReadWindowFlags()` low mask | T18 | OK |
| FLD-2-3E-1 | AIN1-8 0x3E DWC_FLAG_AIN1_8 [1] LOW_FLAG_AIN2 (AIN2, local 1) (R) | S8.2.49 p83 | `ReadWindowFlags()` low mask | T18 | OK |
| FLD-2-3E-0 | AIN1-8 0x3E DWC_FLAG_AIN1_8 [0] LOW_FLAG_AIN1 (AIN1, local 0) (R) | S8.2.49 p83 | `ReadWindowFlags()` low mask | T18 | OK |
| FLD-3-08-15 | AIN9-16 0x08 PGA_CONFIG_AIN15_16 [15] CME_CORR_EN_AIN15 (AIN15, local 1) (R/W) | S8.3.1 p86 | `Input::cm_correction` | T08 | OK |
| FLD-3-08-14_12 | AIN9-16 0x08 PGA_CONFIG_AIN15_16 [14:12] CM_RANGE_AIN15[2:0] (AIN15, local 1) (R/W) | S8.3.1 p86 | `Input.type` (Differential 0, SingleEnded 5, SingleEndedOpenWireSafe 6); others unreachable | T08,T09 | GUARD |
| FLD-3-08-11 | AIN9-16 0x08 PGA_CONFIG_AIN15_16 [11] RESERVED (R/W) | S8.3.1 p86 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-3-08-10_8 | AIN9-16 0x08 PGA_CONFIG_AIN15_16 [10:8] INPUT_RANGE_AIN15[2:0] (AIN15, local 1) (R/W) | S8.3.1 p86 | `Input.range` (V5 0, V2_5 2, V6_25 3, V10 4, V12_5 5); 1, 6, 7 unreachable | T08 | GUARD |
| FLD-3-08-7 | AIN9-16 0x08 PGA_CONFIG_AIN15_16 [7] CME_CORR_EN_AIN16 (AIN16, local 0) (R/W) | S8.3.1 p86 | `Input::cm_correction` | T08 | OK |
| FLD-3-08-6_4 | AIN9-16 0x08 PGA_CONFIG_AIN15_16 [6:4] CM_RANGE_AIN16[2:0] (AIN16, local 0) (R/W) | S8.3.1 p86 | `Input.type` (Differential 0, SingleEnded 5, SingleEndedOpenWireSafe 6); others unreachable | T08,T09 | GUARD |
| FLD-3-08-3 | AIN9-16 0x08 PGA_CONFIG_AIN15_16 [3] RESERVED (R/W) | S8.3.1 p86 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-3-08-2_0 | AIN9-16 0x08 PGA_CONFIG_AIN15_16 [2:0] INPUT_RANGE_AIN16[2:0] (AIN16, local 0) (R/W) | S8.3.1 p86 | `Input.range` (V5 0, V2_5 2, V6_25 3, V10 4, V12_5 5); 1, 6, 7 unreachable | T08 | GUARD |
| FLD-3-09-15 | AIN9-16 0x09 PGA_CONFIG_AIN13_14 [15] CME_CORR_EN_AIN13 (AIN13, local 3) (R/W) | S8.3.2 p87 | `Input::cm_correction` | T08 | OK |
| FLD-3-09-14_12 | AIN9-16 0x09 PGA_CONFIG_AIN13_14 [14:12] CM_RANGE_AIN13[2:0] (AIN13, local 3) (R/W) | S8.3.2 p87 | `Input.type` (Differential 0, SingleEnded 5, SingleEndedOpenWireSafe 6); others unreachable | T08,T09 | GUARD |
| FLD-3-09-11 | AIN9-16 0x09 PGA_CONFIG_AIN13_14 [11] RESERVED (R/W) | S8.3.2 p87 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-3-09-10_8 | AIN9-16 0x09 PGA_CONFIG_AIN13_14 [10:8] INPUT_RANGE_AIN13[2:0] (AIN13, local 3) (R/W) | S8.3.2 p87 | `Input.range` (V5 0, V2_5 2, V6_25 3, V10 4, V12_5 5); 1, 6, 7 unreachable | T08 | GUARD |
| FLD-3-09-7 | AIN9-16 0x09 PGA_CONFIG_AIN13_14 [7] CME_CORR_EN_AIN14 (AIN14, local 2) (R/W) | S8.3.2 p87 | `Input::cm_correction` | T08 | OK |
| FLD-3-09-6_4 | AIN9-16 0x09 PGA_CONFIG_AIN13_14 [6:4] CM_RANGE_AIN14[2:0] (AIN14, local 2) (R/W) | S8.3.2 p87 | `Input.type` (Differential 0, SingleEnded 5, SingleEndedOpenWireSafe 6); others unreachable | T08,T09 | GUARD |
| FLD-3-09-3 | AIN9-16 0x09 PGA_CONFIG_AIN13_14 [3] RESERVED (R/W) | S8.3.2 p87 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-3-09-2_0 | AIN9-16 0x09 PGA_CONFIG_AIN13_14 [2:0] INPUT_RANGE_AIN14[2:0] (AIN14, local 2) (R/W) | S8.3.2 p87 | `Input.range` (V5 0, V2_5 2, V6_25 3, V10 4, V12_5 5); 1, 6, 7 unreachable | T08 | GUARD |
| FLD-3-0A-15 | AIN9-16 0x0A PGA_CONFIG_AIN11_12 [15] CME_CORR_EN_AIN11 (AIN11, local 5) (R/W) | S8.3.3 p88 | `Input::cm_correction` | T08 | OK |
| FLD-3-0A-14_12 | AIN9-16 0x0A PGA_CONFIG_AIN11_12 [14:12] CM_RANGE_AIN11[2:0] (AIN11, local 5) (R/W) | S8.3.3 p88 | `Input.type` (Differential 0, SingleEnded 5, SingleEndedOpenWireSafe 6); others unreachable | T08,T09 | GUARD |
| FLD-3-0A-11 | AIN9-16 0x0A PGA_CONFIG_AIN11_12 [11] RESERVED (R/W) | S8.3.3 p88 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-3-0A-10_8 | AIN9-16 0x0A PGA_CONFIG_AIN11_12 [10:8] INPUT_RANGE_AIN11[2:0] (AIN11, local 5) (R/W) | S8.3.3 p88 | `Input.range` (V5 0, V2_5 2, V6_25 3, V10 4, V12_5 5); 1, 6, 7 unreachable | T08 | GUARD |
| FLD-3-0A-7 | AIN9-16 0x0A PGA_CONFIG_AIN11_12 [7] CME_CORR_EN_AIN12 (AIN12, local 4) (R/W) | S8.3.3 p88 | `Input::cm_correction` | T08 | OK |
| FLD-3-0A-6_4 | AIN9-16 0x0A PGA_CONFIG_AIN11_12 [6:4] CM_RANGE_AIN12[2:0] (AIN12, local 4) (R/W) | S8.3.3 p88 | `Input.type` (Differential 0, SingleEnded 5, SingleEndedOpenWireSafe 6); others unreachable | T08,T09 | GUARD |
| FLD-3-0A-3 | AIN9-16 0x0A PGA_CONFIG_AIN11_12 [3] RESERVED (R/W) | S8.3.3 p88 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-3-0A-2_0 | AIN9-16 0x0A PGA_CONFIG_AIN11_12 [2:0] INPUT_RANGE_AIN12[2:0] (AIN12, local 4) (R/W) | S8.3.3 p88 | `Input.range` (V5 0, V2_5 2, V6_25 3, V10 4, V12_5 5); 1, 6, 7 unreachable | T08 | GUARD |
| FLD-3-0B-15 | AIN9-16 0x0B PGA_CONFIG_AIN9_10 [15] CME_CORR_EN_AIN9 (AIN9, local 7) (R/W) | S8.3.4 p88 | `Input::cm_correction` | T08 | OK |
| FLD-3-0B-14_12 | AIN9-16 0x0B PGA_CONFIG_AIN9_10 [14:12] CM_RANGE_AIN9[2:0] (AIN9, local 7) (R/W) | S8.3.4 p88 | `Input.type` (Differential 0, SingleEnded 5, SingleEndedOpenWireSafe 6); others unreachable | T08,T09 | GUARD |
| FLD-3-0B-11 | AIN9-16 0x0B PGA_CONFIG_AIN9_10 [11] RESERVED (R/W) | S8.3.4 p88 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-3-0B-10_8 | AIN9-16 0x0B PGA_CONFIG_AIN9_10 [10:8] INPUT_RANGE_AIN9[2:0] (AIN9, local 7) (R/W) | S8.3.4 p88 | `Input.range` (V5 0, V2_5 2, V6_25 3, V10 4, V12_5 5); 1, 6, 7 unreachable | T08 | GUARD |
| FLD-3-0B-7 | AIN9-16 0x0B PGA_CONFIG_AIN9_10 [7] CME_CORR_EN_AIN10 (AIN10, local 6) (R/W) | S8.3.4 p88 | `Input::cm_correction` | T08 | OK |
| FLD-3-0B-6_4 | AIN9-16 0x0B PGA_CONFIG_AIN9_10 [6:4] CM_RANGE_AIN10[2:0] (AIN10, local 6) (R/W) | S8.3.4 p88 | `Input.type` (Differential 0, SingleEnded 5, SingleEndedOpenWireSafe 6); others unreachable | T08,T09 | GUARD |
| FLD-3-0B-3 | AIN9-16 0x0B PGA_CONFIG_AIN9_10 [3] RESERVED (R/W) | S8.3.4 p88 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-3-0B-2_0 | AIN9-16 0x0B PGA_CONFIG_AIN9_10 [2:0] INPUT_RANGE_AIN10[2:0] (AIN10, local 6) (R/W) | S8.3.4 p88 | `Input.range` (V5 0, V2_5 2, V6_25 3, V10 4, V12_5 5); 1, 6, 7 unreachable | T08 | GUARD |
| FLD-3-0C-15_14 | AIN9-16 0x0C PGA_BW_SEL_AIN9_16 [15:14] PGA_BW_SEL_AIN9[1:0] (AIN9, local 7) (R/W) | S8.3.5 p89 | `Input.bandwidth` (Low 0, Wide 1); 2, 3 unreachable | T08 | GUARD |
| FLD-3-0C-13_12 | AIN9-16 0x0C PGA_BW_SEL_AIN9_16 [13:12] PGA_BW_SEL_AIN10[1:0] (AIN10, local 6) (R/W) | S8.3.5 p89 | `Input.bandwidth` (Low 0, Wide 1); 2, 3 unreachable | T08 | GUARD |
| FLD-3-0C-11_10 | AIN9-16 0x0C PGA_BW_SEL_AIN9_16 [11:10] PGA_BW_SEL_AIN11[1:0] (AIN11, local 5) (R/W) | S8.3.5 p89 | `Input.bandwidth` (Low 0, Wide 1); 2, 3 unreachable | T08 | GUARD |
| FLD-3-0C-9_8 | AIN9-16 0x0C PGA_BW_SEL_AIN9_16 [9:8] PGA_BW_SEL_AIN12[1:0] (AIN12, local 4) (R/W) | S8.3.5 p89 | `Input.bandwidth` (Low 0, Wide 1); 2, 3 unreachable | T08 | GUARD |
| FLD-3-0C-7_6 | AIN9-16 0x0C PGA_BW_SEL_AIN9_16 [7:6] PGA_BW_SEL_AIN13[1:0] (AIN13, local 3) (R/W) | S8.3.5 p89 | `Input.bandwidth` (Low 0, Wide 1); 2, 3 unreachable | T08 | GUARD |
| FLD-3-0C-5_4 | AIN9-16 0x0C PGA_BW_SEL_AIN9_16 [5:4] PGA_BW_SEL_AIN14[1:0] (AIN14, local 2) (R/W) | S8.3.5 p89 | `Input.bandwidth` (Low 0, Wide 1); 2, 3 unreachable | T08 | GUARD |
| FLD-3-0C-3_2 | AIN9-16 0x0C PGA_BW_SEL_AIN9_16 [3:2] PGA_BW_SEL_AIN15[1:0] (AIN15, local 1) (R/W) | S8.3.5 p89 | `Input.bandwidth` (Low 0, Wide 1); 2, 3 unreachable | T08 | GUARD |
| FLD-3-0C-1_0 | AIN9-16 0x0C PGA_BW_SEL_AIN9_16 [1:0] PGA_BW_SEL_AIN16[1:0] (AIN16, local 0) (R/W) | S8.3.5 p89 | `Input.bandwidth` (Low 0, Wide 1); 2, 3 unreachable | T08 | GUARD |
| FLD-3-0D-15_8 | AIN9-16 0x0D PHASE_DELAY_AIN15_16 [15:8] PHASE_DELAY_AIN15[7:0] (AIN15, local 1) (R/W) | S8.3.6 p90 | `SetInput`, `Input::phase_delay` | T17 | OK |
| FLD-3-0D-7_0 | AIN9-16 0x0D PHASE_DELAY_AIN15_16 [7:0] PHASE_DELAY_AIN16[7:0] (AIN16, local 0) (R/W) | S8.3.6 p90 | `SetInput`, `Input::phase_delay` | T17 | OK |
| FLD-3-0E-15_8 | AIN9-16 0x0E PHASE_DELAY_AIN13_14 [15:8] PHASE_DELAY_AIN13[7:0] (AIN13, local 3) (R/W) | S8.3.7 p90 | `SetInput`, `Input::phase_delay` | T17 | OK |
| FLD-3-0E-7_0 | AIN9-16 0x0E PHASE_DELAY_AIN13_14 [7:0] PHASE_DELAY_AIN14[7:0] (AIN14, local 2) (R/W) | S8.3.7 p90 | `SetInput`, `Input::phase_delay` | T17 | OK |
| FLD-3-0F-15_8 | AIN9-16 0x0F PHASE_DELAY_AIN11_12 [15:8] PHASE_DELAY_AIN11[7:0] (AIN11, local 5) (R/W) | S8.3.8 p90 | `SetInput`, `Input::phase_delay` | T17 | OK |
| FLD-3-0F-7_0 | AIN9-16 0x0F PHASE_DELAY_AIN11_12 [7:0] PHASE_DELAY_AIN12[7:0] (AIN12, local 4) (R/W) | S8.3.8 p90 | `SetInput`, `Input::phase_delay` | T17 | OK |
| FLD-3-10-15_8 | AIN9-16 0x10 PHASE_DELAY_AIN9_10 [15:8] PHASE_DELAY_AIN9[7:0] (AIN9, local 7) (R/W) | S8.3.9 p91 | `SetInput`, `Input::phase_delay` | T17 | OK |
| FLD-3-10-7_0 | AIN9-16 0x10 PHASE_DELAY_AIN9_10 [7:0] PHASE_DELAY_AIN10[7:0] (AIN10, local 6) (R/W) | S8.3.9 p91 | `SetInput`, `Input::phase_delay` | T17 | OK |
| FLD-3-11-15_10 | AIN9-16 0x11 OFS_AIN16 [15:10] RESERVED (R/W) | S8.3.10 p91 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-3-11-9_0 | AIN9-16 0x11 OFS_AIN16 [9:0] OFS_AIN16[9:0] (AIN16, local 0) (R/W) | S8.3.10 p91 | `SetInput`, `Input::offset_quarter_lsb` quarter-LSB16; helper `OffsetCorrectionFor` | T14 | OK |
| FLD-3-12-15_10 | AIN9-16 0x12 OFS_AIN15 [15:10] RESERVED (R/W) | S8.3.11 p91 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-3-12-9_0 | AIN9-16 0x12 OFS_AIN15 [9:0] OFS_AIN15[9:0] (AIN15, local 1) (R/W) | S8.3.11 p91 | `SetInput`, `Input::offset_quarter_lsb` quarter-LSB16; helper `OffsetCorrectionFor` | T14 | OK |
| FLD-3-13-15_10 | AIN9-16 0x13 OFS_AIN14 [15:10] RESERVED (R/W) | S8.3.12 p92 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-3-13-9_0 | AIN9-16 0x13 OFS_AIN14 [9:0] OFS_AIN14[9:0] (AIN14, local 2) (R/W) | S8.3.12 p92 | `SetInput`, `Input::offset_quarter_lsb` quarter-LSB16; helper `OffsetCorrectionFor` | T14 | OK |
| FLD-3-14-15_10 | AIN9-16 0x14 OFS_AIN13 [15:10] RESERVED (R/W) | S8.3.13 p92 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-3-14-9_0 | AIN9-16 0x14 OFS_AIN13 [9:0] OFS_AIN13[9:0] (AIN13, local 3) (R/W) | S8.3.13 p92 | `SetInput`, `Input::offset_quarter_lsb` quarter-LSB16; helper `OffsetCorrectionFor` | T14 | OK |
| FLD-3-15-15_10 | AIN9-16 0x15 OFS_AIN12 [15:10] RESERVED (R/W) | S8.3.14 p92 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-3-15-9_0 | AIN9-16 0x15 OFS_AIN12 [9:0] OFS_AIN12[9:0] (AIN12, local 4) (R/W) | S8.3.14 p92 | `SetInput`, `Input::offset_quarter_lsb` quarter-LSB16; helper `OffsetCorrectionFor` | T14 | OK |
| FLD-3-16-15_10 | AIN9-16 0x16 OFS_AIN11 [15:10] RESERVED (R/W) | S8.3.15 p93 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-3-16-9_0 | AIN9-16 0x16 OFS_AIN11 [9:0] OFS_AIN11[9:0] (AIN11, local 5) (R/W) | S8.3.15 p93 | `SetInput`, `Input::offset_quarter_lsb` quarter-LSB16; helper `OffsetCorrectionFor` | T14 | OK |
| FLD-3-17-15_10 | AIN9-16 0x17 OFS_AIN10 [15:10] RESERVED (R/W) | S8.3.16 p93 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-3-17-9_0 | AIN9-16 0x17 OFS_AIN10 [9:0] OFS_AIN10[9:0] (AIN10, local 6) (R/W) | S8.3.16 p93 | `SetInput`, `Input::offset_quarter_lsb` quarter-LSB16; helper `OffsetCorrectionFor` | T14 | OK |
| FLD-3-18-15_10 | AIN9-16 0x18 OFS_AIN9 [15:10] RESERVED (R/W) | S8.3.17 p93 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-3-18-9_0 | AIN9-16 0x18 OFS_AIN9 [9:0] OFS_AIN9[9:0] (AIN9, local 7) (R/W) | S8.3.17 p93 | `SetInput`, `Input::offset_quarter_lsb` quarter-LSB16; helper `OffsetCorrectionFor` | T14 | OK |
| FLD-3-19-15_14 | AIN9-16 0x19 GAN_AIN16 [15:14] RESERVED (R/W) | S8.3.18 p94 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-3-19-13_0 | AIN9-16 0x19 GAN_AIN16 [13:0] GAN_AIN16[13:0] (AIN16, local 0) (R/W) | S8.3.18 p94 | `SetInput`, `Input::gain`; helper `GainCorrectionFor` | T14 | OK |
| FLD-3-1A-15_14 | AIN9-16 0x1A GAN_AIN15 [15:14] RESERVED (R/W) | S8.3.19 p94 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-3-1A-13_0 | AIN9-16 0x1A GAN_AIN15 [13:0] GAN_AIN15[13:0] (AIN15, local 1) (R/W) | S8.3.19 p94 | `SetInput`, `Input::gain`; helper `GainCorrectionFor` | T14 | OK |
| FLD-3-1B-15_14 | AIN9-16 0x1B GAN_AIN14 [15:14] RESERVED (R/W) | S8.3.20 p95 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-3-1B-13_0 | AIN9-16 0x1B GAN_AIN14 [13:0] GAN_AIN14[13:0] (AIN14, local 2) (R/W) | S8.3.20 p95 | `SetInput`, `Input::gain`; helper `GainCorrectionFor` | T14 | OK |
| FLD-3-1C-15_14 | AIN9-16 0x1C GAN_AIN13 [15:14] RESERVED (R/W) | S8.3.21 p95 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-3-1C-13_0 | AIN9-16 0x1C GAN_AIN13 [13:0] GAN_AIN13[13:0] (AIN13, local 3) (R/W) | S8.3.21 p95 | `SetInput`, `Input::gain`; helper `GainCorrectionFor` | T14 | OK |
| FLD-3-1D-15_14 | AIN9-16 0x1D GAN_AIN12 [15:14] RESERVED (R/W) | S8.3.22 p95 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-3-1D-13_0 | AIN9-16 0x1D GAN_AIN12 [13:0] GAN_AIN12[13:0] (AIN12, local 4) (R/W) | S8.3.22 p95 | `SetInput`, `Input::gain`; helper `GainCorrectionFor` | T14 | OK |
| FLD-3-1E-15_14 | AIN9-16 0x1E GAN_AIN11 [15:14] RESERVED (R/W) | S8.3.23 p96 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-3-1E-13_0 | AIN9-16 0x1E GAN_AIN11 [13:0] GAN_AIN11[13:0] (AIN11, local 5) (R/W) | S8.3.23 p96 | `SetInput`, `Input::gain`; helper `GainCorrectionFor` | T14 | OK |
| FLD-3-1F-15_14 | AIN9-16 0x1F GAN_AIN10 [15:14] RESERVED (R/W) | S8.3.24 p96 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-3-1F-13_0 | AIN9-16 0x1F GAN_AIN10 [13:0] GAN_AIN10[13:0] (AIN10, local 6) (R/W) | S8.3.24 p96 | `SetInput`, `Input::gain`; helper `GainCorrectionFor` | T14 | OK |
| FLD-3-20-15_14 | AIN9-16 0x20 GAN_AIN9 [15:14] RESERVED (R/W) | S8.3.25 p96 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-3-20-13_0 | AIN9-16 0x20 GAN_AIN9 [13:0] GAN_AIN9[13:0] (AIN9, local 7) (R/W) | S8.3.25 p96 | `SetInput`, `Input::gain`; helper `GainCorrectionFor` | T14 | OK |
| FLD-3-21-15 | AIN9-16 0x21 DWC_CFG [15] DWC_STAT_RST (R/W) | S8.3.26 p97 | `ClearWindowFlags(group)` writes 1 then 0 (edge) | T18 | OK |
| FLD-3-21-14_12 | AIN9-16 0x21 DWC_CFG [14:12] RESERVED (R/W) | S8.3.26 p97 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-3-21-11_8 | AIN9-16 0x21 DWC_CFG [11:8] DWC_GLITCH_FILT[3:0] (R/W) | S8.3.26 p97 | `Config::window_glitch` | T18 | OK |
| FLD-3-21-7 | AIN9-16 0x21 DWC_CFG [7] DWC_EN_AIN9 (AIN9, local 7) (R/W) | S8.3.26 p97 | `Window.enabled`; thresholds written before the enable bit | T18 | OK |
| FLD-3-21-6 | AIN9-16 0x21 DWC_CFG [6] DWC_EN_AIN10 (AIN10, local 6) (R/W) | S8.3.26 p97 | `Window.enabled`; thresholds written before the enable bit | T18 | OK |
| FLD-3-21-5 | AIN9-16 0x21 DWC_CFG [5] DWC_EN_AIN11 (AIN11, local 5) (R/W) | S8.3.26 p97 | `Window.enabled`; thresholds written before the enable bit | T18 | OK |
| FLD-3-21-4 | AIN9-16 0x21 DWC_CFG [4] DWC_EN_AIN12 (AIN12, local 4) (R/W) | S8.3.26 p97 | `Window.enabled`; thresholds written before the enable bit | T18 | OK |
| FLD-3-21-3 | AIN9-16 0x21 DWC_CFG [3] DWC_EN_AIN13 (AIN13, local 3) (R/W) | S8.3.26 p97 | `Window.enabled`; thresholds written before the enable bit | T18 | OK |
| FLD-3-21-2 | AIN9-16 0x21 DWC_CFG [2] DWC_EN_AIN14 (AIN14, local 2) (R/W) | S8.3.26 p97 | `Window.enabled`; thresholds written before the enable bit | T18 | OK |
| FLD-3-21-1 | AIN9-16 0x21 DWC_CFG [1] DWC_EN_AIN15 (AIN15, local 1) (R/W) | S8.3.26 p97 | `Window.enabled`; thresholds written before the enable bit | T18 | OK |
| FLD-3-21-0 | AIN9-16 0x21 DWC_CFG [0] DWC_EN_AIN16 (AIN16, local 0) (R/W) | S8.3.26 p97 | `Window.enabled`; thresholds written before the enable bit | T18 | OK |
| FLD-3-22-15_8 | AIN9-16 0x22 DWC_TH_AIN16 [15:8] HIGH_TH_AIN16[7:0] (AIN16, local 0) (R/W) | S8.3.27 p97 | `Window.high_code` -> top byte in the output format (A16) | T18 | OK |
| FLD-3-22-7_0 | AIN9-16 0x22 DWC_TH_AIN16 [7:0] LOW_TH_AIN16[7:0] (AIN16, local 0) (R/W) | S8.3.27 p97 | `Window.low_code` -> top byte in the output format (A16) | T18 | OK |
| FLD-3-23-15_8 | AIN9-16 0x23 DWC_TH_AIN15 [15:8] HIGH_TH_AIN15[7:0] (AIN15, local 1) (R/W) | S8.3.28 p98 | `Window.high_code` -> top byte in the output format (A16) | T18 | OK |
| FLD-3-23-7_0 | AIN9-16 0x23 DWC_TH_AIN15 [7:0] LOW_TH_AIN15[7:0] (AIN15, local 1) (R/W) | S8.3.28 p98 | `Window.low_code` -> top byte in the output format (A16) | T18 | OK |
| FLD-3-24-15_8 | AIN9-16 0x24 DWC_TH_AIN14 [15:8] HIGH_TH_AIN14[7:0] (AIN14, local 2) (R/W) | S8.3.29 p98 | `Window.high_code` -> top byte in the output format (A16) | T18 | OK |
| FLD-3-24-7_0 | AIN9-16 0x24 DWC_TH_AIN14 [7:0] LOW_TH_AIN14[7:0] (AIN14, local 2) (R/W) | S8.3.29 p98 | `Window.low_code` -> top byte in the output format (A16) | T18 | OK |
| FLD-3-25-15_8 | AIN9-16 0x25 DWC_TH_AIN13 [15:8] HIGH_TH_AIN13[7:0] (AIN13, local 3) (R/W) | S8.3.30 p98 | `Window.high_code` -> top byte in the output format (A16) | T18 | OK |
| FLD-3-25-7_0 | AIN9-16 0x25 DWC_TH_AIN13 [7:0] LOW_TH_AIN13[7:0] (AIN13, local 3) (R/W) | S8.3.30 p98 | `Window.low_code` -> top byte in the output format (A16) | T18 | OK |
| FLD-3-26-15_8 | AIN9-16 0x26 DWC_TH_AIN12 [15:8] HIGH_TH_AIN12[7:0] (AIN12, local 4) (R/W) | S8.3.31 p99 | `Window.high_code` -> top byte in the output format (A16) | T18 | OK |
| FLD-3-26-7_0 | AIN9-16 0x26 DWC_TH_AIN12 [7:0] LOW_TH_AIN12[7:0] (AIN12, local 4) (R/W) | S8.3.31 p99 | `Window.low_code` -> top byte in the output format (A16) | T18 | OK |
| FLD-3-27-15_8 | AIN9-16 0x27 DWC_TH_AIN11 [15:8] HIGH_TH_AIN11[7:0] (AIN11, local 5) (R/W) | S8.3.32 p99 | `Window.high_code` -> top byte in the output format (A16) | T18 | OK |
| FLD-3-27-7_0 | AIN9-16 0x27 DWC_TH_AIN11 [7:0] LOW_TH_AIN11[7:0] (AIN11, local 5) (R/W) | S8.3.32 p99 | `Window.low_code` -> top byte in the output format (A16) | T18 | OK |
| FLD-3-28-15_8 | AIN9-16 0x28 DWC_TH_AIN10 [15:8] HIGH_TH_AIN10[7:0] (AIN10, local 6) (R/W) | S8.3.33 p99 | `Window.high_code` -> top byte in the output format (A16) | T18 | OK |
| FLD-3-28-7_0 | AIN9-16 0x28 DWC_TH_AIN10 [7:0] LOW_TH_AIN10[7:0] (AIN10, local 6) (R/W) | S8.3.33 p99 | `Window.low_code` -> top byte in the output format (A16) | T18 | OK |
| FLD-3-29-15_8 | AIN9-16 0x29 DWC_TH_AIN9 [15:8] HIGH_TH_AIN9[7:0] (AIN9, local 7) (R/W) | S8.3.34 p100 | `Window.high_code` -> top byte in the output format (A16) | T18 | OK |
| FLD-3-29-7_0 | AIN9-16 0x29 DWC_TH_AIN9 [7:0] LOW_TH_AIN9[7:0] (AIN9, local 7) (R/W) | S8.3.34 p100 | `Window.low_code` -> top byte in the output format (A16) | T18 | OK |
| FLD-3-2A-15_8 | AIN9-16 0x2A DWC_HYS_AIN15_16 [15:8] HYS_AIN15[7:0] (AIN15, local 1) (R/W) | S8.3.35 p100 | `Window.hysteresis` 0..255 (in top-byte LSBs) | T18 | OK |
| FLD-3-2A-7_0 | AIN9-16 0x2A DWC_HYS_AIN15_16 [7:0] HYS_AIN16[7:0] (AIN16, local 0) (R/W) | S8.3.35 p100 | `Window.hysteresis` 0..255 (in top-byte LSBs) | T18 | OK |
| FLD-3-2B-15_8 | AIN9-16 0x2B DWC_HYS_AIN13_14 [15:8] HYS_AIN13[7:0] (AIN13, local 3) (R/W) | S8.3.36 p100 | `Window.hysteresis` 0..255 (in top-byte LSBs) | T18 | OK |
| FLD-3-2B-7_0 | AIN9-16 0x2B DWC_HYS_AIN13_14 [7:0] HYS_AIN14[7:0] (AIN14, local 2) (R/W) | S8.3.36 p100 | `Window.hysteresis` 0..255 (in top-byte LSBs) | T18 | OK |
| FLD-3-2C-15_8 | AIN9-16 0x2C DWC_HYS_AIN11_12 [15:8] HYS_AIN11[7:0] (AIN11, local 5) (R/W) | S8.3.37 p101 | `Window.hysteresis` 0..255 (in top-byte LSBs) | T18 | OK |
| FLD-3-2C-7_0 | AIN9-16 0x2C DWC_HYS_AIN11_12 [7:0] HYS_AIN12[7:0] (AIN12, local 4) (R/W) | S8.3.37 p101 | `Window.hysteresis` 0..255 (in top-byte LSBs) | T18 | OK |
| FLD-3-2D-15_8 | AIN9-16 0x2D DWC_HYS_AIN9_10 [15:8] HYS_AIN9[7:0] (AIN9, local 7) (R/W) | S8.3.38 p101 | `Window.hysteresis` 0..255 (in top-byte LSBs) | T18 | OK |
| FLD-3-2D-7_0 | AIN9-16 0x2D DWC_HYS_AIN9_10 [7:0] HYS_AIN10[7:0] (AIN10, local 6) (R/W) | S8.3.38 p101 | `Window.hysteresis` 0..255 (in top-byte LSBs) | T18 | OK |
| FLD-3-2E-15_7 | AIN9-16 0x2E TP_CFG [15:7] RESERVED (R/W) | S8.3.39 p101 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-3-2E-6_4 | AIN9-16 0x2E TP_CFG [6:4] TP_MODE[2:0] (R/W) | S8.3.39 p101 | `TestPattern.kind` (Constant 0, Ramp 2); 1 and 3 to 5 unreachable | T20 | GUARD |
| FLD-3-2E-3 | AIN9-16 0x2E TP_CFG [3] RESERVED (R/W) | S8.3.39 p101 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-3-2E-2 | AIN9-16 0x2E TP_CFG [2] TP_DIS_IDX (R/W) | S8.3.39 p101 | `TestPattern::index_insertion` (inverted) | T20 | OK |
| FLD-3-2E-1 | AIN9-16 0x2E TP_CFG [1] TP_UPD_MODE (R/W) | S8.3.39 p101 | `TestPattern::step_every_convst` | T20 | OK |
| FLD-3-2E-0 | AIN9-16 0x2E TP_CFG [0] TP_EN (R/W) | S8.3.39 p101 | `TestPattern.enabled` per group | T20 | OK |
| FLD-3-2F-15_0 | AIN9-16 0x2F TP_AIN16 [15:0] TP_AIN16[15:0] (AIN16, local 0) (R/W) | S8.3.40 p102 | `TestPattern.value[ch]`; in ramp mode the first channel's value is step-1 | T20 | OK |
| FLD-3-30-15_0 | AIN9-16 0x30 TP_AIN15 [15:0] TP_AIN15[15:0] (AIN15, local 1) (R/W) | S8.3.41 p102 | `TestPattern.value[ch]`; in ramp mode the first channel's value is step-1 | T20 | OK |
| FLD-3-31-15_0 | AIN9-16 0x31 TP_AIN14 [15:0] TP_AIN14[15:0] (AIN14, local 2) (R/W) | S8.3.42 p103 | `TestPattern.value[ch]`; in ramp mode the first channel's value is step-1 | T20 | OK |
| FLD-3-32-15_0 | AIN9-16 0x32 TP_AIN13 [15:0] TP_AIN13[15:0] (AIN13, local 3) (R/W) | S8.3.43 p103 | `TestPattern.value[ch]`; in ramp mode the first channel's value is step-1 | T20 | OK |
| FLD-3-33-15_0 | AIN9-16 0x33 TP_AIN12 [15:0] TP_AIN12[15:0] (AIN12, local 4) (R/W) | S8.3.44 p103 | `TestPattern.value[ch]`; in ramp mode the first channel's value is step-1 | T20 | OK |
| FLD-3-34-15_0 | AIN9-16 0x34 TP_AIN11 [15:0] TP_AIN11[15:0] (AIN11, local 5) (R/W) | S8.3.45 p103 | `TestPattern.value[ch]`; in ramp mode the first channel's value is step-1 | T20 | OK |
| FLD-3-35-15_0 | AIN9-16 0x35 TP_AIN10 [15:0] TP_AIN10[15:0] (AIN10, local 6) (R/W) | S8.3.46 p104 | `TestPattern.value[ch]`; in ramp mode the first channel's value is step-1 | T20 | OK |
| FLD-3-36-15_0 | AIN9-16 0x36 TP_AIN9 [15:0] TP_AIN9[15:0] (AIN9, local 7) (R/W) | S8.3.47 p104 | `TestPattern.value[ch]`; in ramp mode the first channel's value is step-1 | T20 | OK |
| FLD-3-37-15_5 | AIN9-16 0x37 GEN_CFG5 [15:5] RESERVED (R/W) | S8.3.48 p104 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-3-37-4 | AIN9-16 0x37 GEN_CFG5 [4] RESERVED (R/W) | S8.3.48 p104 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-3-37-3_2 | AIN9-16 0x37 GEN_CFG5 [3:2] RESERVED (R/W) | S8.3.48 p104 | never set: the shadow keeps the reset value 0; written as 0 | T03 | OK |
| FLD-3-37-1 | AIN9-16 0x37 GEN_CFG5 [1] OFS_CORR_DIS (R/W) | S8.3.48 p104 | `Correction.offset_enabled` per group (inverted) | T14 | OK |
| FLD-3-37-0 | AIN9-16 0x37 GEN_CFG5 [0] GAN_CORR_DIS (R/W) | S8.3.48 p104 | `Correction.gain_enabled` per group (inverted) | T14 | OK |
| FLD-3-3E-15 | AIN9-16 0x3E DWC_FLAG_AIN9_16 [15] HIGH_FLAG_AIN9 (AIN9, local 7) (R) | S8.3.49 p105 | `ReadWindowFlags()` high mask | T18 | OK |
| FLD-3-3E-14 | AIN9-16 0x3E DWC_FLAG_AIN9_16 [14] HIGH_FLAG_AIN10 (AIN10, local 6) (R) | S8.3.49 p105 | `ReadWindowFlags()` high mask | T18 | OK |
| FLD-3-3E-13 | AIN9-16 0x3E DWC_FLAG_AIN9_16 [13] HIGH_FLAG_AIN11 (AIN11, local 5) (R) | S8.3.49 p105 | `ReadWindowFlags()` high mask | T18 | OK |
| FLD-3-3E-12 | AIN9-16 0x3E DWC_FLAG_AIN9_16 [12] HIGH_FLAG_AIN12 (AIN12, local 4) (R) | S8.3.49 p105 | `ReadWindowFlags()` high mask | T18 | OK |
| FLD-3-3E-11 | AIN9-16 0x3E DWC_FLAG_AIN9_16 [11] HIGH_FLAG_AIN13 (AIN13, local 3) (R) | S8.3.49 p105 | `ReadWindowFlags()` high mask | T18 | OK |
| FLD-3-3E-10 | AIN9-16 0x3E DWC_FLAG_AIN9_16 [10] HIGH_FLAG_AIN14 (AIN14, local 2) (R) | S8.3.49 p105 | `ReadWindowFlags()` high mask | T18 | OK |
| FLD-3-3E-9 | AIN9-16 0x3E DWC_FLAG_AIN9_16 [9] HIGH_FLAG_AIN15 (AIN15, local 1) (R) | S8.3.49 p105 | `ReadWindowFlags()` high mask | T18 | OK |
| FLD-3-3E-8 | AIN9-16 0x3E DWC_FLAG_AIN9_16 [8] HIGH_FLAG_AIN16 (AIN16, local 0) (R) | S8.3.49 p105 | `ReadWindowFlags()` high mask | T18 | OK |
| FLD-3-3E-7 | AIN9-16 0x3E DWC_FLAG_AIN9_16 [7] LOW_FLAG_AIN9 (AIN9, local 7) (R) | S8.3.49 p105 | `ReadWindowFlags()` low mask | T18 | OK |
| FLD-3-3E-6 | AIN9-16 0x3E DWC_FLAG_AIN9_16 [6] LOW_FLAG_AIN10 (AIN10, local 6) (R) | S8.3.49 p105 | `ReadWindowFlags()` low mask | T18 | OK |
| FLD-3-3E-5 | AIN9-16 0x3E DWC_FLAG_AIN9_16 [5] LOW_FLAG_AIN11 (AIN11, local 5) (R) | S8.3.49 p105 | `ReadWindowFlags()` low mask | T18 | OK |
| FLD-3-3E-4 | AIN9-16 0x3E DWC_FLAG_AIN9_16 [4] LOW_FLAG_AIN12 (AIN12, local 4) (R) | S8.3.49 p105 | `ReadWindowFlags()` low mask | T18 | OK |
| FLD-3-3E-3 | AIN9-16 0x3E DWC_FLAG_AIN9_16 [3] LOW_FLAG_AIN13 (AIN13, local 3) (R) | S8.3.49 p105 | `ReadWindowFlags()` low mask | T18 | OK |
| FLD-3-3E-2 | AIN9-16 0x3E DWC_FLAG_AIN9_16 [2] LOW_FLAG_AIN14 (AIN14, local 2) (R) | S8.3.49 p105 | `ReadWindowFlags()` low mask | T18 | OK |
| FLD-3-3E-1 | AIN9-16 0x3E DWC_FLAG_AIN9_16 [1] LOW_FLAG_AIN15 (AIN15, local 1) (R) | S8.3.49 p105 | `ReadWindowFlags()` low mask | T18 | OK |
| FLD-3-3E-0 | AIN9-16 0x3E DWC_FLAG_AIN9_16 [0] LOW_FLAG_AIN16 (AIN16, local 0) (R) | S8.3.49 p105 | `ReadWindowFlags()` low mask | T18 | OK |

## Modes and features (201 rows)

| Id | Item | Reference | API | Test | Status |
| --- | --- | --- | --- | --- | --- |
| MOD-01 | 16 simultaneous-sampling 16-bit channels, differential or single-ended | S7.3.1 p25 | `Frame` with 16 codes; `Input` per channel | T08,T13 | OK |
| MOD-02 | Input clamp: inputs swing to 18 V, clamp current rises to 105 V, 10 mA limit | S7.3.2 p25-26 | not provided: no constant; the limit is in docs/hardware.md | T27 | GAP |
| MOD-03 | External series resistor and matched source impedance guidance | S7.3.2 p26, S7.3.3 p26 | none (hardware); docs/inputs, hardware.yml circuit | T29 | INFO |
| MOD-04 | Input resistance 1 Mohm on AINnP and AINnM, independent of rate and range | S7.3.3 p26 | `kInputResistanceOhm` constant; docs/inputs source-impedance error helper none | T27 | INFO |
| MOD-RNG-5-5 | Table 7-1 row: Single-ended +-12.5 V: INPUT_RANGE 5, CM_RANGE 5 | S7.3.4 Table 7-1 p26-27 | `Input{type, range}` -> CM_RANGE 5, INPUT_RANGE 5 | T08,T30 | OK |
| MOD-RNG-5-4 | Table 7-1 row: Single-ended +-10 V: INPUT_RANGE 4, CM_RANGE 5 | S7.3.4 Table 7-1 p26-27 | `Input{type, range}` -> CM_RANGE 5, INPUT_RANGE 4 | T08,T30 | OK |
| MOD-RNG-5-3 | Table 7-1 row: Single-ended +-6.25 V: INPUT_RANGE 3, CM_RANGE 5 | S7.3.4 Table 7-1 p26-27 | `Input{type, range}` -> CM_RANGE 5, INPUT_RANGE 3 | T08,T30 | OK |
| MOD-RNG-5-0 | Table 7-1 row: Single-ended +-5 V: INPUT_RANGE 0, CM_RANGE 5 | S7.3.4 Table 7-1 p26-27 | `Input{type, range}` -> CM_RANGE 5, INPUT_RANGE 0 | T08,T30 | OK |
| MOD-RNG-5-2 | Table 7-1 row: Single-ended +-2.5 V: INPUT_RANGE 2, CM_RANGE 5 | S7.3.4 Table 7-1 p26-27 | `Input{type, range}` -> CM_RANGE 5, INPUT_RANGE 2 | T08,T30 | OK |
| MOD-RNG-0-5 | Table 7-1 row: Differential +-12.5 V: INPUT_RANGE 5, CM_RANGE 0 | S7.3.4 Table 7-1 p26-27 | `Input{type, range}` -> CM_RANGE 0, INPUT_RANGE 5 | T08,T30 | OK |
| MOD-RNG-0-4 | Table 7-1 row: Differential +-10 V: INPUT_RANGE 4, CM_RANGE 0 | S7.3.4 Table 7-1 p26-27 | `Input{type, range}` -> CM_RANGE 0, INPUT_RANGE 4 | T08,T30 | OK |
| MOD-RNG-0-3 | Table 7-1 row: Differential +-6.25 V: INPUT_RANGE 3, CM_RANGE 0 | S7.3.4 Table 7-1 p26-27 | `Input{type, range}` -> CM_RANGE 0, INPUT_RANGE 3 | T08,T30 | OK |
| MOD-RNG-0-0 | Table 7-1 row: Differential +-5 V: INPUT_RANGE 0, CM_RANGE 0 | S7.3.4 Table 7-1 p26-27 | `Input{type, range}` -> CM_RANGE 0, INPUT_RANGE 0 | T08,T30 | OK |
| MOD-RNG-0-2 | Table 7-1 row: Differential +-2.5 V: INPUT_RANGE 2, CM_RANGE 0 | S7.3.4 Table 7-1 p26-27 | `Input{type, range}` -> CM_RANGE 0, INPUT_RANGE 2 | T08,T30 | OK |
| MOD-RNG-6-5 | Table 7-1 row: Single-ended open wire safe +-12.5 V: INPUT_RANGE 5, CM_RANGE 6 | S7.3.4 Table 7-1 p26-27 | `Input{type, range}` -> CM_RANGE 6, INPUT_RANGE 5 | T08,T30 | OK |
| MOD-RNG-6-4 | Table 7-1 row: Single-ended open wire safe +-10 V: INPUT_RANGE 4, CM_RANGE 6 | S7.3.4 Table 7-1 p26-27 | `Input{type, range}` -> CM_RANGE 6, INPUT_RANGE 4 | T08,T30 | OK |
| MOD-RNG-6-3 | Table 7-1 row: Single-ended open wire safe +-6.25 V: INPUT_RANGE 3, CM_RANGE 6 | S7.3.4 Table 7-1 p26-27 | `Input{type, range}` -> CM_RANGE 6, INPUT_RANGE 3 | T08,T30 | OK |
| MOD-RNG-6-0 | Table 7-1 row: Single-ended open wire safe +-5 V: INPUT_RANGE 0, CM_RANGE 6 | S7.3.4 Table 7-1 p26-27 | `Input{type, range}` -> CM_RANGE 6, INPUT_RANGE 0 | T08,T30 | OK |
| MOD-RNG-6-2 | Table 7-1 row: Single-ended open wire safe +-2.5 V: INPUT_RANGE 2, CM_RANGE 6 | S7.3.4 Table 7-1 p26-27 | `Input{type, range}` -> CM_RANGE 6, INPUT_RANGE 2 | T08,T30 | OK |
| MOD-LPF-LOW | Low-bandwidth LPF 25.5 kHz, all ranges (PGA_BW_SEL 0, default) | S7.3.4 Table 7-2 p27 | not provided: no helper; the bandwidth is in docs/hardware.md | T08 | GAP |
| MOD-LPF-W280 | Wide-bandwidth LPF +-2.5 V: 280 kHz (PGA_BW_SEL 1) | S7.3.4 Table 7-2 p27 | not provided: no helper; the bandwidth is in docs/hardware.md | T08,T30 | GAP |
| MOD-LPF-W325 | Wide-bandwidth LPF +-5 V: 325 kHz (PGA_BW_SEL 1) | S7.3.4 Table 7-2 p27 | not provided: no helper; the bandwidth is in docs/hardware.md | T08,T30 | GAP |
| MOD-LPF-W300 | Wide-bandwidth LPF +-6.25 V: 300 kHz (PGA_BW_SEL 1) | S7.3.4 Table 7-2 p27 | not provided: no helper; the bandwidth is in docs/hardware.md | T08,T30 | GAP |
| MOD-LPF-W350 | Wide-bandwidth LPF +-10 V and +-12.5 V: 350 kHz (PGA_BW_SEL 1) | S7.3.4 Table 7-2 p27 | not provided: no helper; the bandwidth is in docs/hardware.md | T08,T30 | GAP |
| MOD-TF-TWOS | Output coding two's complement (default), 0x0000 at 0 V | S7.3.5 Figure 7-4 p27 | `DataFormat::TwosComplement`; `WireToCode` | T10 | OK |
| MOD-TF-BIN | Straight (offset) binary with EN_OFS_BINARY, 0x8000 at 0 V | S7.3.5 Figure 7-4 p27 | `DataFormat::OffsetBinary`; `WireToCode` | T10 | OK |
| MOD-TF-FMT | Same format for all channels | S7.3.5 p27 | one `Interface.format` for the device | T10 | OK |
| MOD-REF-1 | Table 7-3 row 1: REFSEL Low, REFSEL_CTRL_DIS 0: internal 4.096 V reference active, pin REFSEL low: external reference on REFIO (REFSEL_CTRL_DIS 0) | S7.3.6 Table 7-3 p28 | `ReferenceSource::Pin` | T07 | OK |
| MOD-REF-2 | Table 7-3 row 2: REFSEL High, REFSEL_CTRL_DIS 0: REFSEL high, REFSEL_CTRL_DIS 0: internal reference | S7.3.6 Table 7-3 p28 | `ReferenceSource::Pin` | T07 | OK |
| MOD-REF-3 | Table 7-3 row 3: REFSEL X, REFSEL_CTRL_DIS 1, EXT_REF_EN 0: REFSEL_CTRL_DIS 1, EXT_REF_EN 0: internal reference active, pin ignored | S7.3.6 Table 7-3 p28 | `ReferenceSource::Internal` | T07 | OK |
| MOD-REF-4 | Table 7-3 row 4: REFSEL X, REFSEL_CTRL_DIS 1, EXT_REF_EN 1: REFSEL_CTRL_DIS 1, EXT_REF_EN 1: external reference forced on REFIO | S7.3.6 Table 7-3 p28 | `ReferenceSource::External` | T07 | OK |
| MOD-REF-PD | External reference mode without a reference applied: device powers down | S7.3.6 Note 1 p28 | `Reference::External` documented; model knob; no detection | T07 | GUARD |
| MOD-REF-CAP | Reference decoupling: 4.7 uF REFIO, 1 uF REFCAPA/REFCAPB to REFM | S7.3.6 p28, S9.4 p109 | none (hardware); hardware.yml circuit | T29 | INFO |
| MOD-REF-EXT | External reference example REF50xx (4.096 V, REF5040) | S7.3.6 Figure 7-6 p29 | hardware.yml circuit `external-reference` | T29 | INFO |
| MOD-OWS-1 | Open-wire safe mode (CM_RANGE 110b): floating inputs read within +-16 LSB of zero | S7.3.7 p30 | `InputType::SingleEndedOpenWireSafe` | T09 | OK |
| MOD-OWS-2 | Pull-down RPD needed on AINnM side in single-ended open-wire safe use | S7.3.7 Figure 7-7 p30 | none (hardware); docs/inputs, hardware.yml circuit | T29 | INFO |
| MOD-OWS-RPD2.5 | Table 7-4: RPD 2.5 kohm: code error 2 LSB16 (open-wire safe) vs 15 LSB16 (single-ended) | S7.3.7 Table 7-4 p30 | docs/inputs table | T09 | INFO |
| MOD-OWS-RPD5 | Table 7-4: RPD 5 kohm: code error 4 LSB16 (open-wire safe) vs 30 LSB16 (single-ended) | S7.3.7 Table 7-4 p30 | docs/inputs table | T09 | INFO |
| MOD-OWS-RPD7.5 | Table 7-4: RPD 7.5 kohm: code error 6 LSB16 (open-wire safe) vs 45 LSB16 (single-ended) | S7.3.7 Table 7-4 p30 | docs/inputs table | T09 | INFO |
| MOD-OWS-RPD10 | Table 7-4: RPD 10 kohm: code error 8 LSB16 (open-wire safe) vs 60 LSB16 (single-ended) | S7.3.7 Table 7-4 p30 | docs/inputs table | T09 | INFO |
| MOD-OFS-EQ | Offset calibration equation (1): ((code x 4) + OFS) / 4, 10-bit two's complement | S7.3.8 p31 | `SetInput`, `OffsetCorrectionFor` | T14 | OK |
| MOD-OFS-1FFh | Table 7-5: OFS register 1FFh gives ADC code +127 LSB16 | S7.3.8 Table 7-5 p31 | `OffsetCorrectionFor` anchors as `static_assert` | T14 | OK |
| MOD-OFS-100h | Table 7-5: OFS register 100h gives ADC code +64 LSB16 | S7.3.8 Table 7-5 p31 | `OffsetCorrectionFor` anchors as `static_assert` | T14 | OK |
| MOD-OFS-001h | Table 7-5: OFS register 001h gives ADC code +0.25 LSB16 | S7.3.8 Table 7-5 p31 | `OffsetCorrectionFor` anchors as `static_assert` | T14 | OK |
| MOD-OFS-3FFh | Table 7-5: OFS register 3FFh gives ADC code -1 LSB16 | S7.3.8 Table 7-5 p31 | `OffsetCorrectionFor` anchors as `static_assert` | T14 | OK |
| MOD-OFS-2FFh | Table 7-5: OFS register 2FFh gives ADC code -64 LSB16 | S7.3.8 Table 7-5 p31 | `OffsetCorrectionFor` anchors as `static_assert` | T14 | OK |
| MOD-OFS-200h | Table 7-5: OFS register 200h gives ADC code -128 LSB16 | S7.3.8 Table 7-5 p31 | `OffsetCorrectionFor` anchors as `static_assert` | T14 | OK |
| MOD-OFS-DIS | Offset calibration enabled by default; disable with OFS_CORR_DIS | S7.3.8 p31 | `Correction.offset_enabled` | T14 | OK |
| MOD-GAN-EQ | Gain correction equations (2) to (4): applied gain = register / 65536 | S7.3.9 p31 | `SetInput`, `GainCorrectionFor` | T14 | OK |
| MOD-GAN-1FFFh | Table 7-6: GAN 1FFFh: correction 0.124985, ADC code x 1.124985 | S7.3.9 Table 7-6 p31 | `GainCorrectionFor` anchors as `static_assert` | T14 | OK |
| MOD-GAN-0CCDh | Table 7-6: GAN 0CCDh: correction 0.050003, ADC code x 1.05 | S7.3.9 Table 7-6 p31 | `GainCorrectionFor` anchors as `static_assert` | T14 | OK |
| MOD-GAN-0000h | Table 7-6: GAN 0000h: correction 0, ADC code x 1.0 | S7.3.9 Table 7-6 p31 | `GainCorrectionFor` anchors as `static_assert` | T14 | OK |
| MOD-GAN-3333h | Table 7-6: GAN 3333h: correction -0.050003, ADC code x 0.949997 | S7.3.9 Table 7-6 p31 | `GainCorrectionFor` anchors as `static_assert` | T14 | OK |
| MOD-GAN-2000h | Table 7-6: GAN 2000h: correction -0.124985, ADC code x 0.875015 | S7.3.9 Table 7-6 p31 | `GainCorrectionFor` anchors as `static_assert` | T14 | OK |
| MOD-GAN-DIS | Gain calibration disable with GAN_CORR_DIS | S7.3.9 p31 | `Correction.gain_enabled` | T14 | OK |
| MOD-CAL-0 | ADC_CAL module: input disconnected while running, reconnected after TWAIT | S7.3.10 p32 | `RunAdcCalibration` documents that data is invalid meanwhile | T15 | OK |
| MOD-CAL-T1 | Table 7-7: free running CONVST, INT_TRIG_MODE 0: TWAIT = 100,000 x tCONVST | S7.3.10 Table 7-7 p32 | `CalibrationTrigger::Continuous` waits 100000 x period | T15 | OK |
| MOD-CAL-T2 | Table 7-7: single shot, INT_TRIG_MODE 1: TWAIT = 125 ms | S7.3.10 Table 7-7 p32 | `CalibrationTrigger::SingleShot` waits 125 ms | T15 | OK |
| MOD-CAL-S1 | Single-shot step 1: INT_TRIG_MODE to 1 | S7.3.10 p33 | `RunAdcCalibration(SingleShot)` step 1 | T15 | OK |
| MOD-CAL-S2 | Single-shot step 2: ADC_CAL_MODE 01b, 10b or 11b | S7.3.10 p33 | `RunAdcCalibration(SingleShot)` step 2 | T15 | OK |
| MOD-CAL-S3 | Single-shot step 3: ADC_CAL_TRIG 0 to 1 | S7.3.10 p33 | `RunAdcCalibration(SingleShot)` step 3 | T15 | OK |
| MOD-CAL-S4 | Single-shot step 4: wait 10 us | S7.3.10 p33 | `RunAdcCalibration(SingleShot)` step 4 | T15 | OK |
| MOD-CAL-S5 | Single-shot step 5: ADC_CAL_TRIG back to 0 | S7.3.10 p33 | `RunAdcCalibration(SingleShot)` step 5 | T15 | OK |
| MOD-CAL-S6 | Single-shot step 6: a single CONVST pulse with a falling edge, end state high (A39) | S7.3.10 p33 | `RunAdcCalibration(SingleShot)` step 6 | T15 | OK |
| MOD-CAL-S7 | Single-shot step 7: wait 125 ms or read CALIB_BUSY | S7.3.10 p33 | `RunAdcCalibration(SingleShot)` step 7 | T15 | OK |
| MOD-CAL-S8 | Single-shot step 8: normal operation | S7.3.10 p33 | `RunAdcCalibration(SingleShot)` step 8 | T15 | OK |
| MOD-CAL-C1 | Continuous-CONVST step 1: free-running CONVST present | S7.3.10 p33 | `RunAdcCalibration(Continuous)` step 1 | T15 | OK |
| MOD-CAL-C2 | Continuous-CONVST step 2: ADC_CAL_MODE 01b, 10b or 11b | S7.3.10 p33 | `RunAdcCalibration(Continuous)` step 2 | T15 | OK |
| MOD-CAL-C3 | Continuous-CONVST step 3: ADC_CAL_TRIG 0 to 1 | S7.3.10 p33 | `RunAdcCalibration(Continuous)` step 3 | T15 | OK |
| MOD-CAL-C4 | Continuous-CONVST step 4: at least one tCONVST | S7.3.10 p33 | `RunAdcCalibration(Continuous)` step 4 | T15 | OK |
| MOD-CAL-C5 | Continuous-CONVST step 5: ADC_CAL_TRIG back to 0 | S7.3.10 p33 | `RunAdcCalibration(Continuous)` step 5 | T15 | OK |
| MOD-CAL-C6 | Continuous-CONVST step 6: wait 100,000 CONVST cycles | S7.3.10 p33 | `RunAdcCalibration(Continuous)` step 6 | T15 | OK |
| MOD-CAL-C7 | Continuous-CONVST step 7: normal operation | S7.3.10 p33 | `RunAdcCalibration(Continuous)` step 7 | T15 | OK |
| MOD-CAL-PRE | Before ADC_CAL: enable CM error correction if needed, filters disabled, SE_DIFF bits set per bank | S7.3.10 p33 | `RunAdcCalibration` refuses with a filter active or a mixed bank | T15 | GUARD |
| MOD-CAL-MODE0 | ADC_CAL_MODE 00b removes the calibration value from the output | S7.3.10 p33 | `DisableAdcCalibration()`; mode kept after a run | T15 | OK |
| MOD-CAL-RES | Calibration result: channel-to-channel offset 40 to 15 LSB16, offset +-30 to +-8 LSB16 | S7.3.10 Figures 7-11 7-12 p33 | not a behaviour; docs/calibration | T15 | INFO |
| MOD-CAL-PU | ADC_CAL is performed at power up (footnote 5 of Electrical Characteristics) vs optional user trigger | S5.5 note 5 p10, S7.3.10 p32 | ambiguity A38; `Config.cal` optional run in start-up | T15 | OK |
| MOD-FILT-1 | One filter path at a time, applied to all channels | S7.3.11 p34 | `Filter` is one-of (kind) | T16 | OK |
| MOD-PH-EQ | Phase calibration delay = PHASE_DELAY / sampling frequency (equation 5), 1 us at 1 MSPS | S7.3.11.1 p34 | not provided: no helper; `Input::phase_delay` is in samples | T17 | GAP |
| MOD-PH-S1 | Phase configuration step 1: compute PHASE_DELAY_AINx | S7.3.11.1 p34 | `SetInput`, `Input::phase_delay` step 1 | T17 | OK |
| MOD-PH-S2 | Phase configuration step 2: program PHASE_DELAY_AINx | S7.3.11.1 p34 | `SetInput`, `Input::phase_delay` step 2 | T17 | OK |
| MOD-PH-S3 | Phase configuration step 3: set PHASE_DELAY_EN | S7.3.11.1 p34 | `SetInput`, `Input::phase_delay` step 3 | T17 | OK |
| MOD-PH-S4 | Phase configuration step 4: pulse DIG_FILT_SYSREF 0 to 1, wait >= tCONVST, back to 0 | S7.3.11.1 p34 | `SetInput`, `Input::phase_delay` step 4 | T17 | OK |
| MOD-BA-1 | Block average: external clock oversampling (default), one sample per CONVST falling edge, output after N | S7.3.11.2 Figure 7-16 p35 | `Filter{FilterKind::BlockAverage, length, internal_clock = false}` | T16 | OK |
| MOD-BA-2 | Block average: internal clock oversampling, INT_TRIG_MODE 1, one CONVST edge, N-1 internal clocks | S7.3.11.2 Figure 7-17 p35-36 | `Filter{FilterKind::BlockAverage, length, internal_clock = true}` | T16 | OK |
| MOD-BA-3 | Initialise GEN_CFG5: AVG_MODE_OVR_EN 1 then AVG_MODE 1 before using the filters | S7.3.11.2 p35 | `SetFilter` writes the pair in this order | T16 | OK |
| MOD-BA-4 | SYSREF 0 to 1 edge needed for the oversampling filter to work | S7.3.11.2 p35 | `SetFilter` pulse | T16 | OK |
| MOD-BA-OSR1 | Table 7-8: OSR 1: SNR 87.5 dB (+-5 V), max output rate 1 MSPS | S7.3.11.2 Table 7-8 p36 | `OutputRateHz`, `Filter::BlockAverage` ratio | T16 | OK |
| MOD-BA-OSR2 | Table 7-8: OSR 2: SNR 89.5 dB (+-5 V), max output rate 500 kSPS | S7.3.11.2 Table 7-8 p36 | `OutputRateHz`, `Filter::BlockAverage` ratio | T16 | OK |
| MOD-BA-OSR4 | Table 7-8: OSR 4: SNR 91.2 dB (+-5 V), max output rate 250 kSPS | S7.3.11.2 Table 7-8 p36 | `OutputRateHz`, `Filter::BlockAverage` ratio | T16 | OK |
| MOD-BA-OSR8 | Table 7-8: OSR 8: SNR 92.7 dB (+-5 V), max output rate 125 kSPS | S7.3.11.2 Table 7-8 p36 | `OutputRateHz`, `Filter::BlockAverage` ratio | T16 | OK |
| MOD-BA-OSR16 | Table 7-8: OSR 16: SNR 94 dB (+-5 V), max output rate 62.5 kSPS | S7.3.11.2 Table 7-8 p36 | `OutputRateHz`, `Filter::BlockAverage` ratio | T16 | OK |
| MOD-BA-OSR32 | Table 7-8: OSR 32: SNR 96.7 dB (+-5 V), max output rate 31.25 kSPS | S7.3.11.2 Table 7-8 p36 | `OutputRateHz`, `Filter::BlockAverage` ratio | T16 | OK |
| MOD-BA-OSR64 | Table 7-8: OSR 64: SNR 99.1 dB (+-5 V), max output rate 15.625 kSPS | S7.3.11.2 Table 7-8 p36 | `OutputRateHz`, `Filter::BlockAverage` ratio | T16 | OK |
| MOD-BA-OSR128 | Table 7-8: OSR 128: SNR 101.6 dB (+-5 V), max output rate 7.8125 kSPS | S7.3.11.2 Table 7-8 p36 | `OutputRateHz`, `Filter::BlockAverage` ratio | T16 | OK |
| MOD-MVG-1 | Moving average, length by MVG_AVG_LEN up to 128, SYSREF needed | S7.3.11.3 p36 | `Filter::MovingAverage{length}` | T16 | OK |
| MOD-MVG-2 | Moving average frequency response equation (6) | S7.3.11.3 p36-37 | docs/filters; no code | T16 | INFO |
| MOD-FIR-FIR1 | Table 7-9: FIR1 FIR_FILT_SEL 001b, decimation 2, -3 dB 0.218 x fCONVST, group delay 5 tCONVST | S7.3.11.4 Table 7-9 p37-38 | `FilterKind::Fir`, `FirDecimation(FirFilter::Fir1)` | T16 | OK |
| MOD-FIR-FIR2 | Table 7-9: FIR2 FIR_FILT_SEL 010b, decimation 2, -3 dB 0.205 x fCONVST, group delay 3 tCONVST | S7.3.11.4 Table 7-9 p37-38 | `FilterKind::Fir`, `FirDecimation(FirFilter::Fir2)` | T16 | OK |
| MOD-FIR-FIR3 | Table 7-9: FIR3 FIR_FILT_SEL 011b, decimation 4, -3 dB 0.108 x fCONVST, group delay 13 tCONVST | S7.3.11.4 Table 7-9 p37-38 | `FilterKind::Fir`, `FirDecimation(FirFilter::Fir3)` | T16 | OK |
| MOD-FIR-FIR4 | Table 7-9: FIR4 FIR_FILT_SEL 100b, decimation 2, -3 dB 0.205 x fCONVST, group delay 3 tCONVST | S7.3.11.4 Table 7-9 p37-38 | `FilterKind::Fir`, `FirDecimation(FirFilter::Fir4)` | T16 | OK |
| MOD-FIR-FIR5 | Table 7-9: FIR5 FIR_FILT_SEL 101b, decimation 4, -3 dB 0.100 x fCONVST, group delay 9 tCONVST | S7.3.11.4 Table 7-9 p37-38 | `FilterKind::Fir`, `FirDecimation(FirFilter::Fir5)` | T16 | OK |
| MOD-FIR-FIR7 | Table 7-9: FIR7 FIR_FILT_SEL 111b, decimation 8, -3 dB 0.540 (printed; implausible, A33) x fCONVST, group delay 29 tCONVST | S7.3.11.4 Table 7-9 p37-38 | `FilterKind::Fir`, `FirDecimation(FirFilter::Fir7)` | T16 | OK |
| MOD-FIR-6 | FIR6 (110b) reserved | S8.1.15 Table 8-17 p60 | unreachable: `FirFilter` has no sixth enumerator | T01 | GUARD |
| MOD-FIR-COEF | FIR coefficients and frequency response (Figures 7-20 to 7-23) | S7.3.11.4 p38 | docs/filters; none | T16 | TODO |
| MOD-FIR-SYS | SYSREF pulse after programming a FIR filter | S7.3.11.4 p37 | `SetFilter` pulse | T16 | OK |
| MOD-DWC-1 | 8-bit window comparator per channel: low threshold, high threshold, hysteresis | S7.3.12 p39 | `Window` per channel | T18 | OK |
| MOD-DWC-2 | Defaults: hysteresis 0, high 127 (0x7F), low -128 (0x80) (text) vs register reset 0xFF00 | S7.3.12 p39, S8.2.27 p75 | explicit write of thresholds and hysteresis before enabling (A16) | T18 | OK |
| MOD-DWC-3 | Glitch reject filter: 4-bit counter of consecutive violations, common per bank | S7.3.12 p39 | `Config::window_glitch` | T18 | OK |
| MOD-DWC-4 | DWC_RST rising edge resets the flags, common per bank | S7.3.12 p39 | `ClearWindowFlags(group)` | T18 | OK |
| MOD-DWC-5 | HIGH_FLAG and LOW_FLAG per channel, latched; DWC_ALERT_FLAG in DIAG | S7.3.12 p39, S7.3.14.3 p44 | `ReadWindowFlags()`, `Frame.window_alert` | T18 | OK |
| MOD-DWC-6 | DWC_ALERT_INT = OR of high and low, level (ALRM_TYPE 0) or pulse (1), routed to the ALARM pin | S7.3.12 p39, S7.3.13 p41 | `Alarm{WindowAlert, pulse}` | T18,T19 | OK |
| MOD-DWC-7 | High-only or low-only alerts by setting the other threshold to its extreme | S7.3.12 p39 | not provided: no helper; set the other threshold to its limit | T18 | GAP |
| MOD-DWC-8 | Behaviour with hysteresis (Figure 7-25) | S7.3.12 p40 | `Window.hysteresis` | T18 | OK |
| MOD-ALM-0000b | Table 7-10: DRDY_ALRM_SEL 0000b: DRDY: ADC conversion done | S7.3.13 Table 7-10 p41 | `Alarm::function` | T19 | OK |
| MOD-ALM-0001b | Table 7-10: DRDY_ALRM_SEL 0001b: DWC_ALERT_INT: window comparator alert | S7.3.13 Table 7-10 p41 | `Alarm::function` | T19 | OK |
| MOD-ALM-0110b | Table 7-10: DRDY_ALRM_SEL 0110b: ADC_CAL_DONE: calibration done | S7.3.13 Table 7-10 p41 | `Alarm::function` | T19 | OK |
| MOD-ALM-OR | DRDY_ALRM_SEL 1000b: OR of the flags, each masked by ALRM_MASK | S8.1.8 Table 8-10 p56 | `Alarm::OrMasked` guarded | T19 | GUARD |
| MOD-ALM-POL | ALARM active high by default, ALRM_POL 1 makes it active low | S7.3.13 p41 | `Alarm.active_low` | T19 | OK |
| MOD-ALM-TYPE | Pulse mode only for the DWC signal | S7.3.13 p41 | `Alarm.pulse` only with `WindowAlert` (else `InvalidArgument`) | T19 | OK |
| MOD-IF-00 | Table 7-11: 8-lane D[7:0]: DOUT_LANE_SEL 0, ADC_DATA_SDOUT_EN 0 (Figure 5-2) | S7.3.14 Table 7-11 p42 | `Interface{lanes, data_on_sdout}`; `Transfer` for SDOUT, `CaptureLanes` otherwise | T12 | OK |
| MOD-IF-10 | Table 7-11: 4-lane D[7:4]: DOUT_LANE_SEL 1, ADC_DATA_SDOUT_EN 0 (Figure 5-3) | S7.3.14 Table 7-11 p42 | `Interface{lanes, data_on_sdout}`; `Transfer` for SDOUT, `CaptureLanes` otherwise | T12 | OK |
| MOD-IF-20 | Table 7-11: 2-lane D[7:6]: DOUT_LANE_SEL 2, ADC_DATA_SDOUT_EN 0 (Figure 5-4) | S7.3.14 Table 7-11 p42 | `Interface{lanes, data_on_sdout}`; `Transfer` for SDOUT, `CaptureLanes` otherwise | T12 | OK |
| MOD-IF-30 | Table 7-11: 1-lane D7: DOUT_LANE_SEL 3, ADC_DATA_SDOUT_EN 0 (Figure 5-5) | S7.3.14 Table 7-11 p42 | `Interface{lanes, data_on_sdout}`; `Transfer` for SDOUT, `CaptureLanes` otherwise | T12 | OK |
| MOD-IF-31 | Table 7-11: 1-lane SDOUT: DOUT_LANE_SEL 3, ADC_DATA_SDOUT_EN 1 (Figure 5-5) | S7.3.14 Table 7-11 p42 | `Interface{lanes, data_on_sdout}`; `Transfer` for SDOUT, `CaptureLanes` otherwise | T12 | OK |
| MOD-IF-LEN | 16-bit and 24-bit data length; 24-bit packet carries the 16-bit result MSB first, low 8 bits zero without oversampling | S7.3.14 p42 | `Interface.length` | T10 | OK |
| MOD-IF-ORD | In every mode the lowest channel number is transmitted first | S7.3.14.1 p42 | `FrameSlotChannel` | T11 | OK |
| MOD-IF-REG | Register frames always on SDI/SDOUT | S7.5 p50 | `Transfer` for registers in every lane mode | T04,T12 | OK |
| MOD-CH-1 | Table 7-12: 16 channels, ADC_NUM_SEL/ADC_CH_SEL 00b xxxb: AIN1 to AIN16 | S7.3.14.1 Table 7-12 p42 | `ChannelSelect{count, group}`; `FrameSlotChannel` | T11 | OK |
| MOD-CH-2 | Table 7-12: 8 channels, ADC_NUM_SEL/ADC_CH_SEL 01b 0xxb: AIN1-4, AIN13-16 | S7.3.14.1 Table 7-12 p42 | `ChannelSelect{count, group}`; `FrameSlotChannel` | T11 | OK |
| MOD-CH-3 | Table 7-12: 8 channels, ADC_NUM_SEL/ADC_CH_SEL 01b 1xxb: AIN5-12 | S7.3.14.1 Table 7-12 p42 | `ChannelSelect{count, group}`; `FrameSlotChannel` | T11 | OK |
| MOD-CH-4 | Table 7-12: 4 channels, ADC_NUM_SEL/ADC_CH_SEL 10b 00xb: AIN1, AIN2, AIN15, AIN16 | S7.3.14.1 Table 7-12 p42 | `ChannelSelect{count, group}`; `FrameSlotChannel` | T11 | OK |
| MOD-CH-5 | Table 7-12: 4 channels, ADC_NUM_SEL/ADC_CH_SEL 10b 01xb: AIN3, AIN4, AIN13, AIN14 | S7.3.14.1 Table 7-12 p42 | `ChannelSelect{count, group}`; `FrameSlotChannel` | T11 | OK |
| MOD-CH-6 | Table 7-12: 4 channels, ADC_NUM_SEL/ADC_CH_SEL 10b 10xb: AIN5, AIN6, AIN11, AIN12 | S7.3.14.1 Table 7-12 p42 | `ChannelSelect{count, group}`; `FrameSlotChannel` | T11 | OK |
| MOD-CH-7 | Table 7-12: 4 channels, ADC_NUM_SEL/ADC_CH_SEL 10b 11xb: AIN7, AIN8, AIN9, AIN10 | S7.3.14.1 Table 7-12 p42 | `ChannelSelect{count, group}`; `FrameSlotChannel` | T11 | OK |
| MOD-CH-2-0 | Table 7-12: 2 channels, ADC_NUM_SEL 11b ADC_CH_SEL 000b: AIN1, AIN16 | S7.3.14.1 Table 7-12 p42 | `ChannelSelect{count 2, group 0}` | T11 | OK |
| MOD-CH-2-1 | Table 7-12: 2 channels, ADC_NUM_SEL 11b ADC_CH_SEL 001b: AIN2, AIN15 | S7.3.14.1 Table 7-12 p42 | `ChannelSelect{count 2, group 1}` | T11 | OK |
| MOD-CH-2-2 | Table 7-12: 2 channels, ADC_NUM_SEL 11b ADC_CH_SEL 010b: AIN3, AIN14 | S7.3.14.1 Table 7-12 p42 | `ChannelSelect{count 2, group 2}` | T11 | OK |
| MOD-CH-2-3 | Table 7-12: 2 channels, ADC_NUM_SEL 11b ADC_CH_SEL 011b: AIN4, AIN13 | S7.3.14.1 Table 7-12 p42 | `ChannelSelect{count 2, group 3}` | T11 | OK |
| MOD-CH-2-4 | Table 7-12: 2 channels, ADC_NUM_SEL 11b ADC_CH_SEL 100b: AIN5, AIN12 | S7.3.14.1 Table 7-12 p42 | `ChannelSelect{count 2, group 4}` | T11 | OK |
| MOD-CH-2-5 | Table 7-12: 2 channels, ADC_NUM_SEL 11b ADC_CH_SEL 101b: AIN6, AIN11 | S7.3.14.1 Table 7-12 p42 | `ChannelSelect{count 2, group 5}` | T11 | OK |
| MOD-CH-2-6 | Table 7-12: 2 channels, ADC_NUM_SEL 11b ADC_CH_SEL 110b: AIN7, AIN10 | S7.3.14.1 Table 7-12 p42 | `ChannelSelect{count 2, group 6}` | T11 | OK |
| MOD-CH-2-7 | Table 7-12: 2 channels, ADC_NUM_SEL 11b ADC_CH_SEL 111b: AIN8, AIN9 | S7.3.14.1 Table 7-12 p42 | `ChannelSelect{count 2, group 7}` | T11 | OK |
| MOD-DC-1 | Daisy chain topology: CS and SCLK shared, SDOUT of one device to SDI of the next | S7.3.14.2 Figure 7-29 p43 | out of scope in v1 (TODO): `Validate` refuses `daisy.enabled` | T23 | TODO |
| MOD-DC-2 | Register frames take 32 SCLK per device, MSB padded with zeros | S7.3.14.2 p43 | out of scope in v1 (TODO) | T23 | TODO |
| MOD-DC-3 | Chain supports 1, 2, 4 lanes; not with data on SDOUT in 1-lane mode | S7.3.14.2 p43 | out of scope in v1 (TODO) | T23 | TODO |
| MOD-DC-4 | Chain programming: DAISY_CHN_NUM and LOC on all devices, lanes, then DAISY_CHN_EN | S7.3.14.2 p43 | out of scope in v1 (TODO) | T23 | TODO |
| MOD-DIAG-1 | Diagnostic byte appended after the channel words with EN_DIAG_FLAG (EN_STATUS_BITS in the text) | S7.3.14.3 Figure 7-30 p43-44 | `Interface.append_diag`; `Frame.diag` | T10 | OK |
| MOD-DIAG-B7 | Table 7-13: DIAG_FLAG[7] Reserved | S7.3.14.3 Table 7-13 p44 | `Frame.diag` raw bit, never interpreted | T10,T24 | INFO |
| MOD-DIAG-B6 | Table 7-13: DIAG_FLAG[6] DWC_ALERT_FLAG OR of all window comparators | S7.3.14.3 Table 7-13 p44 | `Frame.window_alert` | T10,T24 | OK |
| MOD-DIAG-B5 | Table 7-13: DIAG_FLAG[5] Reserved | S7.3.14.3 Table 7-13 p44 | `Frame.diag` raw bit, never interpreted | T10,T24 | INFO |
| MOD-DIAG-B4 | Table 7-13: DIAG_FLAG[4] Reserved | S7.3.14.3 Table 7-13 p44 | `Frame.diag` raw bit, never interpreted | T10,T24 | INFO |
| MOD-DIAG-B3 | Table 7-13: DIAG_FLAG[3] Reserved | S7.3.14.3 Table 7-13 p44 | `Frame.diag` raw bit, never interpreted | T10,T24 | INFO |
| MOD-DIAG-B2 | Table 7-13: DIAG_FLAG[2] Reserved | S7.3.14.3 Table 7-13 p44 | `Frame.diag` raw bit, never interpreted | T10,T24 | INFO |
| MOD-DIAG-B1 | Table 7-13: DIAG_FLAG[1] Reserved | S7.3.14.3 Table 7-13 p44 | `Frame.diag` raw bit, never interpreted | T10,T24 | INFO |
| MOD-DIAG-B0 | Table 7-13: DIAG_FLAG[0] RESET_DETECT_FLAG reset value 1 (Table 7-13) vs 0 (Table 8-5), A11 | S7.3.14.3 Table 7-13 p44 | `Frame.reset_detected`; `UnexpectedReset` | T10,T24 | OK |
| MOD-XOR-1 | Randomizer: ADC word XOR a PRBS mask, PRBS byte appended | S7.3.14.4 Figure 7-31 p44-45 | `Interface.randomize`; `DecodeFrame` derandomizes | T10 | OK |
| MOD-XOR-2 | MASK_BITS = PRBS x 256 + PRBS0 x 255; result = word XOR mask (equations 7, 8; example 0x1234, 0x1f) | S7.3.14.4 p44 | `DerandomizeMask`, `static_assert` on the example | T10 | OK |
| MOD-XOR-3 | Enable: EN_XOR_PATT 1; XOR_BIT_SEL 1, NUM_XOR_BITS 11b, XOR_MODE 11b | S7.3.14.4 p44 | `Interface.randomize` writes both registers | T10 | OK |
| MOD-XOR-4 | XOR mode not supported for 24-bit frames | S7.3.14.4 p44 | `Validate` refuses `randomize` with `Bits24` (`InvalidArgument`) | T10 | GUARD |
| MOD-FRM-7-32 | Figure 7-32: DIAG and XOR appended after AIN16 | S7.3.14.4 p45-47 | `FrameBytes`, `FrameChannels`, `detail::DecodeFrame` | T10,T12 | OK |
| MOD-FRM-7-33 | Figure 7-33: 8-lane frame | S7.3.14.4 p45-47 | `FrameBytes`, `FrameChannels`, `detail::DecodeFrame` | T10,T12 | OK |
| MOD-FRM-7-34 | Figure 7-34: 4-lane frame | S7.3.14.4 p45-47 | `FrameBytes`, `FrameChannels`, `detail::DecodeFrame` | T10,T12 | OK |
| MOD-FRM-7-35 | Figure 7-35: 2-lane frame | S7.3.14.4 p45-47 | `FrameBytes`, `FrameChannels`, `detail::DecodeFrame` | T10,T12 | OK |
| MOD-FRM-7-36 | Figure 7-36: 1-lane frame | S7.3.14.4 p45-47 | `FrameBytes`, `FrameChannels`, `detail::DecodeFrame` | T10,T12 | OK |
| MOD-TP-1 | Table 7-14 row 1: TP_EN 0, TP_MODE X, TP_DIS_IDX X: ADC data | S7.3.14.5 Table 7-14 p47 | `TestPattern` | T20 | OK |
| MOD-TP-2 | Table 7-14 row 2: TP_EN 1, TP_MODE 0, TP_DIS_IDX 1: fixed pattern, index 0x0000 + TP_AINn | S7.3.14.5 Table 7-14 p47 | `TestPattern` | T20 | OK |
| MOD-TP-3 | Table 7-14 row 3: TP_EN 1, TP_MODE 0, TP_DIS_IDX 0: fixed pattern: index inserted | S7.3.14.5 Table 7-14 p47 | `TestPattern` | T20 | OK |
| MOD-TP-4 | Table 7-14 row 4: TP_EN 1, TP_MODE 1, TP_DIS_IDX 1: ramp, increment at channel frame boundary (TP_UPD_MODE 0) | S7.3.14.5 Table 7-14 p47 | `TestPattern` | T20 | OK |
| MOD-TP-5 | Table 7-14 row 5: TP_EN 1, TP_MODE 2, TP_DIS_IDX 1: ramp, increment at sample frame boundary (TP_UPD_MODE 1) | S7.3.14.5 Table 7-14 p47 | `TestPattern` | T20 | OK |
| MOD-TP-N | Notes: two groups, 24-bit output has zero low byte, TP_AIN1 and TP_AIN16 set the ramp step (step = value + 1) | S7.3.14.5 p48 | `TestPattern::value` | T20 | OK |
| MOD-DRV-00b | Table 7-15: DRIVE_STRENGTH 00b: normal | S7.3.14.6 Table 7-15 p48 | `Interface.drive` | T21 | OK |
| MOD-DRV-01b | Table 7-15: DRIVE_STRENGTH 01b: 0.5 x | S7.3.14.6 Table 7-15 p48 | `Interface.drive` | T21 | OK |
| MOD-DRV-10b | Table 7-15: DRIVE_STRENGTH 10b: 2 x | S7.3.14.6 Table 7-15 p48 | `Interface.drive` | T21 | OK |
| MOD-DRV-11b | Table 7-15: DRIVE_STRENGTH 11b: 1.5 x | S7.3.14.6 Table 7-15 p48 | `Interface.drive` | T21 | OK |
| MOD-DLY | Digital output delay 0 to 5 ns per output (SDOUT, D7 to D0), enabled by DIG_DELAY_EN | S7.3.14.7 p48, S8.1.10 p57 | `Interface.delay_ns` | T21 | OK |
| MOD-RST-1 | Reset by RESET pin (asynchronous, active low) at any time | S7.4.1 p49 | `Reset(ResetKind::Hardware)` with the `Reset` pin hook | T06 | OK |
| MOD-RST-2 | Reset by writing SW_RST | S7.4.1 p49 | `Reset(ResetKind::Software)` | T06 | OK |
| MOD-RST-3 | Registers return to default after reset | S7.4.1 p49 | shadow reset, `UnexpectedReset` detection | T06,T24 | OK |
| MOD-NORM | Conversion at the falling edge of CONVST; wait tPU after reset | S7.4.2 p49 | `Convert()`, waits | T13 | OK |
| MOD-STBY | Standby: front end and internal reference off, about 6.5 mW; DEVICE_PDN 1 | S7.4.3 p49 | `PowerDown()`, `Wake()` | T22 | OK |
| MOD-WR | Register write: 24-bit frame, 8-bit address, 16-bit data, latched on SCLK rising, decoded at CS rising | S7.5.1 Figure 7-41 p50 | `writeRegister` | T04 | OK |
| MOD-WR-SEQ | Table 7-16: bank select frame (0x02 data 0x0001/0002/0004) then writes | S7.5.1 Table 7-16 p50 | `selectBank` | T04 | OK |
| MOD-RD | Register read: select bank, read command frame (0x01, REG_RD_ADD, REG_RD_EN), data in the next frame on SDOUT[23:8] | S7.5.2 Figure 7-42, Table 7-17 p50-51 | `ReadRegister`: BANK_SEL, the read command (GEN_CFG1), then a zero frame whose reply carries the value in SDOUT[23:8] and 0x00 in SDOUT[7:0] | T04 | OK |
| MOD-RD-PIPE | N reads in N+2 frames when the next frame carries the next command | S7.5.2 p50 | not used (two-frame reads only; optimisation) | T04 | INFO |
| MOD-INIT-1 | Table 7-18 step 1: wait 30 ms for supplies and reference | S7.5.3 Table 7-18 p51 | `EnsureInitialized()` (A3, A5) | T06 | OK |
| MOD-INIT-2 | Table 7-18 step 2: software reset (optional) | S7.5.3 Table 7-18 p51 | `EnsureInitialized()` (A3, A5) | T06 | OK |
| MOD-INIT-3 | Table 7-18 step 3: wait 1 ms (optional) | S7.5.3 Table 7-18 p51 | `EnsureInitialized()` (A3, A5) | T06 | OK |
| MOD-INIT-4 | Table 7-18 step 4: select common bank 0x02 0x0001 | S7.5.3 Table 7-18 p51 | `EnsureInitialized()` (A3, A5) | T06 | OK |
| MOD-INIT-6 | Table 7-18 step 6: 0x0A 0x0032: 16-bit, 1-lane on SDOUT | S7.5.3 Table 7-18 p51 | `EnsureInitialized()` (A3, A5) | T06 | OK |
| MOD-TD-5-1 | Figure 5-1: register read and write timing | S5.8 p12-16 | port constants, `Convert`, `WaitDataReady`, `ReadFrame` | T13 | OK |
| MOD-TD-5-2 | Figure 5-2: conversion read, 8-lane (default) | S5.8 p12-16 | port constants, `Convert`, `WaitDataReady`, `ReadFrame` | T13 | OK |
| MOD-TD-5-3 | Figure 5-3: conversion read, 4-lane | S5.8 p12-16 | port constants, `Convert`, `WaitDataReady`, `ReadFrame` | T13 | OK |
| MOD-TD-5-4 | Figure 5-4: conversion read, 2-lane | S5.8 p12-16 | port constants, `Convert`, `WaitDataReady`, `ReadFrame` | T13 | OK |
| MOD-TD-5-5 | Figure 5-5: conversion read, 1-lane | S5.8 p12-16 | port constants, `Convert`, `WaitDataReady`, `ReadFrame` | T13 | OK |
| MOD-TD-5-6 | Figure 5-6: CONVST and DRDY, external clock oversampling | S5.8 p12-16 | port constants, `Convert`, `WaitDataReady`, `ReadFrame` | T13 | OK |
| MOD-TD-5-7 | Figure 5-7: CONVST and DRDY, internal clock oversampling | S5.8 p12-16 | port constants, `Convert`, `WaitDataReady`, `ReadFrame` | T13 | OK |
| MOD-PWR-SEQ | No power-up sequence required between AVDD_5V, AVDD_1V8, IOVDD; AVDD_1V8 may share IOVDD at 1.8 V through a ferrite | S9.3 p109 | docs/hardware; none | T29 | INFO |
| MOD-APP | Typical application: 16-channel power-automation DAQ, RC filter, series resistors | S9.2 p106-108 | docs/inputs, circuit `typical` | T29 | INFO |
| MOD-LAY | Layout guidance | S9.4 p109-110 | docs/hardware | T29 | INFO |
| MOD-FAM | Family members (ADS93x4, 12-bit variant) not documented in this datasheet | S7.3.4 p26, S8.2.16 p71 | `Identify()` accepts DEVICE_ID 0x04 only | T06 | INFO |

## Pins, electrical, timing, package (171 rows)

| Id | Item | Reference | API | Test | Status |
| --- | --- | --- | --- | --- | --- |
| PIN-01 | Pin 1: AIN5P | S4 Table 4-1 p3-4; Figure 4-1 p3 | `Input` for channel 5 (bank 1 (local 4)); positive input | T08,T09,T29 | OK |
| PIN-02 | Pin 2: AIN5M | S4 Table 4-1 p3-4; Figure 4-1 p3 | `Input` for channel 5 (bank 1 (local 4)); negative input | T08,T09,T29 | OK |
| PIN-03 | Pin 3: AIN6P | S4 Table 4-1 p3-4; Figure 4-1 p3 | `Input` for channel 6 (bank 1 (local 5)); positive input | T08,T09,T29 | OK |
| PIN-04 | Pin 4: AIN6M | S4 Table 4-1 p3-4; Figure 4-1 p3 | `Input` for channel 6 (bank 1 (local 5)); negative input | T08,T09,T29 | OK |
| PIN-05 | Pin 5: AIN7P | S4 Table 4-1 p3-4; Figure 4-1 p3 | `Input` for channel 7 (bank 1 (local 6)); positive input | T08,T09,T29 | OK |
| PIN-06 | Pin 6: AIN7M | S4 Table 4-1 p3-4; Figure 4-1 p3 | `Input` for channel 7 (bank 1 (local 6)); negative input | T08,T09,T29 | OK |
| PIN-07 | Pin 7: AIN8P | S4 Table 4-1 p3-4; Figure 4-1 p3 | `Input` for channel 8 (bank 1 (local 7)); positive input | T08,T09,T29 | OK |
| PIN-08 | Pin 8: AIN8M | S4 Table 4-1 p3-4; Figure 4-1 p3 | `Input` for channel 8 (bank 1 (local 7)); negative input | T08,T09,T29 | OK |
| PIN-09 | Pin 9: AIN9P | S4 Table 4-1 p3-4; Figure 4-1 p3 | `Input` for channel 9 (bank 2 (mirrored, local 7)); positive input | T08,T09,T29 | OK |
| PIN-10 | Pin 10: AIN9M | S4 Table 4-1 p3-4; Figure 4-1 p3 | `Input` for channel 9 (bank 2 (mirrored, local 7)); negative input | T08,T09,T29 | OK |
| PIN-11 | Pin 11: AIN10P | S4 Table 4-1 p3-4; Figure 4-1 p3 | `Input` for channel 10 (bank 2 (mirrored, local 6)); positive input | T08,T09,T29 | OK |
| PIN-12 | Pin 12: AIN10M | S4 Table 4-1 p3-4; Figure 4-1 p3 | `Input` for channel 10 (bank 2 (mirrored, local 6)); negative input | T08,T09,T29 | OK |
| PIN-13 | Pin 13: AIN11P | S4 Table 4-1 p3-4; Figure 4-1 p3 | `Input` for channel 11 (bank 2 (mirrored, local 5)); positive input | T08,T09,T29 | OK |
| PIN-14 | Pin 14: AIN11M | S4 Table 4-1 p3-4; Figure 4-1 p3 | `Input` for channel 11 (bank 2 (mirrored, local 5)); negative input | T08,T09,T29 | OK |
| PIN-15 | Pin 15: AIN12P | S4 Table 4-1 p3-4; Figure 4-1 p3 | `Input` for channel 12 (bank 2 (mirrored, local 4)); positive input | T08,T09,T29 | OK |
| PIN-16 | Pin 16: AIN12M | S4 Table 4-1 p3-4; Figure 4-1 p3 | `Input` for channel 12 (bank 2 (mirrored, local 4)); negative input | T08,T09,T29 | OK |
| PIN-17 | Pin 17: AIN13P | S4 Table 4-1 p3-4; Figure 4-1 p3 | `Input` for channel 13 (bank 2 (mirrored, local 3)); positive input | T08,T09,T29 | OK |
| PIN-18 | Pin 18: AIN13M | S4 Table 4-1 p3-4; Figure 4-1 p3 | `Input` for channel 13 (bank 2 (mirrored, local 3)); negative input | T08,T09,T29 | OK |
| PIN-19 | Pin 19: AIN14P | S4 Table 4-1 p3-4; Figure 4-1 p3 | `Input` for channel 14 (bank 2 (mirrored, local 2)); positive input | T08,T09,T29 | OK |
| PIN-20 | Pin 20: AIN14M | S4 Table 4-1 p3-4; Figure 4-1 p3 | `Input` for channel 14 (bank 2 (mirrored, local 2)); negative input | T08,T09,T29 | OK |
| PIN-21 | Pin 21: AIN15P | S4 Table 4-1 p3-4; Figure 4-1 p3 | `Input` for channel 15 (bank 2 (mirrored, local 1)); positive input | T08,T09,T29 | OK |
| PIN-22 | Pin 22: AIN15M | S4 Table 4-1 p3-4; Figure 4-1 p3 | `Input` for channel 15 (bank 2 (mirrored, local 1)); negative input | T08,T09,T29 | OK |
| PIN-23 | Pin 23: AIN16P | S4 Table 4-1 p3-4; Figure 4-1 p3 | `Input` for channel 16 (bank 2 (mirrored, local 0)); positive input | T08,T09,T29 | OK |
| PIN-24 | Pin 24: AIN16M | S4 Table 4-1 p3-4; Figure 4-1 p3 | `Input` for channel 16 (bank 2 (mirrored, local 0)); negative input | T08,T09,T29 | OK |
| PIN-25 | Pin 25: GND | S4 Table 4-1 p3-4; Figure 4-1 p3 | hardware.yml pin; docs/hardware | T29 | OK |
| PIN-26 | Pin 26: AVDD_5V | S4 Table 4-1 p3-4; Figure 4-1 p3 | hardware.yml supply; `Supply::avdd_5v_mv`, range checked in `Validate` | T27,T29 | OK |
| PIN-27 | Pin 27: AVDD_5V | S4 Table 4-1 p3-4; Figure 4-1 p3 | hardware.yml supply; `Supply::avdd_5v_mv`, range checked in `Validate` | T27,T29 | OK |
| PIN-28 | Pin 28: REFM | S4 Table 4-1 p3-4; Figure 4-1 p3 | hardware.yml pin; reference return | T29 | OK |
| PIN-29 | Pin 29: REFCAPB | S4 Table 4-1 p3-4; Figure 4-1 p3 | hardware.yml circuit, 1 uF to REFM | T29 | OK |
| PIN-30 | Pin 30: AVDD_1V8 | S4 Table 4-1 p3-4; Figure 4-1 p3 | hardware.yml supply; `Supply::avdd_1v8_mv`, range checked in `Validate` | T27,T29 | OK |
| PIN-31 | Pin 31: REFSEL | S4 Table 4-1 p3-4; Figure 4-1 p3 | `ReferenceSource::Pin`; level read by the chip, driver never drives it | T07 | OK |
| PIN-32 | Pin 32: RESET | S4 Table 4-1 p3-4; Figure 4-1 p3 | `CtrlPin::Reset` via `GpioSet`; `Reset(Hardware)` | T06 | OK |
| PIN-33 | Pin 33: GND | S4 Table 4-1 p3-4; Figure 4-1 p3 | hardware.yml pin; docs/hardware | T29 | OK |
| PIN-34 | Pin 34: IOVDD | S4 Table 4-1 p3-4; Figure 4-1 p3 | hardware.yml supply; `Supply::iovdd_mv`, range checked in `Validate` | T27,T29 | OK |
| PIN-35 | Pin 35: CS | S4 Table 4-1 p3-4; Figure 4-1 p3 | chip select inside `Transfer`/`CaptureLanes` (port) | T04 | OK |
| PIN-36 | Pin 36: SDI | S4 Table 4-1 p3-4; Figure 4-1 p3 | MOSI inside `Transfer` | T04 | OK |
| PIN-37 | Pin 37: SCLK | S4 Table 4-1 p3-4; Figure 4-1 p3 | clock inside `Transfer`/`CaptureLanes` | T04,T12 | OK |
| PIN-38 | Pin 38: SDOUT | S4 Table 4-1 p3-4; Figure 4-1 p3 | MISO inside `Transfer`; data lane in 1-lane SDOUT mode | T04,T12 | OK |
| PIN-39 | Pin 39: D7 | S4 Table 4-1 p3-4; Figure 4-1 p3 | `CaptureLanes`: lane bit 7 (8-lane); D7 also the 1-lane data pin | T12 | OK |
| PIN-40 | Pin 40: D6 | S4 Table 4-1 p3-4; Figure 4-1 p3 | `CaptureLanes`: lane bit 6 (8-lane); D7 also the 1-lane data pin | T12 | OK |
| PIN-41 | Pin 41: D5 | S4 Table 4-1 p3-4; Figure 4-1 p3 | `CaptureLanes`: lane bit 5 (8-lane); D7 also the 1-lane data pin | T12 | OK |
| PIN-42 | Pin 42: D4 | S4 Table 4-1 p3-4; Figure 4-1 p3 | `CaptureLanes`: lane bit 4 (8-lane); D7 also the 1-lane data pin | T12 | OK |
| PIN-43 | Pin 43: D3 | S4 Table 4-1 p3-4; Figure 4-1 p3 | `CaptureLanes`: lane bit 3 (8-lane); D7 also the 1-lane data pin | T12 | OK |
| PIN-44 | Pin 44: D2 | S4 Table 4-1 p3-4; Figure 4-1 p3 | `CaptureLanes`: lane bit 2 (8-lane); D7 also the 1-lane data pin | T12 | OK |
| PIN-45 | Pin 45: D1 | S4 Table 4-1 p3-4; Figure 4-1 p3 | `CaptureLanes`: lane bit 1 (8-lane); D7 also the 1-lane data pin | T12 | OK |
| PIN-46 | Pin 46: D0 | S4 Table 4-1 p3-4; Figure 4-1 p3 | `CaptureLanes`: lane bit 0 (8-lane); D7 also the 1-lane data pin | T12 | OK |
| PIN-47 | Pin 47: IOVDD | S4 Table 4-1 p3-4; Figure 4-1 p3 | hardware.yml supply; `Supply::iovdd_mv`, range checked in `Validate` | T27,T29 | OK |
| PIN-48 | Pin 48: GND | S4 Table 4-1 p3-4; Figure 4-1 p3 | hardware.yml pin; docs/hardware | T29 | OK |
| PIN-49 | Pin 49: DRDY/ALARM | S4 Table 4-1 p3-4; Figure 4-1 p3 | `CaptureLanes`: lane bit R (8-lane); D7 also the 1-lane data pin | T12 | OK |
| PIN-50 | Pin 50: CONVST | S4 Table 4-1 p3-4; Figure 4-1 p3 | `CtrlPin::Convst` via `GpioSet`, `Convert` | T13 | OK |
| PIN-51 | Pin 51: AVDD_1V8 | S4 Table 4-1 p3-4; Figure 4-1 p3 | hardware.yml supply; `Supply::avdd_1v8_mv`, range checked in `Validate` | T27,T29 | OK |
| PIN-52 | Pin 52: REFCAPA | S4 Table 4-1 p3-4; Figure 4-1 p3 | hardware.yml circuit, 1 uF to REFM | T29 | OK |
| PIN-53 | Pin 53: REFM | S4 Table 4-1 p3-4; Figure 4-1 p3 | hardware.yml pin; reference return | T29 | OK |
| PIN-54 | Pin 54: REFIO | S4 Table 4-1 p3-4; Figure 4-1 p3 | `Reference` choice; hardware.yml circuit 4.7 uF | T07,T29 | OK |
| PIN-55 | Pin 55: GND | S4 Table 4-1 p3-4; Figure 4-1 p3 | hardware.yml pin; docs/hardware | T29 | OK |
| PIN-56 | Pin 56: AVDD_5V | S4 Table 4-1 p3-4; Figure 4-1 p3 | hardware.yml supply; `Supply::avdd_5v_mv`, range checked in `Validate` | T27,T29 | OK |
| PIN-57 | Pin 57: AIN1P | S4 Table 4-1 p3-4; Figure 4-1 p3 | `Input` for channel 1 (bank 1 (local 0)); positive input | T08,T09,T29 | OK |
| PIN-58 | Pin 58: AIN1M | S4 Table 4-1 p3-4; Figure 4-1 p3 | `Input` for channel 1 (bank 1 (local 0)); negative input | T08,T09,T29 | OK |
| PIN-59 | Pin 59: AIN2P | S4 Table 4-1 p3-4; Figure 4-1 p3 | `Input` for channel 2 (bank 1 (local 1)); positive input | T08,T09,T29 | OK |
| PIN-60 | Pin 60: AIN2M | S4 Table 4-1 p3-4; Figure 4-1 p3 | `Input` for channel 2 (bank 1 (local 1)); negative input | T08,T09,T29 | OK |
| PIN-61 | Pin 61: AIN3P | S4 Table 4-1 p3-4; Figure 4-1 p3 | `Input` for channel 3 (bank 1 (local 2)); positive input | T08,T09,T29 | OK |
| PIN-62 | Pin 62: AIN3M | S4 Table 4-1 p3-4; Figure 4-1 p3 | `Input` for channel 3 (bank 1 (local 2)); negative input | T08,T09,T29 | OK |
| PIN-63 | Pin 63: AIN4P | S4 Table 4-1 p3-4; Figure 4-1 p3 | `Input` for channel 4 (bank 1 (local 3)); positive input | T08,T09,T29 | OK |
| PIN-64 | Pin 64: AIN4M | S4 Table 4-1 p3-4; Figure 4-1 p3 | `Input` for channel 4 (bank 1 (local 3)); negative input | T08,T09,T29 | OK |
| PIN-EP | Exposed pad: connect to GND (thermal and ground) | S4 p4, S10 p112 | hardware.yml `exposed_pad` | T29 | OK |
| ELE-ABS-01 | Absolute maximum: AVDD_5V to GND -0.3 to 6 V | S5.1 p5 | not provided: no constant; the limit is in docs/hardware.md | T27 | GAP |
| ELE-ABS-02 | Absolute maximum: AVDD_1V8 to GND -0.3 to 2.1 V | S5.1 p5 | not provided: no constant; the limit is in docs/hardware.md | T27 | GAP |
| ELE-ABS-03 | Absolute maximum: IOVDD to GND -0.3 to 3.6 V | S5.1 p5 | not provided: no constant; the limit is in docs/hardware.md | T27 | GAP |
| ELE-ABS-04 | Absolute maximum: AINnP, AINnM to GND -18 to 18 V | S5.1 p5 | not provided: no constant; the limit is in docs/hardware.md | T27 | GAP |
| ELE-ABS-05 | Absolute maximum: REFIO to GND -0.3 to VREF+0.3 V | S5.1 p5 | docs/inputs | T27 | INFO |
| ELE-ABS-06 | Absolute maximum: REFM to GND -0.3 to 0.3 V | S5.1 p5 | docs/hardware | T27 | INFO |
| ELE-ABS-07 | Absolute maximum: Digital inputs -0.3 to 3.6 V | S5.1 p5 | docs/hardware | T27 | INFO |
| ELE-ABS-08 | Absolute maximum: Input current to any pin +-10 mA | S5.1 p5 | not provided: no constant; the limit is in docs/hardware.md | T27 | GAP |
| ELE-ABS-09 | Absolute maximum: Junction temperature -40 to 150 C | S5.1 p5 | docs/hardware | T27 | INFO |
| ELE-ABS-10 | Absolute maximum: Storage temperature -60 to 150 C | S5.1 p5 | hardware.yml | T27 | INFO |
| ELE-ESD-1 | ESD rating: HBM analog input pins +-4000 V | S5.2 p5 | docs/hardware | T29 | INFO |
| ELE-ESD-2 | ESD rating: HBM all other pins +-2000 V | S5.2 p5 | docs/hardware | T29 | INFO |
| ELE-ESD-3 | ESD rating: CDM all pins +-500 V | S5.2 p5 | docs/hardware | T29 | INFO |
| ELE-ROC-01 | Recommended operating: AVDD_5V 4.75 to 5.25 V (typ 5) | S5.3 p6 | `Validate` supply range (when the application fills `Config::supply`) | T27 | OK |
| ELE-ROC-02 | Recommended operating: AVDD_1V8 1.7 to 1.9 V | S5.3 p6 | same | T27 | OK |
| ELE-ROC-03 | Recommended operating: IOVDD 1.7 to 3.6 V | S5.3 p6 | same | T27 | OK |
| ELE-ROC-04 | Recommended operating: VREF external 4.086 to 4.106 V | S5.3 p6 | not provided: the driver does not take the external reference voltage | T27 | GAP |
| ELE-ROC-05 | Recommended operating: Differential input AINnP, AINnM -17 to 17 V | S5.3 p6 | docs/inputs; constants | T27 | INFO |
| ELE-ROC-06 | Recommended operating: Common mode -12.5 to 12.5 V | S5.3 p6 | docs/inputs | T27 | INFO |
| ELE-ROC-07 | Recommended operating: Single-ended AINnP -13 to 13 V | S5.3 p6 | docs/inputs | T27 | INFO |
| ELE-ROC-08 | Recommended operating: Single-ended AINnM -0.5 to 0.5 V | S5.3 p6 | docs/inputs | T27 | INFO |
| ELE-ROC-09 | Recommended operating: TA -40 to 125 C | S5.3 p6 | hardware.yml | T27 | INFO |
| ELE-THM-1 | Thermal metric RthetaJA 22.3 C/W (RSK VQFN-64) | S5.4 p6 | hardware.yml thermal note | T29 | INFO |
| ELE-THM-2 | Thermal metric RthetaJC top 8.6 (RSK VQFN-64) | S5.4 p6 | hardware.yml thermal note | T29 | INFO |
| ELE-THM-3 | Thermal metric RthetaJB 6.8 (RSK VQFN-64) | S5.4 p6 | hardware.yml thermal note | T29 | INFO |
| ELE-THM-4 | Thermal metric PsiJT 0.1 (RSK VQFN-64) | S5.4 p6 | hardware.yml thermal note | T29 | INFO |
| ELE-THM-5 | Thermal metric PsiJB 6.7 (RSK VQFN-64) | S5.4 p6 | hardware.yml thermal note | T29 | INFO |
| ELE-THM-6 | Thermal metric RthetaJC bottom 0.8 (RSK VQFN-64) | S5.4 p6 | hardware.yml thermal note | T29 | INFO |
| ELE-EC-01 | Electrical: RIN 0.8/1/1.2 Mohm | S5.5 p7 | `kInputResistanceOhm` | T27 | INFO |
| ELE-EC-02 | Electrical: RIN drift 10/30 ppm/C | S5.5 p7 | docs/inputs | T27 | INFO |
| ELE-EC-03 | Electrical: Input capacitance 10 pF | S5.5 p7 | docs/inputs | T27 | INFO |
| ELE-EC-04 | Electrical: FSR +-2.5 V | S5.5 p7 | `RangeMillivolts` | T27 | INFO |
| ELE-EC-05 | Electrical: FSR +-5 V | S5.5 p7 | `RangeMillivolts` | T27 | INFO |
| ELE-EC-06 | Electrical: FSR +-6.25 V | S5.5 p7 | `RangeMillivolts` | T27 | INFO |
| ELE-EC-07 | Electrical: FSR +-10 V | S5.5 p7 | `RangeMillivolts` | T27 | INFO |
| ELE-EC-08 | Electrical: FSR +-12.5 V | S5.5 p7 | `RangeMillivolts` | T27 | INFO |
| ELE-EC-09 | Electrical: Differential VCM +-12.5 V | S5.5 p7 | docs/inputs | T27 | INFO |
| ELE-EC-10 | Electrical: LPF low 25.5 kHz | S5.5 p7 | not provided: no helper; the bandwidth is in docs/hardware.md | T27 | GAP |
| ELE-EC-11 | Electrical: LPF wide +-2.5 V 280 kHz | S5.5 p7 | not provided: no helper; the bandwidth is in docs/hardware.md | T27 | GAP |
| ELE-EC-12 | Electrical: LPF wide +-5 V 325 kHz | S5.5 p7 | not provided: no helper; the bandwidth is in docs/hardware.md | T27 | GAP |
| ELE-EC-13 | Electrical: LPF wide +-6.25 V 300 kHz | S5.5 p7 | not provided: no helper; the bandwidth is in docs/hardware.md | T27 | GAP |
| ELE-EC-14 | Electrical: LPF wide +-10/12.5 V 350 kHz | S5.5 p7 | not provided: no helper; the bandwidth is in docs/hardware.md | T27 | GAP |
| ELE-EC-15 | Electrical: Phase delay low bandwidth 6.2 us | S5.5 p7 | not provided: no constant; the delay is in docs/hardware.md | T27 | GAP |
| ELE-EC-16 | Electrical: Phase delay wide bandwidth 0.5 us | S5.5 p7 | not provided: no constant; the delay is in docs/hardware.md | T27 | GAP |
| ELE-EC-17 | Electrical: Resolution 16 bits no missing codes | S5.5 p7 | not provided: no constant; the format is `Frame::codes` | T27 | GAP |
| ELE-EC-18 | Electrical: DNL +-0.99 LSB | S5.5 p7 | docs | T27 | INFO |
| ELE-EC-19 | Electrical: INL +-1.5 / +-3 LSB | S5.5 p7 | docs | T27 | INFO |
| ELE-EC-20 | Electrical: Offset error +-2.5 V: +-15 typ, +-30 max LSB | S5.5 p7 | docs/calibration | T27 | INFO |
| ELE-EC-21 | Electrical: Offset error other ranges +-8 typ, +-20 max LSB | S5.5 p7 | docs/calibration | T27 | INFO |
| ELE-EC-22 | Electrical: Offset drift 1 to 3 ppm/C | S5.5 p7-8 | docs | T27 | INFO |
| ELE-EC-23 | Electrical: Gain error +-0.012 typ, +-0.025 max %FSR | S5.5 p8 | docs/calibration | T27 | INFO |
| ELE-EC-24 | Electrical: Gain drift 0.8/2.5 ppm/C | S5.5 p8 | docs | T27 | INFO |
| ELE-EC-25 | Electrical: Single-ended open-wire safe: DNL, INL, offset +-15/+-8 LSB, gain +-0.012 % | S5.5 p8 | docs | T27 | INFO |
| ELE-EC-26 | Electrical: SNR single-ended low bandwidth 84 to 88 dB | S5.5 p8 | docs | T27 | INFO |
| ELE-EC-27 | Electrical: SNR single-ended wide bandwidth 75 to 84 dB | S5.5 p8 | docs | T27 | INFO |
| ELE-EC-28 | Electrical: SINAD single-ended | S5.5 p8 | docs | T27 | INFO |
| ELE-EC-29 | Electrical: THD -103 dB, SFDR 100 dB | S5.5 p8 | docs | T27 | INFO |
| ELE-EC-30 | Electrical: Isolation crosstalk -100 dB at dc | S5.5 p8 | docs | T27 | INFO |
| ELE-EC-31 | Electrical: SNR differential low bandwidth 80.5 to 88 dB | S5.5 p9 | docs | T27 | INFO |
| ELE-EC-32 | Electrical: SNR differential wide bandwidth 72 to 83 dB | S5.5 p9 | docs | T27 | INFO |
| ELE-EC-33 | Electrical: SINAD differential | S5.5 p9 | docs | T27 | INFO |
| ELE-EC-34 | Electrical: CMRR 74 dB no CM correction, 100 dB with correction at dV 24 V | S5.5 p9 | not provided: no type; `Input::cm_correction` is a flag | T27 | GAP |
| ELE-EC-35 | Electrical: Open-wire-safe SNR/SINAD figures | S5.5 p9-10 | docs | T27 | INFO |
| ELE-EC-36 | Electrical: VREF internal 4.086/4.096/4.106 V | S5.5 p10 | `kInternalVrefMv` | T27 | INFO |
| ELE-EC-37 | Electrical: Reference drift 15 ppm/C | S5.5 p10 | docs | T27 | INFO |
| ELE-EC-38 | Electrical: Reference buffer impedance 1 kohm | S5.5 p10 | docs | T27 | INFO |
| ELE-EC-39 | Electrical: Reference turn-on 30 ms with 1 uF | S5.5 p10 | `kPowerUpMs`; wait after Reference change (A44) | T27 | INFO |
| ELE-EC-40 | Electrical: Reference input leakage +-10 nA | S5.5 p10 | docs | T27 | INFO |
| ELE-EC-41 | Electrical: VIL 0.3 IOVDD, VIH 0.7 IOVDD | S5.5 p10 | hardware.yml | T27 | INFO |
| ELE-EC-42 | Electrical: VOL 0.4 V at 200 uA, VOH IOVDD-0.4 V | S5.5 p10 | hardware.yml | T27 | INFO |
| ELE-EC-43 | Electrical: Power dissipation 230 typ, 295 max mW | S5.5 p10 | docs/hardware | T27 | INFO |
| ELE-EC-44 | Electrical: IAVDD_5V 30/35 mA | S5.5 p10 | docs/hardware | T27 | INFO |
| ELE-EC-45 | Electrical: IAVDD_1V8 32/38 mA | S5.5 p10 | docs/hardware | T27 | INFO |
| ELE-EC-46 | Electrical: IIOVDD 6/10 mA | S5.5 p10 | docs/hardware | T27 | INFO |
| TIM-01 | Timing: fCONVST 0.1 to 1 MHz | S5.6 p11 | `Sampling.convst_hz` range in `Validate`; `kConvstMinHz`, `kConvstMaxHz` | T13,T27 | OK |
| TIM-02 | Timing: tCONVST = 1/fCONVST | S5.6 p11 | not provided: no helper; the period is 1 / `Sampling::convst_hz` | T13 | GAP |
| TIM-03 | Timing: tPL_CV CONVST high >= 50 ns | S5.6 p11 | `kConvstHighMinNs` (names swapped, A46) | T13 | OK |
| TIM-04 | Timing: tPH_CV CONVST low >= 50 ns (internal OS: tCONVST-1.2us over N, fCONVST <= 800 kHz) | S5.6 p11, Fig 5-7 p16 | not provided: no helper; `Convert` holds CONVST low 50 ns or longer | T13 | GAP |
| TIM-05 | Timing: tsu_CV_hi_DRDY_hi 50 ns (internal clock oversampling only) | S5.6 p11 | `Validate` rejects a CONVST rate out of range | T13 | OK |
| TIM-06 | Timing: fSCLK <= 60 MHz | S5.6 p11 | `kMaxClockHz` (writes, 60 MHz) and `kMaxSdoutClockHz` (reads on SDOUT, mode 0); the port sets the clock (docs/porting.md) | T27 | OK |
| TIM-07 | Timing: tSCLK >= 16 ns | S5.6 p11 | same | T27 | OK |
| TIM-08 | Timing: SCLK high and low 0.45 to 0.55 tSCLK | S5.6 p11 | port documentation | T29 | GUARD |
| TIM-09 | Timing: thi_CS >= 16 ns | S5.6 p11 | port documentation; `Transfer` contract | T29 | GUARD |
| TIM-10 | Timing: tsu_CSCK >= 16 ns | S5.6 p11 | port documentation | T29 | GUARD |
| TIM-11 | Timing: tht_CSCK >= 16 ns | S5.6 p11 | port documentation | T29 | GUARD |
| TIM-12 | Timing: tsu_CKDI >= 3 ns | S5.6 p11 | port documentation | T29 | GUARD |
| TIM-13 | Timing: tht_CKDI >= 1 ns | S5.6 p11 | port documentation | T29 | GUARD |
| TIM-14 | Timing: tsu_DRDYCS >= 0 ns | S5.6 p11 | `ReadFrame` after DRDY | T13 | GUARD |
| TIM-15 | Timing: tht_DRDYCS >= 0 ns | S5.6 p11 | `ReadFrame` deadline (A48) | T13 | GUARD |
| TIM-16 | Timing: tPU 30 ms (1 uF on REFCAP) | S5.7 p12 | `kPowerUpMs` 30 in `EnsureInitialized` and after reset | T06 | OK |
| TIM-17 | Timing: td_CSDO <= 16 ns | S5.7 p12 | port documentation | T29 | OK |
| TIM-18 | Timing: tdz_CSDO <= 7.5 ns | S5.7 p12 | port documentation | T29 | GUARD |
| TIM-19 | Timing: tvt_CKDO >= 7.6 ns | S5.7 p12 | port documentation (sampling edge rule, A2) | T29 | GUARD |
| TIM-20 | Timing: td_CKDO <= 17 ns | S5.7 p12 | port documentation (A2) | T29 | GUARD |
| TIM-21 | Timing: tCYC = 1/fCONVST | S5.7 p12 | engine | T13 | GUARD |
| TIM-22 | Timing: tCONV 1.25 us without oversampling | S5.7 p12 | `ConversionTimeUs` | T13 | OK |
| TIM-23 | Timing: tCONV (N-1) tCYC + 1.25 us with oversampling N | S5.7 p12 | `ConversionTimeUs` | T13 | OK |
| TIM-24 | Timing: td_CVDRDY_hi = tCONV | S5.7 p12 | `WaitDataReady` Time source | T13 | OK |
| TIM-25 | Timing: td_CVDRDY_lo 350 ns | S5.7 p12 | `Convert()` waits before polling DRDY (A45) | T13 | OK |
| TIM-26 | Timing: td_ALARM = tCONV | S5.7 p12 | not provided: no call; `ReadAlarmPin` reads the pin | T19 | GAP |
| PKG-1 | Package RSK VQFN-64, 8.0 x 8.0 mm (7.9 to 8.1), 0.4 mm pitch | S11 p112 | hardware.yml package/outline | T29 | OK |
| PKG-2 | Height 1.0 max (0.8 min) | S11 p112 | hardware.yml outline | T29 | OK |
| PKG-3 | Exposed pad 5.31 mm (RSK0064B) and 5.56 mm (RSK0064E) | S11 p112,p119 | hardware.yml variants | T29 | OK |
| PKG-4 | Orderable ADS9324RSKR 3500 per reel, MSL3, 260 C peak | S11 p115 | hardware.yml variants; README card | T29 | OK |
| PKG-5 | Product status: initial release SBASB22, Dec 2025 | S1 p1 | docs/datasheets.yml | T29 | INFO |
| PKG-6 | Family: ADS93x4 variants referenced in the revision list | S3 p2 | Identify accepts 0x0004 only | T29 | INFO |
