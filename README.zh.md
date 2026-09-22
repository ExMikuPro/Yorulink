# Yorulink

[English](README.md)

Yorulink 是一个面向 MCU ↔ MCU 字节流通信的轻量、硬件无关二进制协议库。
它以单头文件发布，使用固定内存，并可在没有 hosted C runtime 的裸机 C99
工程中使用。

适合 UART、USB CDC、SPI bridge、Host 串口及其他可靠或半可靠字节流链路。

---

> Yoru Series
>
> 面向嵌入式项目的相关轻量工具库。
>
> | Library | 用途 |
> |---|---|
> | [Yorulog](https://github.com/ExMikuPro/Yorulog) | 轻量 UART 日志库 |
> | [Yorush](https://github.com/ExMikuPro/Yorush) | 轻量 UART Shell / 命令解析器 |
> | [Yorulink](https://github.com/ExMikuPro/Yorulink) | 轻量 MCU-to-MCU 二进制通信库 |
> | [Yorunvm](https://github.com/ExMikuPro/Yorunvm) | STM32 片上 NVM / Flash / EEPROM 访问工具 |
> | [Yorukv](https://github.com/ExMikuPro/Yorukv) | 轻量 KV 配置库 |
> | [Yorubench](https://github.com/ExMikuPro/Yorubench) | 轻量性能测量库 |
> | [Yoruassert](https://github.com/ExMikuPro/Yoruassert) | 轻量断言工具 |

## 特性

- 只需复制 `yorulink.h`
- C99，适合 freestanding 环境
- 不使用 `malloc`、`printf` 或必须的 libc 内存/字符串函数
- 固定大小 RX 队列、Parser、Payload 和 TX Frame Buffer
- REQUEST、RESPONSE、EVENT、ERROR
- CRC-16/CCITT-FALSE
- 支持任意拆包、连续帧和错误后重新同步的流式 Parser
- 静态 CMD+OP 分发表
- 带边界检查的 Little Endian Reader/Writer
- 单 pending request 与支持 Tick 回绕的 timeout
- 可直接接入项目统一错误码体系
- Host 测试、ASan/UBSan 和 Parser fuzz target

## 🌙 平台支持

下表记录的是 Yorulink 的实际构建和硬件验证状态，并不是能够运行 Yorulink 的 MCU
白名单。未列出的平台并不代表无法使用；只要能够实现对应的 Transport / Port
Contract，原则上即可移植，Core 不维护器件白名单。设计面向现代工具链，硬件差异
留在可跨架构移植的 Core 之外。

| 状态 | 含义 |
|---|---|
| 🌱 Planned | 已计划验证 |
| 🧩 Porting | 正在移植或验证 |
| 🔧 Build Verified | 已交叉编译并链接；不代表实机运行 |
| 🧪 Hardware Validated | 已在真实硬件完成基本功能验证 |
| 🔥 Stress Validated | 已通过实机压力验证 |
| 🌟 Reference Target | 主要基准或回归目标 |

### 已验证目标

| 平台 | 厂商 | 架构 | Transport | 配置（Payload / RX Queue） | 状态 |
|---|---|---|---|---|---|
| STM32H743XIH6 | ST | Cortex-M7 | USB CDC | 256 B / 512 B | 🔥 Stress Validated |
| STM32G031G8U6 | ST | Cortex-M0+ | USART1 / CH340N | 64 B / 128 B | 🌟 Reference Target · 🔥 Stress Validated |
| PY32F002AF15P6TU | Puya | Cortex-M0+ | USART1 / CH340N | 64 B / 128 B | 🔥 Stress Validated |
| MSPM0G3507 | Texas Instruments | Cortex-M0+ | UART0 / XDS110 Backchannel UART | 64 B / 128 B | 🔥 Stress Validated |

### 🌱 计划验证目标

| 平台 | 架构 | 计划验证内容 |
|---|---|---|
| STM32F070 | Cortex-M0 | 小资源 STM32 目标 |
| STM32F072 | Cortex-M0 | Cortex-M0 / USB-UART 验证 |
| STM32F407 | Cortex-M4F | DMA / 性能验证 |
| STC89C52RC | 8051 | 使用合适现代工具链的旧架构计划验证 |
| IA-16 / 8086 | x86-16 | 旧架构可移植性计划验证 |

Yorulink 面向现代嵌入式 C 工具链。较老 CPU 架构可以作为移植目标，但不会为了
兼容旧式专有编译器方言而牺牲现代 MCU 上的易用性、性能或代码质量。

### 实机验证摘要

| 目标 | 内核 / 时钟 | 配置 | Handle | Transport | Clean stress |
|---|---|---|---:|---|---:|
| STM32H743XIH6 | Cortex-M7 / 64 MHz | 256 / 512 | 1,152 B | USB CDC | 100k PASS |
| STM32G031G8U6 | Cortex-M0+ / 64 MHz | 64 / 128 | 384 B | 115200 UART | 100k PASS |
| PY32F002AF15P6TU | Cortex-M0+ / 24 MHz | 64 / 128 | 384 B | 115200 UART | 100k PASS |
| MSPM0G3507 | Cortex-M0+ / 32 MHz | 64 / 128 | 384 B | 115200 UART | 100k PASS |

表中的 `100k PASS` 表示该平台已完成 100,000 次真实硬件事务，并在对应 clean
stress 阶段未出现意外协议错误。各平台的功能、Parser 恢复和复位后检查也通过。
同一套 Wire Protocol V1 已在 ST、Puya 和 Texas Instruments 目标上完成实机
验证；各测试工程均未修改其所用 Yorulink Core。H743 验证使用的头文件修订与
其他三个目标不同。

### 验证原则

构建验证确认目标工具链可编译链接；实机验证确认开发板上的功能；压力验证记录
在指定 Transport 和配置下完成的一次测试。这些结果不代表其他器件或运行条件
已获验证。观察到的事务速率还受 Transport、波特率、Host 调度、时钟和 SDK
影响，是集成测试结果，不是可跨平台比较的 Core 性能基准。

## 快速开始

移植 Yorulink **不需要修改 `yorulink.h`**。新平台只需提供一个 TX 发送回调，
将收到的任意字节块交给 `YORULINK_Input()`，并在主循环或协议任务中调用
`YORULINK_Process()`。只有本机主动发起有效超时非零的 Request 时才额外需要
毫秒 Tick 回调；默认配置下 Request 会使用超时。

```text
复制 yorulink.h → 一个 .c 定义 IMPLEMENTATION → Init → SetWrite
                                               RX → Input → Process
                                               主动 Request？→ SetTick
```

## 移植指南

### 可选 Port 模板

如果希望直接复制一份适配层再修改，可参考 [`port/`](port/)。
模板将平台相关的 TX 与 Tick 实现放在 Core 之外；
`yorulink.h` 本身不依赖该目录。

### 移植时真正需要的 API

| 使用位置 | 需要做什么 | Yorulink API | 是否必需 |
|---|---|---|---|
| 恰好一个 `.c` 文件 | 启用单头实现 | `YORULINK_IMPLEMENTATION` | 必需 |
| 初始化阶段 | 初始化实例和命令表 | `YORULINK_Init()` | 必需 |
| TX Port | 绑定字节发送函数 | `YORULINK_SetWrite()` | 必需 |
| RX callback / ISR | 将收到的字节入队 | `YORULINK_Input()` | 必需 |
| 主循环 / 协议任务 | 运行解析、分发和超时 | `YORULINK_Process()` | 必需 |
| 毫秒时基 | 支持本机主动 Request 超时 | `YORULINK_SetTick()` | Request 有有效超时时需要 |

`YORULINK_Reply()`、`YORULINK_ReplyError()`、`YORULINK_SendEvent()`、
`YORULINK_Request()` 和 Reader/Writer 是应用/协议 API，不是硬件 Port 接口。

```text
RX：UART / USB / SPI / TCP → Input → RX 队列 → Process → Parser + CRC
                                                        → CMD + OP 表 → Handler

TX：Handler / Request / Event → Yorulink → Write 回调 → Transport
```

Transport 只负责搬运字节，不必理解 Yorulink 帧；Yorulink 也不区分这些字节来自
UART、USB、SPI 还是 TCP。
Core 负责协议、Parser、CRC、事务和 Reader/Writer；应用 / Port 负责 UART、
USB、SPI 或 TCP 接入、Tick 来源、DMA Buffer 生命周期及平台 SDK 调用。
新平台适配原则上应在应用 / Port 层完成，而不是向 Core 中不断加入厂商 `#ifdef`。
下方代码块是应用源文件的片段；可执行语句应分别放进初始化函数、回调或主循环。

### 1. 复制一个头文件，只定义一次实现

最终只需要 `yorulink.h`，不需要 `yorulink.c`、`yorulink_port.c` 或
`yorulink_config.h`。在**恰好一个** `.c` 文件中：

```c
#define YORULINK_IMPLEMENTATION
#include "yorulink.h"
```

其他 `.c` 只写 `#include "yorulink.h"`。多个 Translation Unit 重复定义
`YORULINK_IMPLEMENTATION` 会产生重复符号。配置和错误类型宏必须在所有
Translation Unit 中一致，详见[配置](#配置)。

### 2. 为每条独立链路创建 Handle

```c
static YORULINK_HandleTypeDef hLink;
/* 两种独立 Transport 也可以分别使用 hLinkMotor、hLinkSensor。 */
```

Handle 由调用者持有，并应在链路使用期间一直存在。Yorulink 没有内部全局单例；
每个 Handle 都有自己的 RX 队列、Parser、TX Buffer 和一个 pending Request
槽位。不要把长期使用的 Handle 放在函数的临时栈帧中。

### 3. 定义应用 CMD/OP、Handler 和静态命令表

CMD 是功能域，OP 是该功能域内的操作。下面的数值**只是应用示例**，不是
Wire Protocol 的保留值。

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
    /* 在此将 state 应用到你的 LED。 */
    (void)YORULINK_Reply(link, message, (const yorulink_u8_t *)0, 0u);
}

static const YORULINK_CommandTypeDef commands[] = {
    {CMD_SYSTEM, OP_PING,    0u, 0u, on_ping},
    {CMD_LED,    OP_LED_SET, 1u, 1u, on_led_set}
};
```

每项依次是 `{Command, Operation, MinLength, MaxLength, Handler}`。
Yorulink 会在调用 Handler 前检查注册的 Payload 长度范围。对 Request，未知
CMD/OP 或长度不符会自动返回 ERROR 帧。命令表的生命周期必须覆盖 Handle 的
使用期，因为 `YORULINK_Init()` 保存的是指针。通信双方需自行约定 CMD/OP
含义和 Payload 布局。

### 4. 初始化 Handle

```c
YORULINK_ERROR_TYPE status = YORULINK_Init(
    &hLink, commands,
    (yorulink_u16_t)(sizeof(commands) / sizeof(commands[0])));
if (status != YORULINK_ERR_OK) {
    /* 在应用中处理配置错误。 */
}
```

仅主动发请求、没有传入 Request/Event Handler 的客户端可以使用
`YORULINK_Init(&hLink, NULL, 0u)`。有 Handler 的接收端若需 Reply 或自动
返回 ERROR，仍必须绑定 Write 回调。初始化会校验表项和重复 CMD/OP；
**在初始化之后**绑定回调，因为重新初始化会清除之前的回调。

### 5. 实现并绑定 TX Port

Write 回调收到的是**已完成编码的完整帧**，包括 SOF、Header、Payload 和
CRC。返回类型为 `YORULINK_ERROR_TYPE`；发送 API 会原样传递其返回值。
Transport 只需发送或复制这些字节，不必解析 CMD、OP 或 CRC。

最容易理解的 STM32 HAL **阻塞 UART** 示例：

```c
/* 仅为 STM32 HAL Port 示例；Yorulink Core 不依赖 HAL。 */
static YORULINK_ERROR_TYPE link_write(void *context,
                                      const yorulink_u8_t *data,
                                      yorulink_u16_t length)
{
    UART_HandleTypeDef *huart = (UART_HandleTypeDef *)context;
    if (HAL_UART_Transmit(huart, (uint8_t *)data, length, 100u) != HAL_OK) {
        return YORULINK_ERR_INTERNAL; /* 也可以映射到项目自己的 UART 错误码。 */
    }
    return YORULINK_ERR_OK;
}

YORULINK_SetWrite(&hLink, link_write, &huart1);
```

驱动的部分发送或失败情况仍由应用处理。这里的强制类型转换只因 HAL 的 TX
参数不接受 `const`；请勿修改 `data`。

> **异步 TX 生命周期警告：**`data` 指向 Handle 中可复用的 `TxBuffer`，仅在
> Write 回调返回前有效。如果 DMA 或 USB Middleware 在返回后继续读取，
> **必须先把完整帧复制到应用持有的持久 Buffer/Queue**。提交发送不等于完成。

```c
/* 错误：回调返回后 DMA/USB 可能仍在读取 data。 */
HAL_UART_Transmit_DMA(&huart1, (uint8_t *)data, length);
/* USB 异步发送同理：CDC_Transmit_FS((uint8_t *)data, length); */
return YORULINK_ERR_OK;
```

正确的所有权关系：

```text
Yorulink TxBuffer → copy → 持久 TX Queue → DMA / USB
                              实际发送完成回调后才释放槽位
```

```c
/* 示意：tx_queue_push 必须将 length 个字节全部复制到持久存储。 */
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

队列槽位至少应容纳 `YORULINK_MAX_FRAME_SIZE`；应在实际 DMA/USB 完成回调
之后才复用，并为断线和发送错误制定处理策略。

### 6. RX 收到多少字节，就喂入多少字节

把**实际收到的字节数**传入 `YORULINK_Input()`，并检查返回的接收数。它只将
数据复制进 RX 队列，不运行 Handler。若返回值小于输入长度，RX overflow
计数会增加，处理时 Parser 会重同步。正常运行应通过队列容量和 `Process()`
调度避免溢出。

```c
yorulink_size_t accepted = YORULINK_Input(&hLink, data, length);
if (accepted != length) { /* 记录或处理 RX 过载。 */ }
```

不需要等待完整帧、搜索 `AA 55`、自己计算 CRC 或拼接拆包。一次收到完整帧，
或分多次收到 `AA`、`55 01`、剩余字节，处理后的结果相同。

简短 STM32 回调示意（FS/HS 名称、Handle、Buffer 所有权与重启接收 API
以你的 CubeMX 工程为准）：

```c
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t size)
{
    if (huart == &huart1) {
        (void)YORULINK_Input(&hLink, uart_rx_buffer, (yorulink_size_t)size);
        /* 用工程现有 API 重新启动 UART/DMA 接收。 */
    }
}

static int8_t CDC_Receive_HS(uint8_t *Buf, uint32_t *Len)
{
    (void)YORULINK_Input(&hLink, Buf, (yorulink_size_t)(*Len));
    /* 用工程的 USB Device Handle/API 重新使能 OUT 端点。 */
    return USBD_OK;
}
```

这仅展示 RX callback → Input，并非完整 CubeMX DMA 模板。若回调与
`Process()` 并发，遵循单生产者/单消费者使用方式；多个生产者或同一 Handle
上的并发调用需要由应用串行化。

### 7. 在主循环或协议任务中运行 Process

```c
while (1) {
    YORULINK_Process(&hLink);
    application_process();
}
```

`Input()` 只负责入队；Parser、CRC、CMD/OP 分发、Handler、Response 匹配和
Timeout 回调都在 `Process()` 中执行。要足够频繁地调用它，以免 RX 队列溢出
或超时处理延迟。**默认不要在 UART/USB ISR 中直接调用 `Process()`**，否则
业务 Handler 和回调也在中断上下文运行。RTOS 工程可用专门的协议任务，并
自行安排平台所需的同步。

### 8. 只有主动发起带超时的 Request 才需要 Tick

```c
static yorulink_u32_t link_tick(void *context)
{
    (void)context;
    return HAL_GetTick(); /* STM32 HAL 示例；任何单调毫秒时钟均可。 */
}
YORULINK_SetTick(&hLink, link_tick, (void *)0);
```

只接收 Request、Reply 或 SendEvent **无需** Tick。`YORULINK_Request()`
使用传入的 `timeout_ms`；传 `0` 则使用 `YORULINK_DEFAULT_TIMEOUT_MS`。
有效超时非零而未绑定 Tick 时返回 `YORULINK_ERR_NO_TICK`。默认值为
`1000` ms，所以默认配置下每次 Request 都需要 Tick。毫秒计数允许回绕，
但仍须持续调用 `Process()` 才能检测超时。

### 9. 使用应用层 Event、Request 和 Reply

Event 无需等待对方先发 Request：

```c
enum { CMD_SENSOR = 0x20, OP_SENSOR_UPDATE = 0x01 };
yorulink_u8_t payload[] = {42u};
(void)YORULINK_SendEvent(&hLink, CMD_SENSOR, OP_SENSOR_UPDATE,
                         payload, (yorulink_u16_t)sizeof(payload));
```

主动发 Request 时先绑定 Tick，再提供完成回调：

```c
static void on_response(YORULINK_HandleTypeDef *link,
                        YORULINK_ERROR_TYPE error,
                        const YORULINK_MessageTypeDef *message,
                        void *context)
{
    (void)link;
    (void)context;
    if (message == (const YORULINK_MessageTypeDef *)0) {
        /* 超时：error == YORULINK_ERR_TIMEOUT。 */
    } else if (message->Type == YORULINK_TYPE_ERROR) {
        /* error 是远端的 32-bit 业务错误 ID。 */
    } else if (error == YORULINK_ERR_OK) {
        /* RESPONSE：在此读取 message->Data 和 message->Length。 */
    }
}

(void)YORULINK_Request(&hLink, CMD_SYSTEM, OP_PING,
                       (const yorulink_u8_t *)0, 0u,
                       1000u, on_response, (void *)0);
```

每个 Handle 同时只允许**一个** pending Request；再次发起会得到
`YORULINK_ERR_BUSY`。匹配的 RESPONSE 使 `error == YORULINK_ERR_OK`；
匹配的 ERROR 使 `error` 为远端错误 ID，且 `message` 非空；超时使
`error == YORULINK_ERR_TIMEOUT`、`message == NULL`。回调中的 `message`
及 Payload 属于 Handle，如果回调结束后仍需使用，请先复制。还应检查
`Request()` 的即时返回值：TX 失败会直接返回，不会等完成回调。

处理收到的 Request 时，可用 `YORULINK_Reply(link, message, payload,
length)` 回复成功，或用 `YORULINK_ReplyError(link, message,
application_error)` 回复失败。后者使用 [ERROR Wire 格式](#error-wire-格式)
所述的固定 32-bit 错误 ID。例如：

```c
/* 位于拥有 link 和 message 参数的 Request Handler 内： */
(void)YORULINK_Reply(link, message, (const yorulink_u8_t *)0, 0u);
/* 或者，不回复成功，改为： */
(void)YORULINK_ReplyError(link, message, YORULINK_ERR_UNSUPPORTED);
```

用 `YORULINK_ReaderInit()` 和 `ReadU8/U16/U32/Bytes` 安全解析 Payload；
用 `YORULINK_WriterInit()` 和 `WriteU8/U16/U32/Bytes` 构建 Payload。它们
检查边界，并以 Little Endian 表示多字节值：

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

### 不同 Transport 的接入点

| Transport | RX 接入点 | TX 实现 |
|---|---|---|
| 阻塞 UART | RX IRQ / 轮询 | 阻塞发送 |
| UART DMA | DMA/IDLE 回调 | 持久 DMA TX 队列 |
| USB CDC | CDC Receive 回调 | 持久 USB TX 队列 |
| TCP | Socket `recv` | Socket `send`（处理部分发送） |
| SPI bridge | 收到的字节块 | 平台专用发送 |

这些是接入方式示意，**不代表以上 Transport 都已实机测试**。

### 最小移植 Checklist

- [ ] 复制 `yorulink.h`，恰好一个 `.c` 定义 `YORULINK_IMPLEMENTATION`。
- [ ] 每条链路创建持久的 `YORULINK_HandleTypeDef`。
- [ ] 定义应用 CMD/OP、Handler 与持久命令表。
- [ ] 调用 `YORULINK_Init()` 并检查返回值。
- [ ] 实现 TX Write 回调，并通过 `YORULINK_SetWrite()` 绑定。
- [ ] RX callback 将字节交给 `YORULINK_Input()`，检查接收数。
- [ ] 在主循环或协议任务中定期调用 `YORULINK_Process()`。
- [ ] 主动发带超时的 Request 时绑定 `YORULINK_SetTick()`。
- [ ] 异步 TX 先复制到持久 Buffer/Queue，实际完成后再释放。

你**不需要**修改 `yorulink.h`、实现 CRC、搜索 SOF、等待完整帧、自己拼接
任意拆包、自己分发 CMD/OP、维护 Request SEQ、匹配 Response、处理 Tick
回绕、使用 `malloc` 或引入 RTOS。

## 统一错误码接入

Yorulink 不要求调用者维护第二套错误枚举。在包含头文件前绑定已有错误类型：

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

未覆盖时，头文件提供可独立使用的无符号 32-bit 默认错误类型和数值。三类错误彼此区分：

- Local API Error：通过 API 返回值立即报告。
- Local Protocol Diagnostic：CRC、版本、RX overflow 等只记录 Stats。
- Remote Operation Error：统一业务错误 ID 原样通过 ERROR 帧传输。

上述项目错误码映射仅作示意；应映射应用实际使用的全部错误语义，并让所有
Translation Unit 使用相同定义。

Write callback 返回的错误码会由 `YORULINK_Send()` 及相关 API 原样向上传递，
不会被压缩成 Yorulink 自己的通用 Transport Error。

## ERROR Wire 格式

Wire Protocol V1 只有一种 ERROR 布局。每个 ERROR Payload 都以固定的
Little Endian `uint32` 统一错误 ID 开头：

```text
Offset  Size  Field
0       4     unified_error_id, uint32 little-endian
4       N     可选不透明 Detail（预留扩展）
```

`YORULINK_ERROR_TYPE` 必须至少能容纳 32 bit；更小的项目错误类型会在编译期
报错。V0.1 只发送前四字节 Error ID。

## 配置

| 宏 | 默认值 | 说明 |
|---|---:|---|
| `YORULINK_MAX_PAYLOAD` | `256` | 本地最大 Payload 字节数 |
| `YORULINK_RX_QUEUE_SIZE` | `512` | RX 队列容量 |
| `YORULINK_DEFAULT_TIMEOUT_MS` | `1000` | Request 传入 0 时使用的 timeout |
| `YORULINK_ERROR_TYPE` | `yorulink_error_t` | Public API 错误类型 |
| `YORULINK_ERR_*` | 独立默认值 | 项目统一错误语义映射 |

`YORULINK_MAX_PAYLOAD` 和 `YORULINK_RX_QUEUE_SIZE` 会直接影响**每个**
Handle 的 RAM 占用。配置宏在所有 Translation Unit 中必须一致；通信双方
也要约定可交换的最大 Payload。V1 的 Wire 格式和 32-bit ERROR ID 是固定的，
不通过宏切换。

## DMA 数据生命周期

完整所有权规则与错误/正确示例见[TX Port 步骤](#5-实现并绑定-tx-port)。简言之，
Write 回调中的完整帧指针在回调返回后即失效。

## 资源占用

以下为验证应用的 Release 实测值：

| 目标 | 配置 | `sizeof(YORULINK_HandleTypeDef)` | 约链接 Core text | 完整验证应用 Flash 增量 | 完整应用 RAM 增量 |
|---|---|---:|---:|---:|---:|
| STM32H743XIH6 | 256 / 512 | 1,152 B | 报告未测量 | 未测量 | 未测量 |
| STM32G031G8U6 | 64 / 128 | 384 B | ~2,102 B | 2,952 B | 464 B |
| PY32F002AF15P6TU | 64 / 128 | 384 B | ~2,014 B | 2,884 B | 320 B |
| MSPM0G3507 | 64 / 128 | 384 B | ~4,164 B | 6,872 B | 320 B |

Core text 为基于最终链接符号的估算值。不同目标的编译参数、LTO、section GC、
SDK 集成方式及符号归属可能不同，因此不应直接把这些数字当作跨平台性能或代码
膨胀比较。“完整验证应用增量”包含测试命令、回调和集成代码，不等同于 Yorulink
Core 本身大小。H743 报告给出了最终 Release 应用 Flash（`text+data`）22,500 B
和静态 RAM（`data+bss`）12,312 B，但没有可比的基线增量；另外的 2,160 B USB
TX 队列属于应用层。

每实例 RAM 占用可配置：`YORULINK_MAX_PAYLOAD` 和 `YORULINK_RX_QUEUE_SIZE`
直接影响 Handle 大小。实测 64 / 128 与 256 / 512 配置分别占 384 B 和 1,152 B。
Core 不使用动态内存。

## 构建与测试

```sh
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

使用 `-DYORULINK_ENABLE_SANITIZERS=ON` 开启 ASan/UBSan。Clang 工具链带有
libFuzzer runtime 时，可用 `-DYORULINK_BUILD_FUZZER=ON` 构建 Parser fuzz target。

## 协议摘要

```text
AA 55 | VER | TYPE | SEQ | CMD | OP | LENGTH_LE | PAYLOAD | CRC16_LE
```

CRC 覆盖 `VER` 到 Payload 末尾。规范 Request Vector：

```text
AA 55 01 01 2A 10 02 02 00 05 FF 50 7B
```

在 v0.1.x 系列中，Wire Protocol V1 视为冻结；新增应用 CMD/OP 不需要修改
Wire Version。

## V0.1 不包含

加密、认证、分片、自动重试、动态注册、多节点寻址和自动结构体序列化。

## License

Yorulink 使用 [MIT License](LICENSE)，与其他 Yoru Series 库保持一致。
