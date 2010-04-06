/**
	@file	fast.h
	@brief	shared definitions
	@author	Wes Fournier

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

#define FAST_PROTO "udp.port"
#define FAST_PORT 1337

#define ERR_TRUE	 0
#define ERR_FAIL	-1
#define ERR_BADARG	-2
#define ERR_NOMEM	-3
#define ERR_BADFMT	-4
#define ERR_NOTIMPL	-5

void FAST_setup(int);

void FAST_dissect(int,tvbuff_t*,int,packet_info*,proto_tree*);

void parse_xml(const char*);

/* handles to common packet field tree elements */
extern int ett_fast;
extern int ett_fast_tid;
extern int hf_fast;
extern int hf_fast_tid;

/* configuration variables */
extern guint config_port_number;
extern const char* config_template_xml_path;

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

#define DBG_RET(r)	DBG1("Returned %d",r)

#define TRACE() DBG0("trace")

/*
#define DBG(...)		debug_out(__FILE__,__func__,__LINE__,## __VA_ARGS__)
*/

#endif

