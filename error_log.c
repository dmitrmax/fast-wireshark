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

static gboolean displayDialogs;
static gboolean logToFile;

 /* write dynamic error to log file */
void log_dynamic_error(const FieldType* ftype, const FieldData* fdata){
  
  time_t ltime;   
  FILE *log;
  
  /* get current cal time */
      ltime=time(NULL); 
  
      
  fprintf(stderr,
  "%s\n%s\n\nField Name:\t%s\nField ID:\t%d\nTemplate ID:\t%d\n\n%s\n",
  asctime(localtime(&ltime)),
  fdata->value.ascii.bytes,
  ftype->name,
  ftype->id,
  ftype->tid,
  "**********************************************************************");
      
  if(logToFile){
    log = fopen("error_log.txt","a"); 
    fprintf(log,
    "%s\n%s\n\nField Name:\t%s\nField ID:\t%d\nTemplate ID:\t%d\n\n%s\n",
    asctime(localtime(&ltime)),
    fdata->value.ascii.bytes,
    ftype->name,
    ftype->id,
    ftype->tid,
    "**********************************************************************");
    fclose(log);
  }
  
}

void log_static_error(int err_no, int line, const char* extra_error_info){
  
  time_t ltime;   
  FILE *log;
  char* lineString;
 
  char * string_err_s[6] =
  {
    "[ERR S1] ",
    "[ERR S2] ",
    "[ERR S3] ",
    "[ERR S4] ",
    "[ERR S5] ",
    "[ERR] Template error "
  };
  
  char * err;
  err_no--;
  if(err_no<0 || err_no>5){
    err = (char*)g_strdup_printf("%s", string_err_s[5]);
  } else {
    err = (char*)g_strdup_printf("%s", string_err_s[err_no]);
  }
  
  
  /* get current cal time */
  ltime=time(NULL);
  
  if(line>=0){
    lineString = g_strdup_printf("line(%d)\n", line);
  } else {
    lineString = "";
  }
      
  if(displayDialogs){
    quick_message(g_strdup_printf("FAST Plugin\n%s\n%s%s", err, lineString, extra_error_info));
  }
  
  fprintf(stderr,
  "%s\n%s%s\n%s\n",
  err,
  lineString,
  extra_error_info,
  "**********************************************************************");
  
  if(logToFile){
    log = fopen("error_log.txt","a"); 
    fprintf(log,
    "%s\n%s\n%s%s\n%s\n",
    asctime(localtime(&ltime)),
    err,
    lineString,
    extra_error_info,
    "**********************************************************************");
    fclose(log);
  }
  
}

void setLogSettings(gboolean display, gboolean log){
  displayDialogs = display;
  logToFile = log;
}

/* Function to open a dialog box displaying the message provided. */
void quick_message (gchar *message)
{
   GtkWidget *dialog, *label, *content_area;

   /* Create the widgets */
   dialog = gtk_dialog_new_with_buttons ("FAST Plugin Message",
                                         NULL,
                                         GTK_DIALOG_DESTROY_WITH_PARENT,
                                         GTK_STOCK_OK,
                                         GTK_RESPONSE_NONE,
                                         NULL);
   content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
   label = gtk_label_new (message);

   /* Ensure that the dialog box is destroyed when the user responds */
   g_signal_connect_swapped (dialog,
                             "response",
                             G_CALLBACK (gtk_widget_destroy),
                             dialog);

   /* Add the label, and show everything we've added to the dialog */

   gtk_container_add (GTK_CONTAINER (content_area), label);
   gtk_widget_show_all (dialog);
}