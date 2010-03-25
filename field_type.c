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
	return field_decode_bytes(out,buf,off);
}

gint field_decode_bytes(field_value* out, tvbuff_t* buf, guint off)
{
	gint ret=decode_bytes(buf,off,&(out->str.p));
	if(ret<0)
	{
		DBG0("bad input data format");
		return ERR_BADFMT;
	}
	out->str.len=ret;
	return ret;
}

gint field_decode_flt10(field_value* out, tvbuff_t* buf, guint off)
{
	return ERR_NOTIMPL;
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
		0,
		sizeof(guint32),
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
		0,
		sizeof(gint32),
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
		0,
		sizeof(guint64),
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
		0,
		strlen((const char*)f->value.str.p),
		(const char*)f->value.str.p);
	return 0;
}

gint field_display_bytes(
	struct template_field_type* f,
	proto_tree* tree,
	tvbuff_t* buf)
{
	proto_tree_add_bytes(
		tree,
		f->hf_id,
		buf,
		0,
		f->value.str.len,
		f->value.str.p);
	return 0;
}

gint field_display_utf8(
	struct template_field_type* f,
	proto_tree* tree,
	tvbuff_t* buf)
{
	DBG0("not implemented");
	return ERR_NOTIMPL;
}

gint field_display_flt10(
	struct template_field_type* f,
	proto_tree* tree,
	tvbuff_t* buf)
{
	DBG0("not implemented");
	return ERR_NOTIMPL;
}

gint field_display_fixdec(
	struct template_field_type* f,
	proto_tree* tree,
	tvbuff_t* buf)
{
	DBG0("not implemented");
	return ERR_NOTIMPL;
}
