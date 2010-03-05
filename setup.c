/**
	@file	setup.c
	@brief	|
	@author	Wes Fournier

	|
*/

#include "template.h"

/* used for setting up gui tree */
int ett_fast=-1;
int ett_fast_tid=-1;
int ett_fast_int1=-1;
int ett_fast_int2=-1;
int hf_fast=-1;
int hf_fast_tid=-1;
int hf_fast_int1=-1;
int hf_fast_int2=-1;

void FAST_setup(int id)
{
	/* create gui tree */

	static int* ett[]={
		&ett_fast,
		&ett_fast_tid,
		&ett_fast_int1,
		&ett_fast_int2};

	static hf_register_info hf[]={
		{&hf_fast_tid,
			{"TID","fast.tid",FT_STRING,BASE_NONE,
				NULL,0x0,"TID",HFILL}},
		{&hf_fast_int1,
			{"int1","fast.int1",FT_INT32,BASE_DEC,
				NULL,0x0,"int1",HFILL}},
		{&hf_fast_int2,
			{"int2","fast.int2",FT_UINT32,BASE_DEC,
				NULL,0x0,"int2",HFILL}}
	};

	struct template_type* t;

	proto_register_field_array(id,hf,array_length(hf));
	proto_register_subtree_array(ett,array_length(ett));

	/* setup our protocol templates */

	init_templates();

	create_template("test_template",1,&t);
	append_field(
		"int1",
		FIELD_TYPE_INT32|FIELD_REQUIRED_BIT,
		FIELD_OP_NONE,
		NULL_FIELD_VALUE,
		0,
		hf_fast_int1,
		t,
		0);
	append_field(
		"int2",
		FIELD_TYPE_UINT32|FIELD_REQUIRED_BIT,
		FIELD_OP_NONE,
		NULL_FIELD_VALUE,
		0,
		hf_fast_int2,
		t,
		0);
}
