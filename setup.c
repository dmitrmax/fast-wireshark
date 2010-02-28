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
	/* setup our protocol templates */

	init_templates();

	struct template_type* t;
	struct template_field_type* f_int1;
	struct template_field_type* f_int2;
	create_template("test_template",1,&t);
	append_field(
		"int1",
		FIELD_TYPE_INT32|FIELD_REQUIRED_BIT,
		FIELD_OP_NONE,
		NULL_FIELD_VALUE,
		0,
		-1,
		t,
		&f_int1);
	append_field(
		"int2",
		FIELD_TYPE_UINT32|FIELD_REQUIRED_BIT,
		FIELD_OP_NONE,
		NULL_FIELD_VALUE,
		0,
		-1,
		t,
		&f_int2);

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
		{&ett_fast_int1,
			{"int1","int1",FT_INT32,BASE_DEC,
				NULL,0x0,"int1",HFILL}},
		{&ett_fast_int2,
			{"int2","int2",FT_UINT32,BASE_DEC,
				NULL,0x0,"int2",HFILL}}
	};
	f_int1->hf_id=ett_fast_int1;
	f_int2->hf_id=ett_fast_int2;

	proto_register_field_array(id,hf,array_length(hf));
	proto_register_subtree_array(ett,array_length(ett));
}
