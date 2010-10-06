
#ifndef PDML_PARSER_H_INCLUDED_
#define PDML_PARSER_H_INCLUDED_
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include "xmlPlanWriter.h"


int parsedFastPackets; /* Number of fast packets Parsed */

xmlChar * getFieldType(const xmlChar *fullname);
int parseField(xmlDocPtr doc, xmlNodePtr xmlnode);
int walkFields(xmlDocPtr doc, xmlNodePtr xmlnode);
int parseTemplate(xmlDocPtr doc, xmlNodePtr xmlnode);
int parseFastPacket(xmlDocPtr doc, xmlNodePtr xmlnode);
int parsePacket (xmlDocPtr doc, xmlNodePtr xmlnode);
int parseDoc(char *docname);
int ignoreXmlNode(xmlNodePtr xmlnode);


#endif

