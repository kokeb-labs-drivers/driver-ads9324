# Porting

Run the driver on a new MCU by writing one class: the port.

<!-- generated: datasheet-refs -->
??? abstract "Datasheet sections this page relies on"

    From Texas Instruments SBASB22 -:

    - Pins, package and the data lanes: 4 Pin Configuration and Functions, [p. 3](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=3)
    - SPI and CONVST timing: 5.6 Timing Requirements; 5.7 Switching Characteristics, [p. 11](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=11)
    - Timing diagrams, DRDY and the data frame: 5.8 Timing Diagrams, [p. 12](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=12)
    - Data interface: lanes, word length and channel modes: 7.3.14 Data Interface; 7.3.14.1 ADC Channel Modes, Table 7-12, [p. 42](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=42)
    - Output drive strength and output delay: 7.3.14.6 Digital Output Drive Strength Control; 7.3.14.7 Digital Output Delay Adjustment, [p. 48](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=48)
    - Register write and read frames: 7.5 Programming; 7.5.1 Register Write Operation; 7.5.2 Register Read Operation, [p. 50](https://www.ti.com/lit/ds/symlink/ads9324.pdf#page=50)

    All of them, with the driver API for each: [Datasheet map](datasheet.md).
<!-- /generated -->

The driver never calls your SDK. It calls hooks on a class you derive from `ads9324::SpiInterface<Self>` (in
[`inc/ads9324_spi_interface.hpp`](../inc/ads9324_spi_interface.hpp)). The base uses CRTP and declares no hook
itself; the driver template is constrained by the `ads9324::SpiPort` concept, so calls are resolved at compile
time (no virtual functions, nothing to allocate) and a missing or misdeclared hook is a compile error at the point
of use.

## What you implement

| Method | Required | Contract |
| --- | --- | --- |
| `bool EnsureInitialized() noexcept` | yes | Bring the bus up on the first call; return `true` at once afterwards. `false` makes the driver's `EnsureInitialized()` return `Bus` |
| `bool Transfer(const uint8_t* tx, uint8_t* rx, std::size_t len) noexcept` | yes | One full-duplex frame of `len` bytes with CS low for all of them. `rx` is `nullptr` for writes. `false` on any bus error |
| `void DelayUs(uint32_t us) noexcept` | yes | Wait at least `us` microseconds |
| `bool GpioSet(CtrlPin, GpioSignal) noexcept` | no | Drive RESET and CONVST; `false` if the pin write failed or the pin is not wired |
| `bool GpioGet(CtrlPin, bool& high) noexcept` | no | Read DRDY/ALARM; set `high` to the raw level |
| `bool CaptureLanes(uint8_t lanes, uint8_t* rx, std::size_t len) noexcept` | no | Clock a data frame in on 2, 4 or 8 lanes (or 1 lane on D7) |
| `void Log(LogLevel, const char* message) noexcept` | no | Print one line; `message` is plain text, never a format string |

## The port

```cpp title="my_port.hpp"
--8<-- "tests/docs/my_port.hpp:port"
```

Rules the transfer must follow:

- **One call, one frame.** Keep CS low for all `len` bytes; the chip decodes the frame when CS rises. A register
  frame is 3 bytes; a data frame is up to 50 bytes (`kMaxFrameBytes`). Never split a frame into several transfers.
- **Buffers.** The driver owns the transmit and receive buffers (members of the driver object, no heap, never on
  the caller's stack), so a DMA that reads them reaches them; a port that needs DMA-capable memory places the
  driver object there.
- **The bus requirements below**: mode 0, MSB first, up to `kMaxSdoutClockHz` when the port reads SDOUT
  (`kMaxClockHz` for writes only). Start at 1 MHz on jumper wires.
- **Raw bytes only.** Do not interpret, retry or reorder bytes; the driver owns the framing.
- **Return `false` for a bus error.** The driver turns it into `Error::Bus` and never ignores it.
- **Bounded time-outs.** A stuck bus fails the call; it does not block forever.
- **A transmit buffer of zeros while reading data.** The driver sends zeros during a data frame so that no
  register is written by accident; do not replace them.

## Bus requirements

The driver states what the ADS9324 needs from the bus as `constexpr` values in
[`inc/ads9324_spi_interface.hpp`](../inc/ads9324_spi_interface.hpp) (handbook KB-CPP-123). Check your port's
settings against them with `static_assert`, as the ESP32 example port does, so a port that clocks too fast or in
the wrong mode does not compile:

| Constant | Value | Meaning (datasheet 5.6) |
| --- | --- | --- |
| `kSpiMode` | `0` | CPOL 0, CPHA 0 (derived from the timing diagram: the datasheet names no mode) |
| `kMsbFirst` | `true` | Most significant bit first |
| `kMaxClockHz` | `60'000'000` | f_SCLK max (section 5.6), for writes |
| `kMaxSdoutClockHz` | `29'000'000` | f_SCLK max when the port reads SDOUT in mode 0: the 17 ns td_CKDO must fit half a period (A2) |
| `kMinCsHighNs` | `16` | CS high between two frames, thi_CS |
| `kContinuousWord` | `false` | SCLK may pause inside a frame; only CS framing matters |
| `kCsSetupNs`, `kCsHoldNs` | `16`, `16` | CS low to the first SCLK edge, last SCLK edge to CS high |
| `kFrameIsAtomic` | `true` | CS stays low for a whole frame; the chip decodes it when CS rises |
| `kFrameBits` | `24` | A register frame: 8 address bits and 16 data bits |

## Control pins

Each control pin has a defined idle level, and the port drives it there before it enables the pin's output and
before the driver's first transfer (handbook KB-HW-018). Configuring a pin as an output first and setting its level
afterwards gives the chip a pulse it did not ask for, for example a reset or a conversion.

| `CtrlPin` | Package pin | Polarity | Idle level | Before the first transfer |
| --- | --- | --- | --- | --- |
| `Reset` | RESET | active low | high (`INACTIVE`) | Set high, then enable the output |
| `Convst` | CONVST | the falling edge samples; `ACTIVE` is high | high (`ACTIVE`) | Set high, then enable the output: the driver leaves CONVST high after each pulse |
| `DrdyAlarm` | DRDY/ALARM | active high, an input | | Configure as an input |

`GpioSignal::ACTIVE` always means "function asserted"; the port maps it to the electrical level. A pin tied on the
board needs no hook. `Convert()` is one CONVST pulse: `ACTIVE`, then `INACTIVE` (the falling edge samples), then
`ACTIVE` again, so CONVST is high before DRDY rises (Figure 5-7).

## Optional hooks

The driver finds the optional hooks at compile time with named concepts, in one place (`ads9324::detail`). A
missing hook falls back in a documented, safe way or the call returns `Error::Unsupported`; it never skips a step
silently, and a hook declared with the wrong signature does not compile.

| Hook | Used for | Without it |
| --- | --- | --- |
| `DelayUs()` | The power-up and reset times, the timed ready wait, calibration polling | Compile error (every port needs it) |
| `GpioSet()` | RESET (`Config::hardware_reset`, `Reset(Hardware)`) and CONVST (`Convert()`, `MeasureOnce()`, the SYSREF pulse after a filter change, single-shot calibration) | `Unsupported` from those calls; drive CONVST yourself and use `ReadFrame()` |
| `GpioGet()` | The DRDY pin wait (`Sampling::ready = ReadySource::Pin`) and `ReadAlarmPin()` | The timed wait; `ReadAlarmPin()` returns `Unsupported` |
| `CaptureLanes()` | A data frame on 2, 4 or 8 lanes | `ReadFrame()` returns `Unsupported` for those lanes; one lane on SDOUT needs no hook |
| `Log()` | One line per lifecycle event or failure | Nothing is logged |

`CaptureLanes(lanes, rx, len)` clocks `len` bytes of the frame and packs the bits most significant first, so that
every lane count returns the byte stream the 1-lane SDOUT mode would (`lanes` is 1, 2, 4 or 8). With 8 lanes each
clock yields one byte with D7 as the most significant bit; with 4 lanes two clocks yield a byte (D7 to D4, the high
nibble first); with 2 lanes four clocks (D7 and D6 first). This packing follows the data-interface figures of
section 7.3.14 and is **not verified on hardware**; the host tests check it only against the chip model.

A port with every hook:

```cpp
--8<-- "tests/docs/ports.cpp:full-port"
```

```cpp
--8<-- "tests/docs/ports.cpp:hardware-reset"
```

A `GpioSet()` that returns `void`, a `GpioGet()` that returns `void`, a `CaptureLanes()` with a narrowed parameter
or a `Log()` that returns `int` is a compile error: the driver must see a failed pin write. The
[compile-fail tests](testing.md) check each rule.

## Reference implementations

Every port lives with its example, in `examples/<platform>/`; the driver has none. [Platforms](platforms.md) lists
them and how to add one.

- [`examples/host/host_ads9324_port.hpp`](../examples/host/host_ads9324_port.hpp): any PC, against a simulated
  chip. Start from it for a host build or a test bench; it is also the smallest complete port.
- [`examples/esp32/main/esp32_ads9324_port.hpp`](../examples/esp32/main/esp32_ads9324_port.hpp): ESP-IDF, with
  every hook except `CaptureLanes()` (the data is on one lane). Start from it for any ESP-IDF target.

## Check your port

1. Build it: the ports on this page are compiled by the host CI from [`tests/docs/`](../tests/docs/my_port.hpp),
   with the driver instantiated over them.
2. Call `EnsureInitialized()`: `Ok` proves that writes and reads work and that DEVICE_ID read `0x0004`.
3. Call `RunDataPathSelfTest()`: every channel returns its own test pattern, which proves the data lanes, the
   word format and the channel-to-register mapping of both banks on your wiring.

## Threads

The driver is not thread-safe. Give each driver object one owner task, or guard every call with one mutex. When
several chips share a bus, the port must also serialise the bus. See [Contract](contract.md#threading).

## Editor setup

The clang-format and clang-tidy configuration is in `_config/`, and the repository root has no copy of it. Tools
that take a path use it directly; CI does the same:

| Tool | Setting |
| --- | --- |
| Command line | `clang-format --style=file:_config/.clang-format`, `clang-tidy --config-file=_config/.clang-tidy -p build` |
| VS Code (C/C++ extension) | `"C_Cpp.clang_format_style": "file:${workspaceFolder}/_config/.clang-format"` and `"C_Cpp.codeAnalysis.clangTidy.args": ["--config-file=${workspaceFolder}/_config/.clang-tidy"]` |
| clangd (VS Code clangd, Neovim, Emacs), CLion | These find `.clang-format` and `.clang-tidy` only by name, from the file upwards: copy both to the root once (`cp _config/.clang-format _config/.clang-tidy .`). `.gitignore` lists the copies, so they never reach a commit; copy again after `_config/` changes |

Configure with `-DCMAKE_EXPORT_COMPILE_COMMANDS=ON` (clangd: `--compile-commands-dir=build`) so the editor sees the
same flags as the build.
