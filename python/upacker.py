import struct


class Upacker():
    def __init__(self):
        self._STX_L = 0x55
        self._MAX_PACK_SIZE = 1024 + 4 + 128
        self._calc = 0
        self._check = 0
        self._cnt = 0
        self._flen = 0
        self._state = 0
        self._data = bytearray()

    def _decode(self, d):
        if (self._state == 0 and d == self._STX_L):
            self._state = 1
            self._calc = self._STX_L
        elif self._state == 1:
            self._flen = d & 0xff
            self._calc ^= d & 0xff
            self._state = 2
        elif self._state == 2:
            self._flen |= (d & 0xff) << 8
            self._calc ^= d & 0x3F
            # 数据包超长得情况下直接丢包
            if ((self._flen & 0x3FFF) > self._MAX_PACK_SIZE):
                self._state = 0
                return -1
            else:
                self._data = bytearray(self._flen & 0x3FFF)
            self._state = 3
            self._cnt = 0
        elif self._state == 3:
            header_crc = ((d & 0x03) << 4) | ((self._flen & 0xC000) >> 12)
            self._check = d
            if (header_crc != (self._calc & 0X3C)):
                self._state = 0
                return -1
            self._state = 4
            self._flen &= 0x3FFF
        elif self._state == 4:
            self._data[self._cnt] = d
            self._cnt += 1
            self._calc ^= d
            if self._cnt == self._flen:
                self._state = 0
                #接收完，检查check
                if ((self._calc & 0xFC) == (self._check & 0XFC)):
                    return 0
                else:
                    return -1
        else:
            self._state = 0

        return 1

    # 解包
    def unpack(self, bytes_data, callback):
        ret = 0
        for i in bytes_data:
            ret = self._decode(i)
            if ret == 0:
                callback(self._data)
                # print(self._data)
            elif ret == -1:
                # callback(None)
                print("err")

    # 打包
    def enpack(self, data):
        tmp = bytearray(4)
        tmp[0] = 0x55
        tmp[1] = len(data) & 0xff
        tmp[2] = (len(data) >> 8) & 0xff
        crc = tmp[0] ^ tmp[1] ^ tmp[2]
        tmp[2] |= (crc & 0x0c) << 4
        tmp[3] = 0x03 & (crc >> 4)

        for i in range(len(data)):
            crc ^= data[i]
        tmp[3] |= (crc & 0xfc)

        frame = struct.pack("BBBB%ds" % len(data), tmp[0], tmp[1], tmp[2],
                            tmp[3], data)
        print(frame.hex())
        return frame


def print_hex(bytes):
    hex_byte = [hex(i) for i in bytes]
    print(" ".join(hex_byte))


if __name__ == '__main__':
    buf = bytearray([0x00, 0x01, 0x02,0x03])
    pack = Upacker()
    pkt = pack.enpack(buf)
    pack.unpack(pkt, print_hex)
    
