#include "fast.h"

#define BIG_ENDIAN          0
#define LITTLE_ENDIAN       1

int isBigEndian(void)
{
    union {
        short msb;
        char lsb[sizeof(short)];
    } ss;
    ss.msb = 0x0102;
    if(sizeof(short) == 2) {
        if(ss.lsb[0] == 0x01 && ss.lsb[1] == 0x02)
            return 0; // Big endian
        else
            return 1;
    }
}
