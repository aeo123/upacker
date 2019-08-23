
# Upacker 
用于段对端通讯数据封包、解包，解决各种粘包、分包问题。极简内存占用。

软件包位置： /packages/misc/upacker

## 数据帧格式

---
每包数据负载长度最长位14位16384字节。
每帧数据含4字节Header和N字节负载，包含14位数据长度，4位Header校验，6位负载校验

``` C
Header    4BYTE                                                            Load
----------------------------------------------------------------------
D0[7:0] |D1[7:0]  |D2[5:0]   |D2[7:6]         |D3[1:0]        |D3[7:2]
----------------------------------------------------------------------
包头    |包长(低8) |包长(高6) |Header校验[3:2] |Header校验[5:4] |check[7:2] |data
----------------------------------------------------------------------
0x55    |0XFF     |0X3F      |0X0C            |0X30           |0XFC       |XXXXX
```

### 使用

#### 配置

packer内部需要一段内存用于保存解析完成的包，可以配置为静态内存或者动态内存。
内存分配的长度为MAX_PACK_SIZE，根据应用需要自行调节
```C
#define USE_DYNAMIC_MEM 0
#define MAX_PACK_SIZE 1024 //最长消息长度,最大可用14位即16384
```
完整的packer结构体
```C
//使用动态内存
#define USE_DYNAMIC_MEM 0

#if USE_DYNAMIC_MEM
#define UP_MALLOC
#define UP_FREE
#endif

#define MAX_PACK_SIZE 1024 //最长消息长度,最大可用14位即16384
#define STX_L 0X55         //数据包头

typedef void (*PACKER_CB)(uint8_t *d, uint16_t s);
    
typedef struct
{
#if !USE_DYNAMIC_MEM
        uint8_t data[MAX_PACK_SIZE]; //payload的内存
#else
        uint8_t *data; //用来做payload序列化的内存
#endif
        uint16_t flen; //frame长度
        uint8_t calc;  //frame校验计算值
        uint8_t check; //frame校验值
        uint8_t state; //frame解析状态
        uint16_t cnt;  //frame数据接收cnt
    
        PACKER_CB cb;   //数据包处理回调
        PACKER_CB send; //数据发送回调
} upacker_inst;
```

实例一个packer

``` C
#include "upacker.h"
upacker_inst msg_packer;
```

#### 初始化，需要用户自行实现两个函数

``` C
void data_send(uint8_t *d, uint16_t size);       //发送数据
void handle_callback(uint8_t *d, uint16_t size); //解包成功后的处理回调
```

``` C
/**
  * @brief  串口dma发送接口
  * @note   
  * @param  *d: 
  * @param  size: 
  * @retval None
  */
static void uart_send(uint8_t *d, uint16_t size)
{
    dbuff_push(&uart3_dbuff, d, size);
}


/**
  * @brief  消息解析回调
  * @note   
  * @param  *d: 
  * @param  size: 
  * @retval None
  */
static void handle_cb(uint8_t *d, uint16_t size)
{
    //接收到payload
    rt_kprintf("pack len%d", size);
}


//init packer
upacker_init(&msg_packer, handle_cb, uart_send);
```

#### 解析数据

```C
//使用串口dma接收数据的示例
static void thread_entry(void *parameter)
{
    struct rx_msg msg;
    rt_err_t result;
    rt_uint32_t rx_length;
    static char rx_buffer[UART_QUE_LEN + 1];

    while (1)
    {
        rt_memset(&msg, 0, sizeof(msg));
        /* 从消息队列中读取消息*/
        result = rt_mq_recv(&rx_mq, &msg, sizeof(msg), 10);
        if (result == RT_EOK)
        {
            /* 从串口读取数据*/
            rx_length = rt_device_read(msg.dev, 0, rx_buffer, msg.size);
            //丢到packer解析，成功了调用callback
            upacker_unpack(&msg_packer, (uint8_t *)rx_buffer, msg.size);
        }
    }
}

```

#### 封包数据

```C
        buff[0] = 0x40;
        buff[1] = 0x40;
        upacker_pack(&msg_packer, (uint8_t *)buff, 2);
```

## 应用建议
---
最简单的协议示例，一个字节用来设置指令类型，后面接数据。

```C
#define CMD1_CODE 0X40
#define CMD2_CODE 0X41

static void handle_cb(uint8_t *d, uint16_t size)
{
    //接收到payload
    rt_kprintf("pack len%d", size);

    if(d[0] == CMD1_CODE){
        //处理功能1的数据
        rt_kprintf("cmd1 data: %d", d[1]);
    }else if(d[0] == CMD2_CODE){
        //处理功能2的数据    
        rt_kprintf("cmd2 data: %d", d[1]);
    }
}

//发送cmd1数据
void send_cmd1(){
    buff[0] = 0x40;
    buff[1] = 55;
    upacker_pack(&msg_packer, (uint8_t *)buff, 2);
}     
```
---
使用json序列化数据，把json用来pack传输，收到一帧直接反序列化

---
使用msgpack序列化数据，和json类似

---
使用protobuf序列化数据，类似

### 代码从项目工程复制出来的，有运行问题请提issue












