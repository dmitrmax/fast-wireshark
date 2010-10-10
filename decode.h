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

