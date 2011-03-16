#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <gtk/gtk.h>

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include "debug.h"
#include "template.h"

#include "parse-template.h"

/* Private (static) headers. */
static GNode* parseTemplate (xmlNodePtr cur);
static void set_field_attributes (xmlNodePtr node, FieldType* tfield);
static gboolean ignore_xml_node (xmlNodePtr cur);
static void parser_walk_children (xmlNodePtr cur,
                                  GNode* parent, GNode* tnode_prev);
static GNode* new_parsed_field (xmlNodePtr xmlnode);
static gboolean field_type_match (xmlNodePtr node,
                                  FieldTypeIdentifier type);
static gboolean parse_field_operator (xmlNodePtr xmlnode, FieldType * tfield);
static gboolean parse_operator (xmlNodePtr xmlnode, FieldType * tfield);
static gboolean prepend_length (xmlNodePtr xmlnode, const FieldType* parent,
                                GNode* parent_node);
static gboolean parse_decimal (xmlNodePtr xmlnode, FieldType * tfield, GNode * tnode);
static gboolean parse_sequence (xmlNodePtr xmlnode, FieldType* tfield, GNode* tnode);
static gboolean operator_type_match (xmlNodePtr node, FieldOperatorIdentifier type);

static gint templateID = -1; /* Stores tid while parsing */

/* Function to open a dialog box displaying the message provided. */
void quick_message (gchar *message)
{
   GtkWidget *dialog, *label, *content_area;

   /* Create the widgets */
   dialog = gtk_dialog_new_with_buttons ("Message",
                                         NULL,
                                         GTK_DIALOG_DESTROY_WITH_PARENT,
                                         GTK_STOCK_OK,
                                         GTK_RESPONSE_NONE,
                                         NULL);
   content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
   label = gtk_label_new (message);

   /* Ensure that the dialog box is destroyed when the user responds */
   g_signal_connect_swapped (dialog,
                             "response",
                             G_CALLBACK (gtk_widget_destroy),
                             dialog);

   /* Add the label, and show everything we've added to the dialog */

   gtk_container_add (GTK_CONTAINER (content_area), label);
   gtk_widget_show_all (dialog);
}

/*! \brief  Convert an XML file into an internal representation of
 *          the templates.
 * \param filename  Name of the XML file to parse.
 * \return  An internal tree of FieldTypes.
 */
GNode* parse_templates_xml(const char* filename)
{
	xmlDocPtr doc; /* pointer to XML document */
	xmlNodePtr cur; /* pointer to current node within document */
  GNode* templates; /* Return value, all templates stored as children here. */
  GNode* tnode_prev = 0;

	doc = xmlParseFile(filename); /* attempt to parse xml file and get pointer to parsed document */
	
	if (doc == NULL) {
		quick_message("Document not parsed successfully.");
    fprintf(stderr,"Document not parsed successfully. \n");
		return 0;
	}

  /* Start at the root of the XML document. */
	cur = xmlDocGetRootElement(doc);
	
	if (cur == NULL) {
		fprintf(stderr,"empty document\n");
		xmlFreeDoc(doc);
		return 0;
	}

  templates = g_node_new (0);
  if (!templates)  BAILOUT(0, "Error creating root of templates tree.");
	
	/* Check if root is of type "templates". */
	if (xmlStrcmp(cur->name, (xmlChar*) "templates")) {
		fprintf(stderr,"document of the wrong type, root node != templates\n");
		xmlFreeDoc(doc);
		return 0;
	}
	
  cur = cur->xmlChildrenNode;
  while (cur != NULL) {

    if (!ignore_xml_node(cur)) {
      if ((!xmlStrcasecmp(cur->name, (xmlChar*) "template"))) {
        GNode* tnode;
        tnode = parseTemplate(cur);
        /* Parse template, and check if parse failed. */
        if (tnode) {
          g_node_insert_after (templates, tnode_prev, tnode);
          tnode_prev = tnode;
        }
        else {
          DBG0("Parsing a template failed.");
          xmlFreeDoc(doc);
          return 0;
        }
      }
      else {
        fprintf(stderr,"Warning: Unkown templates child: %s\n", cur->name);
        fprintf(stderr,"Continuing to parse templates file ...\n");
      }
    }
    cur = cur->next;
  }

  xmlFreeDoc(doc);
  return templates;
}


/*! \brief Parse a template section of the XML document
 * \param cur  Current position in the XML structure.
 * \return  Tree containing the internal template definition.
 */
GNode* parseTemplate (xmlNodePtr cur)
{
  GNode* tnode;
  FieldType* tfield;

  tnode = create_field(FieldTypeUInt32, FieldOperatorCopy);
  if (!tnode)  BAILOUT(0, "Error allocating memory.");

  tfield = (FieldType*) tnode->data;

  /* Get name, id, etc. */
  set_field_attributes(cur, tfield);
  
  /* Store templateID */
  tfield->tid = tfield->id;
  templateID = tfield->tid;

  cur = cur->xmlChildrenNode;
  parser_walk_children (cur, tnode, 0);

  return tnode;
}



/*! \brief  Build up the parse tree from XML data,
 *          starting at an arbitrary node.
 *
 * \param cur  Current node in an xml tree.
 * \param parent  This parse tree level's parent.
 * \param tnode_prev  Previous node in parse tree at this level.
 */
void parser_walk_children (xmlNodePtr cur,
                           GNode* parent, GNode* tnode_prev)
{
  while (cur != NULL) {
    if (!ignore_xml_node(cur)) {
      GNode* tnode;
      tnode = new_parsed_field (cur);
      if (tnode) {
        /* DBG0("adding tnode"); */
        g_node_insert_after (parent, tnode_prev, tnode);
        tnode_prev = tnode;
      }
    }
    cur = cur->next;
  }
}


/*! \brief  Create a new field in the parse tree based on an XML node.
 * \param  The XML node which /should/ be a field.
 * \return  The new GNode* containing a FieldType.
 *          NULL if something went wrong.
 */
GNode* new_parsed_field (xmlNodePtr xmlnode)
{
  gboolean found = FALSE;  /* Field type found. */
  gboolean valid = FALSE;  /* Field type valid. */
  GNode* tnode;
  FieldType* tfield;

  tnode = create_field (FieldTypeInvalid, FieldOperatorNone);
  if (!tnode) {
    return 0;
  }
  tfield = (FieldType*) tnode->data;

  set_field_attributes(xmlnode, tfield);

  /* Try the integers. */
  if (!found) {
    found = TRUE;
    if (field_type_match (xmlnode, FieldTypeUInt32)) {
      tfield->type = FieldTypeUInt32;
    }
    else if (field_type_match (xmlnode, FieldTypeUInt64)) {
      tfield->type = FieldTypeUInt64;
    }
    else if (field_type_match (xmlnode, FieldTypeInt32)) {
      tfield->type = FieldTypeInt32;
    }
    else if (field_type_match (xmlnode, FieldTypeInt64)) {
      tfield->type = FieldTypeInt64;
    }
    else {
      found = FALSE;
    }

    if (found) {
      /* check if field has valid operators */
      valid = parse_field_operator(xmlnode, tfield);
    }
  }

  /* Try decimal. */
  if (!found && field_type_match (xmlnode, FieldTypeDecimal)) {
    
    found = TRUE;
    valid = parse_decimal(xmlnode, tfield, tnode);
  }

  /* Try string */
  if(!found && field_type_match (xmlnode, FieldTypeAsciiString)){
    tfield->type = FieldTypeAsciiString;
    found = TRUE;
    valid = parse_field_operator(xmlnode, tfield);
  } else if(!found && field_type_match (xmlnode, FieldTypeUnicodeString)){
    tfield->type = FieldTypeUnicodeString;
    found = TRUE;
    valid = parse_field_operator(xmlnode, tfield);
    if (valid) {
      valid = prepend_length(xmlnode, tfield, tnode);
    }
  }

  /* Try bytevector */
  if(!found && field_type_match (xmlnode, FieldTypeByteVector)){
    tfield->type = FieldTypeByteVector;
    found = TRUE;
    valid = parse_field_operator(xmlnode, tfield);
    if (valid) {
      valid = prepend_length(xmlnode, tfield, tnode);
    }
  }


  /* Try group */
  if(!found && field_type_match(xmlnode, FieldTypeGroup)){
    tfield->type = FieldTypeGroup;
    found = TRUE;
    valid = TRUE;
    parser_walk_children (xmlnode->xmlChildrenNode, tnode, 0);
  }

  /* Try sequence */
  if(!found && field_type_match(xmlnode, FieldTypeSequence)){
    found = TRUE;
    valid = parse_sequence (xmlnode, tfield, tnode);
    if (valid) {
      valid = prepend_length(xmlnode, tfield, tnode);
    }
  }

  /* If we retrieved built up the field,
   * fill its attributes.
   */
  if (found && valid) {
  }
  else {
    /* TODO: Free parse tree. */
    tnode = 0;
    if (!valid) {
      DBG1("Field %s could not be parsed.", xmlnode->name);
    }
    else {
      DBG1("Unknown field type %s.", xmlnode->name);
    }
  }
  return tnode;
}


/*! \brief  Add a length field as a child.
 * \param xmlnode  The XML node whose children we will search for a length.
 * \param parent  Parent field type;
 * \param parent_node  Tree node to build a length child.
 * \return  TRUE iff the a child was added.
 */
gboolean prepend_length (xmlNodePtr xmlnode, const FieldType* parent,
                         GNode* parent_node)
{
  GNode* tnode;
  FieldType* ftype;

  tnode = create_field (FieldTypeUInt32, FieldOperatorNone);
  if (!tnode)  BAILOUT(0, "Error creating exponent field.");
  g_node_insert_before (parent_node, parent_node->children, tnode);
  ftype = (FieldType*) tnode->data;

  for (xmlnode = xmlnode->xmlChildrenNode;
       xmlnode;
       xmlnode = xmlnode->next) {
    if (0 == xmlStrcasecmp(xmlnode->name, (xmlChar*)"length")) {
      set_field_attributes(xmlnode, ftype);
      if (!parse_field_operator(xmlnode, ftype)) {
        BAILOUT(FALSE, "Failed to get length.");
      }
    }
  }
  ftype->mandatory = parent->mandatory;
  return TRUE;
}


/*! \brief  Fill in a decimal field in the parse tree.
 * \param  The XML node which /should/ be a decimal field.
 * \param  A pointer to the template within the parse tree.
 * \return  True if sucessfully parsed
 */
gboolean parse_decimal (xmlNodePtr xmlnode, FieldType * tfield, GNode * tnode)
{
  GNode* exptNode;
  GNode* mantNode;
  FieldType * expt;
  FieldType * mant;

  tfield->type = FieldTypeDecimal;

  /* Add exponent and mantissa. */
  exptNode = create_field (FieldTypeInt32, FieldOperatorNone);
  if (!exptNode)  BAILOUT(0, "Error creating exponent field.");
  g_node_insert_after (tnode, 0,        exptNode);
  expt = (FieldType*) exptNode->data;

  mantNode = create_field (FieldTypeInt64, FieldOperatorNone);
  if (!mantNode)  BAILOUT(0, "Error creating mantissa field.");
  g_node_insert_after (tnode, exptNode, mantNode);
  mant = (FieldType*) mantNode->data;

  /* Get exponent and mantissa operators and values (if given) */
  xmlnode = xmlnode->xmlChildrenNode;
  while (xmlnode != NULL) {
    if (!ignore_xml_node(xmlnode)){
      if(0 == xmlStrcasecmp(xmlnode->name, (xmlChar*)"exponent")){
        set_field_attributes(xmlnode, expt);
        if(!parse_field_operator(xmlnode, expt)) {
          BAILOUT(FALSE, "Failed to get exponent.");
        }
      } else if( 0 == xmlStrcasecmp(xmlnode->name, (xmlChar*)"mantissa")){
        set_field_attributes(xmlnode, mant);
        if(!parse_field_operator(xmlnode, mant)) {
          BAILOUT(FALSE, "Failed to get mantissa.");
        }
      } else {
        /* assume this node is an operator */
        parse_operator (xmlnode, tfield);
      }
    }
    xmlnode = xmlnode->next;
  }

  expt->mandatory = tfield->mandatory;

  if (tfield->key) {
    if (!mant->key) {
      mant->key = g_strdup_printf ("%s-mantissa", tfield->key);
    }
    if (!expt->key) {
      expt->key = g_strdup_printf ("%s-exponent", tfield->key);
    }
  }

  return TRUE;
}


/*! \brief  Fill in a sequence field in the parse tree.
 * \param xmlnode  The XML node which /should/ be a sequence field.
 * \param tfield  Return value. An initialized field type.
 * \param tnode  A pointer to the template within the parse tree.
 * \return  True if sucessfully parsed
 */
gboolean parse_sequence (xmlNodePtr xmlnode, FieldType* tfield, GNode* tnode)
{
  GNode* group_tnode;

  tfield->type = FieldTypeSequence;

  group_tnode = create_field (FieldTypeGroup, FieldOperatorNone);
  if (!group_tnode) {
    return FALSE;
  }
  g_node_insert_after (tnode, 0, group_tnode);
  
  /* Descend the tree on the group. */
  parser_walk_children (xmlnode->xmlChildrenNode, group_tnode, 0);

  return TRUE;
}

/*! \brief  Fill in a field in the parse tree with operator info.
 * \param xmlnode  The XML node which /should/ be a field.
 * \param tfield  A pointer to the field within the parse tree.
 * \return  True if sucessfully parsed
 */
gboolean parse_field_operator(xmlNodePtr xmlnode, FieldType * tfield){
  
  const xmlChar *name;
  name = xmlnode->name;

  /* loop through field to find operators */
  xmlnode = xmlnode->xmlChildrenNode;
  while (xmlnode != NULL) {
    if (!ignore_xml_node(xmlnode)){
      
      return parse_operator(xmlnode, tfield);

    }
    xmlnode = xmlnode->next;
  }
  
  /* DBG0("no operator found");*/
  return TRUE;
}  
  


/*! \brief  Fill in a field in the parse tree with operator info.
 * \param xmlnode  The XML node which /should/ be a operator.
 * \param tfield  A pointer to the operator within the parse tree.
 * \return  True if sucessfully parsed
 */
gboolean parse_operator (xmlNodePtr xmlnode, FieldType * tfield){
  
  xmlChar *prop;
  const xmlChar *name;
  name = xmlnode->name;

  /* DBG2("parse_operator called on node %s  tfield name is %s", name, tfield->name);*/
  
  if (xmlnode==NULL){
    return FALSE;
  }

  if (operator_type_match (xmlnode, FieldOperatorConstant)) {
    tfield->op = FieldOperatorConstant;
  } else if (operator_type_match (xmlnode, FieldOperatorDefault)) {
    tfield->op = FieldOperatorDefault;
  } else if (operator_type_match (xmlnode, FieldOperatorCopy)) {
    tfield->op = FieldOperatorCopy;
  } else if (operator_type_match (xmlnode, FieldOperatorIncrement)) {
    tfield->op = FieldOperatorIncrement;
  } else if (operator_type_match (xmlnode, FieldOperatorDelta)) {
    tfield->op = FieldOperatorDelta;
  } else if (operator_type_match (xmlnode, FieldOperatorTail)) {
    tfield->op = FieldOperatorTail;
  } else {
    DBG2("Invalid operator (%s) for field %s", xmlnode->name, name);
    return FALSE;
  }
			
  /* get value of operator if given */
  prop = xmlGetProp(xmlnode, (xmlChar*)"value");
  if (prop!=NULL) {
    tfield->empty = FALSE;
    string_to_field_value((char*)prop, tfield->type, &tfield->value);
    /* DBG2("value is %s  tfield name is %s", (char *)prop, tfield->name); */
  } else {
    tfield->empty = TRUE;			
	}
		
  return TRUE;
}


/*! \brief  Set standard attributes of a field.
 *
 * Specifically, set 'id', 'name', and 'presence'.
 * \param node  XML node to query for attributes.
 * \param tfield  Return value. Must already be allocated.
 */
void set_field_attributes (xmlNodePtr xmlnode, FieldType* tfield)
{
  const xmlChar* str;
  /* Name. */
  str = xmlGetProp(xmlnode, (xmlChar*) "name");
  if (str) {
    tfield->name = g_strdup ((char*)str);
  }
  xmlFree((void*)str);
  /* set tid to parent template id */
  tfield->tid = templateID;
  /* Identifier number. */
  str = xmlGetProp(xmlnode, (xmlChar*) "id");
  if (str) {
    tfield->id = atoi((char*)str);
  }
  xmlFree((void*)str);
  /* Key. */
  str = xmlGetProp(xmlnode, (xmlChar*) "key");
  if (str) {
    tfield->key = g_strdup ((char*)str);
  } else if (tfield->name){
    tfield->key = g_strdup (tfield->name);
  }
  xmlFree((void*)str);
  /* Presence. */
  str = xmlGetProp(xmlnode, (xmlChar*) "presence");
  if (str) {
    if (0 == xmlStrcasecmp(str, (xmlChar*) "optional")) {
      tfield->mandatory = FALSE;
    }
    else if (0 == xmlStrcasecmp(str, (xmlChar*) "mandatory")) {
      tfield->mandatory = TRUE;
    }
    else {
      DBG1("Error, bad presence option '%s'.", (char*) str);
    }
  }
  xmlFree((void*)str);

  str = xmlGetProp(xmlnode, (xmlChar*) "dictionary");
  if (str) {
    tfield->dictionary = g_strdup((char*) str);
    xmlFree((void*)str);
  }
}


/*! \brief  Check if this XML node should be ignored.
 *
 * Ignored nodes include comments and text between XML tags.
 *
 * \param xmlnode  Current XML node.
 * \return  TRUE iff the node is something we ignore.
 */
gboolean ignore_xml_node (xmlNodePtr xmlnode)
{
  return (0 == xmlStrcasecmp(xmlnode->name, (xmlChar*) "text") ||
          0 == xmlStrcasecmp(xmlnode->name, (xmlChar*) "comment") ||
          0 == xmlStrcasecmp(xmlnode->name, (xmlChar*) "typeRef") ||
          0 == xmlStrcasecmp(xmlnode->name, (xmlChar*) "templateRef") ||
          0 == xmlStrcasecmp(xmlnode->name, (xmlChar*) "length"));
}

/*! \brief  Check if a string from XML.
 * \param node  Node in the XML tree. Its name is to be used in comparison.
 * \param type  Type whose name you want to check against.
 * \return  TRUE iff the node name matches.
 */
gboolean field_type_match (xmlNodePtr node, FieldTypeIdentifier type)
{
  const xmlChar* str1 = node->name;
  const char* str2 = field_typename (type);
  
  if (type == FieldTypeAsciiString) {
    if (0 == xmlStrcasecmp(str1, (xmlChar*) "string")) {
      /* check if charset is ascii or unicode */
      xmlChar * prop = xmlGetProp(node, (xmlChar*) "charset");
      if(prop==NULL){
        /* Assume ascii if not given */
        return TRUE;
      }
      if(0 == xmlStrcasecmp(prop, (xmlChar*) "ascii")){
        return TRUE;
      }
    }
    return FALSE;
  }
  else if (type == FieldTypeUnicodeString) {
    if (0 == xmlStrcasecmp(str1, (xmlChar*) "string")) {
      /* check if charset is ascii or unicode */
      xmlChar * prop = xmlGetProp(node, (xmlChar*)"charset");
      if(prop==NULL){
        /* Assume ascii if not given */
        return FALSE;
      }
      if(0 == xmlStrcasecmp(prop, (xmlChar*) "unicode")){
        return TRUE;
      }
    }
    return FALSE;
  }
  else {
    return (0 == xmlStrcasecmp(str1, (xmlChar*) str2));
  }
}

/*! \brief  Check if a string from XML matches the given Operator's type string
 * \param node  Node in the XML tree. Its name is to be used in comparison.
 * \param type  Type whose name you want to check against.
 * \return  TRUE iff the node name matches.
 */
gboolean operator_type_match (xmlNodePtr node, FieldOperatorIdentifier type)
{
  const xmlChar* str1 = node->name;
  const char* str2 = operator_typename(type);
  return (0 == xmlStrcasecmp(str1, (xmlChar*) str2));
}


