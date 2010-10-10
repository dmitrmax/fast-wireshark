/*!
 * \file decode.c
 * \brief  Decode simple data types directly.
 */

#include <string.h>

#include "decode.h"

#define StopByte 0x80
#define NBitsInByte 8

/*! \brief  Count the number of bytes in a stop bit encoded byte array.
 * \param nbytes  Number of bytes in the array.
 * \param bytes  The actual byte array.
 * \return 0 if the byte array is overrun before a stop bit is found.
 *         Otherwise, a positive count of bytes less than /nbytes/.
 */
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


/*! \brief  Count the bits which will be decoded from a set number of bytes.
 * \return  The bit count.
 */
guint number_decoded_bits (guint nbytes)
{
  return nbytes * (NBitsInByte -1);
}


/*! \brief  Decode a pmap into a pre-allocated boolean array.
 *
 * \param nbytes  Number of bytes to decode.
 * \param bytes  Bytes to decode.
 * \param pmap_res  Return value. Preallocated.
 * \sa number_decoded_bits
 */
void decode_pmap (guint nbytes, const guint8* bytes,
                  gboolean* pmap_res)
{
  guint i;
  guint8 j;

  for (i = 0; i < nbytes; ++i) {
    for (j = 1; j < NBitsInByte; ++j) {
      guint pmap_idx;
      pmap_idx = NBitsInByte * i + j -1;
      pmap_res[pmap_idx] = bytes[i] & (StopByte >> j);
    }
  }
}


/*! \brief  Decode an unsigned 32bit integer,
 *          disregarding the first bit in each byte.
 *
 * This function assumes error checking has already occurred,
 * and truncates its results accordingly.
 *
 * \param nbytes  Number of bytes to decode.
 * \param bytes  Bytes to decode.
 * \return  The decoded uInt32.
 */
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


/*! \brief  Decode an unsigned 64bit integer,
 *          disregarding the first bit in each byte.
 *
 * This function assumes error checking has already occurred,
 * and truncates its results accordingly.
 *
 * \param nbytes  Number of bytes to decode.
 * \param bytes  Bytes to decode.
 * \return  The decoded uInt64.
 */
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


/*! \brief  Decode a signed 32bit integer,
 *          disregarding the first bit in each byte.
 *
 * This function assumes error checking has already occurred,
 * and truncates its results accordingly.
 *
 * \param nbytes  Number of bytes to decode.
 * \param bytes  Bytes to decode.
 * \return  The decoded Int32.
 * \sa decode_uint32
 */
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


/*! \brief  Decode a signed 64bit integer,
 *          disregarding the first bit in each byte.
 *
 * This function assumes error checking has already occurred,
 * and truncates its results accordingly.
 *
 * \param nbytes  Number of bytes to decode.
 * \param bytes  Bytes to decode.
 * \return  The decoded Int64.
 * \sa decode_uint64
 */
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


/*! \brief  Decode an ASCII string.
 *
 * This function assumes error checking has already occurred,
 * and truncates its results accordingly.
 *
 * \param nbytes  Number of bytes to decode.
 * \param bytes  Bytes to decode.
 * \param str  Return value. Must be preallocated to at least /nbytes/.
 */
void decode_ascii_string (guint nbytes, const guint8* bytes,
                          guint8* str)
{
  memcpy (str, bytes, nbytes*sizeof(guint8));
  str[nbytes-1] &= ~StopByte;
}


/*! \brief  Decode a byte vector.
 *
 * This function assumes error checking has already occurred,
 * and truncates its results accordingly.
 *
 * \param nbytes  Number of bytes to decode.
 * \param bytes  Bytes to decode.
 * \param vec  Return value. Must be preallocated to at least /nbytes/.
 */
void decode_byte_vector (guint nbytes, const guint8* bytes,
                         guint8* vec)
{
  memcpy (vec, bytes, nbytes*sizeof(guint8));
}

