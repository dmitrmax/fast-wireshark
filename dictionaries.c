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
 * \file dictionaries.c
 * \brief  Handle dictionary setting on field types in templates
 */
#include <glib.h>
#include <epan/address.h>
#include <string.h>
#include "debug.h"
#include "dictionaries.h"

  /*
   * This is the general structure of how this works
   * the double map after the dest table is done using
   * the ConversationTables struct, defined above
   * map<src,
   *   map<dest,
   *     map<dictionary_name,
   *       map<field_key,value>
   *     map<tempate_id (should be unique),
   *       map<field_key,value>
   */

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

/*!
 * \brief Struct to track all dictionaries
 * Tracks all dictionaries, normal and template.  Stores the dictionaries in a
 * hash table for fast insert/lookup.
 */
struct _conversation_tables {
  GHashTable* normal;
  GHashTable* templates;
};
typedef struct _conversation_tables ConversationTables;

/* Private (static) headers. */
static GHashTable* src_table = 0;

/*!
 * \brief Retrieves a dictionary by name, or creates it if it doesn't exist.
 * \param name The name of the dictionary to retrieve
 */
static GHashTable* get_dictionary(const FieldType * ftype, address src, address dest);

/*!
 * \brief Duplicates an integer into the heap
 * \param i The integer to the duped into the heap
 */
static guint32* int_dup(guint32 i);

/*!
 * \brief frees the memory used by a TypedValue
 * \param val the TypedValue to be liberated
 */
static void free_typed_value(TypedValue* val);

/*!
 * \brief Frees a conversation table including its children
 * Frees the conversation table pointed to by p.  First calls
 * g_hash_table_destroy on the normal and template dictionaries
 * \param p Pointer to the conversation table to free
 */
static void free_conversation_table(gconstpointer p);

/*!
 * \brief Retrieves the conversation table for the given conversation
 * Retrieves the conversation table for the given conversation.  A
 * convenience method so this functionality could be isolated.
 * \param src The source participant of the conversation
 * \param dest The destination participant of the conversation
 * \return The conversation table associated with the pair
 */
static ConversationTables * get_conversation_table(address src, address dest);

/*!
 * \brief Frees the typed value given
 * Frees the typed value provided by first cleaning up the data is stores
 * then deleting the typed value
 * \param val Pointer to the typed value to be freed
 */
static void free_typed_value(TypedValue* val);

GHashTable* get_dictionary(const FieldType * ftype, address src, address dest){
  ConversationTables* conversation_tables = NULL;
  GHashTable* dictionary = NULL;

  conversation_tables = get_conversation_table(src, dest);

  /* Determine if we need a template or a non-template dictionary */
  if(g_strcmp0(ftype->dictionary,TEMPLATE_DICTIONARY) == 0){
    dictionary = (GHashTable*)g_hash_table_lookup(conversation_tables->templates, &(ftype->tid));
    /* If the dictionary does not exist create it */
    if(!dictionary){
      dictionary = g_hash_table_new_full(&g_str_hash,&g_str_equal,
				       (GDestroyNotify)&g_free,
				       (GDestroyNotify)&free_typed_value);
      g_hash_table_insert(conversation_tables->templates, int_dup(ftype->tid), dictionary);
    }
  } else {
    dictionary = (GHashTable*)g_hash_table_lookup(conversation_tables->normal, ftype->dictionary);
    /* If the dictionary does not exist create it */
    if(!dictionary){
      dictionary = g_hash_table_new_full(&g_str_hash,&g_str_equal,
				       (GDestroyNotify)&g_free,
				       (GDestroyNotify)&free_typed_value);
      g_hash_table_insert(conversation_tables->normal, ftype->dictionary, dictionary);
    }
  }
  /* Should never be run */
  if(!dictionary){BAILOUT(NULL,"Unable to retrieve dictionary");}

  return dictionary;
}

guint32* int_dup(guint32 i){
  guint32* ret = NULL;
  ret = (guint32*)g_malloc(sizeof(guint32));
  *ret = i;
  return ret;
}

ConversationTables * get_conversation_table(address src, address dest){
  GHashTable* dest_table = NULL;
  ConversationTables* conversation_tables = NULL;

  /* On first src_table won't exist */
  if(!src_table){
    src_table = g_hash_table_new_full(&addressHash, &addressEqual,
				      (GDestroyNotify)&addressDelete,
				      (GDestroyNotify)&g_hash_table_destroy);
  }
  /* Get the table for this destination, or create if it doesn't exist */
  dest_table = (GHashTable*)g_hash_table_lookup(src_table, &src);
  if(!dest_table){
    dest_table = g_hash_table_new_full(&addressHash, &addressEqual,
				       (GDestroyNotify)&addressDelete,
				       (GDestroyNotify)&free_conversation_table);
    g_hash_table_insert(src_table, copyAddress(&src), dest_table);
  }
  /* Retrieve the conversation tables, or create if not exist */
  conversation_tables = (ConversationTables*)g_hash_table_lookup(dest_table, &dest);
  if(!conversation_tables){
    conversation_tables = (ConversationTables*)g_malloc(sizeof(ConversationTables));
    conversation_tables->normal = g_hash_table_new_full(&g_str_hash, &g_str_equal,
				       &g_free,(GDestroyNotify)&g_hash_table_destroy);
    conversation_tables->templates = g_hash_table_new_full(&g_int_hash, &g_int_equal,
				       &g_free,(GDestroyNotify)&g_hash_table_destroy);
    g_hash_table_insert(dest_table, copyAddress(&dest), conversation_tables);
  }

  return conversation_tables;
}

void free_conversation_table(gconstpointer p){
  ConversationTables* conversation_tables = (ConversationTables*)p;

  if(conversation_tables->normal){
    g_hash_table_destroy(conversation_tables->normal);
  }
  if(conversation_tables->templates){
    g_hash_table_destroy(conversation_tables->templates);
  }
  g_free(conversation_tables);

}

void clear_dictionaries(address src, address dest)
{
  GList* list;
  GList* node;
  ConversationTables * ctables = get_conversation_table(src, dest);

  /* Free all the key/value pairs in all the dictionaries, but keep the
   * dictionaries as they will probably be used again later */
  list = g_hash_table_get_keys(ctables->normal);
  node = list;
  while(node) {
    GHashTable* dictionary = (GHashTable*)g_hash_table_lookup(ctables->normal,
                                                 ((char*)node->data));
    g_hash_table_remove_all(dictionary);
    node = g_list_next(node);
  }
  g_list_free(list);

  /* Same as above, free the value, not the dictionaries as the dictionaries
   * will probably be used again */
  list = g_hash_table_get_keys(ctables->templates);
  node = list;
  while(node) {
    GHashTable* dictionary = (GHashTable*)g_hash_table_lookup(ctables->templates,
                                                 ((char*)node->data));
    g_hash_table_remove_all(dictionary);
    node = g_list_next(node);
  }
  g_list_free(list);

  return;
}

void free_typed_value(TypedValue* val)
{
  if (!val->empty) {
    cleanup_field_value(val->type, &val->value);
  }
  g_free(val);
}


gboolean get_dictionary_value(const FieldType* ftype,
                              FieldData* fdata, address src, address dest)
{
  gboolean found = FALSE;
  GHashTable* dictionary = 0;
  const TypedValue* prev = 0;

  dictionary = get_dictionary(ftype, src, dest);
  if (!ftype->key) {
    BAILOUT(FALSE, "No key on field.");
  }
  prev = (TypedValue*)g_hash_table_lookup(dictionary,ftype->key);
  if (prev) {
    /* Determine if the types match */
    if (prev->type == ftype->type) {
      /* Copy the previous value for use */
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
  else if(ftype->hasDefault){
    /* TODO FIX THIS remove the || TRUE once the other todo has been handled*/
    if(ftype->mandatory || TRUE){
      fdata->status = FieldExists;
      copy_field_value(ftype->type, &ftype->value, &fdata->value);
    }
    else {
      /* TODO Determine if the default value is empty
       * if so set status accordingly and return the value
       * otherwise do the same as above but handle for optional (-1 for ints and such)
       */

    }
  }
  else {
    fdata->status = FieldUndefined;
  }

  return found;
}

void set_dictionary_value(const FieldType* ftype,
                          const FieldData* fdata, address src, address dest)
{
  GHashTable* dictionary = 0;
  TypedValue* prev_value = 0;
  TypedValue* new_value = 0;

  dictionary = get_dictionary(ftype, src, dest);

  if(!ftype->key) {
    return;
  }

  prev_value = (TypedValue*)g_hash_table_lookup(dictionary, ftype->key);
  /* Recycle the previous value */
  if (prev_value) {
    if (!prev_value->empty) {
      cleanup_field_value(ftype->type, &prev_value->value);
    }
    new_value = prev_value;
  }
  else {
    new_value = (TypedValue*)g_malloc(sizeof(TypedValue));
  }
  /* Copy in the values */
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
