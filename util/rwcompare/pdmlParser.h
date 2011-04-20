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

