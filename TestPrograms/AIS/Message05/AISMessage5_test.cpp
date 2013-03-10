/*
 * AISMessage5_test.cpp
 *
 *  Created on: 12 avr. 2013
 *      Author: oc
 */



#include "AISMessage5_test.h"
#include <cerrno>
#include <cstdio>
#include <cstring>

bool operator ==(const IEC61162::ReferencePoint &r1,const IEC61162::ReferencePoint &r2) {
    return ((r1.A == r2.A) && (r1.B == r2.B) && (r1.C == r2.C) && (r1.D == r2.D));
}

bool operator ==(const IEC61162::EstimatedTimeOfArrival &e1,const IEC61162::EstimatedTimeOfArrival &e2) {
    return ((e1.month == e2.month) && (e1.day == e2.day)&& (e1.hour == e2.hour) && (e1.minute == e2.minute));
}

void msgHandler(IEC61162::AISMsg5 msg,void *param) {
    AISMsg5_Tests *caller = static_cast<AISMsg5_Tests *>(param);
    if (caller) {
        // check the decoded message's content against the references data
        const struct AISMsg5_Data *test_data = caller->currentMsg;
        char buffer[255];
        sprintf(buffer,"checking message %s...\n",test_data->rawMsg);
        TS_TRACE(buffer);
        TS_ASSERT_EQUALS(msg->get_repeatIndicator(),test_data->repeatIndicator);
        TS_ASSERT_EQUALS(msg->get_userID(),test_data->userID);
        TS_ASSERT_EQUALS(msg->get_AISVersionIndicator(),test_data->AISVersionIndicator);
        TS_ASSERT_EQUALS(msg->get_IMONumber(),test_data->IMONumber);
        const char *callSign = msg->get_callSign();
        TS_ASSERT(strncmp(callSign,test_data->callSign,strlen(callSign)) == 0);
        const char *name = msg->get_name();
        TS_ASSERT(strncmp(name,test_data->name,strlen(name)) == 0);
        TS_ASSERT_EQUALS(msg->get_typeOfShipAndCargoType(),test_data->typeOfShipAndCargoType);
        TS_ASSERT_EQUALS(msg->get_overallDimensionReferenceForPosition(),test_data->overallDimensionReferenceForPosition);
        TS_ASSERT_EQUALS(msg->get_typeOfPositionDevice(),test_data->typeOfPositionDevice);
        TS_ASSERT_EQUALS(msg->get_ETA(),test_data->ETA);
        TS_ASSERT_EQUALS(msg->get_maximumPresentStaticDraught(),test_data->maximumPresentStaticDraught);
        const char *destination = msg->get_destination();
        TS_ASSERT(strncmp(destination,test_data->destination,strlen(destination)) == 0);
        TS_ASSERT_EQUALS(msg->get_DTE(),test_data->DTE);
    } else {
        TS_FAIL("caller parameter is NULL");
    }
}


