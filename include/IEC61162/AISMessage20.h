/*
 * AISMessage20.h
 * Data link management message
 * cf. R-REC-M.1371-4-201004-E p 122
 *  Created on: 9 avr. 2013
 *      Author: oc
 */

#ifndef _AIS_MESSAGE_20_H_
#define _AIS_MESSAGE_20_H_

#include <bitset>
#include <IEC61162/basicsmartpointer.h>
#include <IEC61162/AISDataTypes.h>

namespace IEC61162 {

    class AISMessage20;
    typedef basic_smart_pointer<AISMessage20>    AISMsg20;

    class AISMessage20 {

        enum fields {
            f_repeatIndicator = 0
            ,f_sourceID
            ,f_offsetNumber1
            ,f_numberOfSlot1
            ,f_timeOut1
            ,f_increment1
            ,f_offsetNumber2
            ,f_numberOfSlot2
            ,f_timeOut2
            ,f_increment2
            ,f_offsetNumber3
            ,f_numberOfSlot3
            ,f_timeOut3
            ,f_increment3
            ,f_offsetNumber4
            ,f_numberOfSlot4
            ,f_timeOut4
            ,f_increment4
            ,f_last
        };
        std::bitset<f_last> available;
        Byte repeatIndicator; //valid values are 0..3, number of time this msg has been repeated, 3 means do not repeat any more, default 0.
        MMSI sourceID;
        Word offsetNumber1;
        Byte numberOfSlot1;
        Byte timeOut1;
        Word increment1;
        Word offsetNumber2;
        Byte numberOfSlot2;
        Byte timeOut2;
        Word increment2;
        Word offsetNumber3;
        Byte numberOfSlot3;
        Byte timeOut3;
        Word increment3;
        Word offsetNumber4;
        Byte numberOfSlot4;
        Byte timeOut4;
        Word increment4;

    public:

        AISMessage20()
            :available(0x0)
            ,repeatIndicator(0)
            ,sourceID(0)
            ,offsetNumber1(0)
            ,numberOfSlot1(0)
            ,timeOut1(0)
            ,increment1(0)
            ,offsetNumber2(0)
            ,numberOfSlot2(0)
            ,timeOut2(0)
            ,increment2(0)
            ,offsetNumber3(0)
            ,numberOfSlot3(0)
            ,timeOut3(0)
            ,increment3(0)
            ,offsetNumber4(0)
            ,numberOfSlot4(0)
            ,timeOut4(0)
            ,increment4(0) {
        }

        ~AISMessage20() {
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

        bool has_offsetNumber1() {
            return available[f_offsetNumber1];
        }

        Word get_offsetNumber1() {
            return offsetNumber1;
        }

        bool has_numberOfSlot1() {
            return available[f_numberOfSlot1];
        }

        Byte get_numberOfSlot1() {
            return numberOfSlot1;
        }

        bool has_timeOut1() {
            return available[f_timeOut1];
        }

        Byte get_timeOut1() {
            return timeOut1;
        }

        bool has_increment1() {
            return available[f_increment1];
        }

        Word get_increment1() {
            return increment1;
        }

        bool has_offsetNumber2() {
            return available[f_offsetNumber2];
        }

        Word get_offsetNumber2() {
            return offsetNumber2;
        }

        bool has_numberOfSlot2() {
            return available[f_numberOfSlot2];
        }

        Byte get_numberOfSlot2() {
            return numberOfSlot2;
        }

        bool has_timeOut2() {
            return available[f_timeOut2];
        }

        Byte get_timeOut2() {
            return timeOut2;
        }

        bool has_increment2() {
            return available[f_increment2];
        }

        Word get_increment2() {
            return increment2;
        }

        bool has_offsetNumber3() {
            return available[f_offsetNumber3];
        }

        Word get_offsetNumber3() {
            return offsetNumber3;
        }

        bool has_numberOfSlot3() {
            return available[f_numberOfSlot3];
        }

        Byte get_numberOfSlot3() {
            return numberOfSlot3;
        }

        bool has_timeOut3() {
            return available[f_timeOut3];
        }

        Byte get_timeOut3() {
            return timeOut3;
        }

        bool has_increment3() {
            return available[f_increment3];
        }

        Word get_increment3() {
            return increment3;
        }

        bool has_offsetNumber4() {
            return available[f_offsetNumber4];
        }

        Word get_offsetNumber4() {
            return offsetNumber4;
        }

        bool has_numberOfSlot4() {
            return available[f_numberOfSlot4];
        }

        Byte get_numberOfSlot4() {
            return numberOfSlot4;
        }

        bool has_timeOut4() {
            return available[f_timeOut4];
        }

        Byte get_timeOut4() {
            return timeOut4;
        }

        bool has_increment4() {
            return available[f_increment4];
        }

        Word get_increment4() {
            return increment4;
        }

        /**
         * build
         * @param message's 8 bits payload
         * @param error code (EXIT_SUCCESS if none)
         * @return basic_smart_pointer object containing an AISMessage19 object if the error code is EXIT_SUCCESS
         */
        static AISMsg20 build(const unsigned char *payload,const size_t payloadSize,int &error);
    };

} //namespace IEC61162

#endif /* _AIS_MESSAGE_20_H_ */
