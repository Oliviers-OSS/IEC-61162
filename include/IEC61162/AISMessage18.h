/*
 * AISMessage18.h
 * Standard Class B equipment position report
 * R-REC-M.1371-4-201004-E p119
 *  Created on: 8 avr. 2013
 *      Author: oc
 */

#ifndef _AIS_MESSAGE_18_H_
#define _AIS_MESSAGE_18_H_

#include <bitset>
#include <IEC61162/basicsmartpointer.h>
#include <IEC61162/AISDataTypes.h>

namespace IEC61162 {

    class AISMessage18;
    typedef basic_smart_pointer<AISMessage18>    AISMsg18;

    class AISMessage18 {

        enum fields {
            f_repeatIndicator = 0
            ,f_userID
            ,f_speedOverGround
            ,f_positionAccuracy
            ,f_longitude
            ,f_latitude
            ,f_courseOverGround
            ,f_trueHeading
            ,f_timeStamp
            ,f_classBUnitFlag
            ,f_classBDisplayFlag
            ,f_classBDSCFlag
            ,f_classBBandFlag
            ,f_classBMessage22Flag
            ,f_modeFlag
            ,f_RAIMFlag
            ,f_communicationStateSelectorFlag
            ,f_communicationState
            ,f_last
        };
        std::bitset<f_last> available;
        unsigned char repeatIndicator; //valid values are 0..3, number of time this msg has been repeated, 3 means do not repeat any more, default 0.
        MMSI userID;
        float speedOverGround; //SOG
        bool positionAccuracy;
        float longitude;
        float latitude;
        float courseOverGround; //COG
        Word trueHeading;
        Byte timeStamp;
        bool classBUnitFlag;
        bool classBDisplayFlag;
        bool classBDSCFlag;
        bool classBBandFlag;
        bool classBMessage22Flag;
        bool modeFlag;
        bool RAIMFlag;
        bool communicationStateSelectorFlag;
        unsigned long communicationState;
    public:
        AISMessage18()
            :available(0x0)
            ,repeatIndicator(0)
            ,userID(0)
            ,speedOverGround(102.3)
            ,positionAccuracy(0)
            ,longitude(181.0)
            ,latitude(91.0)
            ,courseOverGround(360.0)
            ,trueHeading(511)
            ,timeStamp(60)
            ,classBUnitFlag(0)
            ,classBDisplayFlag(0)
            ,classBDSCFlag(0)
            ,classBBandFlag(0)
            ,classBMessage22Flag(0)
            ,modeFlag(0)
            ,RAIMFlag(0)
            ,communicationStateSelectorFlag(0)
            ,communicationState(0) {
        }

        ~AISMessage18() {

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

        bool has_classBUnitFlag() {
            return available[f_classBUnitFlag];
        }

        bool get_classBUnitFlag() {
            return classBUnitFlag;
        }

        bool has_classBDisplayFlag() {
            return available[f_classBDisplayFlag];
        }

        bool get_classBDisplayFlag() {
            return classBDisplayFlag;
        }

        bool has_classBDSCFlag() {
            return available[f_classBDSCFlag];
        }

        bool get_classBDSCFlag() {
            return classBDSCFlag;
        }

        bool has_classBBandFlag() {
            return available[f_classBBandFlag];
        }

        bool get_classBBandFlag() {
            return classBBandFlag;
        }

        bool has_classBMessage22Flag() {
            return available[f_classBMessage22Flag];
        }

        bool get_classBMessage22Flag() {
            return classBMessage22Flag;
        }

        bool has_modeFlag() {
            return available[f_modeFlag];
        }

        bool get_modeFlag() {
            return modeFlag;
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

         bool has_communicationStateSelectorFlag() {
             return available[f_communicationStateSelectorFlag];
         }

         bool get_communicationStateSelectorFlag() {
             return communicationStateSelectorFlag;
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
          * @return basic_smart_pointer object containing an AISMessage18 object if the error code is EXIT_SUCCESS
          */
         static AISMsg18 build(const unsigned char *payload,int &error);
    };
} //namespace IEC61162

#endif /* _AIS_MESSAGE_18_H_ */
