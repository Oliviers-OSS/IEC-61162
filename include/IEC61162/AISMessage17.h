/*
 * AISMessage17.h
 * GNSS broadcast binary message
 * R-REC-M.1371-4-201004-E p116-117
 *  Created on: 8 avr. 2013
 *      Author: oc
 */

#ifndef _AIS_MESSAGE_17_H_
#define _AIS_MESSAGE_17_H_

#include <bitset>
#include <IEC61162/basicsmartpointer.h>
#include <IEC61162/AISDataTypes.h>

namespace IEC61162 {

    class AISMessage17;
    typedef basic_smart_pointer<AISMessage17>    AISMsg17;

    class AISMessage17 {

        enum fields {
            f_repeatIndicator = 0
            ,f_sourceID
            ,f_longitude
            ,f_latitude
            ,f_data
            ,f_last
        };
        std::bitset<f_last> available;

        unsigned char repeatIndicator; //valid values are 0..3, number of time this msg has been repeated, 3 means do not repeat any more, default 0.
        MMSI sourceID;
        float longitude;
        float latitude;
        Byte *data;
        Word sizeInBits;
    public:

        AISMessage17()
        :available(0)
        ,repeatIndicator(0)
        ,sourceID(0)
        ,longitude(181.0)
        ,latitude(91.0)
        ,data(0)
        ,sizeInBits(0) {
        }

        virtual ~AISMessage17() {
            if (data) {
                delete [] data;
            }
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

        /**
         * has_longitude
         * @return true if the longitude is set
         */
        bool has_longitude() {
            return available[f_longitude];
        }

        /**
         * get_longitude
         * @return longitude in degrees
         * East = positive
         * West = negative
         * 181 means not available
         */
        float get_longitude() {
            return longitude;
        }

        /**
         * has_latitude
         * @return true if the has_latitude is set
         */
        bool has_latitude(){
            return available[f_latitude];
        }

        /**
         * get_latitude
         * @return latitude in degrees
         * North = positive
         * South = negative
         * 91 means not available
         */
        float get_latitude() {
            return  latitude;
        }

        virtual bool has_data() {
            return available[f_data];
        }

        virtual const Byte* get_data() {
            return data;
        }

        /**
         * build
         * @param payload message's payload
         * @param error error code (EXIT_SUCCESS if none)
         * @return basic_smart_pointer object containing an AISMessage17 object if the error code is EXIT_SUCCESS
         */
        static AISMsg17 build(const unsigned char *payload,const size_t payloadSize,int &error);
    };
} //namespace IEC61162

#endif /* _AIS_MESSAGE_17_H_ */
