/*!
 * \file dissect.c
 * \brief  Handle payload dissection independent of Wireshark.
 */

#include "debug.h"
#include "decode.h"
#include "template.h"
#include "dictionaries.h"
#include "dissect.h"
  
/*! \brief Dissect a FAST message by the bytes.
 * \param nbytes  Total number of bytes in the message.
 * \param bytes  The FAST message, sized to /nbytes/.
 * \param parent  Return value. The message data is built under it.
 * \return  The template that was used to parse.
 */
const GNode* dissect_fast_bytes (guint nbytes, const guint8* bytes,
                                 GNode* parent)
{
  static guint32 template_id = 0;

  DissectPosition stacked_position;
  DissectPosition* position;
  const GNode* tmpl = 0; /* Template. */
  FieldData* fdata; /* Template TID data node. */

  position = &stacked_position;
  position->offjmp = 0;
  position->offset = 0;
  position->nbytes = nbytes;
  position->bytes  = bytes;

  basic_dissect_pmap (position, position);

  if (!position->pmap) {
    BAILOUT(0,"PMAP not set.");
  }

  /* Initialize head node. */
  fdata = (FieldData*) g_malloc (sizeof (FieldData));
  if (!fdata) {
    g_free (position->pmap);
    BAILOUT(0,"Could not allocate memory.");
  }
  parent->data  = fdata;
  fdata->start  = position->offset;
  fdata->nbytes = 0;
  fdata->empty  = TRUE;

  /* Figure out current Template ID. */
  if (dissect_shift_pmap (position)) {
    /* Decode from the stream. */
    position->offjmp = count_stop_bit_encoded (position->nbytes,
                                               position->bytes);
    fdata->nbytes = position->offjmp;
    template_id = decode_uint32 (position->offjmp,
                                 position->bytes);
    ShiftBytes(position);
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

  g_free(position->pmap);
  return tmpl;
}


/*! \brief  Construct a message data tree (of FieldData).
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
    dnode = dissect_type (tnode, position, parent, dnode);
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
GNode* dissect_type (const GNode* tnode,
                     DissectPosition* position,
                     GNode* parent, GNode* dnode)
{
  /* Map to the dissect functions. */
  static void (*dissect_fn_map[]) (const GNode*, DissectPosition*, GNode*) =
  {
    &dissect_uint32,
    &dissect_uint64,
    &dissect_int32,
    &dissect_int64,
    &dissect_decimal,
    &dissect_ascii_string,
    &dissect_byte_vector, /* Unicode string. */
    &dissect_byte_vector,
    &dissect_group,
    &dissect_sequence,
    0
  };
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

  /* Assure the appropriate function exists. */
  if (!dissect_fn_map[ftype->type]) {
    DBG1("Field type %u not implemented.", (guint) ftype->type);
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

  /* Initialize to sensible defaults. */
  fdata->start  = position->offset;
  fdata->nbytes = 0;
  fdata->empty  = TRUE;
  init_field_value(&fdata->value);

  if (!ftype->mandatory) {
    /* Set empty to false if there should be a value. */
    dissect_optional(tnode, position, dnode_next);
  }
  else {
    fdata->empty = FALSE;
  }

  if (!fdata->empty) {
    /* Call the dissect function. */
    (*dissect_fn_map[ftype->type]) (tnode, position, dnode_next);
  }
  if(!(dnode_next->parent)){
    g_node_destroy(dnode_next);
    /* As we are building the tree, the last node added will the the one we just made */
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


/*! \brief  Given a byte stream, check if an optional field is empty.
 * \param tnode  Template tree node.
 * \param position  Position in the packet.
 * \param dnode  Dissect tree node.
 */
void dissect_optional (const GNode* tnode,
                       DissectPosition* position, GNode* dnode)
{
  SetupDissectStack(ftype, fdata,  tnode, dnode);
  if (ftype->mandatory) {
    BAILOUT(;,"Don't call this function on a mandatory field.");
  }

  if (FieldTypeGroup == ftype->type) {
    fdata->empty = !dissect_shift_pmap(position);
  }
  else if (FieldOperatorConstant == ftype->op) {
    gboolean presence_bit;
    presence_bit = dissect_shift_pmap(position);
    if (presence_bit) {
      fdata->empty = FALSE;
    }
  }
  else {
    gboolean null_shifted;   
    null_shifted = dissect_shift_null(position);

    if (null_shifted) {
      fdata->nbytes = 1;
      switch (ftype->op) {
        case FieldOperatorNone:
        case FieldOperatorDelta:
          /* Do not set previous value. */
          break;
        case FieldOperatorDefault:
        case FieldOperatorCopy:
        case FieldOperatorIncrement:
        case FieldOperatorTail:
          set_dictionary_value(ftype, fdata);
          break;
        default:
          DBG0("Bad operator type.");
          break;
      }
    }
    else {
      fdata->empty = FALSE;
    }
  }
}

/*! \brief  Given a byte stream, dissect an unsigned 32bit integer.
 * \param tnode  Template tree node.
 * \param position  Position in the packet.
 * \param dnode  Dissect tree node.
 */
void dissect_uint32 (const GNode* tnode,
                     DissectPosition* position, GNode* dnode)
{
  gboolean presence_bit;
  SetupDissectStack(ftype, fdata,  tnode, dnode);

  switch (ftype->op) {
    case FieldOperatorNone:
      basic_dissect_uint32(position, fdata);
      if (!ftype->mandatory) {
        -- fdata->value.u32;
      }
      break;

    case FieldOperatorCopy:
      presence_bit = dissect_shift_pmap(position);
      if (presence_bit) {
        basic_dissect_uint32(position, fdata);
        if (!ftype->mandatory) {
          -- fdata->value.u32;
        }
        set_dictionary_value(ftype, fdata);
      } else {
        /* TODO: Check return status. */
        get_dictionary_value(ftype, fdata);
      }
      break;
        
    case FieldOperatorConstant:
      get_dictionary_value(ftype, fdata);
      break;

    case FieldOperatorIncrement:
    case FieldOperatorDefault:
    case FieldOperatorDelta:
    case FieldOperatorTail:
    default:
      /* TODO implement this stuff once dictionaries are implemented */
      DBG0("Only simple types are implemented.");
      break;
  }
}


/*! \brief  Given a byte stream, dissect an unsigned 64bit integer.
 * \param tnode  Template tree node.
 * \param position  Position in the packet.
 * \param dnode  Dissect tree node.
 */
void dissect_uint64 (const GNode* tnode,
                     DissectPosition* position, GNode* dnode)
{
  SetupDissectStack(ftype, fdata,  tnode, dnode);

  if (!ftype->op) {
    basic_dissect_uint64 (position, fdata);
    if (!ftype->mandatory) {
      -- fdata->value.u64;
    }
  }
  else {
    DBG0("Only simple types are implemented.");
  }
}

/*! \brief  Given a byte stream, dissect a signed 32bit integer.
 * \param tnode  Template tree node.
 * \param position  Position in the packet.
 * \param dnode  Dissect tree node.
 */
void dissect_int32 (const GNode* tnode,
                    DissectPosition* position, GNode* dnode)
{
  SetupDissectStack(ftype, fdata,  tnode, dnode);

  if (!ftype->op) {
    basic_dissect_int32 (position, fdata);
    if (!ftype->mandatory && fdata->value.i32 > 0) {
      -- fdata->value.i32;
    }
  }
  else {
    DBG0("Only simple types are implemented.");
  }
}


/*! \brief  Given a byte stream, dissect a signed 64bit integer.
 * \param tnode  Template tree node.
 * \param position  Position in the packet.
 * \param dnode  Dissect tree node.
 */
void dissect_int64 (const GNode* tnode,
                    DissectPosition* position, GNode* dnode)
{
  SetupDissectStack(ftype, fdata,  tnode, dnode);

  if (!ftype->op) {
    basic_dissect_int64 (position, fdata);
    if (!ftype->mandatory && fdata->value.i64 > 0) {
      -- fdata->value.i64;
    }
  }
  else {
    DBG0("Only simple types are implemented.");
  }
}


/*! \brief  Given a byte stream, dissect a decimal number.
 * \param tnode  Template tree node.
 * \param position  Position in the packet.
 * \param dnode  Dissect tree node.
 */
void dissect_decimal (const GNode* tnode,
                      DissectPosition* position, GNode* dnode)
{
  SetupDissectStack(ftype, fdata,  tnode, dnode);

  /* Assure existence of 2 child nodes. */
  if (!tnode->children || !tnode->children->next) {
    BAILOUT(;,"Error in internal decimal field setup.");
  }

  if (!ftype->op) {
    GNode* expt_node;
    GNode* mant_node;

    /* Grab exponent. */
    expt_node = dissect_type (tnode->children, position,
                              dnode, 0);
    /* Grab mantissa. */
    mant_node = dissect_type (tnode->children->next, position,
                              dnode, expt_node);

    /* Count how many bytes were used. */
    fdata->nbytes = position->offset - fdata->start;
  }
  else {
    DBG0("Only simple types are implemented.");
  }
}


/*! \brief  Given a byte stream, dissect an ASCII string.
 * \param tnode  Template tree node.
 * \param position  Position in the packet.
 * \param dnode  Dissect tree node.
 */
void dissect_ascii_string (const GNode* tnode,
                           DissectPosition* position, GNode* dnode)
{
  SetupDissectStack(ftype, fdata,  tnode, dnode);

  if (!ftype->op) {
    basic_dissect_ascii_string (position, fdata);
  }
  else {
    DBG0("Only simple types are implemented.");
  }
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
  SetupDissectStack(ftype, fdata,  tnode, dnode);

  length_node = tnode->children;
  if (!length_node) {
    BAILOUT(;,"Length should be a child node.");
  }

  if (!ftype->op) {
    SizedData* vec;
    vec = &fdata->value.bytevec;

    /* See how big the byte vector is. */
    dissect_uint32 (length_node, position, dnode);

    vec->nbytes = fdata->value.u32;

    /* Get the byte vector. */
    position->offjmp = vec->nbytes;

    vec->bytes = g_malloc ((1+vec->nbytes) * sizeof(guint8));

    if (vec->bytes) {
      decode_byte_vector (vec->nbytes, position->bytes, vec->bytes);
      vec->bytes[vec->nbytes] = 0;
    }

    ShiftBytes(position);
    fdata->nbytes = position->offset - fdata->start;
  }
  else {
    DBG0("Only simple types are implemented.");
  }
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
  fdata->nbytes = position->offset - fdata->start;

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

  dissect_uint32 (length_tnode, position, dnode);
  length = fdata->value.u32;

  parent = dnode;
  dnode  = 0;

  for (i = 0; i < length; ++i) {
    dnode = dissect_type (group_tnode, position, parent, dnode);
  }
  fdata->nbytes = position->offset - fdata->start;
}

