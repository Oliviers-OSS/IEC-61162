/*
 * AISMessage25.h
 * Single slot binary message
 * cf. R-REC-M.1371-4-201004-E p 132-133
 *  Created on: 9 avr. 2013
 *      Author: oc
 */

#ifndef _AIS_MESSAGE_25_H_
#define _AIS_MESSAGE_25_H_

#include <bitset>
#include <IEC61162/basicsmartpointer.h>
#include <IEC61162/AISDataTypes.h>
#include <cstring>

namespace IEC61162 {

    class AISMessage25;
    typedef basic_smart_pointer<AISMessage25>    AISMsg25;

    class AISMessage25 {
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
            ,f_last
        };
        std::bitset<f_last> available;
        Byte repeatIndicator; //valid values are 0..3, number of time this msg has been repeated, 3 means do not repeat any more, default 0.
        MMSI sourceID;
        bool destinationIndicator;
        bool binaryDataFlag;
        MMSI destinationID;
        Word applicationIdentifier;
        Byte binaryData[128/8];
        Byte binaryDataSize; // size in bits

    public:

        AISMessage25()
            :available(0x0)
            ,repeatIndicator(0)
            ,sourceID(0)
            ,destinationIndicator(0)
            ,binaryDataFlag(0)
            ,destinationID(0)
            ,applicationIdentifier(0)
            ,binaryDataSize(0) {
            memset(binaryData,0,sizeof(binaryData));
        }

        virtual ~AISMessage25() {
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

        /**
         * build
         * @param message's 8 bits payload
         * @param error code (EXIT_SUCCESS if none)
         * @return basic_smart_pointer object containing an AISMessage25 object if the error code is EXIT_SUCCESS
         */
        static AISMsg25 build(const unsigned char *payload,int &error);
    };

} //namespace IEC61162

#endif /* _AIS_MESSAGE_25_H_ */
