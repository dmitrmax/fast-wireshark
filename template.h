/**
	@file	template.h
	@brief	protocol template structures
	@author	Wes Fournier

	Here we define most of the structures and information involved
	in defining a FAST protocol template.  These structures are used
	to store the different static template information, as well as
	the structure that stores the current state of the protocol
*/

#ifndef TEMPLATE_H_INCLUDED_
#define TEMPLATE_H_INCLUDED_

#include "fast.h"

struct template_field_type;

struct template_type
{
	char* name; /* name of this template */
	guint8 id; /* numeric ID of this template sent in packets */

	/* list of fields in this template */
	struct template_field_type* fields;

	/* next entry in template list */
	struct template_type* next;
};

void init_templates(void);

gint create_template(
	const char*,
	guint32,
	struct template_type**);

gint reset_template_state(struct template_type*);

gint find_template(const char*, struct template_type**);
gint find_template_byid(guint8,struct template_type**);

/****************************************************************************/

typedef union field_value_type
{
	/* integral types */
	guint32 u32;
	gint32 i32;
	guint64 u64;
	gint64 i64;

	/* ascii, utf8, and byte vector fields */
	struct string_value_type_
	{
		guint32 len; /* ignored for ascii fields */
		guint8* p;
	} str;

	/*struct fixdecimal_value_type_
	{
		gint32 whole;
		guint32 frac;
	} fixdec;*/

	struct base10_float_value_type_
	{
		gint32 mant; /* decimal mantissa */
		gint32 exp; /* base-10 exponent */
	} flt10;

} field_value;

gint copy_field_value(guint type, field_value* src, field_value* dest);

/****************************************************************************/

#define FIELD_TYPE_INT32	0x01	/*  32-bit int */
#define FIELD_TYPE_UINT32	0x02	/*  unsigned 32-bit int */
#define FIELD_TYPE_INT64	0x03	/*  64-bit int */
#define FIELD_TYPE_UINT64	0x04	/*  unsigned 64-bit int */
#define FIELD_TYPE_FLT10	0x05	/*  base-10 floating point */
/*#define FIELD_TYPE_FIXDEC	0x06*/	/*  fixed-point decimal */
#define FIELD_TYPE_UTF8		0x07	/*  UNICODE-8 text */
#define FIELD_TYPE_ASCII	0x08	/*  ASCII text */
#define FIELD_TYPE_BYTES	0x09	/*  byte vector */
#define FIELD_TYPE_SEQ		0x0a	/*  sequence */
#define FIELD_TYPE_GROUP	0x0b	/*  group */

#define FIELD_TYPE_MASK		0xf0

#define FIELD_OP_NONE		0x00
#define FIELD_OP_CONST		0x10	/* constant */
#define FIELD_OP_DEFAULT	0x20	/* default */
#define FIELD_OP_COPY		0x30	/* copy */
#define FIELD_OP_INCR		0x40	/* increment */
#define FIELD_OP_DELTA		0x50	/* delta */
#define FIELD_OP_TAIL		0x60	/* tail */

#define FIELD_STATE_SET		0x01	/* value was sent */
#define FIELD_STATE_NULL	0x02	/* a null was sent */
#define FIELD_STATE_EMPTY	0x03	/* no data was sent */
#define FIELD_STATE_UNDEF	0x0		/* undefined */

#define FIELD_DISPLAY_BIT	0x10	/* should the field display in UI? */

#define FIELD_RAW_NULL		0x80

#define FIELD_MANDATORY_BIT	0x01
#define FIELD_NULLABLE_BIT	0x02
#define FIELD_PMAP_BIT		0x04

struct template_field_type;
typedef gint (*field_read_func_type)(
	struct template_field_type*,
	tvbuff_t*,
	guint);
typedef gint (*field_display_func_type)(
	struct template_field_type*,
	proto_tree*,
	tvbuff_t*);
typedef gint (*field_op_func_type)(
	struct template_field_type*,
	guint8**,
	tvbuff_t*,
	guint*);

struct template_field_type
{
	/*****  These fields should be set when calling add_field() *****/

	char* name; /* name of this field */

	guint8 type;	/* field data type OR'd with field operator */

	gint mandatory; /* 0=field is optional, 1=field is mandatory */

	/* configuration value used for const,default, etc. operators */
	field_value cfg_value;

	/* used to store operators if this is a decimal field */
	guint8 dec_ops;

	/* used to store initial field value, or constant value
		for const op fields */
	field_value value;

	/***** Ignore these fields when calling add_field() *****/

	/* 1=field has a nullable representation 0= it doesnt */
	gint nullable;

	/* mandatory, nullable, has pmap */
	guint8 flags;

	/* offset into current tvbuff and length of corresponding
		data in tvbuff */
    guint offset;
    guint length;

    /* current state of this field and whether to display */
	guint8 state;

	field_read_func_type decode; /* incoming data decode function */
	field_display_func_type display; /* data display function */
	field_op_func_type op; /* field operator function */

	/* used to store subfields for sequences and groups */
	struct template_field_type* subfields;

	/*  gui field ids */
	int hf_id,ett_id;

	/* next field in list */
	struct template_field_type* next;
};

#define FIELD_TYPE(f) 		((f)->type & ~FIELD_TYPE_MASK)
#define FIELD_OP(f)			((f)->type & FIELD_TYPE_MASK)

/* added to support decimal fields */
#define FIELD_DEC_EXP_OP(f)			((f)->dec_op & 0x0f)
#define FIELD_DEC_MNT_OP(f)			((f)->dec_op & 0xf0)
#define FIELD_SET_DEC_EXP_OP(f,op)	((f)->dec_op |= (op))
#define FIELD_SET_DEC_MNT_OP(f,op)	((f)->dec_op |= (op<<4))

#define FIELD_STATE(f)		((f)->state & ~FIELD_DISPLAY_BIT)
#define FIELD_DISPLAY_ON(f)	((f)->state & FIELD_DISPLAY_BIT)

#define TYPE_IS_INTEGER(t) \
	(t==FIELD_TYPE_INT32 || t==FIELD_TYPE_UINT32 || \
		t==FIELD_TYPE_INT64 || t==FIELD_TYPE_UINT64)
#define FIELD_IS_INTEGER(f) (TYPE_IS_INTEGER(FIELD_TYPE(f)))

#define TYPE_IS_DECIMAL(t) (t==FIELD_TYPE_FLT10)
#define FIELD_IS_DECIMAL(f) (TYPE_IS_DECIMAL(FIELD_TYPE(f)))

#define TYPE_IS_BYTESTR(t) \
	(t==FIELD_TYPE_ASCII || t==FIELD_TYPE_UTF8 || t==FIELD_TYPE_BYTES)
#define FIELD_IS_BYTESTR(f) (TYPE_IS_DECIMAL(FIELD_TYPE(f)))

#define TYPE_IS_COMPLEX(t) (t==FIELD_TYPE_SEQ || t==FIELD_TYPE_GROUP)
#define FIELD_IS_COMPLEX(f) (TYPE_IS_COMPLEX(FIELD_TYPE(f)))

#define FIELD_IS_MANDATORY(f)	((f)->mandatory==1)
#define FIELD_HAS_NULL(f)		((f)->nullable==1)

gint create_field(
	struct template_type*,
	struct template_field_type*,
	struct template_field_type**);
gint create_subfield(
	struct template_type*,
	struct template_field_type*,
	struct template_field_type*,
	struct template_field_type**);

gint find_template_field(
	const char*,
	struct template_type*,
	struct template_field_type**);
gint find_template_field_byindex(
	guint,
	struct template_type*,
	struct template_field_type**,
	guint);


void cleanup_all(void);

/****************************************************************************/

/* decode raw data into field_value's */

gint field_decode_uint32	(struct template_field_type*,tvbuff_t*,guint);
gint field_decode_int32		(struct template_field_type*,tvbuff_t*,guint);
gint field_decode_uint64	(struct template_field_type*,tvbuff_t*,guint);
gint field_decode_int64		(struct template_field_type*,tvbuff_t*,guint);

gint field_decode_ascii		(struct template_field_type*,tvbuff_t*,guint);
gint field_decode_utf8		(struct template_field_type*,tvbuff_t*,guint);
gint field_decode_bytes		(struct template_field_type*,tvbuff_t*,guint);

gint field_decode_flt10		(struct template_field_type*,tvbuff_t*,guint);
/*gint field_decode_fixdec	(struct template_field_type*,tvbuff_t*,guint);*/

/****************************************************************************/

/* display fields in UI (if needed) */

gint field_display_uint32	(struct template_field_type*,proto_tree*,tvbuff_t*);
gint field_display_int32	(struct template_field_type*,proto_tree*,tvbuff_t*);
gint field_display_uint64	(struct template_field_type*,proto_tree*,tvbuff_t*);
gint field_display_int64	(struct template_field_type*,proto_tree*,tvbuff_t*);

gint field_display_ascii	(struct template_field_type*,proto_tree*,tvbuff_t*);
gint field_display_utf8		(struct template_field_type*,proto_tree*,tvbuff_t*);
gint field_display_bytes	(struct template_field_type*,proto_tree*,tvbuff_t*);

gint field_display_flt10	(struct template_field_type*,proto_tree*,tvbuff_t*);
/*gint field_display_fixdec	(struct template_field_type*,proto_tree*,tvbuff_t*);
*/
/****************************************************************************/

/*  field operators */
gint field_op_noop		(struct template_field_type*,guint8**,tvbuff_t*,guint*);
gint field_op_const		(struct template_field_type*,guint8**,tvbuff_t*,guint*);
gint field_op_default	(struct template_field_type*,guint8**,tvbuff_t*,guint*);
gint field_op_copy		(struct template_field_type*,guint8**,tvbuff_t*,guint*);
gint field_op_incr		(struct template_field_type*,guint8**,tvbuff_t*,guint*);
gint field_op_delta_num	(struct template_field_type*,guint8**,tvbuff_t*,guint*);
gint field_op_delta_str	(struct template_field_type*,guint8**,tvbuff_t*,guint*);
gint field_op_tail		(struct template_field_type*,guint8**,tvbuff_t*,guint*);
gint field_op_dec		(struct template_field_type*,guint8**,tvbuff_t*,guint*);

#endif
