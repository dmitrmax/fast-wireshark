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
  debug_out(DBGN_FNAME,__func__,__LINE__,s,a,b,c,d,e)
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

#define TRACE() DBG0("trace")

#endif

