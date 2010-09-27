
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include "fast.h"
#include "parse-template.h"
#include "template.h"

/* Private (static) headers. */
static GNode* parseTemplate (xmlNodePtr cur);
static void set_field_attributes (xmlNodePtr node, FieldType* tfield);
static gboolean ignore_xml_node (xmlNodePtr cur);
static void parser_walk_children (xmlNodePtr cur,
                                  GNode* parent, GNode* tnode_prev);
static GNode* new_parsed_field (xmlNodePtr xmlnode);
static gboolean field_type_match (xmlNodePtr node,
                                  FieldTypeIdentifier type);
static gboolean parse_operator (xmlNodePtr xmlnode, FieldType * tfield);
static gboolean parse_decimal (xmlNodePtr xmlnode, FieldType * tfield, GNode * tnode);
static gboolean operator_type_match (xmlNodePtr node, FieldOperatorIdentifier type);

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
	if (xmlStrcmp(cur->name, (const xmlChar *) "templates")) {
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

  /* Assure we can allocate the needed structures. */
  tfield = g_malloc (sizeof (FieldType));
  if (!tfield) {
    DBG0("Error allocating memory.");
    return 0;
  }
  tnode = g_node_new (tfield);
  if (!tnode) {
    DBG0("Error allocating memory.");
    g_free (tfield);
    return 0;
  }

  /* Initialize the field. */
  tfield->name      = 0;
  tfield->id        = 0;
  tfield->mandatory = TRUE;
  tfield->op        = FieldOperatorNone;
  tfield->value     = 0;

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
      valid = parse_operator(xmlnode, tfield);
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
    valid = parse_operator(xmlnode, tfield);
  } else if(!found && field_type_match (xmlnode, FieldTypeUnicodeString)){
    tfield->type = FieldTypeUnicodeString;
    found = TRUE;
    valid = parse_operator(xmlnode, tfield);
  }

  /* Try bytevector */
  if(!found && field_type_match (xmlnode, FieldTypeByteVector)){
    tfield->type = FieldTypeByteVector;
    found = TRUE;
    valid = parse_operator(xmlnode, tfield);
  }


  /* If we retrieved built up the field,
   * fill its attributes.
   */
  if (found && valid) {
    set_field_attributes(xmlnode, tfield);
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


/*! \brief  Fill in a decimal field in the parse tree.
 * \param  The XML node which /should/ be a decimal field.
 * \param  A pointer to the template within the parse tree.
 * \return  True if sucessfully parsed
 */
gboolean parse_decimal (xmlNodePtr xmlnode, FieldType * tfield, GNode * tnode){

    GNode* exptNode;
    GNode* mantNode;
    FieldType * expt;
    FieldType * mant;

    tfield->type = FieldTypeDecimal;

    /* Add exponent and mantissa. */
    exptNode = create_field (FieldTypeInt32, FieldOperatorNone);
    if (!exptNode)  BAILOUT(0, "Error creating exponent field.");
    g_node_insert_after (tnode, 0,        exptNode);

    mantNode = create_field (FieldTypeInt64, FieldOperatorNone);
    if (!mantNode)  BAILOUT(0, "Error creating mantissa field.");
    g_node_insert_after (tnode, exptNode, mantNode);

    /* Get exponent and mantissa operators and values (if given) */
    xmlnode = xmlnode->xmlChildrenNode;
	  while (xmlnode != NULL) {
		  if (!ignore_xml_node(xmlnode)){
        if(0 == xmlStrcasecmp(xmlnode->name, (xmlChar*)"exponent")){
          expt = (FieldType*) exptNode->data;
          if(!parse_operator(xmlnode, expt)) return FALSE;
        } else if( 0 == xmlStrcasecmp(xmlnode->name, (xmlChar*)"mantissa")){
          mant = (FieldType*) mantNode->data;
          if(!parse_operator(xmlnode, mant)) return FALSE;
        } else {
           DBG1("Unknown decimal subfield %s.", xmlnode->name);
           return FALSE;
        }
      }  
    xmlnode = xmlnode->next;
	}

  return TRUE;
}

/*! \brief  Fill in a field in the parse tree with operator info.
 * \param  The XML node which /should/ be a field.
 * \param  A pointer to the field within the parse tree.
 * \return  True if sucessfully parsed
 */
gboolean parse_operator (xmlNodePtr xmlnode, FieldType * tfield){
  
  xmlChar *prop;
  const xmlChar *name;
  name = xmlnode->name;

	/* loop through field to find operators */
	xmlnode = xmlnode->xmlChildrenNode;
	while (xmlnode != NULL) {
		if (!ignore_xml_node(xmlnode)){

      if (operator_type_match (xmlnode, FieldOperatorConstant)) {
        tfield->op = FieldOperatorConstant;
      }
      else if (operator_type_match (xmlnode, FieldOperatorDefault)) {
        tfield->op = FieldOperatorDefault;
      }
      else if (operator_type_match (xmlnode, FieldOperatorCopy)) {
        tfield->op = FieldOperatorCopy;
      }
      else if (operator_type_match (xmlnode, FieldOperatorIncrement)) {
        tfield->op = FieldOperatorIncrement;
      }
      else if (operator_type_match (xmlnode, FieldOperatorDelta)) {
        tfield->op = FieldOperatorDelta;
      }
      else if (operator_type_match (xmlnode, FieldOperatorTail)) {
        tfield->op = FieldOperatorTail;
      } else {
        DBG2("Invalid operator (%s) for field %s", xmlnode->name, name);
        return FALSE;
      }
			
			/* get value of operator if given */
			prop = xmlGetProp(xmlnode, (const xmlChar*)"value");
			if(prop!=NULL){
				tfield->value = prop;
			} else {
				tfield->value = NULL;			
			}

		}
    xmlnode = xmlnode->next;
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
  /* Identifier number. */
  str = xmlGetProp(xmlnode, (xmlChar*) "id");
  if (str) {
    tfield->id = atoi((char*)str);
  }
  xmlFree((void*)str);
  /* Presence. */
  str = xmlGetProp(xmlnode, (xmlChar*) "presence");
  if (str) {
    if (xmlStrcasecmp(str, (xmlChar*) "optional")) {
      tfield->mandatory = FALSE;
    }
    else if (xmlStrcasecmp(str, (xmlChar*) "mandatory")) {
      tfield->mandatory = TRUE;
    }
    else {
      DBG1("Error, bad presence option '%s'.", (char*) str);
    }
  }
  xmlFree((void*)str);
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
          0 == xmlStrcasecmp(xmlnode->name, (xmlChar*) "comment"));
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


