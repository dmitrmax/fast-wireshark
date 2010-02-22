/**
	@file	decode.h
	@brief	|
	@author	Wes Fournier

	|
*/

#ifndef DECODE_H_INCLUDED_
#define DECODE_H_INCLUDED_

#include "fast.h"

gint count_encoded_bytes(const guint8*,guint);

gint decode_int16(const guint8*,guint,gint16*);
gint decode_uint16(const guint8*,guint,guint16*);
gint decode_int32(const guint8*,guint,gint32*);
gint decode_uint32(const guint8*,guint,guint32*);
gint decode_int64(const guint8*,guint,gint64*);
gint decode_uint64(const guint8*,guint,guint64*);

gint decode_ascii(const guint8*,guint,gint8*,guint);
gint decode_bytes(const guint8*,guint,guint8*,guint);
gint decode_utf8(const guint8*,guint,guint8*,guint);

gint decode_flt10(const guint8*,guint,gint32*,gint32*);
gint decode_fixdec(const guint8*,guint,guint,gint32*,gint32*);

#endif
