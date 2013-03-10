/*
 * AISMessage24.h
 * Static data report
 *
 *  Created on: 3 avr. 2013
 *      Author: oc
 */

#ifndef _AIS_MESSAGE_24_H_
#define _AIS_MESSAGE_24_H_

#include <bitset>
#include <IEC61162/basicsmartpointer.h>
#include <IEC61162/AISDataTypes.h>

namespace IEC61162 {

    class AISMessage24;
    class AISMessage24A;
    class AISMessage24B;
    typedef basic_smart_pointer<AISMessage24>    AISMsg24;
    typedef basic_smart_pointer<AISMessage24A>    AISMsg24A;
    typedef basic_smart_pointer<AISMessage24B>    AISMsg24B;

    class AISMessage24 {
    protected:
        enum fields {
             f_repeatIndicator = 0
            ,f_userID
            ,f_partNumber
            ,f_last
        };
        std::bitset<f_last> available;
        Byte repeatIndicator; //valid values are 0..3, number of time this msg has been repeated, 3 means do not repeat any more, default 0.
        MMSI userID;
        Byte partNumber;
        virtual int build(const unsigned char *payload) = 0;
    public:
        AISMessage24()
            :available(0x0)
            ,repeatIndicator(0)
            ,userID(0)
            ,partNumber(4) {
        }

        virtual ~AISMessage24() {
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

        bool has_partNumber() {
           return available[f_partNumber];
        }

        Byte get_partNumber() {
            return partNumber;
        }

        bool is_AISMessage24A() {
            return (0 == partNumber);
        }

        bool is_AISMessage24B() {
            return (1 == partNumber);
        }

        /**
         * build
         * @param message's payload
         * @param error code (EXIT_SUCCESS if none)
         * @return basic_smart_pointer object containing an AISMessage24 (A or B) object if the error code is EXIT_SUCCESS
         */
        static AISMsg24 build(const unsigned char *payload,int &error);
    };

    class AISMessage24A : public AISMessage24 {
        enum fields {
            f_name = 0
            ,f_last
        };
        std::bitset<f_last> available;
        char name[21];
        friend class AISMessage24;
        virtual int build(const unsigned char *payload);
    public:
        AISMessage24A()
            :available(0x0) {
            memset(name,'@',sizeof(name));
            name[sizeof(name)-1] = '\0';
        }

        virtual ~AISMessage24A() {
        }

        bool has_name() {
            return available[f_name];
        }

        const char *get_name() {
            return name;
        }
    };

    class AISMessage24B : public AISMessage24 {
        enum fields {
            f_typeOfShipAndCargoType = 0
            ,f_vendorID
            ,f_callSign
            ,f_dimension
            ,f_motherShip
            ,f_last
        };
        std::bitset<f_last> available;
        Byte typeOfShipAndCargoType;
        VendorID vendorID;
        char callSign[8];
        ReferencePoint dimension;
        MMSI motherShip;
        friend class AISMessage24;
        virtual int build(const unsigned char *payload);
    public:
        AISMessage24B()
            :available(0x0)
            ,typeOfShipAndCargoType(0)
            ,motherShip(0) {
            strcpy(callSign,"@@@@@@");
        }

        virtual ~AISMessage24B() {

        }

        bool has_typeOfShipAndCargoType() {
            return available[f_typeOfShipAndCargoType];
        }

        Byte get_typeOfShipAndCargoType() {
            return typeOfShipAndCargoType;
        }

        bool has_vendorID() {
            return available[f_vendorID];
        }

        VendorID get_vendorID() {
            return vendorID;
        }

        bool has_callSign() {
            return available[f_callSign];
        }

        const char *get_callSign() {
            return callSign;
        }

        bool has_motherShipMMSI() {
            return available[f_motherShip];
        }

        MMSI get_motherShipMMSI() {
            return motherShip;
        }

        bool has_dimension() {
            return available[f_dimension];
        }

        ReferencePoint get_dimension() {
            return dimension;
        }
    };

} //namespace IEC61162

#endif /* _AIS_MESSAGE_24_H_ */
