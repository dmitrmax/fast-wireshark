/*!
 * \file setup.c
 * \brief  Create a simple template to use.
 */

#include "template.h"

/*! \brief  Create dummy templates.
 * \return  The new templates.
 */
GNode* FAST_setup ()
{
  GNode* templates; /* Return value. */
  /* Temporary storage for each field. */
  GNode* parent;
  GNode* tnode_prev;
  GNode* tnode;
  FieldType* tfield;

  templates = g_node_new (0);
  if (!templates)  BAILOUT(0, "Error creating root of templates tree.");

  /*** Create a template 1. ***/
  parent = templates;
  tnode_prev = 0;

  tnode = create_field (FieldTypeInt32, FieldOperatorCopy);
  if (!tnode)  BAILOUT(0, "Error creating a template tree.");
  tfield = (FieldType*) tnode->data;
  tfield->name = g_strdup ("TestTemplate1");
  tfield->id = 1;
  g_node_insert_after (parent, tnode_prev, tnode);

  /* Populate some fields. */
  parent = tnode;
  tnode_prev = 0;

  /* <uInt32 /> */
  tnode = create_field (FieldTypeUInt32, FieldOperatorNone);
  if (!tnode)  BAILOUT(0, "Error creating field.");
  g_node_insert_after (parent, tnode_prev, tnode);
  tnode_prev = tnode;

  /* <uInt64 /> */
  tnode = create_field (FieldTypeUInt64, FieldOperatorNone);
  if (!tnode)  BAILOUT(0, "Error creating field.");
  g_node_insert_after (parent, tnode_prev, tnode);
  tnode_prev = tnode;

  /* <int32 /> */
  tnode = create_field (FieldTypeInt32, FieldOperatorNone);
  if (!tnode)  BAILOUT(0, "Error creating field.");
  g_node_insert_after (parent, tnode_prev, tnode);
  tnode_prev = tnode;

  /* <int64 /> */
  tnode = create_field (FieldTypeInt64, FieldOperatorNone);
  if (!tnode)  BAILOUT(0, "Error creating field.");
  g_node_insert_after (parent, tnode_prev, tnode);
  tnode_prev = tnode;

  /* DBG0("Template setup complete."); */
  return templates;
}

