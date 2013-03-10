/*
 * AISMessage19.h
 * Extended Class B equipment position report
 * cf. R-REC-M.1371-4-201004-E p 120-121
 *  Created on: 9 avr. 2013
 *      Author: oc
 */

#ifndef _AIS_MESSAGE_19_H_
#define _AIS_MESSAGE_19_H_

#include <bitset>
#include <IEC61162/basicsmartpointer.h>
#include <IEC61162/AISDataTypes.h>
#include <cstring>

namespace IEC61162 {

    class AISMessage19;
    typedef basic_smart_pointer<AISMessage19>    AISMsg19;

    class AISMessage19 {

        enum fields {
            f_repeatIndicator = 0
            ,f_userID
            ,f_speedOverGround
            ,f_positionAccuracy
            ,f_longitude
            ,f_latitude
            ,f_courseOverGround
            ,f_trueHeading
            ,f_timeStamp
            ,f_name
            ,f_typeOfShipAndCargoType
            ,f_dimensionOfShipReferenceForPosition
            ,f_typeOfPositionDevice
            ,f_RAIMFlag
            ,f_DTE
            ,f_assignedModeFlag
            ,f_last
        };
        std::bitset<f_last> available;
        Byte repeatIndicator; //valid values are 0..3, number of time this msg has been repeated, 3 means do not repeat any more, default 0.
        MMSI userID;
        float speedOverGround;
        bool positionAccuracy;
        float longitude;
        float latitude;
        float courseOverGround; //COG
        Word trueHeading;
        Byte timeStamp;
        char name[21];
        Byte typeOfShipAndCargoType;
        ReferencePoint dimensionOfShipReferenceForPosition;
        Byte typeOfPositionDevice;
        bool RAIMFlag;
        bool DTE;
        bool  assignedModeFlag;

    public:

        AISMessage19()
        :available(0x0)
        ,repeatIndicator(0)
        ,userID(0)
        ,speedOverGround(102.3)
        ,positionAccuracy(0)
        ,longitude(181.0)
        ,latitude(91.0)
        ,courseOverGround(360.0)
        ,trueHeading(511)
        ,timeStamp(60)
        ,typeOfShipAndCargoType(0)
        ,typeOfPositionDevice(0)
        ,RAIMFlag(0)
        ,DTE(1)
        ,assignedModeFlag(0) {
            strcpy(name,"@@@@@@@@@@@@@@@@@@@@");
        }

        ~AISMessage19() {
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

        /**
         * has_speedOverGround
         * @return true if the speedOverGround field is set
         */
        bool has_speedOverGround(){
            return available[f_speedOverGround];
        }

        /**
         * get_speedOverGround
         * @return speed over ground in knot(s)
         * @warning not a mandatory field, please use the has_speedOverGround method to known if set or not
         * - 102.2 means 102.2 knots or higher
         * - 102.3 means not available
         */
        float get_speedOverGround() {
            return speedOverGround;
        }

        /**
         * has_positionAccuracy
         * @return true if the positionAccuracy is set
         */
        bool has_positionAccuracy(){
            return available[f_positionAccuracy];
        }

        /**
         * get_positionAccuracy
         * @return is set means high (≤ 10 m)
         * else low (>10 m)
         *
         */
        bool get_positionAccuracy() {
            return positionAccuracy;
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

        /**
         * has_courseOverGround
         * @return true if the courseOverGround is set
         */
        bool has_courseOverGround() {
            return available[f_courseOverGround];
        }

        /**
         * get_courseOverGround
         * @return course over ground in degrees (0-359.9)
         * 360 means not available
         */
        float get_courseOverGround() {
            return courseOverGround;
        }

        /**
         * has_trueHeading
         * @return true if the trueHeading is set
         */
        bool has_trueHeading() {
            return available[f_trueHeading];
        }

        /**
         * get_trueHeading
         * @return true Heading in degrees (0-359)
         * 511 means not set;
         */
        unsigned short get_trueHeading() {
            return trueHeading;
        }

        /**
         * has_timeStamp
         * @return true if the timeStamp is set
         */
        bool has_timeStamp(){
            return available[f_timeStamp];
        }

        /**
         * get_timeStamp
         * @return UTC second when the report was generated by the electronic position
         * system (EPFS) (0-59, or 60 if time stamp is not available, which
         * should also be the default value, or 61 if positioning system is in
         * manual input mode, or 62 if electronic position fixing system
         * operates in estimated (dead reckoning) mode, or 63 if the positioning
         * system is inoperative)
         */
        unsigned char get_timeStamp() {
            return timeStamp;
        }

        bool has_name() {
            return available[f_name];
        }

        const char *get_name() {
            return name;
        }

        bool has_typeOfShipAndCargoType() {
            return available[f_typeOfShipAndCargoType];
        }

        unsigned char get_typeOfShipAndCargoType() {
            return typeOfShipAndCargoType;
        }

        bool has_dimensionOfShipReferenceForPosition() {
            return available[f_dimensionOfShipReferenceForPosition];
        }

        ReferencePoint get_dimensionOfShipReferenceForPosition() {
            return dimensionOfShipReferenceForPosition;
        }

        bool has_typeOfPositionDevice() {
            return available[f_typeOfPositionDevice];
        }

        unsigned char get_typeOfPositionDevice() {
            return typeOfPositionDevice;
        }

        /**
         * has_RAIMFlag
         * @return true if the RAIM Flag is set
         */
        bool has_RAIMFlag() {
            return available[f_RAIMFlag];
        }

        /**
         * get_RAIMFlag
         * @return Receiver autonomous integrity monitoring (RAIM) flag of electronic
         * position fixing device; 0 = RAIM not in use = default; 1 = RAIM in
         * use. See R-REC-M.1371-4-201004-E p103 Table 47
         *
         */
        bool get_RAIMFlag() {
            return RAIMFlag;
        }

        bool has_DTE() {
            return available[f_DTE];
        }

        bool get_DTE() {
            return DTE;
        }

        bool  has_assignedModeFlag() {
            return available[f_assignedModeFlag];
        }

        bool  get_assignedModeFlag() {
            return assignedModeFlag;
        }

        /**
         * build
         * @param message's 8 bits payload
         * @param error code (EXIT_SUCCESS if none)
         * @return basic_smart_pointer object containing an AISMessage19 object if the error code is EXIT_SUCCESS
         */
        static AISMsg19 build(const unsigned char *payload,int &error);
    };

} //namespace IEC61162

#endif /* _AIS_MESSAGE_19_H_ */
