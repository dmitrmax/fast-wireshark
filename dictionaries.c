/*!
 * \file dictionaries.c
 * \brief  Handle dictionary setting on field types in templates
 */

#include "dictionaries.h"
#include "dissect.h"
#include "debug.h"
#include "template.h"

/* Private (static) headers. */
static GHashTable* dictionaries_table = 0;

/* TODO: Find this function a home */
/*!
 * \brief Copies a field data, setting the start byte and num bytes to 0 and cloning the value
 * The pointer returned by this is a newly allocated data
 * the memory is the caller's
 * \param field_type The Type of the field to be copied
 * \param field_data The FieldData to be copied
 * \return A copy of field_data with num bytes and start byte set to 0
 */
static FieldData* copy( FieldData* field_data, FieldType* field_type);
static gpointer _copy_convert(gconstpointer, gpointer);

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
 * \param node The current node in the tree
 * \param current_dictionary The most specific dictionary at this point
 */
static void set_dictionary_pointers(GNode* node);

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
    get_dictionary(TEMPLATE_DICTIONARY);
    set_dictionary_pointers(template);
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

void set_dictionary_pointers(GNode* node){
  GNode* child = 0;
  FieldType* field_type = 0;
  field_type = (FieldType*)(node->data);
  /* TODO: Debug code in the tiernary operator: remove everything after and including '?' */
  field_type->dictionary_ptr = get_dictionary(field_type->dictionary?field_type->dictionary:GLOBAL_DICTIONARY);
  child = g_node_first_child(node);
  /* Recurse into child nodes */
  while(child){
    set_dictionary_pointers(child);
    child = g_node_next_sibling(child);
  }
}

GNode* get_dictionary_value(FieldType* field_type){
  GHashTable* dictionary = 0;
  GNode* prev_value = 0;
  dictionary = (GHashTable*)field_type->dictionary_ptr;
  prev_value = (GNode*)g_hash_table_lookup(dictionary,field_type->name);
  return g_node_copy_deep(prev_value, &_copy_convert, field_type);
}

void set_dictionary_value(FieldType* field_type, GNode* value){
  GHashTable* dictionary = 0;
  GNode* prev_value = 0;
  GNode* new_value = 0;
  dictionary = (GHashTable*)field_type->dictionary_ptr;
  prev_value = (GNode*)g_hash_table_lookup(dictionary,field_type->name);
  if(prev_value){
    /* 
     * This function will recurse the tree and free all the nodes,
     * Unsure on if it frees user defined data as well
     */
    g_node_destroy(prev_value);
  }
  new_value = g_node_copy_deep(value, &_copy_convert, field_type);
  if(new_value){
    g_hash_table_insert(dictionary,field_type->name,new_value);
  } else {
    DBG1("Failed to set value for field type: %s", field_type->name);
  }
}

FieldData* copy(FieldData* field_data, FieldType* field_type){
  FieldData* copy = 0;
  copy = g_malloc (sizeof (FieldData));
  copy->start = 0;
  copy->nbytes = 0;
  copy->value = 0;
  switch(field_type->type){
    case FieldTypeUInt32:
      copy->value = g_malloc (sizeof (guint32));
      *((guint32*)copy->value) = *((guint32*)field_data->value);
    break;
    case FieldTypeUInt64:
    case FieldTypeInt32:
    case FieldTypeInt64:
    case FieldTypeDecimal:
    case FieldTypeAsciiString:
    case FieldTypeUnicodeString:
    case FieldTypeByteVector:
    default:
      DBG1("Copy not supported for %i",field_type->type);
    break;
	}
	return copy;
}

gpointer _copy_convert(gconstpointer data, gpointer additional){
	return copy((FieldData*)data, additional);
}