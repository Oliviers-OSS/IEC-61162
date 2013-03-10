/*
 * AISMessage22.h
 * Channel management
 * cf. R-REC-M.1371-4-201004-E p 123-127
 *  Created on: 9 avr. 2013
 *      Author: oc
 */

#ifndef _AIS_MESSAGE_22_H_
#define _AIS_MESSAGE_22_H_

#include <bitset>
#include <IEC61162/basicsmartpointer.h>
#include <IEC61162/AISDataTypes.h>

namespace IEC61162 {

    class AISMessage22;
    typedef basic_smart_pointer<AISMessage22>    AISMsg22;

    class AISMessage22 {
        enum fields {
            f_repeatIndicator = 0
            ,f_stationID
            ,f_channelA
            ,f_channelB
            ,f_txRxMode
            ,f_power
            ,f_longitude1
            ,f_latitude1
            ,f_addressedStationID1
            ,f_longitude2
            ,f_latitude2
            ,f_addressedStationID2
            ,f_addressedOrBroadcastMsgIndicator
            ,f_channelABandwith
            ,f_channelBBandwith
            ,f_transitionalZoneSize
            ,f_last
        };
        std::bitset<f_last> available;
        Byte repeatIndicator; //valid values are 0..3, number of time this msg has been repeated, 3 means do not repeat any more, default 0.
        MMSI stationID;
        Word channelA;
        Word channelB;
        Byte txRxMode;
        bool power;
        float longitude1;
        float latitude1;
        MMSI addressedStationID1;
        float longitude2;
        float latitude2;
        MMSI addressedStationID2;
        bool addressedOrBroadcastMsgIndicator;
        bool channelABandwith;
        bool channelBBandwith;
        Byte transitionalZoneSize;
    public:

        AISMessage22()
            :available(0x0)
            ,repeatIndicator(0)
            ,stationID(0)
            ,channelA(0)
            ,channelB(0)
            ,txRxMode(0)
            ,power(0)
            ,longitude1(181.0)
            ,latitude1(91.0)
            ,addressedStationID1(0)
            ,longitude2(181.0)
            ,latitude2(91.0)
            ,addressedStationID2(0)
            ,addressedOrBroadcastMsgIndicator(0)
            ,channelABandwith(0)
            ,channelBBandwith(0)
            ,transitionalZoneSize(5) {
        }

        ~AISMessage22() {
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
        bool has_sourceID() {
            return available[f_stationID];
        }

        /**
         * get_userID
         * @return
         */
        MMSI get_stationID() {
            return stationID;
        }

        bool has_channelA() {
            return available[f_channelA];
        }

        Word get_channelA() {
            return channelA;
        }

        bool has_channelB() {
            return available[f_channelB];
        }

        Word get_channelB() {
            return channelB;
        }
        bool has_txRxMode() {
            return available[f_txRxMode];
        }
        Byte get_txRxMode() {
            return txRxMode;
        }

        bool has_power() {
            return available[f_power];
        }

        bool get_power() {
            return power;
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

        bool has_addressedStationID1() {
            return available[f_addressedStationID1];
        }

        MMSI get_addressedStationID1() {
            return addressedStationID1;
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

        bool has_addressedStationID2() {
            return available[f_addressedStationID2];
        }

        MMSI get_addressedStationID2() {
            return addressedStationID2;
        }

        bool has_addressedOrBroadcastMsgIndicator() {
            return addressedOrBroadcastMsgIndicator;
        }

        bool get_addressedOrBroadcastMsgIndicator() {
            return addressedOrBroadcastMsgIndicator;
        }

        bool has_channelABandwith() {
            return available[f_channelABandwith];
        }

        bool get_channelABandwith() {
            return channelABandwith;
        }

        bool has_channelBBandwith() {
            return available[f_channelBBandwith];
        }

        bool get_channelBBandwith() {
            return channelBBandwith;
        }

        bool has_transitionalZoneSize() {
            return available[f_transitionalZoneSize];
        }

        Byte get_transitionalZoneSize() {
            return transitionalZoneSize;
        }

        /**
         * build
         * @param message's 8 bits payload
         * @param error code (EXIT_SUCCESS if none)
         * @return basic_smart_pointer object containing an AISMessage22 object if the error code is EXIT_SUCCESS
         */
        static AISMsg22 build(const unsigned char *payload,int &error);
    };

} //namespace IEC61162

#endif /* _AIS_MESSAGE_22_H_ */
