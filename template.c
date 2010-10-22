/*!
 * \file template.c
 * \brief  Handle template storage/lookups.
 */

#include "debug.h"
#include "decode.h"

#include "template.h"

static GHashTable* template_table = 0;
static GNode* template_tree = 0;

/*! \brief  Add a new template to the lookup table.
 *
 * \param templates  The root of the templates tree.
 */
void add_templates (GNode* templates)
{
  GNode* tmpl;
  template_tree = templates;

  /* TODO: Clear hash table and free old templates. */

  if (!template_table) {
    template_table = g_hash_table_new (&g_int_hash, &g_int_equal);
  }

  if (!template_table)  BAILOUT(;,"Template lookup table not created.");

  /* Loop thru templates, add each to lookup table. */
  for (tmpl = templates->children;  tmpl;  tmpl = tmpl->next) {
    FieldType* field;
    field = (FieldType*) tmpl->data;
    g_hash_table_insert (template_table, &field->id, tmpl);
  }
}


/*!
 * \brief  Retrieve the name of the field type.
 * \param type  Field type for the name lookup.
 * \return  A string corresponding to the type given.
 *          If the type is invalid, return an empty string.
 */
const gchar* field_typename (FieldTypeIdentifier type)
{
  static const gchar* names[] =
  {
    "uInt32", "uInt64", "int32", "int64",
    "decimal", "ascii", "unicode", "byteVector",
    "group", "sequence"
  };
  if (0 <= type && type < FieldTypeEnumLimit) {
    return names[type];
  }
  else {
    DBG1("Unknown type %d", type);
    return "";
  }
}

/*!
 * \brief  Retrieve the name of the operator type.
 * \param type  Operator type for the name lookup.
 * \return  A string corresponding to the type given.
 *          If the type is invalid, return an empty string.
 */
const gchar* operator_typename (FieldOperatorIdentifier type)
{
  static const gchar* names[] =
  {
    "no_operator", "constant", "default", "copy",
    "increment", "delta", "tail"
  };
  if(0<= type && type < FieldOperatorEnumLimit) {
    return names[type];
  }
  else {
    DBG1("Unknown type %d", type);
    return "";
  }
}


/*!
 * \brief  Create internal representation for a field wrapped in a GNode.
 *
 * \return  The new, initialized node holding a non-null FieldType.
 *          Both the GNode and the FieldType must be g_free()'d.
 *          NULL if a malloc failed.
 */
GNode* create_field (FieldTypeIdentifier type,
                     FieldOperatorIdentifier op)
{
  FieldType* field;
  GNode* node;
  field = g_malloc (sizeof (FieldType));
  if (!field)  BAILOUT(0, "Error g_malloc().");

  node = g_node_new (field);
  if (!node) {
    g_free (field);
    BAILOUT(0, "Error g_new_node().");
  }

  field->name      = 0;
  field->id        = 0;
  field->key       = 0;
  field->mandatory = TRUE;
  field->type      = type;
  field->op        = op;
  field->value     = 0;
  field->dictionary = 0;
  field->dictionary_ptr = 0;

  return node;
}


/*! \brief  Lookup a template by its ID.
 *
 * \return  NULL if no template could be found.
 */
GNode* find_template (guint32 id)
{
  gint key;
  key = (gint) id;
  return (GNode*) g_hash_table_lookup (template_table, &key);
}

