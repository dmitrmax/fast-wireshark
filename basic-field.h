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
 * \file basic-field.h
 * \brief Essential data-driven field definitions.
 *        Contains data types and conversions that are used for everything.
 */

#ifndef BASIC_FIELD_H_INCLUDED_
#define BASIC_FIELD_H_INCLUDED_
#include <glib.h>


/*! \brief Possible field Types.
 */
enum field_type_identifier_enum
{ 
  FieldTypeUInt32,
  FieldTypeUInt64,
  FieldTypeInt32,
  FieldTypeInt64,
  FieldTypeDecimal,
  FieldTypeAsciiString,
  FieldTypeUnicodeString,
  FieldTypeByteVector,
  FieldTypeGroup,
  FieldTypeSequence,
  FieldTypeError,
  FieldTypeEnumLimit,
  FieldTypeInvalid
};
typedef enum field_type_identifier_enum FieldTypeIdentifier;


/*! \brief Possible field Operators.
 */
enum field_operator_identifier_enum
{
  FieldOperatorNone,
  FieldOperatorConstant,
  FieldOperatorDefault,
  FieldOperatorCopy,
  FieldOperatorIncrement,
  FieldOperatorDelta,
  FieldOperatorTail,
  FieldOperatorEnumLimit
};
typedef enum field_operator_identifier_enum FieldOperatorIdentifier;


/*! \brief  Hold a size prefixed data chunk. 
 */
struct sized_data_struct
{
  guint nbytes;
  guint8* bytes;
};
typedef struct sized_data_struct SizedData;

/*! \brief  Hold a decimal value. 
 */
struct decimal_field_value_struct
{
  gint64 mantissa;
  gint32 exponent;
};
typedef struct decimal_field_value_struct DecimalFieldValue;

/*! \brief  Hold any type of simple field value. 
 */
union field_value_union
{
  guint32 u32;
  gint32 i32;
  guint64 u64;
  gint64 i64;
  DecimalFieldValue decimal;
  SizedData ascii;
  SizedData unicode;
  SizedData bytevec;
  gboolean pmap_exists;
};
typedef union field_value_union FieldValue;


/*! \brief  Initialize the memory of a FieldValue to blank.
 *  \param value The FieldValue to be initialized.
 */
void init_field_value (FieldValue* value);


/*! \brief  Copy a FieldValue, malloc'ing if needed. 
 *  \param type The type of the FieldValue.
 *  \param src FieldValue to be copied.
 *  \param dest FieldValue it will be copied to.
 */
void copy_field_value (FieldTypeIdentifier type, const FieldValue* src,
                       FieldValue* dest);


/*! \brief  Clean up a FieldValue's data members. 
 *  \param type The type of the FieldValue.
 *  \param value The FieldValue to be freed.
 */
void cleanup_field_value (FieldTypeIdentifier type, FieldValue* value);


/*! \brief  Translate a string to a decimal value. 
 *  \param str The string stored in the field to be converted.
 *  \param type The type of the FieldValue to convert to.
 *  \param value The FieldValue that the result will be stored in.
 */
gboolean string_to_field_value(const char* str, FieldTypeIdentifier type, FieldValue* value);

#endif

