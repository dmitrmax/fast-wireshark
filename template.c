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
 * \file template.c
 * \brief  Handle template storage/lookups.
 */
#include "config.h"
#include "wmem_aux.h"
#include "debug.h"
#include "decode.h"
#include "template.h"

static gboolean requires_pmap_bit (const FieldType* ftype);
static void fixup_walk_template (FieldType* parent, GNode* parent_node);

wmem_map_t* create_templates_table(GNode* templates)
{
  GNode* tmpl;
  wmem_map_t* result = wmem_map_new(wmem_epan_scope(), &g_int_hash, &g_int_equal);

  /* Loop thru templates, add each to lookup table. */
  for (tmpl = templates->children; tmpl; tmpl = tmpl->next) {
    FieldType* tfield = (FieldType*) tmpl->data;
    tfield->value.pmap_exists = TRUE;
    fixup_walk_template (tfield, tmpl);
    wmem_map_insert(result, &tfield->id, tmpl);
  }

  return result;
}

const gchar* field_typename (FieldTypeIdentifier type)
{
  static const gchar* names[] =
  {
    "uInt32", "uInt64", "int32", "int64",
    "decimal", "ascii", "unicode", "byteVector",
    "group", "sequence"
  };
  if (0 <= type && type < FieldTypeEnumLimit) {
    return names[type];
  }
  else {
    DBG1("Unknown type %d", type);
    return "";
  }
}


const gchar* operator_typename (FieldOperatorIdentifier type)
{
  static const gchar* names[] =
  {
    "no_operator", "constant", "default", "copy",
    "increment", "delta", "tail"
  };
  if(0<= type && type < FieldOperatorEnumLimit) {
    return names[type];
  }
  else {
    DBG1("Unknown type %d", type);
    return "";
  }
}


GNode* create_field (FieldTypeIdentifier type,
                     FieldOperatorIdentifier op)
{
  FieldType* field = (FieldType*)wmem_new(wmem_epan_scope(), FieldType);
  GNode* node = wmem_node_new (wmem_epan_scope(), field);

  field->name       = 0;
  field->id         = 0;
  field->key        = 0;
  field->mandatory  = TRUE;
  field->type       = type;
  field->op         = op;
  field->hasDefault = FALSE;
  init_field_value(&field->value);
  field->dictionary = 0;

  return node;
}

/*! \brief  Check if a field type needs a bit in the PMAP. */
gboolean requires_pmap_bit (const FieldType* ftype)
{
  if (ftype->type == FieldTypeGroup) {
    return !ftype->mandatory;
  }
  switch (ftype->op) {
    case FieldOperatorConstant:
      return !ftype->mandatory;
    case FieldOperatorDefault:
    case FieldOperatorCopy:
    case FieldOperatorIncrement:
    case FieldOperatorTail:
      return TRUE;
    default:
      return FALSE;
  }
}

/*! \brief  Propagate data down and up the type tree.
 *
 * \param parent  The parent group. Not necessarily contained in parent_node.
 * \param parent_node  The node on whose children this function will operate.
 */
void fixup_walk_template (FieldType* parent, GNode* parent_node)
{
  GNode* tnode;
  for (tnode = parent_node->children;  tnode;  tnode = tnode->next) {
    FieldType* ftype;
    ftype = (FieldType*) tnode->data;
    if (!ftype) {
      DBG0("Null field type.");
      continue;
    }
    if (!parent->value.pmap_exists) {
      if (requires_pmap_bit (ftype)) {
        parent->value.pmap_exists = TRUE;
      }
    }
    /* Only have this field as a parent to recursion if it is a group
     * as only then will it be able to contain a PMAP.
     */
    if (FieldTypeGroup == ftype->type) {
      fixup_walk_template (ftype, tnode);
    }
    else {
      fixup_walk_template (parent, tnode);
    }
  }
}


/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 2
 * tab-width: 2
 * indent-tabs-mode: nil
 * End:
 */
