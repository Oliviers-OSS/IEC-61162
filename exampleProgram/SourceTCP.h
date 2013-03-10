/*
 * SourceTCP.h
 *
 *  Created on: 7 mai 2013
 *      Author: oc
 */

#ifndef _SOURCE_TCP_H_
#define _SOURCE_TCP_H_

#include "globals.h"
#include "SourceDevice.h"
#include "FastMutex.h"
#include <sys/types.h>
#include <sys/socket.h>

#define MODULE_FLAG FLAG_SOURCE

class SourceMultiTCP;

class SourceTCP : public SourceDevice {
    int sockfd;
    char socketBuffer[26];
    char *readCursor;
    char *readLimit;
    FastMutex mutex; //to manage access to the buffer
    int fillBuffer();
    friend class SourceMultiTCP;
    SourceTCP():sockfd(-1),readCursor(socketBuffer),readLimit(socketBuffer) {}
    void connect(const char *serverName, const char *service);
public:
    class exception : public std::exception {
        int error;
        std::string msg;
    public:
        exception(int code,const std::string &message) throw()
            :error(code),msg(message) {
        }
        virtual ~exception() throw() {
        }
        virtual const char* what() {
            return msg.c_str();
        }
        virtual int code() {
            return error;
        }
    };

    SourceTCP(const char *serverName, const char *service);
    ~SourceTCP();
    virtual int getMessage(char *buffer,const size_t bufferSize, size_t &filedSize);

    static SourceTCP* Factory(const char *serverName, const char *service,int &error) {
        SourceTCP *sourceTCP = 0;
        error = EXIT_SUCCESS;
        try {
            sourceTCP = new SourceTCP(serverName,service);
        }
        catch(SourceTCP::exception &e) {
            ERROR_MSG("exception caught: %s",e.what());
            error = e.code();
        }
        catch(std::bad_alloc &e) {
            error = ENOMEM;
            ERROR_MSG("failed to allocate %u bytes for a new SourceTCP object",sizeof(SourceTCP));
        }
        return sourceTCP;
    }
};

#undef MODULE_FLAG

#endif /* _SOURCE_TCP_H_ */
