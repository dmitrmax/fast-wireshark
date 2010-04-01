/**
	@file	setup.c
	@brief	|
	@author	Wes Fournier

	|
*/

#include "template.h"

/* used for setting up gui tree */
int ett_fast_int1=-1;
int ett_fast_int2=-1;
int ett_fast_int3=-1;
int ett_fast_int4=-1;
int hf_fast_int1=-1;
int hf_fast_int2=-1;
int hf_fast_int3=-1;
int hf_fast_int4=-1;
int ett_fast_str1=-1;
int hf_fast_str1=-1;

void FAST_setup(int id)
{
	/* create gui tree */

	static int* ett[]={
		&ett_fast,
		&ett_fast_tid,
		&ett_fast_int1,
		&ett_fast_int2,
		&ett_fast_str1};

	static hf_register_info hf[]={
		{&hf_fast_tid,
			{"TID","fast.tid",FT_STRING,BASE_NONE,
				NULL,0x0,"TID",HFILL}},
		{&hf_fast_int1,
			{"int1","fast.int1",FT_INT32,BASE_DEC,
				NULL,0x0,"int1",HFILL}},
		{&hf_fast_int2,
			{"int2","fast.int2",FT_UINT32,BASE_DEC,
				NULL,0x0,"int2",HFILL}},
		{&hf_fast_int3,
			{"int3","fast.int3",FT_INT64,BASE_DEC,
				NULL,0x0,"int3",HFILL}},
		{&hf_fast_int4,
			{"int4","fast.int4",FT_UINT64,BASE_DEC,
				NULL,0x0,"int4",HFILL}},
		{&hf_fast_str1,
			{"str1","fast.str1",FT_STRING,BASE_NONE,
				NULL,0x0,"str1",HFILL}}
	};

	struct template_type* t;
	struct template_field_type f;

	gint ret;

	proto_register_field_array(id,hf,array_length(hf));
	proto_register_subtree_array(ett,array_length(ett));

	/* setup our protocol templates */

	init_templates();

	ret=create_template("test_template",1,&t);
	if(ret<0)
	{
		DBG_RET(ret);
		return;
	}

	f.type=FIELD_TYPE_INT32|FIELD_OP_NONE;
	f.mandatory=1;
	f.name="int1";
	f.hf_id=hf_fast_int1;
	f.ett_id=ett_fast_int1;
	ret=create_field(t,&f,0);
	if(ret<0)
	{
		DBG_RET(ret);
		return;
	}

	f.type=FIELD_TYPE_UINT32|FIELD_OP_INCR;
	f.mandatory=0;
	f.name="int2";
	f.hf_id=hf_fast_int2;
	f.ett_id=ett_fast_int2;
	ret=create_field(t,&f,0);
	if(ret<0)
	{
		DBG_RET(ret);
		return;
	}

	f.type=FIELD_TYPE_ASCII|FIELD_OP_NONE;
	f.mandatory=1;
	f.name="str1";
	f.hf_id=hf_fast_str1;
	f.ett_id=ett_fast_str1;
	ret=create_field(t,&f,0);
	if(ret<0)
	{
		DBG_RET(ret);
		return;
	}

	f.type=FIELD_TYPE_INT64|FIELD_OP_NONE;
	f.mandatory=1;
	f.name="int3";
	f.hf_id=hf_fast_int3;
	f.ett_id=ett_fast_int3;
	ret=create_field(t,&f,0);
	if(ret<0)
	{
		DBG_RET(ret);
		return;
	}

	f.type=FIELD_TYPE_UINT64|FIELD_OP_NONE;
	f.mandatory=1;
	f.name="int4";
	f.hf_id=hf_fast_int4;
	f.ett_id=ett_fast_int4;
	ret=create_field(t,&f,0);
	if(ret<0)
	{
		DBG_RET(ret);
		return;
	}
}
