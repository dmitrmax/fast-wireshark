/*!
 * \file dissect-read.c
 * \brief  Primitive readers for the byte stream
 *         using the dissector structures.
 */
#include "basic-dissect.h"

#include "debug.h"
#include "decode.h"
#include <string.h>

/*! \brief The sign bit for a 5 byte encoded Int32 */
#define Int32SignBit 0x08
/*! \brief The sign bit for a 10 byte encoded Int64 */
#define Int64SignBit 0x01
/*! \brief The extra bits after the sign bit for an Int32 comprised of 5 bytes */
#define Int32ExtraBits 0x70
/*! \brief The extra bits after the sign bit for an Int64 comprised of 10 bytes */
#define Int64ExtraBits 0x7E
/*! \brief The maximum number of stop bit encoded bytes an Int32 can occupy */
#define Int32MaxBytes 5
/*! \brief The maximum number of stop bit encoded bytes an Int64 can occupy */
#define Int64MaxBytes 10


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
 * \param position  The dissector's current position.
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
#ifdef OVERRUN_DEBUG
    DBG2("PMAP index out of bounds at %u (length %u).",
         position->pmap_idx,
         position->pmap_len);
#endif
  }
  return result;
}


/*! \brief  Retrieve a bit in the PMAP.
 * \param position  The dissector's current position.
 * \return  TRUE or FALSE depending on the PMAP bit value.
 */
gboolean dissect_peek_pmap (DissectPosition* position)
{
  gboolean result;
  result = dissect_shift_pmap (position);
  position->pmap_idx -= 1;
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
#ifdef OVERRUN_DEBUG
    DBG0("index out of bounds (nbytes == 0)");
#endif
  }
  return ret;
}

/*! \brief  Copy current position to a nested one (with a new pmap).
 *
 * Both arguments may be the same.
 *
 * \param parent_position  Position in the packet.
 * \param position  Return value. Moved position with a new pmap.
 */
void basic_dissect_pmap (const DissectPosition* parent_position,
                         DissectPosition* position)
{
  position->offjmp = parent_position->offjmp;
  position->offset = parent_position->offset;
  position->nbytes = parent_position->nbytes;
  position->bytes  = parent_position->bytes;
  position->pmap_len = 0;
  position->pmap_idx = 0;
  position->pmap     = 0;

  /* Decode the pmap. */
  position->offjmp = count_stop_bit_encoded (position->nbytes,
                                             position->bytes);

  position->pmap_len = number_decoded_bits (position->offjmp);
  if (position->pmap_len == 0) {
    BAILOUT(;,"PMAP length is zero bytes?");
  }

  position->pmap = g_malloc (position->pmap_len * sizeof(gboolean));
  if (!position->pmap) {
    position->pmap_len = 0;
    BAILOUT(;,"Could not allocate memory.");
  }

  decode_pmap (position->offjmp, position->bytes, position->pmap);
  ShiftBytes(position);
}

/*! \brief  Given a byte stream, dissect an unsigned 32bit integer.
 * \param position  Position in the packet.
 * \param dnode  Dissect tree node.
 */
void basic_dissect_uint32 (DissectPosition* position, FieldData* fdata)
{
  position->offjmp = count_stop_bit_encoded (position->nbytes,
                                             position->bytes);
  fdata->nbytes = position->offjmp;
  fdata->value.u32 = decode_uint32 (position->offjmp,
                                    position->bytes);
  if (Int32MaxBytes == fdata->nbytes) {
    if ((position->bytes[0] & Int32ExtraBits) > 0) {
      fdata->status = FieldError;
      fdata->value.ascii.bytes = (guint8*)g_strdup_printf("[ERR D2] Too many bits for UInt32");
    }
  }else if(Int32MaxBytes < fdata->nbytes) {
    fdata->status = FieldError;
    fdata->value.ascii.bytes = (guint8*)g_strdup_printf("[ERR D2] Too many bytes for UInt32");
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
  fdata->nbytes = position->offjmp;
  fdata->value.u64 = decode_uint64 (position->offjmp,
                                    position->bytes);
   if (Int64MaxBytes == fdata->nbytes) {
    if ((position->bytes[0] & Int64ExtraBits) > 0) {
      fdata->status = FieldError;
      fdata->value.ascii.bytes = (guint8*)g_strdup_printf("[ERR D2] Too many bits for UInt64");
    }
  }else if(Int64MaxBytes < fdata->nbytes) {
    fdata->status = FieldError;
    fdata->value.ascii.bytes = (guint8*)g_strdup_printf("[ERR D2] Too many bytes for UInt64");
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
  fdata->nbytes = position->offjmp;
  fdata->value.i32 = decode_int32 (position->offjmp,
                                   position->bytes);
  if (Int32MaxBytes == fdata->nbytes) {
    if ((position->bytes[0] & Int32SignBit) == Int32SignBit) {
      if ((position->bytes[0] & Int32ExtraBits) != Int32ExtraBits) {
        fdata->status = FieldError;
        fdata->value.ascii.bytes = (guint8*)g_strdup_printf("[ERR D2] Too many bits for Int32");
      }
    } else {
      if ((position->bytes[0] & Int32ExtraBits) > 0) {
        fdata->status = FieldError;
        fdata->value.ascii.bytes = (guint8*)g_strdup_printf("[ERR D2] Too many bits for Int32");
      }
    }
  }
  else if(Int32MaxBytes < fdata->nbytes) {
    fdata->status = FieldError;
    fdata->value.ascii.bytes = (guint8*)g_strdup_printf("[ERR D2] Too many bytes for Int32");
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
  fdata->nbytes = position->offjmp;
  fdata->value.i64 = decode_int64 (position->offjmp,
                                   position->bytes);
  if (Int64MaxBytes == fdata->nbytes) {
    if ((position->bytes[0] & Int64SignBit) == Int64SignBit) {
      if ((position->bytes[0] & Int64ExtraBits) != Int64ExtraBits) {
        fdata->status = FieldError;
        fdata->value.ascii.bytes = (guint8*)g_strdup_printf("[ERR D2] Too many bits for Int64");
      }
    } else {
      if ((position->bytes[0] & Int64ExtraBits) > 0) {
        fdata->status = FieldError;
        fdata->value.ascii.bytes = (guint8*)g_strdup_printf("[ERR D2] Too many bits for Int64");
      }
    }
  }else if(Int64MaxBytes < fdata->nbytes) {
    fdata->status = FieldError;
    fdata->value.ascii.bytes = (guint8*)g_strdup_printf("[ERR D2] Too many bytes for Int64");
  }
  ShiftBytes(position);
}


/*! \brief  Given a byte stream, dissect an ASCII string.
 * \param position  Position in the packet.
 * \param fdata  Result data.
 */
void basic_dissect_ascii_string (DissectPosition* position, FieldData* fdata)
{
  guint8* bytes;
  guint32 nbytes;
  position->offjmp = count_stop_bit_encoded (position->nbytes,
                                             position->bytes);
  nbytes = position->offjmp;
  fdata->nbytes = nbytes;
  bytes = g_malloc ((1+ nbytes) * sizeof(guint8));
  if (bytes) {
    decode_ascii_string (position->offjmp, position->bytes, bytes);
    bytes[nbytes] = 0;
    fdata->value.ascii.bytes = bytes;
    fdata->value.ascii.nbytes = strlen((char*) bytes);
  }
  ShiftBytes(position);
}

