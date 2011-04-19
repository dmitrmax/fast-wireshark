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
 * \file  decode.h
 * \brief  Lowest level field decoding functions.
 */

#ifndef DECODE_H_INCLUDED_
#define DECODE_H_INCLUDED_

#include <glib.h>

/*! \brief  Count the number of bytes in a stop bit encoded byte array.
 * \param nbytes  Number of bytes in the array.
 * \param bytes  The actual byte array.
 * \return 0 if the byte array is overrun before a stop bit is found.
 *         Otherwise, a positive count of bytes less than /nbytes/.
 */
guint count_stop_bit_encoded (guint nbytes, const guint8* bytes);

/*! \brief  Count the bits which will be decoded from a set number of bytes.
 * \return  The bit count.
 */
guint number_decoded_bits (guint nbytes);

/*! \brief  Decode a pmap into a pre-allocated boolean array.
 *
 * \param nbytes  Number of bytes to decode.
 * \param bytes  Bytes to decode.
 * \param pmap_res  Return value. Preallocated.
 * \sa number_decoded_bits
 */
void decode_pmap (guint nbytes, const guint8* bytes, gboolean* pmap_res);

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
guint32 decode_uint32 (guint nbytes, const guint8* bytes);

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
guint64 decode_uint64 (guint nbytes, const guint8* bytes);

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
gint32 decode_int32 (guint nbytes, const guint8* bytes);

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
gint64 decode_int64 (guint nbytes, const guint8* bytes);

/*! \brief  Decode an ASCII string.
 *
 * This function assumes error checking has already occurred,
 * and truncates its results accordingly.
 *
 * \param nbytes  Number of bytes to decode.
 * \param bytes  Bytes to decode.
 * \param str  Return value. Must be preallocated to at least /nbytes/.
 */
void decode_ascii_string (guint nbytes, const guint8* bytes, guint8* str);

/*! \brief  Decode a byte vector.
 *
 * This function assumes error checking has already occurred,
 * and truncates its results accordingly.
 *
 * \param nbytes  Number of bytes to decode.
 * \param bytes  Bytes to decode.
 * \param vec  Return value. Must be preallocated to at least /nbytes/.
 */
void decode_byte_vector (guint nbytes, const guint8* bytes, guint8* vec);

#endif

