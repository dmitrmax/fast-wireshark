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
#include <epan/address.h>
#include "basic-dissect.h"
#include "template.h"
#include "address-utils.h"

#define GLOBAL_DICTIONARY "global"
#define TEMPLATE_DICTIONARY "template"

/*!
 * \brief Clears the contents of all the dictionaries
 */
void clear_dictionaries(address src, address dest);

/*!
 * \brief Retrieves the previous value of the given field
 * Will do a deep copy of the stored value and return the copy.
 * Caller is responsible for memory.
 * If the field has a default or initial value and is undefined in the dictionary,
 * the default or initial value is returned
 * \param ftype The field to retrieve the previous value of
 * \param fdata Return value. The 'empty' and 'value' members will be set.
 * \return TRUE iff the field data was set from a previous value.
 */
gboolean get_dictionary_value(const FieldType* ftype, FieldData* fdata, address src, address dest);

/*!
 * \brief Sets the value of the field for future look up
 * Will do a deep copy of the given value to remove external modification.
 * \param ftype The field to set the value of.
 * \param fdata Data to store, only 'empty' and 'value' members matter here.
 */
void set_dictionary_value(const FieldType* ftype,
                          const FieldData* fdata, address src, address dest);

#endif

