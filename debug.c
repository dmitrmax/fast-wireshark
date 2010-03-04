/**
	@file	debug.c
	@brief	|
	@author	Wes Fournier

	|
*/

#include "fast.h"

#include <stdarg.h>
#include <stdio.h>

void debug_out(
	const char* file,
	const char* func,
	int line,
	const char* fmt,
	...)
{
	char buf[1024];

	snprintf(buf,1024,"%s(%d) %s: %s\n",file,line,func,fmt);

	va_list args;
	va_start(args,fmt);

	vprintf(buf,args);

	va_end(args);
}
