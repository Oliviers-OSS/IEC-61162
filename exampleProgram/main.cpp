/*
 ============================================================================
 Name        : main.cpp
 Author      : Olivier Charloton
 Version     :
 Copyright   : BSD (example program and LGPL for the library )
 Description : Uses shared library to print greeting
               To run the resulting executable the LD_LIBRARY_PATH must be
               set to ${project_loc}/libIEC61162/.libs
               Alternatively, libtool creates a wrapper shell script in the
               build directory of this program which can be used to run it.
               Here the script will be called exampleProgram.
 ============================================================================
 */

#include "globals.h"
#include "SourceDevice.h"
#include "SourceFile.h"
#include "SourceSerial.h"
#include "SourceTCP.h"
#include "IEC61162/IEC-61162.h"
#include "InternalDatabaseWriter.h"
#include "corbaInterfaces.h"
#include <cstring>

#if HAVE_TAO
#endif //HAVE_TAO

RawDataMsgQueue rawDataMsgQueue;
bool continueProgram = true;
#define MODULE_FLAG FLAG_IDB_WRITER

void *internalDatabaseWriter (void *parameter) {
    int error(EXIT_SUCCESS);
    try {
        InternalDatabaseWriter database;
        unsigned int mask = 0xFFFFFFFF;
        IEC61162::AISMessage aisMessageParser;

        // initialization: "connect messages to the internal database"
        #define CONNECT_DB_AIS_MSG(n) if ((mask & n) == n) aisMessageParser.message##n.connect(sigc::mem_fun(database, &InternalDatabaseWriter::onMessage##n))
        CONNECT_DB_AIS_MSG(1);
        CONNECT_DB_AIS_MSG(2);
        CONNECT_DB_AIS_MSG(3);
        CONNECT_DB_AIS_MSG(4);
        CONNECT_DB_AIS_MSG(5);
        CONNECT_DB_AIS_MSG(6);
        CONNECT_DB_AIS_MSG(7);
        CONNECT_DB_AIS_MSG(8);
        CONNECT_DB_AIS_MSG(9);
        CONNECT_DB_AIS_MSG(10);
        CONNECT_DB_AIS_MSG(12);
        CONNECT_DB_AIS_MSG(13);
        CONNECT_DB_AIS_MSG(14);
        CONNECT_DB_AIS_MSG(15);
        //    CONNECT(16);
        //    CONNECT(17);
        CONNECT_DB_AIS_MSG(18);
        CONNECT_DB_AIS_MSG(19);
        //    CONNECT(20);
        //    CONNECT(21);
        //    CONNECT(22);
        //    CONNECT(23);
        //CONNECT_DB_AIS_MSG(24);
        if ((mask & 24) == 24) {
            aisMessageParser.message24A.connect(sigc::mem_fun(database, &InternalDatabaseWriter::onMessage24A));
            aisMessageParser.message24B.connect(sigc::mem_fun(database, &InternalDatabaseWriter::onMessage24B));
        }
        //    CONNECT(25);
        //    CONNECT(26);
        CONNECT_DB_AIS_MSG(27);
        #undef CONNECT_DB_AIS_MSG

        // get message from the reader
        // then send them to the parser
        std::string message;
        struct timespec timeout;
        timeout.tv_sec = theConfiguration.getTransactionMaxTime();
        timeout.tv_nsec = 0;

        do {
            error = rawDataMsgQueue.getMessage(message,timeout);
            switch(error) {
                case EXIT_SUCCESS: {
                        const char *rawMessage = message.c_str();
                        error = aisMessageParser.parse(rawMessage);
                        if (error != EXIT_SUCCESS) {
                            WARNING_MSG("error %d parsing %s",error,rawMessage);
                        } else {
                            DEBUG_MSG("%s successfully parsed",rawMessage);
                        }
                    }
                    break;
                case ETIMEDOUT:
                    database.onTimeOut(true);
                    break;
                default:
                    DEBUG_MSG("rawDataMsgQueue getMessage error %d",error);
                    break;
            } // switch(error)

        } while(continueProgram);
    }
    catch(InternalDatabaseWriter::exception &e) {
        ERROR_MSG("InternalDatabaseWriter::exception caught, error code = %d, message = %s",e.code(),e.what());
    }

    continueProgram = false;
    return NULL;
}
#undef CONNECT_DB_AIS_MSG

#undef MODULE_FLAG
#define MODULE_FLAG FLAG_SOURCE

static inline int readSource(SourceDevice *device) {
    int error(EXIT_SUCCESS);
    char buffer[127];
    size_t size;
    do {
        error = device->getMessage(buffer,sizeof(buffer),size);
        if ((EXIT_SUCCESS == error) && (size > 0)) {
            rawDataMsgQueue.putMessage(buffer);
        } else {
            continueProgram = false; // TODO only end at the end of file or fatal network error
        }
    } while(continueProgram);

    INFO_MSG("end of source reading...");
    return error;
}

#undef MODULE_FLAG
#define MODULE_FLAG FLAG_INIT

// serial://<device name>[:baudrate][]
// tcp://<full IP address including port>  ex: tcp://127.0.0.1:39000
// udp://<full IP address including port>  ex: udp://127.0.0.1:39000
// file://<full file path> ex: file:///tmp/sample.txt
// mtcp://
//static inli
static inline int SourcesClassFactory(const char* sourceURL, SourceDevice **device) {
    int error(EXIT_SUCCESS);
    const char *protocol = strstr(sourceURL,"://");
    if (protocol != NULL) {
        if (strncasecmp(sourceURL,"file",strlen("file")) == 0) {
            const char *filename = protocol + 3;
            *device = SourceFile::Factory(filename,error);
        } else if (strncasecmp(sourceURL,"serial",strlen("serial")) == 0) {
//TODO serial source
        } else if (strncasecmp(sourceURL,"tcp",strlen("tcp")) == 0) {
            char buffer[strlen(sourceURL)+1];
            strcpy(buffer,protocol + 3);
            char *server = buffer;
            char *service = strchr(server,':');
            if (service != 0) {
                *service = '\0';
                service++;
                *device = SourceTCP::Factory(server,service,error);
            } else {
                error = EINVAL;
                ERROR_MSG("port / service not found in %s",sourceURL);
            }
        } else if (strncasecmp(sourceURL,"udp",strlen("udp")) == 0) {
//TODO UDP source
        } else {
            error = ENODEV;
            ERROR_MSG("Unknown protocol in %s",sourceURL);
        }
    } else {
        error = EINVAL;
        ERROR_MSG("Protocol not found in %s",sourceURL);
    }
    return error;
}

int main(int argc, char *argv[]) {
  int error = theConfiguration.init(argc,argv);
  if (EXIT_SUCCESS == error) {
      SourceDevice *device = 0;
      const char *source = theConfiguration.getSource();
      error = SourcesClassFactory(source,&device);
      if (EXIT_SUCCESS == error) {
          error = startServants(argc,argv);
          if (EXIT_SUCCESS == error) {
              // start the Msg parsing & database thread
              pthread_t parser;
              error = pthread_create(&parser, NULL,internalDatabaseWriter,NULL);
              if (0 == error) {
                  // then the reader loops
                  error = readSource(device);

                  // wait for the end of the parser
                  DEBUG_MSG("waiting for the parser ending");
                  error = pthread_join(parser,NULL);
              } else {
                  ERROR_MSG("pthread_create internalDatabaseWriter error %d",error);
              }
          } // (EXIT_SUCCESS == startServants ) error already printed
      } // !(EXIT_SUCCESS == SourcesClassFactory ) error already printed
  } // !(EXIT_SUCCESS == theConfiguration.init) error already printed
  DEBUG_MSG("Program ended");
  return error;
}
