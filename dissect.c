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
 * \file dissect.c
 * \brief  Implementation of Wireshark independent payload dissecting.
 */

#include <string.h>
#include "debug.h"
#include "decode.h"
#include "template.h"
#include "dictionaries.h"
#include "dissect.h"


/*! \brief  Save some typing when initializing variables
 *          in a dissect_TYPE function.
 * \param ftype  New variable to store the FieldType.
 * \param fdata  New variable to store the FieldData.
 * \param tnode  Template tree node containing /ftype/.
 * \param dnode  Dissect tree node containing /fdata/.
 */
static gboolean dissect_int_op(gint64 * delta, const FieldType * ftype,
                               FieldData * fdata, DissectPosition * position, address* src, address* dest);


/*! \brief  Given a byte stream, dissect delta or increment (or nothing).
 * \param tnode  Template tree node.
 * \param position  Position in the packet.
 * \param dnode  Dissect tree node.
 * \return true if we need to still do a basic dissect
 */
gboolean dissect_ascii_delta(const FieldType* ftype, FieldData* fdata,
                             DissectPosition* position, address* src, address* dest);


#define SetupDissectStack(ftype, fdata, tnode, dnode) \
  const FieldType* ftype; \
  FieldData* fdata; \
  ftype = (FieldType*) tnode->data; \
  fdata = (FieldData*) dnode->data;

#define SetupDissectStackNoFieldData(ftype, tnode, dnode) \
  const FieldType* ftype; \
  ftype = (FieldType*) tnode->data;

#define SetupDissectStackNoFieldType(fdata, tnode, dnode) \
  FieldData* fdata; \
  fdata = (FieldData*) dnode->data;

gboolean dissect_int_op(gint64* delta,
                        const FieldType* ftype,
                        FieldData* fdata,
                        DissectPosition* position, address* src, address* dest)
{
  gboolean presence_bit;
  gboolean dissect_it = FALSE;


  switch(ftype->op) {
    case FieldOperatorConstant:
      BAILOUT(FALSE;,"Don't try to set the dictionary value on a constant");
      break;

    case FieldOperatorDelta:
      {
        FieldData fdata_temp;
        get_dictionary_value(ftype, fdata, *src, *dest);

        if(FieldUndefined == fdata->status)
        {
          fdata->status = FieldExists;

          if(ftype->hasDefault) {
            copy_field_value(ftype->type, &ftype->value, &fdata->value);
          } else {
            /* Zero out all bytes (regardless of integer type) */
            memset(&fdata->value, 0, sizeof(FieldValue));
          }
        }
        else if(FieldEmpty == fdata->status && ftype->mandatory)
        {
    err_d(6, fdata);
          return FALSE;
        }

        basic_dissect_int64(position, &fdata_temp);
        *delta = fdata_temp.value.i64;
        if (!ftype->mandatory && 0 < *delta) {
          *delta -= 1;
        }

      }
      break;

    case FieldOperatorIncrement:
      presence_bit = dissect_shift_pmap(position);
      if(presence_bit) {
        dissect_it = TRUE;
      }
      else {
        /* do a dictionary lookup */
        get_dictionary_value(ftype, fdata, *src, *dest);

        if(FieldEmpty == fdata->status && ftype->mandatory)
        {
    err_d(6, fdata);
          return FALSE;
        }
        else if(FieldUndefined == fdata->status && ftype->mandatory) {
    err_d(5, fdata);
          return FALSE;
        }
        else {
          *delta = 1;
        }
      }
      break;

    default:
      dissect_it = TRUE;
      break;
  }

  return dissect_it;
}


/*! \brief Given a byte stream, concatenates a second ascii string to the first one
 * \param ftype Type of associated data.
 * \param fdata Location of the data.
 * \param position Position in the packet.
 * \param src Location of source dictionary.
 * \param dest Location of destination dictionary.
 */
gboolean dissect_ascii_delta(const FieldType* ftype, FieldData* fdata,
                             DissectPosition* position, address* src, address* dest)
{
  FieldData fdata_temp;
  FieldData input_str;
  FieldData lookup;
  gint32 subtract;
  gint64 cut_length;
  gint64 input_str_len;
  gboolean append_to_front;

  /* get the subtraction length */
  basic_dissect_int32(position, &fdata_temp);
  subtract = fdata_temp.value.i32;

  /* get the previous string */
  if(!get_dictionary_value(ftype, &lookup, *src, *dest)) {
    return TRUE;
  }

  /* get the input string */
  basic_dissect_ascii_string (position, &input_str);

  /* ERROR catching for D7 */

  /* subtration length is greater than 5 */
  if(fdata_temp.nbytes > 5){

    err_d(7, fdata);
    cleanup_field_value(FieldTypeAsciiString, &lookup.value);
    return FALSE;
  }
  /* subtraction length equal to 5 and... */
  if(fdata_temp.nbytes == 5 &&
     (FieldError == fdata_temp.status)) {

    err_d(7, fdata);
    cleanup_field_value(FieldTypeAsciiString, &lookup.value);
    return FALSE;
  }

  /* append to front or tail? */
  append_to_front = (subtract < 0);
  if(append_to_front)
    subtract = -(subtract + 1);

  if((gint32)lookup.value.ascii.nbytes < subtract)
  {
    /* if the previous value is shorter than the
     * subtraction length, this is an error.
     */
    cleanup_field_value(FieldTypeAsciiString, &input_str.value);
    cleanup_field_value(FieldTypeAsciiString, &lookup.value);

    err_d(7, fdata);
    return FALSE;

  }

  /* malloc space for new string */
  cut_length = lookup.value.ascii.nbytes - subtract;
  input_str_len = input_str.value.ascii.nbytes;
  fdata->value.ascii.nbytes = cut_length + input_str_len;
  fdata->value.ascii.bytes = (guint8 *)
          g_malloc((fdata->value.ascii.nbytes + 1)*sizeof(guint8));

  if(append_to_front)
  {
    /* append input string to front */
    memcpy(fdata->value.ascii.bytes,
           input_str.value.ascii.bytes,
           input_str_len);

    /* append cut string to end */
    memcpy(fdata->value.ascii.bytes + input_str_len,
           lookup.value.ascii.bytes + subtract,
           cut_length);
  }
  else
  {
    /* append cut string to front */
    memcpy(fdata->value.ascii.bytes,
           lookup.value.ascii.bytes,
           cut_length);

    /* append input string to end */
    memcpy(fdata->value.ascii.bytes + cut_length,
           input_str.value.ascii.bytes,
           input_str_len);
  }

  /* null terminator */
  fdata->value.ascii.bytes[fdata->value.ascii.nbytes] = 0;

  cleanup_field_value(FieldTypeAsciiString, &input_str.value);
  cleanup_field_value(FieldTypeAsciiString, &lookup.value);

  return FALSE;
}


GNode* dissect_fast_bytes (DissectPosition* position, GNode* parent, address* src, address* dest)
{
  static guint32 template_id = 0;
  GNode* tmpl = 0; /* Template. */
  FieldData* fdata; /* Template ID data node. */

  /* Initialize head node. */
  fdata = (FieldData*) g_malloc (sizeof (FieldData));
  if (!fdata) {
    BAILOUT(0,"Could not allocate memory.");
  }
  fdata->start  = position->offset;
  fdata->nbytes = 0;
  fdata->status  = FieldEmpty;
  parent->data  = fdata;


  basic_dissect_pmap (position, position);

  if (!position->pmap) {
    g_free (fdata);
    BAILOUT(0,"PMAP not set.");
  }

  /* Figure out current Template ID. */
  if (dissect_shift_pmap (position)) {
    /* Decode from the stream. */
    basic_dissect_uint32 (position, fdata);
    template_id = fdata->value.u32;
  }

  tmpl = find_template (template_id);

  /* If no template return null */
  if (!tmpl) {
    g_free(position->pmap);
    return 0;
  }

  /* Dissect the packet. */
  {
    GNode* data_node = 0;
    dissector_walk(tmpl->children, position,
                   parent, data_node, src, dest);
  }

  fdata->nbytes = position->offset - fdata->start;
  g_free(position->pmap);
  return tmpl;
}


void dissector_walk (const GNode* tnode,
                     DissectPosition* position,
                     GNode* parent, GNode* dnode, address* src, address* dest)
{
  while (tnode) {
    dnode = dissect_descend (tnode, position, parent, dnode, src, dest);
    tnode = tnode->next;
  }
}


GNode* dissect_descend (const GNode* tnode,
                        DissectPosition* position,
                        GNode* parent, GNode* dnode, address* src, address* dest)
{
  const FieldType* ftype;
  FieldData* fdata;
  GNode* dnode_next;

  if (!tnode) {
    BAILOUT(NULL,"Template node is NULL.");
  }

  ftype = (FieldType*) tnode->data;

  /* Assure FieldType is good for lookup. */
  if ((guint) ftype->type >= (guint) FieldTypeEnumLimit) {
    DBG1("Unknown field type %u.", (guint) ftype->type);
    return 0;
  }

  /* Set up data. */
  fdata = (FieldData*) g_malloc (sizeof (FieldData));
  if (!fdata)  BAILOUT(0,"Could not allocate memory.");

  dnode_next = g_node_new (fdata);
  if (!dnode_next) {
    g_free (fdata);
    BAILOUT(0,"Could not allocate memory.");
  }
  g_node_insert_after(parent, dnode, dnode_next);

  dissect_value(tnode, position, dnode_next, src, dest);

  if(!(dnode_next->parent)){
    g_node_destroy(dnode_next);   /* TODO: wat? */
    /* As we are building the tree,
     * the last node added will the the one we just made.
     */
    dnode_next = g_node_last_child(parent);
  }
  return dnode_next;
}


void dissect_value (const GNode* tnode,
                    DissectPosition* position, GNode* dnode, address* src, address* dest)
{
  /* Map to the dissect functions. */
  static void (*dissect_fn_map[]) (const GNode*, DissectPosition*, GNode*, address*, address*) =
  {
    &dissect_uint32, &dissect_uint64,
    &dissect_int32,  &dissect_int64,
    &dissect_decimal,
    &dissect_ascii_string, &dissect_unicode_string, &dissect_byte_vector,
    &dissect_group, &dissect_sequence,
    0
  };
  guint start;
  SetupDissectStack(ftype, fdata,  tnode, dnode);

  start = position->offset;

  /* Initialize to sensible defaults. */
  fdata->start  = start;
  fdata->nbytes = 0;
  fdata->status  = FieldEmpty;
  init_field_value(&fdata->value);

  /* Assure the appropriate function exists. */
  if (!dissect_fn_map[ftype->type]) {
    DBG1("Field type %u not implemented.", (guint) ftype->type);
    return;
  }

  if (!ftype->mandatory) {
    /* Set empty to false if there should be a value. */
    dissect_optional(tnode, position, dnode, src, dest);
  }
  else {
    fdata->status = FieldExists;
  }

  if (fdata->status == FieldExists) {
    gboolean operator_used = FALSE;

    switch (ftype->op) {
      case FieldOperatorCopy:
        operator_used = dissect_copy(tnode, position, dnode, src, dest);
        break;

      case FieldOperatorConstant:
        copy_field_value(ftype->type, &ftype->value, &fdata->value);
        operator_used = TRUE;
        break;

      case FieldOperatorDefault:
        operator_used = dissect_default(tnode, position, dnode, src, dest);
        break;

      default:
        break;
    }

    if(!operator_used) {
      /* Call the dissect function. */
      (*dissect_fn_map[ftype->type]) (tnode, position, dnode, src, dest);
    }
  }
  /* Make sure the window is correct. */
  fdata->start = start;
  fdata->nbytes = position->offset - start;
}


void dissect_optional (const GNode* tnode,
                       DissectPosition* position, GNode* dnode, address* src, address* dest)
{
  gboolean check_pmap = FALSE;
  gboolean check_null = FALSE;
  gboolean set_dict   = FALSE;
  SetupDissectStack(ftype, fdata,  tnode, dnode);
  if (ftype->mandatory) {

    BAILOUT(;,"Don't call this function on a mandatory field.");
  }
  if ((FieldTypeDecimal == ftype->type ||
       FieldTypeSequence == ftype->type)
      && FieldOperatorNone == ftype->op) {
    dissect_optional (tnode->children, position, dnode, src, dest);
    return;
  }
  switch (ftype->op) {
    case FieldOperatorNone:
      if (FieldTypeGroup == ftype->type) {
        check_pmap = TRUE;
      }
      else {
        check_null = TRUE;
        set_dict   = TRUE;
      }
      break;
    case FieldOperatorConstant:
      check_pmap = TRUE;
      break;
    case FieldOperatorDelta:
      check_null = TRUE;
      break;
    case FieldOperatorDefault:
    case FieldOperatorCopy:
    case FieldOperatorIncrement:
    case FieldOperatorTail:
      check_pmap = TRUE;
      check_null = TRUE;
      set_dict   = TRUE;
      break;
    default:
      DBG0("Bad operator type.");
      break;
  }

  if ((fdata->status == FieldEmpty) && check_pmap) {
    gboolean bit = dissect_peek_pmap(position);
    if (( bit && !check_null) ||
        (!bit &&  check_null)) {
      fdata->status = FieldExists;
    }
  }
  if ((fdata->status == FieldEmpty) && check_null) {
    fdata->status = dissect_shift_null(position) ? FieldEmpty : FieldExists;
  }
  if ((fdata->status == FieldEmpty) && set_dict) {
    set_dictionary_value(ftype, fdata, *src, *dest);
  }
  if (check_pmap && (!check_null || (fdata->status == FieldEmpty))) {
    dissect_shift_pmap(position);
  }
}


gboolean dissect_copy(const GNode* tnode,
                      DissectPosition* position, GNode* dnode, address* src, address* dest)
{
  gboolean used = TRUE;
  gboolean presence_bit;

  SetupDissectStack(ftype, fdata, tnode, dnode);

  presence_bit = dissect_shift_pmap(position);
  if(presence_bit) {
    used = FALSE;
  } else {
    get_dictionary_value(ftype, fdata, *src, *dest);

    if(FieldUndefined == fdata->status && ftype->mandatory)
    {
      err_d(5, fdata);

      return FALSE;
    }
    else if(FieldEmpty == fdata->status && ftype->mandatory)
    {
      err_d(6, fdata);

      return FALSE;
    }

  }

  return used;
}


gboolean dissect_default(const GNode* tnode,
                         DissectPosition* position, GNode* dnode, address* src, address* dest)
{
  gboolean used = TRUE;
  gboolean presence_bit;
  SetupDissectStack(ftype, fdata,  tnode, dnode);
  presence_bit = dissect_shift_pmap(position);
  if(presence_bit) {
    used = FALSE;
  } else {
    copy_field_value(ftype->type, &ftype->value, &fdata->value);
    set_dictionary_value(ftype, fdata, *src, *dest);
  }
  return used;
}


void dissect_uint32 (const GNode* tnode,
                     DissectPosition* position, GNode* dnode, address* src, address* dest)
{
  gint64 delta = 0;
  gboolean dissect_it = FALSE;
  SetupDissectStack(ftype, fdata,  tnode, dnode);

  dissect_it = dissect_int_op(&delta, ftype, fdata, position, src, dest);

  if(dissect_it) {
    basic_dissect_uint32(position, fdata);
    if (!ftype->mandatory) {
      delta = -1;
    }
  }

  fdata->value.u32 = (guint32) (fdata->value.u32 + delta);
  set_dictionary_value(ftype, fdata, *src, *dest);
}


void dissect_uint64 (const GNode* tnode,
                     DissectPosition* position, GNode* dnode, address* src, address* dest)
{
  gboolean dissect_it = FALSE;
  gint64 delta = 0;
  SetupDissectStack(ftype, fdata,  tnode, dnode);

  dissect_it = dissect_int_op(&delta, ftype, fdata, position, src, dest);

  if (dissect_it) {
    basic_dissect_uint64 (position, fdata);
    if (!ftype->mandatory) {
      delta = -1;
    }
  }

  fdata->value.u64 += delta;
  set_dictionary_value(ftype, fdata, *src, *dest);
}


void dissect_int32 (const GNode* tnode,
                    DissectPosition* position, GNode* dnode, address* src, address* dest)
{
  gboolean dissect_it = FALSE;
  gint64 delta = 0;
  SetupDissectStack(ftype, fdata,  tnode, dnode);

  dissect_it = dissect_int_op(&delta, ftype, fdata, position, src, dest);

  if (dissect_it) {
    basic_dissect_int32 (position, fdata);
    if (!ftype->mandatory && 0 < fdata->value.i32) {
      delta = -1;
    }

  }

  fdata->value.i32 = (gint32) (fdata->value.i32 + delta);
  set_dictionary_value(ftype, fdata, *src, *dest);
}


void dissect_int64 (const GNode* tnode,
                    DissectPosition* position, GNode* dnode, address* src, address* dest)
{
  gboolean dissect_it = FALSE;
  gint64 delta = 0;
  SetupDissectStack(ftype, fdata,  tnode, dnode);

  dissect_it = dissect_int_op(&delta, ftype, fdata, position, src, dest);

  if (dissect_it) {
    basic_dissect_int64 (position, fdata);

    if (!ftype->mandatory && 0 < fdata->value.i64) {
      delta = -1;
    }
  }

  fdata->value.i64 += delta;
  set_dictionary_value(ftype, fdata, *src, *dest);
}


void dissect_decimal (const GNode* tnode,
                      DissectPosition* position, GNode* dnode, address* src, address* dest)
{
  gint32 expt;       gint64 mant;
  GNode* expt_node;  GNode* mant_node;

  SetupDissectStack(ftype, fdata,  tnode, dnode);

  /* Assure existence of 2 child nodes. */
  if (!tnode->children || !tnode->children->next) {
    BAILOUT(;,"Error in internal decimal field setup.");
  }

  expt_node = tnode->children;
  mant_node = expt_node->next;

  if(FieldOperatorDelta == ftype->op) {
    gint64 delta = 0;
    FieldData expt_data;
    FieldData mant_data;
    dissect_int_op(&delta, ftype, &expt_data, position, src, dest);
    expt = (gint32) (delta + expt_data.value.decimal.exponent);

    dissect_int_op(&delta, ftype, &mant_data, position, src, dest);
    mant = delta + mant_data.value.decimal.mantissa;
  }
  else {
    /* Grab exponent. */
    dissect_value (expt_node, position, dnode, src, dest);
    expt = fdata->value.i32;
    /* Grab mantissa. */
    dissect_value (mant_node, position, dnode, src, dest);
    mant = fdata->value.i64;
  }

  fdata->value.decimal.mantissa = mant;
  fdata->value.decimal.exponent = expt;

  set_dictionary_value(ftype, fdata, *src, *dest);
}


void dissect_ascii_string (const GNode* tnode,
                           DissectPosition* position, GNode* dnode, address* src, address* dest)
{
  gboolean dissect_it = FALSE;
  SetupDissectStack(ftype, fdata,  tnode, dnode);

  switch(ftype->op) {
    case FieldOperatorConstant:
      BAILOUT(;,"Who let a constant in here?");
      break;
    case FieldOperatorCopy:
    case FieldOperatorDefault:
    case FieldOperatorNone:
      dissect_it = TRUE;
      break;
    case FieldOperatorDelta:
    case FieldOperatorTail:
        dissect_it = dissect_ascii_delta(ftype, fdata, position, src, dest);
      break;
    default:
      DBG0("Invalid Operator.");
      break;
  }
  if (dissect_it) {
    basic_dissect_ascii_string (position, fdata);
  }

  set_dictionary_value(ftype, fdata, *src, *dest);
}


void dissect_unicode_string (const GNode* tnode,
                             DissectPosition* position, GNode* dnode, address* src, address* dest)
{
  dissect_byte_vector (tnode, position, dnode, src, dest);
}


void dissect_byte_vector (const GNode* tnode,
                          DissectPosition* position, GNode* dnode, address* src, address* dest)
{
  GNode* length_node;
  gboolean dissect_it = FALSE;
  SetupDissectStack(ftype, fdata,  tnode, dnode);

  length_node = tnode->children;
  if (!length_node) {
    BAILOUT(;,"Length should be a child node.");
  }

  switch(ftype->op) {
    case FieldOperatorConstant:
      BAILOUT(;,"Who let a constant in here?");
      break;
    case FieldOperatorCopy:
    case FieldOperatorDefault:
    case FieldOperatorNone:
      dissect_it = TRUE;
      break;
    case FieldOperatorDelta:
    case FieldOperatorTail:
      {
        SizedData* vec;
        FieldData fdata_temp;
        FieldData input_str;
        FieldData lookup;
        gint64 subtract;
        gint64 cut_length;
        gint64 input_str_len;
        gboolean append_to_front;

        /* get the subtraction length */
        basic_dissect_int64(position, &fdata_temp);
        subtract = fdata_temp.value.i64;

        /* get the previous string */
        if(!get_dictionary_value(ftype, &lookup, *src, *dest))
        {
          dissect_it = TRUE;
          break;
        }

        /* get the input string */
        /* ----------------------------- TODO: generalize this. */
        vec = &input_str.value.bytevec;

        /* See how big the byte vector is. */
        dissect_value (length_node, position, dnode, src, dest);

        vec->nbytes = fdata->value.u32;

        /* Get the byte vector. */
        position->offjmp = vec->nbytes;

        vec->bytes = (guint8*)g_malloc ((1+vec->nbytes) * sizeof(guint8));

        if (vec->bytes) {
          decode_byte_vector (vec->nbytes, position->bytes, vec->bytes);
          vec->bytes[vec->nbytes] = 0;
        }

        ShiftBytes(position);
        /* --------------------------------- */

        /* append to front or tail? */
        append_to_front = (subtract < 0);
        if(append_to_front)
          subtract = -(subtract + 1);

        if(lookup.value.bytevec.nbytes < subtract)
        {
          /* if the previous value is shorter than the
           * subtraction length, this is an error.
           *
           */
          cleanup_field_value(FieldTypeByteVector, &input_str.value);
          cleanup_field_value(FieldTypeByteVector, &lookup.value);
	  err_d(7, fdata);
        }

        /* malloc space for new string */
        cut_length = lookup.value.bytevec.nbytes - subtract;
        input_str_len = input_str.value.bytevec.nbytes;
        fdata->value.bytevec.nbytes = cut_length + input_str_len;
        fdata->value.bytevec.bytes = (guint8 *)
                g_malloc((fdata->value.bytevec.nbytes + 1)*sizeof(guint8));

        if(append_to_front)
        {
          /* append input string to front */
          memcpy(fdata->value.bytevec.bytes,
                 input_str.value.bytevec.bytes,
                 input_str_len);

          /* append cut string to end */
          memcpy(fdata->value.bytevec.bytes + input_str_len,
                 lookup.value.bytevec.bytes + subtract,
                 cut_length);
        }
        else
        {
          /* append cut string to front */
          memcpy(fdata->value.bytevec.bytes,
                 lookup.value.bytevec.bytes,
                 cut_length);

          /* append input string to end */
          memcpy(fdata->value.bytevec.bytes + cut_length,
                 input_str.value.bytevec.bytes,
                 input_str_len);
        }

        /* null terminator */
        fdata->value.bytevec.bytes[fdata->value.ascii.nbytes] = 0;

        cleanup_field_value(FieldTypeByteVector, &input_str.value);
        cleanup_field_value(FieldTypeByteVector, &lookup.value);
      }
      break;
    default:
      DBG0("Invalid Operator.");
      break;
  }
  if (dissect_it) {
    SizedData* vec;
    vec = &fdata->value.bytevec;

    /* See how big the byte vector is. */
    dissect_value (length_node, position, dnode, src, dest);

    vec->nbytes = fdata->value.u32;

    /* Get the byte vector. */
    position->offjmp = vec->nbytes;

    vec->bytes = (guint8*)g_malloc ((1+vec->nbytes) * sizeof(guint8));

    if (vec->bytes) {
      decode_byte_vector (vec->nbytes, position->bytes, vec->bytes);
      vec->bytes[vec->nbytes] = 0;
    }

    ShiftBytes(position);
  }
  set_dictionary_value(ftype, fdata, *src, *dest);
}


void dissect_group (const GNode* tnode,
                    DissectPosition* position, GNode* dnode, address* src, address* dest)
{
  DissectPosition stacked_position;
  DissectPosition* nested_position;
  SetupDissectStackNoFieldData(ftype, tnode, dnode);

  if (ftype->value.pmap_exists) {
    basic_dissect_pmap (position, &stacked_position);
    nested_position = &stacked_position;
  }
  else {
    stacked_position.pmap = NULL;
    nested_position = position;
  }

  /* Recurse down the tree, building onto dnode. */
  dissector_walk (tnode->children, nested_position, dnode, 0, src, dest);

  position->offjmp = nested_position->offset - position->offset;
  ShiftBytes(position);

  if (stacked_position.pmap) {
    g_free(stacked_position.pmap);
  }
}


void dissect_sequence (const GNode* tnode,
                       DissectPosition* position, GNode* dnode, address* src, address* dest)
{
  guint32 length;
  guint32 i;
  GNode* parent;
  GNode* length_tnode;
  GNode* group_tnode;
  SetupDissectStackNoFieldType(fdata,  tnode, dnode);

  if (!tnode->children || !tnode->children->next) {
    BAILOUT(;,"Error in sequence setup.");
  }
  length_tnode = tnode->children;
  group_tnode  = length_tnode->next;

  dissect_value (length_tnode, position, dnode, src, dest);
  length = fdata->value.u32;
  parent = dnode;
  dnode  = 0;
  for (i = 0; i < length; ++i) {
    if (!position->nbytes) {
      DBG0("Sequence bailing, no space left in packet.");
      break;
    }
    if (position->pmap_idx >= position->pmap_len) {
      DBG0("Sequence bailing, no space left in pmap.");
      break;
    }
    dnode = dissect_descend (group_tnode, position, parent, dnode, src, dest);
  }
}
