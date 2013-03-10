/*
 * AISMessage1.h
 *  Position reports, cf. R-REC-M.1371-4-201004-E p101
 *  Created on: 9 mars 2013
 *      Author: oc
 */

#ifndef _AIS_MESSAGE_1_H_
#define _AIS_MESSAGE_1_H_

#include <bitset>
#include <IEC61162/basicsmartpointer.h>
#include <IEC61162/AISDataTypes.h>

namespace IEC61162 {

    class AISMessage1;
    typedef basic_smart_pointer<AISMessage1>    AISMsg1;

    class AISMessage1 {

        enum fields {
             f_repeatIndicator = 0
            ,f_userID
            ,f_navigationStatus
            ,f_rateOfTurn
            ,f_speedOverGround
            ,f_positionAccuracy
            ,f_longitude
            ,f_latitude
            ,f_courseOverGround
            ,f_trueHeading
            ,f_timeStamp
            ,f_specialManoeuvreIndicator
            ,f_RAIMFlag
            ,f_communicationState
            ,f_last
        };
        std::bitset<f_last> available;

        unsigned char repeatIndicator; //valid values are 0..3, number of time this msg has been repeated, 3 means do not repeat any more, default 0.
        MMSI userID;
        Byte navigationStatus;
        float rateOfTurn; // ROT
        float speedOverGround; //SOG
        bool positionAccuracy;
        float longitude;
        float latitude;
        float courseOverGround; //COG
        unsigned short trueHeading;
        unsigned char timeStamp;
        unsigned char specialManoeuvreIndicator;
        bool RAIMFlag;
        unsigned long communicationState;

    public:

        AISMessage1()
            :available(0x0)
            ,repeatIndicator(0)
            ,userID(0)
            ,navigationStatus(15)
            ,rateOfTurn(0x80)
            ,speedOverGround(102.3)
            ,positionAccuracy(false)
            ,longitude(181.0)
            ,latitude(91.0)
            ,courseOverGround(360.0)
            ,trueHeading(511)
            ,timeStamp(60)
            ,specialManoeuvreIndicator(0x0)
            ,RAIMFlag(0)
            ,communicationState(0x0) {
        }

        ~AISMessage1() {

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
         * has_navigationStatus
         * @return
         */
        bool has_navigationStatus() {
            return available[f_navigationStatus];
        }

        /**
         * get_navigationStatus
         * @return
         */
        enum NavigationStatus get_navigationStatus() {
            return static_cast<NavigationStatus>(navigationStatus);
        }

        /**
         * has_rateOfTurn
         * @return true if the repeatIndicator field is set
         */
        bool has_rateOfTurn() {
            return available[f_rateOfTurn];
        }

        /**
         * get_rateOfTurn
         * @return rate of turn in degrees per minute
         *  -positives values mean turning right
         *  -negatives values mean turning left
         *  +127 = turning right at more than 5o per 30 s (No TI available)
         *  –127 = turning left at more than 5o per 30 s (No TI available)
         *  –128 (80 hex) indicates no turn information available (default).
         */
        float get_rateOfTurn() {
            return rateOfTurn;
        }

        /**
         * has_speedOverGround
         * @return true if the speedOverGround field is set
         */
        bool has_speedOverGround(){
            return available[f_speedOverGround];
        }

        /**
         * get_speedOverGround
         * @return speed over ground in knot(s)
         * @warning not a mandatory field, please use the has_speedOverGround method to known if set or not
         * - 102.2 means 102.2 knots or higher
         * - 102.3 means not available
         */
        float get_speedOverGround() {
            return speedOverGround;
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
         * has_courseOverGround
         * @return true if the courseOverGround is set
         */
        bool has_courseOverGround() {
            return available[f_courseOverGround];
        }

        /**
         * get_courseOverGround
         * @return course over ground in degrees (0-359.9)
         * 360 means not available
         */
        float get_courseOverGround() {
            return courseOverGround;
        }

        /**
         * has_trueHeading
         * @return true if the trueHeading is set
         */
        bool has_trueHeading() {
            return available[f_trueHeading];
        }

        /**
         * get_trueHeading
         * @return true Heading in degrees (0-359)
         * 511 means not set;
         */
        unsigned short get_trueHeading() {
            return trueHeading;
        }

        /**
         * has_timeStamp
         * @return true if the timeStamp is set
         */
        bool has_timeStamp(){
            return available[f_timeStamp];
        }

        /**
         * get_timeStamp
         * @return UTC second when the report was generated by the electronic position
         * system (EPFS) (0-59, or 60 if time stamp is not available, which
         * should also be the default value, or 61 if positioning system is in
         * manual input mode, or 62 if electronic position fixing system
         * operates in estimated (dead reckoning) mode, or 63 if the positioning
         * system is inoperative)
         */
        unsigned char get_timeStamp() {
            return timeStamp;
        }

        /**
         * has_specialManoeuvreIndicator
         * @return true if the specialManoeuvreIndicator is set
         */
        bool has_specialManoeuvreIndicator(){
            return available[f_specialManoeuvreIndicator];
        }

        /**
         * get_specialManoeuvreIndicator
         * @return 0 = not available = default
         * 1 = not engaged in special manoeuvre
         * 2 = engaged in special manoeuvre
         * (i.e.: regional passing arrangement on Inland Waterway)
         * @warning was previously part of the reserved For Regional Application field.
         */
        unsigned char get_specialManoeuvreIndicator() {
            return specialManoeuvreIndicator;
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
         * @param payload message's payload
         * @param error error code (EXIT_SUCCESS if none)
         * @return basic_smart_pointer object containing an AISMessage1 object if the error code is EXIT_SUCCESS
         */
        static AISMsg1 build(const unsigned char *payload,int &error);
    };
} // namespace IEC61162

#endif /* _AIS_MESSAGE_1_H_ */
