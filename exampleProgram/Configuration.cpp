/*
 * Configuration.cpp
 *
 *  Created on: 6 mai 2013
 *      Author: oc
 */

#include "Configuration.h"
#include "debug.h"
#include <dbgflags-1/goodies.h>
#include <getopt.h>
#include <cstdio>
#include <cstring>

#define MODULE_FLAG FLAG_INIT

Configuration theConfiguration;

static __inline void printVersion(void)
{
    printf(TO_STRING(PROGRAM_NAME) " v" PACKAGE_VERSION "\n");
}

static __inline void printHelp(const char *errorMsg)
{
#define USAGE  "Usage: " TO_STRING(PROGRAM_NAME) " [OPTIONS] \n" \
    "  -c, --console                     \n"\
    "  -l, --log-level=<level>           \n"\
    "  -l, --log-mask=<mask>             \n"\
    "  -h, --help                        \n"\
    "  -v, --version                     \n"

    if (errorMsg != NULL)
    {
        fprintf(stderr,"Error: %s" USAGE,errorMsg);
    }
    else
    {
        fprintf(stdout,USAGE);
    }

#undef USAGE
}

// we can't use the getopt* function because of the TAO ORB parameters
// -p=<value>
// --param=value
// -p value
// --param value
int Configuration::init(int argc, char *argv[]) {
    int error(EXIT_SUCCESS);
    int n(1);
    while ((n < argc) && (EXIT_SUCCESS == error)) {
        const char *arg = argv[n];
        if ('-' == arg[0]) {
            register const char *c = arg + 1;
            switch(*c) {
                case 's':
                    c++;
                    if ((*c) == '=') {
                        c++;
                        if (*c != '\0') {
                            source = c;
                        } else {
                            error = EINVAL;
                            ERROR_MSG("source argument not set");
                        }
                    } else {
                        arg = argv[++n];
                        if ((*arg != '\0') && (*arg != '-')) {
                            source = arg;
                        } else {
                            error = EINVAL;
                            ERROR_MSG("source argument not set");
                        }
                    }
                    break;
                case 'd':
                    c++;
                    if ((*c) == '=') {
                        c++;
                        if (*c != '\0') {
                            database = c;
                        } else {
                            error = EINVAL;
                            ERROR_MSG("database argument not set");
                        }
                    } else {
                        arg = argv[++n];
                        if ((*arg != '\0') && (*arg != '-')) {
                            database = arg;
                        } else {
                            error = EINVAL;
                            ERROR_MSG("database argument not set");
                        }
                    }
                    break;
                case 'l':
                    c++;
                    errno = EXIT_SUCCESS;
                    if ((*c) == '=') {
                        c++;
                        if (*c != '\0') {
                            const unsigned int newLevel = stringToSyslogLevel(c);
                            error = errno;
                            if (EXIT_SUCCESS == error) {
                                setlogmask(LOG_UPTO(newLevel));
                            } else {
                                ERROR_MSG("invalid log level argument");
                            }
                        } else {
                            error = EINVAL;
                            ERROR_MSG("log level argument not set");
                        }
                    } else {
                        arg = argv[++n];
                        if ((*arg != '\0') && (*arg != '-')) {
                            const unsigned int newLevel = stringToSyslogLevel(arg);
                            error = errno;
                            if (EXIT_SUCCESS == error) {
                                setlogmask(LOG_UPTO(newLevel));
                            } else {
                                ERROR_MSG("invalid argument %s for setting the log level",arg);
                            }
                        } else {
                            error = EINVAL;
                            ERROR_MSG("log level argument not set");
                        }
                    }
                    break;
                case 'm':
                    c++;
                    if ((*c) == '=') {
                        c++;
                        if (*c != '\0') {
                            debugFlags.mask = strtoul(c,NULL,0);
                        } else {
                            error = EINVAL;
                            ERROR_MSG("debug mask argument not set");
                        }
                    } else {
                        arg = argv[++n];
                        if ((*arg != '\0') && (*arg != '-')) {
                            debugFlags.mask = strtoul(arg,NULL,0);
                        } else {
                            error = EINVAL;
                            ERROR_MSG("debug mask argument not set");
                        }
                    }
                    break;
                case 'h':
                    printHelp(0);
                    exit(EXIT_SUCCESS);
                    break;
                case 'v':
                    printVersion();
                    exit(EXIT_SUCCESS);
                    break;
                case '-': // long arguments
                    c ++;
                    if (strncmp(c,"source",strlen("source")) == 0) {
                        c += strlen("source");
                        if ((*c) == '=') {
                            c++;
                            if (*c != '\0') {
                                source = c;
                            } else {
                                error = EINVAL;
                                ERROR_MSG("source argument not set");
                            }
                        } else {
                            arg = argv[++n];
                            if (((*arg != '\0') && (*arg != '-'))) {
                                source = arg;
                            } else {
                                error = EINVAL;
                                ERROR_MSG("source argument not set");
                            }
                        }
                    } else if (strncmp(c,"database",strlen("database")) == 0) {
                        c += strlen("database");
                        if ((*c) == '=') {
                            c++;
                            if (*c != '\0') {
                                database = c;
                            } else {
                                error = EINVAL;
                                ERROR_MSG("database argument not set");
                            }
                        } else {
                            arg = argv[++n];
                            if (((*arg != '\0') && (*arg != '-'))) {
                                database = arg;
                            } else {
                                error = EINVAL;
                                ERROR_MSG("database argument not set");
                            }
                        }
                    } else if (strncmp(c,"log-level",strlen("log-level")) == 0) {
                        c += strlen("log-level");
                        if ((*c) == '=') {
                            c++;
                            const unsigned int newLevel = stringToSyslogLevel(c);
                            error = errno;
                            if (EXIT_SUCCESS == error) {
                                setlogmask(LOG_UPTO(newLevel));
                            }
                        } else {
                            arg = argv[++n];
                            const unsigned int newLevel = stringToSyslogLevel(arg);
                            error = errno;
                            if (EXIT_SUCCESS == error) {
                                setlogmask(LOG_UPTO(newLevel));
                            } else {
                                ERROR_MSG("invalid argument %s for setting the log level",arg);
                            }
                        }
                    } else if (strncmp(c,"log-mask",strlen("log-mask")) == 0) {
                        c += strlen("log-mask");
                        if ((*c) == '=') {
                            c++;
                            debugFlags.mask = strtoul(c,NULL,0);
                        } else {
                            arg = argv[++n];
                            debugFlags.mask = strtoul(arg,NULL,0);
                        }
                    } else if (strncmp(c,"help",strlen("help")) == 0) {
                        printHelp(0);
                        exit(EXIT_SUCCESS);
                    } else if (strncmp(c,"version",strlen("version")) == 0) {
                        printVersion();
                        exit(EXIT_SUCCESS);
                    } else {
                        ERROR_MSG("unknown long option %s",arg);
                    }
                    break;
    #if HAVE_TAO
                case 'O':
                    break;
    #endif //HAVE_TAO
                default:
                    error = EINVAL;
                    ERROR_MSG("invalid argument %s",arg);
                    break;
            } //switch(c)
        } else { //('-' == argv[0])
            error = EINVAL;
            ERROR_MSG("invalid argument %s",arg);
        }
        n++;
    } //while ((n < argc) && (EXIT_SUCCESS == error))

    if (EXIT_SUCCESS == error) {
        // check mandatory parameters
        if (source.empty()) {
            error = EINVAL;
            ERROR_MSG("source argument not set");
        }
    }
    return error;
}
