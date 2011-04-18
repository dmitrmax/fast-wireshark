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
#include "basic-field.h"

/*! \brief  Hold data relevant to a template definition.
 */
struct field_type_struct
{
  char* name;
  /*! \brief  Field id. Typed for the hash lookup. */
  gint id;  /* id of the field or template */
  gint tid;  /* id of the template or id of the template a field is in */
  char* key;
  gboolean mandatory;
  FieldTypeIdentifier type;
  FieldOperatorIdentifier op;
  gboolean hasDefault;
  FieldValue value;
  /*! \brief Name of the dictionary used for this field */
  char * dictionary;
  
};
typedef struct field_type_struct FieldType;


void add_templates (GNode* tmpl);
const gchar* field_typename (FieldTypeIdentifier type);
const gchar* operator_typename (FieldOperatorIdentifier type);
GNode* create_field (FieldTypeIdentifier type,
                     FieldOperatorIdentifier op);

GNode* find_template (guint32 id);
GNode* full_templates_tree ();

#endif

