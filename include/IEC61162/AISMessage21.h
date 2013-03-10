/*
 * AISMessage21.h
 * Aids-to-navigation report (AtoN)
 * cf. R-REC-M.1371-4-201004-E p 123-127
 *  Created on: 9 avr. 2013
 *      Author: oc
 */

#ifndef _AIS_MESSAGE_21_H_
#define _AIS_MESSAGE_21_H_

#include <bitset>
#include <IEC61162/basicsmartpointer.h>
#include <IEC61162/AISDataTypes.h>
#include <cstring>

namespace IEC61162 {

    class AISMessage21;
    typedef basic_smart_pointer<AISMessage21>    AISMsg21;

    class AISMessage21 {

        enum fields {
            f_repeatIndicator = 0
            ,f_sourceID
            ,f_typeOfAidsToNavigation
            ,f_nameOfAidsToNavigation
            ,f_positionAccuracy
            ,f_longitude
            ,f_latitude
            ,f_dimRefPointForPosition
            ,f_typeOfPositionDevice
            ,f_timeStamp
            ,f_offPositionIndicator
            ,f_AtoNStatus
            ,f_RAIMFlag
            ,f_virtualAtoNFlag
            ,f_assignedModeFlag
            ,f_nameOfAidToNavigationExtension
            ,f_last
        };
        std::bitset<f_last> available;
        Byte repeatIndicator; //valid values are 0..3, number of time this msg has been repeated, 3 means do not repeat any more, default 0.
        MMSI sourceID;
        Byte typeOfAidsToNavigation;
        char nameOfAidsToNavigation[21];
        bool positionAccuracy;
        float longitude;
        float latitude;
        ReferencePoint dimRefPointForPosition;
        Byte typeOfPositionDevice;
        Byte timeStamp;
        bool offPositionIndicator;
        Byte AtoNStatus;
        bool RAIMFlag;
        bool virtualAtoNFlag;
        bool assignedModeFlag;
        char nameOfAidToNavigationExtension[15];

    public:

        AISMessage21()
            :available(0x0)
            ,repeatIndicator(0)
            ,sourceID(0)
            ,typeOfAidsToNavigation(0)
            ,positionAccuracy(0)
            ,longitude(181.0)
            ,latitude(91.0)
            ,typeOfPositionDevice(0)
            ,timeStamp(60)
            ,offPositionIndicator(0)
            ,AtoNStatus(0)
            ,RAIMFlag(0)
            ,virtualAtoNFlag(0)
            ,assignedModeFlag(0) {
            strcpy(nameOfAidsToNavigation,"@@@@@@@@@@@@@@@@@@@@");
            strcpy(nameOfAidToNavigationExtension,"@@@@@@@@@@@@@@");
        }

        ~AISMessage21() {
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

        bool has_typeOfAidsToNavigation() {
            return available[f_typeOfAidsToNavigation];
        }

        Byte get_typeOfAidsToNavigation() {
            return typeOfAidsToNavigation;
        }

        bool has_nameOfAidsToNavigation() {
            return available[f_nameOfAidsToNavigation];
        }

        const char* get_nameOfAidsToNavigation() {
            return nameOfAidsToNavigation;
        }

        bool has_positionAccuracy() {
            return available[f_positionAccuracy];
        }

        bool get_positionAccuracy() {
            return positionAccuracy;
        }

        bool has_longitude() {
            return available[f_longitude];
        }

        float get_longitude() {
            return longitude;
        }

        bool has_latitude() {
            return available[f_latitude];
        }

        float get_latitude() {
            return latitude;
        }

        bool has_dimRefPointForPosition() {
            return available[f_dimRefPointForPosition];
        }

        const ReferencePoint &get_dimRefPointForPosition() {
            return dimRefPointForPosition;
        }

        bool has_typeOfPositionDevice() {
            return available[f_typeOfPositionDevice];
        }

        Byte get_typeOfPositionDevice() {
            return typeOfPositionDevice;
        }

        bool has_timeStamp() {
            return available[f_timeStamp];
        }

        Byte get_timeStamp() {
            return timeStamp;
        }

        bool has_offPositionIndicator() {
            return available[f_offPositionIndicator];
        }

        bool get_offPositionIndicator() {
            return offPositionIndicator;

        }

        bool has_AtoNStatus() {
            return available[f_AtoNStatus];
        }

        Byte get_AtoNStatus() {
            return AtoNStatus;
        }

        bool has_RAIMFlag() {
            return RAIMFlag;
        }

        bool get_RAIMFlag() {
            return RAIMFlag;
        }

        bool has_virtualAtoNFlag() {
            return available[f_virtualAtoNFlag];
        }

        bool get_virtualAtoNFlag() {
            return virtualAtoNFlag;
        }

        bool has_assignedModeFlag() {
            return available[f_assignedModeFlag];
        }

        bool get_assignedModeFlag() {
            return assignedModeFlag;
        }

        bool has_nameOfAidToNavigationExtension() {
            return available[f_nameOfAidToNavigationExtension];
        }

        const char* get_nameOfAidToNavigationExtension() {
            return nameOfAidToNavigationExtension;
        }

        /**
         * build
         * @param message's 8 bits payload
         * @param error code (EXIT_SUCCESS if none)
         * @return basic_smart_pointer object containing an AISMessage19 object if the error code is EXIT_SUCCESS
         */
        static AISMsg21 build(const unsigned char *payload,const size_t payloadSize,int &error);
    };

} //namespace IEC61162

#endif /* _AIS_MESSAGE_21_H_ */
