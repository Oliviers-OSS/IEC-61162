/*
 * AISMessage15.h
 * Interrogation
 * R-REC-M.1371-4-201004-E p115
 *  Created on: 8 avr. 2013
 *      Author: oc
 */

#ifndef _AIS_MESSAGE_15_H_
#define _AIS_MESSAGE_15_H_

#include <bitset>
#include <IEC61162/basicsmartpointer.h>
#include <IEC61162/AISDataTypes.h>

namespace IEC61162 {

    class AISMessage15;
    typedef basic_smart_pointer<AISMessage15>    AISMsg15;

    class AISMessage15 {

        enum fields {
            f_repeatIndicator = 0
            ,f_sourceID
            ,f_destinationID1
            ,f_messageID1_1
            ,f_slotOffset1_1
            ,f_messageID1_2
            ,f_slotOffset1_2
            ,f_destinationID2
            ,f_messageID2_1
            ,f_slotOffset2_1
            ,f_last
        };
        std::bitset<f_last> available;

        unsigned char repeatIndicator; //valid values are 0..3, number of time this msg has been repeated, 3 means do not repeat any more, default 0.
        MMSI sourceID;
        MMSI destinationID1;
        Byte messageID1_1;
        Word slotOffset1_1;
        Byte messageID1_2;
        Word slotOffset1_2;
        MMSI destinationID2;
        Byte messageID2_1;
        Word slotOffset2_1;
    public:

        AISMessage15()
            :available(0)
            ,repeatIndicator(0)
            ,sourceID(0)
            ,destinationID1(0)
            ,messageID1_1(0)
            ,slotOffset1_1(0)
            ,messageID1_2(0)
            ,slotOffset1_2(0)
            ,destinationID2(0)
            ,messageID2_1(0)
            ,slotOffset2_1(0) {
        }

        ~AISMessage15() {

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
         * has_sourceID
         * @return
         */
        bool has_sourceID() {
            return available[f_sourceID];
        }

        /**
         * get_sourceID
         * @return
         */
        MMSI get_sourceID() {
            return sourceID;
        }

        bool has_destinationID1() {
            return available[f_destinationID1];
        }

        MMSI get_DestinationID1() {
            return destinationID1;
        }

        bool has_messageID1_1() {
            return available[f_messageID1_1];
        }

        Byte get_messageID1_1() {
            return messageID1_1;
        }

        bool has_slotOffset1_1() {
            return available[f_slotOffset1_1];
        }

        Word get_slotOffset1_1() {
            return slotOffset1_1;
        }

        bool has_messageID1_2() {
            return available[f_messageID1_2];
        }

        Byte get_messageID1_2() {
            return messageID1_2;
        }

        bool has_slotOffset1_2() {
            return available[f_slotOffset1_2];
        }

        Word get_slotOffset1_2() {
            return slotOffset1_2;
        }

        bool has_destinationID2() {
            return available[f_destinationID2];
        }

        MMSI get_destinationID2() {
            return destinationID2;
        }

        bool has_messageID2_1() {
            return available[f_messageID2_1];
        }

        Byte get_messageID2_1() {
            return messageID2_1;
        }

        bool has_slotOffset2_1() {
            return available[f_slotOffset2_1];
        }

        Word get_slotOffset2_1() {
            return slotOffset2_1;
        }

        /**
         * build
         * @param payload message's payload
         * @param error error code (EXIT_SUCCESS if none)
         * @return basic_smart_pointer object containing an AISMessage15 object if the error code is EXIT_SUCCESS
         */
        static AISMsg15 build(const unsigned char *payload,const size_t payloadSize,int &error);
    };
} //namespace IEC61162

#endif /* _AIS_MESSAGE_15_H_ */
