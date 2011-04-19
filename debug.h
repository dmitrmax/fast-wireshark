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
 * \file  debug.h
 * \brief  Useful debug macros.
 */

#ifndef DEBUG_H_
#define DEBUG_H_

/*!
 * \brief Prints a line of debugging information including file and line number
 * Uses fprintf to print to stderr the given format string and arguments
 * Inserts the file name, function and line number of where the debug statement
 * exists
 * Max length of the filled string is 1024
 * \param file The name of the file
 * \param function The name of the function
 * \param line_num The line number
 * \param fmt The printf format string
 * \param args The arguments to be put in fmt
 */
void debug_out(const char*,
               const char*,
               int,
               const char* fmt,
               ...);

/*!
 * \brief Defines a macro for the name of the function we are currently in
 */
#ifdef _MSC_VER
# define __func__ __FUNCTION__
#endif

/*!
 * \brief Defines a macro for the name of the file we are currently in
 */
#ifndef DBGN_FILENAME
#define DBGN_FILENAME __FILE__
#endif

/*!
 * \brief Various macros for printing a different amount of arguments
 * \param s The printf formatting string to output
 * \param a-d The various arguments for s
 */
#define DBG0(s)  debug_out(DBGN_FILENAME,__func__,__LINE__,s)
#define DBG1(s,a)  debug_out(DBGN_FILENAME,__func__,__LINE__,s,a)
#define DBG2(s,a,b)  debug_out(DBGN_FILENAME,__func__,__LINE__,s,a,b)
#define DBG3(s,a,b,c)  debug_out(DBGN_FILENAME,__func__,__LINE__,s,a,b,c)
#define DBG4(s,a,b,c,d)  debug_out(DBGN_FILENAME,__func__,__LINE__,s,a,b,c,d)
#define DBG5(s,a,b,c,d,e) \
  debug_out(DBGN_FILENAME,__func__,__LINE__,s,a,b,c,d,e)
#define DBG6(s,a,b,c,d,e,f) \
  debug_out(DBGN_FILENAME,__func__,__LINE__,s,a,b,c,d,e,f)
#define DBG7(s,a,b,c,d,e,f,g) \
  debug_out(DBGN_FILENAME,__func__,__LINE__,s,a,b,c,d,e,f,g)
#if 0
/* Use this if you like C99. */
#define DBG(...)		debug_out(DBGN_FILENAME,__func__,__LINE__,## __VA_ARGS__)
#endif

/*!
 * \brief Simple macro for quickly printing what integer got returned
 */
#define DBG_RET(r)  DBG1("Returned %d",r)

/*!
 * \brief Macro to be used as a placeholder for future error handling
 * Macro that prints the string given and returns the value given.
 * To be used when a function errors, but proper handling of the error
 * would slow development and the error should not happen normally
 * \param r The value to return
 * \param s The string to print
 */
#define BAILOUT(r,s) \
  do { \
    DBG1("%s", s); \
    return r; \
  } while (0)

/*!
 * \brief Macro to be used as a placeholder for future error handling
 * Macro that prints the string given and returns
 * To be used when a function errors, but proper handling of the error
 * would slow development and the error should not happen normally
 * \param s The string to print
 */
#define BAILOUT_VOID(s) \
  do { \
    DBG1("%s", s); \
    return; \
  } while (0)

/*!
 * \brief Macro for quick checking if control flow logic is working correctly
 */
#define TRACE() DBG0("trace")

#endif

