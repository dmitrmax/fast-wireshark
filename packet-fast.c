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

#include <glib.h>
#include <gmodule.h>
#include <epan/prefs.h>
#include <epan/packet.h>

#include "debug.h"
#include "dissect.h"
#include "parse-template.h"
#include "template.h"
#include "dictionaries.h"



/*! \brief  If not build statically, define a version.
 *
 * Wireshark requires this.
 */
#ifndef ENABLE_STATIC
G_MODULE_EXPORT const gchar version[] = "0.1";
#endif

#define UN_NAMED "un-named"

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
/*! Shows empty fields in data tree if true */
static gboolean show_empty_optional_fields = 1;
/*! If true does not capture or dissect packets */
static gboolean enabled = 0;

enum ProtocolImplem { GenericImplem, CMEImplem, UMDFImplem, NImplem };
/*! The specific implementation of FAST to use. */
static gint implementation = 0;
/*! Table to hold pointers to previously parsed packets for Non-sequental Disection */
static GHashTable* parsed_packets_table = 0;

struct parsed_packet_data
{
  GNode * dataTree;
  const GNode * tmpl;
  guint32 frameNum;
};
typedef struct parsed_packet_data PacketData;


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
    { &hf_fast[FieldTypeUInt32],        { "uInt32",     "fast.uint32",     FT_NONE, BASE_NONE, NULL, 0, "", HFILL } },
    { &hf_fast[FieldTypeUInt64],        { "uInt64",     "fast.uint64",     FT_NONE, BASE_NONE, NULL, 0, "", HFILL } },
    { &hf_fast[FieldTypeInt32],         { "int32",      "fast.int32",      FT_NONE, BASE_NONE, NULL, 0, "", HFILL } },
    { &hf_fast[FieldTypeInt64],         { "int64",      "fast.int64",      FT_NONE, BASE_NONE, NULL, 0, "", HFILL } },
    { &hf_fast[FieldTypeDecimal],       { "decimal",    "fast.decimal",    FT_NONE, BASE_NONE, NULL, 0, "", HFILL } },
    { &hf_fast[FieldTypeAsciiString],   { "ascii",      "fast.ascii",      FT_NONE, BASE_NONE, NULL, 0, "", HFILL } },
    { &hf_fast[FieldTypeUnicodeString], { "unicode",    "fast.unicode",    FT_NONE, BASE_NONE, NULL, 0, "", HFILL } },
    { &hf_fast[FieldTypeByteVector],    { "byteVector", "fast.bytevector", FT_NONE, BASE_NONE, NULL, 0, "", HFILL } },
    { &hf_fast[FieldTypeGroup],         { "group",      "fast.group",      FT_NONE, BASE_NONE, NULL, 0, "", HFILL } },
    { &hf_fast[FieldTypeSequence],      { "sequence",   "fast.sequence",   FT_NONE, BASE_NONE, NULL, 0, "", HFILL } },
    { &hf_fast_tid,                     { "tid",        "fast.tid",        FT_NONE, BASE_NONE, NULL, 0, "", HFILL } }
  };
  static enum_val_t radio_buttons[] = 
  {
    { "Generic", "Generic", GenericImplem },
    { "CME", "CME", CMEImplem },
    { "UMDF", "UMDF", UMDFImplem },
    { 0, 0, 0 }
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

  prefs_register_bool_preference(module,
                                   "enabled",
                                   "Plugin Enabled",
                                   "Check if you want the plugin to capture and dissect packets",
                                   &enabled);
                                   
  prefs_register_uint_preference(module,
                                 "port",
                                 "Listen port",
                                 "Enter a valid port number (1024-65535)",
                                 10,
                                 &config_port_number);
  prefs_register_string_preference(module,
                                   "template",
                                   "XML template file",
                                   "Enter a valid filesystem path",
                                   &config_template_xml_path);

  prefs_register_bool_preference(module,
                                   "show_empty",
                                   "Show empty optional fields",
                                   "Check if you want to see fields that are empty and were not sent in the packet",
                                   &show_empty_optional_fields);
                                   
                                  
  prefs_register_enum_preference(module,
                                  "implementation",
                                  "FAST Implementation",
                                  "Select the specific implementation of FAST",
                                  &implementation,
                                  radio_buttons,
                                  FALSE);
    

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
  
  if(enabled && !initialized){
    fast_handle = create_dissector_handle(&dissect_fast, proto_fast);
    initialized = TRUE;
  }
  
  if(enabled && initialized){
  
    dissector_delete(portField, currentPort, fast_handle);

    /* Set up port number. */
    if (initialized && config_port_number == 0) {
      config_port_number = 1337;
      fprintf(stderr, "FAST - WARNING: Port is not set, using default %u\n", config_port_number);
    }
    currentPort = config_port_number;

    /* Read templates file. */
    if (initialized && config_template_xml_path) {
      GNode* templates;
      fprintf(stderr, "Using xml file %s ...\n",config_template_xml_path);
      templates = parse_templates_xml (config_template_xml_path);
      set_dictionaries(templates);
      if (templates) {
        add_templates(templates);
      }
    }

    /* Tell Wireshark what underlying protocol and port we use. */
    dissector_add(portField, config_port_number, fast_handle);
  
  } else {
    dissector_delete(portField, currentPort, fast_handle);  
  }
  
}

/*! \brief Hook function that Wireshark calls to dissect a packet.
 */
void dissect_fast(tvbuff_t* tvb, packet_info* pinfo, proto_tree* tree)
{
  frame_data *frameData;
  gpointer * packetParsed = 0;
  PacketData * packetData;
  frameData = pinfo->fd;
  
  
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

    /* Create display subtree. */
    ti = proto_tree_add_item(tree, proto_fast, tvb, 0, -1, ENC_NA);
    fast_tree = proto_item_add_subtree(ti, ett_fast);
    
    if(!parsed_packets_table) {
      parsed_packets_table = g_hash_table_new(&g_int_hash, &g_int_equal);
      /* If we fail to make the table bail */
      if(!parsed_packets_table) { BAILOUT(;,"Packet lookup table not created."); }
    }
  
    /* check if this packet has already been parsed and get it if it has */
    packetParsed = g_hash_table_lookup(parsed_packets_table, &(frameData->num));
    
    if (packetParsed) {
      /* packet in dict so display original parsed packet*/
      packetData = (PacketData*) packetParsed;
      parent = packetData->dataTree;
      tmpl = packetData->tmpl;
      
    } else {
      guint header_offset = 0;
      parent = g_node_new(0);
      if (!parent) {
        BAILOUT(;,"Could not allocate memory.");
      }

      /* Dissect the payload. */
      nbytes = tvb_length (tvb);
      
      if (implementation == CMEImplem) {
        header_offset = 5;
      }
      else if (implementation == UMDFImplem) {
        header_offset = 10;
      }

      bytes = ep_tvb_memdup (tvb, 0, nbytes);
      tmpl = dissect_fast_bytes (nbytes, bytes, parent, header_offset);
           
      /* Store pointers to display tree so it can be loaded if user clicks on this packet again */
      packetData = (PacketData*)malloc(sizeof(PacketData));
      packetData->dataTree = parent;
      packetData->tmpl = tmpl;
      packetData->frameNum = frameData->num;
      
      if (implementation == CMEImplem || implementation == UMDFImplem) {
        set_dictionaries(full_templates_tree());
      }
      
      g_hash_table_insert(parsed_packets_table, &(packetData->frameNum), packetData);
    }
    
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
    char * field_name;
    ftype = (FieldType*) tmpl->data;
    fdata = (FieldData*) parent->data;
    if(ftype->name){
      field_name = ftype->name;
    } else {
      field_name = UN_NAMED;
    }

    item = proto_tree_add_none_format(tree, hf_fast_tid, tvb,
                                      fdata->start, fdata->nbytes,
                                      "%s - tid: %d", field_name, ftype->id);

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
    char * field_name;
    ftype = (FieldType*) tnode->data;
    fdata = (FieldData*) dnode->data;
    if(ftype->name){
      field_name = ftype->name;
    } else {
      field_name = UN_NAMED;
    }
    
    if (ftype->type < FieldTypeEnumLimit) {
      header_field = hf_fast[ftype->type];
    }
    if (!fdata->empty) {
      switch (ftype->type) {
        case FieldTypeUInt32:
          /*type - name (id): value*/
          proto_tree_add_none_format(tree, header_field, tvb,
                                     fdata->start, fdata->nbytes,
                                     "uInt32 - %s (%d): %u",
                                     field_name,
                                     ftype->id,
                                     fdata->value.u32);
          break;
        case FieldTypeUInt64:
          proto_tree_add_none_format(tree, header_field, tvb,
                                     fdata->start, fdata->nbytes,
                                     "uInt64 - %s (%d): %" G_GINT64_MODIFIER "u",
                                     field_name,
                                     ftype->id,
                                     fdata->value.u64);
          break;
        case FieldTypeInt32:
          proto_tree_add_none_format(tree, header_field, tvb,
                                     fdata->start, fdata->nbytes,
                                     "int32 - %s (%d): %d",
                                     field_name,
                                     ftype->id,
                                     fdata->value.i32);
          break;
        case FieldTypeInt64:
          proto_tree_add_none_format(tree, header_field, tvb,
                                     fdata->start, fdata->nbytes,
                                     "int64 - %s (%d): %" G_GINT64_MODIFIER "d",
                                     field_name,
                                     ftype->id,
                                     fdata->value.i64);
          break;
        case FieldTypeDecimal:
          proto_tree_add_none_format(tree, header_field, tvb,
                                     fdata->start, fdata->nbytes,
                                     "decimal - %s (%d): %" G_GINT64_MODIFIER "de%d",
                                     field_name,
                                     ftype->id,
                                     fdata->value.decimal.mantissa,
                                     fdata->value.decimal.exponent);
          break;
        case FieldTypeAsciiString:
          proto_tree_add_none_format(tree, header_field, tvb,
                                     fdata->start, fdata->nbytes,
                                     "ascii - %s (%d): %s", field_name,
                                     ftype->id,
                                     fdata->value.ascii.bytes);
          break;
        case FieldTypeUnicodeString:
          proto_tree_add_none_format(tree, header_field, tvb,
                                     fdata->start, fdata->nbytes,
                                     "unicode - %s (%d): %s", field_name,
                                     ftype->id,
                                     fdata->value.unicode.bytes);
          break;
        case FieldTypeByteVector:
          {
            guint8* str;
            const SizedData* vec;
            vec = &fdata->value.bytevec;
            str = (guint8*) g_malloc ((1+2*vec->nbytes) * sizeof(guint8));
            if (str) {
              guint i;
              for (i = 0; i < vec->nbytes; ++i) {
                g_snprintf ((gchar*)(2*i + str), 3*sizeof(guint8),
                            "%02x", vec->bytes[i]);
              }
              str[2*vec->nbytes] = 0;
              proto_tree_add_none_format(tree, header_field, tvb,
                                         fdata->start, fdata->nbytes,
                                         "byteVector - %s (%d): %s", field_name,
                                         ftype->id,
                                         str);
              g_free (str);
            }
            else {
              DBG0("Error allocating memory.");
              proto_tree_add_none_format(tree, header_field, tvb,
                                         fdata->start, fdata->nbytes,
                                         "byteVector - %s (%d): %s", field_name,
                                         ftype->id,
                                         "");
            }
          }
          break;
        case FieldTypeGroup:
          {
            proto_item* item;
            proto_tree* subtree;
            item = proto_tree_add_none_format(tree, header_field, tvb,
                                              fdata->start, fdata->nbytes,
                                              "group - %s (%d):", field_name,
                                              ftype->id
                                              );

            subtree = proto_item_add_subtree(item, ett_fast);
            display_fields (tvb, subtree, tnode->children, dnode->children);
          }
          break;
        case FieldTypeSequence:
          {
            proto_item* item;
            proto_tree* subtree;
            GNode* length_tnode;
            item = proto_tree_add_none_format(tree, header_field, tvb,
                                              fdata->start, fdata->nbytes,
                                              "sequence - %s (%d):", field_name,
                                              ftype->id
                                              );

            subtree = proto_item_add_subtree(item, ett_fast);

            length_tnode = tnode->children;
            if (length_tnode) {
              GNode* group_tnode;
              GNode* group_dnode;
              group_tnode = length_tnode->next;
              /* Loop thru each child group in the data tree,
               * using the same child group in the type tree.
               */
              for (group_dnode = dnode->children;
                   group_dnode;
                   group_dnode = group_dnode->next) {
                display_fields (tvb, subtree, group_tnode, group_dnode);
              }
            }
            else {
              DBG0("Sequence has no children in field type tree.");
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
      if(show_empty_optional_fields){
        proto_tree_add_none_format(tree, header_field, tvb,
                                 fdata->start, fdata->nbytes,
                                 "%s (empty %s)",
                                 field_name,
                                 field_typename(ftype->type));                           
      }
    }

    tnode = tnode->next;
    dnode = dnode->next;
  }
}

