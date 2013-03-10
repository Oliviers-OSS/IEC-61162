/*
 * AISMessage27.h
 *
 *  Created on: 1 avr. 2013
 *      Author: oc
 */

#ifndef _AIS_MESSAGE_27_H_
#define _AIS_MESSAGE_27_H_

#include <bitset>
#include <IEC61162/basicsmartpointer.h>
#include <IEC61162/AISDataTypes.h>
#include <cstring>
#include <cmath>

namespace IEC61162 {
    class AISMessage27;
    typedef basic_smart_pointer<AISMessage27>    AISMsg27;

    class AISMessage27 {
        enum fields {
             f_repeatIndicator = 0
            ,f_userID
            ,f_positionAccuracy
            ,f_RAIMFlag
            ,f_navigationStatus
            ,f_longitude
            ,f_latitude
            ,f_speedOverGround
            ,f_courseOverGround
            ,f_statusOfCurrentGNSSPosition
            ,f_last
        };
        std::bitset<f_last> available;
        unsigned char repeatIndicator; //valid values are 0..3, number of time this msg has been repeated, 3 means do not repeat any more, default 0.
        MMSI userID;
        bool positionAccuracy;
        bool RAIMFlag;
        Byte navigationStatus;
        float longitude;
        float latitude;
        float speedOverGround; //SOG
        float courseOverGround; //COG
        bool statusOfCurrentGNSSPosition;
    public:
        AISMessage27()
            :available(0x0)
            ,repeatIndicator(0)
            ,userID(0)
            ,positionAccuracy(0)
            ,RAIMFlag(false)
            ,navigationStatus(15)
            ,longitude(NAN)
            ,latitude(NAN)
            ,speedOverGround(63)
            ,courseOverGround(511)
            ,statusOfCurrentGNSSPosition(1) {
        }

        ~AISMessage27() {
        }

        bool has_repeatIndicator() {
            return available[f_repeatIndicator];
        }

        unsigned char get_repeatIndicator() {
            return repeatIndicator;
        }

        bool has_userID() {
            return available[f_userID];
        }

        MMSI get_userID() {
            return userID;
        }

        bool has_positionAccuracy() {
            return available[f_positionAccuracy];
        }

        bool get_positionAccuracy() {
            return positionAccuracy;
        }

        bool has_RAIMFlag() {
            return available[f_RAIMFlag];
        }

        bool get_RAIMFlag() {
            return RAIMFlag;
        }

        bool has_navigationStatus() {
            return available[f_navigationStatus];
        }

        Byte get_navigationStatus() {
            return navigationStatus;
        }

        bool has_longitude() {
            return available[f_longitude];
        }

        float get_longitude() {
            return longitude;
        }

        bool has_latitude() {
            return available[f_latitude];
        }

        float get_latitude() {
            return latitude;
        }

        bool has_speedOverGround() {
            return available[f_speedOverGround];
        }

        float get_speedOverGround() {
            return speedOverGround;
        }

        bool has_courseOverGround() {
            return available[f_courseOverGround];
        }

        float get_courseOverGround() {
            return courseOverGround;
        }

        bool has_statusOfCurrentGNSSPosition() {
            return available[f_statusOfCurrentGNSSPosition];
        }

        bool get_statusOfCurrentGNSSPosition() {
            return statusOfCurrentGNSSPosition;
        }

        /**
         * build
         * @param message's 8 bits payload
         * @param error code (EXIT_SUCCESS if none)
         * @return basic_smart_pointer object containing an AISMessage27 object if the error code is EXIT_SUCCESS
         */
        static AISMsg27 build(const unsigned char *payload,int &error);
    };

} // namespace IEC61162


#endif /* _AIS_MESSAGE_27_H_ */
