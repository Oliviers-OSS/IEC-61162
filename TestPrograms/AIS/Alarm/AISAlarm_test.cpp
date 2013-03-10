/*
 * AISAlarm_test.cpp
 *
 *  Created on: 07 jun. 2013
 *      Author: oc
 */



#include "AISAlarm_test.h"
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <ctime>

static time_t getTime(const unsigned char hour, const unsigned char minutes, const unsigned char seconds) {
    time_t currentTime(0);
    struct tm baseTime;
    time(&currentTime);
    if (gmtime_r(&currentTime,&baseTime) == NULL) {
        memset(&baseTime,0,sizeof(baseTime));
        ERROR_MSG("gmtime_r error");
    }
    baseTime.tm_hour = hour;
    baseTime.tm_min = minutes;
    baseTime.tm_sec = seconds;
    return mktime(&baseTime);
}

void msgHandler(IEC61162::Alarm msg,void *param) {
    AISAlarm_Test *caller = static_cast<AISAlarm_Test *>(param);
    if (caller) {
        // check the decoded message's content against the references data
        const struct AISAlarm_Data *test_data = caller->currentMsg;
        char buffer[255];
        sprintf(buffer,"checking message %s...\n",test_data->rawMsg);
        TS_TRACE(buffer);
        const time_t ref = getTime(test_data->hour,test_data->minutes,test_data->seconds);
        TS_ASSERT_EQUALS(msg->get_timeOfAlarmCondictionChange(),ref);
        TS_ASSERT_EQUALS(msg->get_id(),test_data->id);
        TS_ASSERT_EQUALS(msg->get_condition(),test_data->condition);
        TS_ASSERT_EQUALS(msg->get_state(),test_data->state);
        TS_ASSERT_EQUALS(msg->get_description(),test_data->description);
    } else {
        TS_FAIL("caller parameter is NULL");
    }
}


