/*
 * AISMessage5.h
 * Ship static and voyage related data, cf. R-REC-M.1371-4-201004-E p105
 *  Created on: 1 avr. 2013
 *      Author: oc
 */

#ifndef _AIS_MESSAGE_5_H_
#define _AIS_MESSAGE_5_H_

#include <bitset>
#include <IEC61162/basicsmartpointer.h>
#include <IEC61162/AISDataTypes.h>
#include <cstring>

namespace IEC61162 {
    class AISMessage5;
    typedef basic_smart_pointer<AISMessage5>    AISMsg5;

    class AISMessage5 {
        enum fields {
             f_repeatIndicator = 0
            ,f_userID
            ,f_AISVersionIndicator
            ,f_IMONumber
            ,f_callSign
            ,f_name
            ,f_typeOfShipAndCargoType
            ,f_overallDimensionReferenceForPosition
            ,f_typeOfPositionDevice
            ,f_ETA
            ,f_maximumPresentStaticDraught
            ,f_destination
            ,f_DTE
            ,f_last
        };
        std::bitset<f_last> available;
        unsigned char repeatIndicator; //valid values are 0..3, number of time this msg has been repeated, 3 means do not repeat any more, default 0.
        MMSI userID;
        unsigned char AISVersionIndicator;
        unsigned long IMONumber;
        char callSign[8];
        char name[21];
        unsigned char typeOfShipAndCargoType;
        ReferencePoint overallDimensionReferenceForPosition;
        unsigned char typeOfPositionDevice;
        EstimatedTimeOfArrival ETA;
        float maximumPresentStaticDraught;
        char destination[21];
        bool DTE;
    public:

        AISMessage5()
            :available(0x0)
            ,repeatIndicator(0)
            ,userID(0)
            ,AISVersionIndicator(0) //station compliant with Recommendation ITU-R M.1371-1
            ,IMONumber(0)
            ,typeOfShipAndCargoType(0)
            ,typeOfPositionDevice(0)
            ,maximumPresentStaticDraught(0.0)
            ,DTE(1) {
            strcpy(callSign,"@@@@@@@");
            strcpy(name,"@@@@@@@@@@@@@@@@@@@@");
            strcpy(destination,"@@@@@@@@@@@@@@@@@@@@");
        }

        ~AISMessage5() {
        }

        /**
         * has_repeatIndicator
         * @return true if the repeatIndicator field is set
         */
        bool has_repeatIndicator() {
            return available[f_repeatIndicator];
        }

        unsigned char get_AISVersionIndicator() {
            return AISVersionIndicator;
        }

        unsigned long get_IMONumber() {
            return IMONumber;
        }

        const char *get_callSign() {
            return callSign;
        }

        const char *get_name() {
            return name;
        }

        unsigned char get_typeOfShipAndCargoType() {
            return typeOfShipAndCargoType;
        }

        ReferencePoint get_overallDimensionReferenceForPosition() {
            return overallDimensionReferenceForPosition;
        }

        unsigned char get_typeOfPositionDevice() {
            return typeOfPositionDevice;
        }

        EstimatedTimeOfArrival get_ETA() {
            return ETA;
        }

        float get_maximumPresentStaticDraught() {
            return maximumPresentStaticDraught;
        }

        const char *get_destination() {
            return destination;
        }

        bool get_DTE() {
            return DTE;
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
        bool has_userID() {
            return available[f_userID];
        }

        /**
         * get_userID
         * @return
         */
        MMSI get_userID() {
            return userID;
        }

        bool has_AISVersionIndicator() {
            return available[f_AISVersionIndicator];
        }

        bool has_IMONumber() {
            return available[f_IMONumber];
        }

        bool has_callSign() {
           return available[f_callSign];
        }

        bool has_name() {
           return available[f_name];
        }

        bool has_typeOfShipAndCargoType() {
            return available[f_typeOfShipAndCargoType];
        }

        bool has_overallDimensionReferenceForPosition() {
            return available[f_overallDimensionReferenceForPosition];
        }

        bool has_typeOfPositionDevice() {
            return available[f_typeOfPositionDevice];
        }

        bool has_ETA() {
            return available[f_ETA];
        }

        bool has_maximumPresentStaticDraught() {
            return available[f_maximumPresentStaticDraught];
        }

        bool has_destination() {
            return available[f_destination];
        }

        bool has_DTE() {
            return available[f_DTE];
        }


        /**
         * build
         * @param message's 8 bits payload
         * @param error code (EXIT_SUCCESS if none)
         * @return basic_smart_pointer object containing an AISMessage5 object if the error code is EXIT_SUCCESS
         */
        static AISMsg5 build(const unsigned char *payload,int &error);
    };
} //namespace IEC61162


#endif /* _AIS_MESSAGE_5_H_ */
