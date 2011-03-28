/*!
 * \file template.c
 * \brief  Handle template storage/lookups.
 */

#include "debug.h"
#include "decode.h"

#include "template.h"

static GHashTable* template_table = 0;
static GNode* template_tree = 0;

static gboolean requires_pmap_bit (const FieldType* ftype);
static void fixup_walk_template (FieldType* parent, GNode* parent_node);

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
    FieldType* tfield;
    tfield = (FieldType*) tmpl->data;
    tfield->value.pmap_exists = TRUE;
    fixup_walk_template (tfield, tmpl);
    g_hash_table_insert (template_table, &tfield->id, tmpl);
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

  field->name       = 0;
  field->id         = 0;
  field->key        = 0;
  field->mandatory  = TRUE;
  field->type       = type;
  field->op         = op;
  field->hasDefault = FALSE;
  init_field_value(&field->value);
  field->dictionary = 0;

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

/*! \brief  Check if a field type needs a bit in the PMAP. */
gboolean requires_pmap_bit (const FieldType* ftype)
{
  if (ftype->type == FieldTypeGroup) {
    return !ftype->mandatory;
  }
  switch (ftype->op) {
    case FieldOperatorConstant:
      return !ftype->mandatory;
    case FieldOperatorDefault:
    case FieldOperatorCopy:
    case FieldOperatorIncrement:
    case FieldOperatorTail:
      return TRUE;
    default:
      return FALSE;
  }
}

/*! \brief  Propagate data down and up the type tree.
 *
 * \param parent  The parent group. Not necessarily contained in parent_node.
 * \param parent_node  The node on whose children this function will operate.
 */
void fixup_walk_template (FieldType* parent, GNode* parent_node)
{
  GNode* tnode;
  for (tnode = parent_node->children;  tnode;  tnode = tnode->next) {
    FieldType* ftype;
    ftype = (FieldType*) tnode->data;
    if (!ftype) {
      DBG0("Null field type.");
      continue;
    }
    if (!parent->value.pmap_exists) {
      if (requires_pmap_bit (ftype)) {
        parent->value.pmap_exists = TRUE;
      }
    }
    /* Only have this field as a parent to recursion if it is a group
     * as only then will it be able to contain a PMAP.
     */
    if (FieldTypeGroup == ftype->type) {
      fixup_walk_template (ftype, tnode);
    }
    else {
      fixup_walk_template (parent, tnode);
    }
  }
}

/*! \brief  Get the entire templates tree.
 */
GNode* full_templates_tree ()
{
  return template_tree;
}

