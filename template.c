/**
	@file	template.c
	@brief	|
	@author	Wes Fournier

	|
*/

#include "template.h"

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
	if(!name) return -1;

	// create entry in our global template list

	struct template_list_node* currnode=0;
	if(!all_templates)
	{
		currnode=all_templates=g_malloc(sizeof(struct template_list_node));
		if(!currnode) return -2;
		currnode->next=0;
		currnode->p=0;
	}
	else
	{
		for(currnode=all_templates;currnode->next;currnode=currnode->next);
		currnode->next = g_malloc(sizeof(struct template_list_node));
		if(!currnode->next) return -2;
		currnode=currnode->next;
		currnode->next=0;
		currnode->p=0;
	}

	// create template information struct

	struct template_type* p = g_malloc(sizeof(struct template_type));
	if(!p)
	{
		g_free(currnode);
		return -2;
	}

	p->fields=0;

	p->name = g_malloc(strlen(name));
	if(!p->name)
	{
		g_free(p);
		g_free(currnode);
		return -2;
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
	void* def_value,
	guint32 def_value_size,
	struct template_type* tmpl,
	struct template_field_type** out)
{
	if(!name) return -1;
	if(!tmpl) return -1;

	// create template field info struct

	struct template_field_type* f=g_malloc(
		sizeof(struct template_field_type));
	if(!f)
	{
		return -2;
	}
	memset(f,0,sizeof(struct template_field_type));

	f->name=g_malloc(strlen(name));
	if(!f->name)
	{
		g_free(f);
		return -2;
	}
	memcpy(f->name,name,strlen(name));

	f->type=type;
	f->op=op;

	if(def_value)
	{
		f->def_value=g_malloc(def_value_size);
		if(!f->def_value)
		{
			g_free(f->name);
			g_free(f);
			return -2;
		}
		memcpy(f->def_value,def_value,def_value_size);
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
			g_free(tmp->def_value);
			g_free(tmp->value);
			g_free(tmp->prev_value);
			g_free(tmp);
		}

		struct template_type* tmp=cur->p;
		cur=cur->next;
		g_free(tmp->name);
		g_free(tmp);
	}
}

gint find_template(const char* name, struct template_type** out)
{
	if(!name) return -1;
	if(!out) return -1;

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

	return -2;
}

gint find_template_byid(guint8 id, struct template_type** out)
{
	if(!out) return -1;

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
	return -2;
}

gint find_template_field(
	const char* name,
	struct template_type* tmpl,
	struct template_field_type** out)
{
	if(!name) return -1;
	if(!tmpl) return -1;
	if(!out) return -1;

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
	return -2;
}

gint find_template_field_byindex(
	guint index,
	struct template_type* tmpl,
	struct template_field_type** out,
	guint skip_required)
{
	if(!tmpl) return -1;
	if(!out) return -1;

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
	return -2;
}
