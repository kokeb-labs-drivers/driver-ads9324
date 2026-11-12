# Troubleshooting

Symptoms, causes and fixes, cheapest check first.

<!-- generated: datasheet-refs -->
??? abstract "Datasheet sections this page relies on"

    From Texas Instruments SBASB22 -:

    - Reset: 7.4.1 Reset, [p. 49](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=49)

    All of them, with the driver API for each: [Datasheet map](datasheet.md).
<!-- /generated -->

## EnsureInitialized returns Bus

The port reported a failure: either `EnsureInitialized()` returned `false` or a transfer did.

1. Check that your port's `EnsureInitialized()` returns `true` on its own (log it).
2. Check the bus pins in your port against the board.
3. With a logic analyser, check that the first frame (a write to GEN_CFG1, the software reset) appears on the bus
   as three bytes with CS low for all of them.

## EnsureInitialized returns DeviceIdMismatch

DEVICE_ID did not read `0x0004`, or BANK_SEL did not read as the driver left it. The chip answered with something
else, or nothing answered.

| Read value | Likely cause | Fix |
| --- | --- | --- |
| `0x0000` | SDOUT not connected, or CS not reaching the chip | Check SDOUT and CS continuity |
| `0xFFFF` | SDOUT floating high, or the chip unpowered | Check the three supplies and that SDOUT is not pulled up hard |
| Anything else | Wrong SPI mode, or a shifted bit | Use mode 0; check that CS stays low for the whole 24-bit frame and that the clock is at most 29 MHz (`kMaxSdoutClockHz`; 60 MHz is the table limit for writes only) |

## EnsureInitialized returns Timeout

CALIB_BUSY did not clear after the reset. Check the three supplies, that RESET is high (or released by your
`GpioSet()` with `GpioSignal::INACTIVE`), and the reference: with `ReferenceSource::External` the chip needs a
4.096 V source on REFIO.

## EnsureInitialized returns Unsupported

`Config::hardware_reset` is set but the port has no `GpioSet()`, or the configuration asks for the daisy chain
(out of scope: [Limitations](limitations.md)).

## EnsureInitialized returns InvalidArgument

`Validate()` rejected the configuration and nothing was sent. Call `ads9324::Validate(config)` on the host to see
which rule: the combinations are listed on [Configuration](configuration.md#validation).

## ReadFrame returns DataMismatch

The frame is all zeros or all ones, which is what a stuck or floating SDOUT reads. A real conversion of an input at
a code limit is never all `0x00` or all `0xFF` across 16 channels.

1. Check SDOUT, CS and the SPI mode.
2. Check that a conversion happened: pulse CONVST (or use `MeasureOnce()`) before `ReadFrame()`.
3. Run `RunDataPathSelfTest()`: it uses test patterns, so it works with the inputs unconnected.
4. When you are sure the frame is a real one, `Sampling::reject_stuck_frames = false` turns the check off.

## A channel reads another channel's value

The AIN9 to AIN16 registers are in reverse order; the driver maps them, so a swap points at the wiring or at the
frame layout. Check the channel count and group (`ChannelSelect`): with 8 channels the frame carries AIN1 to AIN8 or
AIN9 to AIN16 only, and `Frame::present_mask` says which. `RunDataPathSelfTest()` returns `DataMismatch` when a
channel returns another channel's pattern.

## Reads return UnexpectedReset

The DIAG byte or RESET_DETECT_FLAG reported a reset: the chip reset since the driver started it, usually a brown-out
on one of the supplies. The driver is uninitialised again; `EnsureInitialized()` writes the configuration again.
If it repeats, check the supplies and their decoupling. `Wake()` returns it too when the registers did not
survive standby.

## A reading sits at a code limit

`Frame::clipped_mask` has the channel's bit set: the input is outside the range. Choose a larger `InputRange`, or
check the input for a missing divider. An open input pair reads near zero code only in the open-wire safe type.

## The voltage is wrong by a fixed factor

`microvolts` is computed from the channel's configured range; if you changed a channel's range outside the
driver, the arithmetic uses the old one. Use `SetInput()`, and `GetConfig().inputs[n]` to see what the driver
holds. Offset and gain errors of the board go into `Input::offset_quarter_lsb` and `Input::gain`.

## The window alert never fires, or fires at once

The thresholds compare the top byte of the code (two's complement), not volts: ±40 on the top byte is ±40 x 256
codes. The flags are latched: `ReadWindowFlags()` shows what happened since the last `ClearWindowFlags()`. A window
cannot be used with offset binary data (`Validate()` refuses it).

## RunAdcCalibration returns Refused or Timeout

`Refused`: a digital filter is on (switch it off with `SetFilter()` first) or the chip is in standby. `Timeout`:
CALIB_BUSY stayed set for 150 ms; check the supplies, the reference and, for the free-running mode, that CONVST
really runs.

## MeasureOnce returns Timeout

With `ReadySource::Pin` the DRDY pin did not assert within `Sampling::ready_timeout_us`: check that
`Alarm::function` is `Drdy`, the polarity and the wiring, and that CONVST really pulses. Check that your port's
`DelayUs()` really waits: a delay that returns at once makes every timeout far too short.
