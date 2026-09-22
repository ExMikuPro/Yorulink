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

Porting Yorulink does **not** require editing `yorulink.h`. Provide one TX
callback, pass each received byte block to `YORULINK_Input()`, and call
`YORULINK_Process()` in the main loop or a protocol task. A millisecond Tick
callback is additionally needed only for locally initiated Requests with a
nonzero effective timeout (the default configuration uses one).

```text
copy yorulink.h → define IMPLEMENTATION once → Init → SetWrite
                                               RX → Input → Process
                                               Request? → SetTick
```

## Porting Guide

### The actual porting API

| Where | What to do | Yorulink API | Required? |
|---|---|---|---|
| Exactly one `.c` file | Emit the single-header implementation | `YORULINK_IMPLEMENTATION` | Yes |
| Initialization | Initialize a handle and command table | `YORULINK_Init()` | Yes |
| TX port | Bind the byte writer | `YORULINK_SetWrite()` | Yes |
| RX callback / ISR | Queue received bytes | `YORULINK_Input()` | Yes |
| Main loop / task | Run parsing, dispatch, and timeouts | `YORULINK_Process()` | Yes |
| Millisecond clock | Time locally initiated Requests | `YORULINK_SetTick()` | When Request has an effective timeout |

`YORULINK_Reply()`, `YORULINK_ReplyError()`, `YORULINK_SendEvent()`,
`YORULINK_Request()`, and the Reader/Writer helpers are application/protocol
APIs, not hardware-port hooks.

```text
RX: UART / USB / SPI / TCP → Input → RX queue → Process → parser + CRC
                                                       → CMD + OP table → handler

TX: handler / Request / Event → Yorulink → Write callback → transport
```

The transport only moves bytes; it need not inspect a Yorulink frame. Yorulink
does not know whether those bytes came from UART, USB, SPI, or TCP.
The code blocks below are application-file fragments: put executable statements
inside your initialization, callback, or main-loop functions as appropriate.

### 1. Copy one header and define its implementation once

Copy only `yorulink.h`. There is no required `yorulink.c`, `yorulink_port.c`,
or `yorulink_config.h`. In **exactly one** `.c` file:

```c
#define YORULINK_IMPLEMENTATION
#include "yorulink.h"
```

In all other `.c` files, use only `#include "yorulink.h"`. Defining the
implementation in more than one translation unit causes duplicate symbols.
Configuration and error-type macros must have the same values in every
translation unit; see [Configuration](#configuration).

### 2. Allocate one handle per independent link

```c
static YORULINK_HandleTypeDef hLink;
/* Two independent transports could instead use hLinkMotor and hLinkSensor. */
```

The caller owns this storage for the lifetime of the link. Yorulink has no
internal global singleton. Each handle has its own RX queue, parser, TX buffer,
and one pending Request slot. Do not put a long-lived handle on a function's
temporary stack frame.

### 3. Define application CMD/OP values, handlers, and a static table

CMD identifies a feature group; OP identifies an action within it. The values
below are **application examples**, not reserved Wire Protocol values.

```c
enum { CMD_SYSTEM = 0x01, CMD_LED = 0x10 };
enum { OP_PING = 0x01, OP_LED_SET = 0x01 };

static void on_ping(YORULINK_HandleTypeDef *link,
                    const YORULINK_MessageTypeDef *message)
{
    (void)YORULINK_Reply(link, message, (const yorulink_u8_t *)0, 0u);
}

static void on_led_set(YORULINK_HandleTypeDef *link,
                       const YORULINK_MessageTypeDef *message)
{
    YORULINK_ReaderTypeDef reader;
    yorulink_u8_t state;
    YORULINK_ReaderInit(&reader, message->Data, message->Length);
    if (YORULINK_ReadU8(&reader, &state) == 0u ||
        YORULINK_ReaderRemaining(&reader) != 0u || state > 1u) {
        (void)YORULINK_ReplyError(link, message, YORULINK_ERR_INVALID_ARG);
        return;
    }
    /* Apply state to your application LED here. */
    (void)YORULINK_Reply(link, message, (const yorulink_u8_t *)0, 0u);
}

static const YORULINK_CommandTypeDef commands[] = {
    {CMD_SYSTEM, OP_PING,    0u, 0u, on_ping},
    {CMD_LED,    OP_LED_SET, 1u, 1u, on_led_set}
};
```

Each entry is `{Command, Operation, MinLength, MaxLength, Handler}`. Yorulink
checks the registered payload-length range before calling a handler. For a
Request with an unknown CMD/OP or invalid length, it sends an ERROR frame.
Keep the table alive as long as the handle uses it; `YORULINK_Init()` stores
its pointer. If both ends use a CMD/OP, agree on its meaning and payload layout.

### 4. Initialize the handle

```c
YORULINK_ERROR_TYPE status = YORULINK_Init(
    &hLink, commands,
    (yorulink_u16_t)(sizeof(commands) / sizeof(commands[0])));
if (status != YORULINK_ERR_OK) {
    /* Handle the configuration error in your application. */
}
```

For an endpoint that only initiates requests and has no incoming
Request/Event handlers, `YORULINK_Init(&hLink, NULL, 0u)` is supported. A
receive-only endpoint with handlers still needs a Write callback if it must
send Replies or automatic ERROR frames. Initialization validates table entries
and duplicate CMD/OP pairs; bind callbacks **after** initialization because
reinitialization clears them.

### 5. Implement the TX port and bind it

The Write callback receives a **complete encoded frame**, including SOF,
header, payload, and CRC. It returns `YORULINK_ERROR_TYPE`; the result is
propagated unchanged by send APIs. The transport should send or copy these
bytes, without parsing CMD, OP, or CRC.

For a simple STM32 HAL **blocking UART** port:

```c
/* STM32 HAL example only; Yorulink Core has no HAL dependency. */
static YORULINK_ERROR_TYPE link_write(void *context,
                                      const yorulink_u8_t *data,
                                      yorulink_u16_t length)
{
    UART_HandleTypeDef *huart = (UART_HandleTypeDef *)context;
    if (HAL_UART_Transmit(huart, (uint8_t *)data, length, 100u) != HAL_OK) {
        return YORULINK_ERR_INTERNAL; /* Or a mapped project UART error. */
    }
    return YORULINK_ERR_OK;
}

YORULINK_SetWrite(&hLink, link_write, &huart1);
```

The application must also handle partial/error conditions of its own driver.
The cast above is only for the HAL API, which does not accept a `const` TX
pointer; do not alter `data`.

> **Async TX lifetime warning:** `data` points into the handle's reusable
> `TxBuffer`. It is valid only until the Write callback returns. If DMA or USB
> middleware reads it later, **copy the entire frame to persistent,
> application-owned storage before returning**. Submission is not completion.

```c
/* WRONG: DMA/USB may still read data after this callback returns. */
HAL_UART_Transmit_DMA(&huart1, (uint8_t *)data, length);
/* Likewise wrong when asynchronous: CDC_Transmit_FS((uint8_t *)data, length); */
return YORULINK_ERR_OK;
```

Correct ownership pattern:

```text
Yorulink TxBuffer → copy → persistent TX queue → DMA / USB
                              keep slot until transfer-complete callback
```

```c
/* Sketch: tx_queue_push must copy all length bytes into persistent storage. */
static YORULINK_ERROR_TYPE async_link_write(void *context,
                                            const yorulink_u8_t *data,
                                            yorulink_u16_t length)
{
    (void)context;
    if (!tx_queue_push(data, length)) return YORULINK_ERR_BUFFER_FULL;
    tx_kick();
    return YORULINK_ERR_OK;
}
```

Queue capacity must cover `YORULINK_MAX_FRAME_SIZE`; do not reuse a slot until
the actual DMA/USB completion callback, and define a disconnect/error policy.

### 6. Feed RX bytes without reconstructing frames

Pass **exactly the received byte count** to `YORULINK_Input()` and check the
returned accepted count. It copies bytes into the RX queue; it does not run
handlers. If fewer bytes are accepted, the queue overflow counter increments
and the parser resynchronizes when processed. Size the queue and schedule
`Process()` so this is not normal operation.

```c
yorulink_size_t accepted = YORULINK_Input(&hLink, data, length);
if (accepted != length) { /* Record or recover from RX overload. */ }
```

There is no need to wait for a full frame, search for `AA 55`, calculate CRC,
or join split packets. One callback containing a whole frame and many
callbacks containing `AA`, then `55 01`, then the remaining bytes produce the
same result after processing.

Short STM32 callback sketches (adapt FS/HS names, handles, buffer ownership,
and re-arm calls to your CubeMX project):

```c
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t size)
{
    if (huart == &huart1) {
        (void)YORULINK_Input(&hLink, uart_rx_buffer, (yorulink_size_t)size);
        /* Re-arm UART/DMA using your project's receive API. */
    }
}

static int8_t CDC_Receive_HS(uint8_t *Buf, uint32_t *Len)
{
    (void)YORULINK_Input(&hLink, Buf, (yorulink_size_t)(*Len));
    /* Re-arm the USB OUT endpoint using your project's device handle/API. */
    return USBD_OK;
}
```

These show only the RX-to-Input connection, not complete CubeMX DMA templates.
If a callback can run concurrently with `Process()`, use the documented
single-producer/single-consumer pattern; serialize multiple producers or
concurrent calls to the same handle.

### 7. Run the protocol in the main loop or a task

```c
while (1) {
    YORULINK_Process(&hLink);
    application_process();
}
```

`Input()` queues bytes. `Process()` performs parsing, CRC checks, CMD/OP
dispatch, handler calls, response matching, and timeout callbacks. Call it
often enough to prevent RX queue overflow and service timeouts. By default,
**do not call `Process()` from a UART/USB ISR**: that would run application
handlers and callbacks in interrupt context. With an RTOS, use a dedicated
protocol task and arrange synchronization appropriate to the platform.

### 8. Bind Tick only when initiating timed Requests

```c
static yorulink_u32_t link_tick(void *context)
{
    (void)context;
    return HAL_GetTick(); /* STM32 HAL example; any monotonic ms clock works. */
}
YORULINK_SetTick(&hLink, link_tick, (void *)0);
```

Receiving Requests and sending Replies or Events does **not** need Tick.
`YORULINK_Request()` uses `timeout_ms`, or
`YORULINK_DEFAULT_TIMEOUT_MS` when passed `0`. If the effective timeout is
nonzero and no Tick is bound, it returns `YORULINK_ERR_NO_TICK`. With the
default `1000` ms configuration, every Request needs Tick. The millisecond
counter may wrap; keep calling `Process()` to detect expiry.

### 9. Use the application-level messaging APIs

An unsolicited Event needs no peer request:

```c
enum { CMD_SENSOR = 0x20, OP_SENSOR_UPDATE = 0x01 };
yorulink_u8_t payload[] = {42u};
(void)YORULINK_SendEvent(&hLink, CMD_SENSOR, OP_SENSOR_UPDATE,
                         payload, (yorulink_u16_t)sizeof(payload));
```

To initiate a Request, bind Tick, then provide a completion callback:

```c
static void on_response(YORULINK_HandleTypeDef *link,
                        YORULINK_ERROR_TYPE error,
                        const YORULINK_MessageTypeDef *message,
                        void *context)
{
    (void)link;
    (void)context;
    if (message == (const YORULINK_MessageTypeDef *)0) {
        /* Timeout: error == YORULINK_ERR_TIMEOUT. */
    } else if (message->Type == YORULINK_TYPE_ERROR) {
        /* Remote 32-bit business error is in error. */
    } else if (error == YORULINK_ERR_OK) {
        /* RESPONSE; inspect message->Data and message->Length here. */
    }
}

(void)YORULINK_Request(&hLink, CMD_SYSTEM, OP_PING,
                       (const yorulink_u8_t *)0, 0u,
                       1000u, on_response, (void *)0);
```

Only **one** Request may be pending per handle; another returns
`YORULINK_ERR_BUSY`. The matching RESPONSE supplies `error ==
YORULINK_ERR_OK`; a matching ERROR supplies its remote error ID and a
non-null message; timeout supplies `YORULINK_ERR_TIMEOUT` and a null message.
The callback's `message` and payload are handle-owned and should be copied if
needed after the callback returns. Check the immediate return of `Request()`:
TX failures are reported there, before any completion callback.

Within a handler for a received Request, use `YORULINK_Reply(link, message,
payload, length)` for success or `YORULINK_ReplyError(link, message,
application_error)` for failure. The latter sends the fixed 32-bit error ID
described in [ERROR Wire Format](#error-wire-format). For example:

```c
/* Inside a Request handler with link and message: */
(void)YORULINK_Reply(link, message, (const yorulink_u8_t *)0, 0u);
/* Or, instead of success: */
(void)YORULINK_ReplyError(link, message, YORULINK_ERR_UNSUPPORTED);
```

Use `YORULINK_ReaderInit()` plus `ReadU8/U16/U32/Bytes` to decode a payload;
use `YORULINK_WriterInit()` plus `WriteU8/U16/U32/Bytes` to build one. These
helpers check bounds and encode multi-byte values in little-endian order:

```c
yorulink_u8_t out[3];
YORULINK_WriterTypeDef writer;
YORULINK_WriterInit(&writer, out, (yorulink_u16_t)sizeof(out));
(void)YORULINK_WriteU8(&writer, 1u);
(void)YORULINK_WriteU16(&writer, 0x1234u); /* out = 01 34 12 */
if (YORULINK_WriterError(&writer) == 0u) {
    (void)YORULINK_Reply(link, message, out, YORULINK_WriterLength(&writer));
}
```

### Transport mapping

| Transport | RX entry point | TX implementation |
|---|---|---|
| Blocking UART | RX IRQ / polling | Blocking transmit |
| UART DMA | DMA/IDLE callback | Persistent DMA TX queue |
| USB CDC | CDC Receive callback | Persistent USB TX queue |
| TCP | Socket `recv` | Socket `send` (handle partial writes) |
| SPI bridge | Received byte block | Platform-specific send |

These are integration patterns, **not** claims that all listed transports have
been hardware-tested.

### Minimum porting checklist

- [ ] Copy `yorulink.h`; define `YORULINK_IMPLEMENTATION` in exactly one `.c`.
- [ ] Allocate a persistent `YORULINK_HandleTypeDef` per link.
- [ ] Define application CMD/OP values, handlers, and a persistent command table.
- [ ] Call `YORULINK_Init()` and handle its return value.
- [ ] Implement a TX Write callback and bind it with `YORULINK_SetWrite()`.
- [ ] Forward RX blocks to `YORULINK_Input()` and check accepted counts.
- [ ] Call `YORULINK_Process()` regularly from the main loop or protocol task.
- [ ] If initiating timed Requests, bind `YORULINK_SetTick()`.
- [ ] For asynchronous TX, copy to a persistent buffer/queue and release only on completion.

You do **not** need to modify `yorulink.h`, implement CRC, scan SOF, wait for
full frames, manually reassemble arbitrary chunks, dispatch CMD/OP, maintain
Request sequence numbers, match Responses, handle millisecond-tick wraparound,
use `malloc`, or adopt an RTOS.

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
#define YORULINK_ERR_UNKNOWN_COMMAND   YORU_ERR_UNKNOWN_COMMAND
#define YORULINK_ERR_UNKNOWN_OPERATION YORU_ERR_UNKNOWN_OPERATION
#define YORULINK_ERR_BAD_LENGTH        YORU_ERR_BAD_LENGTH

#define YORULINK_IMPLEMENTATION
#include "yorulink.h"
```

Without overrides, a standalone unsigned 32-bit error type and values are provided.
Local API failures are returned immediately, parser diagnostics stay in Stats,
and remote business errors travel unchanged in an ERROR frame.
The custom mappings above are illustrative; map every error semantic your
application uses and provide identical definitions to every translation unit.

The write callback's return value is propagated unchanged through
`YORULINK_Send()` and related APIs, so driver-specific errors are not collapsed
into a Yorulink-owned transport error.

## ERROR Wire Format

Wire Protocol V1 has one ERROR layout. Every ERROR payload begins with a fixed
unsigned 32-bit unified error ID in little-endian order:

```text
Offset  Size  Field
0       4     unified_error_id, uint32 little-endian
4       N     optional opaque detail (reserved for future use)
```

`YORULINK_ERROR_TYPE` must be at least 32 bits; smaller project error types fail
at compile time. V0.1 sends only the four-byte Error ID.

## Configuration

Define configuration macros before every inclusion, preferably through global
compiler definitions.

| Macro | Default | Description |
|---|---:|---|
| `YORULINK_MAX_PAYLOAD` | `256` | Maximum local payload bytes |
| `YORULINK_RX_QUEUE_SIZE` | `512` | RX queue capacity in bytes |
| `YORULINK_DEFAULT_TIMEOUT_MS` | `1000` | Timeout used when Request passes zero |
| `YORULINK_ERROR_TYPE` | `yorulink_error_t` | Project-wide public error type |
| `YORULINK_ERR_*` | standalone defaults | Project error semantic mappings |

`YORULINK_MAX_PAYLOAD` and `YORULINK_RX_QUEUE_SIZE` directly affect the RAM
used by **each** handle. Configure them consistently in all translation units;
the peers must also agree on the largest payload they exchange. The wire
format and 32-bit ERROR ID are fixed for V1 and are not selected by a macro.

## Transport and DMA Lifetime

See the [TX port step](#5-implement-the-tx-port-and-bind-it) for the full
ownership rule and incorrect/correct DMA and USB examples. In short: the Write
callback's frame pointer expires when the callback returns.

## Resource Footprint

One measured STM32H743 Cortex-M7 Release (`-Os`) build with
`YORULINK_MAX_PAYLOAD=256` and `YORULINK_RX_QUEUE_SIZE=512` linked about
**2.2 KiB of Yorulink core code** and allocated **1,152 B** for one
`YORULINK_HandleTypeDef`. These are measurements of that build, not fixed
requirements: compiler, optimization, configuration, and which APIs are linked
change the result. The separate USB CDC TX queue (2,160 B in that test app)
is **not** part of Yorulink Core. No dynamic allocation is used.

## Validated Hardware

The integration was exercised on an **STM32H743 / Cortex-M7** using a USB CDC
transport. A 100,000-Request/Response hardware stress run completed with zero
unexpected ERROR frames, timeouts, host CRC mismatches, or payload mismatches.
Reset, USB re-enumeration, and post-reset PING/ECHO also passed. A subsequent
RGB GPIO command cycle was verified on the board. This is a validation record,
not a claim of hardware testing on every STM32 or every transport.

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
