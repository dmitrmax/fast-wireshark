/**
	@file	field_type.c
	@brief	field decoding and display
	@author	Wes Fournier

	Field decoding and display functions, of which we must have a
	different one for each type of field
*/

#include "template.h"
#include "decode.h"

gint field_decode_uint32(field_value* out, tvbuff_t* buf, guint off)
{
	return decode_uint32(buf,off,&(out->u32));
}

gint field_decode_int32(field_value* out, tvbuff_t* buf, guint off)
{
	return decode_int32(buf,off,&(out->i32));
}

gint field_decode_uint64(field_value* out, tvbuff_t* buf, guint off)
{
	return decode_uint64(buf,off,&(out->u64));
}

gint field_decode_int64(field_value* out, tvbuff_t* buf, guint off)
{
	return decode_int64(buf,off,&(out->i64));
}

gint field_decode_ascii(field_value* out, tvbuff_t* buf, guint off)
{
	return decode_ascii(buf,off,&(out->str.p));
}

gint field_decode_utf8(field_value* out, tvbuff_t* buf, guint off)
{
	return decode_utf8(buf,off,&out->str.p,&out->str.len);
}

gint field_decode_bytes(field_value* out, tvbuff_t* buf, guint off)
{
	gint ret;
	ret = decode_bytes (buf, off, &out->str.p, &out->str.len);
	if(ret<0)
	{
		DBG0("bad input data format");
		return ERR_BADFMT;
	}
	return ret;
}

gint field_decode_flt10(field_value* out, tvbuff_t* buf, guint off)
{
    return decode_flt10 (buf, off, &out->flt10.mant, &out->flt10.exp);
}

gint field_decode_fixdec(field_value* out, tvbuff_t* buf, guint off)
{
	return ERR_NOTIMPL;
}

/****************************************************************************/

gint field_display_uint32(
	struct template_field_type* f,
	proto_tree* tree,
	tvbuff_t* buf)
{
	proto_tree_add_uint(
		tree,
		f->hf_id,
		buf,
		f->offset,
		f->length,
		f->value.u32);
	return 0;
}

gint field_display_int32(
	struct template_field_type* f,
	proto_tree* tree,
	tvbuff_t* buf)
{
	proto_tree_add_int(
		tree,
		f->hf_id,
		buf,
		f->offset,
		f->length,
		f->value.i32);
	return 0;
}

gint field_display_uint64(
	struct template_field_type* f,
	proto_tree* tree,
	tvbuff_t* buf)
{
	proto_tree_add_uint64(
		tree,
		f->hf_id,
		buf,
		f->offset,
		f->length,
		f->value.u64);
	return 0;
}

gint field_display_int64(
	struct template_field_type* f,
	proto_tree* tree,
	tvbuff_t* buf)
{
	proto_tree_add_int64(
		tree,
		f->hf_id,
		buf,
		0,
		sizeof(gint64),
		f->value.i64);
	return 0;
}

gint field_display_ascii(
	struct template_field_type* f,
	proto_tree* tree,
	tvbuff_t* buf)
{
	proto_tree_add_string(
		tree,
		f->hf_id,
		buf,
		f->offset,
		f->length,
		(const char*)f->value.str.p);
	return 0;
}

gint field_display_bytes(
	struct template_field_type* f,
	proto_tree* tree,
	tvbuff_t* buf)
{
	int i;
	char* hexstring = (char*) ep_alloc 
		(2*f->value.str.len + sizeof (char));

	for (i = 0; i < f->value.str.len; ++i)
	{
		sprintf (hexstring + 2*i,
                 "%2x",
                 * ((guint8*) (&f->value.str.p[i])));
	}
	hexstring[2*i] = 0;

	proto_tree_add_string(
		tree,
		f->hf_id,
		buf,
		f->offset,
		f->length,
		hexstring);
	return 0;
}

gint field_display_utf8(
	struct template_field_type* f,
	proto_tree* tree,
	tvbuff_t* buf)
{
	char* str = (char*) ep_alloc (f->value.str.len+sizeof(char));
	memcpy (str, f->value.str.p, f->value.str.len);
	str[f->value.str.len] = 0;
	proto_tree_add_string(
		tree,
		f->hf_id,
		buf,
		f->offset,
		f->length,
		str);
	return 0;
}

gint field_display_flt10(
	struct template_field_type* f,
	proto_tree* tree,
	tvbuff_t* buf)
{
	char* str = (char*) ep_alloc (30 * sizeof (char));

    g_snprintf (str, 30, "%de%d", f->value.flt10.mant,
                f->value.flt10.exp);

	proto_tree_add_string(
		tree,
		f->hf_id,
		buf,
		f->offset,
		f->length,
		str);
	return 0;
}

gint field_display_fixdec(
	struct template_field_type* f,
	proto_tree* tree,
	tvbuff_t* buf)
{
	DBG0("not implemented");
	return ERR_NOTIMPL;
}
