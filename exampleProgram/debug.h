#ifndef _DEBUG_H_
#define _DEBUG_H_

#define DebugBreak()     __asm__ ("int $0x3")

#if HAVE_DBGFLAGS

#ifndef LOGGER
#define LOGGER consoleLogger
#endif

#ifndef DEBUG_LOG_HEADER
#define DEBUG_LOG_HEADER ""
#endif /*DEBUG_LOG_HEADER */
#define DEBUG_EOL "\n"

#include <dbgflags-1/dbgflags.h>
#include <dbgflags-1/loggers.h>
#include <dbgflags/debug_macros.h>

#ifdef _DEBUG_
#define DEFAULT_LOG_OPTION LOG_PERROR|LOG_PID|LOG_TID
#else
#define DEFAULT_LOG_OPTION LOG_PID|LOG_TID
#endif //_DEBUG_

#define FLAG_INIT           FLAG_0
#define FLAG_RUNTIME        FLAG_1
#define FLAG_SOURCE         FLAG_2
#define FLAG_ITC            FLAG_3
#define FLAG_IDB_WRITER     FLAG_4
#define FLAG_IDB_READER     FLAG_5
#define FLAG_CORBA          FLAG_6
#define FLAG_LOCKS          FLAG_7

#ifdef _DEBUG_
#define INITIAL_FLAGS_VALUE  FLAG_CORBA //0xFFFFFFFF //FLAG_INIT|FLAG_SINGLETON|FLAG_RUNTIME|FLAG_MONITOR|FLAG_CORBA|FLAG_CONFIG|FLAG_NOTIFICATION // ((unsigned int)-1)
#define INITIAL_LOG_LEVEL    "debug"
#define DUMP() dump()
#else
#define INITIAL_FLAGS_VALUE  //FLAG_RUNTIME|FLAG_MONITOR
#define INITIAL_LOG_LEVEL    "warning"
#define DUMP()
#endif

extern DebugFlags debugFlags;

#define TRACE_FCT_CALL traceFunctionCall<debugLogger> f(logDebug,__FUNCTION__,((debugFlags.mask &  MODULE_FLAG)!=0x0))

#else

#define TRACE_FCT_CALL

#endif //HAVE_DBGFLAGS

#include <ostream>
#include <exception>

inline std::ostream& operator<< (std::ostream &o, std::exception &ex) {
  return o << "Exception : " << ex.what() ;
}

#endif /* _DEBUG_H_ */
