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
 * \file basic-dissect.c
 * \brief  Implementation of the primitive data dissector.
 */

#include "config.h"

#include <string.h>
#include <epan/wmem/wmem.h>

#include "basic-dissect.h"

#include "debug.h"
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


void err_d(guint8 err_no, FieldData* fdata)
{
  char * string_err_d[15] =
  {
    "[ERR D1] ",
    "[ERR D2] Integer does not fall within the bounds of the specified type",
    "[ERR D3] ",
    "[ERR D4] Retrieved differently typed value from dictionary",
    "[ERR D5] Mandatory field not present, undefined previous value",
    "[ERR D6] Mandatory field not present, empty previous value",
    "[ERR D7] Invalid subtraction length",
    "[ERR D8] ",
    "[ERR D9] Template does not exist"
  };

  fdata->status = FieldError;
  fdata->value.ascii.bytes = (guint8*)wmem_strdup_printf(wmem_file_scope(), "%s", string_err_d[err_no - 1]);
}


void ShiftBytes(DissectPosition* position)
{
  ShiftBuffer(position->offjmp, position->offset,
              position->nbytes, position->bytes);
}


gboolean dissect_shift_pmap (DissectPosition* position)
{
  if (position->pmap_idx >= position->pmap_len) {
#ifdef OVERRUN_DEBUG
    DBG2("PMAP index out of bounds at %u (length %u).",
         position->pmap_idx,
         position->pmap_len);
#endif
    return FALSE;
  }

  return position->pmap[position->pmap_idx++];
}

gboolean dissect_peek_pmap (DissectPosition* position)
{
  if (position->pmap_idx >= position->pmap_len) {
#ifdef OVERRUN_DEBUG
    DBG2("PMAP index out of bounds at %u (length %u).",
         position->pmap_idx,
         position->pmap_len);
#endif
    return FALSE;
  }

  return position->pmap[position->pmap_idx];
}

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

  position->pmap = (gboolean*)wmem_alloc (wmem_packet_scope(), position->pmap_len * sizeof(gboolean));

  decode_pmap (position->offjmp, position->bytes, position->pmap);
  ShiftBytes(position);
}


void basic_dissect_uint32 (DissectPosition* position, FieldData* fdata)
{
  position->offjmp = count_stop_bit_encoded (position->nbytes,
                                             position->bytes);
  fdata->nbytes = position->offjmp;
  fdata->value.u32 = decode_uint32 (position->offjmp,
                                    position->bytes);
  if (Int32MaxBytes == fdata->nbytes) {
    if ((position->bytes[0] & Int32ExtraBits) > 0) {
      err_d(2, fdata);
    }
  }else if(Int32MaxBytes < fdata->nbytes) {
    err_d(2, fdata);
  }
  ShiftBytes(position);
}


void basic_dissect_uint64 (DissectPosition* position, FieldData* fdata)
{
  position->offjmp = count_stop_bit_encoded (position->nbytes,
                                             position->bytes);
  fdata->nbytes = position->offjmp;
  fdata->value.u64 = decode_uint64 (position->offjmp,
                                    position->bytes);
   if (Int64MaxBytes == fdata->nbytes) {
    if ((position->bytes[0] & Int64ExtraBits) > 0) {
      err_d(2, fdata);
    }
  }else if(Int64MaxBytes < fdata->nbytes) {
    err_d(2, fdata);
  }
  ShiftBytes(position);
}


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
	err_d(2, fdata);
      }
    } else {
      if ((position->bytes[0] & Int32ExtraBits) != 0) {
	err_d(2, fdata);
      }
    }
  }
  else if(Int32MaxBytes < fdata->nbytes) {
    err_d(2, fdata);
  }
  ShiftBytes(position);
}


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
	err_d(2, fdata);
      }
    } else {
      if ((position->bytes[0] & Int64ExtraBits) > 0) {
	err_d(2, fdata);
      }
    }
  }else if(Int64MaxBytes < fdata->nbytes) {
    err_d(2, fdata);
  }
  ShiftBytes(position);
}


void basic_dissect_ascii_string (DissectPosition* position, FieldData* fdata)
{
  guint8* bytes;
  guint32 nbytes;
  position->offjmp = count_stop_bit_encoded (position->nbytes,
                                             position->bytes);
  nbytes = position->offjmp;
  fdata->nbytes = nbytes;
  bytes = (guint8*)wmem_alloc (wmem_file_scope(), (1+ nbytes) * sizeof(guint8));
  if (bytes) {
    decode_ascii_string (position->offjmp, position->bytes, bytes);
    bytes[nbytes] = 0;
    fdata->value.ascii.bytes = bytes;
    fdata->value.ascii.nbytes = strlen((char*) bytes);
  }
  ShiftBytes(position);
}
