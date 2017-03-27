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
#include "config.h"
#include <epan/wmem/wmem.h>
#include "basic-field.h"

/*! \brief  Hold data relevant to a template definition.
 */
struct field_type_struct
{
  char* name;
  gint id;  /* Field id. Typed for the hash lookup. */
  gint tid;  /* id of the template or id of the template a field is in */
  char* key;
  gboolean mandatory;
  FieldTypeIdentifier type;
  FieldOperatorIdentifier op;
  gboolean hasDefault;
  FieldValue value;
  char * dictionary; /* Name of the dictionary used for this field */

};
typedef struct field_type_struct FieldType;

/*! \brief  Creates templates lookup table for a given templates tree.
 * \param templ  The root of the templates tree.
 */
wmem_map_t* create_templates_table(GNode* tmpl);

/*!
 * \brief  Retrieve the name of the field type.
 * \param type  Field type for the name lookup.
 * \return  A string corresponding to the type given.
 *          If the type is invalid, return an empty string.
 */
const gchar* field_typename (FieldTypeIdentifier type);

/*!
 * \brief  Retrieve the name of the operator type.
 * \param type  Operator type for the name lookup.
 * \return  A string corresponding to the type given.
 *          If the type is invalid, return an empty string.
 */
const gchar* operator_typename (FieldOperatorIdentifier type);

/*!
 * \brief  Create internal representation for a field wrapped in a GNode.
 * \param  type  Type of the field to be created.
 * \param  op  Any operator to be used with the new field.
 * \return  The new, initialized node holding a non-null FieldType.
 *          NULL if a malloc failed.
 */
GNode* create_field (FieldTypeIdentifier type,
                     FieldOperatorIdentifier op);

#endif

/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 2
 * tab-width: 2
 * indent-tabs-mode: nil
 * End:
 */
