#ifndef _GLOBALS_H
#define _GLOBALS_H

#include "config.h"
#include <cerrno>
#include <cstdlib>
#include <pthread.h>

#define STRING(x) #x
#define TO_STRING(x) STRING(x)
#define FEOL "\n"
#define EOM "\r\n"
#define NO_AUTO_TIMEOUT_COMMIT
#define SQLITE_JOURNAL_MODE "MEMORY"    // valid parameters: MEMORY, WAL ...

#include "debug.h"
#include "FastMutex.h"
#include "locks.h"
#include "tools.h"
#include "Configuration.h"
#include "InterThreadsMsgQueue.h"

typedef MsgQueue<20,std::string> RawDataMsgQueue;

extern RawDataMsgQueue rawDataMsgQueue;

#endif // _GLOBALS_H
