# ADS9324 host example

The driver on your PC, with no hardware: the host port in
[`host_ads9324_port.hpp`](host_ads9324_port.hpp) connects the real driver to a simulated ADS9324 (the chip model
the host tests use), and [`main.cpp`](main.cpp) initialises it, reads the 16 channels, sets a window
comparator on one channel and runs the ADC calibration.

## Build and run

From the repository root, with CMake 3.16+ and a C++20 compiler:

```bash
cmake -S examples/host -B build-example
cmake --build build-example
./build-example/ads9324_host_example
```

## Expected output

```text
ADS9324 driver 0.1.0 on the host
ads9324: initialised
  AIN1    -3996 mV
  ...
  AIN16    3503 mV
window alert pin asserted, low flags 0x0004
calibration done, mode kept: idle
```

The host tests build this folder too and run the example (CTest `ads9324_host_example`), so CI fails if it stops
compiling or a call fails.

## Files

| File | What it is |
| --- | --- |
| [`main.cpp`](main.cpp) | The example: initialise, read, print |
| [`host_ads9324_port.hpp`](host_ads9324_port.hpp) | The host port: bus hooks over the simulated chip, `DelayUs()` and `Log()` |
| [`CMakeLists.txt`](CMakeLists.txt) | Standalone project; adds the driver with `add_subdirectory(../..)` |
