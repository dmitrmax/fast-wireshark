/**
	@file	setup.c
	@brief	|
	@author	Wes Fournier

	|
*/

#include "template.h"

void FAST_setup(int id)
{
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
}
