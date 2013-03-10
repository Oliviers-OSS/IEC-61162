/*
 * debug.h
 *
 *  Created on: 8 mars 2013
 *      Author: oc
 */

#ifndef _DEBUG_H_
#define _DEBUG_H_

#define DebugBreak()     __asm__ ("int $0x3")

#if HAVE_DBGFLAGS

#ifndef LOGGER
#define LOGGER consoleLogger
#endif

#include <dbgflags-1/dbgflags.h>
#include <dbgflags-1/loggers.h>
#include <dbgflags/debug_macros.h>
#include <cassert>

//TODO: groups AIS messages flags

#define FLAG_RUNTIME        FLAG_ID_0
#define FLAG_AIS_RUNTIME    FLAG_ID_1
#define FLAG_AIS_MSG_1      FLAG_ID_2
#define FLAG_AIS_MSG_2      FLAG_ID_3
#define FLAG_AIS_MSG_3      FLAG_ID_4
#define FLAG_AIS_MSG_4      FLAG_ID_5
#define FLAG_AIS_MSG_5      FLAG_ID_6
#define FLAG_AIS_MSG_6      FLAG_ID_7
#define FLAG_AIS_MSG_7      FLAG_ID_8
#define FLAG_AIS_MSG_8      FLAG_ID_9
#define FLAG_AIS_MSG_9      FLAG_ID_10
#define FLAG_AIS_MSG_10     FLAG_ID_11
#define FLAG_AIS_MSG_11     FLAG_ID_12
#define FLAG_AIS_MSG_12     FLAG_ID_13
#define FLAG_AIS_MSG_13     FLAG_ID_14
#define FLAG_AIS_MSG_14     FLAG_ID_15
#define FLAG_AIS_MSG_15     FLAG_ID_16
#define FLAG_AIS_MSG_16     FLAG_ID_17
#define FLAG_AIS_MSG_17     FLAG_ID_18
#define FLAG_AIS_MSG_18     FLAG_ID_19
#define FLAG_AIS_MSG_19     FLAG_ID_20
#define FLAG_AIS_MSG_20     FLAG_ID_21
#define FLAG_AIS_MSG_21     FLAG_ID_22
#define FLAG_AIS_MSG_22     FLAG_ID_23
#define FLAG_AIS_MSG_23     FLAG_ID_24
#define FLAG_AIS_MSG_24     FLAG_ID_25
#define FLAG_AIS_MSG_25     FLAG_ID_26
#define FLAG_AIS_MSG_26     FLAG_ID_27
#define FLAG_AIS_MSG_27     FLAG_ID_28
#define FLAG_AIS_ALARMS     FLAG_ID_29
#define FLAG_GPS            FLAG_ID_30

extern DebugFlags debugFlags;

/*
 * TMP
 */

static inline char* printBinary(const unsigned char v) {
    static const char *stringQuartet[] = {
             "0000"
            ,"0001"
            ,"0010"
            ,"0011"
            ,"0100"
            ,"0101"
            ,"0110"
            ,"0111"
            ,"1000"
            ,"1001"
            ,"1010"
            ,"1011"
            ,"1100"
            ,"1101"
            ,"1110"
            ,"1111"
    };
    static char buffer[10];
    const unsigned char high = v >> 4;
    const unsigned char low = v & 0x0F;
    sprintf(buffer,"%s%s\n",stringQuartet[high],stringQuartet[low]);
    return buffer;
}
#define DEBUG_VAR_BIN(x)        FILTER LOGGER(LOG_DEBUG|LOG_OPTS, SIMPLE_DEBUG_LOG_HEADER __FILE__ "(%d) %s: " #x " = %s" DEBUG_EOL,__LINE__,__FUNCTION__,printBinary(x))

#ifdef _DEBUG_
#define INITIAL_DEBUG_FLAG  0xFFFFFFFF
#else
#define INITIAL_DEBUG_FLAG  (FLAG_RUNTIME|FLAG_AIS_RUNTIME)
#endif

#else // HAVE_DBGFLAGS

#include <cstdio>

#define FLAG_RUNTIME
#define FLAG_AIS_RUNTIME
#define FLAG_AIS_MSG_1
#define FLAG_AIS_MSG_2
#define FLAG_AIS_MSG_3
#define FLAG_AIS_MSG_4
#define FLAG_AIS_MSG_5
#define FLAG_AIS_MSG_6
#define FLAG_AIS_MSG_7
#define FLAG_AIS_MSG_8
#define FLAG_AIS_MSG_9
#define FLAG_AIS_MSG_10
#define FLAG_AIS_MSG_11
#define FLAG_AIS_MSG_12
#define FLAG_AIS_MSG_13
#define FLAG_AIS_MSG_14
#define FLAG_AIS_MSG_15
#define FLAG_AIS_MSG_16
#define FLAG_AIS_MSG_17
#define FLAG_AIS_MSG_18
#define FLAG_AIS_MSG_19
#define FLAG_AIS_MSG_20
#define FLAG_AIS_MSG_21
#define FLAG_AIS_MSG_22
#define FLAG_AIS_MSG_23
#define FLAG_AIS_MSG_24
#define FLAG_AIS_MSG_25
#define FLAG_AIS_MSG_26
#define FLAG_AIS_MSG_27
#define FLAG_AIS_ALARMS
#define FLAG_GPS

#define ERROR_MSG           printf
#define WARNING_MSG         printf
#define NOTICE_MSG          printf
#define INFO_MSG(fmt,...)
#define DEBUG_MSG(fmt,...)
#define DEBUG_POSMSG(fmt,...)
#define DEBUG_MARK
#define DEBUG_VAR(x,f)
#define DEBUG_VAR_BOOL(Var)
#define DEBUG_DUMP_MEMORY(Var,Size)
#define CTX_MSG(fmt,...)

#endif //HAVE_DBGFLAGS

#endif /* _DEBUG_H_ */
