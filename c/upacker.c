/**
  ******************************************************************************
  * @file    drv_packer.c
  * @author  zpw
  * @version V1.0
  * @date    
  * @brief   链路层通讯协议
  ******************************************************************************
  * @attention
  *          链路层通讯协议，数据封包解包
  ******************************************************************************
  */
#include "upacker.h"

static uint8_t frame_decode(upacker_inst_t packer, uint8_t d);
static uint8_t frame_encode(upacker_inst_t packer, uint8_t *data, uint16_t size);

/**
 * @brief  使用动态内存时需要初始化
 * @note   size pack缓存的长度，大于最大的数据包长度就行,使用PACK_SIZE
            无rtos最好用静态内存,不然要改heap
 * @param  *cmd_packer: 
 * @param  *handler: 
 * @retval None
 */
int upacker_init(upacker_inst_t packer, PACKER_CB h, PACKER_CB s)
{

#if USE_DYNAMIC_MEM
    packer->data = (uint8_t *)UP_MALLOC(MAX_PACK_SIZE);
    if (!packer->data)
    {
        return -1;
    }
#endif

    packer->cb = h;
    packer->send = s;
    return 0;
}

/**
 * @brief  解包输入数据
 * @note   
 * @param  cmd_packer: 
 * @param  *buff: 
 * @param  size: 
 * @retval None
 */
void upacker_unpack(upacker_inst_t packer, uint8_t *buff, uint16_t size)
{
    for (uint16_t i = 0; i < size; i++)
    {
        if (frame_decode(packer, buff[i]))
        {
            //解析成功,回调处理
            packer->cb(packer->data, packer->flen);
        }
    }
}

/**
 * @brief  封包数据并发送
 * @note   
 * @param  *packer: 
 * @param  *buff: 
 * @param  size: 
 * @retval None
 */
void upacker_pack(upacker_inst_t packer, uint8_t *buff, uint16_t size)
{
    frame_encode(packer, buff, size);
}

static uint8_t frame_decode(upacker_inst_t packer, uint8_t d)
{

    if (packer->state == 0 && d == STX_L)
    {
        packer->state = 1;
        packer->calc = 0x55;
    }
    else if (packer->state == 1)
    {
        packer->flen = d;
        packer->calc ^= d;
        packer->state = 2;
    }
    else if (packer->state == 2)
    {
        //长度信息
        packer->flen |= (uint16_t)d << 8;
        packer->calc ^= d & 0x3F;

        //数据包超长得情况下直接丢包
        if ((packer->flen & 0x3FFF) > MAX_PACK_SIZE)
        {
            packer->state = 0;
        }
        packer->state = 3;
        packer->cnt = 0;
    }
    else if (packer->state == 3)
    {
        //header校验
        uint8_t hc = ((d & 0x03) << 4) | (packer->flen & 0xC000) >> 12;

        packer->check = d;
        if (hc != (packer->calc & 0X3C))
        {
            packer->state = 0;
            return 0;
        }
        packer->state = 4;
        packer->flen &= 0x3FFF;
    }
    else if (packer->state == 4)
    {
        packer->data[packer->cnt++] = d;
        packer->calc ^= d;

        if (packer->cnt == packer->flen)
        {
            packer->state = 0;

            //接收完，检查check
            if ((packer->calc & 0xFC) == (packer->check & 0XFC))
            {
                return 1;
            }
            else
            {
                return 0;
            }
        }
    }
    else
    {
        packer->state = 0;
    }
    return 0;
}

static uint8_t frame_encode(upacker_inst_t packer, uint8_t *data, uint16_t size)
{

    uint8_t tmp[4] = {0};
    uint8_t crc = 0;

    if (size > 16384)
    {
        return 0;
    }

    tmp[0] = 0x55;
    tmp[1] = size & 0xff;
    tmp[2] = (size >> 8) & 0x3f; //低14位用来保存size;header校验4位
    crc = tmp[0] ^ tmp[1] ^ tmp[2];
    tmp[2] |= (crc & 0x0C) << 4; //tmp[2][7:6]保存header检验[3:2]
    tmp[3] = 0x03 & (crc >> 4);  //tmp[3][1:0]保存header校验[5:4]
    for (int i = 0; i < size; i++)
    {
        crc ^= data[i];
    }

    tmp[3] |= (crc & 0xfc); //tmp[3][7:2]保存data check[7:2]
    packer->send(tmp, 4);
    packer->send(data, size);

    return 1;
}
