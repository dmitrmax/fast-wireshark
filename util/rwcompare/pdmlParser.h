
#ifndef PDML_PARSER_H_INCLUDED_
#define PDML_PARSER_H_INCLUDED_
#include <glib.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

int parsedFastPackets; /* Number of fast packets Parsed */

xmlChar * getFieldType(const xmlChar *fullname);
gboolean parseField(xmlDocPtr doc, xmlNodePtr xmlnode);
gboolean walkFields(xmlDocPtr doc, xmlNodePtr xmlnode);
gboolean parseTemplate(xmlDocPtr doc, xmlNodePtr xmlnode);
gboolean parseFastPacket(xmlDocPtr doc, xmlNodePtr xmlnode);
gboolean parsePacket (xmlDocPtr doc, xmlNodePtr xmlnode);
gboolean parseDoc(const char *docname);
gboolean ignoreXmlNode(xmlNodePtr xmlnode);
gboolean generatePlanFromPDML(const char * pdmlFilename, const char * outputPlanFilename);


#endif

