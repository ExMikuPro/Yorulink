# Yorulink Port Example

`port/` contains an optional copy-and-edit integration template.

Yorulink Core does not depend on these files. For minimum integration, only
`yorulink.h` is required. The Port Example is an integration template, not a
required Yorulink ABI layer.

```text
Application     CMD / OP / handlers
      ↓
Port            TX / Tick / transport context
      ↓
Yorulink Core   parser / CRC / transactions / Reader / Writer
```

1. Copy `yorulink.h` and `yorulink_port_example.h/.c` into your application.
   Rename the example files if useful (`yorulink_port.h/.c`, `app_link.c`, or
   `protocol_port.c`); Yorulink does not care about their names. Update the
   include in the `.c` file if you rename the header.
2. Set the `USER CONFIG` values in the Port header, then implement the `USER
   PORT` Write function. Its default returns `YORULINK_ERR_NO_TRANSPORT`, so
   an unedited template cannot report a false successful send.
3. If this endpoint locally calls `YORULINK_Request()` with a nonzero
   **effective** timeout, replace the Tick stub with a real monotonic,
   naturally wrapping millisecond counter. A `timeout_ms` argument of zero
   uses `YORULINK_DEFAULT_TIMEOUT_MS`. If no timed local Requests occur,
   set `YORULINK_PORT_ENABLE_TICK` to `0`; receiving Requests, replying, and
   sending Events do not need Tick.
4. Call `YORULINK_PortInit()`, send received bytes to `YORULINK_Input()`, and
   run `YORULINK_Process()` from the main loop or a protocol task.

The Port `.c` file defines `YORULINK_IMPLEMENTATION` and can be the sole Core
implementation translation unit. If your project already defines it in
another `.c`, remove that define from the Port `.c`. Defining the implementation
in two translation units causes duplicate symbols.

## Application wiring

The application owns each handle and its command table. Define application
CMD/OP values and handlers outside the Port. The following fragments assume
the application defines `APP_COMMAND`, `APP_OPERATION`, `app_handler`, and
`transport_context`. Keep the table alive for as long as the handle uses it.
An endpoint without command handlers may pass a null table and zero count.

```c
static YORULINK_HandleTypeDef hLink;
static const YORULINK_CommandTypeDef commands[] = {
    { APP_COMMAND, APP_OPERATION, 0u, 0u, app_handler }
};

/* Inside application initialization, using its own command table: */
YORULINK_ERROR_TYPE error = YORULINK_PortInit(
    &hLink,
    commands,
    (yorulink_u16_t)(sizeof(commands) / sizeof(commands[0])),
    transport_context);
/* Check error before using hLink. */

/* UART ISR/callback, USB CDC callback, SPI receive block, or socket recv: */
yorulink_size_t accepted = YORULINK_Input(&hLink, rx_data, rx_length);
/* Handle accepted < rx_length as an RX queue overflow. */

/* In the main loop or protocol task: */
while (1) {
    YORULINK_Process(&hLink);
}
```

Pass the actual number of received bytes. There is no need to assemble a
complete frame, search for `AA55`, calculate CRC, or wait for a fixed packet.
By default, do not call `YORULINK_Process()` in a UART/USB ISR: application
handlers and response callbacks then run in ISR context. Application code uses
`YORULINK_SendEvent()`, `YORULINK_Request()`, and `YORULINK_GetStats()` directly
with `&hLink`; the Port adds no wrappers for those Core APIs. Multiple handles
can each be initialized with a different transport context.

## TX ownership and configuration

Write receives a complete frame, but `data` points into the handle's TX buffer
and is valid only until Write returns. A blocking transport can send from it
directly. DMA, USB, and other asynchronous transports must copy the frame into
persistent application-owned storage or a queue before returning. Free or
reuse that storage only after actual transmission completes.

The template's payload/RX settings of `64 / 128` are examples; the Core defaults
remain `256 / 512`. Every translation unit that includes Yorulink must see
identical configuration if it affects types or ABI, including
`YORULINK_MAX_PAYLOAD`, `YORULINK_RX_QUEUE_SIZE`, `YORULINK_ERROR_TYPE`, and
`YORULINK_ERR_*` overrides. Include the Port header consistently, or put the
configuration in a shared project header included before `yorulink.h`.
