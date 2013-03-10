/*
 * SourceFile.h
 *
 *  Created on: 7 mai 2013
 *      Author: oc
 */

#ifndef _SOURCE_FILE_H_
#define _SOURCE_FILE_H_

#include "globals.h"
#include "SourceDevice.h"

#define MODULE_FLAG FLAG_SOURCE
#include "DelimitedFile.h"

class SourceFile : public SourceDevice {
    DelimitedFile<char,4096> file;
public:
    SourceFile(const char *filename):file(filename) {

    }

    virtual ~SourceFile() {

    }

    virtual int getMessage(char *buffer,const size_t bufferSize, size_t &usedSize) {
        int error(EXIT_SUCCESS);
        try {
            usedSize = file.getLineEx(buffer,bufferSize,FEOL,EOM);
            DEBUG_VAR(buffer,"%s");
        } //
        catch(DelimitedFile<char,4096>::exception &e) {
            ERROR_MSG("exception caught: %s",e.what());
            error = e.code();
        }
        return error;
    }

    static SourceFile* Factory(const char *filename,int &error) {
        SourceFile *sourceFile = 0;
        error = EXIT_SUCCESS;
        try {
            sourceFile = new SourceFile(filename);
        }
        catch(DelimitedFile<char,4096>::exception &e) {
            ERROR_MSG("exception caught: %s",e.what());
            error = e.code();
        }
        catch(std::bad_alloc &e) {
            error = ENOMEM;
            ERROR_MSG("failed to allocate %u bytes for a new SourceFile object",sizeof(SourceFile));
        }
        return sourceFile;
    }
};


#undef MODULE_FLAG

#endif /* _SOURCE_FILE_H_ */
