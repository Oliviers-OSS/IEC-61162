/*
 * debug.cpp
 *
 *  Created on: 5 mai 2013
 *      Author: oc
 */

#include "globals.h"

#if HAVE_DBGFLAGS

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define MODULE_FLAG FLAG_RUNTIME

static void help(FILE *stream);
static int dbgCommandsHandler(int argc, char *argv[], FILE *stream);

DebugFlags debugFlags = {
    "IEC61162",
    {
        "runtime"
        , "AIS_Runtime"
        , "AIS_MSG01"
        , "AIS_MSG02"
        , "AIS_MSG03"
        , "AIS_MSG04"
        , "AIS_MSG05"
        , "AIS_MSG06"
        , "AIS_MSG07"
        , "AIS_MSG08"
        , "AIS_MSG09"
        , "AIS_MSG10"
        , "AIS_MSG11"
        , "AIS_MSG12"
        , "AIS_MSG13"
        , "AIS_MSG14"
        , "AIS_MSG15"
        , "AIS_MSG16"
        , "AIS_MSG17"
        , "AIS_MSG18"
        , "AIS_MSG19"
        , "AIS_MSG20"
        , "AIS_MSG21"
        , "AIS_MSG22"
        , "AIS_MSG23"
        , "AIS_MSG24"
        , "AIS_MSG25"
        , "AIS_MSG26"
        , "AIS_MSG27"
        , "AIS_ALARMS"
        , "GPS"
        , ""
    }
    , INITIAL_DEBUG_FLAG
    , {help, dbgCommandsHandler}
};

void help(FILE *stream) {
    //fprintf(stream, "debug help commands:\n\ttest\n\thello world\n\tcmd -f<arg1> [-s] [-t<arg2>]\n");
}

static int dbgCommandsHandler(int argc, char *argv[], FILE *stream) {
    int error(EINVAL);
    return error;
}


class LibraryDebugFlagsMgr {
#define CMDLINE_SRC "/proc/self/cmdline"
    int readCommandLine() {
        int error(EXIT_SUCCESS);
        int cmdLine = open(CMDLINE_SRC,O_RDONLY);
        if (cmdLine != -1) {
            char buffer[4096];
            ssize_t n = read(cmdLine,buffer,sizeof(buffer));
            if (n != -1) {
                const char *cursor = buffer;
            } else {
                error = errno;
                ERROR_MSG("read " CMDLINE_SRC " error %d (%m)",error);
            }
            close(cmdLine);
            cmdLine = -1;
        } else {
            error = errno;
            ERROR_MSG("open " CMDLINE_SRC " error %d (%m)",error);
        }
        return error;
    }
public:
    LibraryDebugFlagsMgr() {
        registerLibraryDebugFlags(&debugFlags);
    }

    ~LibraryDebugFlagsMgr() {
        unregisterDebugFlags(&debugFlags);
    }

};

LibraryDebugFlagsMgr dbgFlagsMgr __attribute__((init_priority(1000)));


#endif // HAVE_DBGFLAGS
