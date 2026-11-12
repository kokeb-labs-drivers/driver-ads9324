# Examples

One folder per platform under `examples/`, each with its own port, built by CI on every pull request. The driver
itself has no platform code; [Platforms](platforms.md) lists what exists.

## Host

[`examples/host/`](../examples/host/README.md) runs the driver on any PC against a simulated ADS9324, through the
host port [`host_ads9324_port.hpp`](../examples/host/host_ads9324_port.hpp). No hardware:

```bash
cmake -S examples/host -B build-example && cmake --build build-example && ./build-example/ads9324_host_example
```

The host tests build it with warnings as errors and run it (CTest `ads9324_host_example`).

## ESP32

ESP-IDF applications that run the driver on an ESP32-C6 DevKitC-1 with an ADS9324 on its SPI bus; the wiring is in the
[example folder's README](../examples/esp32/README.md) and the port in
[`esp32_ads9324_port.hpp`](../examples/esp32/main/esp32_ads9324_port.hpp). The project points
`EXTRA_COMPONENT_DIRS` at the repository root, which is the driver's ESP-IDF component.

### The apps

<!-- generated: examples -->
| App | What it shows | Source | CI |
| --- | --- | --- | --- |
| `ads9324_basic_example` :material-star:{ title="featured" } | Initialise the ADS9324 from menuconfig values and log four of the 16 channels twice a second | [`ads9324_basic_example.cpp`](../examples/esp32/main/ads9324_basic_example.cpp) | yes |

Build and flash one (from `examples/esp32`, ESP-IDF environment loaded):

```bash
./scripts/build_app.sh ads9324_basic_example Release
./scripts/flash_app.sh flash_monitor ads9324_basic_example Release
```

Source of this table: [`examples/esp32/app_config.yml`](../examples/esp32/app_config.yml).
<!-- /generated -->

### Set up once

```bash
git submodule update --init examples/esp32/scripts   # the ESP-IDF build scripts
cd examples/esp32
. $IDF_PATH/export.sh                                 # ESP-IDF v5.5
./scripts/build_app.sh list                           # every app and build type
```

The scripts read [`app_config.yml`](../examples/esp32/app_config.yml) (the same file this page's table comes
from) and build each app as its own firmware image.

### What to read in an example

| You want to | Read |
| --- | --- |
| See the lifecycle and every `Result` checked | `ads9324_basic_example.cpp` |
| Write an ESP-IDF port | `esp32_ads9324_port.hpp` |
| Use menuconfig values | `ads9324_kconfig.hpp` and [`_config/Kconfig`](../_config/Kconfig) |
