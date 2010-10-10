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

