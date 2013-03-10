/*
 * AISMessage12.h
 * cf. R-REC-M.1371-4-201004-E p113
 *  Created on: 8 avr. 2013
 *      Author: oc
 */

#ifndef _AIS_MESSAGE_12_H_
#define _AIS_MESSAGE_12_H_

#include <bitset>
#include <IEC61162/basicsmartpointer.h>
#include <IEC61162/AISDataTypes.h>
#include <cstring>

namespace IEC61162 {

    class AISMessage12;
    typedef basic_smart_pointer<AISMessage12>    AISMsg12;

    class AISMessage12 {

        enum fields {
            f_repeatIndicator = 0
            ,f_sourceID
            ,f_sequenceNumber
            ,f_destinationID
            ,f_retransmissionFlag
            ,f_safetyRelatedText
            ,f_last
        };
        std::bitset<f_last> available;

        unsigned char repeatIndicator; //valid values are 0..3, number of time this msg has been repeated, 3 means do not repeat any more, default 0.
        MMSI sourceID;
        Byte sequenceNumber;
        MMSI destinationID;
        bool retransmissionFlag;
        char safetyRelatedText[160]; // up to 936 bits encoding 6-bit ASCII characters

    public:

        AISMessage12()
            :available(0x0)
            ,repeatIndicator(0)
            ,sourceID(0)
            ,sequenceNumber(0)
            ,destinationID(0)
            ,retransmissionFlag(0) {
            memset(safetyRelatedText,0,sizeof(safetyRelatedText));
        }

        ~AISMessage12() {
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
            return available[f_sourceID];
        }

        /**
         * get_userID
         * @return
         */
        MMSI get_sourceID() {
            return sourceID;
        }

        bool has_sequenceNumber() {
            return available[f_sequenceNumber];
        }

        Byte get_sequenceNumber() {
            return sequenceNumber;
        }

        bool has_destinationID() {
            return available[f_destinationID];
        }

        MMSI get_destinationID() {
            return destinationID;
        }

        bool has_retransmissionFlag() {
            return available[f_retransmissionFlag];
        }

        bool get_retransmissionFlag() {
            return retransmissionFlag;
        }

        bool has_safetyRelatedText() {
            return available[f_safetyRelatedText];
        }

        const char *get_safetyRelatedText() {
            return safetyRelatedText;
        }

        /**
         * build
         * @param payload message's payload
         * @param error error code (EXIT_SUCCESS if none)
         * @return basic_smart_pointer object containing an AISMessage9 object if the error code is EXIT_SUCCESS
         */
        static AISMsg12 build(const unsigned char *payload,const size_t payloadSize,int &error);
    };
} //namespace IEC61162




#endif /* _AIS_MESSAGE_12_H_ */
