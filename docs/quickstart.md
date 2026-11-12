# Quickstart

Run the driver on your PC first, with no hardware; then read the 16 channels from your MCU: wire the chip, write a
small port class, call two driver functions.

<!-- generated: datasheet-refs -->
??? abstract "Datasheet sections this page relies on"

    From Texas Instruments SBASB22 -:

    - Pins, package and the data lanes: 4 Pin Configuration and Functions, [p. 3](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=3)
    - Initialization example, single lane on SDOUT: 7.5.3 Initialization Example - Single Lane Mode on SDOUT, [p. 51](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=51)

    All of them, with the driver API for each: [Datasheet map](datasheet.md).
<!-- /generated -->

## Try it on your PC

[`examples/host/`](../examples/host/README.md) runs the real driver against a simulated ADS9324, through a host
port. From the repository root, with CMake and a C++20 compiler:

```bash
cmake -S examples/host -B build-example
cmake --build build-example
./build-example/ads9324_host_example
```

```text
ADS9324 driver 0.1.0 on the host
ads9324: initialised
  AIN1    -3996 mV
  ...
  AIN16    3503 mV
window alert pin asserted, low flags 0x0004
calibration done, mode kept: idle
```

The calls in [`main.cpp`](../examples/host/main.cpp) are the ones below; only the port differs on an MCU. CI builds
and runs this example on every pull request.

## 1. Wire it

| MCU | ADS9324 | Notes |
| --- | --- | --- |
| SPI SCLK | SCLK | Mode 0, up to 60 MHz; 1 MHz for bring-up on jumper wires |
| SPI MOSI | SDI | |
| SPI MISO | SDOUT | The registers and, in the default configuration, the data of all 16 channels |
| GPIO (chip select) | CS | Low for the whole frame: one transfer per frame |
| GPIO | CONVST | Idles high; the falling edge samples all 16 channels. A timer may drive it instead |
| GPIO | RESET | Idles high; optional, tie it high if the port has no hook |
| GPIO input | DRDY/ALARM | Optional: with it the driver waits for the result, without it for the conversion time |
| 5 V, 1.8 V, 1.7 V to 3.6 V | AVDD_5V, AVDD_1V8, IOVDD | 1 µF and 0.1 µF at each supply pin |
| GND | GND, REFM, exposed pad | |

REFSEL high selects the internal reference (the default). The [Hardware](hardware.md) page has the full pin list,
the reference and the timing. Every analog input needs a series resistor: the datasheet rating is ±18 V.

## 2. Add the driver

Add the repository as a submodule and link the `ads9324::ads9324` CMake target. [Installation](installation.md)
covers CMake on any MCU, the installed package and ESP-IDF.

```bash
git submodule add https://github.com/kokeb-labs-drivers/driver-ads9324.git external/driver-ads9324
```

## 3. Write the port

The port is the only platform code: it brings the bus up and moves frames. Replace the `platform_*` calls with your
SDK's.

```cpp title="my_port.hpp"
--8<-- "tests/docs/my_port.hpp:port"
```

[Porting](porting.md) explains each method and the optional hooks. A port with `GpioSet()` for CONVST and RESET and
`GpioGet()` for DRDY lets `MeasureOnce()` run a whole conversion.

## 4. Read a frame

```cpp title="main.cpp"
--8<-- "tests/docs/quickstart.cpp:quickstart"
```

What each step does:

1. `EnsureInitialized()` calls your port's `EnsureInitialized()`, waits the power-up time, resets the chip, checks
   that DEVICE_ID reads `0x0004`, waits for the calibration to idle and writes `Config{}`: 16 channels, 16-bit
   words, one lane on SDOUT, ±5 V differential inputs.
2. `ReadFrame()` clocks one data frame of zeros and decodes it; with a port that has `GpioSet()`,
   `MeasureOnce()` pulses CONVST and waits for the result first.
3. `microvolts[0]` is AIN1 in microvolts, from the code and the channel's range.

This example is built by the host CI, so it compiles against the current headers.

## If it does not work

| Symptom | First thing to check |
| --- | --- |
| `EnsureInitialized()` returns `Bus` | Your port's `EnsureInitialized()`: the bus did not come up |
| `EnsureInitialized()` returns `DeviceIdMismatch` | The SPI mode, the wiring and IOVDD ([Troubleshooting](troubleshooting.md#ensureinitialized-returns-deviceidmismatch)) |
| `EnsureInitialized()` returns `Timeout` | The supplies and the reference: calibration stayed busy |
| `ReadFrame()` returns `DataMismatch` | SDOUT is stuck low or high ([Troubleshooting](troubleshooting.md#readframe-returns-datamismatch)) |

## Next

- [API guide](api-guide.md): measurements, ranges, windows, calibration, errors.
- [Configuration](configuration.md): every `Config` field, the data interface, Kconfig.
- [Examples](examples.md): the host example and the MCU examples.
- [Platforms](platforms.md): which MCUs have a port and an example.
