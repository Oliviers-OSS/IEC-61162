/*
 * DelimitedFile_tests.h
 *
 *  Created on: 30 avr. 2013
 *      Author: oc
 */

#ifndef _DELIMITED_FILE_TESTS_H_
#define _DELIMITED_FILE_TESTS_H_

#include "config.h"
#define CXXTEST_HAVE_EH
#include <cxxtest/TestSuite.h>
#include <cerrno>
#include <cstdlib>
#include <unistd.h>
#include <dbgflags-1/debug_macros.h>
#include "../../DelimitedFile.h"

#define STRING(x) #x
#define TO_STRING(x) STRING(x)
#define EOM "\r\n"
#include <dbgflags-1/loggers.h>

class DelimitedFile_tests : public CxxTest::TestSuite {

public:
    void test_basic_tiny_file() {
        const size_t pageSize = 4096; //getpagesize();
        try {
            DelimitedFile<char,pageSize> file(TO_STRING(SRCDIR)"/test0.txt");
            ssize_t size(0);
            char buffer[127];
            do {
                size = file.getLine(buffer,sizeof(buffer),"\n");
                if (size) {
                    TS_ASSERT_EQUALS(buffer,"0123\n");
                }
            } while( size > 0);

        } //
        catch(DelimitedFile<char,pageSize>::exception &e) {
            ERROR_MSG("exception caught: %s",e.what());
            TS_FAIL("exception caught from File");
        }
    }

    void test_basic_getline_mono() {
        const size_t pageSize = 4096;
        const char *filesContent[] = {
                "012345678901234567890\n"
                ,"abcdefghijklmnopqrstuvwxyz\n"
                ,"ABCDEFGHIJKLMNOPQRSTUVWXYZ\n"
        };
        try {
            DelimitedFile<char,pageSize> file(TO_STRING(SRCDIR)"/test1.txt");
            ssize_t size(0);
            char buffer[127];
            unsigned int n(0);
            do {
                size = file.getLine(buffer,sizeof(buffer),"\n");
                if (size) {
                    TS_ASSERT_EQUALS(buffer,filesContent[n]);
                }
                n++;
                if (n >= (sizeof(filesContent)/sizeof(filesContent[0]))) {
                    n = 0;
                }
            } while( size > 0);
        } //
        catch(DelimitedFile<char,pageSize>::exception &e) {
            ERROR_MSG("exception caught: %s",e.what());
            TS_FAIL("exception caught from File");
        }
    }

    void test_basic_getlineex_mono() {
        const size_t pageSize = 4096;
        const char *filesContent[] = {
                "012345678901234567890\r\n"
                ,"abcdefghijklmnopqrstuvwxyz\r\n"
                ,"ABCDEFGHIJKLMNOPQRSTUVWXYZ\r\n"
        };
        try {
            DelimitedFile<char,pageSize> file(TO_STRING(SRCDIR)"/test1.txt");
            ssize_t size(0);
            char buffer[127];
            unsigned int n(0);
            do {
                size = file.getLineEx(buffer,sizeof(buffer),"\n",EOM);
                if (size) {
                    TS_ASSERT_EQUALS(buffer,filesContent[n]);
                }
                n++;
                if (n >= (sizeof(filesContent)/sizeof(filesContent[0]))) {
                    n = 0;
                }
            } while( size > 0);

        } //
        catch(DelimitedFile<char,pageSize>::exception &e) {
            ERROR_MSG("exception caught: %s",e.what());
            TS_FAIL("exception caught from File");
        }
    }

    void test_basic_file_error() {
        const size_t pageSize = 4096;
        //TS_ASSERT_THROWS(DelimitedFile<char,pageSize> file("/foo/test1.txt"), File<char,pageSize>::exception);
    }

    void test_basic_bad_delimited_file() {
        const size_t pageSize = 4096;
        try {
            DelimitedFile<char,pageSize> file(TO_STRING(SRCDIR)"/badFile.txt");
            ssize_t size(0);
            char buffer[127];
            unsigned int n(0);
            do {
                size = file.getLine(buffer,sizeof(buffer));
                TS_ASSERT_EQUALS(size,0);
            } while( size > 0);

        } //
        catch(DelimitedFile<char,pageSize>::exception &e) {
            ERROR_MSG("exception caught: %s",e.what());
            TS_FAIL("exception caught from File");
        }
    }

    void test_small_buffer() {
        const size_t pageSize = 5;
        const char *filesContent[] = {
                "012345678901234567890\n"
                ,"abcdefghijklmnopqrstuvwxyz\n"
                ,"ABCDEFGHIJKLMNOPQRSTUVWXYZ\n"
        };
        try {
            DelimitedFile<char,5> file(TO_STRING(SRCDIR)"/test1.txt");
            ssize_t size(0);
            char buffer[127];
            unsigned int n(0);
            do {
                size = file.getLine(buffer,sizeof(buffer),"\n");
                if (size) {
                    TS_ASSERT_EQUALS(buffer,filesContent[n]);
                }
                n++;
                if (n >= (sizeof(filesContent)/sizeof(filesContent[0]))) {
                    n = 0;
                }
            } while( size > 0);
        } //
        catch(DelimitedFile<char,pageSize>::exception &e) {
            ERROR_MSG("exception caught: %s",e.what());
            TS_FAIL("exception caught from File");
        }
    }

    void test_pb_refCountry() {
        const size_t pageSize = 4096;
        const char *filesContent[] = {
            "0;\"Not set\""
            ,"201;\"Albania (Republic of)\""
            ,"202;\"Andorra (Principality of)\""
            ,"203;\"Austria\""
            ,"204;\"Azores - Portugal\""
            ,"205;\"Belgium\""
            ,"206;\"Belarus (Republic of)\""
            ,"207;\"Bulgaria (Republic of)\""
            ,"208;\"Vatican City State\""
            ,"209;\"Cyprus (Republic of)\""
            ,"210;\"Cyprus (Republic of)\""
            ,"211;\"Germany (Federal Republic of)\""
            ,"212;\"Cyprus (Republic of)\""
            ,"213;\"Georgia\""
        };
        try {
            DelimitedFile<char,pageSize> file(TO_STRING(SRCDIR)"/countries_test.csv");
            ssize_t size(0);
            char buffer[127];
            unsigned int n(0);
            do {
                size = file.getLineEx(buffer,sizeof(buffer),"\n","");
                if (size) {
                    TS_ASSERT_EQUALS(buffer,filesContent[n]);
                }
                n++;
                if (n >= (sizeof(filesContent)/sizeof(filesContent[0]))) {
                    n = 0;
                }
            } while( size > 0);
        } //
        catch(DelimitedFile<char,pageSize>::exception &e) {
            ERROR_MSG("exception caught: %s",e.what());
            TS_FAIL("exception caught from File");
        }
    }
};


#endif /* _DELIMITED_FILE_TESTS_H_ */
