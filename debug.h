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

void debug_out(const char*,
               const char*,
               int,
               const char* fmt,
               ...);

#ifdef _MSC_VER
# define __func__ __FUNCTION__
#endif

#ifndef DBGN_FILENAME
#define DBGN_FILENAME __FILE__
#endif

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

#define DBG_RET(r)  DBG1("Returned %d",r)

#define BAILOUT(r,s) \
  do { \
    DBG1("%s", s); \
    return r; \
  } while (0)
  
#define BAILOUT_VOID(s) \
  do { \
    DBG1("%s", s); \
    return; \
  } while (0)

#define TRACE() DBG0("trace")

#endif

