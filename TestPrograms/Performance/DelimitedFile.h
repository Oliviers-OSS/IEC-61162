/*
 * DelimitedFile.h
 *
 *  Created on: 29 avr. 2013
 *      Author: oc
 */

#ifndef _DELIMITED_FILE_H_
#define _DELIMITED_FILE_H_

#include "globals.h"
#include <exception>
#include <string>
#include <fcntl.h>
#include <cerrno>
#include <cstdio>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include "tools.h"

template <typename datatype = char, size_t size = 4096>
class DelimitedFile {
    int file;
    pthread_mutex_t getLineMutex;
    datatype *getLineBuffer;
    datatype *getLineReadCursor;
    datatype *getLineBufferLimit;

    int fillBuffer() {
        int error(EXIT_SUCCESS);
        datatype *writeCursor = getLineBuffer;
        size_t toRead = size * sizeof(datatype);
        getLineReadCursor = getLineBufferLimit = getLineBuffer;
        errno = 0;
        do {
            const ssize_t nbRead = read(file,(void *)writeCursor,toRead);
            if (nbRead >= 0) {
                //toRead -= nbRead;
                getLineBufferLimit += nbRead;
                toRead = 0;
            } else {
              error =  errno;
              if (error != EINTR) {
                  ERROR_MSG("read error %d (%m)",error);
                  toRead = 0;
              }
            }
        } while(toRead > 0);

        return error;
    }

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

    DelimitedFile(const char *filename)
        :file(-1)
        ,getLineMutex(PTHREAD_MUTEX_INITIALIZER)
        ,getLineBuffer(0)
        ,getLineReadCursor(0)
        ,getLineBufferLimit(0) {
        file = open(filename,O_RDONLY);
        if (file != -1) {
            int error = posix_fadvise(file, 0,0,POSIX_FADV_SEQUENTIAL);
            if (unlikely(error != 0)) {
                WARNING_MSG("posix_fadvise POSIX_FADV_SEQUENTIAL error %d",error);
            }

            error = posix_fadvise(file, 0,0,POSIX_FADV_WILLNEED);
            if (unlikely(error != 0)) {
                WARNING_MSG("posix_fadvise POSIX_FADV_WILLNEED error %d",error);
            }

            getLineBufferLimit = getLineReadCursor = getLineBuffer = new datatype[size];
            error = fillBuffer();
            if (unlikely(error != 0)) {
                std::string errorMsg;
                sprintf(errorMsg,"read %s error %d (%m)",filename,error);
                throw exception(error,errorMsg); //error already logged
            }
        } else {
            const int error(errno);
            std::string errorMsg;
            sprintf(errorMsg,"open %s error %d (%m)",filename,error);
            ERROR_MSG("open %s error %d (%m)",filename,error);
            throw exception(error,errorMsg);
        }
    }

    ~DelimitedFile() {
        if (file != -1) {
            close(file);
            file = -1;
        }
        pthread_mutex_destroy(&getLineMutex);

        delete[] getLineBuffer;
        getLineReadCursor = getLineBuffer = 0;
    }

    ssize_t getLine(datatype *line,const size_t stringSize, const char *delimiter = "\r\n") {
        MutexMgr mutexMgr(getLineMutex);
        int error(EXIT_SUCCESS);
        register ssize_t length(0);
        register datatype *writeCursor = line;
        datatype *writeLimit = line + stringSize -1;
        const datatype *delimiterCursor = delimiter;

        // delimiterCursor == '\0' means we have found the delimiter(s)
        // getLineBufferLimit != getLineBuffer means the data buffer is not empty (getLineBufferLimit is updated by fillBuffer)
        while ((writeCursor < writeLimit) && (*delimiterCursor != '\0') && (getLineBufferLimit != getLineBuffer)) {
            *writeCursor = *getLineReadCursor;
            ++length;
            if (*writeCursor != *delimiterCursor) {
                delimiterCursor = delimiter;
            } else {
                ++delimiterCursor;
            }
            writeCursor++;
            getLineReadCursor++;
            if (getLineReadCursor >= getLineBufferLimit) {
                error = fillBuffer();
            }
        }

        if (*delimiterCursor != '\0') {
            // the buffer doesn't contain the delimiter (at least the full sequence)
            length = 0;
        }

        *writeCursor = '\0'; //TODO set it depending of the datatype, cf. char tray

        return length;
    }

    ssize_t getLineEx(datatype *line,const size_t stringSize, const char *fileDelimiter = "\n",const char *newDelimiter = "\r\n") {
        ssize_t length =  getLine(line,stringSize,fileDelimiter);
        const size_t fileDelimiterLength(strlen(fileDelimiter));
        if (length >= fileDelimiterLength) {
            // check if the current delimiter is already right
            const size_t newDelimiterLength(strlen(newDelimiter));
            const char *lastChars = line + length - newDelimiterLength -1;
            if (strcmp(lastChars,newDelimiter) != 0) {
                // delimiter must be changed
                const size_t newSize = length + newDelimiterLength - fileDelimiterLength;
                if (newSize < stringSize) {
                    register char *replaceStartPos = line + length - fileDelimiterLength;
                    *replaceStartPos = '\0';
                    strcat(replaceStartPos,newDelimiter);
                    length = newSize;
                } else {
                    WARNING_MSG("buffer'size doesn't allow to replace the delimiter in the file by the new one");
                    length = 0;
                }
            } else {
                //DEBUG_MSG("current delimiter is already right");
            }
        }
        return length;
    }
};

#endif /* _DELIMITED_FILE_H_ */
