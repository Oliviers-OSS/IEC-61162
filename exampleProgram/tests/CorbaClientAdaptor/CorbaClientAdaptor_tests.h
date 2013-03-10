/*
 * CorbaClientAdaptor_tests.h
 *
 *  Created on: 08 sept. 2013
 *      Author: oc
 */

#ifndef _CORBA_CLIENT_ADAPTOR_TESTS_H_
#define _CORBA_CLIENT_ADAPTOR_TESTS_H_

#include "config.h"
#include "CorbaClientAdaptor.h"
#define CXXTEST_HAVE_EH
#include <cxxtest/TestSuite.h>
#include <cerrno>
#include <cstdlib>
#include <unistd.h>
#include <cstdio>
#include <dbgflags-1/debug_macros.h>
#include <memory>

#define STRING(x) #x
#define TO_STRING(x) STRING(x)
#define EOM "\r\n"
#include <dbgflags-1/loggers.h>

class CorbaClientAdaptor_tests : public CxxTest::TestSuite {

public:
    void test_basic_build() {
        ::AIS::VesselsData_var v(new ::AIS::VesselsData);
        CorbaClientAdaptor cca(v);
    }

    void test_setNumberOfSubItems() {
        ::AIS::VesselsData_var v(new ::AIS::VesselsData);
        CorbaClientAdaptor cca(v);
        cca.setNumberOfSubItems(7);
    }

    void test_setLabels() {
        const size_t numberOfColumns(7);
        ::AIS::VesselsData_var v(new ::AIS::VesselsData);
        CorbaClientAdaptor cca(v);
        cca.setNumberOfSubItems(numberOfColumns);
        char Buffer[50];
        unsigned int i;
        for(i = 0 ; i <  numberOfColumns ; i++) {
            sprintf(Buffer,"Label %u",i);
            cca.setLabel(i,Buffer);
        }
        for(i = 0 ; i <  numberOfColumns ; i++) {
            sprintf(Buffer,"Label %u",i);
            const char *label =  cca.getLabel(i);
            TS_ASSERT_EQUALS(Buffer,label);
        }
    }

    void test_setValue_int() {
        const size_t numberOfColumns(7);
        const size_t numberOfLines(10);
        ::AIS::VesselsData_var v(new ::AIS::VesselsData);
        CorbaClientAdaptor cca(v);
        cca.setNumberOfSubItems(numberOfColumns);
        unsigned int line,column;
        for(line = 0; line < numberOfLines; line++) {
            for(column = 0 ; column <  numberOfColumns ; column++) {
                const int value = column + 10;
                cca.setValue(line,column,value);
            }
        }
        cca.reSizeData(numberOfLines);
        TS_ASSERT_EQUALS(numberOfLines,cca.getSizeData());
        for(line = 0; line < numberOfLines; line++) {
            for(column = 0 ; column <  numberOfColumns ; column++) {
                const int value = column + 10;
                CORBA::Any v = cca.getValue(line,column);
                CORBA::Long val;
                v >>= val;
                TS_ASSERT_EQUALS(val,value);
            }
        }
    }

    void test_setValue_double() {
        const size_t numberOfColumns(7);
        const size_t numberOfLines(10);
        ::AIS::VesselsData_var v(new ::AIS::VesselsData);
        CorbaClientAdaptor cca(v);
        cca.setNumberOfSubItems(numberOfColumns);
        unsigned int line,column;
        for(line = 0; line < numberOfLines; line++) {
            for(column = 0 ; column <  numberOfColumns ; column++) {
                const double value(column * 10.0);
                cca.setValue(line,column,value);
            }
        }
        cca.reSizeData(numberOfLines);
        TS_ASSERT_EQUALS(numberOfLines,cca.getSizeData());
        for(line = 0; line < numberOfLines; line++) {
            for(column = 0 ; column <  numberOfColumns ; column++) {
                const double value(column * 10.0);
                CORBA::Any v = cca.getValue(line,column);
                CORBA::Double val;
                v >>= val;
                TS_ASSERT_EQUALS((double)val,value);
            }
        }
    }

    void test_setValue_string() {
        const size_t numberOfColumns(7);
        const size_t numberOfLines(10);
        ::AIS::VesselsData_var v(new ::AIS::VesselsData);
        CorbaClientAdaptor cca(v);
        cca.setNumberOfSubItems(numberOfColumns);
        unsigned int line,column;
        char value[50];
        for(line = 0; line < numberOfLines; line++) {
            for(column = 0 ; column <  numberOfColumns ; column++) {
                sprintf(value,"Value %u",column);
                cca.setValue(line,column,value);
            }
        }
        cca.reSizeData(numberOfLines);
        TS_ASSERT_EQUALS(numberOfLines,cca.getSizeData());
        for(line = 0; line < numberOfLines; line++) {
            for(column = 0 ; column <  numberOfColumns ; column++) {
                sprintf(value,"Value %u",column);
                CORBA::Any v = cca.getValue(line,column);
                const char *val;
                v >>= val;
                TS_ASSERT_EQUALS(val,value);
            }
        }
    }
};


#endif /* _CORBA_CLIENT_ADAPTOR_TESTS_H_ */
