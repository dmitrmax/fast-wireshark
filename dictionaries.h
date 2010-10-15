/*!
 * \file dictionaries.h
 * \brief Dictionary structure definitions
 *
 * Define structures used for dictionaries.  These
 * structures are used to store and retrieve values
 * in dictionaries.
 */

#ifndef DICTIONARIES_H_INCLUDED_
#define DICTIONARIES_H_INCLUDED_
#include <glib.h>
#include "template.h"

#define GLOBAL_DICTIONARY "global"
#define TEMPLATE_DICTIONARY "template"

/*!
 * \brief Struct to allow a field type to be associated with the value.
 * Dictionary stored values are typed because it is an error to attempt to use
 * mismatched types when retrieving values.
 */
struct typed_value_struct
{
  FieldTypeIdentifier type;
  GNode * node_value;
};
typedef struct typed_value_struct TypedValue;

/*!
 * \brief Sets the dictionary pointer for each field tyep in the template tree
 * Sets the dictionary_ptr in each field type to the most specific dictionary
 * reference at that point.  Uses the value of dictionary to determine which
 * dictionary to use.
 * \param template_tree The tree containing all the templates to have dictionaries
 * set.
 */
void set_dictionaries(GNode* template_tree);
GNode* get_dictionary_value(FieldType field_type);
void set_dictionary_value(FieldType field_type, GNode* value);

#endif

