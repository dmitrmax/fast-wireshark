
#ifndef PARSETEMPLATE_H_INCLUDED_
#define PARSETEMPLATE_H_INCLUDED_

#include <glib.h>

GNode* parse_templates_xml (const char* filename);
void quick_message (gchar *message);

#endif

