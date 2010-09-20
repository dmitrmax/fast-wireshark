/**
	@file	fast.h
	@brief	shared definitions

	Shared include file for other headers
	shared constants and function prototypes from dissect.c and setup.c
*/

#ifndef FAST_H_INCLUDED_
#define FAST_H_INCLUDED_

#ifdef HAVE_CONFIG_H
#	include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gmodule.h>
#include <epan/prefs.h>
#include <epan/packet.h>

#define ERR_TRUE	 0
#define ERR_FAIL	-1
#define ERR_BADARG	-2
#define ERR_NOMEM	-3
#define ERR_BADFMT	-4
#define ERR_NOTIMPL	-5

GNode* FAST_setup ();

void parse_xml(const char*);

void debug_out(
	const char*,
	const char*,
	int,
	const char* fmt,
	...);

#ifdef _MSC_VER
#	define __func__ __FUNCTION__
#endif

#define DBG0(s)			debug_out(__FILE__,__func__,__LINE__,s)
#define DBG1(s,a)		debug_out(__FILE__,__func__,__LINE__,s,a)
#define DBG2(s,a,b)		debug_out(__FILE__,__func__,__LINE__,s,a,b)
#define DBG3(s,a,b,c)	debug_out(__FILE__,__func__,__LINE__,s,a,b,c)
#define DBG4(s,a,b,c,d)	debug_out(__FILE__,__func__,__LINE__,s,a,b,c,d)
#define DBG5(s,a,b,c,d,e) \
	debug_out(__FILE__,__func__,__LINE__,s,a,b,c,d,e)

#define DBG_RET(r)	DBG1("Returned %d",r)

#define BAILOUT(r,s) \
  do { \
    DBG1("%s", s); \
    return r; \
  } while (0)

#define TRACE() DBG0("trace")

/*
#define DBG(...)		debug_out(__FILE__,__func__,__LINE__,## __VA_ARGS__)
*/

#endif

