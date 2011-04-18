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
 * \file  debug-tree.h
 * \brief  Useful tree debug functions.
 */

#ifndef DEBUG_TREE_H_
#define DEBUG_TREE_H_

#include "template.h"

void debug_print_field (FieldType* field, int indent);
void debug_print_field_tree (GNode * node, int level);
void debug_print_template_tree(GNode* template_tree);

#endif

