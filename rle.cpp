#include <cstring>

int encode_rle(unsigned char *const dst, const unsigned char *const src, int size)
{
    int stat[256];

    memset(stat,0,sizeof(stat));

    for(int i = 0; i < size; ++i) ++stat[src[i]];

    int tag = -1;

    for (int i = 0; i < 256; ++i) {
        if (!stat[i]) {
            tag = i;
            break;
        }
    }

    if (tag < 0) return -1;

    int ptr = 0;
    int len = 1;
    int sym_prev = -1;

    dst[ptr++] = tag;

    for(int i = 0; i < size; ++i) {
        int sym = src[i];

        if (sym_prev != sym || len >= 255 || i == size - 1) {
            if (len > 1) {
                if (len == 2) {
                    dst[ptr++] = sym_prev;
                } else {
                    dst[ptr++] = tag;
                    dst[ptr++] = len - 1;
                }
            }
            dst[ptr++]=sym;
            sym_prev=sym;
            len=1;
        } else {
            ++len;
        }
    }

    dst[ptr++] = tag;	//end of file marked with zero length rle
    dst[ptr++] = 0;

    return ptr;
}
