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
	if(pmap_size<0) return;
	off+=pmap_size;

	//TODO: we should check the PMAP to see if TID is present,
	// 	for now we assume it is there
	// figure out current Template ID
	ret=decode_uint32(tvb,off,&current_tid);
	if(ret<0) return;
	off+=ret;

	struct template_type* t=0;
	ret=find_template_byid(current_tid,&t);
	if(ret<0) return;
	if(!t)
	{
		// unknown template

		proto_tree_add_string(
			tree,
			hf_fast_tid,
			0,
			0,
			strlen("Unknown"),
			"Unknown");
		return;
	}

	proto_tree_add_string(
		tree,
		hf_fast_tid,
		0,
		0,
		strlen(t->name),
		t->name);

	struct template_field_type* cur=0;
	int i=0;
	for(cur=t->fields;cur;cur=cur->next)
	{
		//TODO: here we would check our field state and pmap[i]
		// to figure out if we actually need to decode the data,
		// or use a field operator on existing data
		// right now we are ignoring the PMAP and assuming all fields
		// are required

		if(cur->read)
		{
			ret=(cur->read)(cur,tvb,off);
			if(ret<0) return;
			off+=ret;

			i++;
		}

		//TODO: here we would display the value of this field
	}
}
