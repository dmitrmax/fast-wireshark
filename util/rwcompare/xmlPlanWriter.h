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
#ifndef PLAN_WRITER_H_INCLUDED_
#define PLAN_WRITER_H_INCLUDED_
#define OUTPUT_ENCODING "ISO-8859-1"
#include <glib.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>


xmlTextWriterPtr writer;


gboolean initXMLWriter(const char *output);
gboolean writeMessage(const xmlChar* tid);
gboolean closeMessage();
gboolean writeNestedType(const xmlChar* type, const xmlChar* value);
gboolean closeNestedType();
gboolean writeField(const xmlChar* type, const xmlChar* value);
void closeAndWriteXMLOutput();


#endif

