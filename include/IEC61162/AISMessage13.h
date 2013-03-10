/*
 * AISMessage013.h
 * Safety related acknowledge, cf. R-REC-M.1371-4-201004-E p109
 *  Created on: 5 avr. 2013
 *      Author: oc
 */

#ifndef _AIS_MESSAGE_13_H_
#define _AIS_MESSAGE_13_H_


#include <bitset>
#include <IEC61162/basicsmartpointer.h>
#include <IEC61162/AISDataTypes.h>

namespace IEC61162 {

    class AISMessage13;
    typedef basic_smart_pointer<AISMessage13>    AISMsg13;

    class AISMessage13 {
        enum fields {
            f_repeatIndicator = 0
            ,f_sourceID
            ,f_destinationID1
            ,f_sequenceNumberForID1
            ,f_destinationID2
            ,f_sequenceNumberForID2
            ,f_destinationID3
            ,f_sequenceNumberForID3
            ,f_destinationID4
            ,f_sequenceNumberForID4
            ,f_last
        };
        std::bitset<f_last> available;
        Byte repeatIndicator; //valid values are 0..3, number of time this msg has been repeated, 3 means do not repeat any more, default 0.
        MMSI sourceID;
        MMSI destinationID1;
        Byte sequenceNumberForID1;
        MMSI destinationID2;
        Byte sequenceNumberForID2;
        MMSI destinationID3;
        Byte sequenceNumberForID3;
        MMSI destinationID4;
        Byte sequenceNumberForID4;

    public:
        AISMessage13()
            :repeatIndicator(0)
            ,sourceID(0)
            ,destinationID1(0)
            ,sequenceNumberForID1(0)
            ,destinationID2(0)
            ,sequenceNumberForID2(0)
            ,destinationID3(0)
            ,sequenceNumberForID3(0)
            ,destinationID4(0)
            ,sequenceNumberForID4(0) {
        }

        ~AISMessage13() {
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

        bool has_sequenceNumberForID1() {
            return available[f_sequenceNumberForID1];
        }

        bool has_destinationID2() {
            return available[f_destinationID2];
        }

        bool has_sequenceNumberForID2() {
            return available[f_sequenceNumberForID2];
        }

        bool has_destinationID3() {
            return available[f_destinationID3];
        }

        bool has_sequenceNumberForID3() {
            return available[f_sequenceNumberForID3];
        }

        bool has_destinationID4() {
            return available[f_destinationID4];
        }

        bool has_sequenceNumberForID4() {
            return available[f_sequenceNumberForID4];
        }

        MMSI get_destinationID1() {
            return destinationID1;
        }

        Byte get_sequenceNumberForID1() {
            return sequenceNumberForID1;
        }

        MMSI get_destinationID2() {
            return destinationID2;
        }

        Byte get_sequenceNumberForID2() {
            return sequenceNumberForID2;
        }

        MMSI get_destinationID3() {
            return destinationID3;
        }

        Byte get_sequenceNumberForID3() {
            return sequenceNumberForID3;
        }

        MMSI get_destinationID4() {
            return destinationID4;
        }

        Byte get_sequenceNumberForID4() {
            return sequenceNumberForID4;
        }

        /**
         * build
         * @param message's 8 bits payload
         * @param error code (EXIT_SUCCESS if none)
         * @return basic_smart_pointer object containing an AISMessage13 object if the error code is EXIT_SUCCESS
         */
         static AISMsg13 build(const unsigned char *payload,const size_t payloadSize,int &error);
    };

} // namespace IEC61162

#endif /* _AIS_MESSAGE_13_H_ */
