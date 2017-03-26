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
 * \file  debug-tree.c
 * \brief  Tree debugging output functions.
 */

#include <glib/gprintf.h>

#include "debug-tree.h"

void debug_print_template_tree(GNode* template_tree){
  GNode* template_node = 0;
  template_node = g_node_first_child(template_tree);

  /*
   * Loop through all the templates to print
   */
  while(template_node){

    debug_print_field_tree(template_node, 0);
    template_node = g_node_next_sibling(template_node);
  }
}

void debug_print_field_tree (GNode * node, int level){

  GNode* child = 0;
  FieldType* field_type = 0;
  field_type = (FieldType*)(node->data);
  debug_print_field (field_type, level*2);
  child = g_node_first_child(node);
  /* Recurse into child nodes */
  while(child){
    debug_print_field_tree (child, level+1);
    child = g_node_next_sibling(child);
  }

}

void debug_print_field (FieldType* field, int indent){
  fprintf(stderr,"%*stype: %s name: %s id: %d operator: %s dictionaryname: %s key: %s tid: %d\n",
       indent, "",
       field_typename (field->type),
       field->name,
       field->id,
       operator_typename (field->op),
       field->dictionary,
       field->key,
       field->tid);
}
