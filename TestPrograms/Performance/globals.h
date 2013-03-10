#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include "config.h"

#include <cstdlib>
#include <cerrno>
#include <cstdio>
#include <pthread.h>
#include <string>

#define STRING(x) #x
#define TO_STRING(x) STRING(x)
#define EOM "\r\n"

#include "debug.h"
#include "InterThreadsMsgQueue.h"

typedef MsgQueue<10,std::string> File2ParserMsg;

#endif /* _GLOBAL_H_ */
