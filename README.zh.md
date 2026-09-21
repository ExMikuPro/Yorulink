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

## 快速开始

### 1. 添加头文件

将 `yorulink.h` 复制到项目中。

### 2. 只在一个源文件中启用实现

```c
#define YORULINK_IMPLEMENTATION
#include "yorulink.h"
```

其他源文件正常包含：

```c
#include "yorulink.h"
```

### 3. 绑定 Transport

```c
static YORULINK_ERROR_TYPE link_write(
    void *context,
    const yorulink_u8_t *data,
    yorulink_u16_t length)
{
    /* 返回前完成发送，或复制到由 DMA/Port 持有的持久 Buffer。 */
    return YORULINK_ERR_OK;
}
```

接收端调用 `YORULINK_Input()` 复制字节，主循环或协议任务调用
`YORULINK_Process()`。`Input` 不会直接执行应用 Handler。

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

影响结构布局或 wire 的宏必须在所有 Translation Unit 和通信双方保持一致。

## DMA 数据生命周期

Write callback 获得完整帧，但数据指针仅在 callback 返回前有效。DMA Port 必须
先复制到持久 TX Queue/Buffer，再启动异步发送。

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

## V0.1 不包含

加密、认证、分片、自动重试、动态注册、多节点寻址和自动结构体序列化。

## License

Yorulink 使用 [MIT License](LICENSE)，与其他 Yoru Series 库保持一致。
