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

/* these must be created on the heap */
static hf_register_info* hfreginfo=0;
static int** ettreginfo=0;

gint setup_template_fields(
	struct template_type* t,
	int global_fast_id)
{
	struct template_field_type* cur;
	gint i;
	gint field_count;
	char buf[256];
	gint type,base;
	/*header_field_info tmpinfo;*/

	if(!t)
	{
		DBG0("Null argument");
		return ERR_BADARG;
	}

	g_free(hfreginfo);
	g_free(ettreginfo);

	for(field_count=0,cur=t->fields;cur;cur=cur->next,field_count++);

	/* add an entry for the TID */
	hfreginfo=g_malloc(sizeof(hf_register_info)*(field_count+1));
	if(!hfreginfo)
	{
		return ERR_NOMEM;
	}
	memset(hfreginfo,0,sizeof(hf_register_info)*(field_count+1));

	/* add an entry for the base node and TID */
	ettreginfo=g_malloc(sizeof(int*)*(field_count+2));
	if(!ettreginfo)
	{
		return ERR_NOMEM;
	}
	memset(ettreginfo,0,sizeof(int*)*(field_count+2));

	/***** fill out our registration arrays *****/

	hfreginfo[0].p_id=&hf_fast_tid;
	hfreginfo[0].hfinfo.name="TID";
	hfreginfo[0].hfinfo.abbrev="fast.tid";
	hfreginfo[0].hfinfo.type=FT_STRING;
	hfreginfo[0].hfinfo.display=BASE_NONE;
	hfreginfo[0].hfinfo.blurb="TID";

	ettreginfo[0]=&ett_fast;
	ettreginfo[1]=&ett_fast_tid;

	for(i=0,cur=t->fields;cur;cur=cur->next,i++)
	{
		/*DBG2("current field %s:%d",cur->name,i);*/

		switch(FIELD_TYPE(cur))
		{
		case FIELD_TYPE_INT32:
			type=FT_INT32;
			base=BASE_DEC;
			break;
		case FIELD_TYPE_UINT32:
			type=FT_UINT32;
			base=BASE_DEC;
			break;
		case FIELD_TYPE_INT64:
			type=FT_INT64;
			base=BASE_DEC;
			break;
		case FIELD_TYPE_UINT64:
			type=FT_UINT64;
			base=BASE_DEC;
			break;
		case FIELD_TYPE_UTF8:
		case FIELD_TYPE_ASCII:
		case FIELD_TYPE_BYTES:
		case FIELD_TYPE_FLT10:
			type=FT_STRING;
			base=BASE_NONE;
			break;
		case FIELD_TYPE_SEQ:
		case FIELD_TYPE_GROUP:
			DBG1("field type %d unimplemented",FIELD_TYPE(cur));
			g_free(hfreginfo);
			g_free(ettreginfo);
			return ERR_BADARG;
		default:
			DBG1("Unknown field type %d",FIELD_TYPE(cur));
			g_free(hfreginfo);
			g_free(ettreginfo);
			return ERR_BADARG;
		}

		g_snprintf(buf,sizeof(buf),"fast.%s",cur->name);
		hfreginfo[i+1].p_id=&(cur->hf_id);
		hfreginfo[i+1].hfinfo.name=cur->name;
		hfreginfo[i+1].hfinfo.abbrev=g_strdup(buf);
		hfreginfo[i+1].hfinfo.type=type;
		hfreginfo[i+1].hfinfo.display=base;
		hfreginfo[i+1].hfinfo.abbrev=cur->name;
		ettreginfo[i+2]=&(cur->ett_id);
	}

	/* now we can register our arrays */
	proto_register_field_array(global_fast_id,hfreginfo,field_count+1);
	proto_register_subtree_array(ettreginfo,field_count+2);

	/*for(i=0;i<field_count+1;i++) DBG2("hf%d: %d",i,*(hfreginfo[i].p_id));
	for(i=0;i<field_count+2;i++) DBG2("ett%d: %d",i,*(ettreginfo[i]));*/

	return 0;
}

static void process_fields(
	struct template_field_type* cur,
	tvbuff_t* tvb,
	guint off,
	proto_tree* tree,
	guint8* pmap,
	guint pmap_len)
{
	int ret;
	/*TRACE();*/

	for(;cur;cur=cur->next)
	{
		/*DBG1("current field %s",cur->name);*/
		if(!(cur->op))
		{
			DBG1("no operator function for %s",cur->name);
			return;
		}

		cur->offset = off;
		ret=(cur->op)(cur,&pmap,tvb,&off);
		cur->length = off - cur->offset;

		if(ret<0)
		{
			DBG_RET(ret);
			return;
		}

		if(FIELD_DISPLAY_ON(cur))
		{
			if(!(cur->display))
			{
				DBG1("no display function for %s",cur->name);
				return;
			}

			(cur->display)(cur,tree,tvb);
		}
	}
}

void FAST_dissect(int proto_fast, tvbuff_t* tvb, int n, packet_info* pinfo,
	proto_tree* tree)
{
	gint ret=0;
	guint off=0;
	guint8* pmap=0;
	gint pmap_size;
	struct template_type* t=0;
	proto_item* ti=NULL;

	pmap_size=decode_pmap(tvb,off,&pmap);
	if(pmap_size<0)
	{
		DBG_RET(pmap_size);
		return;
	}
	off+=pmap_size;

	if(pmap[0]) /*  check to see if TID is present */
	{
		/*  figure out current Template ID */
		ret=decode_uint32(tvb,off,&current_tid);
		if(ret<0) return;
		off+=ret;
	}
	else
	{
		DBG0("TID not present");
		current_tid=last_tid;
	}

	ret=find_template_byid(current_tid,&t);
	if(ret<0)
	{
		DBG_RET(ret);
		return;
	}
	if(!t)
	{
		DBG1("No template found for TID %d",current_tid);
		return;
	}

	if(last_tid!=current_tid)
	{
		reset_template_state(t);
		setup_template_fields(t,proto_fast);

		last_tid=current_tid;
	}

	ti=proto_tree_add_item(tree,proto_fast,tvb,0,-1,FALSE);
	tree=proto_item_add_subtree(ti,ett_fast);

	/*if(!t)
	{
		proto_tree_add_string(
			tree,
			hf_fast_tid,
			tvb,
			0,
			strlen("Unknown"),
			"Unknown");

		DBG1("Unknown or invalid template ID %d",current_tid);
		g_free(pmap);
		return;
	}*/

	proto_tree_add_string(
		tree,
		hf_fast_tid,
		tvb,
		0,
		strlen(t->name),
		t->name);

	process_fields(
		t->fields,
		tvb,
		off,
		tree,
		pmap+1,
		pmap_size-1);

	g_free(pmap);
}
