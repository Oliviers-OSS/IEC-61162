/*
 * IEC61162Msg_Tests.cpp
 *
 *  Created on: 11 avr. 2013
 *      Author: oc
 */

#include "IEC61162Msg_Tests.h"
#include <cstring>

void proprietaryMsgHandler(IEC61162::raw_sentence msg,void *param) {
    AISMsg1_Tests *caller = static_cast<AISMsg1_Tests *>(param);
    if (caller) {
        if (msg->compare("$PGRMM,WGS 84*06"EOM) == 0) {
            caller->lastHandlerTestResult = EXIT_SUCCESS;
        } else {
            caller->lastHandlerTestResult = EXIT_FAILURE;
        }
    }
}
