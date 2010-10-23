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
#include "basic-dissect.h"
#include "template.h"

#define GLOBAL_DICTIONARY "global"
#define TEMPLATE_DICTIONARY "template"

/*!
 * \brief Sets the dictionary pointer for each field tyep in the template tree
 * Sets the dictionary_ptr in each field type to the most specific dictionary
 * reference at that point.  Uses the value of dictionary to determine which
 * dictionary to use.
 * \param template_tree The tree containing all the templates to have
 * dictionaries set.
 */
void set_dictionaries(GNode* template_tree);

/*!
 * \brief Retrieves the previous value of the given field
 * Will do a deep copy of the stored value and return the copy.
 * Caller is responsible for memory.
 * \param ftype The field to retrieve the previous value of
 * \param fdata Return value. The 'empty' and 'value' members will be set.
 * \return TRUE iff the field data was set from a previous value.
 */
gboolean get_dictionary_value(const FieldType* ftype, FieldData* fdata);

/*!
 * \brief Sets the value of the field for future look up
 * Will do a deep copy of the given value to remove external modification.
 * \param ftype The field to set the value of.
 * \param fdata Data to store, only 'empty' and 'value' members matter here.
 */
void set_dictionary_value(const FieldType* ftype,
                          const FieldData* fdata);

#endif

