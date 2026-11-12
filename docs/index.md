---
hide:
  - navigation
  - toc
---

<div class="kb-hero" markdown>
<span class="kb-kicker">Driver</span>

# ADS9324 Driver

<p>A C++20 driver for the TI ADS9324, a 16-channel, 16-bit, 1 MSPS simultaneous-sampling SAR ADC with an
integrated programmable-gain front end and an SPI interface. It starts the chip safely, programs every channel,
reads and decodes a whole frame of 16 samples, and runs the window comparator and the calibration, on any MCU.</p>

[Quickstart](quickstart.md){ .md-button .md-button--primary }
[API guide](api-guide.md){ .md-button }
[Source on GitHub](https://github.com/kokeb-labs-drivers/driver-ads9324){ .md-button }
</div>

The driver has no platform code. You give it a small class that moves SPI frames and waits; it does the rest: the
reset sequence, the identity check, the three register banks (the AIN9 to AIN16 bank is laid out in reverse order,
and the driver knows), the data frame in every lane and word format, unit conversion and bounded waits.

<div class="kb-chip" markdown>
<div class="kb-chip__head">
<span class="kb-chip__kicker">16-channel simultaneous-sampling ADC</span>
<span class="kb-chip__name">ADS9324</span>
<span class="kb-chip__vendor">Texas Instruments</span>
</div>

Function
:   16 channels sampled together, 16 bits each, up to 1 MSPS per channel; differential or single-ended inputs

Inputs
:   1 MΩ input impedance; ±2.5 V, ±5 V, ±6.25 V, ±10 V and ±12.5 V ranges per channel; 25 kHz or 325 kHz bandwidth

Interface
:   SPI for the registers (24-bit frames, up to 60 MHz); data on 1, 2, 4 or 8 lanes, 16 or 24 bits per channel

Digital
:   Block, moving-average and FIR filters; offset, gain and phase calibration; window comparator; alarm pin

Reference
:   Internal 4.096 V or external on REFIO

Supply
:   5 V analog, 1.8 V analog, 1.7 V to 3.6 V I/O
</div>

## What you get

- **One object per chip.** `ADS9324<YourPort>` takes a reference to your port and does no I/O until
  `EnsureInitialized()`. See [API guide](api-guide.md).
- **Errors you can act on.** Every call returns `Result<T>` with an `Error` such as `Bus`, `Timeout`,
  `DeviceIdMismatch` or `UnexpectedReset`. See [API guide](api-guide.md#handle-errors).
- **A configuration that is checked.** `Config{}` is the datasheet's initialisation example; `Validate()` rejects
  an impossible combination before any bus traffic, and `CheckedConfig` rejects it at compile time. See
  [Configuration](configuration.md).
- **Per-channel settings that land on the right channel.** The AIN9 to AIN16 registers are in reverse order; the
  driver maps every channel and the host tests prove it for all 16. See [Architecture](architecture.md).
- **Every data format.** 1, 2, 4 or 8 lanes, 16- or 24-bit words, the diagnostic byte and the randomizer,
  all 15 channel modes. See [Configuration](configuration.md#data-interface).
- **Zero-overhead portability.** The port is a CRTP class checked by a concept: the bus hooks and a delay, plus
  optional CONVST, RESET, DRDY and lane-capture hooks found at compile time. See [Porting](porting.md).
- **No heap, no exceptions, no RTTI, no floating point.** See [Contract](contract.md).
- **Tested without hardware.** Host tests run the driver against an independent model of the chip. See
  [Testing and CI](testing.md).

## Datasheet

<!-- generated: datasheet-card -->
| Document | Revision | Get it |
| --- | --- | --- |
| Datasheet: ADS9324 16-Channel, 16-Bit, 1MSPS, Simultaneous-Sampling SAR ADC With Integrated Analog Front-End (Texas Instruments SBASB22) | -, 2025-12 | [PDF](https://www.ti.com/lit/ds/symlink/ads9324.pdf) · [vendor page](https://www.ti.com/product/ADS9324) |

Written against these revisions; the SHA-256 of each file is in [`docs/datasheets.yml`](datasheets.yml). Where each part
of the driver is in the datasheet: [Datasheet map](datasheet.md).
<!-- /generated -->

## Where to go next

<div class="grid cards" markdown>

-   :material-rocket-launch-outline: **[Quickstart](quickstart.md)**

    ---

    Wire the chip, write the port, read the 16 channels.

-   :material-chip: **[Hardware](hardware.md)**

    ---

    Pins, supplies, reference, bus settings and timing.

-   :material-file-document-check-outline: **[Contract](contract.md)**

    ---

    What the driver promises, what it needs, threading and errors.

-   :material-code-braces: **[API guide](api-guide.md)**

    ---

    Initialise, configure, measure, set windows and calibrate, task by task.

-   :material-swap-horizontal: **[Porting](porting.md)**

    ---

    Implement the port for your MCU.

-   :material-alert-circle-outline: **[Limitations](limitations.md)**

    ---

    What is not supported, and what is not verified on hardware.

</div>
