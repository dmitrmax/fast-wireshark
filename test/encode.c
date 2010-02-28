
#include "encode.h"

void encode_pmap (const GByteArray* pmap, GByteArray** pto_a)
{
    GByteArray* a = *pto_a;
    guint j;
    guint i = a->len;
    guint8 mask = 0;

    a = g_byte_array_append (a, &mask, 1);
    mask = 1 << 6;

    for (j = 0; j < pmap->len; ++j)
    {
        if (! mask)
        {
            a = g_byte_array_append (a, &mask, 1);
            mask = 1 << 6;
            ++i;
        }

        if (pmap->data[j])
            a->data[i] |= mask;

        mask >>= 1;
    }

    a->data[i] |= 1 << 7;
    *pto_a = a;
}

void encode_uint32 (guint32 x, GByteArray** arr)
{
    guint8 buf[9];
    size_t maxc = 9;
    int i = maxc;

    do
    {
        --i;
        buf[i] = x & 0x7f;
        x >>= 7;
    } while (0 != x);

    buf[maxc -1] |= 0x80;

    *arr = g_byte_array_append (*arr, &buf[i], maxc - i);
}

void encode_int32 (gint32 x, GByteArray** arr)
{
    guint8 buf[9];
    size_t maxc = 9;
    int i = maxc;

    while (1)
    {
        --i;
        buf[i] = x & 0x7f;
        x >>= 6;
        if (x == 0 || ~x == 0)  break;
        x >>= 1;
    }

    buf[maxc -1] |= 0x80;

    *arr = g_byte_array_append (*arr, &buf[i], maxc - i);
}

