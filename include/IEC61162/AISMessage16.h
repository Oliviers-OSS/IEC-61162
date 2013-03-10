/*
 * AISMessage16.h
 * Assigned mode command
 * R-REC-M.1371-4-201004-E p116-117
 *  Created on: 8 avr. 2013
 *      Author: oc
 */

#ifndef _AIS_MESSAGE_16_H_
#define _AIS_MESSAGE_16_H_

#include <bitset>
#include <IEC61162/basicsmartpointer.h>
#include <IEC61162/AISDataTypes.h>

namespace IEC61162 {

    class AISMessage16;
    typedef basic_smart_pointer<AISMessage16>    AISMsg16;

    class AISMessage16 {

        enum fields {
            f_repeatIndicator = 0
            ,f_sourceID
            ,f_destinationIdA
            ,f_offsetA
            ,f_incrementA
            ,f_destinationIdB
            ,f_offsetB
            ,f_incrementB
            ,f_last
        };
        std::bitset<f_last> available;

        unsigned char repeatIndicator; //valid values are 0..3, number of time this msg has been repeated, 3 means do not repeat any more, default 0.
        MMSI sourceID;
        MMSI destinationIdA;
        Word offsetA;
        Word incrementA;
        MMSI destinationIdB;
        Word offsetB;
        Word incrementB;
    public:

        AISMessage16()
            :available(0)
            ,repeatIndicator(0)
            ,sourceID(0)
            ,destinationIdA(0)
            ,offsetA(0)
            ,incrementA(0)
            ,destinationIdB(0)
            ,offsetB(0)
            ,incrementB(0) {
        }

        ~AISMessage16() {

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

        bool has_destinationIdA() {
            return available[f_destinationIdA];
        }

        MMSI get_destinationIdA() {
            return destinationIdA;
        }

        bool has_offsetA() {
            return available[offsetA];
        }

        Word get_offsetA() {
            return offsetA;
        }

        bool has_incrementA() {
            return available[f_incrementA];
        }

        Word get_incrementA() {
            return incrementA;
        }

        bool has_destinationIdB() {
            return available[f_destinationIdB];
        }

        MMSI get_destinationIdB() {
            return destinationIdB;
        }

        bool has_offsetB() {
            return available[offsetB];
        }

        Word get_offsetB() {
            return offsetB;
        }

        bool has_incrementB() {
            return available[f_incrementB];
        }

        Word get_incrementB() {
            return incrementB;
        }

        /**
         * build
         * @param payload message's payload
         * @param error error code (EXIT_SUCCESS if none)
         * @return basic_smart_pointer object containing an AISMessage15 object if the error code is EXIT_SUCCESS
         */
        static AISMsg16 build(const unsigned char *payload,const size_t payloadSize,int &error);
    };
} //namespace IEC61162

#endif /* _AIS_MESSAGE_16_H_ */
