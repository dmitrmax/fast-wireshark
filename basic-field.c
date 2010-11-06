#include <stdlib.h>
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
                                     (1+src->bytevec.nbytes) * sizeof(guint8));
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

void string_to_field_value(const char* str, FieldTypeIdentifier type, FieldValue* value)
{
    switch (type) {
      int i;
      int len;
      
      case FieldTypeUInt32:
        value->u32 = atoi((char*)str);
        break;
      case FieldTypeInt32:
        value->i32 = atoi((char*)str);
        break;
      case FieldTypeUInt64:
        value->u64 = g_ascii_strtoull((char*)str, NULL, 10);
        break;
      case FieldTypeInt64:
        value->i64 = g_ascii_strtoll((char*)str, NULL, 10);
        break;
        
      case FieldTypeDecimal:
        len = strlen(str);
        
        DBG1("decimal string: %s", str);
        break;
        
      case FieldTypeAsciiString:
      case FieldTypeUnicodeString:
        value->bytevec.nbytes = strlen(str);
        value->bytevec.bytes = (guint8*)g_strdup(str);
        break;
      
      case FieldTypeByteVector:
        len = strlen(str);
        value->bytevec.nbytes = len/2;
        value->bytevec.bytes = g_malloc(1+value->bytevec.nbytes);
        for(i=0; i + 1 < len; i += 2) {
          int nibble1;
          int nibble2;
          nibble1 = g_ascii_xdigit_value(str[i]);
          nibble2 = g_ascii_xdigit_value(str[i+1]);
          
          if(-1 != nibble1 && -1 != nibble2) {
            value->bytevec.bytes[i/2] = (nibble1 << 4) | nibble2;
          }
          else {
            value->bytevec.bytes[i/2] = 0;
            DBG2("invalid character: index %d in string [%s]", i, str);
          }
        }
        
        break;
      default:
        DBG0("Called with bad type.");
        break;
    }
}

