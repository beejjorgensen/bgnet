#include <stdio.h>

// Only define this if you're on a little-endian system!
#define LITTLE_ENDIAN
#define ntohll htonll

unsigned long long htonll(unsigned long long x)
{
#ifdef LITTLE_ENDIAN
    x = ((x & 0x00000000000000ffULL) << 56) |
        ((x & 0x000000000000ff00ULL) << 40) |
        ((x & 0x0000000000ff0000ULL) << 24) |
        ((x & 0x00000000ff000000ULL) << 8)  |
        ((x & 0x000000ff00000000ULL) >> 8)  |
        ((x & 0x0000ff0000000000ULL) >> 24) |
        ((x & 0x00ff000000000000ULL) >> 40) |
        ((x & 0xff00000000000000ULL) >> 56);
#endif
    return x;
}

int main(void)
{
    unsigned long long v[4] = {
        0x0000000000000000,
        0x0123456789ABCDEF,
        0xFEDCBA9876543210,
        0xFFFFFFFFFFFFFFFF,
    };

    for (int i = 0; i < 4; i++) {
        unsigned long long val = v[i];
        unsigned long long net_val = htonll(val);
        unsigned long long host_val = ntohll(net_val);

        printf("%016llX -> %016llX -> %016llX\n", val, net_val, host_val);
    }
}
