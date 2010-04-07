/**
	@file	setup.c
	@brief	|
	@author	Wes Fournier

	|
*/

#include "template.h"

void FAST_setup(int id)
{
	struct template_type* t;
	struct template_field_type f;

	init_templates();

	create_template("test_template",1,&t);

	f.type=FIELD_TYPE_INT32|FIELD_OP_NONE;
	f.mandatory=1;
	f.name="int1";
	create_field(t,&f,0);

	f.type=FIELD_TYPE_UINT32|FIELD_OP_INCR;
	f.mandatory=0;
	f.name="int2";
	create_field(t,&f,0);

	f.type=FIELD_TYPE_ASCII|FIELD_OP_NONE;
	f.mandatory=1;
	f.name="str1";
	create_field(t,&f,0);

	f.type=FIELD_TYPE_INT64|FIELD_OP_NONE;
	f.mandatory=1;
	f.name="int3";
	create_field(t,&f,0);

	f.type=FIELD_TYPE_UINT64|FIELD_OP_NONE;
	f.mandatory=1;
	f.name="int4";
	create_field(t,&f,0);

	f.type=FIELD_TYPE_BYTES|FIELD_OP_NONE;
	f.mandatory=1;
	f.name="byte1";
	create_field(t,&f,0);

	f.type=FIELD_TYPE_FLT10|FIELD_OP_NONE;
	f.mandatory=1;
	f.name="scaled1";
	create_field(t,&f,0);
}
