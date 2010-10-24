
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "debug.h"
#include "xmlPlanWriter.h"

#include "pdmlParser.h"

/* Get the typename of the field by
  removeing the 'fast.' section from
  the full field name */
xmlChar * getFieldType(const xmlChar *fullname){

  int length;
  xmlChar * type;

  length = xmlStrlen(fullname);

  if(length<6){
    fprintf(stderr, "ERROR: field name does not contain 'fast.' name=%s\n",fullname);
    return NULL;
  }

  /* get field type substring */
  /* ex. fast.int32 -> int32 */
  type = xmlStrdup(fullname+5);

  return type;
}

/* Parse a field */
gboolean parseField(xmlDocPtr doc, xmlNodePtr xmlnode){

  int rc = TRUE;
  xmlChar *type;
  xmlChar *fullname;
  xmlChar* prop;
  const xmlChar* value;

  /* Get field type */
  fullname = xmlGetProp( xmlnode, (xmlChar *)"name");
  type = getFieldType(fullname);
  xmlFree(fullname);
  if(type==NULL){
    BAILOUT(FALSE, "ERROR: field type wrong, not in format fast.x (ex. fast.int32).");
  }

  /* Get value of field */
  prop = xmlGetProp( xmlnode, (xmlChar *)"showname");
  value = xmlStrchr(prop, ':');
  if (value) {
    if (value[0] && value[1]) {
      value = value+2;
    }
    else {
      value = (xmlChar*) "";
    }
  }

  if (xmlStrcasecmp(type, (xmlChar *)"tid") == 0) {
    if (rc)  rc = writeNestedType((xmlChar*)"Message", value);
    if (rc)  rc = walkFields(doc, xmlnode);
    if (rc)  rc = closeNestedType("Message");
  }
  else if (xmlStrcasecmp(type, (xmlChar *)"group")==0){
    /* parse and write group */
    if (rc)  rc = writeNestedType((xmlChar*)"group", value);
    if (rc)  rc = walkFields(doc, xmlnode);
    if (rc)  rc = closeNestedType("group");
  } else if (xmlStrcasecmp(type, (xmlChar *)"sequence")==0){
    /* parse and write sequence */
    if (rc)  rc = writeNestedType((xmlChar*)"sequence", value);
    if (rc)  rc = walkFields(doc, xmlnode);
    if (rc)  rc = closeNestedType("sequence");
  }
  else {
    /* Treat as normal field. */
    /* write field */
    rc = writeField(type, value);
  }

  if (prop)  xmlFree(prop);
  xmlFree(type);

  return rc;

}

/* Walk through the childern of the given xmlnode
   and parse the fields */
gboolean walkFields(xmlDocPtr doc, xmlNodePtr xmlnode){

  /* loop through packet and find fields */
	xmlnode = xmlnode->xmlChildrenNode;
	while (xmlnode != NULL) {
		if(ignoreXmlNode(xmlnode)){
      xmlnode = xmlnode->next;
      continue;
    }

    if (xmlStrcmp(xmlnode->name, (xmlChar *)"field")==0){
      if(!parseField(doc, xmlnode)){
        return FALSE;
      }
    }  	

    xmlnode = xmlnode->next;
  }  

  return TRUE;
}

/* Parse the template section of the fast packet info */
gboolean parseTemplate(xmlDocPtr doc, xmlNodePtr xmlnode){

  return parseField(doc, xmlnode);
}

/* Parse a fast section of a packet */
gboolean parseFastPacket(xmlDocPtr doc, xmlNodePtr xmlnode){

  gboolean successp = TRUE;

	/* loop through packet and find fields */
	xmlnode = xmlnode->xmlChildrenNode;
	while (xmlnode != NULL && successp) {
		if(ignoreXmlNode(xmlnode)){
      xmlnode = xmlnode->next;
      continue;
    }

    if (xmlStrcmp(xmlnode->name, (xmlChar *)"field")==0){
      /* Look for template fields */
      xmlChar *prop;
      prop = xmlGetProp( xmlnode, (xmlChar *)"name");
      if(xmlStrcmp(prop, (xmlChar *)"fast.tid")==0){
        if(!parseTemplate(doc, xmlnode)){
          successp = FALSE;
        }
      }
      xmlFree(prop);
    }

    xmlnode = xmlnode->next;
  }

  if (successp) {
    parsedFastPackets++;
  }
  return successp;
}

/*! \brief  Parse a Packet section of the XML document
 *          looking for fast info.
 */
gboolean parsePacket (xmlDocPtr doc, xmlNodePtr xmlnode) {
  gboolean successp = TRUE;
  /* loop through packet and find fields */
  xmlnode = xmlnode->xmlChildrenNode;
  while (xmlnode != NULL && successp) {
    if(ignoreXmlNode(xmlnode)){
      xmlnode = xmlnode->next;
      continue;
    }
    if (xmlStrcmp(xmlnode->name, (xmlChar *)"proto")==0) {
      xmlChar *prop;
      prop = xmlGetProp( xmlnode, (xmlChar *)"name");
      if(xmlStrcmp(prop, (xmlChar *)"fast")==0){
        if(!parseFastPacket(doc, xmlnode)){
          successp = FALSE;
        }
      }
      xmlFree(prop);
    }

    xmlnode = xmlnode->next;
  }

  return successp;
}

/* Parse the pdml file looking for packets */
gboolean parseDoc(const char *docname) {

	xmlDocPtr doc; /* pointer to XML document */
	xmlNodePtr xmlnode; /* pointer to current node within document */

	doc = xmlParseFile(docname); /* attempt to parse xml file and get pointer to parsed document */
	
	if (doc == NULL) {
    BAILOUT(FALSE, "Document not parsed successfully.");
	}
	
	xmlnode = xmlDocGetRootElement(doc);  /* start at the root of the XML document */
	
	if (xmlnode == NULL) {
		xmlFreeDoc(doc);
		BAILOUT(FALSE, "Empty document.");
	}
	
	xmlnode = xmlnode->xmlChildrenNode;
	while (xmlnode != NULL) {

		if(!ignoreXmlNode(xmlnode)){
		
      if ((!xmlStrcmp(xmlnode->name, (xmlChar *)"packet"))){
			  /* parse template, and check if parse failed */
			  if(!parsePacket(doc, xmlnode)){
				  xmlFreeDoc(doc);
				  return FALSE;
        }
		  } else {
			  fprintf(stderr,"Warning: what is a %s?\n", xmlnode->name);
		  }

		}
		xmlnode = xmlnode->next;
	}
	
	xmlFreeDoc(doc);
	return TRUE;
}

gboolean generatePlanFromPDML(const char * pdmlFilename, const char * outputPlanFilename){

	gboolean success;
  parsedFastPackets = 0;
  
  success = initXMLWriter(outputPlanFilename);
  if (success) {

	  success = parseDoc(pdmlFilename);
  
	  if(success){
		  printf("Sucessfuly parsed xml doc %s\n", pdmlFilename);
      printf("Found %d fast packets\n", parsedFastPackets);
      closeAndWriteXMLOutput();
      success = TRUE;
	  } else {
		  fprintf(stderr,"Failed to parse %s\n", pdmlFilename);
      success = FALSE;
	  }
  }

	return success;
}

/* Check if parser should ignore given node */
gboolean ignoreXmlNode(xmlNodePtr xmlnode){
  return (0 == xmlStrcasecmp(xmlnode->name, (xmlChar*) "text") ||
          0 == xmlStrcasecmp(xmlnode->name, (xmlChar*) "comment"));
}


