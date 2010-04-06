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
	header_field_info tmpinfo;

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

	/* add an entry for the base node and TID */
	ettreginfo=g_malloc(sizeof(int*)*(field_count+2));
	if(!ettreginfo)
	{
		return ERR_NOMEM;
	}

	/***** fill out our registration arrays *****/

	hfreginfo[0].p_id=&hf_fast_tid;
	tmpinfo.name="TID";
	tmpinfo.abbrev="fast.tid";
	tmpinfo.type=FT_STRING;
	tmpinfo.display=BASE_NONE;
	tmpinfo.strings=NULL;
	tmpinfo.bitmask=0x0;
	tmpinfo.blurb="TID";
	hfreginfo[0].hfinfo=tmpinfo;
	proto_register_field_array(global_fast_id,&(hfreginfo[0]),1);

	ettreginfo[0]=&ett_fast;
	ettreginfo[1]=&ett_fast_tid;

	for(i=0,cur=t->fields;cur;cur=cur->next,i++)
	{
		DBG2("current field %s:%d",cur->name,i);

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
			type=FT_STRING;
			base=BASE_NONE;
			break;
		case FIELD_TYPE_FLT10:
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
		tmpinfo.name=cur->name;
		tmpinfo.abbrev=g_strdup(buf);
		tmpinfo.type=type;
		tmpinfo.display=base;
		tmpinfo.abbrev=cur->name;
		hfreginfo[i+1].hfinfo=tmpinfo;
		ettreginfo[i+2]=&(cur->ett_id);

		proto_register_field_array(global_fast_id,&(hfreginfo[i+1]),1);
	}

	/* now we can register our arrays */
	/*proto_register_field_array(global_fast_id,hfreginfo,field_count+1);*/
	proto_register_subtree_array(ettreginfo,field_count+2);

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
		DBG1("current field %s",cur->name);
		if(!(cur->op))
		{
			DBG0("no operator function");
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
				DBG0("no display function");
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

		if(last_tid!=current_tid)
		{
			find_template_byid(current_tid,&t);
			if(t)
			{
				reset_template_state(t);
				/*setup_template_fields(t,proto_fast);*/
			}

			last_tid=current_tid;
		}
	}

	ret=find_template_byid(current_tid,&t);
	if(ret<0)
	{
		DBG_RET(ret);
		return;
	}

	ti=proto_tree_add_item(tree,proto_fast,tvb,0,-1,FALSE);
	tree=proto_item_add_subtree(ti,ett_fast);

	if(!t)
	{
		/*  unknown template, or no template specified */
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
	}

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
