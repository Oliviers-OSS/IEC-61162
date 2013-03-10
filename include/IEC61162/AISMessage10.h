/*
 * AISMessage10.h
 * UTC and date inquiry, cf R-REC-M.1371-4-201004-E p112
 *  Created on: 7 avr. 2013
 *      Author: oc
 */

#ifndef _AIS_MESSAGE_10_H_
#define _AIS_MESSAGE_10_H_

namespace IEC61162 {
    class AISMessage10;
    typedef basic_smart_pointer<AISMessage10>    AISMsg10;

    class AISMessage10 {
        enum fields {
            f_repeatIndicator = 0
            ,f_sourceID
            ,f_destinationID
            ,f_last
        };
        std::bitset<f_last> available;
        Byte repeatIndicator; //valid values are 0..3, number of time this msg has been repeated, 3 means do not repeat any more, default 0.
        MMSI sourceID;
        MMSI destinationID;

    public:

        AISMessage10()
            :available(0x0)
            ,repeatIndicator(0)
            ,sourceID(0)
            ,destinationID(0) {
        }

        ~AISMessage10() {
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

        /**
         * has_destinationID
         * @return
         */
        bool has_destinationID() {
            return available[f_destinationID];
        }

        /**
         * get_destinationID
         * @return
         */
        MMSI get_destinationID() {
            return destinationID;
        }

        /**
         * build
         * @param payload message's payload
         * @param error error code (EXIT_SUCCESS if none)
         * @return basic_smart_pointer object containing an AISMessage9 object if the error code is EXIT_SUCCESS
         */
        static AISMsg10 build(const unsigned char *payload,int &error);
    };

} //namespace IEC61162



#endif /* _AIS_MESSAGE_10_H_ */
