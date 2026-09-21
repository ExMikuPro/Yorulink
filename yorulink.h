#ifndef YORULINK_H
#define YORULINK_H

#ifdef __cplusplus
extern "C" {
#endif

/* Configuration. Values affecting structure layout must match in every TU. */
#ifndef YORULINK_MAX_PAYLOAD
#define YORULINK_MAX_PAYLOAD 256u
#endif

#ifndef YORULINK_RX_QUEUE_SIZE
#define YORULINK_RX_QUEUE_SIZE 512u
#endif

#ifndef YORULINK_DEFAULT_TIMEOUT_MS
#define YORULINK_DEFAULT_TIMEOUT_MS 1000u
#endif

#ifndef YORULINK_WIRE_ERROR_BITS
#define YORULINK_WIRE_ERROR_BITS 32u
#endif

#if YORULINK_MAX_PAYLOAD > 65524u
#error "YORULINK_MAX_PAYLOAD must leave room for the 11-byte frame overhead"
#endif

#if YORULINK_RX_QUEUE_SIZE < 1u
#error "YORULINK_RX_QUEUE_SIZE must be at least 1"
#endif

#if (YORULINK_WIRE_ERROR_BITS != 16u) && (YORULINK_WIRE_ERROR_BITS != 32u)
#error "YORULINK_WIRE_ERROR_BITS must be 16 or 32"
#endif

#if defined(__CHAR_BIT__) && (__CHAR_BIT__ != 8)
#error "Yorulink Wire V1 requires 8-bit bytes"
#endif

typedef
#if defined(__UINT8_TYPE__)
__UINT8_TYPE__
#else
unsigned char
#endif
yorulink_u8_t;

typedef
#if defined(__INT8_TYPE__)
__INT8_TYPE__
#else
signed char
#endif
yorulink_i8_t;

typedef
#if defined(__UINT16_TYPE__)
__UINT16_TYPE__
#else
unsigned short
#endif
yorulink_u16_t;

typedef
#if defined(__INT16_TYPE__)
__INT16_TYPE__
#else
signed short
#endif
yorulink_i16_t;

typedef
#if defined(__UINT32_TYPE__)
__UINT32_TYPE__
#else
unsigned long
#endif
yorulink_u32_t;

typedef
#if defined(__INT32_TYPE__)
__INT32_TYPE__
#else
signed long
#endif
yorulink_i32_t;

typedef
#if defined(__SIZE_TYPE__)
__SIZE_TYPE__
#else
unsigned long
#endif
yorulink_size_t;

typedef char yorulink__u8_size_check[(sizeof(yorulink_u8_t) == 1u) ? 1 : -1];
typedef char yorulink__u16_size_check[(sizeof(yorulink_u16_t) == 2u) ? 1 : -1];
typedef char yorulink__u32_size_check[(sizeof(yorulink_u32_t) == 4u) ? 1 : -1];

/*
 * Override YORULINK_ERROR_TYPE and the YORULINK_ERR_* macros before including
 * this file to bind Yorulink directly to a project-wide error system.
 */
#ifndef YORULINK_ERROR_TYPE
typedef yorulink_u32_t yorulink_error_t;
#define YORULINK_ERROR_TYPE yorulink_error_t
#endif

#ifndef YORULINK_ERR_OK
#define YORULINK_ERR_OK ((YORULINK_ERROR_TYPE)0)
#endif
#ifndef YORULINK_ERR_INVALID_ARG
#define YORULINK_ERR_INVALID_ARG ((YORULINK_ERROR_TYPE)1)
#endif
#ifndef YORULINK_ERR_BAD_CONFIG
#define YORULINK_ERR_BAD_CONFIG ((YORULINK_ERROR_TYPE)2)
#endif
#ifndef YORULINK_ERR_NO_TRANSPORT
#define YORULINK_ERR_NO_TRANSPORT ((YORULINK_ERROR_TYPE)3)
#endif
#ifndef YORULINK_ERR_NO_TICK
#define YORULINK_ERR_NO_TICK ((YORULINK_ERROR_TYPE)4)
#endif
#ifndef YORULINK_ERR_BUFFER_FULL
#define YORULINK_ERR_BUFFER_FULL ((YORULINK_ERROR_TYPE)5)
#endif
#ifndef YORULINK_ERR_PAYLOAD_TOO_LARGE
#define YORULINK_ERR_PAYLOAD_TOO_LARGE ((YORULINK_ERROR_TYPE)6)
#endif
#ifndef YORULINK_ERR_BUSY
#define YORULINK_ERR_BUSY ((YORULINK_ERROR_TYPE)7)
#endif
#ifndef YORULINK_ERR_TRANSPORT
#define YORULINK_ERR_TRANSPORT ((YORULINK_ERROR_TYPE)8)
#endif
#ifndef YORULINK_ERR_TIMEOUT
#define YORULINK_ERR_TIMEOUT ((YORULINK_ERROR_TYPE)9)
#endif
#ifndef YORULINK_ERR_MALFORMED
#define YORULINK_ERR_MALFORMED ((YORULINK_ERROR_TYPE)10)
#endif
#ifndef YORULINK_ERR_UNKNOWN_COMMAND
#define YORULINK_ERR_UNKNOWN_COMMAND ((YORULINK_ERROR_TYPE)11)
#endif
#ifndef YORULINK_ERR_UNKNOWN_OPERATION
#define YORULINK_ERR_UNKNOWN_OPERATION ((YORULINK_ERROR_TYPE)12)
#endif
#ifndef YORULINK_ERR_BAD_LENGTH
#define YORULINK_ERR_BAD_LENGTH ((YORULINK_ERROR_TYPE)13)
#endif
#ifndef YORULINK_ERR_UNSUPPORTED
#define YORULINK_ERR_UNSUPPORTED ((YORULINK_ERROR_TYPE)14)
#endif
#ifndef YORULINK_ERR_INTERNAL
#define YORULINK_ERR_INTERNAL ((YORULINK_ERROR_TYPE)15)
#endif

#define YORULINK_WIRE_ERROR_SIZE (YORULINK_WIRE_ERROR_BITS / 8u)
typedef char yorulink__error_type_width_check[
    (sizeof(YORULINK_ERROR_TYPE) * 8u >= YORULINK_WIRE_ERROR_BITS) ? 1 : -1
];

#define YORULINK_PROTOCOL_VERSION 0x01u
#define YORULINK_SOF0 0xAAu
#define YORULINK_SOF1 0x55u
#define YORULINK_FRAME_OVERHEAD 11u
#define YORULINK_MAX_FRAME_SIZE (YORULINK_FRAME_OVERHEAD + YORULINK_MAX_PAYLOAD)

typedef enum {
    YORULINK_TYPE_REQUEST = 0x01,
    YORULINK_TYPE_RESPONSE = 0x02,
    YORULINK_TYPE_EVENT = 0x03,
    YORULINK_TYPE_ERROR = 0x04
} YORULINK_MessageType;

typedef struct YORULINK_HandleTypeDef YORULINK_HandleTypeDef;

typedef struct {
    yorulink_u8_t Version;
    yorulink_u8_t Type;
    yorulink_u8_t Sequence;
    yorulink_u8_t Command;
    yorulink_u8_t Operation;
    yorulink_u16_t Length;
    const yorulink_u8_t *Data;
} YORULINK_MessageTypeDef;

typedef void (*YORULINK_CommandHandlerTypeDef)(
    YORULINK_HandleTypeDef *hlink,
    const YORULINK_MessageTypeDef *message);

typedef struct {
    yorulink_u8_t Command;
    yorulink_u8_t Operation;
    yorulink_u16_t MinLength;
    yorulink_u16_t MaxLength;
    YORULINK_CommandHandlerTypeDef Handler;
} YORULINK_CommandTypeDef;

typedef YORULINK_ERROR_TYPE (*YORULINK_WriteFnTypeDef)(
    void *context, const yorulink_u8_t *data, yorulink_u16_t length);

typedef yorulink_u32_t (*YORULINK_TickFnTypeDef)(void *context);

typedef void (*YORULINK_ResponseHandlerTypeDef)(
    YORULINK_HandleTypeDef *hlink,
    YORULINK_ERROR_TYPE error,
    const YORULINK_MessageTypeDef *message,
    void *context);

typedef struct YORULINK_ReaderTypeDef {
    const yorulink_u8_t *Data;
    yorulink_u16_t Length;
    yorulink_u16_t Offset;
    yorulink_u8_t Error;
} YORULINK_ReaderTypeDef;

typedef struct YORULINK_WriterTypeDef {
    yorulink_u8_t *Data;
    yorulink_u16_t Capacity;
    yorulink_u16_t Offset;
    yorulink_u8_t Error;
} YORULINK_WriterTypeDef;

typedef struct YORULINK_StatsTypeDef {
    yorulink_u32_t FramesReceived;
    yorulink_u32_t CrcErrors;
    yorulink_u32_t VersionErrors;
    yorulink_u32_t TypeErrors;
    yorulink_u32_t LengthErrors;
    yorulink_u32_t SemanticErrors;
    yorulink_u32_t RxOverflows;
    yorulink_u32_t UnmatchedResponses;
    yorulink_u32_t RequestTimeouts;
} YORULINK_StatsTypeDef;

typedef enum {
    YORULINK_PARSE_WAIT_SOF0 = 0,
    YORULINK_PARSE_WAIT_SOF1,
    YORULINK_PARSE_VERSION,
    YORULINK_PARSE_TYPE,
    YORULINK_PARSE_SEQUENCE,
    YORULINK_PARSE_COMMAND,
    YORULINK_PARSE_OPERATION,
    YORULINK_PARSE_LENGTH_LOW,
    YORULINK_PARSE_LENGTH_HIGH,
    YORULINK_PARSE_PAYLOAD,
    YORULINK_PARSE_CRC_LOW,
    YORULINK_PARSE_CRC_HIGH
} YORULINK_ParserStateTypeDef;

struct YORULINK_HandleTypeDef {
    const YORULINK_CommandTypeDef *CommandTable;
    yorulink_u16_t CommandCount;
    YORULINK_WriteFnTypeDef Write;
    void *WriteContext;
    YORULINK_TickFnTypeDef Tick;
    void *TickContext;

    yorulink_u8_t RxQueue[YORULINK_RX_QUEUE_SIZE + 1u];
    volatile yorulink_size_t RxHead;
    volatile yorulink_size_t RxTail;
    volatile yorulink_u8_t ParserNeedsResync;

    YORULINK_ParserStateTypeDef ParserState;
    YORULINK_MessageTypeDef ParsedMessage;
    yorulink_u8_t Payload[YORULINK_MAX_PAYLOAD > 0u ? YORULINK_MAX_PAYLOAD : 1u];
    yorulink_u16_t PayloadOffset;
    yorulink_u16_t CalculatedCrc;
    yorulink_u16_t ReceivedCrc;

    yorulink_u8_t TxBuffer[YORULINK_MAX_FRAME_SIZE];
    yorulink_u8_t NextSequence;
    yorulink_u8_t PendingActive;
    yorulink_u8_t PendingSequence;
    yorulink_u8_t PendingCommand;
    yorulink_u8_t PendingOperation;
    yorulink_u32_t PendingStartTick;
    yorulink_u32_t PendingTimeout;
    YORULINK_ResponseHandlerTypeDef PendingCallback;
    void *PendingContext;

    YORULINK_StatsTypeDef Stats;
};

yorulink_u16_t YORULINK_Crc16(const yorulink_u8_t *data, yorulink_u16_t length);
YORULINK_ERROR_TYPE YORULINK_Init(YORULINK_HandleTypeDef *hlink,
                                  const YORULINK_CommandTypeDef *table,
                                  yorulink_u16_t count);
void YORULINK_SetWrite(YORULINK_HandleTypeDef *hlink, YORULINK_WriteFnTypeDef fn, void *context);
void YORULINK_SetTick(YORULINK_HandleTypeDef *hlink, YORULINK_TickFnTypeDef fn, void *context);
yorulink_size_t YORULINK_Input(YORULINK_HandleTypeDef *hlink,
                               const yorulink_u8_t *data,
                               yorulink_size_t length);
void YORULINK_Process(YORULINK_HandleTypeDef *hlink);
YORULINK_ERROR_TYPE YORULINK_Send(YORULINK_HandleTypeDef *hlink,
                                  yorulink_u8_t type,
                                  yorulink_u8_t sequence,
                                  yorulink_u8_t command,
                                  yorulink_u8_t operation,
                                  const yorulink_u8_t *payload,
                                  yorulink_u16_t length);
YORULINK_ERROR_TYPE YORULINK_SendEvent(YORULINK_HandleTypeDef *hlink,
                                       yorulink_u8_t command,
                                       yorulink_u8_t operation,
                                       const yorulink_u8_t *payload,
                                       yorulink_u16_t length);
YORULINK_ERROR_TYPE YORULINK_Reply(YORULINK_HandleTypeDef *hlink,
                                   const YORULINK_MessageTypeDef *request,
                                   const yorulink_u8_t *payload,
                                   yorulink_u16_t length);
YORULINK_ERROR_TYPE YORULINK_ReplyError(YORULINK_HandleTypeDef *hlink,
                                        const YORULINK_MessageTypeDef *request,
                                        YORULINK_ERROR_TYPE error);
YORULINK_ERROR_TYPE YORULINK_GetRemoteError(const YORULINK_MessageTypeDef *message,
                                            YORULINK_ERROR_TYPE *error);
YORULINK_ERROR_TYPE YORULINK_Request(YORULINK_HandleTypeDef *hlink,
                                     yorulink_u8_t command,
                                     yorulink_u8_t operation,
                                     const yorulink_u8_t *payload,
                                     yorulink_u16_t length,
                                     yorulink_u32_t timeout_ms,
                                     YORULINK_ResponseHandlerTypeDef callback,
                                     void *context);

void YORULINK_ReaderInit(YORULINK_ReaderTypeDef *reader,
                         const yorulink_u8_t *data,
                         yorulink_u16_t length);
yorulink_u8_t YORULINK_ReadU8(YORULINK_ReaderTypeDef *reader, yorulink_u8_t *value);
yorulink_u8_t YORULINK_ReadI8(YORULINK_ReaderTypeDef *reader, yorulink_i8_t *value);
yorulink_u8_t YORULINK_ReadU16(YORULINK_ReaderTypeDef *reader, yorulink_u16_t *value);
yorulink_u8_t YORULINK_ReadI16(YORULINK_ReaderTypeDef *reader, yorulink_i16_t *value);
yorulink_u8_t YORULINK_ReadU32(YORULINK_ReaderTypeDef *reader, yorulink_u32_t *value);
yorulink_u8_t YORULINK_ReadI32(YORULINK_ReaderTypeDef *reader, yorulink_i32_t *value);
yorulink_u8_t YORULINK_ReadBytes(YORULINK_ReaderTypeDef *reader,
                                 yorulink_u8_t *value,
                                 yorulink_u16_t length);
yorulink_u16_t YORULINK_ReaderRemaining(const YORULINK_ReaderTypeDef *reader);
yorulink_u8_t YORULINK_ReaderError(const YORULINK_ReaderTypeDef *reader);

void YORULINK_WriterInit(YORULINK_WriterTypeDef *writer,
                         yorulink_u8_t *data,
                         yorulink_u16_t capacity);
yorulink_u8_t YORULINK_WriteU8(YORULINK_WriterTypeDef *writer, yorulink_u8_t value);
yorulink_u8_t YORULINK_WriteI8(YORULINK_WriterTypeDef *writer, yorulink_i8_t value);
yorulink_u8_t YORULINK_WriteU16(YORULINK_WriterTypeDef *writer, yorulink_u16_t value);
yorulink_u8_t YORULINK_WriteI16(YORULINK_WriterTypeDef *writer, yorulink_i16_t value);
yorulink_u8_t YORULINK_WriteU32(YORULINK_WriterTypeDef *writer, yorulink_u32_t value);
yorulink_u8_t YORULINK_WriteI32(YORULINK_WriterTypeDef *writer, yorulink_i32_t value);
yorulink_u8_t YORULINK_WriteBytes(YORULINK_WriterTypeDef *writer,
                                  const yorulink_u8_t *value,
                                  yorulink_u16_t length);
yorulink_u16_t YORULINK_WriterLength(const YORULINK_WriterTypeDef *writer);
yorulink_u16_t YORULINK_WriterRemaining(const YORULINK_WriterTypeDef *writer);
yorulink_u8_t YORULINK_WriterError(const YORULINK_WriterTypeDef *writer);

const YORULINK_StatsTypeDef *YORULINK_GetStats(const YORULINK_HandleTypeDef *hlink);
void YORULINK_ResetStats(YORULINK_HandleTypeDef *hlink);

#ifdef __cplusplus
}
#endif

#endif /* YORULINK_H */

#ifdef YORULINK_IMPLEMENTATION
#ifndef YORULINK_IMPLEMENTATION_ONCE
#define YORULINK_IMPLEMENTATION_ONCE

static void yorulink__copy_(yorulink_u8_t *dst,
                            const yorulink_u8_t *src,
                            yorulink_size_t length)
{
    while (length != 0u) {
        *dst++ = *src++;
        --length;
    }
}

static yorulink_u16_t yorulink__crc_byte_(yorulink_u16_t crc, yorulink_u8_t value)
{
    yorulink_u8_t bit;
    crc ^= (yorulink_u16_t)((yorulink_u16_t)value << 8u);
    for (bit = 0u; bit < 8u; ++bit) {
        crc = (crc & 0x8000u) != 0u
                  ? (yorulink_u16_t)((crc << 1u) ^ 0x1021u)
                  : (yorulink_u16_t)(crc << 1u);
    }
    return crc;
}

yorulink_u16_t YORULINK_Crc16(const yorulink_u8_t *data, yorulink_u16_t length)
{
    yorulink_u16_t crc = 0xFFFFu;
    yorulink_u16_t index;
    if (data == (const yorulink_u8_t *)0 && length != 0u) {
        return crc;
    }
    for (index = 0u; index < length; ++index) {
        crc = yorulink__crc_byte_(crc, data[index]);
    }
    return crc;
}

static void yorulink__parser_reset_(YORULINK_HandleTypeDef *hlink)
{
    hlink->ParserState = YORULINK_PARSE_WAIT_SOF0;
    hlink->PayloadOffset = 0u;
    hlink->CalculatedCrc = 0xFFFFu;
    hlink->ReceivedCrc = 0u;
}

void YORULINK_ResetStats(YORULINK_HandleTypeDef *hlink)
{
    if (hlink != (YORULINK_HandleTypeDef *)0) {
        hlink->Stats.FramesReceived = 0u;
        hlink->Stats.CrcErrors = 0u;
        hlink->Stats.VersionErrors = 0u;
        hlink->Stats.TypeErrors = 0u;
        hlink->Stats.LengthErrors = 0u;
        hlink->Stats.SemanticErrors = 0u;
        hlink->Stats.RxOverflows = 0u;
        hlink->Stats.UnmatchedResponses = 0u;
        hlink->Stats.RequestTimeouts = 0u;
    }
}

const YORULINK_StatsTypeDef *YORULINK_GetStats(const YORULINK_HandleTypeDef *hlink)
{
    return hlink == (const YORULINK_HandleTypeDef *)0
               ? (const YORULINK_StatsTypeDef *)0
               : &hlink->Stats;
}

YORULINK_ERROR_TYPE YORULINK_Init(YORULINK_HandleTypeDef *hlink,
                                  const YORULINK_CommandTypeDef *table,
                                  yorulink_u16_t count)
{
    yorulink_u16_t outer;
    yorulink_u16_t inner;
    if (hlink == (YORULINK_HandleTypeDef *)0 ||
        (table == (const YORULINK_CommandTypeDef *)0 && count != 0u)) {
        return YORULINK_ERR_INVALID_ARG;
    }
    for (outer = 0u; outer < count; ++outer) {
        if (table[outer].Handler == (YORULINK_CommandHandlerTypeDef)0 ||
            table[outer].MinLength > table[outer].MaxLength ||
            table[outer].MaxLength > YORULINK_MAX_PAYLOAD) {
            return YORULINK_ERR_BAD_CONFIG;
        }
        for (inner = 0u; inner < outer; ++inner) {
            if (table[inner].Command == table[outer].Command &&
                table[inner].Operation == table[outer].Operation) {
                return YORULINK_ERR_BAD_CONFIG;
            }
        }
    }

    hlink->CommandTable = table;
    hlink->CommandCount = count;
    hlink->Write = (YORULINK_WriteFnTypeDef)0;
    hlink->WriteContext = (void *)0;
    hlink->Tick = (YORULINK_TickFnTypeDef)0;
    hlink->TickContext = (void *)0;
    hlink->RxHead = 0u;
    hlink->RxTail = 0u;
    hlink->ParserNeedsResync = 0u;
    yorulink__parser_reset_(hlink);
    hlink->ParsedMessage.Version = 0u;
    hlink->ParsedMessage.Type = 0u;
    hlink->ParsedMessage.Sequence = 0u;
    hlink->ParsedMessage.Command = 0u;
    hlink->ParsedMessage.Operation = 0u;
    hlink->ParsedMessage.Length = 0u;
    hlink->ParsedMessage.Data = hlink->Payload;
    hlink->NextSequence = 1u;
    hlink->PendingActive = 0u;
    hlink->PendingSequence = 0u;
    hlink->PendingCommand = 0u;
    hlink->PendingOperation = 0u;
    hlink->PendingStartTick = 0u;
    hlink->PendingTimeout = 0u;
    hlink->PendingCallback = (YORULINK_ResponseHandlerTypeDef)0;
    hlink->PendingContext = (void *)0;
    YORULINK_ResetStats(hlink);
    return YORULINK_ERR_OK;
}

void YORULINK_SetWrite(YORULINK_HandleTypeDef *hlink, YORULINK_WriteFnTypeDef fn, void *context)
{
    if (hlink != (YORULINK_HandleTypeDef *)0) {
        hlink->Write = fn;
        hlink->WriteContext = context;
    }
}

void YORULINK_SetTick(YORULINK_HandleTypeDef *hlink, YORULINK_TickFnTypeDef fn, void *context)
{
    if (hlink != (YORULINK_HandleTypeDef *)0) {
        hlink->Tick = fn;
        hlink->TickContext = context;
    }
}

YORULINK_ERROR_TYPE YORULINK_Send(YORULINK_HandleTypeDef *hlink,
                                  yorulink_u8_t type,
                                  yorulink_u8_t sequence,
                                  yorulink_u8_t command,
                                  yorulink_u8_t operation,
                                  const yorulink_u8_t *payload,
                                  yorulink_u16_t length)
{
    yorulink_u16_t crc;
    yorulink_u16_t frame_length;
    if (hlink == (YORULINK_HandleTypeDef *)0) {
        return YORULINK_ERR_INVALID_ARG;
    }
    if (length > YORULINK_MAX_PAYLOAD) {
        return YORULINK_ERR_PAYLOAD_TOO_LARGE;
    }
    if (payload == (const yorulink_u8_t *)0 && length != 0u) {
        return YORULINK_ERR_INVALID_ARG;
    }
    if (type < YORULINK_TYPE_REQUEST || type > YORULINK_TYPE_ERROR ||
        (type == YORULINK_TYPE_EVENT ? sequence != 0u : sequence == 0u) ||
        (type == YORULINK_TYPE_ERROR && length < YORULINK_WIRE_ERROR_SIZE)) {
        return YORULINK_ERR_INVALID_ARG;
    }
    if (hlink->Write == (YORULINK_WriteFnTypeDef)0) {
        return YORULINK_ERR_NO_TRANSPORT;
    }

    hlink->TxBuffer[0] = YORULINK_SOF0;
    hlink->TxBuffer[1] = YORULINK_SOF1;
    hlink->TxBuffer[2] = YORULINK_PROTOCOL_VERSION;
    hlink->TxBuffer[3] = type;
    hlink->TxBuffer[4] = sequence;
    hlink->TxBuffer[5] = command;
    hlink->TxBuffer[6] = operation;
    hlink->TxBuffer[7] = (yorulink_u8_t)(length & 0xFFu);
    hlink->TxBuffer[8] = (yorulink_u8_t)(length >> 8u);
    if (length != 0u) {
        yorulink__copy_(&hlink->TxBuffer[9], payload, length);
    }
    crc = YORULINK_Crc16(&hlink->TxBuffer[2], (yorulink_u16_t)(7u + length));
    hlink->TxBuffer[9u + length] = (yorulink_u8_t)(crc & 0xFFu);
    hlink->TxBuffer[10u + length] = (yorulink_u8_t)(crc >> 8u);
    frame_length = (yorulink_u16_t)(YORULINK_FRAME_OVERHEAD + length);
    return hlink->Write(hlink->WriteContext, hlink->TxBuffer, frame_length) == YORULINK_ERR_OK
               ? YORULINK_ERR_OK
               : YORULINK_ERR_TRANSPORT;
}

YORULINK_ERROR_TYPE YORULINK_SendEvent(YORULINK_HandleTypeDef *hlink,
                                       yorulink_u8_t command,
                                       yorulink_u8_t operation,
                                       const yorulink_u8_t *payload,
                                       yorulink_u16_t length)
{
    return YORULINK_Send(hlink, YORULINK_TYPE_EVENT, 0u, command, operation, payload, length);
}

YORULINK_ERROR_TYPE YORULINK_Reply(YORULINK_HandleTypeDef *hlink,
                                   const YORULINK_MessageTypeDef *request,
                                   const yorulink_u8_t *payload,
                                   yorulink_u16_t length)
{
    if (request == (const YORULINK_MessageTypeDef *)0 ||
        request->Type != YORULINK_TYPE_REQUEST || request->Sequence == 0u) {
        return YORULINK_ERR_INVALID_ARG;
    }
    return YORULINK_Send(hlink, YORULINK_TYPE_RESPONSE, request->Sequence,
                         request->Command, request->Operation, payload, length);
}

YORULINK_ERROR_TYPE YORULINK_ReplyError(YORULINK_HandleTypeDef *hlink,
                                        const YORULINK_MessageTypeDef *request,
                                        YORULINK_ERROR_TYPE error)
{
    yorulink_u8_t payload[YORULINK_WIRE_ERROR_SIZE];
    yorulink_u32_t raw = (yorulink_u32_t)error;
    yorulink_u8_t index;
    if (request == (const YORULINK_MessageTypeDef *)0 ||
        request->Type != YORULINK_TYPE_REQUEST || request->Sequence == 0u) {
        return YORULINK_ERR_INVALID_ARG;
    }
    for (index = 0u; index < YORULINK_WIRE_ERROR_SIZE; ++index) {
        payload[index] = (yorulink_u8_t)(raw >> ((yorulink_u32_t)index * 8u));
    }
    return YORULINK_Send(hlink, YORULINK_TYPE_ERROR, request->Sequence,
                         request->Command, request->Operation,
                         payload, YORULINK_WIRE_ERROR_SIZE);
}

YORULINK_ERROR_TYPE YORULINK_GetRemoteError(const YORULINK_MessageTypeDef *message,
                                            YORULINK_ERROR_TYPE *error)
{
    yorulink_u32_t raw = 0u;
    yorulink_u8_t index;
    if (message == (const YORULINK_MessageTypeDef *)0 ||
        error == (YORULINK_ERROR_TYPE *)0 || message->Type != YORULINK_TYPE_ERROR) {
        return YORULINK_ERR_INVALID_ARG;
    }
    if (message->Data == (const yorulink_u8_t *)0 ||
        message->Length < YORULINK_WIRE_ERROR_SIZE) {
        return YORULINK_ERR_MALFORMED;
    }
    for (index = 0u; index < YORULINK_WIRE_ERROR_SIZE; ++index) {
        raw |= (yorulink_u32_t)message->Data[index] << ((yorulink_u32_t)index * 8u);
    }
    *error = (YORULINK_ERROR_TYPE)raw;
    return YORULINK_ERR_OK;
}

yorulink_size_t YORULINK_Input(YORULINK_HandleTypeDef *hlink,
                               const yorulink_u8_t *data,
                               yorulink_size_t length)
{
    yorulink_size_t accepted = 0u;
    yorulink_size_t tail;
    yorulink_size_t next;
    if (hlink == (YORULINK_HandleTypeDef *)0 ||
        (data == (const yorulink_u8_t *)0 && length != 0u)) {
        return 0u;
    }
    tail = hlink->RxTail;
    while (accepted < length) {
        next = tail + 1u;
        if (next == (yorulink_size_t)(YORULINK_RX_QUEUE_SIZE + 1u)) {
            next = 0u;
        }
        if (next == hlink->RxHead) {
            break;
        }
        hlink->RxQueue[tail] = data[accepted];
        tail = next;
        ++accepted;
    }
    hlink->RxTail = tail;
    if (accepted != length) {
        ++hlink->Stats.RxOverflows;
        hlink->ParserNeedsResync = 1u;
    }
    return accepted;
}

static void yorulink__dispatch_(YORULINK_HandleTypeDef *hlink)
{
    const YORULINK_CommandTypeDef *entry = (const YORULINK_CommandTypeDef *)0;
    YORULINK_ResponseHandlerTypeDef callback;
    void *callback_context;
    YORULINK_ERROR_TYPE remote_error;
    yorulink_u16_t index;
    yorulink_u8_t command_exists = 0u;
    YORULINK_MessageTypeDef *message = &hlink->ParsedMessage;

    if ((message->Type == YORULINK_TYPE_REQUEST && message->Sequence == 0u) ||
        (message->Type == YORULINK_TYPE_EVENT && message->Sequence != 0u) ||
        ((message->Type == YORULINK_TYPE_RESPONSE || message->Type == YORULINK_TYPE_ERROR) &&
         message->Sequence == 0u) ||
        (message->Type == YORULINK_TYPE_ERROR &&
         message->Length < YORULINK_WIRE_ERROR_SIZE)) {
        ++hlink->Stats.SemanticErrors;
        return;
    }

    if (message->Type == YORULINK_TYPE_RESPONSE || message->Type == YORULINK_TYPE_ERROR) {
        if (hlink->PendingActive != 0u &&
            message->Sequence == hlink->PendingSequence &&
            message->Command == hlink->PendingCommand &&
            message->Operation == hlink->PendingOperation) {
            callback = hlink->PendingCallback;
            callback_context = hlink->PendingContext;
            hlink->PendingActive = 0u;
            hlink->PendingCallback = (YORULINK_ResponseHandlerTypeDef)0;
            hlink->PendingContext = (void *)0;
            remote_error = YORULINK_ERR_OK;
            if (message->Type == YORULINK_TYPE_ERROR &&
                YORULINK_GetRemoteError(message, &remote_error) != YORULINK_ERR_OK) {
                ++hlink->Stats.SemanticErrors;
                return;
            }
            if (callback != (YORULINK_ResponseHandlerTypeDef)0) {
                callback(hlink, remote_error, message, callback_context);
            }
        } else {
            ++hlink->Stats.UnmatchedResponses;
        }
        return;
    }

    for (index = 0u; index < hlink->CommandCount; ++index) {
        if (hlink->CommandTable[index].Command == message->Command) {
            command_exists = 1u;
            if (hlink->CommandTable[index].Operation == message->Operation) {
                entry = &hlink->CommandTable[index];
                break;
            }
        }
    }

    if (message->Type == YORULINK_TYPE_EVENT) {
        if (entry != (const YORULINK_CommandTypeDef *)0 &&
            message->Length >= entry->MinLength && message->Length <= entry->MaxLength) {
            entry->Handler(hlink, message);
        }
        return;
    }

    if (entry == (const YORULINK_CommandTypeDef *)0) {
        (void)YORULINK_ReplyError(hlink, message,
                                  command_exists != 0u
                                      ? YORULINK_ERR_UNKNOWN_OPERATION
                                      : YORULINK_ERR_UNKNOWN_COMMAND);
    } else if (message->Length < entry->MinLength || message->Length > entry->MaxLength) {
        (void)YORULINK_ReplyError(hlink, message, YORULINK_ERR_BAD_LENGTH);
    } else {
        entry->Handler(hlink, message);
    }
}

static void yorulink__parse_byte_(YORULINK_HandleTypeDef *hlink, yorulink_u8_t value)
{
    switch (hlink->ParserState) {
    case YORULINK_PARSE_WAIT_SOF0:
        if (value == YORULINK_SOF0) {
            hlink->ParserState = YORULINK_PARSE_WAIT_SOF1;
        }
        break;
    case YORULINK_PARSE_WAIT_SOF1:
        if (value == YORULINK_SOF1) {
            hlink->ParserState = YORULINK_PARSE_VERSION;
            hlink->CalculatedCrc = 0xFFFFu;
        } else if (value != YORULINK_SOF0) {
            hlink->ParserState = YORULINK_PARSE_WAIT_SOF0;
        }
        break;
    case YORULINK_PARSE_VERSION:
        if (value != YORULINK_PROTOCOL_VERSION) {
            ++hlink->Stats.VersionErrors;
            yorulink__parser_reset_(hlink);
        } else {
            hlink->ParsedMessage.Version = value;
            hlink->CalculatedCrc = yorulink__crc_byte_(hlink->CalculatedCrc, value);
            hlink->ParserState = YORULINK_PARSE_TYPE;
        }
        break;
    case YORULINK_PARSE_TYPE:
        if (value < YORULINK_TYPE_REQUEST || value > YORULINK_TYPE_ERROR) {
            ++hlink->Stats.TypeErrors;
            yorulink__parser_reset_(hlink);
        } else {
            hlink->ParsedMessage.Type = value;
            hlink->CalculatedCrc = yorulink__crc_byte_(hlink->CalculatedCrc, value);
            hlink->ParserState = YORULINK_PARSE_SEQUENCE;
        }
        break;
    case YORULINK_PARSE_SEQUENCE:
        hlink->ParsedMessage.Sequence = value;
        hlink->CalculatedCrc = yorulink__crc_byte_(hlink->CalculatedCrc, value);
        hlink->ParserState = YORULINK_PARSE_COMMAND;
        break;
    case YORULINK_PARSE_COMMAND:
        hlink->ParsedMessage.Command = value;
        hlink->CalculatedCrc = yorulink__crc_byte_(hlink->CalculatedCrc, value);
        hlink->ParserState = YORULINK_PARSE_OPERATION;
        break;
    case YORULINK_PARSE_OPERATION:
        hlink->ParsedMessage.Operation = value;
        hlink->CalculatedCrc = yorulink__crc_byte_(hlink->CalculatedCrc, value);
        hlink->ParserState = YORULINK_PARSE_LENGTH_LOW;
        break;
    case YORULINK_PARSE_LENGTH_LOW:
        hlink->ParsedMessage.Length = value;
        hlink->CalculatedCrc = yorulink__crc_byte_(hlink->CalculatedCrc, value);
        hlink->ParserState = YORULINK_PARSE_LENGTH_HIGH;
        break;
    case YORULINK_PARSE_LENGTH_HIGH:
        hlink->ParsedMessage.Length |= (yorulink_u16_t)((yorulink_u16_t)value << 8u);
        hlink->CalculatedCrc = yorulink__crc_byte_(hlink->CalculatedCrc, value);
        hlink->PayloadOffset = 0u;
        if (hlink->ParsedMessage.Length > YORULINK_MAX_PAYLOAD) {
            ++hlink->Stats.LengthErrors;
            yorulink__parser_reset_(hlink);
        } else {
            hlink->ParserState = hlink->ParsedMessage.Length == 0u
                                     ? YORULINK_PARSE_CRC_LOW
                                     : YORULINK_PARSE_PAYLOAD;
        }
        break;
    case YORULINK_PARSE_PAYLOAD:
        hlink->Payload[hlink->PayloadOffset++] = value;
        hlink->CalculatedCrc = yorulink__crc_byte_(hlink->CalculatedCrc, value);
        if (hlink->PayloadOffset == hlink->ParsedMessage.Length) {
            hlink->ParserState = YORULINK_PARSE_CRC_LOW;
        }
        break;
    case YORULINK_PARSE_CRC_LOW:
        hlink->ReceivedCrc = value;
        hlink->ParserState = YORULINK_PARSE_CRC_HIGH;
        break;
    case YORULINK_PARSE_CRC_HIGH:
        hlink->ReceivedCrc |= (yorulink_u16_t)((yorulink_u16_t)value << 8u);
        if (hlink->ReceivedCrc == hlink->CalculatedCrc) {
            ++hlink->Stats.FramesReceived;
            yorulink__dispatch_(hlink);
        } else {
            ++hlink->Stats.CrcErrors;
        }
        yorulink__parser_reset_(hlink);
        break;
    default:
        yorulink__parser_reset_(hlink);
        break;
    }
}

void YORULINK_Process(YORULINK_HandleTypeDef *hlink)
{
    yorulink_size_t head;
    yorulink_u8_t value;
    YORULINK_ResponseHandlerTypeDef callback;
    void *callback_context;
    if (hlink == (YORULINK_HandleTypeDef *)0) {
        return;
    }
    if (hlink->ParserNeedsResync != 0u) {
        hlink->ParserNeedsResync = 0u;
        yorulink__parser_reset_(hlink);
    }
    head = hlink->RxHead;
    while (head != hlink->RxTail) {
        value = hlink->RxQueue[head];
        ++head;
        if (head == (yorulink_size_t)(YORULINK_RX_QUEUE_SIZE + 1u)) {
            head = 0u;
        }
        hlink->RxHead = head;
        yorulink__parse_byte_(hlink, value);
    }
    if (hlink->PendingActive != 0u && hlink->PendingTimeout != 0u &&
        hlink->Tick != (YORULINK_TickFnTypeDef)0 &&
        (yorulink_u32_t)(hlink->Tick(hlink->TickContext) - hlink->PendingStartTick) >=
            hlink->PendingTimeout) {
        callback = hlink->PendingCallback;
        callback_context = hlink->PendingContext;
        hlink->PendingActive = 0u;
        hlink->PendingCallback = (YORULINK_ResponseHandlerTypeDef)0;
        hlink->PendingContext = (void *)0;
        ++hlink->Stats.RequestTimeouts;
        if (callback != (YORULINK_ResponseHandlerTypeDef)0) {
            callback(hlink, YORULINK_ERR_TIMEOUT,
                     (const YORULINK_MessageTypeDef *)0, callback_context);
        }
    }
}

YORULINK_ERROR_TYPE YORULINK_Request(YORULINK_HandleTypeDef *hlink,
                                     yorulink_u8_t command,
                                     yorulink_u8_t operation,
                                     const yorulink_u8_t *payload,
                                     yorulink_u16_t length,
                                     yorulink_u32_t timeout_ms,
                                     YORULINK_ResponseHandlerTypeDef callback,
                                     void *context)
{
    YORULINK_ERROR_TYPE status;
    yorulink_u8_t sequence;
    yorulink_u32_t effective_timeout;
    if (hlink == (YORULINK_HandleTypeDef *)0) {
        return YORULINK_ERR_INVALID_ARG;
    }
    if (hlink->PendingActive != 0u) {
        return YORULINK_ERR_BUSY;
    }
    effective_timeout = timeout_ms == 0u ? YORULINK_DEFAULT_TIMEOUT_MS : timeout_ms;
    if (effective_timeout != 0u && hlink->Tick == (YORULINK_TickFnTypeDef)0) {
        return YORULINK_ERR_NO_TICK;
    }
    sequence = hlink->NextSequence;
    hlink->NextSequence = sequence == 0xFFu ? 1u : (yorulink_u8_t)(sequence + 1u);
    hlink->PendingActive = 1u;
    hlink->PendingSequence = sequence;
    hlink->PendingCommand = command;
    hlink->PendingOperation = operation;
    hlink->PendingTimeout = effective_timeout;
    hlink->PendingStartTick = effective_timeout == 0u
                                  ? 0u
                                  : hlink->Tick(hlink->TickContext);
    hlink->PendingCallback = callback;
    hlink->PendingContext = context;
    status = YORULINK_Send(hlink, YORULINK_TYPE_REQUEST, sequence,
                           command, operation, payload, length);
    if (status != YORULINK_ERR_OK) {
        hlink->PendingActive = 0u;
        hlink->PendingCallback = (YORULINK_ResponseHandlerTypeDef)0;
        hlink->PendingContext = (void *)0;
    }
    return status;
}

static yorulink_u8_t yorulink__reader_can_(const YORULINK_ReaderTypeDef *reader,
                                           yorulink_u16_t length)
{
    return (yorulink_u8_t)(reader != (const YORULINK_ReaderTypeDef *)0 &&
                           reader->Error == 0u && reader->Offset <= reader->Length &&
                           length <= (yorulink_u16_t)(reader->Length - reader->Offset));
}

static yorulink_u8_t yorulink__writer_can_(const YORULINK_WriterTypeDef *writer,
                                           yorulink_u16_t length)
{
    return (yorulink_u8_t)(writer != (const YORULINK_WriterTypeDef *)0 &&
                           writer->Error == 0u && writer->Offset <= writer->Capacity &&
                           length <= (yorulink_u16_t)(writer->Capacity - writer->Offset));
}

static yorulink_u8_t yorulink__reader_fail_(YORULINK_ReaderTypeDef *reader)
{
    if (reader != (YORULINK_ReaderTypeDef *)0) {
        reader->Error = 1u;
    }
    return 0u;
}

static yorulink_u8_t yorulink__writer_fail_(YORULINK_WriterTypeDef *writer)
{
    if (writer != (YORULINK_WriterTypeDef *)0) {
        writer->Error = 1u;
    }
    return 0u;
}

void YORULINK_ReaderInit(YORULINK_ReaderTypeDef *reader,
                         const yorulink_u8_t *data,
                         yorulink_u16_t length)
{
    if (reader != (YORULINK_ReaderTypeDef *)0) {
        reader->Data = data;
        reader->Length = length;
        reader->Offset = 0u;
        reader->Error = (yorulink_u8_t)(data == (const yorulink_u8_t *)0 && length != 0u);
    }
}

yorulink_u8_t YORULINK_ReadU8(YORULINK_ReaderTypeDef *reader, yorulink_u8_t *value)
{
    if (value == (yorulink_u8_t *)0 || yorulink__reader_can_(reader, 1u) == 0u) {
        return yorulink__reader_fail_(reader);
    }
    *value = reader->Data[reader->Offset++];
    return 1u;
}

yorulink_u8_t YORULINK_ReadI8(YORULINK_ReaderTypeDef *reader, yorulink_i8_t *value)
{
    yorulink_u8_t raw;
    if (value == (yorulink_i8_t *)0 || YORULINK_ReadU8(reader, &raw) == 0u) {
        return yorulink__reader_fail_(reader);
    }
    *value = (yorulink_i8_t)raw;
    return 1u;
}

yorulink_u8_t YORULINK_ReadU16(YORULINK_ReaderTypeDef *reader, yorulink_u16_t *value)
{
    yorulink_u16_t offset;
    if (value == (yorulink_u16_t *)0 || yorulink__reader_can_(reader, 2u) == 0u) {
        return yorulink__reader_fail_(reader);
    }
    offset = reader->Offset;
    *value = (yorulink_u16_t)((yorulink_u16_t)reader->Data[offset] |
                              (yorulink_u16_t)((yorulink_u16_t)reader->Data[offset + 1u] << 8u));
    reader->Offset = (yorulink_u16_t)(offset + 2u);
    return 1u;
}

yorulink_u8_t YORULINK_ReadI16(YORULINK_ReaderTypeDef *reader, yorulink_i16_t *value)
{
    yorulink_u16_t raw;
    if (value == (yorulink_i16_t *)0 || YORULINK_ReadU16(reader, &raw) == 0u) {
        return yorulink__reader_fail_(reader);
    }
    *value = (yorulink_i16_t)raw;
    return 1u;
}

yorulink_u8_t YORULINK_ReadU32(YORULINK_ReaderTypeDef *reader, yorulink_u32_t *value)
{
    yorulink_u16_t offset;
    if (value == (yorulink_u32_t *)0 || yorulink__reader_can_(reader, 4u) == 0u) {
        return yorulink__reader_fail_(reader);
    }
    offset = reader->Offset;
    *value = (yorulink_u32_t)reader->Data[offset]
             | ((yorulink_u32_t)reader->Data[offset + 1u] << 8u)
             | ((yorulink_u32_t)reader->Data[offset + 2u] << 16u)
             | ((yorulink_u32_t)reader->Data[offset + 3u] << 24u);
    reader->Offset = (yorulink_u16_t)(offset + 4u);
    return 1u;
}

yorulink_u8_t YORULINK_ReadI32(YORULINK_ReaderTypeDef *reader, yorulink_i32_t *value)
{
    yorulink_u32_t raw;
    if (value == (yorulink_i32_t *)0 || YORULINK_ReadU32(reader, &raw) == 0u) {
        return yorulink__reader_fail_(reader);
    }
    *value = (yorulink_i32_t)raw;
    return 1u;
}

yorulink_u8_t YORULINK_ReadBytes(YORULINK_ReaderTypeDef *reader,
                                 yorulink_u8_t *value,
                                 yorulink_u16_t length)
{
    if ((value == (yorulink_u8_t *)0 && length != 0u) ||
        yorulink__reader_can_(reader, length) == 0u) {
        return yorulink__reader_fail_(reader);
    }
    if (length != 0u) {
        yorulink__copy_(value, &reader->Data[reader->Offset], length);
        reader->Offset = (yorulink_u16_t)(reader->Offset + length);
    }
    return 1u;
}

yorulink_u16_t YORULINK_ReaderRemaining(const YORULINK_ReaderTypeDef *reader)
{
    return reader == (const YORULINK_ReaderTypeDef *)0 || reader->Offset > reader->Length
               ? 0u
               : (yorulink_u16_t)(reader->Length - reader->Offset);
}

yorulink_u8_t YORULINK_ReaderError(const YORULINK_ReaderTypeDef *reader)
{
    return (yorulink_u8_t)(reader == (const YORULINK_ReaderTypeDef *)0 || reader->Error != 0u);
}

void YORULINK_WriterInit(YORULINK_WriterTypeDef *writer,
                         yorulink_u8_t *data,
                         yorulink_u16_t capacity)
{
    if (writer != (YORULINK_WriterTypeDef *)0) {
        writer->Data = data;
        writer->Capacity = capacity;
        writer->Offset = 0u;
        writer->Error = (yorulink_u8_t)(data == (yorulink_u8_t *)0 && capacity != 0u);
    }
}

yorulink_u8_t YORULINK_WriteU8(YORULINK_WriterTypeDef *writer, yorulink_u8_t value)
{
    if (yorulink__writer_can_(writer, 1u) == 0u) {
        return yorulink__writer_fail_(writer);
    }
    writer->Data[writer->Offset++] = value;
    return 1u;
}

yorulink_u8_t YORULINK_WriteI8(YORULINK_WriterTypeDef *writer, yorulink_i8_t value)
{
    return YORULINK_WriteU8(writer, (yorulink_u8_t)value);
}

yorulink_u8_t YORULINK_WriteU16(YORULINK_WriterTypeDef *writer, yorulink_u16_t value)
{
    yorulink_u16_t offset;
    if (yorulink__writer_can_(writer, 2u) == 0u) {
        return yorulink__writer_fail_(writer);
    }
    offset = writer->Offset;
    writer->Data[offset] = (yorulink_u8_t)(value & 0xFFu);
    writer->Data[offset + 1u] = (yorulink_u8_t)(value >> 8u);
    writer->Offset = (yorulink_u16_t)(offset + 2u);
    return 1u;
}

yorulink_u8_t YORULINK_WriteI16(YORULINK_WriterTypeDef *writer, yorulink_i16_t value)
{
    return YORULINK_WriteU16(writer, (yorulink_u16_t)value);
}

yorulink_u8_t YORULINK_WriteU32(YORULINK_WriterTypeDef *writer, yorulink_u32_t value)
{
    yorulink_u16_t offset;
    if (yorulink__writer_can_(writer, 4u) == 0u) {
        return yorulink__writer_fail_(writer);
    }
    offset = writer->Offset;
    writer->Data[offset] = (yorulink_u8_t)(value & 0xFFu);
    writer->Data[offset + 1u] = (yorulink_u8_t)((value >> 8u) & 0xFFu);
    writer->Data[offset + 2u] = (yorulink_u8_t)((value >> 16u) & 0xFFu);
    writer->Data[offset + 3u] = (yorulink_u8_t)(value >> 24u);
    writer->Offset = (yorulink_u16_t)(offset + 4u);
    return 1u;
}

yorulink_u8_t YORULINK_WriteI32(YORULINK_WriterTypeDef *writer, yorulink_i32_t value)
{
    return YORULINK_WriteU32(writer, (yorulink_u32_t)value);
}

yorulink_u8_t YORULINK_WriteBytes(YORULINK_WriterTypeDef *writer,
                                  const yorulink_u8_t *value,
                                  yorulink_u16_t length)
{
    if ((value == (const yorulink_u8_t *)0 && length != 0u) ||
        yorulink__writer_can_(writer, length) == 0u) {
        return yorulink__writer_fail_(writer);
    }
    if (length != 0u) {
        yorulink__copy_(&writer->Data[writer->Offset], value, length);
        writer->Offset = (yorulink_u16_t)(writer->Offset + length);
    }
    return 1u;
}

yorulink_u16_t YORULINK_WriterLength(const YORULINK_WriterTypeDef *writer)
{
    return writer == (const YORULINK_WriterTypeDef *)0 ? 0u : writer->Offset;
}

yorulink_u16_t YORULINK_WriterRemaining(const YORULINK_WriterTypeDef *writer)
{
    return writer == (const YORULINK_WriterTypeDef *)0 || writer->Offset > writer->Capacity
               ? 0u
               : (yorulink_u16_t)(writer->Capacity - writer->Offset);
}

yorulink_u8_t YORULINK_WriterError(const YORULINK_WriterTypeDef *writer)
{
    return (yorulink_u8_t)(writer == (const YORULINK_WriterTypeDef *)0 || writer->Error != 0u);
}

#endif /* YORULINK_IMPLEMENTATION_ONCE */
#endif /* YORULINK_IMPLEMENTATION */
