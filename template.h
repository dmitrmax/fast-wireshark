/*!
 * \file template.h
 * \brief Protocol template structures.
 *
 * Here we define most of the structures and information involved
 * in defining a FAST protocol template. These structures are used
 * to store the different static template information, as well as
 * the structure that stores the current state of the protocol.
 */

#ifndef TEMPLATE_H_INCLUDED_
#define TEMPLATE_H_INCLUDED_
#include "fast.h"

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
  FieldTypeEnumLimit
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

typedef void* FieldValue;

/*! \brief  Hold data relevant to a template definition.
 */
struct field_type_struct
{
  char* name;
  /*! \brief  Template id. Typed for the hash lookup. */
  gint id;
  gboolean mandatory;
  FieldTypeIdentifier type;
  FieldOperatorIdentifier op;
  FieldValue* value;
};
typedef struct field_type_struct FieldType;


void add_templates (GNode* tmpl);

GNode* create_field (FieldTypeIdentifier type,
                     FieldOperatorIdentifier op);

GNode* find_template (guint32 id);

#endif

