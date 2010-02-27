/**
	@file	template.c
	@brief	|
	@author	Wes Fournier

	|
*/

#include "template.h"
#include "decode.h"

struct template_list_node
{
	struct template_type* p;
	struct template_list_node* next;
};

static struct template_list_node* all_templates;

void init_templates(void)
{
	all_templates=0;
}

gint create_template(
	const char* name,
	guint32 id,
	struct template_type** out)
{
	if(!name) return ERR_BADARG;

	// create entry in our global template list

	struct template_list_node* currnode=0;
	if(!all_templates)
	{
		currnode=all_templates=g_malloc(sizeof(struct template_list_node));
		if(!currnode) return ERR_NOMEM;
		currnode->next=0;
		currnode->p=0;
	}
	else
	{
		for(currnode=all_templates;currnode->next;currnode=currnode->next);
		currnode->next = g_malloc(sizeof(struct template_list_node));
		if(!currnode->next) return ERR_NOMEM;
		currnode=currnode->next;
		currnode->next=0;
		currnode->p=0;
	}

	// create template information struct

	struct template_type* p = g_malloc(sizeof(struct template_type));
	if(!p)
	{
		g_free(currnode);
		return ERR_NOMEM;
	}

	p->fields=0;

	p->name = g_malloc(strlen(name));
	if(!p->name)
	{
		g_free(p);
		g_free(currnode);
		return ERR_NOMEM;
	}
	memcpy(p->name,name,strlen(name));

	p->id=id;

	if(out) *out=p;
	currnode->p=p;

	return 0;
}

gint append_field(
	const char* name,
	guint8 type,
	guint8 op,
	field_value def_value,
	guint32 def_value_size,
	int hf_id,
	struct template_type* tmpl,
	struct template_field_type** out)
{
	if(!name) return ERR_BADARG;
	if(!tmpl) return ERR_BADARG;

	// create template field info struct

	struct template_field_type* f=g_malloc(
		sizeof(struct template_field_type));
	if(!f)
	{
		return ERR_NOMEM;
	}
	memset(f,0,sizeof(struct template_field_type));

	f->name=g_malloc(strlen(name));
	if(!f->name)
	{
		g_free(f);
		return ERR_NOMEM;
	}
	memcpy(f->name,name,strlen(name));

	f->type=type;
	f->op=op;
	f->hf_id=hf_id;

	switch(FIELD_TYPE(f))
	{
	case FIELD_TYPE_INT32: f->read=read_int32_field; break;
	case FIELD_TYPE_UINT32: f->read=read_uint32_field; break;
	case FIELD_TYPE_INT64: f->read=read_int64_field; break;
	case FIELD_TYPE_UINT64: f->read=read_uint64_field; break;
	case FIELD_TYPE_FLT10: f->read=read_flt10_field; break;
	case FIELD_TYPE_FIXDEC: f->read=read_fixdec_field; break;
	case FIELD_TYPE_UTF8: f->read=read_utf8_field; break;
	case FIELD_TYPE_ASCII: f->read=read_ascii_field; break;
	case FIELD_TYPE_BYTES: f->read=read_bytes_field; break;
	case FIELD_TYPE_SEQ: f->read=0; break;
	case FIELD_TYPE_GROUP: f->read=0; break;
	default:
		g_free(f->name);
		g_free(f);
		return ERR_BADARG;
	}

	if(def_value.val)
	{
		if(FIELD_TYPE(f)>FIELD_TYPE_FIXDEC)
		{
			f->def_value.val=g_malloc(def_value_size);
			if(!(f->def_value.val))
			{
				g_free(f->name);
				g_free(f);
				return ERR_NOMEM;
			}
			memcpy(f->def_value.val,def_value.val,def_value_size);
		}
		else
		{
			f->def_value.val = def_value.val;
		}
	}

	// find place to put field
	struct template_field_type* cur=0;
	if(!tmpl->fields)
	{
		tmpl->fields=f;
	}
	else
	{
		for(cur=tmpl->fields;cur->next;cur=cur->next);
		cur->next=f;
	}

	if(out) *out=f;

	return 0;
}

void cleanup_all(void)
{
	/*struct template_list_node* cur=all_templates;
	while(cur)
	{
		if(!cur->p) continue;

		struct template_field_type* curf=cur->p->fields;
		while(curf)
		{
			struct template_field_type* tmp=curf;
			curf=curf->next;
			g_free(tmp->name);
			//g_free(tmp->def_value);
			//g_free(tmp->value);
			g_free(tmp->prev_value);
			g_free(tmp);
		}

		struct template_type* tmp=cur->p;
		cur=cur->next;
		g_free(tmp->name);
		g_free(tmp);
	}*/
}

gint find_template(const char* name, struct template_type** out)
{
	if(!name) return ERR_NOMEM;
	if(!out) return ERR_NOMEM;

	*out=0;

	struct template_list_node* cur=all_templates;
	while(cur)
	{
		if(!cur->p) continue;

		if(strcmp(name,cur->p->name)==0)
		{
			*out=cur->p;
			return 0;
		}
		cur=cur->next;
	}

	return 0;
}

gint find_template_byid(guint8 id, struct template_type** out)
{
	if(!out) return ERR_BADARG;

	*out=0;
	struct template_list_node* cur=all_templates;
	while(cur)
	{
		if(!cur->p) continue;

		if(cur->p->id==id)
		{
			*out=cur->p;
			return 0;
		}
		cur=cur->next;
	}
	return 0;
}

gint find_template_field(
	const char* name,
	struct template_type* tmpl,
	struct template_field_type** out)
{
	if(!name) return ERR_BADARG;
	if(!tmpl) return ERR_BADARG;
	if(!out) return ERR_BADARG;

	*out=0;
	struct template_field_type* cur=tmpl->fields;
	while(cur)
	{
		if(strcmp(name,cur->name)==0)
		{
			*out=cur;
			return 0;
		}
		cur=cur->next;
	}
	return 0;
}

gint find_template_field_byindex(
	guint index,
	struct template_type* tmpl,
	struct template_field_type** out,
	guint skip_required)
{
	if(!tmpl) return ERR_BADARG;
	if(!out) return ERR_BADARG;

	*out=0;
	struct template_field_type* cur=tmpl->fields;
	int i=0;
	while(cur)
	{
		if(!skip_required || !FIELD_REQUIRED(cur))
		{
			if(i==index)
			{
				*out=cur;
				return 0;
			}
			index++;
		}

		cur=cur->next;
	}
	return 0;
}

gint read_uint32_field(
	struct template_field_type* f,
	tvbuff_t* buf,
	guint off)
{
	return decode_uint32(buf,off,&(f->value.u32));
}

gint read_int32_field(
	struct template_field_type* f,
	tvbuff_t* buf,
	guint off)
{
	return decode_int32(buf,off,&(f->value.i32));
}

gint read_uint64_field(
	struct template_field_type* f,
	tvbuff_t* buf,
	guint off)
{
	return decode_uint64(buf,off,&(f->value.u64));
}

gint read_int64_field(
	struct template_field_type* f,
	tvbuff_t* buf,
	guint off)
{
	return decode_int64(buf,off,&(f->value.i64));
}

gint read_ascii_field(
	struct template_field_type* f,
	tvbuff_t* buf,
	guint off)
{
	if(f->value.str) g_free(f->value.str);
	gint ret= decode_ascii(buf,off,&(f->value.str));
	f->size=ret;
	return ret;
}

gint read_utf8_field(
	struct template_field_type* f,
	tvbuff_t* buf,
	guint off)
{
	return read_bytes_field(f,buf,off);
}

gint read_bytes_field(
	struct template_field_type* f,
	tvbuff_t* buf,
	guint off)
{
	if(f->value.str) g_free(f->value.str);
	gint ret=decode_utf8(buf,off,&(f->value.str));
	f->size=ret;
	return ret;
}

gint read_flt10_field(
	struct template_field_type* f,
	tvbuff_t* buf,
	guint off)
{
	return decode_flt10(buf,off,f->value.dec,f->value.dec+1);
}

gint read_fixdec_field(
	struct template_field_type* f,
	tvbuff_t* buf,
	guint off)
{
	return  decode_fixdec(buf,off,f->wholebits,f->value.dec,f->value.dec+1);
}

gint reset_template_state(struct template_type* t)
{
	return ERR_NOTIMPL;
}

gint set_field_display(proto_tree* tree, struct template_field_type* f)
{
	/*switch(FIELD_TYPE(f))
	{
	case FIELD_TYPE_INT32:
	}

	return 0;*/
}
