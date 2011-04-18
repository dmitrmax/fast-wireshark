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

/*!
 * \file  debug.c
 * \brief  Debugging output functions.
 */

#include <glib/gprintf.h>

#include "debug.h"

/*! \brief  Print debugging info.
 * \param file  Filename of caller.
 * \param func  Function name of caller.
 * \param line  Line number of caller.
 * \param fmt   Format string for a printf.
 */
void debug_out(const char* file,
               const char* func,
               int line,
               const char* fmt,
               ...)
{
  char buf[1024];
  va_list args;

  g_snprintf(buf,1024,"%s(%d) %s: %s\n",file,line,func,fmt);

  va_start(args,fmt);

  g_vfprintf(stderr,buf,args);

  va_end(args);
}

