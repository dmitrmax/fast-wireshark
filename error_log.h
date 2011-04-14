#ifndef ERROR_LOG_H_INCLUDED_
#define ERROR_LOG_H_INCLUDED_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <gtk/gtk.h>
#include "template.h"
#include "basic-dissect.h"

void setLogSettings(gboolean display, gboolean log);

void log_dynamic_error(const FieldType* ftype, const FieldData* fdata);

void log_static_error(int type, int line, const char* extra_error_info);

void quick_message (gchar *message);

#endif