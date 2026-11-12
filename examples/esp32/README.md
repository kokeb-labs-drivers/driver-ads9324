# ADS9324 ESP32 examples

ESP-IDF applications that run the ADS9324 driver on an ESP32-C6, built by CI on every pull request.

## Wiring (ESP32-C6 DevKitC-1)

| ESP32-C6 | ADS9324 | Notes |
| --- | --- | --- |
| GPIO6 | SCLK | SPI2, mode 0, 10 MHz in the example |
| GPIO7 | SDI | |
| GPIO2 | SDOUT | The data frame is read here (one lane) |
| GPIO10 | CS | Driven by spi_master, one transfer per frame |
| GPIO11 | RESET | Optional; set `reset = -1` in `Esp32Ads9324Pins` when tied high |
| GPIO4 | CONVST | Idles high; the falling edge starts a conversion. `-1` when a timer drives it |
| GPIO5 | DRDY/ALARM | Input; `-1` when not wired (the driver then waits the conversion time) |
| 3V3 | IOVDD | 1.7 V to 3.6 V; the data timing is specified at 1.7 V to 1.9 V |
| GND | GND | |

The analog supplies (AVDD_5V, AVDD_1V8) and the reference come from your board; see the hardware page.

## Build and flash

```bash
git submodule update --init scripts      # the ESP-IDF build scripts
. $IDF_PATH/export.sh                     # ESP-IDF v5.5
./scripts/build_app.sh list
./scripts/build_app.sh ads9324_basic_example Debug
./scripts/flash_app.sh flash_monitor ads9324_basic_example Debug
```

`idf.py menuconfig` shows "ADS9324 16-channel ADC" under Component config: the CONVST frequency, the word
length, the DIAG byte, the ready source and the DRDY timeout that `ads9324::KconfigDefaults()` returns.

## Expected output

```text
I (312) ads9324_example: ADS9324 driver 0.1.0
ads9324: initialised
I (318) ads9324_example: initialised
I (820) ads9324_example: AIN1 -3996 mV, AIN2 -3497 mV, AIN3 -2996 mV, AIN4 -2496 mV
```

An `EnsureInitialized failed: DeviceIdMismatch` line means DEVICE_ID did not read 0x0004: check CS, the SPI mode and
the SDOUT wire first.

## Files

| File | What it is |
| --- | --- |
| [`main/ads9324_basic_example.cpp`](main/ads9324_basic_example.cpp) | The example app |
| [`main/esp32_ads9324_port.hpp`](main/esp32_ads9324_port.hpp) | The ESP-IDF port: bus, RESET, CONVST, DRDY, delay, log |
| [`CMakeLists.txt`](CMakeLists.txt) | The ESP-IDF project; `EXTRA_COMPONENT_DIRS` points at the repository root, which is the component |
| [`app_config.yml`](app_config.yml) | The app matrix for the scripts, CI and the docs |
