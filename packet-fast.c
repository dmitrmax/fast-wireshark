/*
* This file is part of FAST Wireshark.
*
* FAST Wireshark is free software: you can redistribute it and/or modify
* it under the terms of the Lesser GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* FAST Wireshark is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* Lesser GNU General Public License for more details.
*
* You should have received a copy of the Lesser GNU General Public License
* along with FAST Wireshark.  If not, see
* <http://www.gnu.org/licenses/lgpl.txt>.
*/

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
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <gmodule.h>
#include <epan/prefs.h>
#include <epan/packet.h>
#include <epan/column-info.h>
#include <epan/uat.h>

#include "debug.h"
#include "dissect.h"
#include "parse-template.h"
#include "template.h"
#include "dictionaries.h"
#include "debug-tree.h"
#include "error_log.h"

void proto_register_fast(void);
void proto_reg_handoff_fast(void);

/* User Access Table */
typedef struct _fast_port_item_t {
    guint port;
} fast_port_item_t;

/* Checks to see if a particular packet information element is needed for the packet list */
#define CHECK_COL(cinfo, el) \
    /* We are constructing columns, and they're writable */ \
    (col_get_writable(cinfo, el) && \
      /* There is at least one column in that format */ \
    ((cinfo)->col_first[el] >= 0))


#define UNNAMED "un-named"

/*! Global id of our protocol plugin used by Wireshark. */
static int proto_fast = -1;
/* Initialize the protocol and registered fields. */
static int hf_fast[FieldTypeEnumLimit];
static int hf_fast_tid        = -1;
static gboolean message_error = FALSE;

/* Initialize the subtree pointer. */
static gint ett_fast = -1;


/****** Preference controls ******/
/*! Port number to use. */
static fast_port_item_t* fast_uats = NULL;
static guint             config_n_port_items = 0;
static char * config_port_field = "udp.port";
/*! Template xml file, absolute or relative pathname. */
static const char* config_template_xml_path = NULL;
/*! Shows empty fields in data tree if true */
static gboolean show_empty_optional_fields = 1;
/*! If true does not capture or dissect packets */
static gboolean enabled = 0;
/*! If true display decimal fields in scientific notation */
static gboolean sciNotation = 1;
static gboolean showFieldKeys = 0;
static gboolean showFieldDictionaries = 0;
static gboolean showFieldOperators = 0;
static gboolean showFieldMandatoriness = 0;
static gboolean config_show_dialog_windows = 1;
static gboolean config_log_errors = 1;
static const char* config_log_file_name = NULL;
static gboolean reparse_templates = 1; /* Tells dissector to reparse the template xml file */
static uat_t   *config_port_list_uat = NULL;

enum ProtocolImplem { GenericImplem, CMEImplem, UMDFImplem, MOEXImplem, NImplem };
enum Protocol { UDPImplem, TCPImplem, NOImplem };
/*! The specific implementation of FAST to use. */
static gint implementation = 0;
/*! Default protocol is UDP (0).  Other option is TCP (1)*/
static gint protocol = 0;
/*! Table to hold pointers to previously dissected
 * packets to assure sequential disection.
 */
static GHashTable* dissected_packets_table = 0;

struct packet_data_struct
{
  GList* dataTrees;
  GList* tmplTrees;
  guint32 frameNum;
};
typedef struct packet_data_struct PacketData;


/*** Forward declarations. ***/

static int dissect_fast (tvbuff_t*, packet_info*, proto_tree*, void*);
static void display_message (tvbuff_t* tvb, proto_tree* tree,
                             const GNode* tmpl, const GNode* parent,
			     packet_info* pinfo);
static void display_fields (tvbuff_t* tvb, proto_tree* tree,
                            const GNode* tnode, const GNode* dnode,
                            packet_info* pinfo);
static char * toDecimal(gint32 expt, gint64 mant);
static char * generate_field_info(const FieldType* ftype);

UAT_DEC_CB_DEF(fast_uats, port, fast_port_item_t)

/*! \brief  Register the plugin with Wireshark.
 * This function is called by Wireshark
 */

static gboolean
fast_config_port_list_update_cb(void* r _U_, char** err)
{
    const fast_port_item_t* item = (const fast_port_item_t*)r;

    if(item->port < 65536 && item->port > 0)
        return TRUE;

    if (err)
        *err = g_strdup("Port value must be in range of 1 to 65535");
    return FALSE;
}

void proto_register_fast (void)
{
  /* Header fields which always exist. */
  static hf_register_info hf[] =
  {
    { &hf_fast[FieldTypeUInt32],        { "uInt32",     "fast.uint32",      FT_NONE,     BASE_NONE, NULL, 0, "", HFILL } },
    { &hf_fast[FieldTypeUInt64],        { "uInt64",     "fast.uint64",      FT_NONE,     BASE_NONE, NULL, 0, "", HFILL } },
    { &hf_fast[FieldTypeInt32],         { "int32",      "fast.int32",       FT_NONE,     BASE_NONE, NULL, 0, "", HFILL } },
    { &hf_fast[FieldTypeInt64],         { "int64",      "fast.int64",       FT_NONE,     BASE_NONE, NULL, 0, "", HFILL } },
    { &hf_fast[FieldTypeDecimal],       { "decimal",    "fast.decimal",     FT_NONE,     BASE_NONE, NULL, 0, "", HFILL } },
    { &hf_fast[FieldTypeAsciiString],   { "ascii",      "fast.ascii",       FT_NONE,     BASE_NONE, NULL, 0, "", HFILL } },
    { &hf_fast[FieldTypeUnicodeString], { "unicode",    "fast.unicode",     FT_NONE,     BASE_NONE, NULL, 0, "", HFILL } },
    { &hf_fast[FieldTypeByteVector],    { "byteVector", "fast.bytevector",  FT_NONE,     BASE_NONE, NULL, 0, "", HFILL } },
    { &hf_fast[FieldTypeGroup],         { "group",      "fast.group",       FT_NONE,     BASE_NONE, NULL, 0, "", HFILL } },
    { &hf_fast[FieldTypeSequence],      { "sequence",   "fast.sequence",    FT_NONE,     BASE_NONE, NULL, 0, "", HFILL } },
    { &hf_fast[FieldTypeError],         { "error",      "fast.ERROR",       FT_NONE,     BASE_NONE, NULL, 0, "Dynamic error in packet", HFILL } },
    { &hf_fast_tid,                     { "tid",        "fast.tid",         FT_NONE,     BASE_NONE, NULL, 0, "", HFILL } }

  };
  static enum_val_t radio_buttons[] =
  {
    { "Generic", "Generic", GenericImplem },
    { "CME", "CME", CMEImplem },
    { "UMDF", "UMDF", UMDFImplem },
    { "MOEX", "MOEX", MOEXImplem },
    { 0, 0, 0 }
  };
  static enum_val_t protocol_buttons[] =
  {
    { "UDP", "UDP", UDPImplem },
    { "TCP", "TCP", TCPImplem },
    { 0, 0, 0 }
  };

  static uat_field_t fast_uats_flds[] = {
      UAT_FLD_DEC(fast_uats, port, "Port", "Port Number"),
      UAT_END_FIELDS
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


  config_port_list_uat = uat_new("FAST Port List",
                                 sizeof(fast_port_item_t),
                                 "fast_ports",
                                 TRUE,
                                 (void*)&fast_uats,
                                 &config_n_port_items,
                                 UAT_AFFECTS_DISSECTION,
                                 NULL,
                                 NULL,
                                 fast_config_port_list_update_cb,
                                 NULL,
                                 proto_reg_handoff_fast,
                                 NULL,
                                 fast_uats_flds);

  /*** register plugin preferences ***/

  prefs_register_bool_preference(module,
                                   "enabled",
                                   "Plugin Enabled",
                                   "Check if you want the plugin to capture and dissect packets",
                                   &enabled);

  prefs_register_uat_preference(module,
                                "port_list",
                                "Listen port list",
                                "Enter a valid port numbers",
                                config_port_list_uat);

  prefs_register_enum_preference(module,
                                  "protocol",
                                  "Protocol",
                                  "Network Protocol",
                                  &protocol,
                                  protocol_buttons,
                                  FALSE);

  prefs_register_filename_preference(module,
                                   "template",
                                   "XML template file",
                                   "Enter a valid filesystem path",
                                   &config_template_xml_path);

  prefs_register_bool_preference(module,
                                   "show_empty",
                                   "Show empty optional fields",
                                   "Check if you want to see fields that are empty and were not sent in the packet",
                                   &show_empty_optional_fields);

  prefs_register_bool_preference(module,
                                   "sci_notation",
                                   "Show all decimals in scientific notation",
                                   "Check if you want to see all decimal fields in scientific notation",
                                   &sciNotation);


  prefs_register_enum_preference(module,
                                  "implementation",
                                  "FAST Implementation",
                                  "Select the specific implementation of FAST",
                                  &implementation,
                                  radio_buttons,
                                  FALSE);

  prefs_register_bool_preference(module,
                                  "show_field_dictionaries",
                                  "Show each field's dictionary",
                                  "Show each field's dictionary",
                                  &showFieldDictionaries);

  prefs_register_bool_preference(module,
                                  "show_field_keys",
                                  "Show each field's dictionary key",
                                  "Show each field's dictionary key",
                                  &showFieldKeys);


  prefs_register_bool_preference(module,
                                  "show_field_operators",
                                  "Show each field's operator (if any)",
                                  "Show each field's operator (if any)",
                                  &showFieldOperators);

  prefs_register_bool_preference(module,
                                  "show_field_mandatoriness",
                                  "Show if each field is mandatory",
                                  "Show if each field is mandatory",
                                  &showFieldMandatoriness);

  prefs_register_bool_preference(module,
                                  "enable_dialogs",
                                  "Enable error dialogs",
                                  "Shows global and static errors in dialog windows\ntshark WILL NOT function with this enabled",
                                  &config_log_errors);

  prefs_register_bool_preference(module,
                                  "enable_logging",
                                  "Enable logging to file",
                                  "Logs all errors to error_log.txt",
                                  &config_log_errors);

  prefs_register_filename_preference(module,
                                     "log_file_name",
                                     "Log file",
                                     "Enter a valid filesystem path",
                                     &config_log_file_name);


  register_dissector("fast", dissect_fast, proto_fast);
}

/*! \brief  Set user preferences.
 *
 * Port and template file are currently the only prefs being set.
 * This function is called by Wireshark when
 * 1. The program starts up.
 * 2. Preferences for the plugin have changed.
 */
void proto_reg_handoff_fast(void)
{
  static gboolean initialized = FALSE;
  static dissector_handle_t fast_handle;
  static char* currentTemplateXML = 0;

  fast_set_log_settings(config_show_dialog_windows, config_log_errors, config_log_file_name);

  /* listen for TCP or UDP, depending on user preference */
  if(protocol){
    config_port_field = "tcp.port";
  } else {
    config_port_field = "udp.port";
  }

  if(enabled && !initialized){
    fast_handle = create_dissector_handle(&dissect_fast, proto_fast);
    initialized = TRUE;
  }

  if(enabled && initialized){
    guint i = 0;
    for(i = 0; i < config_n_port_items; i++) {
        /* Tell Wireshark what underlying protocol and port we use. */
        dissector_add_uint(config_port_field, fast_uats[i].port, fast_handle);
    }

    /* Read templates file. */
    if (initialized && config_template_xml_path) {
        fprintf(stderr, "Using xml file %s ...\n",config_template_xml_path);
        if(g_strcmp0(config_template_xml_path, currentTemplateXML)!=0){
            reparse_templates = TRUE;
        }
    }
  }

  /* TODO: remove previous port bindings */
}

/*! \brief Hook function that Wireshark calls to dissect a packet.
 *  \param tvb the actual packet data
 *  \param pinfo metadata for this packet
 *  \param tree where we put the data we want wireshark to print
 */
int dissect_fast(tvbuff_t* tvb, packet_info* pinfo, proto_tree* tree, void* data_unused _U_)
{
  frame_data* frameData;
  PacketData* packetData;
  frameData = pinfo->fd;

  if(reparse_templates){
    GNode* templates;
    templates = parse_templates_xml (config_template_xml_path);
    add_templates(templates);
    reparse_templates = FALSE;
  }

  /* fill in protocol column */
  if (CHECK_COL(pinfo->cinfo, COL_PROTOCOL)) {
    col_set_str(pinfo->cinfo, COL_PROTOCOL, "FAST");
  }

  /* clear anything out of the info column */
  if (CHECK_COL(pinfo->cinfo, COL_INFO)) {
    col_clear(pinfo->cinfo, COL_INFO);
  }

  /* Only do dissection if we are asked */
  if (tree) {
    proto_item* ti;
    proto_tree* fast_tree;

    /* Create display subtree. */
    ti = proto_tree_add_item(tree, proto_fast, tvb, 0, -1, ENC_NA);
    fast_tree = proto_item_add_subtree(ti, ett_fast);

    if(!dissected_packets_table) {
      dissected_packets_table = g_hash_table_new(&g_int_hash, &g_int_equal);
      /* If we fail to make the table bail */
      if(!dissected_packets_table) { BAILOUT(0;,"Packet lookup table not created."); }
    }

    /* check if this packet has already been dissected and get it if it has */
    packetData = (PacketData*) g_hash_table_lookup(dissected_packets_table, &(frameData->num));

    /* if this packet has not been dissected yet, dissect it */
    if (!packetData) {
      DissectPosition stacked_position;
      DissectPosition* position;
      guint header_offset = 0;

      /* ignore headers for CME and UMDF */
      if (implementation == CMEImplem) {
        header_offset = 5;
      }
      else if (implementation == UMDFImplem) {
        header_offset = 10;
      } else if (implementation == MOEXImplem) {
        header_offset = 4;
      }

      /* Store pointers to display tree so it can be
       * loaded if user clicks on this packet again.
       */
      packetData = (PacketData*)g_malloc(sizeof(PacketData));
      packetData->frameNum = frameData->num;
      packetData->dataTrees = 0;
      packetData->tmplTrees = 0;

      position = &stacked_position;
      position->offjmp = header_offset;
      position->offset = 0;
      position->nbytes = tvb_reported_length (tvb);
      position->bytes  = (guint8*)tvb_memdup (wmem_file_scope(), tvb, 0, position->nbytes);

      ShiftBytes(position);

      while (position->nbytes) {
        GNode* data;
        GNode* tmpl;
        data = g_node_new(0);

        /* call function in dissect.c that dissects the data */
        tmpl = dissect_fast_bytes (position, data, &pinfo->src, &pinfo->dst);

        /* If no template is found for the message make a fake message/template then break out */
        if(tmpl == NULL){
          GNode* tnode;
          GNode* vnode;
          GNode* dnode;
          FieldType* tfield;
          FieldType* vfield;
          FieldData* fdata;

          /* Create a template that contains one ascii field */
          tnode = create_field(FieldTypeUInt32, FieldOperatorCopy);
          tfield = (FieldType*) tnode->data;
          tfield->name = "Error Template";
          /* Put the erronous tid in as this templates id */
          fdata = (FieldData*)data->data;
          tfield->id = fdata->value.u32;
          vnode = create_field(FieldTypeAsciiString, FieldOperatorNone);
          vfield = (FieldType*) vnode->data;
          vfield->name = "Error Message";
          vfield->id = 0;
          g_node_insert_after(tnode,0,vnode);
          tmpl = tnode;

          /* Create data for the above template that describes the error */
          fdata = (FieldData*) g_malloc(sizeof(FieldData));
          g_free(data->data);
          g_node_destroy(data);
          data = g_node_new(fdata);
          fdata->start = 0;
          fdata->nbytes = 0;
          fdata-> status = FieldEmpty;
          fdata->value.u32 = -1;
          fdata = (FieldData*) g_malloc(sizeof(FieldData));
          dnode = g_node_new(fdata);
          g_node_insert_after(data,NULL,dnode);
          fdata->start = 0;
          fdata->nbytes = 0;

          /* throw dynamic error D9: template does not exist */
          err_d(9, fdata);

          /* Stop parsing the packet as we don't know whats going on any more */
          position->nbytes = 0;
        }
        packetData->dataTrees = g_list_append(packetData->dataTrees, data);
        packetData->tmplTrees = g_list_append(packetData->tmplTrees, tmpl);
      }

      /* TODO: Issue 87 should remove this */
      if (implementation == CMEImplem || implementation == UMDFImplem || implementation == MOEXImplem) {
        /* resets the dictionaries for CME and UMDF between packets */
        clear_dictionaries(pinfo->src, pinfo->dst);
      }

      g_hash_table_insert(dissected_packets_table, &(packetData->frameNum), packetData);
    }

    /* scope created to escape compile error due to mixed code and declarations */
    {
      GList* tmplTrees;
      GList* dataTrees;
      GNode* template_node;
      GNode* parent;
      guint message_cnt = 0;
      tmplTrees = packetData->tmplTrees;
      dataTrees = packetData->dataTrees;

      while (tmplTrees && dataTrees) {
        template_node  = (GNode*) tmplTrees->data;
        parent    = (GNode*) dataTrees->data;
        message_error = FALSE;
        display_message (tvb, fast_tree, template_node, parent, pinfo);

        /* add info to the info column */
        message_cnt++;
        if (CHECK_COL(pinfo->cinfo, COL_INFO)) {
          if(message_error) {
            /* leave error message in info column */
          } else if(message_cnt > 1) {
            col_add_fstr(pinfo->cinfo, COL_INFO,"%d messages", message_cnt);
          } else {
            /* dig up template name and tid from previously dissected message */
            col_add_fstr(pinfo->cinfo, COL_INFO,"%s - tid: %d",
                         ((FieldType *)template_node->data)->name,
                         ((FieldType *)template_node->data)->id);
          }
        }

        tmplTrees = tmplTrees->next;
        dataTrees = dataTrees->next;
      }
    }
  }

  return tvb_reported_length(tvb);
}


/*! \brief  Store all message data in a proto_tree.
 *  \param tvb packet data
 *  \param tree where we store stuff for wireshark to print
 *  \param tmpl template to use
 *  \param parent top level of FAST protocol tree - says "FAST (FIX Adapted for STreaming) Protocol"
 *  \param pinfo packet metadata
 */
void display_message (tvbuff_t* tvb, proto_tree* tree,
		      const GNode* tmpl, const GNode* parent,
		      packet_info* pinfo)
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
      field_name = UNNAMED;
    }

    /* add message information to the proto_tree */
    item = proto_tree_add_none_format(tree, hf_fast_tid, tvb,
                                      fdata->start, fdata->nbytes,
                                      "%s - tid: %d", field_name, ftype->id);

    newtree = proto_item_add_subtree(item, ett_fast);
    display_fields(tvb, newtree,
                   tmpl->children, parent->children, pinfo);

  }
}



/*! \brief  Add field data to a proto_tree.
 *  \param tvb packet data
 *  \param tree where we store stuff for wireshark to display
 *  \param tnode template node
 *  \param dnode data node
 *  \param pinfo packet metadata
 */
void display_fields (tvbuff_t* tvb, proto_tree* tree,
                     const GNode* tnode, const GNode* dnode,
		     packet_info* pinfo)
{
  if (!dnode) {
    BAILOUT(;,"Data node is null!");
  }
  while (tnode) {
    int header_field = -1;
    const FieldType* ftype;
    const FieldData* fdata;
    char * field_name;
    char * field_inf;
    char * decimalNum;
    ftype = (FieldType*) tnode->data;
    fdata = (FieldData*) dnode->data;


    if(ftype->name){
      field_name = ftype->name;
    } else {
      field_name = UNNAMED;
    }

    /* Generate optional field_info string */
    field_inf = generate_field_info(ftype);

    if (ftype->type < FieldTypeEnumLimit) {
      header_field = hf_fast[ftype->type];
    }
    if (fdata->status == FieldExists) {
      /* add field values to proto_tree:
       * type - name (id): value
       */
      switch (ftype->type) {

        case FieldTypeUInt32:
          proto_tree_add_none_format(tree, header_field, tvb,
                                     fdata->start, fdata->nbytes,
                                     "uInt32 - %s (%d)%s: %u",
                                     field_name,
                                     ftype->id,
                                     field_inf,
                                     fdata->value.u32);
          break;

        case FieldTypeUInt64:
          proto_tree_add_none_format(tree, header_field, tvb,
                                     fdata->start, fdata->nbytes,
                                     "uInt64 - %s (%d)%s: %" G_GINT64_MODIFIER "u",
                                     field_name,
                                     ftype->id,
                                     field_inf,
                                     fdata->value.u64);
          break;

        case FieldTypeInt32:
          proto_tree_add_none_format(tree, header_field, tvb,
                                     fdata->start, fdata->nbytes,
                                     "int32 - %s (%d)%s: %d",
                                     field_name,
                                     ftype->id,
                                     field_inf,
                                     fdata->value.i32);
          break;

        case FieldTypeInt64:
          proto_tree_add_none_format(tree, header_field, tvb,
                                     fdata->start, fdata->nbytes,
                                     "int64 - %s (%d)%s: %" G_GINT64_MODIFIER "d",
                                     field_name,
                                     ftype->id,
                                     field_inf,
                                     fdata->value.i64);
          break;

        case FieldTypeDecimal:
          if(!sciNotation) {
            /* get the decimal representation of the field */
            decimalNum = toDecimal(fdata->value.decimal.exponent, fdata->value.decimal.mantissa);
          } else { decimalNum = NULL; }
          if(sciNotation || decimalNum==NULL ){
            proto_tree_add_none_format(tree, header_field, tvb,
                                     fdata->start, fdata->nbytes,
                                     "decimal - %s (%d)%s: %" G_GINT64_MODIFIER "de%d",
                                     field_name,
                                     ftype->id,
                                     field_inf,
                                     fdata->value.decimal.mantissa,
                                     fdata->value.decimal.exponent);
          } else {
            proto_tree_add_none_format(tree, header_field, tvb,
                                     fdata->start, fdata->nbytes,
                                     "decimal - %s (%d)%s: %s",
                                     field_name,
                                     ftype->id,
                                     field_inf,
                                     decimalNum);
          }
          break;

        case FieldTypeAsciiString:
          proto_tree_add_none_format(tree, header_field, tvb,
                                     fdata->start, fdata->nbytes,
                                     "ascii - %s (%d)%s: %s", field_name,
                                     ftype->id,
                                     field_inf,
                                     fdata->value.ascii.bytes);
          break;

        case FieldTypeUnicodeString:
          proto_tree_add_none_format(tree, header_field, tvb,
                                     fdata->start, fdata->nbytes,
                                     "unicode - %s (%d)%s: %s", field_name,
                                     ftype->id,
                                     field_inf,
                                     fdata->value.unicode.bytes);
          break;

        case FieldTypeByteVector:
          {
            /* convert bytevector to string */
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

              /* add bytevector string to proto_tree */
              proto_tree_add_none_format(tree, header_field, tvb,
                                         fdata->start, fdata->nbytes,
                                         "byteVector - %s (%d)%s: %s", field_name,
                                         ftype->id,
                                         field_inf,
                                         str);
              g_free (str);
            }
            else {
              DBG0("Error allocating memory.");

              proto_tree_add_none_format(tree, header_field, tvb,
                                         fdata->start, fdata->nbytes,
                                         "byteVector - %s (%d)%s: %s", field_name,
                                         ftype->id, field_inf,
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
            display_fields (tvb, subtree, tnode->children, dnode->children, pinfo);
          }
          break;

        case FieldTypeSequence:
          {
            proto_item* item;
            proto_tree* subtree;
            GNode* length_tnode;

            item = proto_tree_add_none_format(tree, header_field, tvb,
                                              fdata->start, fdata->nbytes,
                                              "sequence - %s (%d)%s length %d:", field_name,
                                              ftype->id, field_inf,fdata->value.u32
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
              display_fields (tvb, subtree, group_tnode, group_dnode, pinfo);
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
    else if (fdata->status == FieldEmpty) {
      /* The field is empty. */
      if(show_empty_optional_fields){

        proto_tree_add_none_format(tree, header_field, tvb,
                                 fdata->start, fdata->nbytes,
                                 "%s (empty %s)",
                                 field_name,
                                 field_typename(ftype->type));
      }
    } else {
      /* The field has an error */
      header_field = hf_fast[FieldTypeError];
      message_error = TRUE;

      proto_tree_add_none_format(tree, header_field, tvb, 0, 0,
                                 "%s - %s (%d): %s",
                                 "ERROR",
                                 field_name,
                                 ftype->id,
                                 fdata->value.ascii.bytes);


      /* display error message in info column */
      if(CHECK_COL(pinfo->cinfo, COL_INFO)) {
        col_add_fstr(pinfo->cinfo, COL_INFO, "%s", fdata->value.ascii.bytes);
      }

      fast_log_dynamic_error(ftype, fdata);
    }

    tnode = tnode->next;
    dnode = dnode->next;
  }

}

/*! \brief generate information about a field to be displayed in wireshark
 *  \param ftype field type
 *  \return field info
 */
char * generate_field_info(const FieldType* ftype){
  char * field_inf;
  char * temp;

  field_inf=NULL;
  if(showFieldDictionaries){
    /* dictionary */
    if(ftype->dictionary==NULL){
      field_inf = g_strdup_printf("[dictionary=global");
    } else {
      field_inf = g_strdup_printf("[dictionary=%s", ftype->dictionary);
    }
  }
  if(showFieldKeys && ftype->key!=NULL){
    /* key */
    temp = field_inf;
    if(temp){
      field_inf = g_strdup_printf("%s key=%s", temp, ftype->key);
      g_free(temp);
    } else {
      field_inf = g_strdup_printf("[key=%s", ftype->key);
    }
  }
  if(showFieldOperators){
    /* operator */
    temp = field_inf;
    if(temp){
      field_inf = g_strdup_printf("%s operator=%s", temp, operator_typename(ftype->op));
      g_free(temp);
    } else {
      field_inf = g_strdup_printf("[operator=%s", operator_typename(ftype->op));
    }
  }
  if(showFieldMandatoriness){
    /* mandatory? */
    temp = field_inf;
    if(temp){
      if(ftype->mandatory) field_inf = g_strdup_printf("%s mandatory", temp);
      else field_inf = g_strdup_printf("%s not mandatory", temp);
      g_free(temp);
    } else {
      if(ftype->mandatory) field_inf = g_strdup_printf("[mandatory");
      else field_inf = g_strdup_printf("[not mandatory");
    }
  }
  if(field_inf!=NULL){
    /* other field info */
    temp=field_inf;
    field_inf = g_strdup_printf("%s]", temp);
    g_free(temp);
  } else {
    field_inf = "";
  }

  return field_inf;
}


#define MaxMant 21 /* max char length (decimal) of 64bit number including sign */
#define MaxExpt 10
/*! \brief takes an exponent and mantissa and converts it into a decimal in a string
 *  \param expt exponent of decimal number
 *  \param mant mantissa of decimal number
 *  \return string representation of decimal number
 */
char * toDecimal(gint32 expt, gint64 mant)
{
  char * decimalNum;
  char zeros[MaxExpt+1]; /*array to hold zero padding */
  char neg[2]; /* array to hold negative sign or null string*/
  int i;

  if(expt==0){
    return "1";
  }

  if(expt<-MaxExpt || expt>MaxExpt){
    return NULL;
  }

  /* Char* to hold string represenation of mant */
  decimalNum = (char *) g_malloc(sizeof(char)*MaxMant);

  if(expt<0){
    char left [MaxExpt+2];
    char right [MaxMant];
    int point;
    int len;
    int r;

    expt = (-1)*expt; /*abs(expt) */
    if(mant<0){
      mant = (-1) * mant;
      neg[0]='-'; neg[1]='\0';
    } else { neg[0]='\0'; }
    len = sprintf (decimalNum, "%" G_GINT64_MODIFIER "d", mant); /* print base to string and get length */
    point = len-expt; /* figure out where the decimal should be */
    if(point>0){ /*point in middle of base */
      /* break into two parts and put back together */
      for(i=0; i<point; i++) left[i] = decimalNum[i];
      left[i]='\0';
      r=0;
      for(i=point; i<MaxMant; i++){
        right[r]=decimalNum[i];
        r++;
        if(decimalNum[i]=='\0') break;
      }
      sprintf(decimalNum, "%s%s.%s", neg, left, right);
    } else { /* point to left of base */
      point = (-1)*point;
      for(i=0; i<point; i++){ /* gen zeros for left of base */
        zeros[i]='0';
      }
      zeros[i] = '\0';
      sprintf(right, "%s.%s%s", neg, zeros, decimalNum);
      sprintf(decimalNum, "%s", right);
    }
  } else {
    for(i=0; i<expt; i++){/* gen zeros to pad right side of base */
      zeros[i]='0';
    }
    zeros[i] = '\0';
    sprintf (decimalNum, "%" G_GINT64_MODIFIER "d%s", mant, zeros);
  }

  return decimalNum;
}
