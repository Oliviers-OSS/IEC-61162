/*
 * AISMessage24_test.cpp
 *
 *  Created on: 12 avr. 2013
 *      Author: oc
 */



#include "AISMessage24_test.h"
#include <cerrno>
#include <cstdio>
#include <cstring>

bool operator ==(const IEC61162::ReferencePoint &r1,const IEC61162::ReferencePoint &r2) {
    return ((r1.A == r2.A) && (r1.B == r2.B) && (r1.C == r2.C) && (r1.D == r2.D));
}

bool operator ==(const IEC61162::EstimatedTimeOfArrival &e1,const IEC61162::EstimatedTimeOfArrival &e2) {
    return ((e1.month == e2.month) && (e1.day == e2.day)&& (e1.hour == e2.hour) && (e1.minute == e2.minute));
}

void msgHandlerA(IEC61162::AISMsg24A msg,void *param) {
    AISMsg24_Tests *caller = static_cast<AISMsg24_Tests *>(param);
    if (caller) {
        const struct AISMsg24_Data *test_data = caller->currentMsg;
        char buffer[255];
        sprintf(buffer,"checking message 24A %s...\n",test_data->rawMsg);
        TS_TRACE(buffer);

        TS_ASSERT_EQUALS(msg->get_repeatIndicator(),test_data->repeatIndicator);
        TS_ASSERT_EQUALS(msg->get_userID(),test_data->userID);
        TS_ASSERT_EQUALS(msg->get_partNumber(),test_data->partNumber);

        const char *name = msg->get_name();
        TS_ASSERT(strncmp(name,test_data->name,strlen(name)) == 0);
    } else {
        TS_FAIL("caller parameter is NULL");
    }
}

void msgHandlerB(IEC61162::AISMsg24B msg,void *param) {
    AISMsg24_Tests *caller = static_cast<AISMsg24_Tests *>(param);
    if (caller) {
        const struct AISMsg24_Data *test_data = caller->currentMsg;
        char buffer[255];
        sprintf(buffer,"checking message 24B %s...\n",test_data->rawMsg);
        TS_TRACE(buffer);

        TS_ASSERT_EQUALS(msg->get_repeatIndicator(),test_data->repeatIndicator);
        TS_ASSERT_EQUALS(msg->get_userID(),test_data->userID);
        TS_ASSERT_EQUALS(msg->get_partNumber(),test_data->partNumber);


        TS_ASSERT_EQUALS(msg->get_typeOfShipAndCargoType(),test_data->typeOfShipAndCargoType);
        //IEC61162::VendorID vendorID;
        const char *callSign = msg->get_callSign();
        TS_ASSERT(strncmp(callSign,test_data->callSign,strlen(callSign)) == 0);

        TS_ASSERT_EQUALS(msg->get_dimension(),test_data->dimension);
        //IEC61162::MMSI motherShip;
    } else {
        TS_FAIL("caller parameter is NULL");
    }
}

void msgHandler(IEC61162::AISMsg24 msg,void *param) {
    AISMsg24_Tests *caller = static_cast<AISMsg24_Tests *>(param);
    if (caller) {
        // check the decoded message's content against the references data
        const struct AISMsg24_Data *test_data = caller->currentMsg;
        char buffer[255];
        sprintf(buffer,"checking message 24 %s...\n",test_data->rawMsg);
        TS_TRACE(buffer);

        TS_ASSERT_EQUALS(msg->get_repeatIndicator(),test_data->repeatIndicator);
        TS_ASSERT_EQUALS(msg->get_userID(),test_data->userID);
        TS_ASSERT_EQUALS(msg->get_partNumber(),test_data->partNumber);

        if (msg->is_AISMessage24A()) {
            IEC61162::AISMsg24A msgA = msg.down_cast<IEC61162::AISMessage24A>();
            if (!msgA.isNull()) {
                msgHandlerA(msgA,param);
            }
        }
        if (msg->is_AISMessage24B()) {
            IEC61162::AISMsg24B msgB = msg.down_cast<IEC61162::AISMessage24B>();
            if (!msgB.isNull()) {
                msgHandlerB(msgB,param);
            }
        }
    } else {
        TS_FAIL("caller parameter is NULL");
    }
}


