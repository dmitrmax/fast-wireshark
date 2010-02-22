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

#define FIELD_TYPE_INT32	0x01	// 32-bit int
#define FIELD_TYPE_UINT32	0x02	// unsigned 32-bit int
#define FIELD_TYPE_INT64	0x03	// 64-bit int
#define FIELD_TYPE_UINT64	0x04	// unsigned 64-bit int
#define FIELD_TYPE_FLT10	0x05	// base-10 floating point
//#define FIELD_TYPE_FLT754	0x05	// IEEE-754 encoded float
#define FIELD_TYPE_FIXDEC	0x06	// fixed-point decimal
#define FIELD_TYPE_UTF8		0x07	// UNICODE-8 text
#define FIELD_TYPE_ASCII	0x08	// ASCII text
#define FIELD_TYPE_BYTES	0x09	// byte vector
#define FIELD_TYPE_SEQ		0x0a	// sequence
#define FIELD_TYPE_GROUP	0x0b	// group

#define FIELD_REQUIRED_BIT	0x80	// is the field required
#define FIELD_HAS_NULL_BIT	0x40	// is the field nullable

#define FIELD_TYPE_MASK		0xf0

#define FIELD_OP_CONST		0x01	// constant
#define FIELD_OP_DEFAULT	0x02	// default
#define FIELD_OP_COPY		0x03	// copy
#define FIELD_OP_INCR		0x04	// increment
#define FIELD_OP_DELTA		0x05	// delta
#define FIELD_OP_TAIL		0x06	// tail

#define FIELD_STATE_SET		0x01	// assigned
#define FIELD_STATE_EMPTY	0x02	// empty
#define FIELD_STATE_UNDEF	0x03	// undefined

#define FIELD_RAW_NULL		0x80
#define FIELD_NULL			0x0

#define FIELD_TYPE(f) 		((f)->type & ~FIELD_TYPE_MASK)
#define FIELD_REQUIRED(f)	((f)->type & FIELD_REQUIRED_BIT)
#define FIELD_HAS_NULL(f)	((f)->type & FIELD_HAS_NULL_BIT)

struct template_field_type
{
	char* name;

	guint8 type,op;

	void* def_value;
	guint32 def_value_size;

	void* value;
	void* prev_value;
	guint32 size,prev_size;

	guint8 state,prev_state;

	struct template_field_type* next;
};

typedef struct template_field_type proto_field;

struct template_type
{
	char* name;
	guint8 id;

	struct template_field_type* fields;
};

typedef struct template_type proto_template;

void init_templates(void);

gint create_template(
	const char*,
	guint32,
	struct template_type**);

gint append_field(
	const char*,
	guint8,
	guint8,
	void*,
	guint32,
	struct template_type*,
	struct template_field_type**);

void cleanup_all(void);

gint find_template(const char*, struct template_type**);
gint find_template_byid(guint8,struct template_type**);
gint find_template_field(
	const char*,
	struct template_type*,
	struct template_field_type**);
gint find_template_field_byindex(
	guint,
	struct template_type*,
	struct template_field_type**,
	guint);

#endif
