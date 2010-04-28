/**
	@file	template.c
	@brief	|
	@author	Wes Fournier

	|
*/

#include "template.h"
#include "decode.h"

/*struct template_list_node
{
	struct template_type* p;
	struct template_list_node* next;
};*/

/*static struct template_list_node* all_templates;*/

static struct template_type* all_templates=0;

void init_templates(void)
{
	all_templates=0;
}

gint create_template(
	const char* name,
	guint32 id,
	struct template_type** out)
{
	struct template_type* p;

	if(!name) return ERR_BADARG;

	/* make sure our name or id does not already exist */
	if(find_template(name,&p)==1 || find_template_byid(id,&p)==1)
	{
		DBG0("duplicate template id");
		return ERR_BADARG;
	}

	/*  create template information struct */

	p = g_malloc(sizeof(struct template_type));
	if(!p)
	{
		DBG0("out of memory");
		return ERR_NOMEM;
	}

	p->fields=0;
	p->next=0;

	p->name = g_strdup(name);
	if(!p->name)
	{
		g_free(p);
		DBG0("out of memory");
		return ERR_NOMEM;
	}

	p->id=id;

	/* add to list */
	if(!all_templates)
	{
		all_templates=p;
	}
	else
	{
		struct template_type* cur;
		for(cur=all_templates;cur->next;cur=cur->next);

		cur->next=p;
	}

	if(out) *out=p;

	return 0;
}

gint create_field(
	struct template_type* in_template,
	struct template_field_type* params,
	struct template_field_type** outptr)
{
	struct template_field_type* field;

	if(/*!in_template ||*/ !params || !params->name)
	{
		DBG0("null argument");
		return ERR_BADARG;
	}

	DBG3("field %s type: %d op: %d",
		params->name,
		FIELD_TYPE(params),
		FIELD_OP(params));

	/* create space for our field */
	field = g_malloc(sizeof(struct template_field_type));
	if(!field)
	{
		DBG0("out of memory");
		return ERR_NOMEM;
	}
	memset(field,0,sizeof(struct template_field_type));

	/* copy name */
	{
		size_t size = (7+strlen(params->name))*sizeof(char);
		field->name = g_malloc (size);
		g_snprintf (field->name, size, "fast.%s", params->name);
	}
	if(!field->name)
	{
		g_free(field);
		DBG0("out of memory");
		return ERR_NOMEM;
	}

	/************************************************************************/

	/* operator state setup */
	switch(FIELD_OP(params))
	{
	case FIELD_OP_CONST:

		/* set current value to always be input */
		if(copy_field_value(
			FIELD_TYPE(params),
			&(params->cfg_value),
			&(field->value))<0)
		{
			g_free(field->name);
			g_free(field);
			DBG0("invalid field type");
			return ERR_BADARG;
		}

		field->op = field_op_noop;

		break;

	case FIELD_OP_DEFAULT:

		if(copy_field_value(
			FIELD_TYPE(params),
			&(params->cfg_value),
			&(field->cfg_value))<0)
		{
			g_free(field->name);
			g_free(field);
			DBG0("invalid field type");
			return ERR_BADARG;
		}

		field->op = field_op_default;
		if(params->mandatory) field->nullable=1;

		break;

	case FIELD_OP_COPY:
		field->op = field_op_copy;
		if(params->mandatory) field->nullable=1;
		break;

	case FIELD_OP_INCR:

		if(FIELD_IS_BYTESTR(params) || FIELD_TYPE(params)==FIELD_TYPE_GROUP)
		{
			g_free(field->name);
			g_free(field);
			DBG0("invalid field type");
			return ERR_BADARG;
		}

		field->op = field_op_incr;
		if(params->mandatory) field->nullable=1;
		break;

	case FIELD_OP_DELTA:

		/* delta is complicated enough we need two versions, for strings
			and integers */
		if(FIELD_IS_INTEGER(params) || FIELD_IS_DECIMAL(params) ||
			FIELD_TYPE(params)==FIELD_TYPE_SEQ)
		{
			field->op = field_op_delta_num;
		}
		else if(FIELD_IS_BYTESTR(params))
		{
			field->op = field_op_delta_str;
		}
		else
		{
			g_free(field->name);
			g_free(field);
			DBG0("invalid field type");
			return ERR_BADARG;
		}

		break;

	case FIELD_OP_TAIL:

		if(!FIELD_IS_BYTESTR(params))
		{
			g_free(field->name);
			g_free(field);
			DBG0("invalid field type");
			return ERR_BADARG;
		}

		field->op=field_op_tail;
		break;

	case FIELD_OP_NONE:
		field->op=field_op_noop;
		break;

	default:
		g_free(field->name);
		g_free(field);
		DBG0("invalid field operator");
		return ERR_BADARG;
	}

	/************************************************************************/

	/* decode and display setup */
	switch(FIELD_TYPE(params))
	{
	case FIELD_TYPE_UINT32:
		field->decode=	field_decode_uint32;
		field->display=	field_display_uint32;
		break;
	case FIELD_TYPE_INT32:
		field->decode=	field_decode_int32;
		field->display=	field_display_int32;
		break;

	case FIELD_TYPE_UINT64:
		field->decode=	field_decode_uint64;
		field->display=	field_display_uint64;
		break;
	case FIELD_TYPE_INT64:
		field->decode=	field_decode_int64;
		field->display=	field_display_int64;
		break;

	case FIELD_TYPE_FLT10:
		field->decode=	field_decode_flt10;
		field->display=	field_display_flt10;

		/*field->display=field_display_flt10;
		field->op=field_op_dec;*/

		break;
	/*case FIELD_TYPE_FIXDEC:
		field->decode=	field_decode_fixdec;
		field->display=	field_display_fixdec;
		break;*/

	case FIELD_TYPE_ASCII:
		field->decode=	field_decode_ascii;
		field->display=	field_display_ascii;
		break;
	case FIELD_TYPE_UTF8:
		field->decode=	field_decode_utf8;
		field->display=	field_display_utf8;
		break;

	case FIELD_TYPE_BYTES:
		field->decode=	field_decode_bytes;
		field->display=	field_display_bytes;
		break;

	case FIELD_TYPE_SEQ:
		field->decode=field_decode_uint32;
		field->display=0;
		break;
	case FIELD_TYPE_GROUP:
		field->decode=0;
		field->display=0;
		/*DBG0("field type implemented");
		return ERR_NOTIMPL;*/
		break;

	default:
		g_free(field->name);
		g_free(field);
		DBG0("invalid field type");
		return ERR_BADARG;
	}

	/************************************************************************/

	field->type=params->type;
	field->mandatory=params->mandatory;

	field->state=FIELD_STATE_UNDEF;

	/* we are ignoring subfields for now */
	field->subfields=0;

	field->hf_id=-1;
	field->ett_id=-1;

	field->next=0;

	/************************************************************************/

	if(!in_template && (FIELD_IS_COMPLEX(params)
		/*|| FIELD_IS_DECIMAL(params)*/))
	{
		/* nothing */
	}
	if(!in_template->fields)
	{
		in_template->fields=field;
	}
	else
	{
		struct template_field_type* cur;
		for(cur=in_template->fields;cur->next;cur=cur->next);
		cur->next=field;
	}

	if(outptr) *outptr=field;

	return 0;
}

gint create_subfield(
	struct template_type* t,
	struct template_field_type* parent,
	struct template_field_type* params,
	struct template_field_type** outptr)
{
	struct template_field_type* p;
	gint ret;

	if(!t || !parent || !params) return ERR_BADARG;

	if(!FIELD_IS_COMPLEX(parent))
	{
		return ERR_BADARG;
	}

	ret=create_field(0,params,&p);
	if(ret<0 || !p)
	{
		return ret;
	}

	if(!parent->subfields)
	{
		parent->subfields=p;
	}
	else
	{
		struct template_field_type* cur;
		for(cur=parent->subfields; cur->next; cur=cur->next);
		cur->next=p;
	}

	if(outptr) *outptr=p;

	return 0;
}

void cleanup_all(void)
{
	/* Err... Nested block comments... */
#if 0
	struct template_list_node* cur=all_templates;
	while(cur)
	{
		if(!cur->p) continue;

		struct template_field_type* curf=cur->p->fields;
		while(curf)
		{
			struct template_field_type* tmp=curf;
			curf=curf->next;
			g_free(tmp->name);
			/* g_free(tmp->def_value); */
			/* g_free(tmp->value); */
			g_free(tmp->prev_value);
			g_free(tmp);
		}

		struct template_type* tmp=cur->p;
		cur=cur->next;
		g_free(tmp->name);
		g_free(tmp);
	}
#endif
}

gint find_template(const char* name, struct template_type** out)
{
	struct template_type* cur;

	if(!name || !out)
	{
		DBG0("null argument");
		return ERR_BADARG;
	}

	for(cur=all_templates;cur;cur=cur->next)
	{
		if(strcmp(name,cur->name)==0)
		{
			*out=cur;
			return 1;
		}
	}

	return 0;
}

gint find_template_byid(guint8 id, struct template_type** out)
{
	struct template_type* cur;

	if(!out)
	{
		DBG0("null argument");
		return ERR_BADARG;
	}

	for(cur=all_templates;cur;cur=cur->next)
	{
		if(cur->id==id)
		{
			*out=cur;
			return 1;
		}
	}

	return 0;
}

gint find_template_field(
	const char* name,
	struct template_type* tmpl,
	struct template_field_type** out)
{
	struct template_field_type* cur;

	if(!name || !tmpl || !out)
	{
		DBG0("null argument");
		return ERR_BADARG;
	}

	cur=tmpl->fields;
	for(cur=tmpl->fields;cur;cur=cur->next)
	{
		if(strcmp(name,cur->name)==0)
		{
			*out=cur;
			return 1;
		}
	}

	return 0;
}

gint copy_field_value(
	guint type,
	field_value* src,
	field_value* dest)
{
	switch(type)
	{
	case FIELD_TYPE_UINT32:
		dest->u32=src->u32;
		break;
	case FIELD_TYPE_INT32:
		dest->i32=src->i32;
		break;
	case FIELD_TYPE_UINT64:
		dest->u64=src->u64;
		break;
	case FIELD_TYPE_INT64:
		dest->i64=src->i64;
		break;
	case FIELD_TYPE_FLT10:
		dest->flt10=src->flt10;
		break;
	/*case FIELD_TYPE_FIXDEC:
		dest->fixdec=src->fixdec;
		break;*/
	case FIELD_TYPE_ASCII:
		dest->str.p=(guint8*)g_strdup((gchar*)src->str.p);
		if(!dest->str.p)
		{
			DBG0("out of memory");
			return ERR_NOMEM;
		}
		break;
	case FIELD_TYPE_UTF8:
	case FIELD_TYPE_BYTES:
		dest->str.p=g_memdup(src->str.p,src->str.len);
		if(!dest->str.p)
		{
			DBG0("out of memory");
			return ERR_NOMEM;
		}
		dest->str.len=src->str.len;
		break;
	default:
		DBG0("invalid field type");
		return ERR_BADARG;
	}
	return 0;
}

gint reset_template_state(struct template_type* t)
{
	DBG0("not implemented");
	return ERR_NOTIMPL;
}
