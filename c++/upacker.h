#ifndef _DRV_PACKER_H_
#define _DRV_PACKER_H_

#include "stdint.h"  
#include <functional>

using namespace std;

#define USE_DYNAMIC_MEM     0           //使用动态内存

#if USE_DYNAMIC_MEM
#define UP_MALLOC
#define UP_FREE
#endif

#define MAX_PACK_SIZE       16384       //最长消息长度,最大可用14位即16384
#define STX_L               0X55        //数据包头

typedef std::function<void(uint8_t *, uint16_t)> PACKER_CB;


class Upacker
{


public:
#if !USE_DYNAMIC_MEM
    uint8_t data[MAX_PACK_SIZE];    //用来做payload序列化的内存
#else
    uint8_t *data;                      //用来做payload序列化的内存
#endif
    uint16_t    flen;                      //frame长度
    uint8_t     calc;                      //frame校验计算值
    uint8_t     check;                     //frame校验值
    uint8_t     state;                     //frame解析状态
    uint16_t    cnt;                       //frame数据接收cnt

    PACKER_CB   cb;    //frame数据接收cnt
    PACKER_CB   send; //数据发送回调


    int  upacker_init(PACKER_CB handler, PACKER_CB s);
    void upacker_pack(uint8_t *buff, uint16_t size);
    void upacker_unpack(uint8_t *buff, uint16_t size);
private:
    uint8_t frame_decode(uint8_t d);
    uint8_t frame_encode(uint8_t *data, uint16_t size);
} ;





#endif
