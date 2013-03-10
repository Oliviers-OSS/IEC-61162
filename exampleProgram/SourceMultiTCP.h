/*
 * SourceMultiTCP.h
 *
 *  Created on: 26 mai 2013
 *      Author: oc
 */

#ifndef SOURCEMULTITCP_H_
#define SOURCEMULTITCP_H_

#include "globals.h"
#include "SourceDevice.h"
#include "SourceTCP.h"
#include "FastMutex.h"
#include <sys/types.h>
#include <sys/socket.h>

#define MODULE_FLAG FLAG_SOURCE

class SourceMultiTCP: public SourceDevice {
    unsigned int numberOfSocket;
    SourceTCP *Sockets;
    struct pollfd *socketsPollfd;
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

    struct address {
        const char *server;
        const char *service;
    };

    SourceMultiTCP(const address addresses[], const size_t number);
    ~SourceMultiTCP();

    virtual int getMessage(char *buffer,const size_t bufferSize, size_t &filedSize);

    static SourceMultiTCP* Factory(const address addresses[], const size_t number,int &error) {
        SourceMultiTCP *sourceMultiTCP = 0;
        error = EXIT_SUCCESS;
        try {
            sourceMultiTCP = new SourceMultiTCP(addresses,number);
        }
        catch(SourceMultiTCP::exception &e) {
            ERROR_MSG("exception SourceMultiTCP caught: %s",e.what());
            error = e.code();
        }
        catch(SourceTCP::exception &e) {
            ERROR_MSG("exception SourceTCP caught: %s",e.what());
            error = e.code();
        }
        catch(std::bad_alloc &e) {
            error = ENOMEM;
            ERROR_MSG("failed to allocate %u bytes for a new SourceMultiTCP object",sizeof(SourceMultiTCP));
        }
        return sourceMultiTCP;
    }
};

#undef MODULE_FLAG

#endif /* SOURCEMULTITCP_H_ */
