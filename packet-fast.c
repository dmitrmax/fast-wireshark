/**
	@file	packet-fast.c
	@brief	boilerplate plugin code for wireshark
	@author	Wes Fournier

	This file contains the actual hooks that wireshark calls to talk to
	our plugin.  both the file name and many of the functions have to be
	named very specifically so the wireshark build script and
	dynamic linking system will work, so dont go changing names!

	The actual main dissection code is in dissect.c, and the code that
	sets up the field tables and gui stuff is in setup.c
*/

/* need our internal prototypes */
#include "fast.h"

/* generated configuration */
#ifdef HAVE_CONFIG_H
#	include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gmodule.h>
#include <epan/prefs.h>
#include <epan/packet.h>

/* define a version if we are not being built statically, as
wireshark expects it to exist in shared modules */
#ifndef ENABLE_STATIC
G_MODULE_EXPORT const gchar version[] = "0.1";
#endif

/* global id of our protocol plugin used by wireshark */
static int proto_fast=-1;

/* handle to our dissector */
static dissector_handle_t fast_handle;

/* forward declarations */
void proto_register_fast(void);
void proto_reg_handoff_fast(void);
void dissect_fast(tvbuff_t*, packet_info*, proto_tree*);

/* these are hooks that have to be here if this plugin is being
dynamically linked to */
#ifndef ENABLE_STATIC
G_MODULE_EXPORT void plugin_register(void)
{
	if(proto_fast==-1)
	{
		proto_register_fast();
	}
}

G_MODULE_EXPORT void plugin_reg_handoff(void)
{
	proto_reg_handoff_fast();
}
#endif

/* used for setting up the information subtree */
static int ett_fast=-1;
static int ett_fast_text=-1;
static int* ett[] = {&ett_fast, &ett_fast_text};

static int hf_fast=-1;
static int hf_fast_text=-1;

/* registers our plugin */
void proto_register_fast(void)
{
    static hf_register_info hf[] =
    {
        { &hf_fast,
            {"Data", "fast.data", FT_NONE, BASE_NONE,
                NULL, 0x0, "FAST PDU", HFILL}},
        { &hf_fast_text,
            { "Text", "fast.text", FT_STRING, BASE_NONE,
                NULL, 0x0, "Text", HFILL }}
    };

	/* only do this if we haven't already */
	if(proto_fast==-1)
	{
        module_t* mod;
		proto_fast=proto_register_protocol(
			"FAST (FIX Adjusted for STreaming) Protocol", /* long name */
			"FAST", /* short name */
			"fast" /* abbrev */
			);

		/* registers our module's dissector registration hook */
		mod=prefs_register_protocol(
			proto_fast,proto_reg_handoff_fast);

        /* registers our field array */
        proto_register_field_array (proto_fast, hf, array_length (hf));

		/* registers our subtree array */
		proto_register_subtree_array(ett,array_length(ett));

		/* our actual setup function defined in setup.c */
		FAST_setup(proto_fast);
	}
}

/* setup related to our dissector */
void proto_reg_handoff_fast(void)
{
	static int initialized=FALSE;
	if(!initialized)
	{
		/* create our dissector */
		fast_handle=create_dissector_handle(dissect_fast,proto_fast);

		/* tell wireshark what underlying protocol and port we use */
		dissector_add(FAST_PROTO,FAST_PORT,fast_handle);
	}
}

/* this is the actual hook to the dissection function */
void dissect_fast(tvbuff_t* tvb, packet_info* pinfo, proto_tree* tree)
{
	/* fill in protocol column */
	if(check_col(pinfo->cinfo,COL_PROTOCOL))
	{
		col_set_str(pinfo->cinfo,COL_PROTOCOL,"FAST");
	}

	/* clear anything out of the info column */
	if(check_col(pinfo->cinfo,COL_INFO))
	{
		col_clear(pinfo->cinfo,COL_INFO);
	}

	/* only do dissection if we are asked */
	if(tree)
	{
		proto_item* ti=NULL;
        ti=proto_tree_add_item(tree,proto_fast,tvb,0,-1,FALSE);
		tree=proto_item_add_subtree(ti,ett_fast);

        {
            gint length;
            guint8 *str;
            length=tvb_reported_length(tvb);
            str=tvb_get_ephemeral_string(tvb, 0, length);
            proto_tree_add_string(tree, hf_fast_text, tvb, 0, length, (gchar*) str);
        }

		/* call our actual dissection function */
		FAST_dissect(proto_fast,tvb,0,pinfo,tree);
	}
}
