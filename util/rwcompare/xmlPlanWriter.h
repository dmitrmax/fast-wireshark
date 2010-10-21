
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

