/*
 * BasicFunctions.h
 *
 *  Created on: 12 avr. 2013
 *      Author: oc
 */

#ifndef BASICFUNCTIONS_H_
#define BASICFUNCTIONS_H_

#include "config.h"
#include <cxxtest/TestSuite.h>
#include <sigc++/sigc++.h>
#include <cerrno>
#include <cstdlib>
#include <IEC61162/IEC-61162.h>
#include <cmath>
#include <cstring>

#define EOM "\r\n"
#include <dbgflags-1/loggers.h>
#include "../../../libIEC61162/AISMessages.cpp"

class AISBasicFunctions_Tests : public CxxTest::TestSuite {

    signed char ROT(float rotSensor) {
        signed char AISValue(0x80);
        if (rotSensor < 0.0) {
            AISValue = -nearbyintf(4.733 * sqrt(-rotSensor));
        } else {
            AISValue = nearbyintf(4.733 * sqrt(rotSensor));
        }
        return AISValue;
    }

public:

    void test_decodePayload(void) {
        //                          012345678901234
        const char *payload6bits = "!AIVDM,1,1,,A,1P000Oh1IT1svTP2r:43grwb0Eq4,0*01"EOM;
        unsigned char payload8bits[128];
        size_t payloadSizeInBits(0);
        std::memset(payload8bits,0,sizeof(payload8bits));
        int error = decodePayload(payload6bits+14,payload8bits,payloadSizeInBits);
        TS_ASSERT_EQUALS(error,EXIT_SUCCESS);
        if (EXIT_SUCCESS == error) {
            register const unsigned char *p(payload8bits);
            const unsigned char *e(payload8bits + (payloadSizeInBits/8));
            const unsigned char result[] = {0x06,0x00,0x00,0x01,0xFC,0x01,0x66,0x40,0x7B,0xFA,0x48,0x02,0xE8,0xA1,0x03,0xBF,0xAF,0xEA,0x01,0x5E,0x44};
            register const unsigned char *r(result);
            while(p<e) {
                TS_ASSERT_EQUALS(*p,*r);
                p++;
                r++;
            }
        }
    }

    void test_RateOfTurn(void) {
        const float relativeErrorAccepted(5.0); // 5 degrees / minute because of used encoding formula
        int error;
        signed char rawROTValue;
        bool available;
        float rateOfTurn;
        float sensorValue;

        sensorValue = 1.1;
        rawROTValue = ROT(sensorValue);
        error = RateOfTurn(rawROTValue,rateOfTurn,available);
        TS_ASSERT_DELTA(rateOfTurn,sensorValue,relativeErrorAccepted);
        TS_ASSERT_EQUALS(error,EXIT_SUCCESS);
        TS_ASSERT_EQUALS(available,true);

        sensorValue = 690.0;
        rawROTValue = ROT(sensorValue);
        error = RateOfTurn(rawROTValue,rateOfTurn,available);
        TS_ASSERT_DELTA(rateOfTurn,sensorValue,relativeErrorAccepted);
        TS_ASSERT_EQUALS(error,EXIT_SUCCESS);
        TS_ASSERT_EQUALS(available,true);

        sensorValue = -690.0;
        rawROTValue = ROT(sensorValue);
        error = RateOfTurn(rawROTValue,rateOfTurn,available);
        TS_ASSERT_DELTA(rateOfTurn,sensorValue,relativeErrorAccepted);
        TS_ASSERT_EQUALS(error,EXIT_SUCCESS);
        TS_ASSERT_EQUALS(available,true);

        rawROTValue = 127 ; //+127 = turning right at more than 5o per 30 s (No TI available)
        error = RateOfTurn(rawROTValue,rateOfTurn,available);
        TS_ASSERT_EQUALS(available,false);
        TS_ASSERT_IS_INFINITE(rateOfTurn);
        TS_ASSERT(rateOfTurn > 0);
        TS_ASSERT_EQUALS(error,ERROR_CODE(FACILITY_AIS_MSG,OUT_OF_BOUNDARY));

        rawROTValue = -127 ; //*127 = turning left at more than 5o per 30 s (No TI available)
        error = RateOfTurn(rawROTValue,rateOfTurn,available);
        TS_ASSERT_EQUALS(available,false);
        TS_ASSERT_IS_INFINITE(rateOfTurn);
        TS_ASSERT(rateOfTurn < 0);
        TS_ASSERT_EQUALS(error,ERROR_CODE(FACILITY_AIS_MSG,OUT_OF_BOUNDARY));

        rawROTValue = -128 ; //–128 (80 hex) indicates no turn information available (default).
        error = RateOfTurn(rawROTValue,rateOfTurn,available);
        TS_ASSERT_EQUALS(available,false);
        TS_ASSERT_IS_NAN(rateOfTurn);
        TS_ASSERT_EQUALS(error,EXIT_SUCCESS);
    }

    void test_longitude(void) {
        const float relativeErrorAccepted(0.0001);
        long rawLongValue;
        bool available;
        float longitude;

        rawLongValue = 0xF7F490;
        longitude = setLongitude(rawLongValue,available);
        TS_ASSERT_EQUALS(available,true);
        TS_ASSERT_DELTA(longitude,27.083334,relativeErrorAccepted);

        rawLongValue = 0xB9F7680;
        longitude = setLongitude(rawLongValue,available);
        TS_ASSERT_EQUALS(available,true);
        TS_ASSERT_DELTA(longitude,-122.392531666667,relativeErrorAccepted);

        rawLongValue = 0x6791AC0;
        longitude = setLongitude(rawLongValue,available);
        TS_ASSERT_EQUALS(available,false);
        TS_ASSERT_EQUALS(longitude,181.0);
    }

    void test_latitude(void) {
        const float relativeErrorAccepted(0.001);
        long rawLatValue;
        bool available;
        float latitude;

        rawLatValue = 0x2E8A10;
        latitude = setLatitude(rawLatValue,available);
        TS_ASSERT_EQUALS(available,true);
        TS_ASSERT_DELTA(latitude,5.08333333333333,relativeErrorAccepted);

        rawLatValue = 0x7056492;
        latitude = setLatitude(rawLatValue,available);
        TS_ASSERT_EQUALS(available,true);
        TS_ASSERT_DELTA(latitude,-27.372981666,relativeErrorAccepted);

        rawLatValue = 0x3412140;
        latitude = setLatitude(rawLatValue,available);
        TS_ASSERT_EQUALS(available,false);
        TS_ASSERT_DELTA(latitude,91.0,relativeErrorAccepted);
    }

    void test_removeTrailingCharacters(void) {
        const char *strings[] = {
            "hello@@@"
            ,"@@world"
            ,"@@@@"
            ,""
        };
        char string[256];
        strcpy(string,strings[0]);
        char *end = string + strlen(string) - 1;
        removeTrailingCharacters(string,end);
        TS_ASSERT_EQUALS(string,"hello");

        strcpy(string,strings[1]);
        end = string + strlen(string) - 1;
        removeTrailingCharacters(string,end);
        TS_ASSERT_EQUALS(string,strings[1]);

        strcpy(string,strings[2]);
        end = string + strlen(string) - 1;
        removeTrailingCharacters(string,end);
        TS_ASSERT_EQUALS(string,"");

        strcpy(string,strings[3]);
        end = string + strlen(string) - 1;
        removeTrailingCharacters(string,end);
        TS_ASSERT_EQUALS(string,"");
    }

    void test_removeLeadingCharacters(void) {
        const char *strings[] = {
            "@@@@@@@@@AS,GREECE"
            ,"hello@@@"
            ,"@@@@"
            ,""
        };
        char string[256];
        strcpy(string,strings[0]);
        removeLeadingCharacters(string);
        TS_ASSERT_EQUALS(string,"AS,GREECE");

        strcpy(string,strings[1]);
        removeLeadingCharacters(string);
        TS_ASSERT_EQUALS(string,strings[1]);

        strcpy(string,strings[2]);
        removeLeadingCharacters(string);
        TS_ASSERT_EQUALS(string,"");

        strcpy(string,strings[3]);
        removeLeadingCharacters(string);
        TS_ASSERT_EQUALS(string,"");
    }

    void test_string_filters(void) {
        const char *strings[] = {
            "@@@@@@@@@Hello World@@@@@"
            ,"hello"
            ,"@@@@"
            ,""
        };
        char string[256];
        strcpy(string,strings[0]);
        char *end = string + strlen(string) - 1;
        removeTrailingCharacters(string,end);
        removeLeadingCharacters(string);
        TS_ASSERT_EQUALS(string,"Hello World");

        strcpy(string,strings[1]);
        end = string + strlen(string) - 1;
        removeTrailingCharacters(string,end);
        removeLeadingCharacters(string);
        TS_ASSERT_EQUALS(string,strings[1]);

        strcpy(string,strings[2]);
        end = string + strlen(string) - 1;
        removeTrailingCharacters(string,end);
        removeLeadingCharacters(string);
        TS_ASSERT_EQUALS(string,"");

        strcpy(string,strings[3]);
        end = string + strlen(string) - 1;
        removeTrailingCharacters(string,end);
        removeLeadingCharacters(string);
        TS_ASSERT_EQUALS(string,"");
    }
};

#endif /* BASICFUNCTIONS_H_ */
