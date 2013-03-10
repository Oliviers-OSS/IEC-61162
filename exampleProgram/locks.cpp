/*
 * locks.cpp
 *
 *  Created on: 23 août 2013
 *      Author: oc
 */

#include "locks.h"
#include "tools.h"

MutexMgr::exception::exception(int code,const char *format,...):error(code) {
    va_list optional_arguments;
    va_start(optional_arguments, format);
    vsprintf(msg,format,optional_arguments);
    va_end(optional_arguments);
}
