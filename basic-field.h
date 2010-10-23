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
};
typedef union field_value_union FieldValue;

void init_field_value (FieldValue* value);
void copy_field_value (FieldTypeIdentifier type, const FieldValue* src,
                       FieldValue* dest);
void cleanup_field_value (FieldTypeIdentifier type, FieldValue* value);

#endif

