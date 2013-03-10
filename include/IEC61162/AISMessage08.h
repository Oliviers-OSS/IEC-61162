/*
 * AISMessage08.h
 * Binary broadcast message, cf R-REC-M.1371-4-201004-E p110
 *  Created on: 7 avr. 2013
 *      Author: oc
 */

#ifndef _AIS_MESSAGE_08_H_
#define _AIS_MESSAGE_08_H_

#include <bitset>
#include <IEC61162/basicsmartpointer.h>
#include <IEC61162/AISDataTypes.h>

namespace IEC61162 {
    class AISMessage8;
    typedef basic_smart_pointer<AISMessage8>    AISMsg8;

    class AISMessage8 {
        enum fields {
            f_repeatIndicator = 0
            ,f_sourceID
            ,f_binaryData
            ,f_last
        };
        std::bitset<f_last> available;
        Byte repeatIndicator; //valid values are 0..3, number of time this msg has been repeated, 3 means do not repeat any more, default 0.
        MMSI sourceID;
        BinaryData binaryData;

    public:

        AISMessage8()
            :available(0)
            ,repeatIndicator(0)
            ,sourceID(0) {
        }

        virtual ~AISMessage8() {
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
        Byte get_repeatIndicator() {
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

        bool has_binaryData() {
            return available[f_binaryData];
        }

        const BinaryData *get_binaryData() {
            return &binaryData;
        }

        /**
         * build
         * @param payload message's payload
         * @param error error code (EXIT_SUCCESS if none)
         * @return basic_smart_pointer object containing an AISMessage9 object if the error code is EXIT_SUCCESS
         */
        static AISMsg8 build(const unsigned char *payload,const size_t payloadSize,int &error);
    };
} //namespace IEC61162



#endif /* _AIS_MESSAGE_08_H_ */
