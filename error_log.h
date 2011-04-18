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

#ifndef ERROR_LOG_H_INCLUDED_
#define ERROR_LOG_H_INCLUDED_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <gtk/gtk.h>
#include "template.h"
#include "basic-dissect.h"

/*! \brief set user preferences for error dialogs and log files
 *  \param display true if user wants error dialogs to display
 *  \param log true if user wants error messages written to log file
 */
void setLogSettings(gboolean display, gboolean log);

/*! \brief log a dynamic error that has occurred
*  \param ftype field type for the field that caused the error
*  \param fdata field data for the field that caused the error
*/
void log_dynamic_error(const FieldType* ftype, const FieldData* fdata);

/*! \brief log a static error that has occurred
*  \param ftype field type for the field that caused the error
*  \param fdata field data for the field that caused the error
*/
void log_static_error(int type, int line, const char* extra_error_info);

/*! \brief open a gtk dialog box displaying the message provided
*  \param message the message to display in the dialog
*/
void quick_message (gchar *message);

#endif