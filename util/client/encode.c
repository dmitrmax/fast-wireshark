/*
 * This file is part of FAST Wireshark.
 *
 * FAST Wireshark is free software: you can redistribute it and/or modify
 * it under the terms of the Lesser GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * FAST Wireshark is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * Lesser GNU General Public License for more details.
 * 
 * You should have received a copy of the Lesser GNU General Public License
 * along with FAST Wireshark.  If not, see 
 * <http://www.gnu.org/licenses/lgpl.txt>.
 */
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
    encode_uint64 (x, arr);
}

void encode_int32 (gint32 x, GByteArray** arr)
{
    encode_int64 (x, arr);
}

void encode_uint64 (guint64 x, GByteArray** arr)
{
    guint8 buf[17];
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

void encode_int64 (gint64 x, GByteArray** arr)
{
    guint8 buf[17];
    guint maxc = 9;
    guint i = maxc;

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

void encode_ascii (const guint8* str, GByteArray** arr)
{
    guint len = strlen ((char*) str);
    guint8 b;

    if (0 == len)
    {
        fputs ("Zero length string not allowed!\n", stderr);
        exit (1); /* Die fast */
    }

    b = str[len -1] | (1 << 7);

    *arr = g_byte_array_append (*arr, str, len-1);
    *arr = g_byte_array_append (*arr, &b, 1);
}

void encode_bytevec (const guint8* str, GByteArray** arr)
{
    guint len = (strlen ((char*) str) +1)/2; /* ceil(length/2) */
    encode_uint32 (len, arr);
    encode_hex (str, arr);
}

void encode_hex (const guint8* str, GByteArray** arr)
{
    guint len = strlen ((char*) str);
    guint8 b = 0;
    guint shf = 8;
    guint i;

    for (i = 0; i < len; ++i)
    {
        guint8 c = toupper (str[i]);

        if      ('0' <= c && c <= '9')  c -= '0';
        else if ('A' <= c && c <= 'F')  c -= ('A' - 10);
        else continue; /* Skip garbage */

        shf -= 4;
        b |= (c << shf);

        if (0 == shf)
        {
            *arr = g_byte_array_append (*arr, &b, 1);
            shf = 8;
            b = 0;
        }
    }

    if (8 != shf)
    {
        fprintf (stderr, "WARNING uneven hex count: %s\n", str);
        *arr = g_byte_array_append (*arr, &b, 1);
    }
}

void encode_bit (const guint8* str, GByteArray** arr)
{
    guint len = strlen ((char*) str);
    guint8 b = 0;
    guint shf = 8;
    guint i;

    for (i = 0; i < len; ++i)
    {
        guint8 c = toupper (str[i]);

        if ('0' == c || '1' == c)  c -= '0';
        else continue; /* Skip garbage */

        shf -= 1;
        b |= (c << shf);

        if (0 == shf)
        {
            *arr = g_byte_array_append (*arr, &b, 1);
            shf = 8;
            b = 0;
        }
    }

    if (8 != shf)
    {
        fprintf (stderr, "WARNING uneven bit count: %s\n", str);
        *arr = g_byte_array_append (*arr, &b, 1);
    }
}

