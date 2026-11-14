# Figures

<!-- Written by kokeb-docs figures from docs/diagrams/figures/*.py. Do not edit by hand. -->

Every figure on this site is drawn by a Python module from facts extracted from the code, so it is rebuilt,
not redrawn, when the code changes. CI runs `kokeb-docs figures --check` and fails when a
committed figure no longer matches the code. Each figure below lists the files it is drawn from.

## Inside the ADS9324

<!-- figure: chip-blocks -->
<picture>
  <source media="(prefers-color-scheme: dark)" srcset="chip-blocks.dark.svg">
  <img src="chip-blocks.svg"
    alt="Block diagram of the ADS9324: its function blocks, how they connect and which package pins reach
    each block.">
</picture>
<!-- /figure -->

Hardware description: [`docs/hardware.yml`](../hardware.yml)

Drawn from:

- [`docs/hardware.yml`](../hardware.yml)
- [`inc/ads9324.hpp`](../../inc/ads9324.hpp)
- [`inc/ads9324_spi_interface.hpp`](../../inc/ads9324_spi_interface.hpp)

## What the ADS9324 does

<!-- figure: chip-capabilities -->
<picture>
  <source media="(prefers-color-scheme: dark)" srcset="chip-capabilities.dark.svg">
  <img src="chip-capabilities.svg"
    alt="Capability map of the ADS9324: Simultaneous sampling (supported): Samples all 16 inputs at one
    instant on CONVST and reads the whole frame of results; Programmable gain front end (supported): Per
    channel: ±2.5 V to ±12.5 V, differential, single-ended or open-wire safe, 25 or 325 kHz bandwidth;
    Reference (supported): The internal 4.096 V reference and buffer, or an external reference;
    Calibration (supported): System offset, gain and phase per channel, and the chip's own ADC gain and
    offset calibration; Digital filters (supported): Block average, moving average or low-pass FIR
    filtering of the oversampled results; Window comparator (supported): Flags a channel that leaves a
    high and low threshold with hysteresis, on the ALARM pin; Data interface (partial): 1, 2, 4 or 8
    data lanes, 16- or 24-bit words, the output randomizer and test patterns; Standby (supported):
    Powers down the analog front end and the reference, then wakes with the registers checked; Reset and
    identity (supported): Hardware or software reset, the device ID check and the reset-detect flag.">
</picture>
<!-- /figure -->

Hardware description: [`docs/hardware.yml`](../hardware.yml)

Drawn from:

- [`docs/hardware.yml`](../hardware.yml)
- [`inc/ads9324.hpp`](../../inc/ads9324.hpp)
- [`inc/ads9324_spi_interface.hpp`](../../inc/ads9324_spi_interface.hpp)

## ADS9324: Typical application

<!-- figure: chip-circuit-typical -->
<picture>
  <source media="(prefers-color-scheme: dark)" srcset="chip-circuit-typical.dark.svg">
  <img src="chip-circuit-typical.svg"
    alt="Schematic of the ADS9324 typical application: the chip with U2 MCU, C1 capacitor 1 µF, C2 capacitor
    0.1 µF, C3 capacitor 1 µF, C4 capacitor 0.1 µF, C5 capacitor 1 µF, C6 capacitor 0.1 µF, C7 capacitor
    4.7 µF, C8 capacitor 1 µF, C9 capacitor 1 µF, R1 resistor 1 kΩ, R2 resistor 1 kΩ, C10 capacitor 10
    nF; the MCU wires carry the port hooks GpioGet(CtrlPin::DrdyAlarm), GpioSet(CtrlPin::Convst),
    GpioSet(CtrlPin::Reset), Transfer().">
</picture>
<!-- /figure -->

Hardware description: [`docs/hardware.yml`](../hardware.yml)

Drawn from:

- [`docs/hardware.yml`](../hardware.yml)
- [`inc/ads9324.hpp`](../../inc/ads9324.hpp)
- [`inc/ads9324_spi_interface.hpp`](../../inc/ads9324_spi_interface.hpp)

## ADS9324: SPI register frame

<!-- figure: chip-frame-spi -->
<picture>
  <source media="(prefers-color-scheme: dark)" srcset="chip-frame-spi.dark.svg">
  <img src="chip-frame-spi.svg"
    alt="SPI register frame of the ADS9324, 24 bits, most significant bit first. Write: register address
    (bits 23 to 16), 16-bit value (bits 15 to 0); Read: 0x01 (GEN_CFG1) (bits 23 to 16), register to
    read (bits 15 to 8), 0x01 (read) (bits 7 to 0); Reply: 16-bit register value (bits 23 to 8), 0x00
    (bits 7 to 0). Below, the SPI mode 0 waveform of one transfer in the same bit grid.">
</picture>
<!-- /figure -->

Hardware description: [`docs/hardware.yml`](../hardware.yml)

Drawn from:

- [`docs/hardware.yml`](../hardware.yml)
- [`inc/ads9324.hpp`](../../inc/ads9324.hpp)
- [`inc/ads9324_spi_interface.hpp`](../../inc/ads9324_spi_interface.hpp)

## ADS9324

<!-- figure: chip-hero -->
<picture>
  <source media="(prefers-color-scheme: dark)" srcset="chip-hero.dark.svg">
  <img src="chip-hero.svg"
    alt="Overview of the ADS9324, a 16-channel, 16-bit SAR ADC that samples all of its inputs at the same
    instant at up to 1 MSPS, each through its own programmable-gain front end with 1 MΩ input impedance,
    so sensors and transformers connect without drivers. A MCU talks to it over SPI, 1 to 8 data lanes;
    it drives DRDY/ALARM line; it reads back Sensors and transformers. Headline figures: 16 x 16-bit
    inputs sampled at one instant; 1 MSPS on each channel; ±12.5 V widest input range; down to ±2.5 V; 1
    MΩ input impedance of the PGA; 4.096 V internal reference, 15 ppm/°C typical; 8 x 8 mm VQFN-64 body.">
</picture>
<!-- /figure -->

Hardware description: [`docs/hardware.yml`](../hardware.yml)

Drawn from:

- [`docs/hardware.yml`](../hardware.yml)
- [`inc/ads9324.hpp`](../../inc/ads9324.hpp)
- [`inc/ads9324_spi_interface.hpp`](../../inc/ads9324_spi_interface.hpp)

## ADS9324 package, VQFN-64

<!-- figure: chip-outline -->
<picture>
  <source media="(prefers-color-scheme: dark)" srcset="chip-outline.dark.svg">
  <img src="chip-outline.svg"
    alt="Outline of the ADS9324 in VQFN-64 drawn to scale from above: body 8.0 x 8.0 x 1.0 mm, pitch 0.4 mm,
    64 pins and an exposed pad.">
</picture>
<!-- /figure -->

Hardware description: [`docs/hardware.yml`](../hardware.yml)

Drawn from:

- [`docs/hardware.yml`](../hardware.yml)
- [`inc/ads9324.hpp`](../../inc/ads9324.hpp)
- [`inc/ads9324_spi_interface.hpp`](../../inc/ads9324_spi_interface.hpp)

## ADS9324 pinout, VQFN-64

<!-- figure: chip-pinout -->
<picture>
  <source media="(prefers-color-scheme: dark)" srcset="chip-pinout.dark.svg">
  <img src="chip-pinout.svg"
    alt="Pinout of the ADS9324 in VQFN-64, top view: 64 pins and an exposed pad, each with its name, its
    electrical type as a colour and the driver hook that uses it.">
</picture>
<!-- /figure -->

Hardware description: [`docs/hardware.yml`](../hardware.yml)

Drawn from:

- [`docs/hardware.yml`](../hardware.yml)
- [`inc/ads9324.hpp`](../../inc/ads9324.hpp)
- [`inc/ads9324_spi_interface.hpp`](../../inc/ads9324_spi_interface.hpp)

## Wiring: Typical application

<!-- figure: chip-wiring-typical -->
<picture>
  <source media="(prefers-color-scheme: dark)" srcset="chip-wiring-typical.dark.svg">
  <img src="chip-wiring-typical.svg"
    alt="Wiring of the ADS9324 for the Typical application: MCU SPI SCK to SCLK (Transfer()); MCU SPI MOSI to
    SDI (Transfer()); MCU SPI MISO to SDOUT (Transfer()); MCU GPIO / CS to CS (Transfer()); MCU GPIO to
    RESET (GpioSet(CtrlPin::Reset)); MCU GPIO / timer to CONVST (GpioSet(CtrlPin::Convst)); MCU GPIO
    (IRQ) to DRDY/ALARM (GpioGet(CtrlPin::DrdyAlarm)). Supplies, grounds and the support parts are
    listed under the drawing.">
</picture>
<!-- /figure -->

Hardware description: [`docs/hardware.yml`](../hardware.yml)

Drawn from:

- [`docs/hardware.yml`](../hardware.yml)
- [`inc/ads9324.hpp`](../../inc/ads9324.hpp)
- [`inc/ads9324_spi_interface.hpp`](../../inc/ads9324_spi_interface.hpp)

## The layers, one of them yours

<!-- figure: driver-layers -->
<picture>
  <source media="(prefers-color-scheme: dark)" srcset="driver-layers.dark.svg">
  <img src="driver-layers.svg"
    alt="Layer diagram: your application calls the ADS9324 core, which sends register reads and writes
    through the SPI port contract to the port class you write, which drives the chip; the core and the
    contract are the same on every MCU.">
</picture>
<!-- /figure -->

Figure module: [`docs/diagrams/figures/driver-layers.py`](../../docs/diagrams/figures/driver-layers.py)

Drawn from:

- `public calls` in [`inc/ads9324.hpp`](../../inc/ads9324.hpp)
- `SpiInterface` in [`inc/ads9324_spi_interface.hpp`](../../inc/ads9324_spi_interface.hpp)

## One call through the layers

<!-- figure: one-call-through-the-layers -->
<picture>
  <source media="(prefers-color-scheme: dark)" srcset="one-call-through-the-layers.dark.svg">
  <img src="one-call-through-the-layers.svg"
    alt="One call through the layers: your code calls MeasureOnce(), the core exchanges frames with the chip
    through your port and returns the result to your code.">
</picture>
<!-- /figure -->

Hardware description: [`docs/model/one-call-through-the-layers.seq.yml`](../model/one-call-through-the-layers.seq.yml)

Drawn from:

- [`docs/model/one-call-through-the-layers.seq.yml`](../model/one-call-through-the-layers.seq.yml)

## One register read on the SPI bus

<!-- figure: spi-register-read -->
<picture>
  <source media="(prefers-color-scheme: dark)" srcset="spi-register-read.dark.svg">
  <img src="spi-register-read.svg"
    alt="A register read on the ADS9324 SPI bus: the host sends the read command inside one chip-select
    frame, then clocks out a second frame in which the chip returns the value on SDOUT.">
</picture>
<!-- /figure -->

Hardware description: [`docs/model/spi-register-read.seq.yml`](../model/spi-register-read.seq.yml)

Drawn from:

- [`docs/model/spi-register-read.seq.yml`](../model/spi-register-read.seq.yml)
