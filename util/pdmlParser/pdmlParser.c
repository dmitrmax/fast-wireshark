
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
int parseField(xmlDocPtr doc, xmlNodePtr xmlnode){

  int rc;
  xmlChar *type;
  xmlChar *value;
  xmlChar *fullname;
  

  /* Get field type */
  fullname = xmlGetProp( xmlnode, "name");
  type = getFieldType(fullname);
  if(type==NULL){
    fprintf(stderr, "ERROR: field type wrong, not in format fast.x (ex. fast.int32)\n");
    return false;
  }

  if(xmlStrcmp(type, (const xmlChar *)"group")==0){
    /* parse and write group */
    rc = writeGroup();
    if(rc==false) return false;

    rc = walkFields(doc, xmlnode);
    if(rc==false) return false;

    rc = closeGroup();
    if(rc==false) return false;

    return true;
  } else if(xmlStrcmp(type, (const xmlChar *)"sequence")==0){
    /* parse and write sequence */
    rc = writeSequence();
    if(rc==false) return false;

    rc = walkFields(doc, xmlnode);
    if(rc==false) return false;

    rc = closeSequence();
    if(rc==false) return false;

    return true;
  }
  /* else treat as normal field */

  /*Get value of field */
  value = xmlGetProp( xmlnode, "show");

  /* write field */
  rc = writeField(type, value);
  if(rc==false) return false;

  return true;

}

/* Walk through the childern of the given xmlnode
   and parse the fields */
int walkFields(xmlDocPtr doc, xmlNodePtr xmlnode){

  /* loop through packet and find fields */
	xmlnode = xmlnode->xmlChildrenNode;
	while (xmlnode != NULL) {
		if(ignoreXmlNode(xmlnode)){
      xmlnode = xmlnode->next;
      continue;
    }

    if (xmlStrcmp(xmlnode->name, (const xmlChar *)"field")==0){
      if(!parseField(doc, xmlnode)){
        return false;
      }
    }  	

    xmlnode = xmlnode->next;
  }  

  return true;
}

/* Parse the template section of the fast packet info */
int parseTemplate(xmlDocPtr doc, xmlNodePtr xmlnode){

  int rc;
  xmlChar *tid;
  xmlChar *prop;

  tid = xmlGetProp( xmlnode, "show");
  if(tid==NULL){
    fprintf(stderr, "ERROR: no template id\n");
  }

  /* write message element to output file */
  rc = writeMessage(tid);
  if(rc==false) return false;

  rc = walkFields(doc, xmlnode);
  if(rc==false) return false;
  
  rc = closeMessage();
  if(rc==false) return false;

  return true;
}

/* Parse a fast section of a packet */
int parseFastPacket(xmlDocPtr doc, xmlNodePtr xmlnode){

  xmlChar *prop;

	/* loop through packet and find fields */
	xmlnode = xmlnode->xmlChildrenNode;
	while (xmlnode != NULL) {
		if(ignoreXmlNode(xmlnode)){
      xmlnode = xmlnode->next;
      continue;
    }

    /* Look for template fields */
    if (xmlStrcmp(xmlnode->name, (const xmlChar *)"field")==0){
      prop = xmlGetProp( xmlnode, "name");
      if(xmlStrcmp(prop, (const xmlChar *)"fast.tid")==0){
        if(!parseTemplate(doc, xmlnode)){
          return false;
        }
      }
    }

    xmlnode = xmlnode->next;
  }

  parsedFastPackets++;
  return true;
}

/* Parse a Packet section of the XML document
    looking for fast info */
int parsePacket (xmlDocPtr doc, xmlNodePtr xmlnode) {

  xmlChar *prop;

	/* loop through packet and find fields */
	xmlnode = xmlnode->xmlChildrenNode;
	while (xmlnode != NULL) {
		if(ignoreXmlNode(xmlnode)){
      xmlnode = xmlnode->next;
      continue;
    }

    if (xmlStrcmp(xmlnode->name, (const xmlChar *)"proto")==0){
      prop = xmlGetProp( xmlnode, "name");
      if(xmlStrcmp(prop, (const xmlChar *)"fast")==0){
        if(!parseFastPacket(doc, xmlnode)){
          return false;
        }
      }
    }

    xmlnode = xmlnode->next;
  }
    
  return true;
}

/* Parse the pdml file looking for packets */
int parseDoc(char *docname) {

	xmlDocPtr doc; /* pointer to XML document */
	xmlNodePtr xmlnode; /* pointer to current node within document */

	doc = xmlParseFile(docname); /* attempt to parse xml file and get pointer to parsed document */
	
	if (doc == NULL ) {
		fprintf(stderr,"Document not parsed successfully. \n");
		return false;
	}
	
	xmlnode = xmlDocGetRootElement(doc);  /* start at the root of the XML document */
	
	if (xmlnode == NULL) {
		fprintf(stderr,"empty document\n");
		xmlFreeDoc(doc);
		return false;
	}
	
	xmlnode = xmlnode->xmlChildrenNode;
	while (xmlnode != NULL) {

		if(!ignoreXmlNode(xmlnode)){
		
      if ((!xmlStrcmp(xmlnode->name, (const xmlChar *)"packet"))){
			  /* parse template, and check if parse failed */
			  if(!parsePacket(doc, xmlnode)){
				  xmlFreeDoc(doc);
				  return false;
        }
		  } else {
			  fprintf(stderr,"Warning: what is a %s?\n", xmlnode->name);
		  }

		}
		xmlnode = xmlnode->next;
	}
	
	xmlFreeDoc(doc);
	return true;
}

int main(int argc, char **argv) {	

	int sucess;
	char *docname;
  char *output;
  parsedFastPackets = 0;
		
	if (argc <= 2) {
		printf("Usage: %s inputXML outputXML\n", argv[0]);
    return(1);
	}

	
	docname = argv[1];
  output = argv[2];
  
  if(initXMLWriter(output)){

	  sucess = parseDoc(docname);
  
	  if(sucess){
		  printf("Sucessfuly parsed xml doc %s\n", docname);
      printf("Found %d fast packets\n", parsedFastPackets);
      closeAndWriteXMLOutput();
	  } else {
		  fprintf(stderr,"Failed to parse %s\n", docname);
      return 0;
	  }
  }

	return (0);
}

/* Check if parser should ignore given node */
int ignoreXmlNode(xmlNodePtr xmlnode){
  return (0 == xmlStrcasecmp(xmlnode->name, (xmlChar*) "text") ||
          0 == xmlStrcasecmp(xmlnode->name, (xmlChar*) "comment"));
}


