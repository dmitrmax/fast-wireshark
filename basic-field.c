
#include <string.h>

#include "debug.h"

#include "basic-field.h"

/*! \brief  Initialize the memory of a FieldValue.
 */
void init_field_value (FieldValue* value)
{
  memset(value, 0, sizeof(FieldValue));
}

/*! \brief  Copy a FieldValue, malloc'ing if needed. */
void copy_field_value (FieldTypeIdentifier type,
                       const FieldValue* src,
                       FieldValue* dest)
{
  switch (type) {
    case FieldTypeUInt32:
    case FieldTypeUInt64:
    case FieldTypeInt32:
    case FieldTypeInt64:
    case FieldTypeDecimal:
      memcpy(dest, src, sizeof(FieldValue));
      break;
    case FieldTypeAsciiString:
    case FieldTypeUnicodeString:
    case FieldTypeByteVector:
      dest->bytevec.nbytes = src->bytevec.nbytes;
      dest->bytevec.bytes = g_memdup(src->bytevec.bytes,
                                     src->bytevec.nbytes * sizeof(guint8));
      break;
    default:
      DBG0("Called with bad type.");
      break;
  }
}

/*! \brief  Clean up a FieldValue's data members. */
void cleanup_field_value (FieldTypeIdentifier type, FieldValue* value)
{
  switch (type) {
    case FieldTypeUInt32:
    case FieldTypeUInt64:
    case FieldTypeInt32:
    case FieldTypeInt64:
    case FieldTypeDecimal:
      /* Nothing. */
      break;
    case FieldTypeAsciiString:
    case FieldTypeUnicodeString:
    case FieldTypeByteVector:
      value->bytevec.nbytes = 0;
      if (value->bytevec.bytes) {
        g_free(value->bytevec.bytes);
        value->bytevec.bytes = 0;
      }
      break;
    default:
      DBG0("Called with bad type.");
      break;
  }
}

