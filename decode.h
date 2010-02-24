/**
	@file	decode.h
	@brief	|
	@author	Wes Fournier

	|
*/

#ifndef DECODE_H_INCLUDED_
#define DECODE_H_INCLUDED_

#include "fast.h"

gint count_encoded_bytes(tvbuff_t*,guint);

gint decode_int32(tvbuff_t*,guint,gint32*);
gint decode_uint32(tvbuff_t*,guint,guint32*);
gint decode_int64(tvbuff_t*,guint,gint64*);
gint decode_uint64(tvbuff_t*,guint,guint64*);

gint decode_ascii(tvbuff_t*,guint,guint8**);
gint decode_bytes(tvbuff_t*,guint,guint8**);
gint decode_utf8(tvbuff_t*,guint,guint8**);

gint decode_flt10(tvbuff_t*,guint,gint32*,gint32*);
gint decode_fixdec(tvbuff_t*,guint,guint,gint32*,gint32*);

gint decode_pmap(tvbuff_t*,guint,guint8**);

#endif
