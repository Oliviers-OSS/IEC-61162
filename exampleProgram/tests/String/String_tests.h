/*
 * String_tests.h
 *
 *  Created on: 22 août 2013
 *      Author: oc
 */

#ifndef _STRING_TESTS_H_
#define _STRING_TESTS_H_

#define CXXTEST_HAVE_EH
#include <cxxtest/TestSuite.h>
#include "../../String.h"

#define STRING(x) #x
#define TO_STRING(x) STRING(x)
#define EOM "\r\n"

class InternalDatabase_tests : public CxxTest::TestSuite {

public:

    void test_init() {
        String s1;
        TS_ASSERT(s1.isEmpty());
        const std::string test1("test");
        String s2(test1);
        TS_ASSERT_EQUALS(test1,s2);
        const char *test2 = "test";
        String s3(test2);
        TS_ASSERT_EQUALS(test2,(const char*)s3);
    }

    void test_operators_equal() {
        String s1;
        TS_ASSERT(s1.isEmpty());
        s1 = "test";
        TS_ASSERT_EQUALS("test",(const char*)s1);
        std::string t1("test1");
        s1 = t1;
        TS_ASSERT_EQUALS(t1.c_str(),(const char*)s1);
        String s2(s1);
        TS_ASSERT_EQUALS(s1,s2);
        String s3 = s2;
        TS_ASSERT_EQUALS(s1,s3);
    }

    void test_Format() {
        String s1;
        s1.Format("test int=%u, string=%s",5,"string");
        TS_ASSERT_EQUALS((const char*)s1,"test int=5, string=string");
    }

    void test_printf() {
        char result[255];
        String s("test");
        sprintf(result,"%s",(const char*)s); // warning: cannot pass objects of non-POD type ‘class String’ through ‘...’; call will abort at runtime => cast
        TS_ASSERT_EQUALS((const char*)s,result);
    }
};


#endif /* _STRING_TESTS_H_ */
