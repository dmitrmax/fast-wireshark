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
	int i=0;
	guint8 b=0;
	if(!buf)
	{
		DBG0("null argument");
		return ERR_BADARG;
	}

	do
	{
		/*  will throw an exception if the data is malformed */
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
	guint32 ret=0;

	int i=0;
	guint8 b=0;

	if(!buf || !out)
	{
		DBG0("null argument");
	}

	do
	{
		ret<<=7;
		b=tvb_get_guint8(buf,off+i);

		ret |= b & ~STOP_BIT;

		i++;
	} while(!(b&STOP_BIT));

	if(i>sizeof(guint32))
	{
		DBG0("bad input data format");
		return ERR_BADFMT;
	}

	*out = ret;

	return i;
}

gint decode_int32(
	tvbuff_t* buf,
	guint off,
	gint32* out)
{
	gint32 ret=0;
	int i=0;
	guint8 b;

	if(!buf || !out)
	{
		DBG0("null argument");
		return ERR_BADARG;
	}

	do
	{
		b = tvb_get_guint8 (buf, off +i);
		ret = (ret << 7) | (b & 0x7F); /* Fill in next 7 bits */
		++ i;
	} while (! (b & STOP_BIT));

	{
		int shf = 32 - 7*i;
		ret = (ret << shf) >> shf; /* Sign extend */
	}

	*out = ret;
	return i;
}

gint decode_uint64(
	tvbuff_t* buf,
	guint off,
	guint64* out)
{
	guint64 ret=0;

	int i=0;
	guint8 b=0;

	if(!buf || !out)
	{
		DBG0("null argument");
	}

	do
	{
		ret<<=7;
		b=tvb_get_guint8(buf,off+i);

		ret |= b & ~STOP_BIT;

		i++;
	} while(!(b&STOP_BIT));

	if(i>sizeof(guint64))
	{
		DBG0("bad input data format");
		return ERR_BADFMT;
	}

	*out = ret;

	return i;
}

gint decode_int64(
	tvbuff_t* buf,
	guint off,
	gint64* out)
{
	gint64 ret=0;
	int i=0;
	guint8 b;

	if(!buf || !out)
	{
		DBG0("null argument");
		return ERR_BADARG;
	}

	do
	{
		b = tvb_get_guint8 (buf, off +i);
		ret = (ret << 7) | (b & 0x7F); /* Fill in next 7 bits */
		++ i;
	} while (! (b & STOP_BIT));

	{
		int shf = 32 - 7*i;
		ret = (ret << shf) >> shf; /* Sign extend */
	}

	*out = ret;
	return i;
}

/* NOTE this returns memory in out that must be free'd with g_free() */
gint decode_ascii(
	tvbuff_t* buf,
	guint off,
	guint8** out)
{
	gint ret;
	guint8* s;
	int i;

	ret = count_encoded_bytes(buf,off);
	if(ret<0) return ret;

	s=tvb_get_string(buf,off,ret);
	if(!s)
	{
		DBG0("out of memory");
		return ERR_NOMEM;
	}
	for(i=0;i<ret;i++) s[i]=s[i] & ~STOP_BIT;

	*out=s;

	return ret;
}

/* NOTE this returns memory that must be free'd with g_free() */
gint decode_bytes(
	tvbuff_t* buf,
    guint off,
    guint8** out,
    guint32* sz)
{
    gint ret;
    guint8* p;

    ret = decode_uint32(buf,off,sz);
    if(ret<0) return ret;

    p=tvb_memdup(buf,off+ret,*sz);
    if(!p)
    {
        DBG0("out of memory");
        return ERR_NOMEM;
    }
    *out=p;
    return ret + *sz;
}

/* NOTE: this returns memory in out that must be free'd with g_free() */
gint decode_utf8(
	tvbuff_t* buf,
	guint off,
	guint8** out,
    guint32* sz)
{
	return decode_bytes(buf,off,out,sz);
}

gint decode_flt10(
	tvbuff_t* buf,
	guint off,
	gint32* coeff,
	gint32* exponent)
{
	int ret;
	const guint orig_off = off;
	ret = decode_int32 (buf, off, exponent);
		/* printf ("exponent: %d\n", *exponent); */
	if (ret < 0)  return ret;
	off += ret;
	ret = decode_int32 (buf, off, coeff);
		/* printf ("coeff: %d\n", *coeff); */
	if (ret < 0)  return ret;
	off += ret;
	return off - orig_off;
}

/*gint decode_fixdec(
	tvbuff_t* buf,
	guint off,
	guint wholebits,
	gint32* wholepart_out,
	gint32* decpart_out)
{
	DBG0("not implemented");
	return ERR_NOTIMPL;
}*/

gint decode_pmap(
	tvbuff_t* buf,
	guint off,
	guint8** outbits)
{
	gint sz;
	guint8* ret;
	int i;

	sz=count_encoded_bytes(buf,off);
	if(sz<0) return sz;

	ret=g_malloc(sz*7);
	if(!ret)
	{
		DBG0("out of memory");
		return ERR_NOMEM;
	}

	/* Is there a reason this was being set to zero? -- grn */
	/* guint8 b=0; */

	for(i=0;i<sz*7;)
	{
		guint8 b;
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

gint decode_check_null(
	tvbuff_t* buf,
	guint off)
{
	guint8 b=tvb_get_guint8(buf,off);
	if(b==0x80) return 1;
	return 0;
}
