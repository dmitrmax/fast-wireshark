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
 *         Really only used for printing tree data.
 */

#ifndef DEBUG_TREE_H_
#define DEBUG_TREE_H_ 

#include "template.h"


/*!
 * \brief Prints the field to debug.  Called by debug_print_field_tree.
 * \param field FieldType to be printed.
 * \param int The amount to indent the print.
 */
void debug_print_field (FieldType* field, int indent);


/*!
 * \brief Prints a field tree to debug.  This is a recursive function.
 * \param node The root node of the field tree.
 * \param level The current level of parsing we are at.
 */
void debug_print_field_tree (GNode * node, int level);


/*!
 * \brief Prints a template tree to debug.
 * \param template_tree Template tree to be printed
 */
void debug_print_template_tree(GNode* template_tree);

#endif

