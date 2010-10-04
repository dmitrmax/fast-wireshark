/*!
 * \file  fast.h
 * \brief shared definitions
 *
 * Shared include file for other headers.
 * Shared constants and function prototypes.
 */

#ifndef FAST_H_INCLUDED_
#define FAST_H_INCLUDED_

#include "debug.h"

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

#endif

