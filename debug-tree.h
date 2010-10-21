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

