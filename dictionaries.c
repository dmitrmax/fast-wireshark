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

static GList* template_dictionaries = 0;

/*!
 * \brief Retrieves a dictionary by name, or creates it if it doesn't exist.
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
 * \brief frees the memory used by a TypedValue
 * \param val the TypedValue to be liberated
 */
static void free_typed_value(TypedValue* val);

/*!
 * \brief Traverses the template tree and sets the dictionary that should be used at each field.
 * \param parent  The parent field. It should have a dictionary set.
 * \param parent_node  The parent's node in the tree
 */
void set_dictionary_pointers(const FieldType* parent, GNode* parent_node);

void set_dictionaries(GNode* template_tree){
  GNode* template = 0;
  
  clear_dictionaries();
  if(!dictionaries_table) {
    dictionaries_table =
      g_hash_table_new_full(&g_str_hash, &g_str_equal, &g_free,
                            (GDestroyNotify) &g_hash_table_destroy);
  }
  /* If we fail to make the table bail */
  if(!dictionaries_table) { BAILOUT(;,"Dictionary lookup table not created."); }
  template = g_node_first_child(template_tree);
  /* Create the global dictionary */
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
    get_dictionary(TEMPLATE_DICTIONARY);
    field_type->dictionary_ptr = get_dictionary(field_type->dictionary);
    set_dictionary_pointers(field_type, template);
    template_dictionaries =
      g_list_prepend(template_dictionaries,
                     get_dictionary(TEMPLATE_DICTIONARY));
    g_hash_table_steal(dictionaries_table, TEMPLATE_DICTIONARY);
    template = g_node_next_sibling(template);
  }

}

GHashTable* get_dictionary(char* name){
  GHashTable* dictionary = 0;
  dictionary = g_hash_table_lookup(dictionaries_table, name);
  if(!dictionary){
    dictionary = g_hash_table_new_full(&g_str_hash, &g_str_equal, &g_free,
                                       (GDestroyNotify)&free_typed_value);
    g_hash_table_insert(dictionaries_table, name, dictionary);
  }
  return dictionary;
}

void clear_dictionaries() 
{
  GList* list;
  list = g_hash_table_get_keys(dictionaries_table);
  
  while(list) {
    GHashTable* dictionary = g_hash_table_lookup(dictionaries_table,
                                                 ((char*)list->data));
    g_hash_table_remove_all(dictionary);
    list = g_list_next(list);
  }
  
  g_list_free(list);  
  list = template_dictionaries;
  
  while(list) {
    GHashTable* dictionary = list->data;
    g_hash_table_remove_all(dictionary);
    list = g_list_next(list);
  }
  
}

static void free_typed_value(TypedValue* val)
{
  if (!val->empty) {
    cleanup_field_value(val->type, &val->value);
  }
  g_free(val);
}

void remove_dictionary(char* name){
  if(!g_hash_table_remove(dictionaries_table, name)){
    DBG1("Attempt to remove non-existent dictionary: %s", name);
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
      fdata.status = FieldExists;
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
  if (!ftype->key) {
    BAILOUT(FALSE, "No key on field.");
  }
  prev = g_hash_table_lookup(dictionary,ftype->key);
  if (prev) {
    if (prev->type == ftype->type) {
      found = TRUE;
      fdata->status = prev->empty ? FieldEmpty : FieldExists;
      if (fdata->status == FieldExists) {
        copy_field_value(ftype->type, &prev->value, &fdata->value);
      }
    }
    else {
      /* An the types differ which is a dynamic error
       * The cast is to change the gchar* to guint8* so that our types will work with it
       */
      err_d(4, fdata);
    }
  }
  else {
    fdata->status = FieldUndefined;
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
  
  if(!ftype->key) {
    return;
  }

  prev_value = g_hash_table_lookup(dictionary, ftype->key);
  
  if (prev_value) {
    if (!prev_value->empty) {
      cleanup_field_value(ftype->type, &prev_value->value);
    }
    new_value = prev_value;
  }
  else {
    new_value = g_malloc(sizeof(TypedValue));
  }
  new_value->type = ftype->type;
  new_value->empty = fdata->status == FieldEmpty;
  if (!new_value->empty) {
    copy_field_value(ftype->type, &fdata->value, &new_value->value);
  }
  if (new_value) {
    /* Only have to insert if we created a new value. */
    if (!prev_value) {
      g_hash_table_insert(dictionary, g_strdup(ftype->key), new_value);
    }
  } else {
    DBG1("Failed to set value for field type: %s", ftype->name);
  }
}

