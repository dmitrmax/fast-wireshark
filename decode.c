/**
	@file	decode.c
	@brief	low-level bit decoding routines
	@author	Wes Fournier

	|
*/

#include "decode.h"

#define STOP_BIT 0x80
#define SIGN_BIT 0x40

gint count_encoded_bytes(const guint8* in, guint max_in_bytes)
{
	int i;
	for(i=0;i<max_in_bytes;i++)
	{
		if(in[i]&STOP_BIT)
		{
			return i+1;
		}
	}
	return -1;
}

gint decode_int16(
	const guint8* in,
	guint max_in_bytes,
	gint16* out)
{
	if(count_encoded_bytes(in,max_in_bytes)!=sizeof(gint16))
	{
		return -1;
	}

	gint16 ret;

	ret= in[0] & ~(STOP_BIT|SIGN_BIT); // drop sign as well
	ret<<=7;
	ret |= in[1] & ~STOP_BIT;

	// complement if negative
	if(in[0] & SIGN_BIT) ret = ~ret;

	*out = g_ntohs(ret);

	return sizeof(gint16);
}

gint decode_uint16(
	const guint8* in,
	guint max_in_bytes,
	guint16* out)
{
	if(count_encoded_bytes(in,max_in_bytes)!=sizeof(guint16))
	{
		return -1;
	}

	guint16 ret;

	ret= in[0] & ~(STOP_BIT);
	ret<<=7;
	ret |= in[1] & ~STOP_BIT;

	*out = g_ntohs(ret);

	return sizeof(guint16);
}

gint decode_int32(
	const guint8* in,
	guint max_in_bytes,
	gint32* out)
{
	if(count_encoded_bytes(in,max_in_bytes)!=sizeof(gint32))
	{
		return -1;
	}

	gint32 ret;

	ret= in[0] & ~(STOP_BIT|SIGN_BIT); // drop sign as well
	ret<<=7;
	ret |= in[1] & ~STOP_BIT;
	ret<<=7;
	ret |= in[2] & ~STOP_BIT;
	ret<<=7;
	ret |= in[3] & ~STOP_BIT;

	// complement if negative
	if(in[0] & SIGN_BIT) ret = ~ret;

	*out = g_ntohl(ret);

	return sizeof(gint32);
}

gint decode_uint32(
	const guint8* in,
	guint max_in_bytes,
	guint32* out)
{
	if(count_encoded_bytes(in,max_in_bytes)!=sizeof(guint32))
	{
		return -1;
	}

	guint32 ret;

	ret= in[0] & ~(STOP_BIT);
	ret<<=7;
	ret |= in[1] & ~STOP_BIT;
	ret<<=7;
	ret |= in[2] & ~STOP_BIT;
	ret<<=7;
	ret |= in[3] & ~STOP_BIT;

	*out = g_ntohl(ret);

	return sizeof(guint32);
}

gint decode_uint64(
	const guint8* in,
	guint max_in_bytes,
	guint64* out)
{
	if(count_encoded_bytes(in,max_in_bytes)!=sizeof(guint64))
	{
		return -1;
	}

	guint64 ret;

	ret = in[0] & ~STOP_BIT;
	ret<<=7;
	ret |= in[1] & ~STOP_BIT;
	ret<<=7;
	ret |= in[2] & ~STOP_BIT;
	ret<<=7;
	ret |= in[3] & ~STOP_BIT;
	ret<<=7;
	ret |= in[4] & ~STOP_BIT;
	ret<<=7;
	ret |= in[5] & ~STOP_BIT;
	ret<<=7;
	ret |= in[6] & ~STOP_BIT;
	ret<<=7;
	ret |= in[7] & ~STOP_BIT;

	*out = GUINT64_FROM_BE(ret); // glib has no g_ntoh64()

	return sizeof(guint64);
}

gint decode_int64(
	const guint8* in,
	guint max_in_bytes,
	gint64* out)
{
	if(count_encoded_bytes(in,max_in_bytes)!=sizeof(gint64))
	{
		return -1;
	}

	gint64 ret;

	ret = in[0] & ~(STOP_BIT|SIGN_BIT);
	ret<<=7;
	ret |= in[1] & ~STOP_BIT;
	ret<<=7;
	ret |= in[2] & ~STOP_BIT;
	ret<<=7;
	ret |= in[3] & ~STOP_BIT;
	ret<<=7;
	ret |= in[4] & ~STOP_BIT;
	ret<<=7;
	ret |= in[5] & ~STOP_BIT;
	ret<<=7;
	ret |= in[6] & ~STOP_BIT;
	ret<<=7;
	ret |= in[7] & ~STOP_BIT;

	if(in[0]&SIGN_BIT) ret=~ret;

	*out = GINT64_FROM_BE(ret); // glib has no g_noth64()

	return sizeof(gint64);
}

gint decode_ascii(
	const guint8* in,
	guint max_in_bytes,
	gint8* out,
	guint max_out_size)
{
	memset(out,0,max_out_size);

	int i;
	for(i=0;i<max_in_bytes && i<max_out_size-1;i++)
	{
		out[i]=in[i]& ~STOP_BIT;
		if(in[i]&STOP_BIT)
		{
			return i+1;
		}
	}

	return -1;
}

gint decode_bytes(
	const guint8* in,
	guint max_in_bytes,
	guint8* out,
	guint max_out_size)
{
	memset(out,0,max_out_size);

	guint32 sz;
	gint off=decode_uint32(in,max_in_bytes,&sz);
	if(off==-1) return -1;

	memcpy(out,in+off,sz>max_in_bytes?max_in_bytes:sz);

	return sz;
}

gint decode_utf8(
	const guint8* in,
	guint max_in_bytes,
	guint8* out,
	guint max_out_size)
{
	memset(out,0,max_out_size);

	// drop one byte for null terminator
	return decode_bytes(in,max_in_bytes,out,max_out_size-1);
}

gint decode_flt10(
	const guint8* in,
	guint max_in_bytes,
	gint32* out_wholepart,
	gint32* out_decpart)
{
	return -1;
}

gint decode_fixdec(
	const guint8* in,
	guint max_in_bytes,
	guint dec_bits,
	gint32* out_wholepart,
	gint32* out_decpart)
{
	return -1;
}
