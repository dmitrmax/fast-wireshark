
#ifndef PLAN_WRITER_H_INCLUDED_
#define PLAN_WRITER_H_INCLUDED_
#define OUTPUT_ENCODING "ISO-8859-1"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>

#define false 0
#define true 1



xmlTextWriterPtr writer;


int initXMLWriter(const char *output);
int writeMessage(const xmlChar* tid);
int closeMessage();
int writeGroup();
int closeGroup();
int writeSequence();
int closeSequence();
int writeField(const xmlChar* type, const xmlChar* value);
void closeAndWriteXMLOutput();


#endif

