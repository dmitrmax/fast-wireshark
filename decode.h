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

guint count_stop_bit_encoded (guint nbytes, const guint8* bytes);
guint number_decoded_bits (guint nbytes);

void decode_pmap (guint nbytes, const guint8* bytes, gboolean* pmap_res);
guint32 decode_uint32 (guint nbytes, const guint8* bytes);
guint64 decode_uint64 (guint nbytes, const guint8* bytes);
gint32 decode_int32 (guint nbytes, const guint8* bytes);
gint64 decode_int64 (guint nbytes, const guint8* bytes);
void decode_ascii_string (guint nbytes, const guint8* bytes, guint8* str);
void decode_byte_vector (guint nbytes, const guint8* bytes, guint8* vec);

#endif

