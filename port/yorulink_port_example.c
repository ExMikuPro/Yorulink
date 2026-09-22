/* Optional copy-and-edit Port. This may be the application's sole
 * YORULINK_IMPLEMENTATION translation unit; remove the define if another
 * translation unit already emits the Core implementation. */
#define YORULINK_IMPLEMENTATION
#include "yorulink_port_example.h"

/* =========================================================
 * USER PORT: WRITE
 * =========================================================
 * Send exactly length bytes. The data pointer refers to Yorulink's
 * handle-owned TxBuffer and is valid only until this callback returns.
 * Blocking transports may transmit directly. For DMA, USB, or another
 * asynchronous driver, copy the whole frame into a persistent,
 * application-owned buffer or queue before returning. Release that
 * storage only after actual transmission completes.
 *
 * Platform directions only: STM32 HAL, TI MSPM0 DriverLib, Puya, WCH,
 * an 8051 UART, or a custom BSP can each provide the transport write.
 */
static YORULINK_ERROR_TYPE yorulink_port_write(
    void *context,
    const yorulink_u8_t *data,
    yorulink_u16_t length)
{
    (void)context;
    (void)data;
    (void)length;

    /* USER CODE BEGIN WRITE
     * Replace this body with a platform write. Return YORULINK_ERR_OK
     * only after the complete frame has been sent or safely accepted
     * into application-owned asynchronous storage.
     * USER CODE END WRITE */
    return YORULINK_ERR_NO_TRANSPORT;
}

#if YORULINK_PORT_ENABLE_TICK
/* Replace the stub when locally initiated Requests use an effective
 * nonzero timeout. Return real, monotonic milliseconds; natural unsigned
 * wraparound is allowed. Possible sources include HAL_GetTick(),
 * app_millis(), an RTOS tick converted to ms, or a hardware timer. */
static yorulink_u32_t yorulink_port_tick(void *context)
{
    (void)context;

    /* USER CODE BEGIN TICK */
    return 0u;
    /* USER CODE END TICK */
}
#endif

YORULINK_ERROR_TYPE YORULINK_PortInit(
    YORULINK_HandleTypeDef *hlink,
    const YORULINK_CommandTypeDef *commands,
    yorulink_u16_t command_count,
    void *transport_context)
{
    YORULINK_ERROR_TYPE error;

    if (hlink == (YORULINK_HandleTypeDef *)0) {
        return YORULINK_ERR_INVALID_ARG;
    }

    error = YORULINK_Init(hlink, commands, command_count);
    if (error != YORULINK_ERR_OK) {
        return error;
    }

    YORULINK_SetWrite(hlink, yorulink_port_write, transport_context);
#if YORULINK_PORT_ENABLE_TICK
    YORULINK_SetTick(hlink, yorulink_port_tick, transport_context);
#endif
    return YORULINK_ERR_OK;
}
