/*
 * AISMessage25_test.cpp
 *
 *  Created on: 11 jun. 2513
 *      Author: oc
 */



#include "AISMessage25_test.h"
#include <cerrno>
#include <cstdio>
#include <cstring>

bool operator ==(const IEC61162::ReferencePoint &r1,const IEC61162::ReferencePoint &r2) {
    return ((r1.A == r2.A) && (r1.B == r2.B) && (r1.C == r2.C) && (r1.D == r2.D));
}

bool operator ==(const IEC61162::EstimatedTimeOfArrival &e1,const IEC61162::EstimatedTimeOfArrival &e2) {
    return ((e1.month == e2.month) && (e1.day == e2.day)&& (e1.hour == e2.hour) && (e1.minute == e2.minute));
}

void msgHandler(IEC61162::AISMsg25 msg,void *param) {
    AISMsg25_Tests *caller = static_cast<AISMsg25_Tests *>(param);
    if (caller) {
        // check the decoded message's content against the references data
        const struct AISMsg25_Data *test_data = caller->currentMsg;

        TS_ASSERT_EQUALS(msg->get_repeatIndicator(),test_data->repeatIndicator);
        TS_ASSERT_EQUALS(msg->get_sourceID(),test_data->sourceID);
        TS_ASSERT_EQUALS(msg->get_destinationIndicator(),test_data->destinationIndicator);
        TS_ASSERT_EQUALS(msg->get_binaryDataFlag(),test_data->binaryDataFlag);
        TS_ASSERT_EQUALS(msg->get_destinationID(),test_data->destinationID);
        TS_ASSERT_EQUALS(msg->get_applicationIdentifier(),test_data->applicationIdentifier);
        TS_ASSERT_EQUALS(msg->get_binaryDataSize(),test_data->binaryDataSize);
        //const IEC61162::BinaryData *binData = msg->get_binaryData();
        //IEC61162::Byte binaryData[128/8];
    } else {
        TS_FAIL("caller parameter is NULL");
    }
}


