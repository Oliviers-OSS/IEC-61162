/*
 * AISMessage11.h
 * Base station report
 *  Created on: 1 avr. 2013
 *      Author: oc
 */

#ifndef _AIS_MESSAGE_11_H_
#define _AIS_MESSAGE_11_H_

#include <bitset>
#include <IEC61162/basicsmartpointer.h>
#include <IEC61162/AISDataTypes.h>

namespace IEC61162 {

    class AISMessage11;
    typedef basic_smart_pointer<AISMessage11>    AISMsg11;

    class AISMessage11 {
        enum fields {
             f_repeatIndicator = 0
            ,f_userID
            ,f_UTC_year
            ,f_UTC_month
            ,f_UTC_day
            ,f_UTC_hour
            ,f_UTC_minute
            ,f_UTC_second
            ,f_positionAccuracy
            ,f_longitude
            ,f_latitude
            ,f_typeOfPositionDevice
            ,f_transMissionControlForLongRangeBroadcastMessage
            ,f_RAIMFlag
            ,f_communicationState
            ,f_last
        };
        std::bitset<f_last> available;
        unsigned char repeatIndicator; //valid values are 0..3, number of time this msg has been repeated, 3 means do not repeat any more, default 0.
        MMSI userID;
        unsigned short UTC_year;
        unsigned char UTC_month;
        unsigned char UTC_day;
        unsigned char UTC_hour;
        unsigned char UTC_minute;
        unsigned char UTC_second;
        bool positionAccuracy;
        float longitude;
        float latitude;
        unsigned char typeOfPositionDevice;
        bool transMissionControlForLongRangeBroadcastMessage;
        bool RAIMFlag;
        unsigned long communicationState;
    public:
        AISMessage11()
            :available(0x0)
            ,repeatIndicator(0)
            ,userID(0)
            ,UTC_year(0)
            ,UTC_month(0)
            ,UTC_day(0)
            ,UTC_hour(24)
            ,UTC_minute(60)
            ,UTC_second(60)
            ,positionAccuracy(false)
            ,longitude(181.0)
            ,latitude(91.0)
            ,typeOfPositionDevice(0)
            ,transMissionControlForLongRangeBroadcastMessage(0)
            ,RAIMFlag(0)
            ,communicationState(0x0) {

        }

        ~AISMessage11() {
        }

        /**
         * has_repeatIndicator
         * @return true if the repeatIndicator field is set
         */
        bool has_repeatIndicator() {
            return available[f_repeatIndicator];
        }

        /**
         * get_repeatIndicator
         * @return return the repeatIndicator value
         */
        unsigned char get_repeatIndicator() {
            return repeatIndicator;
        }

        /**
         * has_userID
         * @return
         */
        bool has_userID() {
            return available[f_userID];
        }

        /**
         * get_userID
         * @return
         */
        MMSI get_userID() {
            return userID;
        }

        /**
         * has_UTC_year
         * @return true if the UTC year field is set
         */
        bool has_UTC_year() {
            return available[f_UTC_year];
        }

        /**
         *
         */
        unsigned short get_UTC_year() {
            return UTC_year;
        }

        bool has_UTC_month() {
            return available[f_UTC_month];
        }

        unsigned char get_UTC_month() {
            return UTC_month;
        }

        bool has_UTC_day() {
            return available[f_UTC_day];
        }

        unsigned char get_UTC_day() {
           return UTC_day;
        }

        bool has_UTC_hour() {
             return available[f_UTC_hour];
        }

        unsigned char get_UTC_hour() {
           return UTC_hour;
        }

        bool has_UTC_minute() {
             return available[f_UTC_minute];
        }

        unsigned char get_UTC_minute() {
           return UTC_minute;
        }

        bool has_UTC_second() {
           return available[f_UTC_second];
        }

        unsigned char get_UTC_second() {
           return UTC_second;
        }

        /**
         * has_positionAccuracy
         * @return true if the positionAccuracy is set
         */
        bool has_positionAccuracy(){
            return available[f_positionAccuracy];
        }

        /**
         * get_positionAccuracy
         * @return is set means high (≤ 10 m)
         * else low (>10 m)
         *
         */
        bool get_positionAccuracy() {
            return positionAccuracy;
        }

        /**
         * has_longitude
         * @return true if the longitude is set
         */
        bool has_longitude() {
            return available[f_longitude];
        }

        /**
         * get_longitude
         * @return longitude in degrees
         * East = positive
         * West = negative
         * 181 means not available
         */
        float get_longitude() {
            return longitude;
        }

        /**
         * has_latitude
         * @return true if the has_latitude is set
         */
        bool has_latitude(){
            return available[f_latitude];
        }

        /**
         * get_latitude
         * @return latitude in degrees
         * North = positive
         * South = negative
         * 91 means not available
         */
        float get_latitude() {
            return  latitude;
        }

        /**
         *
         * @return
         */
        bool has_typeOfPositionDevice() {
            return available[f_typeOfPositionDevice];
        }

        /**
         *
         * @return
         */
        Byte get_typeOfPositionDevice() {
            return typeOfPositionDevice;
        }

        /**
         *
         * @return
         */
        bool has_transMissionControlForLongRangeBroadcastMessage() {
            return available[f_transMissionControlForLongRangeBroadcastMessage];
        }

        /**
         *
         * @return
         */
        bool get_transMissionControlForLongRangeBroadcastMessage() {
            return transMissionControlForLongRangeBroadcastMessage;
        }

        /**
         * has_RAIMFlag
         * @return true if the RAIM Flag is set
         */
        bool has_RAIMFlag() {
            return available[f_RAIMFlag];
        }

        /**
         * get_RAIMFlag
         * @return Receiver autonomous integrity monitoring (RAIM) flag of electronic
         * position fixing device; 0 = RAIM not in use = default; 1 = RAIM in
         * use. See R-REC-M.1371-4-201004-E p103 Table 47
         *
         */
        bool get_RAIMFlag() {
            return RAIMFlag;
        }

        /**
         * has_communicationState
         * @return true if the communicationState is set
         */
        bool has_communicationState(){
            return available[f_communicationState];
        }

        /**
         * get_communicationState
         * @return communication state data (SOTDMA / ITDMA)
         * See R-REC-M.1371-4-201004-E p103 Table 47
         */
        unsigned long get_communicationState () {
            return communicationState;
        }

        /**
         * build
         * @param message's payload
         * @param error code (EXIT_SUCCESS if none)
         * @return basic_smart_pointer object containing an AISMessage11 object if the error code is EXIT_SUCCESS
         */
        static AISMsg11 build(const unsigned char *payload,int &error);
    };
} //namespace IEC61162


#endif /* _AIS_MESSAGE_11_H_ */
