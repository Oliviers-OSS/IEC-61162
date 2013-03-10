/*
 * AISMessage19_test.cpp
 *
 *  Created on: 12 avr. 2013
 *      Author: oc
 */



#include "AISMessage19_test.h"
#include <cerrno>
#include <cstdio>
#include <cstring>

bool operator ==(const IEC61162::ReferencePoint &r1,const IEC61162::ReferencePoint &r2) {
    return ((r1.A == r2.A) && (r1.B == r2.B) && (r1.C == r2.C) && (r1.D == r2.D));
}

bool operator ==(const IEC61162::EstimatedTimeOfArrival &e1,const IEC61162::EstimatedTimeOfArrival &e2) {
    return ((e1.month == e2.month) && (e1.day == e2.day)&& (e1.hour == e2.hour) && (e1.minute == e2.minute));
}

void msgHandler(IEC61162::AISMsg19 msg,void *param) {
    AISMsg19_Tests *caller = static_cast<AISMsg19_Tests *>(param);
    if (caller) {
        // check the decoded message's content against the references data
        const struct AISMsg19_Data *test_data = caller->currentMsg;
        char buffer[255];
        sprintf(buffer,"checking message %s...\n",test_data->rawMsg);
        TS_TRACE(buffer);
        TS_ASSERT_EQUALS(msg->get_repeatIndicator(),test_data->repeatIndicator);
        TS_ASSERT_EQUALS(msg->get_userID(),test_data->userID);
        TS_ASSERT_EQUALS(msg->get_speedOverGround(),test_data->speedOverGround);
        TS_ASSERT_EQUALS(msg->get_positionAccuracy(),test_data->positionAccuracy);
        TS_ASSERT_EQUALS(msg->get_longitude(),test_data->longitude);
        TS_ASSERT_EQUALS(msg->get_latitude(),test_data->latitude);
        TS_ASSERT_EQUALS(msg->get_courseOverGround(),test_data->courseOverGround);
        TS_ASSERT_EQUALS(msg->get_trueHeading(),test_data->trueHeading);
        TS_ASSERT_EQUALS(msg->get_timeStamp(),test_data->timeStamp);
        TS_ASSERT_EQUALS(msg->get_name(),test_data->name);
        TS_ASSERT_EQUALS(msg->get_typeOfShipAndCargoType(),test_data->typeOfShipAndCargoType);
        TS_ASSERT_EQUALS(msg->get_dimensionOfShipReferenceForPosition(),test_data->dimensionOfShipReferenceForPosition);
        TS_ASSERT_EQUALS(msg->get_typeOfPositionDevice(),test_data->typeOfPositionDevice);
        TS_ASSERT_EQUALS(msg->get_RAIMFlag(),test_data->RAIMFlag);
        TS_ASSERT_EQUALS(msg->get_DTE(),test_data->DTE);
        TS_ASSERT_EQUALS(msg->get_assignedModeFlag(),test_data->assignedModeFlag);
    } else {
        TS_FAIL("caller parameter is NULL");
    }
}


