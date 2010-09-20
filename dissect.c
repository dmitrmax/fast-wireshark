/*!
 * \file dissect.c
 * \brief  Handle payload dissection independent of Wireshark.
 */

#include "dissect.h"
#include "fast.h"
#include "template.h"
#include "decode.h"

/*! \brief  Shift a buffer by a certain amount.
 *
 * All arguments are modified accordingly.
 *
 * \param offjmp  The amount to move forward in the buffer.
 * \param offset  Distance into the buffer currently.
 * \param nbytes  Number of indices remaining in the buffer.
 * \param bytes  Buffer that is offset by /offset/ from its real start.
 * \sa ShiftBytes
 */
#define ShiftBuffer(offjmp, offset, nbytes, bytes) \
  do { \
    offset += offjmp; \
    nbytes -= offjmp; \
    bytes   = offjmp + bytes; \
    offjmp  = 0; \
  } while (0)

/*! \brief  Shift the byte position of a dissection.
 * \sa ShiftBuffer
 */
#define ShiftBytes(positon) \
  ShiftBuffer(position->offjmp, position->offset, \
              position->nbytes, position->bytes)


/*! \brief  Claim and retrieve a bit in the PMAP.
 * \param position  The dissector's currect position.
 * \return  TRUE or FALSE depending on the PMAP bit value.
 */
gboolean dissect_shift_pmap (DissectPosition* position)
{
  gboolean result = FALSE;
  if (position->pmap_idx < position->pmap_len) {
    result = position->pmap[position->pmap_idx];
    position->pmap_idx += 1;
  }
  else {
    DBG2("PMAP index out of bounds at %u (length %u).",
         position->pmap_idx,
         position->pmap_len);
  }
  return result;
}

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

  position->pmap_len = 0;
  position->pmap_idx = 0;
  position->pmap     = 0;

  /* Decode the pmap. */
  position->offjmp = count_stop_bit_encoded (position->nbytes,
                                             position->bytes);

  position->pmap_len = number_decoded_bits (position->offjmp);
  if (position->pmap_len == 0)
    BAILOUT(0,"PMAP length is zero bytes?");

  position->pmap = g_malloc (position->pmap_len * sizeof(gboolean));
  if (!position->pmap)  BAILOUT(0,"Could not allocate memory.");

  decode_pmap (position->offjmp, position->bytes, position->pmap);
  ShiftBytes(position);

  /* Initialize head node. */
  fdata = (FieldData*) g_malloc (sizeof (FieldData));
  if (!fdata) {
    g_free (position->pmap);
    BAILOUT(0,"Could not allocate memory.");
  }
  parent->data  = fdata;
  fdata->start  = position->offset;
  fdata->nbytes = 0;
  fdata->value  = 0;

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
    0, /* &dissect_group, */
    0, /* &dissect_sequence, */
    0
  };
  const FieldType* ftype;
  FieldData* fdata;
  GNode* dnode_next;

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

  /* Call the dissect function. */
  (*dissect_fn_map[ftype->type]) (tnode, position, dnode_next);

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


/*! \brief  Given a byte stream, dissect an unsigned 32bit integer.
 * \param tnode  Template tree node.
 * \param position  Position in the packet.
 * \param dnode  Dissect tree node.
 */
void dissect_uint32 (const GNode* tnode,
                     DissectPosition* position, GNode* dnode)
{
  SetupDissectStack(ftype, fdata,  tnode, dnode);

  if (ftype->mandatory && !ftype->op) {
    position->offjmp = count_stop_bit_encoded (position->nbytes,
                                               position->bytes);
    fdata->start = position->offset;
    fdata->nbytes = position->offjmp;
    fdata->value = g_malloc (sizeof (guint32));
    if (fdata->value) {
      *(guint32*)fdata->value = decode_uint32 (position->offjmp,
                                               position->bytes);
    }
    ShiftBytes(position);
  }
  else {
    DBG0("Only simple types are implemented.");
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

  if (ftype->mandatory && !ftype->op) {
    position->offjmp = count_stop_bit_encoded (position->nbytes,
                                               position->bytes);
    fdata->start = position->offset;
    fdata->nbytes = position->offjmp;
    fdata->value = g_malloc (sizeof (guint64));
    if (fdata->value) {
      *(guint64*)fdata->value = decode_uint64 (position->offjmp,
                                               position->bytes);
    }
    ShiftBytes(position);
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

  if (ftype->mandatory && !ftype->op) {
    position->offjmp = count_stop_bit_encoded (position->nbytes,
                                               position->bytes);
    fdata->start = position->offset;
    fdata->nbytes = position->offjmp;
    fdata->value = g_malloc (sizeof (gint32));
    if (fdata->value) {
      *(gint32*)fdata->value = decode_int32 (position->offjmp,
                                             position->bytes);
    }
    ShiftBytes(position);
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

  if (ftype->mandatory && !ftype->op) {
    position->offjmp = count_stop_bit_encoded (position->nbytes,
                                               position->bytes);
    fdata->start = position->offset;
    fdata->nbytes = position->offjmp;
    fdata->value = g_malloc (sizeof (gint64));
    if (fdata->value) {
      *(gint64*)fdata->value = decode_int64 (position->offjmp,
                                             position->bytes);
    }
    ShiftBytes(position);
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

  if (ftype->mandatory && !ftype->op) {
    GNode* expt_node;
    GNode* mant_node;

    fdata->start = position->offset;

    /* Grab exponent. */
    expt_node = dissect_type (tnode->children, position,
                              dnode, 0);
    /* Grab mantissa. */
    mant_node = dissect_type (tnode->children->next, position,
                              dnode, expt_node);

    /* Count how many bytes were used. */
    fdata->nbytes = position->offset - fdata->start;
    fdata->value = 0;
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

  if (ftype->mandatory && !ftype->op) {
    position->offjmp = count_stop_bit_encoded (position->nbytes,
                                               position->bytes);
    fdata->start = position->offset;
    fdata->nbytes = position->offjmp;
    fdata->value = g_malloc (position->offjmp * sizeof(guint8));
    if (fdata->value) {
      decode_ascii_string (position->offjmp,
                           position->bytes,
                           (guint8*) fdata->value);
    }
    ShiftBytes(position);
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
  SetupDissectStack(ftype, fdata,  tnode, dnode);

  if (ftype->mandatory && !ftype->op) {
    guint vecsize;
    fdata->start = position->offset;
    fdata->nbytes = 0;

    /* See how big the byte vector is. */
    position->offjmp = count_stop_bit_encoded (position->nbytes,
                                               position->bytes);
    fdata->nbytes += position->offjmp;
    vecsize = decode_uint32 (position->offjmp,
                             position->bytes);
    ShiftBytes(position);

    /* Get the byte vector. */
    position->offjmp = vecsize;
    fdata->nbytes += position->offjmp;
    fdata->value = g_malloc (position->offjmp * sizeof(guint8));
    if (fdata->value) {
      decode_byte_vector (position->offjmp,
                          position->bytes,
                          (guint8*) fdata->value);
    }
    ShiftBytes(position);
  }
  else {
    DBG0("Only simple types are implemented.");
  }
}

