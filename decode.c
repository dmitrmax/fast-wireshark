/**
	@file	decode.c
	@brief	low-level bit decoding routines
	@author	Wes Fournier

	|
*/

#include "decode.h"

#define STOP_BIT 0x80
#define SIGN_BIT 0x40

gint count_encoded_bytes(tvbuff_t* buf, guint off)
{
	if(!buf) return ERR_BADARG;

	int i=0;
	guint8 b=0;
	do
	{
		// will throw an exception if the data is malformed
		b=tvb_get_guint8(buf,off+i);
		i++;
	} while(!(b&STOP_BIT));

	return i;
}

gint decode_uint32(
	tvbuff_t* buf,
	guint off,
	guint32* out)
{
	if(!buf) return ERR_BADARG;
	if(!out) return ERR_BADARG;

	guint32 ret=0;

	int i=0;
	guint8 b=0;

	do
	{
		ret<<=7;
		b=tvb_get_guint8(buf,off+i);

		ret |= b & ~STOP_BIT;

		i++;
	} while(!(b&STOP_BIT));

	if(i>sizeof(guint32)) return ERR_BADFMT;

	*out = ret;

	return i;
}

gint decode_int32(
	tvbuff_t* buf,
	guint off,
	gint32* out)
{
	if(!buf) return ERR_BADARG;
	if(!out) return ERR_BADARG;

	gint32 ret=0;
	int i=1;

	guint8 b=tvb_get_guint8(buf,off);
	int sign=b&SIGN_BIT;
	ret|=b&~(STOP_BIT|SIGN_BIT);

	if(!(b&STOP_BIT))
	{
		do
		{
			b=tvb_get_guint8(buf,off+i);
			ret<<=7;
			ret|=b & ~STOP_BIT;
			i++;
		} while(!(b&STOP_BIT));
	}

	if(i>sizeof(gint32)) return ERR_BADFMT;

		/* Changed to not convert from bigendian,
		 * whole routine is still untested.
		 * -- grencez Sat Feb 27 16:06:16 EST 2010
		 */
	if (sign) ret = -ret;

	*out = ret;
	return i;
}

gint decode_uint64(
	tvbuff_t* buf,
	guint off,
	guint64* out)
{
	if(!buf) return ERR_BADARG;
	if(!out) return ERR_BADARG;

	guint64 ret=0;

	int i=0;
	guint8 b=0;
	do
	{
		b=tvb_get_guint8(buf,off+i);

		ret<<=7;
		ret |= b & ~STOP_BIT;

		i++;
	} while(!(b&STOP_BIT));

	if(i>sizeof(guint64)) return ERR_BADFMT;

	*out=ret;

	return i;
}

gint decode_int64(
	tvbuff_t* buf,
	guint off,
	gint64* out)
{
	if(!buf) return ERR_BADARG;
	if(!out) return ERR_BADARG;

	gint64 ret=0;
	int i=1;

	guint8 b=tvb_get_guint8(buf,off);
	int sign=b&SIGN_BIT;
	ret|=b&~(STOP_BIT|SIGN_BIT);

	if(!(b&STOP_BIT))
	{
		do
		{
			b=tvb_get_guint8(buf,off+i);
			ret<<=7;
			ret|=b & ~STOP_BIT;
			i++;
		} while(!(b&STOP_BIT));
	}

	if(i>sizeof(gint64)) return ERR_BADFMT;

	if(sign) ret=-ret;
	*out=-ret;

	return i;
}

//NOTE this returns memory in out that must be free'd with g_free()
gint decode_ascii(
	tvbuff_t* buf,
	guint off,
	guint8** out)
{
	gint ret=count_encoded_bytes(buf,off);
	if(ret<0) return ret;

	guint8* s=tvb_get_string(buf,off,ret);
	if(!s) return ERR_NOMEM;
	int i;
	for(i=0;i<ret;i++) s[i]=s[i] & ~STOP_BIT;

	*out=s;

	return ret;
}

//NOTE this returns memory that must be free'd with g_free()
gint decode_bytes(
	tvbuff_t* buf,
	guint off,
	guint8** out)
{
	guint32 sz;
	gint ret=decode_uint32(buf,off,&sz);
	if(ret<0) return ret;

	guint8* p=tvb_memdup(buf,off,sz);
	if(!p) return ERR_NOMEM;
	*out=p;
	return sz;
}

//NOTE: this returns memory in out that must be free'd with g_free()
gint decode_utf8(
	tvbuff_t* buf,
	guint off,
	guint8** out)
{
	return decode_bytes(buf,off,out);
}

gint decode_flt10(
	tvbuff_t* buf,
	guint off,
	gint32* wholepart_out,
	gint32* decpart_out)
{
	return ERR_NOTIMPL;
}

gint decode_fixdec(
	tvbuff_t* buf,
	guint off,
	guint wholebits,
	gint32* wholepart_out,
	gint32* decpart_out)
{
	return ERR_NOTIMPL;
}

gint decode_pmap(
	tvbuff_t* buf,
	guint off,
	guint8** outbits)
{
	gint sz=count_encoded_bytes(buf,off);
	if(sz<0) return sz;

	guint8* ret=g_malloc(sz*7);
	if(!ret) return ERR_NOMEM;

	int i;
	guint8 b=0;
	for(i=0;i<sz*7;)
	{
		b=tvb_get_guint8(buf,off+i);
		ret[i++]=b&0x40;
		ret[i++]=b&0x20;
		ret[i++]=b&0x10;
		ret[i++]=b&0x08;
		ret[i++]=b&0x04;
		ret[i++]=b&0x02;
		ret[i++]=b&0x01;
	}

	*outbits=ret;

	return sz;
}
