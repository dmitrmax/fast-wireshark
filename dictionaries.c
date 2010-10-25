/*!
 * \file dictionaries.c
 * \brief  Handle dictionary setting on field types in templates
 */

#include <string.h>

#include "debug.h"

#include "dictionaries.h"

/*!
 * \brief Struct to allow a field type to be associated with the value.
 * Dictionary stored values are typed because it is an error to attempt to use
 * mismatched types when retrieving values.
 */
struct typed_value_struct
{
  FieldTypeIdentifier type;
  gboolean empty;
  FieldValue value;
};
typedef struct typed_value_struct TypedValue;


/* Private (static) headers. */
static GHashTable* dictionaries_table = 0;


/*!
 * \brief Retrieves a dictionary by name, or creates it if it doesnt exist.
 * \param name The name of the dictionary to retrieve
 */
static GHashTable* get_dictionary(char* name);

/*!
 * \brief Removes a dictionary by name from the dictionary lookup.
 * Will not delete the dictionary, only remove the dictionary lookup table's
 * reference to it.
 * \param name The name of the dictionary to remove
 */
static void remove_dictionary(char* name);

/*!
 * \brief Traverses the template tree and sets the dictionary that should be used at each field.
 * \param parent  The parent field. It should have a dictionary set.
 * \param parent_node  The parent's node in the tree
 */
void set_dictionary_pointers(const FieldType* parent, GNode* parent_node);

void set_dictionaries(GNode* template_tree){
  GNode* template = 0;
  if(!dictionaries_table) {
    dictionaries_table = g_hash_table_new(&g_str_hash, &g_str_equal);
  }
  /* If we fail to make the table bail */
  if(!dictionaries_table) { BAILOUT(;,"Dictionary lookup table not created."); }
  template = g_node_first_child(template_tree);
  /* Create the gobal dictionary */
  get_dictionary(GLOBAL_DICTIONARY);
  
  /* 
   * Loop through all the templates, add and remove template dictionaries from the dictionary
   * reference for each template_tree
   */
  while(template){
    FieldType* field_type;

    field_type = (FieldType*) template->data;
    if (!field_type->dictionary) {
      field_type->dictionary = g_strdup(GLOBAL_DICTIONARY);
    }
    field_type->dictionary_ptr = get_dictionary(field_type->dictionary);

    get_dictionary(TEMPLATE_DICTIONARY);
    set_dictionary_pointers(field_type, template);
    remove_dictionary(TEMPLATE_DICTIONARY);
    template = g_node_next_sibling(template);
  }

}

GHashTable* get_dictionary(char* name){
  GHashTable* dictionary = 0;
  dictionary = g_hash_table_lookup(dictionaries_table, name);
  if(!dictionary){
    dictionary = g_hash_table_new(&g_str_hash, &g_str_equal);
    g_hash_table_insert(dictionaries_table, name, dictionary);
  }
  return dictionary;
}

void remove_dictionary(char* name){
  if(!g_hash_table_remove(dictionaries_table, name)){
    DBG1("Attempt to remove non-existant dictionary: %s", name);
  }
}

void set_dictionary_pointers(const FieldType* parent, GNode* parent_node)
{
  GNode* node = 0;
  if (!parent_node) {
    BAILOUT(;,"Passed NULL parent node.");
  }
  if (!parent->dictionary_ptr) {
    BAILOUT(;,"Require parent field to have a dictionary.");
  }

  for (node = parent_node->children;  node;  node = node->next) {
    FieldType* field_type = 0;
    field_type = (FieldType*)(node->data);

    if (field_type->dictionary) {
      field_type->dictionary_ptr = get_dictionary(field_type->dictionary);
    }
    else {
      field_type->dictionary_ptr = parent->dictionary_ptr;
    }
    if (!field_type->empty) {
      FieldData fdata;
      fdata.empty = FALSE;
      /* Not using copy_field_value() to avoid extra malloc/free. */
      memcpy(&fdata.value, &field_type->value, sizeof(FieldValue));
      set_dictionary_value(field_type, &fdata);
    }
    /* Recurse into child nodes */
    set_dictionary_pointers(field_type, node);
  }
}

gboolean get_dictionary_value(const FieldType* ftype,
                              FieldData* fdata)
{
  gboolean found = FALSE;
  GHashTable* dictionary = 0;
  const TypedValue* prev = 0;
  dictionary = (GHashTable*)ftype->dictionary_ptr;
  prev = g_hash_table_lookup(dictionary,ftype->key);
  if (prev) {
    if (prev->type == ftype->type) {
      found = TRUE;
      fdata->empty = prev->empty;
      if (!fdata->empty) {
        copy_field_value(ftype->type, &prev->value, &fdata->value);
      }
    }
    else {
      DBG2("[%s] Retrieved differently typed value for %s.",
          "ERR D4", ftype->name);
    }
  }
  return found;
}

void set_dictionary_value(const FieldType* ftype,
                          const FieldData* fdata)
{
  GHashTable* dictionary = 0;
  TypedValue* prev_value = 0;
  TypedValue* new_value = 0;
  dictionary = (GHashTable*)ftype->dictionary_ptr;
  prev_value = g_hash_table_lookup(dictionary, ftype->key);
  if (prev_value) {
    cleanup_field_value(ftype->type, &prev_value->value);
    new_value = prev_value;
  }
  else {
    new_value = g_malloc(sizeof(TypedValue));
  }
  new_value->type = ftype->type;
  new_value->empty = fdata->empty;
  if (!new_value->empty) {
    copy_field_value(ftype->type, &fdata->value, &new_value->value);
  }
  if (new_value) {
    /* Only have to insert if we created a new value. */
    if (!prev_value) {
      g_hash_table_insert(dictionary, ftype->key, new_value);
    }
  } else {
    DBG1("Failed to set value for field type: %s", ftype->name);
  }
}

