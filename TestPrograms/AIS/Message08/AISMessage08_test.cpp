/*
 * AISMessage20_test.cpp
 *
 *  Created on: 12 avr. 2013
 *      Author: oc
 */



#include "AISMessage08_test.h"
#include <cerrno>
#include <cstdio>
#include <cstring>

bool operator ==(const IEC61162::ReferencePoint &r1,const IEC61162::ReferencePoint &r2) {
    return ((r1.A == r2.A) && (r1.B == r2.B) && (r1.C == r2.C) && (r1.D == r2.D));
}

bool operator ==(const IEC61162::EstimatedTimeOfArrival &e1,const IEC61162::EstimatedTimeOfArrival &e2) {
    return ((e1.month == e2.month) && (e1.day == e2.day)&& (e1.hour == e2.hour) && (e1.minute == e2.minute));
}

void msgHandler(IEC61162::AISMsg8 msg,void *param) {
    AISMsg08_Tests *caller = static_cast<AISMsg08_Tests *>(param);
    if (caller) {
        // check the decoded message's content against the references data
        const struct AISMsg08_Data *test_data = caller->currentMsg;

        TS_ASSERT_EQUALS(msg->get_repeatIndicator(),test_data->repeatIndicator);
        TS_ASSERT_EQUALS(msg->get_sourceID(),test_data->sourceID);
        const IEC61162::BinaryData *binData = msg->get_binaryData();
        TS_ASSERT_EQUALS(binData->applicationIdentifier.id,test_data->applicationID);

    } else {
        TS_FAIL("caller parameter is NULL");
    }
}


