/*
 * SourceMultiTCP.cpp
 *
 *  Created on: 26 mai 2013
 *      Author: oc
 */

#include "globals.h"
#include "SourceMultiTCP.h"
#include <cstring>
#include <poll.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MODULE_FLAG FLAG_SOURCE

 SourceMultiTCP::SourceMultiTCP(const address addresses[], const size_t number)
     :numberOfSocket(number)
     ,Sockets(0)
     ,socketsPollfd(0) {
     register const address *cursor = addresses;
     const address *limit = addresses + number;
     Sockets = new SourceTCP[number];
     socketsPollfd = new pollfd[number];
     register SourceTCP *socketCursor = Sockets;
     register struct pollfd *pollCursor = socketsPollfd;
     while(cursor < limit) {
         socketCursor->connect(cursor->server,cursor->service);
         pollCursor->fd = socketCursor->sockfd;
         pollCursor->events = POLLIN;
         pollCursor->revents = 0;
         cursor++;
         socketCursor++;
         pollCursor++;
     } //while(cursor < limit)
}

 SourceMultiTCP::~SourceMultiTCP() {
    if (Sockets) {
        delete [] Sockets;
        Sockets = 0;
    }
    if (socketsPollfd) {
        delete [] socketsPollfd;
        socketsPollfd = 0;
    }
}

int SourceMultiTCP::getMessage(char *buffer,const size_t bufferSize, size_t &filedSize) {
    int error(EXIT_SUCCESS);
    int timeout(-1);
    filedSize = 0;
    int rv = poll(socketsPollfd,numberOfSocket,timeout);
    if (rv > 0) {
        // look for socket(s) with data to read
        register SourceTCP *socketCursor = Sockets;
        register struct pollfd *pollCursor = socketsPollfd;
        register struct pollfd *pollCursorLimit = socketsPollfd + numberOfSocket;
        while((pollCursor < pollCursorLimit) && (0 == filedSize)){
            if (pollCursor->revents & POLLIN) {
                error = socketCursor->getMessage(buffer,bufferSize,filedSize);
            }
            pollCursor++;
            socketCursor++;
        }
    } else if (0 == rv) {
        WARNING_MSG("poll timeout (%d ms)",timeout);
    } else {
        error = errno;
        ERROR_MSG("poll error %d (%m)",error);
    }
    return error;
}
