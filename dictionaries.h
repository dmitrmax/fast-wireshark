#ifndef DICTIONARIES_H_INCLUDED_
#define PDICTIONARIES_H_INCLUDED_
#include <glib.h>
#include "template.h"

struct typed_value_struct
{
  FieldTypeIdentifier type;
  GNode * node_value;
};
typedef struct typed_value_struct TypedValue;




#endif

