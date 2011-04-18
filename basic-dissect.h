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
 * \file dissect-read.h
 * \brief  Primitive readers for the byte stream
 *         Basically these are helper functions for dissect.c functions.
 *         While dissect.c does the logic dissection, these functions
 *         deal with actually dissecting the data within the messages.
 */
#ifndef BASIC_DISSECT_H_INCLUDED_
#define BASIC_DISSECT_H_INCLUDED_

#include "basic-field.h"

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

/*! \brief The different states a value can have throughout the application.
 */
enum field_status_enum
{
  FieldExists,
  FieldEmpty,
  FieldUndefined,
  FieldError
};
typedef enum field_status_enum FieldStatus;


/*! \brief  Identify the position of this field in the stream.
 */
struct field_data_struct
{
  guint start;
  guint nbytes;
  FieldStatus status;
  FieldValue value;
};
typedef struct field_data_struct FieldData;


/*! \brief  Hold current dissection state/position. */
struct dissect_position_struct
{
  guint offjmp; /* Number of bytes to the next offset. */
  guint offset; /* Current offset in the byte array. */
  guint nbytes;
  const guint8* bytes;

  guint pmap_len;
  guint pmap_idx;
  gboolean* pmap;
};
typedef struct dissect_position_struct DissectPosition;


/*! \brief  Throw a dynamic error
* \param err_no the dynamic error code
* \param fdata the field data
*/
void err_d(guint8 err_no, FieldData* fdata);


/*! \brief  Shift the byte position of a dissection.
 * \sa ShiftBuffer
 */
void ShiftBytes(DissectPosition* position);


/*! \brief  Claim and retrieve a bit in the PMAP.
 * \param position  The dissector's current position.
 * \return  TRUE or FALSE depending on the PMAP bit value.
 */
gboolean dissect_shift_pmap (DissectPosition* position);


/*! \brief  Retrieve a bit in the PMAP.
 * \param position  The dissector's current position.
 * \return  TRUE or FALSE depending on the PMAP bit value.
 */
gboolean dissect_peek_pmap (DissectPosition* position);


/*! \brief  Detect and skip null values.
 * \param position  The dissector's currect position.
 * \return  TRUE if a null value was detected and skipped.
 */
gboolean dissect_shift_null (DissectPosition* position);


/*! \brief  Copy current position to a nested one (with a new pmap).
 *
 * Both arguments may be the same.
 *
 * \param parent_position  Position in the packet.
 * \param position  Return value. Moved position with a new pmap.
 */
void basic_dissect_pmap (const DissectPosition* parent_position,
                         DissectPosition* position);


/*! \brief  Given a byte stream, dissect an unsigned 32bit integer.
 * \param position  Position in the packet.
 * \param dnode  Dissect tree node.
 */
void basic_dissect_uint32 (DissectPosition* position, FieldData* fdata);


/*! \brief  Given a byte stream, dissect an unsigned 64bit integer.
 * \param position  Position in the packet.
 * \param fdata  Result data.
 */
void basic_dissect_uint64 (DissectPosition* position, FieldData* fdata);


/*! \brief  Given a byte stream, dissect a signed 32bit integer.
 * \param position  Position in the packet.
 * \param fdata  Result data.
 */
void basic_dissect_int32 (DissectPosition* position, FieldData* fdata);


/*! \brief  Given a byte stream, dissect a signed 64bit integer.
 * \param position  Position in the packet.
 * \param fdata  Result data.
 */
void basic_dissect_int64 (DissectPosition* position, FieldData* fdata);


/*! \brief  Given a byte stream, dissect an ASCII string.
 * \param position  Position in the packet.
 * \param fdata  Result data.
 */
void basic_dissect_ascii_string (DissectPosition* position, FieldData* fdata);


#endif

