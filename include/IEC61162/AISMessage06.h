/*
 * AISMessage6.h
 * Addressed binary message, cf. R-REC-M.1371-4-201004-E p108
 *  Created on: 1 avr. 2013
 *      Author: oc
 */

#ifndef _AIS_MESSAGE_6_H_
#define _AIS_MESSAGE_6_H_

#include <bitset>
#include <IEC61162/basicsmartpointer.h>
#include <IEC61162/AISDataTypes.h>

namespace IEC61162 {
    class AISMessage6;
    typedef basic_smart_pointer<AISMessage6>    AISMsg6;

    struct BinaryData {
        ApplicationIdentifier applicationIdentifier;
        Byte applicationData[920/8];
        size_t applicationDataSize;

        BinaryData()
            :applicationDataSize(0) {
        }
    };

    class AISMessage6 {
    protected:
        enum fields {
            f_repeatIndicator = 0
            ,f_sourceID
            ,f_sequenceNumber
            ,f_destinationID
            ,f_retransmitFlag
            ,f_binaryData
            ,f_last
        };
        std::bitset<f_last> available;
        Byte repeatIndicator; //valid values are 0..3, number of time this msg has been repeated, 3 means do not repeat any more, default 0.
        MMSI sourceID;
        Byte sequenceNumber;
        MMSI destinationID;
        bool retransmitFlag;
        BinaryData binaryData;

    public:

        AISMessage6()
            :available(0x0)
            ,repeatIndicator(0)
            ,sourceID(0)
            ,sequenceNumber(0)
            ,destinationID(0)
            ,retransmitFlag(false) {
        }

        virtual ~AISMessage6() {
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
            return f_sourceID;
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

        bool has_retransmitFlag() {
            return available[f_retransmitFlag];
        }

        bool get_retransmitFlag() {
            return retransmitFlag;
        }

        bool has_binaryData() {
            return available[f_binaryData];
        }

        const BinaryData *get_binaryData() {
            return &binaryData;
        }

        /**
         * build
         * @param message's 8 bits payload
         * @param error code (EXIT_SUCCESS if none)
         * @return basic_smart_pointer object containing an AISMessage6 object if the error code is EXIT_SUCCESS
         */
        static AISMsg6 build(const unsigned char *payload,const size_t payloadSize,int &error);
    };

} // namespace IEC61162

#endif /* _AIS_MESSAGE_6_H_ */
