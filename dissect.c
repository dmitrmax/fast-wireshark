/*!
 * \file dissect.c
 * \brief  Handle payload dissection independent of Wireshark.
 */

#include <string.h>
#include "debug.h"
#include "decode.h"
#include "template.h"
#include "dictionaries.h"
#include "dissect.h"
  
static gboolean dissect_int_op(gint64 * delta, const FieldType * ftype, 
                               FieldData * fdata, DissectPosition * position);
gboolean dissect_ascii_delta(const FieldType* ftype, FieldData* fdata,
                             DissectPosition* position);
  
/*! \brief Dissect a FAST message by the bytes.
 * \param position  Current position in bytes.
 * \param parent  Return value. The message data is built under it.
 * \return  The template that was used to dissect.
 */
GNode* dissect_fast_bytes (DissectPosition* position, GNode* parent)
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

  /* Bail out if template not found. */
  if (!tmpl) {

    g_free(position->pmap);
    g_free(fdata);
    DBG1("Template %d not defined.", template_id);
    return 0;
  }

  /* Dissect the packet. */
  {
    GNode* data_node = 0;
    dissector_walk(tmpl->children, position,
                   parent, data_node);
  }

  fdata->nbytes = position->offset - fdata->start;
  g_free(position->pmap);
  return tmpl;
}


/*! \brief  Construct
 a message data tree (of FieldData).
 * \param tnode  Template node, contains type definition.
 * \param position  Current position in message.
 * \param parent  Parent node in data tree.
 * \param dnode  Previous node in data tree.
 */
void dissector_walk (const GNode* tnode,
                     DissectPosition* position,
                     GNode* parent, GNode* dnode)
{
  while (tnode) {
    dnode = dissect_descend (tnode, position, parent, dnode);
    tnode = tnode->next;
  }
}


/*! \brief  Dissect a certain data type.
 * \param tnode  Template node, contains type definition.
 * \param position  Current position in message.
 * \param parent  Parent node in data tree.
 * \param dnode  Previous node in data tree.
 * \return  Node that was created.
 */
GNode* dissect_descend (const GNode* tnode,
                        DissectPosition* position,
                        GNode* parent, GNode* dnode)
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

  dissect_value(tnode, position, dnode_next);

  if(!(dnode_next->parent)){
    g_node_destroy(dnode_next);   /* TODO: wat? */
    /* As we are building the tree,
     * the last node added will the the one we just made.
     */
    dnode_next = g_node_last_child(parent);
  }
  return dnode_next;
}


/*! \brief  Save some typing when initializing valiables
 *          in a dissect_TYPE function.
 * \param ftype  New variable to store the FieldType.
 * \param fdata  New variable to store the FieldData.
 * \param tnode  Template tree node containing /ftype/.
 * \param dnode  Dissect tree node containing /fdata/.
 */
#define SetupDissectStack(ftype, fdata, tnode, dnode) \
  const FieldType* ftype; \
  FieldData* fdata; \
  ftype = (FieldType*) tnode->data; \
  fdata = (FieldData*) dnode->data;


/*! \brief  Given a byte stream, dissect some value.
 * \param tnode  Template tree node.
 * \param position  Position in the packet.
 * \param dnode  Dissect tree node.
 */
void dissect_value (const GNode* tnode,
                    DissectPosition* position, GNode* dnode)
{
  /* Map to the dissect functions. */
  static void (*dissect_fn_map[]) (const GNode*, DissectPosition*, GNode*) =
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
    dissect_optional(tnode, position, dnode);
  }
  else {
    fdata->status = FieldExists;
  }

  if (fdata->status == FieldExists) {
    gboolean operator_used = FALSE;
  
    switch (ftype->op) {
      case FieldOperatorCopy:
        operator_used = dissect_copy(tnode, position, dnode);
        break;
        
      case FieldOperatorConstant:
        copy_field_value(ftype->type, &ftype->value, &fdata->value);
        operator_used = TRUE;
        break;
        
      case FieldOperatorDefault:
        operator_used = dissect_default(tnode, position, dnode);
        break;
        
      default:
        break;
    } 
       
    if(!operator_used) {
      /* Call the dissect function. */
      (*dissect_fn_map[ftype->type]) (tnode, position, dnode);
    }
  }
  /* Make sure the window is correct. */
  fdata->start = start;
  fdata->nbytes = position->offset - start;
}

/*! \brief  Given a byte stream, check if an optional field is empty.
 * \param tnode  Template tree node.
 * \param position  Position in the packet.
 * \param dnode  Dissect tree node.
 */
void dissect_optional (const GNode* tnode,
                       DissectPosition* position, GNode* dnode)
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
    dissect_optional (tnode->children, position, dnode);
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
    set_dictionary_value(ftype, fdata);
  }
  if (check_pmap && (!check_null || (fdata->status == FieldEmpty))) {
    dissect_shift_pmap(position);
  }
}

/*! \brief Given a byte stream with a copy operator, dissect it
 *            if it is used
 * \param tnode  Template tree node.
 * \param position  Position in the packet.
 * \param dnode  Dissect tree node.
 * \return true if the copy operator is used
 */
gboolean dissect_copy(const GNode* tnode,
                      DissectPosition* position, GNode* dnode)
{
  gboolean used = TRUE;
  gboolean presence_bit;
  SetupDissectStack(ftype, fdata, tnode, dnode);
  presence_bit = dissect_shift_pmap(position);
  if(presence_bit) {
    used = FALSE;
  } else {
    get_dictionary_value(ftype, fdata);
    
    if(FieldUndefined == fdata->status && ftype->mandatory)
    {
      fdata->status = FieldError;
      fdata->value.ascii.bytes = (guint8*)g_strdup_printf(
          "[ERR D5] Mandatory field not present, undefined previous value");
      return FALSE;
    }
    
  }
  return used;
}                      

/*! \brief Given a byte stream with a default operator, dissect it
 *            if it is used
 * \param tnode  Template tree node.
 * \param position  Position in the packet.
 * \param dnode  Dissect tree node.
 * \return true if the default operator is used
 */
gboolean dissect_default(const GNode* tnode,
                         DissectPosition* position, GNode* dnode)
{
  gboolean used = TRUE;  
  gboolean presence_bit;
  SetupDissectStack(ftype, fdata,  tnode, dnode);
  presence_bit = dissect_shift_pmap(position);
  if(presence_bit) {
    used = FALSE;
  } else {
    copy_field_value(ftype->type, &ftype->value, &fdata->value);
    set_dictionary_value(ftype, fdata);
  }
  return used;
}

/*! \brief  Given a byte stream, dissect delta or increment (or nothing).
 * \param tnode  Template tree node.
 * \param position  Position in the packet.
 * \param dnode  Dissect tree node.
 * \return true if we need to still do a basic dissect
 */
gboolean dissect_int_op(gint64* delta,
                        const FieldType* ftype,
                        FieldData* fdata,
                        DissectPosition* position)
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
        get_dictionary_value(ftype, fdata);
        
        if(FieldUndefined == fdata->status)
        {
          fdata->status = FieldExists;
          
          if(!ftype->empty) {
            copy_field_value(ftype->type, &ftype->value, &fdata->value);
          } else {
            /* Zero out all bytes (regardless of integer type) */
            memset(&fdata->value, 0, sizeof(FieldValue));
          }
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
      } else if(ftype->mandatory) {
        fdata->status = FieldError;
        fdata->value.ascii.bytes = (guint8*)g_strdup_printf(
          "[ERR D5] Mandatory field not present, undefined previous value");
        return FALSE;
      }
      else {
        *delta = 1;
      }
      break;
      
    default:
      dissect_it = TRUE;      
      break;
  }
  
  return dissect_it;
}

/*! \brief  Given a byte stream, dissect an unsigned 32bit integer.
 * \param tnode  Template tree node.
 * \param position  Position in the packet.
 * \param dnode  Dissect tree node.
 */
void dissect_uint32 (const GNode* tnode,
                     DissectPosition* position, GNode* dnode)
{
  gint64 delta = 0;
  gboolean dissect_it = FALSE;
  SetupDissectStack(ftype, fdata,  tnode, dnode);

  dissect_it = dissect_int_op(&delta, ftype, fdata, position);  
  
  if(dissect_it) {
    basic_dissect_uint32(position, fdata);
    if (!ftype->mandatory) {
      delta = -1;
    }
  }
    
  fdata->value.u32 = (guint32) (fdata->value.u32 + delta);
  set_dictionary_value(ftype, fdata);
}


/*! \brief  Given a byte stream, dissect an unsigned 64bit integer.
 * \param tnode  Template tree node.
 * \param position  Position in the packet.
 * \param dnode  Dissect tree node.
 */
void dissect_uint64 (const GNode* tnode,
                     DissectPosition* position, GNode* dnode)
{
  gboolean dissect_it = FALSE;
  gint64 delta = 0;
  SetupDissectStack(ftype, fdata,  tnode, dnode);

  dissect_it = dissect_int_op(&delta, ftype, fdata, position);
  
  if (dissect_it) {
    basic_dissect_uint64 (position, fdata);
    if (!ftype->mandatory) {
      delta = -1;
    }
  }
  
  fdata->value.u64 += delta;
  set_dictionary_value(ftype, fdata);
}

/*! \brief  Given a byte stream, dissect a signed 32bit integer.
 * \param tnode  Template tree node.
 * \param position  Position in the packet.
 * \param dnode  Dissect tree node.
 */
void dissect_int32 (const GNode* tnode,
                    DissectPosition* position, GNode* dnode)
{
  gboolean dissect_it = FALSE;
  gint64 delta = 0;
  SetupDissectStack(ftype, fdata,  tnode, dnode);

  dissect_it = dissect_int_op(&delta, ftype, fdata, position);
  
  if (dissect_it) {
    basic_dissect_int32 (position, fdata);
    if (!ftype->mandatory && 0 < fdata->value.i32) {
      delta = -1;
    }
    
  }
  
  fdata->value.i32 = (gint32) (fdata->value.i32 + delta);
  set_dictionary_value(ftype, fdata);
}


/*! \brief  Given a byte stream, dissect a signed 64bit integer.
 * \param tnode  Template tree node.
 * \param position  Position in the packet.
 * \param dnode  Dissect tree node.
 */
void dissect_int64 (const GNode* tnode,
                    DissectPosition* position, GNode* dnode)
{
  gboolean dissect_it = FALSE;
  gint64 delta = 0;
  SetupDissectStack(ftype, fdata,  tnode, dnode);

  dissect_it = dissect_int_op(&delta, ftype, fdata, position);
  
  if (dissect_it) {
    basic_dissect_int64 (position, fdata);
    
    if (!ftype->mandatory && 0 < fdata->value.i64) {
      delta = -1;
    }
  }
  
  fdata->value.i64 += delta;
  set_dictionary_value(ftype, fdata);
}


/*! \brief  Given a byte stream, dissect a decimal number.
 * \param tnode  Template tree node.
 * \param position  Position in the packet.
 * \param dnode  Dissect tree node.
 */
void dissect_decimal (const GNode* tnode,
                      DissectPosition* position, GNode* dnode)
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
    dissect_int_op(&delta, ftype, &expt_data, position);
    expt = (gint32) (delta + expt_data.value.decimal.exponent);
    
    dissect_int_op(&delta, ftype, &mant_data, position);    
    mant = delta + mant_data.value.decimal.mantissa;
  }
  else {
    /* Grab exponent. */
    dissect_value (expt_node, position, dnode);
    expt = fdata->value.i32;
    /* Grab mantissa. */
    dissect_value (mant_node, position, dnode);
    mant = fdata->value.i64;
  }
  
  fdata->value.decimal.mantissa = mant;
  fdata->value.decimal.exponent = expt;

  set_dictionary_value(ftype, fdata);
}


/*! \brief  Given a byte stream, dissect an ASCII string.
 * \param tnode  Template tree node.
 * \param position  Position in the packet.
 * \param dnode  Dissect tree node.
 */
void dissect_ascii_string (const GNode* tnode,
                           DissectPosition* position, GNode* dnode)
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
        dissect_it = dissect_ascii_delta(ftype, fdata, position);
      break; 
    default:
      DBG0("Invalid Operator.");
      break;
  }
  if (dissect_it) {
    basic_dissect_ascii_string (position, fdata);
  }
  
  set_dictionary_value(ftype, fdata);
}

/*! \brief 
 * 
 */
gboolean dissect_ascii_delta(const FieldType* ftype, FieldData* fdata,
                             DissectPosition* position)
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
  if(!get_dictionary_value(ftype, &lookup)) {
    return TRUE;
  }
  
  /* get the input string */
  basic_dissect_ascii_string (position, &input_str);
  
  /* ERROR catching for D7 */
  
  /* subtration length is greater than 5 */
  if(fdata_temp.nbytes > 5){
    
    fdata->status = FieldError;
    fdata->value.ascii.bytes = (guint8*)g_strdup_printf(
        "[ERR D7] The value of subtraction exceeds the bounds of int32");
    cleanup_field_value(FieldTypeAsciiString, &lookup.value);
    return FALSE;
  }
  /* subtraction length equal to 5 and... */
  if(fdata_temp.nbytes == 5 && 
     (FieldError == fdata_temp.status)) {
    
    fdata->status = FieldError;      
    fdata->value.ascii.bytes = (guint8*)g_strdup_printf(
        "[ERR D7] The value of subtraction exceeds the bounds of int32");
    cleanup_field_value(FieldTypeAsciiString, &lookup.value);
    return FALSE;
  }
  
  /* append to front or tail? */
  append_to_front = (subtract < 0);
  if(append_to_front)
    subtract = -(subtract + 1);

  if(lookup.value.ascii.nbytes < subtract) 
  {
    /* if the previous value is shorter than the
     * subtraction length, this is an error.
     */
    cleanup_field_value(FieldTypeAsciiString, &input_str.value);
    cleanup_field_value(FieldTypeAsciiString, &lookup.value); 
    
    fdata->status = FieldError;
    fdata->value.ascii.bytes = (guint8*)g_strdup_printf(
      "[ERR D7] The subtraction length is larger than the %s",
       "number of characters in the base value");
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

/*! \brief  Given a byte stream, dissect a unicode string.
 * \param tnode  Template tree node.
 * \param position  Position in the packet.
 * \param dnode  Dissect tree node.
 * \sa dissect_byte_vector
 */
void dissect_unicode_string (const GNode* tnode,
                             DissectPosition* position, GNode* dnode)
{
  dissect_byte_vector (tnode, position, dnode);
}


/*! \brief  Given a byte stream, dissect a byte vector.
 * \param tnode  Template tree node.
 * \param position  Position in the packet.
 * \param dnode  Dissect tree node.
 */
void dissect_byte_vector (const GNode* tnode,
                          DissectPosition* position, GNode* dnode)
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
        if(!get_dictionary_value(ftype, &lookup)) 
        {
          dissect_it = TRUE;
          break;
        }
        
        /* get the input string */
        /* ----------------------------- TODO: generalize this. */
        vec = &input_str.value.bytevec;

        /* See how big the byte vector is. */
        dissect_value (length_node, position, dnode);

        vec->nbytes = fdata->value.u32;

        /* Get the byte vector. */
        position->offjmp = vec->nbytes;

        vec->bytes = g_malloc ((1+vec->nbytes) * sizeof(guint8));

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
           * TODO: make this a legit error.
           */
          cleanup_field_value(FieldTypeByteVector, &input_str.value);
          cleanup_field_value(FieldTypeByteVector, &lookup.value); 
          BAILOUT(;,"[ERR D7]: The subtraction length is larger than the"
                      " number of characters in the base value");
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
    dissect_value (length_node, position, dnode);

    vec->nbytes = fdata->value.u32;

    /* Get the byte vector. */
    position->offjmp = vec->nbytes;

    vec->bytes = g_malloc ((1+vec->nbytes) * sizeof(guint8));

    if (vec->bytes) {
      decode_byte_vector (vec->nbytes, position->bytes, vec->bytes);
      vec->bytes[vec->nbytes] = 0;
    }

    ShiftBytes(position);
  }
  set_dictionary_value(ftype, fdata);
}


/*! \brief  Given a byte stream, dissect a group.
 *
 * \param tnode  Template tree node.
 * \param position  Position in the packet.
 * \param dnode  Dissect tree node.
 */
void dissect_group (const GNode* tnode,
                    DissectPosition* position, GNode* dnode)
{
  DissectPosition stacked_position;
  DissectPosition* nested_position;
  SetupDissectStack(ftype, fdata,  tnode, dnode);

  if (ftype->value.pmap_exists) {
    basic_dissect_pmap (position, &stacked_position);
    nested_position = &stacked_position;
  }
  else {
    stacked_position.pmap = NULL;
    nested_position = position;
  }

  /* Recurse down the tree, building onto dnode. */
  dissector_walk (tnode->children, nested_position, dnode, 0);

  position->offjmp = nested_position->offset - position->offset;
  ShiftBytes(position);

  if (stacked_position.pmap) {
    g_free(stacked_position.pmap);
  }
}


/*! \brief  Given a byte stream, dissect a sequence.
 *
 * \param tnode  Template tree node.
 * \param position  Position in the packet.
 * \param dnode  Dissect tree node.
 */
void dissect_sequence (const GNode* tnode,
                       DissectPosition* position, GNode* dnode)
{
  guint32 length;
  guint32 i;
  GNode* parent;
  GNode* length_tnode;
  GNode* group_tnode;
  SetupDissectStack(ftype, fdata,  tnode, dnode);

  if (!tnode->children || !tnode->children->next) {
    BAILOUT(;,"Error in sequence setup.");
  }
  length_tnode = tnode->children;
  group_tnode  = length_tnode->next;

  dissect_value (length_tnode, position, dnode);
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
    dnode = dissect_descend (group_tnode, position, parent, dnode);
  }
}

