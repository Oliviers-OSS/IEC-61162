/*
 * AISMessage26.h
 * Multiple slot binary message with communications state
 * cf. R-REC-M.1371-4-201004-E p 133-135
 *  Created on: 9 avr. 2013
 *      Author: oc
 */

#ifndef _AIS_MESSAGE_26_H_
#define _AIS_MESSAGE_26_H_

#include <bitset>
#include <IEC61162/basicsmartpointer.h>
#include <IEC61162/AISDataTypes.h>
#include <cstring>

namespace IEC61162 {

    class AISMessage26;
    typedef basic_smart_pointer<AISMessage26>    AISMsg26;

    class AISMessage26 {
        int buildNoBinaryDataFlag(const unsigned char *payload,const size_t payloadSize);
        int buildBinaryDataFlag(const unsigned char *payload,const size_t payloadSize);
        int buildDestinationIndicatorNoBinaryDataFlag(const unsigned char *payload,const size_t payloadSize);
        int buildDestinationIndicatorBinaryDataFlag(const unsigned char *payload,const size_t payloadSize);
    protected:
        enum fields {
            f_repeatIndicator = 0
            ,f_sourceID
            ,f_destinationIndicator
            ,f_binaryDataFlag
            ,f_destinationID
            ,f_applicationIdentifier
            ,f_binaryData
            ,f_binaryDataSize
            ,f_communicationStateSelectorFlag
            ,f_communicationState
            ,f_last
        };
        std::bitset<f_last> available;
        Byte repeatIndicator; //valid values are 0..3, number of time this msg has been repeated, 3 means do not repeat any more, default 0.
        MMSI sourceID;
        bool destinationIndicator;
        bool binaryDataFlag;
        MMSI destinationID;
        Word applicationIdentifier;
        Byte binaryData[(128/8)+(224/8)+(224/8)+(224/8)+(224/8)]; //1024 bits
        size_t binaryDataSize; // size in bits
        bool communicationStateSelectorFlag;
        DWord communicationState;
    public:

        AISMessage26()
            :available(0x0)
            ,repeatIndicator(0)
            ,sourceID(0)
            ,destinationIndicator(0)
            ,binaryDataFlag(0)
            ,destinationID(0)
            ,applicationIdentifier(0)
            ,binaryDataSize(0)
            ,communicationStateSelectorFlag(0)
            ,communicationState(0) {
            memset(binaryData,0,sizeof(binaryData));
        }

        virtual ~AISMessage26() {
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

        bool has_destinationIndicator() {
            return available[f_destinationIndicator];
        }

        bool get_destinationIndicator() {
            return destinationIndicator;
        }

        bool has_binaryDataFlag() {
            return available[f_binaryData];
        }

        bool get_binaryDataFlag() {
            return binaryDataFlag;
        }

        bool has_destinationID() {
            return available[f_destinationID];
        }

        MMSI get_destinationID() {
            return destinationID;
        }

        bool has_applicationIdentifier() {
            return available[f_applicationIdentifier];
        }

        Word get_applicationIdentifier() {
            return applicationIdentifier;
        }

        bool has_binaryData() {
            return available[f_binaryData];
        }

        const Byte *get_binaryData() {
            return binaryData;
        }

        bool has_binaryDataSize() {
            return available[f_binaryDataSize];
        }

        size_t get_binaryDataSize() {
            return binaryDataSize;
        }

        bool has_communicationStateSelectorFlag() {
            return available[f_communicationStateSelectorFlag];
        }

        bool get_communicationStateSelectorFlag() {
            return communicationStateSelectorFlag;
        }

        bool has_communicationState() {
            return available[f_communicationState];
        }

        DWord get_communicationState() {
            return communicationState;
        }

        /**
         * build
         * @param message's 8 bits payload
         * @param error code (EXIT_SUCCESS if none)
         * @return basic_smart_pointer object containing an AISMessage26 object if the error code is EXIT_SUCCESS
         */
        static AISMsg26 build(const unsigned char *payload,const size_t payloadSize,int &error);
    };

} //namespace IEC61162

#endif /* _AIS_MESSAGE_25_H_ */
