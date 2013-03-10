/*
 * AISAlarm.h
 *
 *  Created on: 7 juin 2013
 *      Author: oc
 */

#ifndef _AIS_ALARM_H_
#define _AIS_ALARM_H_

#include <IEC61162/basicsmartpointer.h>
#include <IEC61162/AISDataTypes.h>
#include <ctime>
#include <string>

namespace IEC61162 {
    class AISAlarm;
    typedef basic_smart_pointer<AISAlarm>    Alarm;

    class AISAlarm {
        time_t timeOfAlarmCondictionChange;
        Byte id;
        char condition;
        char state;
        std::string description;
    public:
        AISAlarm()
            :timeOfAlarmCondictionChange(0),id(0)
            ,condition('\0'),state('\0') {
        }

        ~AISAlarm(){
        }

        time_t get_timeOfAlarmCondictionChange() {
            return timeOfAlarmCondictionChange;
        }

        Byte get_id() {
            return id;
        }

        char get_condition() {
            return condition;
        }

        char get_state() {
            return state;
        }

        const char *get_description() {
            return description.c_str();
        }

        static Alarm build(const char *payload,int &error);
    };
}


#endif /* _AIS_ALARM_H_ */
