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
