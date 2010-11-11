
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"

#include "basic-field.h"

static void  string_to_decimal_value (const char* str, FieldValue* value);

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
        value->u32 = atoi(str);
        break;
      case FieldTypeInt32:
        value->i32 = atoi(str);
        break;
      case FieldTypeUInt64:
        value->u64 = g_ascii_strtoull(str, NULL, 10);
        break;
      case FieldTypeInt64:
        value->i64 = g_ascii_strtoll(str, NULL, 10);
        break;
        
      case FieldTypeDecimal:
        string_to_decimal_value (str, value);
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


/*! \brief  Translate a string to a decimal value. */
void  string_to_decimal_value (const char* str, FieldValue* value)
{
  unsigned left, right, len;
  unsigned dot;
  unsigned i;
  unsigned start, end;
  char* buf;

  len = strlen(str);
  start = 0;
  end   = len -1;
  left  = end;
  right = start;
  dot   = len;

  buf = g_strdup (str);
  if (!buf) {
    BAILOUT(;,"Failed malloc");
  }

  for (i = 0; i < len; ++i) {
    switch (buf[i]) {
      case '-':
      case '+':
        start = i+1;
        break;
      case '0':
        break;
      case '.':
        dot = i;
        break;
      default:
        if (i < left)   left  = i;
        if (i > right)  right = i;
        break;
    }
  }

  if (dot <= right) {
    /* Scoot that memory over. */
    memmove (dot+buf, 1+dot+buf, right-dot);
    right -= 1;
  }
  dot -= 1;  /* Now sitting on the 10^0 digit. */
  value->decimal.exponent = (gint32) dot - (gint32) right;

  end = 1+right;
  buf[end] = 0;
  value->decimal.mantissa = g_ascii_strtoll(buf, NULL, 10);
  if (0 == value->decimal.mantissa) {
    /* Our /right/ was never set correctly, could be invalid.
     * Thus, the exponent may be strange. Zero it.
     */
    value->decimal.exponent = 0;
  }

  g_free(buf);
}

