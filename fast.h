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

#define ERR_TRUE	0
#define ERR_FALSE	-1
#define ERR_BADARG	-2
#define ERR_NOMEM	-3
#define ERR_BADFMT	-4
#define ERR_NOTIMPL	-5

void FAST_setup(int);

void FAST_dissect(int,tvbuff_t*,int,packet_info*,proto_tree*);

#endif
