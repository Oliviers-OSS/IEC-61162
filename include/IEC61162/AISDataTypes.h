/*
 * AISDataTypes.h
 *
 *  Created on: 1 avr. 2013
 *      Author: oc
 */

#ifndef _AIS_DATA_TYPES_H_
#define _AIS_DATA_TYPES_H_

#include <cstring>
#include <ctime>

namespace IEC61162 {

    typedef unsigned long  MMSI;
    typedef unsigned char  Byte;
    typedef signed char    SByte;
    typedef unsigned short Word;
    typedef unsigned long  DWord;

    struct ReferencePoint {
        unsigned long A;
        unsigned long B;
        unsigned char C;
        unsigned char D;

        ReferencePoint(unsigned long a = 0,unsigned long b = 0,unsigned long c = 0,unsigned long d = 0)
            :A(a),B(b),C(c),D(d) {
        }

        ~ReferencePoint() {
        }
    };

    struct EstimatedTimeOfArrival {
        unsigned char month;
        unsigned char day;
        unsigned char hour;
        unsigned char minute;

        EstimatedTimeOfArrival(unsigned char m = 0,unsigned char d = 0,unsigned char h = 24,unsigned char mn = 60)
            :month(m),day(d),hour(h),minute(mn) {
        }

        ~EstimatedTimeOfArrival() {
        }

        bool has_month() {
            return (month != 0);
        }

        bool has_day() {
            return (day != 0);
        }

        bool has_hour() {
            return (hour != 24);
        }

        bool has_minute() {
            return (minute != 60);
        }

        operator time_t () {
            time_t value(0);
            if (has_month() && has_day() && has_hour() && has_minute()) {
                struct tm EPOC_ETA;
                time_t currentTime = time(NULL);
                if (gmtime_r(&currentTime, &EPOC_ETA)) {
                    // adjust the year based on the month
                    if (month < (EPOC_ETA.tm_mon+1)) { // +1 because struct tm's month are 0..11 and AIS ones 1..12
                        EPOC_ETA.tm_year++;
                    }
                    EPOC_ETA.tm_mon = month -1;
                    EPOC_ETA.tm_mday = day;
                    EPOC_ETA.tm_hour = hour;
                    EPOC_ETA.tm_min = minute;
                    EPOC_ETA.tm_sec = 0;
                    value = mktime(&EPOC_ETA);
                }
            }
            return value;
        }
    };

    struct ApplicationIdentifier {
        unsigned short id;

        ApplicationIdentifier()
            :id(0) {
        }

        ~ApplicationIdentifier() {
        }

        unsigned short designedAreaCode() {
            // 10-bit designated area code (DAC)
            const unsigned short code = (id & 0xFFC0) >> 4;
            return code;
        }

        Byte functionIdentifier() {
            // 6-bit function identifier (FI) – allows for 64 unique application specific messages.
            const Byte fctId = id & 0x000F;
            return fctId;
        }

        operator unsigned short() {
            return id;
        }
    };

    class VendorID {
        // Vendor ID field
        // Unique identification of the Unit by a number as defined by the
        // manufacturer (option; “@@@@@@@” = not available = default)
        // cf. table 76A, R-REC-M.1371-4-201004-E p132 for details
    public:
        union  {
            struct {
                char manufacturerID[3]; // 18 bits. The Manufacturer's ID bits indicate the manufacture’s mnemonic code consisting of three 6 bit ASCII characters
                Byte unitModelcode;      // 4 bits. The Unit Model Code bits indicate the binary coded series number of the model. The first model of the manufacture uses “1” and the number is incremented at the release of a new model. The code reverts to “1” after reaching to “15”. The “0” is not used
                DWord unitSerialNumber; // 20 bits. The Unit Serial Number bits indicate the manufacture traceable serial number. When the serial number is composed of numeric only, the binary coding should be used. If it includes figure(s), the manufacture can define the coding method. The coding method should be mentioned in the manual
            };
            char string[8];
        } fields;

        VendorID() {
            std::strcpy(fields.string,"@@@@@@@");
        }

        ~VendorID() {
        }

        bool is_set() {
            return (strcmp(fields.string,"@@@@@@@") != 0);
        }
    };

    enum NavigationStatus {
         under_way_using_engine = 0
        ,at_anchor = 1
        ,not_under_command = 2
        ,restricted_maneuverability = 3
        ,constrained_by_her_draught = 4
        ,moored = 5
        ,aground = 6
        ,engaged_in_fishing = 7
        ,under_way_sailing = 8
        //9 = reserved for future amendment of navigational status for ships carrying DG, HS, or MP, or IMO hazard or pollutant category C, high speed craft (HSC),
        //10 = reserved for future amendment of navigational status for ships carrying dangerous goods (DG), harmful substances (HS) or marine pollutants (MP), or IMO hazard or pollutant category A, wing in grand (WIG);
        //11-13 = reserved for future use,
        ,AIS_SART = 14 //  AIS-SART (active),
        ,not_defined = 15 // default (also used by AIS-SART under test)
    };

    enum TypeOfElectronicPositionFixingDevice {
        undefined = 0
        ,GPS = 1 // Global Positioning System
        ,GNSS // (GLONASS)
        ,Combined_GPS_GLONASS
        ,Loran_C = 4
        ,Chayka = 5
        ,Integrated_Navigation_System = 6
        ,Surveyed = 7
        ,Galileo = 8
        //9-14 = not used
        ,Internal_GNSS
    };

} /* namespace IEC61162 */


#endif /* _AIS_DATA_TYPES_H_ */
