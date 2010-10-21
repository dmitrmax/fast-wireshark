/*!
 * \file dissect-read.c
 * \brief  Primitive readers for the byte stream
 *         using the dissector structures.
 */

#include "debug.h"
#include "decode.h"

#include "basic-dissect.h"

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
void ShiftBytes(DissectPosition* position)
{
  ShiftBuffer(position->offjmp, position->offset,
              position->nbytes, position->bytes);
}


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

/*! \brief  Detect and skip null values.
 * \param position  The dissector's currect position.
 * \return  TRUE if a null value was detected and skipped.
 */
gboolean dissect_shift_null(DissectPosition* position)
{
  gboolean ret = FALSE;
  if(position->nbytes > 0) {
    if(0x80 == position->bytes[0]) {
      ret = TRUE;
      position->offjmp = 1;
      ShiftBytes(position);    
    }
  }
  else {
    DBG0("index out of bounds (nbytes == 0)");
  }
  return ret;
}

/*! \brief  Given a byte stream, dissect an unsigned 32bit integer.
 * \param position  Position in the packet.
 * \param dnode  Dissect tree node.
 */
void basic_dissect_uint32 (DissectPosition* position, FieldData* fdata)
{
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


/*! \brief  Given a byte stream, dissect an unsigned 64bit integer.
 * \param position  Position in the packet.
 * \param fdata  Result data.
 */
void basic_dissect_uint64 (DissectPosition* position, FieldData* fdata)
{
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

/*! \brief  Given a byte stream, dissect a signed 32bit integer.
 * \param position  Position in the packet.
 * \param fdata  Result data.
 */
void basic_dissect_int32 (DissectPosition* position, FieldData* fdata)
{
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


/*! \brief  Given a byte stream, dissect a signed 64bit integer.
 * \param position  Position in the packet.
 * \param fdata  Result data.
 */
void basic_dissect_int64 (DissectPosition* position, FieldData* fdata)
{
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


/*! \brief  Given a byte stream, dissect an ASCII string.
 * \param position  Position in the packet.
 * \param fdata  Result data.
 */
void basic_dissect_ascii_string (DissectPosition* position, FieldData* fdata)
{
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

