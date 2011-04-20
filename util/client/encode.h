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
#ifndef ENCODE_H_
#define ENCODE_H_

#include <assert.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <glib.h>

void encode_pmap (const GByteArray* pmap, GByteArray** pto_a);
void encode_uint32 (guint32 x, GByteArray** arr);
void encode_int32 (gint32 x, GByteArray** arr);
void encode_uint64 (guint64 x, GByteArray** arr);
void encode_int64 (gint64 x, GByteArray** arr);
void encode_ascii (const guint8* str, GByteArray** arr);
void encode_bytevec (const guint8* str, GByteArray** arr);

void encode_hex (const guint8* str, GByteArray** arr);
void encode_bit (const guint8* str, GByteArray** arr);

#endif

