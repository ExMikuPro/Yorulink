#include "yorulink_port_example.h"

static YORULINK_HandleTypeDef first_link;
static YORULINK_HandleTypeDef second_link;

int main(void)
{
    int first_context;
    int second_context;
    const yorulink_u8_t incoming[] = {0xAAu, 0x55u};

    if (YORULINK_PortInit((YORULINK_HandleTypeDef *)0,
                          (const YORULINK_CommandTypeDef *)0, 0u,
                          (void *)0) != YORULINK_ERR_INVALID_ARG) {
        return 1;
    }
    if (YORULINK_PortInit(&first_link,
                          (const YORULINK_CommandTypeDef *)0, 1u,
                          (void *)0) != YORULINK_ERR_INVALID_ARG) {
        return 2;
    }
    if (YORULINK_PortInit(&first_link,
                          (const YORULINK_CommandTypeDef *)0, 0u,
                          &first_context) != YORULINK_ERR_OK ||
        YORULINK_PortInit(&second_link,
                          (const YORULINK_CommandTypeDef *)0, 0u,
                          &second_context) != YORULINK_ERR_OK) {
        return 3;
    }
    if (first_link.WriteContext != &first_context ||
        second_link.WriteContext != &second_context ||
        first_link.Write == (YORULINK_WriteFnTypeDef)0) {
        return 4;
    }
#if YORULINK_PORT_ENABLE_TICK
    if (first_link.Tick == (YORULINK_TickFnTypeDef)0 ||
        first_link.TickContext != &first_context) {
        return 5;
    }
#else
    if (first_link.Tick != (YORULINK_TickFnTypeDef)0) {
        return 5;
    }
#endif
    if (YORULINK_SendEvent(&first_link, 1u, 1u,
                           (const yorulink_u8_t *)0, 0u)
        != YORULINK_ERR_NO_TRANSPORT) {
        return 6;
    }
    if (YORULINK_Input(&first_link, incoming, sizeof(incoming))
        != sizeof(incoming)) {
        return 7;
    }
    YORULINK_Process(&first_link);
    if (YORULINK_GetStats(&first_link) == (const YORULINK_StatsTypeDef *)0) {
        return 8;
    }
    return 0;
}
