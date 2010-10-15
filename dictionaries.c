/*!
 * \file dictionaries.c
 * \brief  Handle dictionary setting on field types in templates
 */

#include "dictionaries.h"
#include "debug.h"

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
static GHashTable* remove_dictionary(char* name);

/*!
 * \brief Traverses the template tree and sets the dictionary that should be used at each field.
 * \param node The current node in the tree
 * \param current_dictionary The most specific dictionary at this point
 */
static void set_dictionary_pointers(GNode* node, GHashTable* current_dictionary);

void set_dictionaries(GNode* template_tree){
  GNode* template = 0;
  if(!dictionaries_table) {
    dictionaries_table = g_hash_table_new(&g_str_hash, NULL);
  }
  /* If we fail to make the table bail */
  if(!dictionaries_table) { BAILOUT(;,"Dictionary lookup table not created."); }
  template = g_node_first_child(template_tree);
  /* 
   * Loop through all the templates, add and remove template dictionaries from the dictionary
   * reference for each template_tree
   */
  while(template){
    get_dictionary(TEMPLATE_DICTIONARY);
    set_dictionary_pointers(template, get_dictionary(GLOBAL_DICTIONARY);
    remove_dictionary(TEMPLATE_DICTIONARY);
  }
  

}

GHashTable* get_dictionary(char* name){
  GHashTable* dictionary = 0;
  dictionary = g_hash_table_lookup(dictionaries_table, name);
  if(!dictionary){
    dictionary = g_hash_table_new(&g_str_hash, NULL);
    g_hash_table_insert(dictionaries_table, name, dictionary);
  }
  return dictionary;
}

void remove_dictionary(char* name){
  if(!g_hash_table_remove(dictionaries_table, name)){
    DEBUG1("Attempt to remove non-existant dictionary: %s", name);
  }
}

void set_dictionary_pointers(GNode* node, GHashTable* current_dictionary){
  GNode* child = 0;
  FieldType* field_type = 0;
  field_type = (FieldType*)(node->data);
  /* If this field has an explicit dictionary use it */
  if(field_type->dictionary){
    current_dictionary = get_dictionary(field_type->dictionary);
  }
  fild_type->dictionary_ptr = current_dictionary;
  child = g_node_first_child(node);
  /* Recurse into child nodes */
  while(child){
    set_dictionary_pointers(child, current_dictionary);
    child = g_node_next_sibling(child);
  }
}