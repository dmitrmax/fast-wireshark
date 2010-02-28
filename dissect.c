/**
	@file	dissect.c
	@brief	packet dissection routine
	@author	Wes Fournier

	|
*/

#include "fast.h"
#include "template.h"
#include "decode.h"

static guint32 current_tid=0;
static guint32 last_tid=0;

void FAST_dissect(int proto_fast, tvbuff_t* tvb, int n, packet_info* pinfo,
	proto_tree* tree)
{
	gint ret=0;
	guint off=0;

	proto_item* ti=NULL;
	ti=proto_tree_add_item(tree,proto_fast,tvb,0,-1,FALSE);
	tree=proto_item_add_subtree(ti,ett_fast);

	guint8* pmap=0;
	gint pmap_size=decode_pmap(tvb,off,&pmap);
	if(pmap_size<0)
	{
		DBG_RET(pmap_size);
		return;
	}
	off+=pmap_size;
	off=1;

	struct template_type* t=0;

	if(pmap[0]) // check to see if TID is present
	{
		// figure out current Template ID
		ret=decode_uint32(tvb,off,&current_tid);
		if(ret<0) return;
		off+=ret;

		if(last_tid!=current_tid)
		{
			find_template_byid(current_tid,&t);
			if(t) reset_template_state(t);
			last_tid=current_tid;
		}
	}

	ret=find_template_byid(current_tid,&t);
	if(ret<0)
	{
		DBG_RET(ret);
		return;
	}

	if(!t)
	{
		// unknown template, or no template specified
		proto_tree_add_string(
			tree,
			hf_fast_tid,
			tvb,
			0,
			strlen("Unknown"),
			"Unknown");

		DBG("Unknown or invalid template ID %d",current_tid);
		return;
	}

	proto_tree_add_string(
		tree,
		hf_fast_tid,
		tvb,
		0,
		strlen(t->name),
		t->name);

	struct template_field_type* cur=0;
	int i=1;
	for(cur=t->fields;cur;cur=cur->next)
	{
		if((FIELD_REQUIRED(cur) || pmap[i]) && cur->read)
		{
			ret=(cur->read)(cur,tvb,off);
			if(ret<0)
			{
				DBG_RET(ret);
				return;
			}
			off+=ret;

			i++;
		}
		else if(cur->op_func)
		{
			ret=(cur->op_func)(cur);
			if(ret<0)
			{
				DBG_RET(ret);
				return;
			}
		}

		if(cur->display)
		{
			ret=(cur->display)(cur,tree,tvb);
			if(ret<0)
			{
				DBG_RET(ret);
				return;
			}
		}
	}
}
