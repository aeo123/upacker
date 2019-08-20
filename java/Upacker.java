package ltd.zlink.ae_tracer.msg;

public class Upacker {

    // 最长消息长度
    private final int MAX_PACK_SIZE = 5012;
    private final byte STX_L = 0X55;

    private final int RET_FAILED = -1;
    private final int RET_SUCCESS = 0;
    private final int RET_PROCESS = 1;

    //cmd数据
    private byte[] data;

    // cmd长度
    private int flen;

    // cmd 校验
    private int check;

    // cmd 校验计算值
    private int calc;

    // cmd 解析状态
    private int state;

    // cmd数据接收cnt
    private int cnt;

    private MsgCallback msgCallback;

    Upacker(final MsgCallback cb) {
        msgCallback = cb;
    }


    /**
     * 解析封包
     *
     * @param buff
     */
    public void unpack(byte[] buff) {
        int ret = 0;
        for (byte i : buff) {
            ret = frameDecode(i);
            if (ret == RET_SUCCESS) {
                msgCallback.onMsgPrased(data, flen);
            } else if (ret == RET_FAILED) {
                msgCallback.onMsgFailed();
            }
        }
    }

    private int frameDecode(byte d) {

        if (state == 0 && d == STX_L) {
            state = 1;
            calc = STX_L;
        } else if (state == 1) {
            flen = d & 0xff;
            calc ^= d & 0xff;
            state = 2;
        } else if (state == 2) {
            flen |= (d & 0xff) << 8;
            calc ^= d & 0x3F;
            // 数据包超长得情况下直接丢包
            if ((flen & 0x3FFF) > MAX_PACK_SIZE) {
                state = 0;

                return RET_FAILED;
            } else {
                data = new byte[flen & 0x3FFF];

            }
            state = 3;
            cnt = 0;
        } else if (state == 3) {
            int header_crc = ((d & 0x03) << 4) | ((flen & 0xC000) >> 12);
            check = d;
            if (header_crc != (calc & 0X3C)) {
                state = 0;

                return RET_FAILED;
            }
            state = 4;
            flen &= 0x3FFF;
        } else if (state == 4) {
            data[cnt++] = d;
            calc ^= d;
            if (cnt == flen) {
                state = 0;
                //接收完，检查check
                if ((calc & 0xFC) == (check & 0XFC)) {

                    return RET_SUCCESS;
                } else {
                    return RET_FAILED;
                }
            }
        } else {
            state = 0;
        }
        return RET_PROCESS;
    }

    /**
     * 打包数据，静态方法
     *
     * @param data
     * @return
     */
    public static byte[] frameEncode(byte[] data) {

        byte tmp[] = new byte[4 + data.length];
        int crc = 0;
        tmp[0] = 0x55;
        tmp[1] = (byte) (data.length & 0xff);
        tmp[2] = (byte) ((data.length >> 8) & 0xff);
        crc = tmp[0] ^ tmp[1] ^ tmp[2];
        tmp[2] |= (byte) ((crc & 0x0C) << 4);
        tmp[3] = (byte) (0x03 & (crc >> 4));

        for (int i = 0; i < data.length; i++) {
            crc ^= data[i];
        }
        tmp[3] |= (crc & 0xfc);
        System.arraycopy(data, 0, tmp, 4, data.length);

        return tmp;
    }
}
