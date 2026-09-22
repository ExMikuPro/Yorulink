/*
 * Yorulink Port Example
 *
 * Optional copy-and-edit integration template.
 *
 * Yorulink Core does not depend on this file.
 *
 * Copy this file and yorulink_port_example.c into your
 * application, then edit the USER PORT sections.
 */
#ifndef YORULINK_PORT_EXAMPLE_H
#define YORULINK_PORT_EXAMPLE_H

/* =========================================================
 * USER CONFIG
 * =========================================================
 * 64 / 128 are example settings for this template, not Core defaults
 * or requirements for every MCU. Keep all Yorulink configuration
 * identical in every translation unit that includes yorulink.h.
 */
#ifndef YORULINK_MAX_PAYLOAD
#define YORULINK_MAX_PAYLOAD 64u
#endif

#ifndef YORULINK_RX_QUEUE_SIZE
#define YORULINK_RX_QUEUE_SIZE 128u
#endif

/* A Tick is needed only for locally initiated YORULINK_Request() calls
 * whose effective timeout is nonzero. Receiving Requests, replying, and
 * sending Events do not need one. Set this to 0 to omit the Tick hook. */
#ifndef YORULINK_PORT_ENABLE_TICK
#define YORULINK_PORT_ENABLE_TICK 1
#endif

#include "yorulink.h"

#ifdef __cplusplus
extern "C" {
#endif

/* The application owns hlink and the command table for their full lifetime.
 * transport_context is passed unchanged to Write and optional Tick. */
YORULINK_ERROR_TYPE YORULINK_PortInit(
    YORULINK_HandleTypeDef *hlink,
    const YORULINK_CommandTypeDef *commands,
    yorulink_u16_t command_count,
    void *transport_context);

#ifdef __cplusplus
}
#endif

#endif /* YORULINK_PORT_EXAMPLE_H */
