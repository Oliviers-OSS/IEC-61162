/*
 * AISMessage23.h
 * Group Assignment Command
 * cf. R-REC-M.1371-4-201004-E p 123-127
 *  Created on: 9 avr. 2013
 *      Author: oc
 */

#ifndef _AIS_MESSAGE_23_H_
#define _AIS_MESSAGE_23_H_

#include <bitset>
#include <IEC61162/basicsmartpointer.h>
#include <IEC61162/AISDataTypes.h>

namespace IEC61162 {

    class AISMessage23;
    typedef basic_smart_pointer<AISMessage23>    AISMsg23;

    class AISMessage23 {
        enum fields {
            f_repeatIndicator = 0
            ,f_stationID
            ,f_longitude1
            ,f_latitude1
            ,f_longitude2
            ,f_latitude2
            ,f_stationType
            ,f_typeOfShipAndCargoType
            ,f_txRxMode
            ,f_reportingInterval
            ,f_quietTime
            ,f_last
        };
        std::bitset<f_last> available;
        Byte repeatIndicator; //valid values are 0..3, number of time this msg has been repeated, 3 means do not repeat any more, default 0.
        MMSI stationID;
        float longitude1;
        float latitude1;
        float longitude2;
        float latitude2;
        Byte stationType;
        Byte typeOfShipAndCargoType;
        Byte txRxMode;
        Byte reportingInterval;
        Byte quietTime;
    public:

        AISMessage23()
            :available(0x0)
            ,repeatIndicator(0)
            ,stationID(0)
            ,longitude1(181.0)
            ,latitude1(91.0)
            ,longitude2(181.0)
            ,latitude2(91.0)
            ,stationType(0)
            ,typeOfShipAndCargoType(0)
            ,txRxMode(0)
            ,reportingInterval(0)
            ,quietTime(0) {
        }

        ~AISMessage23() {
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
        bool has_stationID() {
            return available[f_stationID];
        }

        /**
         * get_userID
         * @return
         */
        MMSI get_stationID() {
            return stationID;
        }

        bool has_longitude1() {
            return available[f_longitude1];
        }

        float get_longitude1() {
            return longitude1;
        }

        bool has_latitude1() {
            return available[f_latitude1];
        }

        float get_latitude1() {
            return latitude1;
        }

        bool has_longitude2() {
            return available[f_longitude2];
        }

        float get_longitude2() {
            return longitude2;
        }

        bool has_latitude2() {
            return available[f_latitude2];
        }

        float get_latitude2() {
            return latitude2;
        }

        bool has_stationType() {
            return available[f_stationType];
        }

        Byte get_stationType() {
            return stationType;
        }

        bool has_typeOfShipAndCargoType() {
            return available[f_typeOfShipAndCargoType];
        }

        Byte get_typeOfShipAndCargoType() {
            return typeOfShipAndCargoType;
        }

        bool has_txRxMode() {
            return available[f_txRxMode];
        }
        Byte get_txRxMode() {
            return txRxMode;
        }

        bool has_reportingInterval() {
            return available[f_reportingInterval];
        }

        Byte get_reportingInterval() {
            return reportingInterval;
        }

        bool has_quietTime() {
            return available[f_quietTime];
        }

        Byte get_quietTime() {
            return quietTime;
        }

        /**
         * build
         * @param message's 8 bits payload
         * @param error code (EXIT_SUCCESS if none)
         * @return basic_smart_pointer object containing an AISMessage23 object if the error code is EXIT_SUCCESS
         */
        static AISMsg23 build(const unsigned char *payload,int &error);
    };

} //namespace IEC61162

#endif /* _AIS_MESSAGE_23_H_ */
