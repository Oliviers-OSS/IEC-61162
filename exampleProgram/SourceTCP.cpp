/*
 * SourceTCP.cpp
 *
 *  Created on: 9 mai 2013
 *      Author: oc
 */

#include "globals.h"
#include "SourceTCP.h"
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MODULE_FLAG FLAG_SOURCE

static inline void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    } else {
        return &(((struct sockaddr_in6*)sa)->sin6_addr);
    }
}

inline int SourceTCP::fillBuffer() {
    int error(EXIT_SUCCESS);
    // the smallest AIS message is 72 bits length = 9 bytes (payload) + header (13 min) + checksum and padding (4) ... = 26
    char *writeCursor = socketBuffer;
    size_t toRead = sizeof(socketBuffer);
    readCursor = readLimit = socketBuffer;
    errno = 0;
    do {
        const ssize_t nbRead = recv(sockfd, readLimit, toRead, 0);
        if (nbRead >= 0) {
            toRead -= nbRead;
            readLimit += nbRead;
        } else {
            error =  errno;
            if (error != EINTR) {
                ERROR_MSG("recv error %d (%m)",error);
                toRead = 0;
            }
        }
    } while(toRead > 0);

    return error;
}

void SourceTCP::connect(const char *serverName, const char *service) {
    struct addrinfo hints;
    struct addrinfo *serverInfo;
    int error;
    std::string errorMsg;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // IP4 or IP6
    hints.ai_socktype = SOCK_STREAM; // TCP

    error = getaddrinfo(serverName, service, &hints, &serverInfo);
    if (0 == error) {
        // connect to the first server which accepts
        struct addrinfo *cursor;
        std::string errorMsg;
        sprintf(errorMsg,"can't connect to any available server");
        for(cursor = serverInfo; ((cursor != NULL)&& (-1 == sockfd)); cursor = cursor->ai_next) {
            sockfd = socket(cursor->ai_family, cursor->ai_socktype,cursor->ai_protocol);
            if (sockfd != -1) {
                if ((::connect(sockfd, cursor->ai_addr, cursor->ai_addrlen) == 0)) {
                    char server[INET6_ADDRSTRLEN];
                    if (inet_ntop(cursor->ai_family, get_in_addr((struct sockaddr *)cursor->ai_addr),server, sizeof(server)) != 0) {
                        NOTICE_MSG("connecting to %s:%s...",server,service);
                        errorMsg = "";
                    } else {
                        error = errno;
                        sprintf(errorMsg,"inet_ntop error %d (%m)",error);
                    }
                } else {
                    error = errno;
                    WARNING_MSG("connect error %d (%m)",error);
                    close(sockfd);
                    sockfd = -1;
                }
            } else {
                error = errno;
                WARNING_MSG("socket %d,%d,%d error %d(%m)",cursor->ai_family, cursor->ai_socktype,cursor->ai_protocol,error);
            }
        } //for(cursor = servinfo; ((cursor != NULL)&& (-1 == sockfd)); cursor = cursor->ai_next)

        freeaddrinfo(serverInfo);
        serverInfo = 0;

        if (error != EXIT_SUCCESS) {
            throw exception(error,errorMsg);
        }
    } else {
        std::string errorMsg;
        ERROR_MSG("getaddrinfo %s %s error %d",serverName,service,error);
        sprintf(errorMsg,"getaddrinfo %s %s error %d",serverName,service,error);
        throw exception(error,errorMsg);
    }

}

SourceTCP::SourceTCP(const char *serverName, const char *service)
:sockfd(-1)
,readCursor(socketBuffer)
,readLimit(socketBuffer) {
    connect(serverName,service);
}

SourceTCP::~SourceTCP() {
    if (sockfd != -1) {
        close(sockfd);
    }
}

int SourceTCP::getMessage(char *buffer,const size_t bufferSize, size_t &filedSize) {
    int error(EXIT_SUCCESS);
    MutexMgr mutexMgr(mutex);
    enum States {
        waitingForMessageStart
        ,readingMessage
        ,MessageRead
        ,Error
    } state = waitingForMessageStart;
    static const char *endOfMessage = EOM;
    const char *endOfMessageCursor = endOfMessage;
    register char *writeCursor = buffer;
    char const *writeLimit = buffer + bufferSize -1;

    while ((writeCursor < writeLimit)) {
        if (readCursor >= readLimit) {
            error = fillBuffer();
            if ((error != EXIT_SUCCESS) || (readLimit == socketBuffer)) { // readLimit == socketBuffer means no more data from the server to read
                state = Error;
                break;
            }
        }

        if (waitingForMessageStart == state) {
            switch(*readCursor) {
                case '!':
                    // break is missing
                case '$':
                    state = readingMessage;
                    *writeCursor = *readCursor;
                    writeCursor++;
                    break;
                default:
                    // ignore it
                    break;
            } //switch(*readCursor)
            readCursor++;
        } else if (readingMessage == state) {
            *writeCursor = *readCursor;
            writeCursor++;
            if (*endOfMessageCursor == *readCursor) {
                endOfMessageCursor++;
                if ('\0' == *endOfMessageCursor) {
                    state = MessageRead;
                    break;
                }
            }
            readCursor++;
        } else {
            break;
        }
    }
    *writeCursor = '\0';
    if (MessageRead == state) {
        filedSize = writeCursor - buffer;
    } else {
        filedSize = 0;
    }
    DEBUG_VAR(buffer,"%s");

    return error;
}
