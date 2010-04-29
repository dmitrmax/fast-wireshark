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

	create_template("TestTemplate1",1,&t);

	f.type=FIELD_TYPE_INT32|FIELD_OP_NONE;
	f.mandatory=1;
	f.name="a";
	create_field(t,&f,0);

	f.type=FIELD_TYPE_INT32|FIELD_OP_NONE;
	f.mandatory=1;
	f.name="b";
	create_field(t,&f,0);

	f.type=FIELD_TYPE_INT32|FIELD_OP_NONE;
	f.mandatory=1;
	f.name="c";
	create_field(t,&f,0);

	f.type=FIELD_TYPE_INT64|FIELD_OP_NONE;
	f.mandatory=1;
	f.name="x";
	create_field(t,&f,0);

	f.type=FIELD_TYPE_INT64|FIELD_OP_NONE;
	f.mandatory=1;
	f.name="y";
	create_field(t,&f,0);

	f.type=FIELD_TYPE_INT64|FIELD_OP_NONE;
	f.mandatory=1;
	f.name="z";
	create_field(t,&f,0);

	f.type=FIELD_TYPE_ASCII|FIELD_OP_NONE;
	f.mandatory=1;
	f.name="name";
	create_field(t,&f,0);
}

