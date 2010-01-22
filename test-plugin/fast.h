/**
	@file	fast.h
	@brief	shared definitions
	@author	Wes Fournier

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

void FAST_setup(int);

void FAST_dissect(int,tvbuff_t*,int,packet_info*,proto_tree*);

#endif
