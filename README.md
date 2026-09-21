# Yorulink

[中文说明](README.zh.md)

Yorulink is a lightweight, hardware-independent binary communication library
for MCU-to-MCU byte streams. It is distributed as one header, uses fixed memory,
and is designed for bare-metal C99 projects without a hosted C runtime.

It is a good fit for UART, USB CDC, SPI bridges, host serial links, and other
reliable or semi-reliable byte streams where predictable memory usage matters.

---

> Yoru Series
>
> Related lightweight utility libraries for embedded projects.
>
> | Library | Role |
> |---|---|
> | [Yorulog](https://github.com/ExMikuPro/Yorulog) | Lightweight UART logger |
> | [Yorush](https://github.com/ExMikuPro/Yorush) | Lightweight UART shell / command parser |
> | [Yorulink](https://github.com/ExMikuPro/Yorulink) | Lightweight MCU-to-MCU binary communication library |
> | [Yorunvm](https://github.com/ExMikuPro/Yorunvm) | STM32 on-chip NVM / Flash / EEPROM access helper |
> | [Yorukv](https://github.com/ExMikuPro/Yorukv) | Lightweight KV configuration library |
> | [Yorubench](https://github.com/ExMikuPro/Yorubench) | Lightweight performance measurement library |
> | [Yoruassert](https://github.com/ExMikuPro/Yoruassert) | Lightweight assertion helper |

## Features

- Single-header integration: copy only `yorulink.h`
- C99 and freestanding-friendly
- No `malloc`, `printf`, or required libc memory/string functions
- Fixed-size RX queue, parser storage, payload storage, and TX frame buffer
- REQUEST, RESPONSE, EVENT, and ERROR frames
- CRC-16/CCITT-FALSE
- Streaming parser with arbitrary chunk boundaries and resynchronization
- Static CMD+OP dispatch table
- Bounds-checked little-endian Reader/Writer helpers
- One pending request per link with wrap-safe timeout handling
- Project-wide unified error code integration
- Host tests, ASan/UBSan support, and a parser fuzz target

## Quick Start

### 1. Add the header

Copy `yorulink.h` into your project.

### 2. Define the implementation once

In exactly one `.c` file:

```c
#define YORULINK_IMPLEMENTATION
#include "yorulink.h"
```

Every other translation unit only includes it normally:

```c
#include "yorulink.h"
```

### 3. Bind the transport

```c
static YORULINK_ERROR_TYPE link_write(
    void *context,
    const yorulink_u8_t *data,
    yorulink_u16_t length)
{
    /* Send now, or copy into a persistent DMA-owned queue before returning. */
    return YORULINK_ERR_OK;
}

YORULINK_HandleTypeDef link;

void app_init(void)
{
    (void)YORULINK_Init(&link, commands, command_count);
    YORULINK_SetWrite(&link, link_write, transport_context);
}
```

Feed received bytes with `YORULINK_Input()` and call `YORULINK_Process()` from
the main loop or protocol task. `Input` only copies bytes and never invokes an
application handler.

## Unified Error Codes

Yorulink does not require the application to adopt a second error enum. Bind it
directly to the project's existing error system before including the header:

```c
#define YORULINK_ERROR_TYPE            YORU_ErrorTypeDef
#define YORULINK_ERR_OK                YORU_OK
#define YORULINK_ERR_INVALID_ARG       YORU_ERR_INVALID_ARG
#define YORULINK_ERR_BUSY              YORU_ERR_BUSY
#define YORULINK_ERR_TIMEOUT           YORU_ERR_TIMEOUT
#define YORULINK_ERR_BUFFER_FULL       YORU_ERR_RX_OVERFLOW
#define YORULINK_ERR_TRANSPORT         YORU_ERR_TRANSPORT
#define YORULINK_ERR_UNKNOWN_COMMAND   YORU_ERR_UNKNOWN_COMMAND
#define YORULINK_ERR_UNKNOWN_OPERATION YORU_ERR_UNKNOWN_OPERATION
#define YORULINK_ERR_BAD_LENGTH        YORU_ERR_BAD_LENGTH

#define YORULINK_IMPLEMENTATION
#include "yorulink.h"
```

Without overrides, a standalone unsigned 32-bit error type and values are provided.
Local API failures are returned immediately, parser diagnostics stay in Stats,
and remote business errors travel unchanged in an ERROR frame.

## ERROR Wire Width

ERROR payloads begin with a little-endian unified error ID. The default is
32-bit:

```c
#define YORULINK_WIRE_ERROR_BITS 32u
```

Set it to `16u` only when the project error space is guaranteed to fit. Both
peers and every translation unit must use the same value. This setting is not
negotiated at runtime.

## Configuration

Define configuration macros before every inclusion, preferably through global
compiler definitions.

| Macro | Default | Description |
|---|---:|---|
| `YORULINK_MAX_PAYLOAD` | `256` | Maximum local payload bytes |
| `YORULINK_RX_QUEUE_SIZE` | `512` | RX queue capacity in bytes |
| `YORULINK_DEFAULT_TIMEOUT_MS` | `1000` | Timeout used when Request passes zero |
| `YORULINK_WIRE_ERROR_BITS` | `32` | Unified wire error width: 16 or 32 |
| `YORULINK_ERROR_TYPE` | `yorulink_error_t` | Project-wide public error type |
| `YORULINK_ERR_*` | standalone defaults | Project error semantic mappings |

## Transport and DMA Lifetime

The write callback receives a complete frame, but the pointer is valid only
until the callback returns. A DMA adapter must copy the frame into persistent
storage before starting asynchronous transmission.

## Build and Test

```sh
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

Enable sanitizers with `-DYORULINK_ENABLE_SANITIZERS=ON`. Build the parser fuzz
target with `-DYORULINK_BUILD_FUZZER=ON` when the Clang toolchain includes the
libFuzzer runtime.

## Protocol Summary

```text
AA 55 | VER | TYPE | SEQ | CMD | OP | LENGTH_LE | PAYLOAD | CRC16_LE
```

CRC covers `VER` through the end of PAYLOAD. The normative request vector is:

```text
AA 55 01 01 2A 10 02 02 00 05 FF 50 7B
```

## Scope

V0.1 intentionally does not provide encryption, authentication, fragmentation,
automatic retry, dynamic registration, multi-node addressing, or automatic
struct serialization.

## License

Yorulink is released under the [MIT License](LICENSE), consistent with the
other Yoru Series libraries.
