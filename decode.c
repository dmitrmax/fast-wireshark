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

/*!
 * \file decode.c
 * \brief  Decode simple data types directly.
 */

#include <string.h>

#include "decode.h"

/*!
 * \brief Determines which bit is the top bit, the leading bit of the byte (left most)
 */
#define StopByte 0x80

/*!
 * \brief Number of bits in a byte
 */
#define NBitsInByte 8

guint count_stop_bit_encoded (guint nbytes, const guint8* bytes)
{
  guint i;

  for (i = 0; i < nbytes; ++i) {
    if (bytes[i] & StopByte) {
      return i+1;
    }
  }

  return 0;
}

guint number_decoded_bits (guint nbytes)
{
  return nbytes * (NBitsInByte -1);
}

void decode_pmap (guint nbytes, const guint8* bytes,
                  gboolean* pmap_res)
{
  guint i;
  guint8 j;

  for (i = 0; i < nbytes; ++i) {
    for (j = 1; j < NBitsInByte; ++j) {
      guint pmap_idx;
      pmap_idx = (NBitsInByte-1) * i + j -1;
      pmap_res[pmap_idx] = bytes[i] & (StopByte >> j);
    }
  }
}

guint32 decode_uint32 (guint nbytes, const guint8* bytes)
{
  guint32 n = 0;
  guint i;
  if (0 == nbytes)  return 0;

  for (i = 0; i < nbytes -1; ++i) {
    n |= bytes[i];  /* Put in lower bits. */
    n <<= NBitsInByte -1;  /* Scoot over for next data. */
  }
  /* (i == nbytes -1) */
  n |= bytes[i] & ~StopByte;
  return n;
}

guint64 decode_uint64 (guint nbytes, const guint8* bytes)
{
  guint64 n = 0;
  guint i;
  if (0 == nbytes)  return 0;

  for (i = 0; i < nbytes -1; ++i) {
    n |= bytes[i];  /* Put in lower bits. */
    n <<= NBitsInByte -1;  /* Scoot over for next data. */
  }
  /* (i == nbytes -1) */
  n |= bytes[i] & ~StopByte;
  return n;
}

gint32 decode_int32 (guint nbytes, const guint8* bytes)
{
  gint32 n;
  if (0 == nbytes)  return 0;
  n = (gint32) decode_uint32 (nbytes, bytes);

  /* If we have a sign bit, */
  if (bytes[0] & (StopByte >> 1)) {
    guint nbits;
    nbits = number_decoded_bits (nbytes);
    if (nbits < 32) {
      /* sign extend /n/. */
      n |= ~(gint32)0 << nbits;
    }
  }
  return n;
}

gint64 decode_int64 (guint nbytes, const guint8* bytes)
{
  gint64 n;
  if (0 == nbytes)  return 0;
  n = (gint64) decode_uint64 (nbytes, bytes);

  /* If we have a sign bit, */
  if (bytes[0] & (StopByte >> 1)) {
    guint nbits;
    nbits = number_decoded_bits (nbytes);
    if (nbits < 64) {
      /* sign extend /n/. */
      n |= ~(gint64)0 << nbits;
    }
  }
  return n;
}

void decode_ascii_string (guint nbytes, const guint8* bytes,
                          guint8* str)
{
  if (nbytes > 0) {
    memcpy (str, bytes, nbytes*sizeof(guint8));
    str[nbytes-1] &= ~StopByte;
  }
}

void decode_byte_vector (guint nbytes, const guint8* bytes,
                         guint8* vec)
{
  memcpy (vec, bytes, nbytes*sizeof(guint8));
}

