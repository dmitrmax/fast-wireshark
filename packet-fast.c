/*!
 * \file packet-fast.c
 * \brief  Boilerplate plugin code for Wireshark.
 *
 * This file contains the actual hooks that wireshark calls to talk to
 * our plugin. Both the file name and many of the functions have to be
 * named very specifically so the wireshark build script and
 * dynamic linking system will work, so dont go changing names!
 *
 * The actual main dissection code is in dissect.c, and the code that
 * sets up the field tables and gui stuff is in setup.c
 */

/* Generated configuration. */
#ifdef HAVE_CONFIG_H
#	include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gmodule.h>
#include <epan/prefs.h>
#include <epan/packet.h>

#include "dissect.h"
#include "fast.h"
#include "parse-template.h"
#include "template.h"

/*! \brief  If not build statically, define a version.
 *
 * Wireshark requires this.
 */
#ifndef ENABLE_STATIC
G_MODULE_EXPORT const gchar version[] = "0.1";
#endif

/*! Global id of our protocol plugin used by Wireshark. */
static int proto_fast = -1;
/* Initialize the protocol and registered fields. */
static int hf_fast[FieldTypeEnumLimit];
static int hf_fast_tid      = -1;

/* Initialize the subtree pointer. */
static gint ett_fast = -1;


/****** Preference controls ******/
/*! Port number to use. */
static guint config_port_number = 0;
/*! Template xml file, absolute or relative pathname. */
static const char* config_template_xml_path = 0;


/*** Forward declarations. ***/
static void proto_register_fast ();
static void proto_reg_handoff_fast ();
static void dissect_fast (tvbuff_t*, packet_info*, proto_tree*);
static void display_message (tvbuff_t* tvb, proto_tree* tree,
                             const GNode* tmpl, const GNode* parent);
static void display_fields (tvbuff_t* tvb, proto_tree* tree,
                            const GNode* tnode, const GNode* dnode);


/*** Required hooks if the plugin is dynamically linked to. ***/
#ifndef ENABLE_STATIC
G_MODULE_EXPORT
void plugin_register()
{
  if (proto_fast == -1) {
    proto_register_fast();
  }
}

G_MODULE_EXPORT
void plugin_reg_handoff()
{
  proto_reg_handoff_fast();
}
#endif


/*! \brief  Register the plugin with Wireshark.
 * This function is called by Wireshark 
 */
void proto_register_fast ()
{
  /* Header fields which always exist. */
  static hf_register_info hf[] =
  {
    { &hf_fast[FieldTypeUInt32],        { "uInt32",     "fast.uint32",   FT_UINT32, BASE_DEC,   NULL, 0, "", HFILL } },
    { &hf_fast[FieldTypeUInt64],        { "uInt64",     "fast.uint64",   FT_UINT64, BASE_DEC,   NULL, 0, "", HFILL } },
    { &hf_fast[FieldTypeInt32],         { "int32",      "fast.int32",    FT_INT32,  BASE_DEC,   NULL, 0, "", HFILL } },
    { &hf_fast[FieldTypeInt64],         { "int64",      "fast.int64",    FT_INT64,  BASE_DEC,   NULL, 0, "", HFILL } },
    { &hf_fast[FieldTypeDecimal],       { "decimal",    "fast.decimal",  FT_NONE,   BASE_NONE,  NULL, 0, "", HFILL } },
    { &hf_fast[FieldTypeAsciiString],   { "ascii",      "fast.ascii",    FT_NONE,   BASE_NONE,  NULL, 0, "", HFILL } },
    { &hf_fast[FieldTypeUnicodeString], { "unicode",    "fast.unicode",  FT_NONE,   BASE_NONE,  NULL, 0, "", HFILL } },
    { &hf_fast[FieldTypeByteVector],    { "byteVector", "fast.bytevec",  FT_NONE,   BASE_NONE,  NULL, 0, "", HFILL } },
    { &hf_fast[FieldTypeGroup],         { "group",      "fast.group",    FT_NONE,   BASE_NONE,  NULL, 0, "", HFILL } },
    { &hf_fast[FieldTypeSequence],      { "sequence",   "fast.sequence", FT_NONE,   BASE_NONE,  NULL, 0, "", HFILL } },
    { &hf_fast_tid,                     { "tid",        "fast.tid",      FT_UINT32, BASE_DEC,   NULL, 0, "", HFILL } }
  };
  /* Subtree array. */
  static gint *ett[] = {
    &ett_fast
  };
  module_t* module;

  if (proto_fast != -1)  return;

  /* Register long, short, and abbreviated forms of the protocol name. */
  proto_fast =
    proto_register_protocol("FAST (FIX Adapted for STreaming) Protocol",
                            "FAST",
                            "fast");

  /* Register header fields and subtree. */
  proto_register_field_array(proto_fast, hf, array_length(hf));
  proto_register_subtree_array(ett, array_length(ett));


  /* registers our module's dissector registration hook */
  module = prefs_register_protocol(proto_fast,
                                   proto_reg_handoff_fast);

  prefs_register_uint_preference(module,
                                 "port",
                                 "FAST listen port",
                                 "Enter a valid port number (1024-65535)",
                                 10,
                                 &config_port_number);
  prefs_register_string_preference(module,
                                   "template",
                                   "FAST XML Template file",
                                   "Enter a valid filesystem path",
                                   &config_template_xml_path);

  register_dissector("fast", &dissect_fast, proto_fast);
}

/*! \brief  Set user preferences.
 *
 * Port and template file are currently the only prefs being set.
 * This function is called by Wireshark when
 * 1. The program starts up.
 * 2. Preferences for the plugin have changed.
 */
void proto_reg_handoff_fast ()
{
  static gboolean initialized = FALSE;
  static guint currentPort = 0;
  static dissector_handle_t fast_handle;
  const char* portField = "udp.port";

  if (!initialized) {
    fast_handle = create_dissector_handle(&dissect_fast, proto_fast);

    /* TODO: REMOVEME */
    /* Hard coded template setup. */
    /* add_templates (FAST_setup()); */
  }
  else {
    dissector_delete(portField, currentPort, fast_handle);
  }

  /* Set up port number. */
  if (initialized && config_port_number == 0) {
    config_port_number = 1337;
    fprintf(stderr, "FAST - WARNING: Port is not set, using default %u\n", config_port_number);
  }
  currentPort = config_port_number;

  /* Read templates file. */
  if (initialized && config_template_xml_path) {
    GNode* templates;
    printf("Using xml file %s ...\n",config_template_xml_path);
    templates = parse_templates_xml (config_template_xml_path);
    if (templates) {
      add_templates(templates);
    }
  }

  /* Tell Wireshark what underlying protocol and port we use. */
  dissector_add(portField, config_port_number, fast_handle);

  if (!initialized) {
    initialized = TRUE;
  }
}

/*! \brief Hook function that Wireshark calls to dissect a packet.
 */
void dissect_fast(tvbuff_t* tvb, packet_info* pinfo, proto_tree* tree)
{
  /* fill in protocol column */
  if (check_col(pinfo->cinfo, COL_PROTOCOL)) {
    col_set_str(pinfo->cinfo, COL_PROTOCOL, "FAST");
  }

  /* clear anything out of the info column */
  if (check_col(pinfo->cinfo, COL_INFO)) {
    col_clear(pinfo->cinfo, COL_INFO);
  }

  /* Only do dissection if we are asked */
  if (tree)
  {
    proto_item* ti;
    proto_tree* fast_tree;

    guint nbytes;
    guint8* bytes;
    const GNode* tmpl;
    GNode* parent;

    parent = g_node_new(0);
    if (!parent) {
      BAILOUT(;,"Could not allocate memory.");
    }

    /* Create display subtree. */
    ti = proto_tree_add_item(tree, proto_fast, tvb, 0, -1, ENC_NA);
    fast_tree = proto_item_add_subtree(ti, ett_fast);

    /* Dissect the payload. */
    nbytes = tvb_length (tvb);
    bytes = ep_tvb_memdup (tvb, 0, nbytes);
    tmpl = dissect_fast_bytes (nbytes, bytes, parent);

    /* Setup for display. */
    display_message (tvb, fast_tree, tmpl, parent);
  }
}

/*! \brief  Store all message data in a proto_tree.
 */
void display_message (tvbuff_t* tvb, proto_tree* tree,
                      const GNode* tmpl, const GNode* parent)
{
  if (tmpl) {
    proto_item* item;
    proto_tree* newtree;
    const FieldType* ftype;
    const FieldData* fdata;
    ftype = (FieldType*) tmpl->data;
    fdata = (FieldData*) parent->data;

    item = proto_tree_add_uint(tree, hf_fast_tid, tvb,
                               fdata->start, fdata->nbytes,
                               ftype->id);

    newtree = proto_item_add_subtree(item, ett_fast);
    display_fields(tvb, newtree,
                   tmpl->children, parent->children);
  }
}

/*! \brief  Add field data to a proto_tree.
 */
void display_fields (tvbuff_t* tvb, proto_tree* tree,
                     const GNode* tnode, const GNode* dnode)
{
  if (!dnode) {
    BAILOUT(;,"Data node is null!");
  }
  while (tnode) {
    int header_field = -1;
    const FieldType* ftype;
    const FieldData* fdata;
    ftype = (FieldType*) tnode->data;
    fdata = (FieldData*) dnode->data;
    if (ftype->type < FieldTypeEnumLimit) {
      header_field = hf_fast[ftype->type];
    }
    if (fdata->value || dnode->children) {
      switch (ftype->type) {
        case FieldTypeUInt32:
          proto_tree_add_uint(tree, header_field, tvb,
                              fdata->start, fdata->nbytes,
                              *(guint32*) fdata->value);
          break;
        case FieldTypeUInt64:
          proto_tree_add_uint64(tree, header_field, tvb,
                                fdata->start, fdata->nbytes,
                                *(guint64*) fdata->value);
          break;
        case FieldTypeInt32:
          proto_tree_add_int(tree, header_field, tvb,
                             fdata->start, fdata->nbytes,
                             *(gint32*) fdata->value);
          break;
        case FieldTypeInt64:
          proto_tree_add_int64(tree, header_field, tvb,
                               fdata->start, fdata->nbytes,
                               *(gint64*) fdata->value);
          break;
        case FieldTypeDecimal:
          {
            GNode* node;
            gint32 expt = 0;
            gint64 mant = 0;

            node = dnode->children;
            if (node) {
              expt = *(gint32*) ((FieldData*) node->data) -> value;
              node = node->next;
            }
            if (node) {
              mant = *(gint64*) ((FieldData*) node->data) -> value;
            }

            proto_tree_add_none_format(tree, header_field, tvb,
                                       fdata->start, fdata->nbytes,
                                       "decimal: %" G_GINT64_MODIFIER "de%d",
                                       mant, expt);
          }
          break;
        case FieldTypeAsciiString:
          {
            guint8* str;
            str = (guint8*) g_malloc ((1+fdata->nbytes) * sizeof(guint8));
            if (str) {
              memcpy (str, fdata->value, fdata->nbytes * sizeof(guint8));
              str[fdata->nbytes] = 0;
              proto_tree_add_none_format(tree, header_field, tvb,
                                         fdata->start, fdata->nbytes,
                                         "ascii: %s", str);
              g_free (str);
            }
            else {
              DBG0("Error allocating memory.");
              proto_tree_add_none_format(tree, header_field, tvb,
                                         fdata->start, fdata->nbytes,
                                         "ascii: %s", "");
            }
          }
          break;
        case FieldTypeUnicodeString:
          {
            guint8* str;
            str = (guint8*) g_malloc ((1+fdata->nbytes) * sizeof(guint8));
            if (str) {
              memcpy (str, fdata->value, fdata->nbytes * sizeof(guint8));
              str[fdata->nbytes] = 0;
              proto_tree_add_none_format(tree, header_field, tvb,
                                         fdata->start, fdata->nbytes,
                                         "unicode: %s", str);
              g_free (str);
            }
            else {
              DBG0("Error allocating memory.");
              proto_tree_add_none_format(tree, header_field, tvb,
                                         fdata->start, fdata->nbytes,
                                         "unicode: %s", "");
            }
          }
          break;
        case FieldTypeByteVector:
          {
            guint8* str;
            str = (guint8*) g_malloc ((1+2*fdata->nbytes) * sizeof(guint8));
            if (str) {
              guint i;
              const guint8* bytes;
              bytes = (guint8*) fdata->value;
              for (i = 0; i < fdata->nbytes; ++i) {
                g_snprintf ((gchar*)(2*i + str), 2*sizeof(guint8),
                            "%x", bytes[i]);
              }
              proto_tree_add_none_format(tree, header_field, tvb,
                                         fdata->start, fdata->nbytes,
                                         "byteVector: %s", str);
              g_free (str);
            }
            else {
              DBG0("Error allocating memory.");
              proto_tree_add_none_format(tree, header_field, tvb,
                                         fdata->start, fdata->nbytes,
                                         "byteVector: %s", "");
            }
          }
          break;
        case FieldTypeGroup:
          {
            proto_item* item;
            proto_tree* subtree;
            item = proto_tree_add_none_format(tree, header_field, tvb,
                                              fdata->start, fdata->nbytes,
                                              "group:");

            subtree = proto_item_add_subtree(item, ett_fast);
            display_fields (tvb, subtree, tnode->children, dnode->children);
          }
          break;
        case FieldTypeSequence:
          {
            proto_item* item;
            proto_tree* subtree;
            GNode* group_dnode;
            item = proto_tree_add_none_format(tree, header_field, tvb,
                                              fdata->start, fdata->nbytes,
                                              "sequence:");

            subtree = proto_item_add_subtree(item, ett_fast);
            /* Loop thru each child group in the data tree,
             * using the same child group in the type tree.
             */
            for (group_dnode = dnode->children;
                 group_dnode;
                 group_dnode = group_dnode->next) {
              display_fields (tvb, subtree, tnode->children, group_dnode);
            }
          }
          break;
        default:
          DBG1("Bad field type %u", ftype->type);
          break;
      }
    }
    else {
      /* The field is empty. */
      proto_tree_add_none_format(tree, header_field, tvb,
                                 fdata->start, fdata->nbytes,
                                 "(empty) %s:", field_typename(ftype->type));
    }

    tnode = tnode->next;
    dnode = dnode->next;
  }
}

