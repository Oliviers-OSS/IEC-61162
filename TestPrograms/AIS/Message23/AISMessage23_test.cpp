/*
 * AISMessage23_test.cpp
 *
 *  Created on: 11 jun. 2013
 *      Author: oc
 */



#include "AISMessage23_test.h"
#include <cerrno>
#include <cstdio>
#include <cstring>

bool operator ==(const IEC61162::ReferencePoint &r1,const IEC61162::ReferencePoint &r2) {
    return ((r1.A == r2.A) && (r1.B == r2.B) && (r1.C == r2.C) && (r1.D == r2.D));
}

bool operator ==(const IEC61162::EstimatedTimeOfArrival &e1,const IEC61162::EstimatedTimeOfArrival &e2) {
    return ((e1.month == e2.month) && (e1.day == e2.day)&& (e1.hour == e2.hour) && (e1.minute == e2.minute));
}

void msgHandler(IEC61162::AISMsg23 msg,void *param) {
    AISMsg23_Tests *caller = static_cast<AISMsg23_Tests *>(param);
    if (caller) {
        // check the decoded message's content against the references data
        const struct AISMsg23_Data *test_data = caller->currentMsg;

        TS_ASSERT_EQUALS(msg->get_repeatIndicator(),test_data->repeatIndicator);
        TS_ASSERT_EQUALS(msg->get_stationID(),test_data->stationID);
        TS_ASSERT_EQUALS(msg->get_longitude1(),test_data->longitude1);
        TS_ASSERT_EQUALS(msg->get_latitude1(),test_data->latitude1);
        TS_ASSERT_EQUALS(msg->get_longitude2(),test_data->longitude2);
        TS_ASSERT_EQUALS(msg->get_latitude2(),test_data->latitude2);
        TS_ASSERT_EQUALS(msg->get_stationType(),test_data->stationType);
        TS_ASSERT_EQUALS(msg->get_typeOfShipAndCargoType(),test_data->typeOfShipAndCargoType);
        TS_ASSERT_EQUALS(msg->get_txRxMode(),test_data->txRxMode);
        TS_ASSERT_EQUALS(msg->get_reportingInterval(),test_data->reportingInterval);
        TS_ASSERT_EQUALS(msg->get_quietTime(),test_data->quietTime);

    } else {
        TS_FAIL("caller parameter is NULL");
    }
}


