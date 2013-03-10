#include "globals.h"

#define MODULE_FLAG FLAG_RUNTIME

#if HAVE_TAO
// use C++ interface (streams) only when using CORBA
emergencyLogger logEmergency;
alertLogger logAlert;
criticalLogger logCritical;
errorLogger logError;
warningLogger logWarning;
noticeLogger logNotice;
infoLogger logInfo;
debugLogger logDebug;
contextLogger logContext;
#endif //HAVE_TAO

int dbgCommandsHandler(int argc, char *argv[], FILE *stream);
void help(FILE *stream);

DebugFlags debugFlags = {
    "test",
    {
        "init"
        , "runtime"
        , "source"
        , "messages"
        , "DB_writer"
        , "DB_reader"
        , "CORBA"
        , ""
        , ""
        , ""
        , ""
        , ""
        , ""
        , ""
        , ""
        , ""
        , ""
        , ""
        , ""
        , ""
        , ""
        , ""
        , ""
        , ""
        , ""
        , ""
        , ""
        , ""
        , ""
        , ""
        , ""
        , ""
    }
    , INITIAL_FLAGS_VALUE
    , {help, dbgCommandsHandler}
};

void help(FILE *stream) {
    fprintf(stream, "debug help commands:\n\ttest\n\thello world\n\tcmd -f<arg1> [-s] [-t<arg2>]\n");
}

int dbgCommandsHandler(int argc, char *argv[], FILE *stream) {
    int status = EXIT_SUCCESS;
    if (argc > 1) {
#if 0
        if (strcmp(argv[0], "test") == 0) {
            testDbgCommandsHandler(stream);
        } else if ((strcmp(argv[0], "hello") == 0) && (strcmp(argv[1], "world") == 0)) {
            helloWorldDbgCommandsHandler(stream);
        } else if (strcmp(argv[0], "cmd") == 0) {
            int optc;
            cmdArguments arguments;
            memset(&arguments,0,sizeof(arguments));
            while ((optc = getopt_long(argc, argv, "f:st:", longopts, NULL)) != -1) {
                switch (optc) {
                    case 'f':
                        arguments.first = optarg;
                        break;
                    case 's':
                        arguments.second = 1;
                        break;
                    case 't':
                        if (optarg != NULL) {
                            arguments.third = optarg;
                        }
                        break;
                    default:
                        status = EINVAL;
                        ERROR_MSG("invalid cmd command parameter");
                }
            } /* while ((optc = getopt_long (argc, argv, "f:st:", longopts, NULL)) != -1) */

            if (EXIT_SUCCESS == status) {
                status = cmdDbgCommandsHandler(&arguments,stream);
            }
        } else { /* unknow/invalid cmd */
            status = EINVAL;
            ERROR_MSG("unknow or invalid cmd received");
        }
#endif
    } else { /*! argc > 1 */
        ERROR_MSG("bad debug cmd received");
        status = EINVAL;
    }

    DEBUG_VAR(status, "%d");
    return status;
}
