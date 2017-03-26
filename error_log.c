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

#include "error_log.h"
#include "debug.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <wsutil/report_err.h>

static gboolean config_display_dialogs;
static gboolean config_log_to_file;
static const char* config_log_file_name;

void fast_log_dynamic_error(const FieldType* ftype, const FieldData* fdata)
{
  time_t ltime;
  FILE *log;

  /* get current cal time */
  ltime=time(NULL);

  /* write error log to a string */
  fprintf(stderr,
          "%s\n\nField Name:\t%s\nField ID:\t%d\nTemplate ID:\t%d\n\n%s\n",
          fdata->value.ascii.bytes,     /* error message  */
          ftype->name,                  /* field name     */
          ftype->id,                    /* field id       */
          ftype->tid,                   /* template id    */
          "**********************************************************************");

  /* write error log to file if user preference is set */
  if(config_log_to_file) {
    log = fopen(config_log_file_name, "a");
    fprintf(log,
            "%s\n%s\n\nField Name:\t%s\nField ID:\t%d\nTemplate ID:\t%d\n\n%s\n",
            asctime(localtime(&ltime)),   /* time stamp     */
            fdata->value.ascii.bytes,     /* error message  */
            ftype->name,                  /* field name     */
            ftype->id,                    /* field id       */
            ftype->tid,                   /* template id    */
            "**********************************************************************");
    fclose(log);
  }
}

void fast_log_static_error(int err_no, int line, const char* extra_error_info)
{
  time_t ltime;
  FILE *log = NULL;
  char* line_string = NULL;

  static const char * string_err_s[6] =
  {
    "[ERR S1] ",
    "[ERR S2] ",
    "[ERR S3] ",
    "[ERR S4] ",
    "[ERR S5] ",
    "[ERR] Template error "
  };

  /* set error message string */
  const char * err;
  err_no--;
  if(err_no < 0 || err_no > 5) {
    err = string_err_s[5];
  } else {
    err = string_err_s[err_no];
  }

  /* get current cal time */
  ltime = time(NULL);

  if(line >= 0) {
    line_string = g_strdup_printf("line(%d)\n", line);
  } else {
    line_string = g_strdup_printf(" ");
  }

  /* display pop-up dialog if user preference is set */
  if(config_display_dialogs)
    report_failure("FAST Plugin\n%s\n%s%s", err, line_string, extra_error_info);

  /* display error message in console */
  fprintf(stderr,
          "%s\n%s%s\n%s\n",
          err,
          line_string,
          extra_error_info,
          "**********************************************************************");

  /* write error message to file if user preference is set */
  if(config_log_to_file){
    log = fopen(config_log_file_name, "a");
    fprintf(log,
            "%s\n%s\n%s%s\n%s\n",
            asctime(localtime(&ltime)),
            err,
            line_string,
            extra_error_info,
            "**********************************************************************");
    fclose(log);
  }

  g_free(line_string);
}

void fast_set_log_settings(gboolean display, gboolean log, const gchar* log_file_name) {
  config_display_dialogs = display;
  config_log_to_file = log;
  config_log_file_name = log_file_name;

  fprintf(stderr, "output log file: %s\n", config_log_file_name);
}
