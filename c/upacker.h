#ifndef _DRV_PACKER_H_
#define _DRV_PACKER_H_

#include "stdint.h"

#ifdef __cplusplus
extern "C"
{
#endif

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

        typedef upacker_inst *upacker_inst_t;

        int upacker_init(upacker_inst_t packer, PACKER_CB handle, PACKER_CB send);
        void upacker_pack(upacker_inst_t packer, uint8_t *buff, uint16_t size);
        void upacker_unpack(upacker_inst_t packer, uint8_t *buff, uint16_t size);

#ifdef __cplusplus
}
#endif
#endif
