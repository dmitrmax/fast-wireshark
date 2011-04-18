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
 */

#ifndef BASIC_FIELD_H_INCLUDED_
#define BASIC_FIELD_H_INCLUDED_
#include <glib.h>

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


/*! \brief  Hold a size prefixed data chunk. */
struct sized_data_struct
{
  guint nbytes;
  guint8* bytes;
};
typedef struct sized_data_struct SizedData;

/*! \brief  Hold a decimal value. */
struct decimal_field_value_struct
{
  gint64 mantissa;
  gint32 exponent;
};
typedef struct decimal_field_value_struct DecimalFieldValue;

/*! \brief  Hold any type of simple field value. */
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

void init_field_value (FieldValue* value);
void copy_field_value (FieldTypeIdentifier type, const FieldValue* src,
                       FieldValue* dest);
void cleanup_field_value (FieldTypeIdentifier type, FieldValue* value);
gboolean string_to_field_value(const char* str, FieldTypeIdentifier type, FieldValue* value);

#endif

