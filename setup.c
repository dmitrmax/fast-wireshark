/**
	@file	setup.c
	@brief	|
	@author	Wes Fournier

	|
*/

#include "template.h"

/* used for setting up gui tree */
int ett_fast=-1;
int ett_fast_int1=-1;
int ett_fast_int2=-1;
int hf_fast=-1;
int hf_fast_int1=-1;
int hf_fast_int2=-1;

void FAST_setup(int id)
{
	/* setup our protocol templates */

	init_templates();

	struct template_type* t;
	create_template("test",1,&t);
	field_value v;
	v.val=0;
	append_field(
		"int1",
		FIELD_TYPE_INT32|FIELD_REQUIRED_BIT,
		0,
		v,
		0,
		t,
		0);
	append_field(
		"int2",
		FIELD_TYPE_UINT32|FIELD_REQUIRED_BIT,
		0,
		v,
		0,
		t,
		0);

	/* create gui tree */

	static int* ett[] = {&ett_fast,&ett_fast_int1,&ett_fast_int2};
	static hf_register_info hf[] = {
		{&hf_fast_int1,
			{"Int1","fast.int1",FT_INT,BASE_NONE,
				NULL,0x0,"Int1",HFILL}},
		{&hf_fast_int2,
			{"Int2","fast.int2",FT_INT,BASE_NONE,
				NULL,0x0,"Int2",HFILL}}
	};

	proto_register_field_array(id,hf,array_length(hf));
	proto_register_subtree_array(ett,array_length(ett));
}
