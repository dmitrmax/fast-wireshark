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

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"

#include "basic-field.h"

static gboolean string_to_decimal_value (const char* str, FieldValue* value);
static gboolean is_number(const char* str);

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

static gboolean is_number(const char* str){
  int i;
  int len;
  len=strlen(str);
  for(i=0; i<len; i++){
    if(!g_ascii_isdigit(str[i]) && str[i]!='-'){
      return FALSE;
    }
  }
  return TRUE;
}

gboolean string_to_field_value(const char* str, FieldTypeIdentifier type, FieldValue* value)
{
    switch (type) {
      int i;
      int len;
      
      
      case FieldTypeUInt32:
        if(!is_number(str)){
          DBG1("not a number %s", str);
          return FALSE;
        }
        value->u32 = atoi(str);
        break;
      case FieldTypeInt32:
        if(!is_number(str)){
          DBG1("not a number %s", str);
          return FALSE;
        }
        value->i32 = atoi(str);
        break;
      case FieldTypeUInt64:
        if(!is_number(str)){
          DBG1("not a number %s", str);
          return FALSE;
        }
        value->u64 = g_ascii_strtoull(str, NULL, 10);
        break;
      case FieldTypeInt64:
        if(!is_number(str)){
          DBG1("not a number %s", str);
          return FALSE;
        }
        value->i64 = g_ascii_strtoll(str, NULL, 10);
        break;
        
      case FieldTypeDecimal:
        if(!string_to_decimal_value (str, value)){
          return FALSE;
        }
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
            return FALSE;
          }
        }
        
        break;
      default:
        DBG0("Called with bad type.");
        return FALSE;
        break;
    }
    return TRUE;
}


/*! \brief  Translate a string to a decimal value. */
gboolean string_to_decimal_value (const char* str, FieldValue* value)
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
    return FALSE;
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
        if(dot!=len){
          return FALSE;
        }
        dot = i;
        break;
      default:
        if(!g_ascii_isdigit(buf[i])){
          return FALSE;
        }
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
     * Thus, the exponent may be strange.
     */
    value->decimal.exponent = 0;
    /* return FALSE; */
  }

  g_free(buf);
  return TRUE;
}

