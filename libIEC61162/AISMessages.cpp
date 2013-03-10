/*
 * AISMessages.cpp
 *
 *  Created on: 10 mars 2013
 *      Author: oc
 */


#include "globals.h"
#include "IEC61162/IEC-61162.h"
#include <arpa/inet.h>
#include <cstring>
#include <cmath>
//#include <cfenv>
#include <fenv.h>
#include "AISFragmentsMgr.h"
#include <cerrno>
#include <typeinfo>
#include <ctime>
#include<algorithm>

using namespace std;
using namespace IEC61162;

#define MODULE_FLAG FLAG_AIS_RUNTIME

static unsigned char get6BitsData(const unsigned char data) {
	unsigned char value = data - 48;
	if (value > 40) {
		value -= 8;
	}
	return value;
}

static int membitscpy(unsigned char *dest, const size_t bitsOffset, const unsigned char value, const size_t n) {
	int error = EXIT_SUCCESS;
	if (n <= 8) {
		const div_t offsets = div(bitsOffset,8);
		const unsigned short source = dest[offsets.quot] << 8 | value << (sizeof(source)*8 - offsets.rem - n);
		const unsigned short s = htons(source); // because we work on bytes
		memcpy(dest+offsets.quot,&s,sizeof(source));
	} else {
		error = EINVAL;
	}
	return error;
}

// The data payload is an ASCII-encoded bit vector. Each character represents six bits of data.
static inline int decodePayload(const char *payload,unsigned char *messageData,size_t &messageSize) {
	int error(EXIT_SUCCESS);

	const char *endOfPayload = reinterpret_cast<const char *>(std::strchr((const char*)payload,','));
	if (endOfPayload) {
		size_t bitsOffset(0);
		register const char *p(payload);
		const unsigned char padding = *(endOfPayload + 1) - '0';
		messageSize = (reinterpret_cast<unsigned int>(endOfPayload) - reinterpret_cast<unsigned int>(payload)) * 6 - ((0 == padding)?-6:padding);
		while ((p < endOfPayload) && (EXIT_SUCCESS == error)) {
			const unsigned char value(get6BitsData(*p));
			error = membitscpy(messageData,bitsOffset,value,6);
			p++;
			bitsOffset += 6;
		}
	} else {
		error = EPROTO;
	}
	return error;
}

static inline int setValue(const unsigned char *data,const size_t offsetInBits,const size_t sizeInBits,unsigned char *value) {
	int error(EXIT_SUCCESS);
	const div_t endPosition = div(offsetInBits + sizeInBits,8);
	const unsigned char endPosMask(0xFF << (8 - endPosition.rem));

	if (endPosition.quot != 0) {
		// first fill the buffer with only the needed data
		// then shift the buffer to get proper alignment
		const unsigned int bufferSize(endPosition.quot+1);
		unsigned char buffer[bufferSize];
		const div_t startPosition = div(offsetInBits,8);
		const unsigned char startPosMask = (0xFF >> startPosition.rem);
		const unsigned char nbCells(endPosition.quot - startPosition.quot);
		register unsigned char *dst = buffer;

		const unsigned char reportMask(~endPosMask);
		unsigned char toReport(0);

		register const unsigned char *src = data + startPosition.quot;
		unsigned char byte = *src &  startPosMask;
		*dst = byte >> (8 - endPosition.rem);
		toReport = byte & reportMask;
		src++;
		dst++;

		register int i;
		for(i=1;i<nbCells;i++) {
			byte = *src;
			src++;
			*dst = (toReport << endPosition.rem) | (byte >> (8 - endPosition.rem));
			dst++;
			toReport = byte & reportMask;
		}

		byte = *src &  endPosMask;
		*dst = (toReport << endPosition.rem) | (byte >> (8 - endPosition.rem));


		#if __BYTE_ORDER == __BIG_ENDIAN
		dst = value;
		src = buffer;
		for(i=0;i<=nbCells;i++) {
			*dst = *src;
			src++;
			dst++;
		}
		#else
		// Little endian => copy in reverse order to set bytes in the right position
		dst = value + nbCells;
		src = buffer;
		for(i=0;i<=nbCells;i++) {
			*dst = *src;
			src++;
			dst--;
		}
		#endif

	} else {
		*value = (data[endPosition.quot] & endPosMask) >> endPosition.rem;
	}

	return error;
}

static inline void removeTrailingCharacters(char *begin, char *end) {
    register char *cursor = end;
    while((('@' == *cursor) || (' ' == *cursor)) && (cursor >= begin)) {
        *cursor = '\0';
        cursor--;
    }
}

static inline void removeLeadingCharacters(char *begin) {
    register char *readCursor = begin;
    register char *writeCursor = begin;
    while ((readCursor != '\0') && ( ('@' == *readCursor) || (' ' == *readCursor))) {
        readCursor++;
    }
    while(*readCursor != '\0') {
        *writeCursor = *readCursor;
        readCursor++;
        writeCursor++;
    }
    *writeCursor = '\0';
}

static int setString(const unsigned char *data,const size_t offsetInBits,const size_t sizeInBits,char *value) {
    int error(EXIT_SUCCESS);

    // Patch (temporary ?):
    // The AIS test message 14 contains 3 bits in excess in its text part,
    // instead of refusing the message, ignore bits in excess if any.
    //if ((sizeInBits %6) == 0) {
    const div_t sizeTest = div(offsetInBits,6);
    const size_t correctedTextSize = sizeInBits - sizeTest.rem;
    if (sizeTest.rem != 0) {
        WARNING_MSG("Invalid string size (not equal to 0 modulo 6), ignoring the last %u bits.",sizeTest.rem);
    }

    if (correctedTextSize != 0) {
        // table 44 from R-REC-M.1371-4-201004-E chap 3 p100
        //                                   0000000000111111111122222222 223333 333333444444444455555555556666
        //                                   0123456789012345678901234567 890123 456789012345678901234567890123
        static const char *sixBitASCIITable="@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_ !\"#$%&'()*+,-./0123456789:;<=>?";
        size_t currentOffsetInBits = offsetInBits;
        const size_t endOffsetInBits = offsetInBits + correctedTextSize;
        register char *writePosition = value;
        while(currentOffsetInBits < endOffsetInBits) {
            const div_t currentStartOffset = div(currentOffsetInBits,8);
            const unsigned char firstMask = (0xFF >> currentStartOffset.rem);
            const unsigned char availableBits = 8 - currentStartOffset.rem;
            unsigned char sixBitValue = 0;
            if (availableBits >=6) {
                // all bits are in the first cell
                sixBitValue = (data[currentStartOffset.quot] & firstMask) >> (2 - currentStartOffset.rem);
            } else {
                // bits are on two cells
                const unsigned char remainingBits = 6 - availableBits;
                const unsigned char complementRemainingBits = 8 - remainingBits;
                const unsigned char remainingBitsMask = 0xFF << complementRemainingBits;
                sixBitValue = ((data[currentStartOffset.quot] & firstMask) << remainingBits) | ((data[currentStartOffset.quot+1] & remainingBitsMask) >> complementRemainingBits);
            }
            *writePosition = sixBitASCIITable[sixBitValue];
            writePosition++;
            currentOffsetInBits +=6;
        } //while(currentOffsetInBits < endOffsetInBits)
        *writePosition = '\0';

        // remove leading and trailing @ and ' ' characters if any
        writePosition--;
        removeTrailingCharacters(value,writePosition);
        removeLeadingCharacters(value);
    } else {
        //ERROR_MSG("String size is not equal to 0 modulo 6");
        ERROR_MSG("String size is NULL !");
        error = EINVAL;
    }

    return error;
}

/*
 * Rate Of Turn, 8 bits:
 * ---------------------
 * 0 to +126 = turning right at up to 708° per min or higher
 * 0 to –126 = turning left at up to 708° per min or higher
 * Values between 0 and 708° per min coded by
 * ROTAIS = 4.733 SQRT(ROTsensor) degrees per min
 * where ROTsensor is the Rate of Turn as input by an external Rate of
 * Turn Indicator (TI). ROTAIS is rounded to the nearest integer value.
 * +127 = turning right at more than 5o per 30 s (No TI available)
 * –127 = turning left at more than 5o per 30 s (No TI available)
 * –128 (80 hex) indicates no turn information available (default).
 * ROT data should not be derived from COG information.
 */
static inline int RateOfTurn(const signed char rawValue, float &value,bool &available) {
	int error = EXIT_SUCCESS;
	if ((rawValue >= -126 ) && (rawValue <= 126)) {
		feclearexcept(FE_ALL_EXCEPT);
		errno = 0;
		value = powf((float)rawValue / 4.7333,2);
		double v = pow( (double)rawValue / 4.7333,2);
		available = ((fetestexcept(FE_INVALID | FE_DIVBYZERO | FE_OVERFLOW | FE_UNDERFLOW) == 0) && (0 == errno));
		if (available) {
			// restore the sign
			// (negatives values means turning left, positive values means turning right)
			if (rawValue < 0) {
				value = -value;
			}
		} else {
			if (errno != 0) {
				error = errno;
			} else {
				error = ERROR_CODE(FACILITY_AIS_MSG,OUT_OF_VALIDITY_DOMAIN);
			}
		}
	} else if (127 == rawValue) {
		// +127 = turning right at more than 5o per 30 s (No TI available)
		available = false;
		error = ERROR_CODE(FACILITY_AIS_MSG,OUT_OF_BOUNDARY);
		value = INFINITY;
	} else if (-127 == rawValue) {
		// –127 = turning left at more than 5o per 30 s (No TI available)
		available = false;
		error = ERROR_CODE(FACILITY_AIS_MSG,OUT_OF_BOUNDARY);
		value = -INFINITY;
	} else if (-128 == rawValue) {
		// –128 (80 hex) indicates no turn information available (default).
		available = false;
		value = NAN;
	}
	return error;
}

// Longitude, 28 bits, in 1/10 000 min (±180o, East = positive (as per 2’s
// complement), West = negative (as per 2’s complement).
// 181= (6791AC0h) = not available = default)
static inline float setLongitude(const long rawLongValue, bool &available) {
    float longitude = 181.0;
    available = (rawLongValue != 0x6791AC0);
    if (available) {
        if (rawLongValue & (1 << 27)) {
            longitude = - (float)((~rawLongValue) & (0xFFFFFFFF >> 5))/ 600000.0;
        } else {
            longitude = (float)rawLongValue / 600000.0; // convert To Degrees
        }
    }
    return longitude;
}

// Latitude, 27 bits,  in 1/10 000 min (±90°, North = positive (as per 2’s
// complement), South = negative (as per 2’s complement).
// 91° = (3412140h) = not available = default)
static inline float setLatitude(const long rawLatValue, bool &available) {
    float latitude = 91.0;
    available = (rawLatValue != 0x3412140);
    if (available) {
        if (rawLatValue & (1 << 26)) {
            latitude = - (float)((~rawLatValue) & (0xFFFFFFFF >> 6))/ 600000.0;
        } else {
            latitude = (float)rawLatValue / 600000.0; // convert To Degrees
        }
    }
    return latitude;
}

#undef MODULE_FLAG
#define MODULE_FLAG FLAG_AIS_MSG_1

inline AISMsg1 AISMessage1::build(const unsigned char *payload,int &error) {
	error = EXIT_SUCCESS;
	AISMsg1 message = AISMsg1(new (std::nothrow) AISMessage1);
	if (!message.isNull()) {
		// Messages 1, 2, 3: Position reports
		// cf. R-REC-M.1371-4-201004-E p 101-102 for details

		const unsigned char *cursor = payload;
		// ID 6 bits from 0 to 5

		// repeatIndicator: 2 bits from 6 to 7 b00000011 = 0x3
		message->repeatIndicator = (*cursor) & 0x3;
		message->available[f_repeatIndicator] = true;
		DEBUG_VAR(message->repeatIndicator,"%u");

		// User ID (MMSI): 30 bits from 8 to 37
		setValue(payload,8,30,(unsigned char *)&message->userID);
		message->available[f_userID] = true;
		DEBUG_VAR(message->userID,"%u");

		// navigationStatus: 4 bits from 38 to 41 => the last 2 bits of the 4th byte and the 2 first bits of the 5th.
		message->navigationStatus = ((payload[4] &  0x3) << 2) | ((payload[5] & 0xC0) >> 6);
		message->available[f_navigationStatus] = (message->navigationStatus != 15);
		DEBUG_VAR(message->navigationStatus,"%d");

		// Rate of Turn: 8 bits from 42 to 49
		const signed char rawROTValue = ((payload[5] & 0x3F) << 2) | (payload[6] & 0xC0) >> 6 ;
		bool available;
		RateOfTurn(rawROTValue,message->rateOfTurn,available);
		message->available[f_rateOfTurn] = available;
		DEBUG_VAR(message->rateOfTurn,"%d");

		// Speed Over Ground: 10 bits from 50 to 59 => the last 6 bits of the 6th byte and the 4 first bits of the 7th byte.
		const unsigned short rawSOGValue = (payload[6] & 0x30) << 4 | (payload[6] & 0xF) << 4 | (payload[7] & 0xF0) >> 4;
		message->speedOverGround = (float)rawSOGValue / 10.0;
		message->available[f_speedOverGround] = (rawSOGValue != 1023);
		DEBUG_VAR(message->speedOverGround,"%f");

		// positionAccuracy: 1 bit, bit 60 (on the 7th byte)
		message->positionAccuracy = ((payload[7] & 0x8) != 0);
		message->available[f_positionAccuracy] = true;
		DEBUG_VAR_BOOL(message->positionAccuracy);

		// Longitude: 28 bits, from 61 to 88, in 1/10 000 min
		long rawLongValue = 0x0;
		setValue(payload,61,28,(unsigned char *)&rawLongValue);
		message->longitude = setLongitude(rawLongValue,available);
		message->available[f_longitude] = available;
		DEBUG_VAR(message->longitude,"%f");

		// Latitude: 27 bits, from 89 to 115, in 1/10 000 min
		long rawLatValue = 0x0;
		setValue(payload,89,27,(unsigned char *)&rawLatValue);
		message->latitude = setLatitude(rawLatValue,available);
		message->available[f_latitude] = available;
		DEBUG_VAR(message->latitude,"%f");

		// courseOverGround: 12 bits, from 116 to 127
		const long rawCOGValue = (payload[14] & 0x0F) << 8 | payload[15];
		if (message->available[f_courseOverGround] = (rawCOGValue != 0xE10)) {
			message->courseOverGround = (float)rawCOGValue/10.0;
		} else {
		    message->courseOverGround = 360.0;
		}
		DEBUG_VAR(message->courseOverGround,"%f");

		// True heading: 9 bits, from 128 to 136
		message->trueHeading = ((payload[16] >> 7) << 8) | (payload[16] << 1) | (payload[17] & 0x80) >> 7;
		message->available[f_trueHeading] = (message->trueHeading != 511);
		DEBUG_VAR(message->trueHeading,"%u");

		// timeStamp: 6 bits, from 137 to 142
		// UTC second when the report was generated by the electronic position
		// system (EPFS) (0-59, or 60 if time stamp is not available, which
		// should also be the default value, or 61 if positioning system is in
		// manual input mode, or 62 if electronic position fixing system
		// operates in estimated (dead reckoning) mode, or 63 if the positioning
		// system is inoperative)
		message->timeStamp = (payload[17] & 0x7E) >> 1;
		message->available[f_timeStamp] = (message->timeStamp != 60);
		DEBUG_VAR(message->timeStamp,"%u");

		// special manoeuvre indicator (Warning was previously part of the reserved For Regional Application field):
		// 2 bits from 143 to 144
		message->specialManoeuvreIndicator = ((payload[17] & 0x1) << 1) | ((payload[18] & 0x80) >> 7);
		message->available[f_specialManoeuvreIndicator] = (message->specialManoeuvreIndicator != 0);
		DEBUG_VAR(message->specialManoeuvreIndicator,"%u");

		// (spare) 3 bits from 145 to 147, should be zero

		// RAIM-flag: 1 bit (148)
		// Receiver autonomous integrity monitoring (RAIM) flag of electronic
		// position fixing device; 0 = RAIM not in use = default; 1 = RAIM in
		// use. See Table 47
		message->RAIMFlag = (payload[18] & 0x8) >> 3;
		message->available[f_RAIMFlag] = true;
		DEBUG_VAR(message->RAIMFlag,"%u");

		// communicationState: 19 bits from 149 to 168 (Warning was previously 18 bits)
		setValue(payload,149,19,(unsigned char *)&message->communicationState);
		message->available[f_communicationState] = true;
		DEBUG_VAR(message->communicationState,"0x%X");
	} else {
		error = ENOMEM;
		ERROR_MSG("failed to allocate %d byte for a new AI Message 1",sizeof(AISMessage1));
	}
	return message;
}

#undef MODULE_FLAG
#define MODULE_FLAG FLAG_AIS_MSG_2

inline AISMsg2 AISMessage2::build(const unsigned char *payload,int &error) {
    error = EXIT_SUCCESS;
    AISMsg2 message = AISMsg2(new (std::nothrow) AISMessage2);
    if (!message.isNull()) {
        // Messages 1, 2, 3: Position reports
        // cf. R-REC-M.1371-4-201004-E p 101-102 for details

        const unsigned char *cursor = payload;
        // ID 6 bits from 0 to 5

        // repeatIndicator: 2 bits from 6 to 7 b00000011 = 0x3
        message->repeatIndicator = (*cursor) & 0x3;
        message->available[f_repeatIndicator] = true;
        DEBUG_VAR(message->repeatIndicator,"%u");

        // User ID (MMSI): 30 bits from 8 to 37
        setValue(payload,8,30,(unsigned char *)&message->userID);
        message->available[f_userID] = true;
        DEBUG_VAR(message->userID,"%u");

        // navigationStatus: 4 bits from 38 to 41 => the last 2 bits of the 4th byte and the 2 first bits of the 5th.
        message->navigationStatus = ((payload[4] &  0x3) << 2) | ((payload[5] & 0xC0) >> 6);
        message->available[f_navigationStatus] = (message->navigationStatus != 15);
        DEBUG_VAR(message->navigationStatus,"%u");

        // Rate of Turn: 8 bits from 42 to 49
        const char rawROTValue = ((payload[5] & 0x3F) << 2) | (payload[6] & 0xC0) >> 6 ;
        bool available;
        RateOfTurn(rawROTValue,message->rateOfTurn,available);
        message->available[f_rateOfTurn] = available;
        DEBUG_VAR(message->rateOfTurn,"%f");

        // Speed Over Ground: 10 bits from 50 to 59 => the last 6 bits of the 6th byte and the 4 first bits of the 7th byte.
        const unsigned short rawSOGValue = (payload[6] & 0x30) << 4 | (payload[6] & 0xF) << 4 | (payload[7] & 0xF0) >> 4;
        message->speedOverGround = (float)rawSOGValue / 10.0;
        message->available[f_speedOverGround] = (rawSOGValue != 1023);
        DEBUG_VAR(message->speedOverGround,"%f");

        // positionAccuracy: 1 bit, bit 60 (on the 7th byte)
        message->positionAccuracy = ((payload[7] & 0x8) != 0);
        message->available[f_positionAccuracy] = true;
        DEBUG_VAR_BOOL(message->positionAccuracy);

        // Longitude: 28 bits, from 61 to 88, in 1/10 000 min
        long rawLongValue = 0x0;
        setValue(payload,61,28,(unsigned char *)&rawLongValue);
        message->longitude = setLongitude(rawLongValue,available);
        message->available[f_longitude] = available;
        DEBUG_VAR(message->longitude,"%f");

        // Latitude: 27 bits, from 89 to 115, in 1/10 000 min
        long rawLatValue = 0x0;
        setValue(payload,89,27,(unsigned char *)&rawLatValue);
        message->latitude = setLatitude(rawLatValue,available);
        message->available[f_latitude] = available;
        DEBUG_VAR(message->latitude,"%f");

        // courseOverGround: 12 bits, from 116 to 127
        const long rawCOGValue = (payload[14] & 0x0F) << 8 | payload[15];
        if (message->available[f_courseOverGround] = (rawCOGValue != 0xE10)) {
            message->courseOverGround = (float)rawCOGValue/10.0;
        } else {
            message->courseOverGround = 360.0;
        }
        DEBUG_VAR(message->courseOverGround,"%f");

        // True heading: 9 bits, from 128 to 136
        message->trueHeading = ((payload[16] >> 7) << 8) | (payload[16] << 1) | (payload[17] & 0x80) >> 7;
        message->available[f_trueHeading] = (message->trueHeading != 511);
        DEBUG_VAR(message->trueHeading,"%u");

        // timeStamp: 6 bits, from 137 to 142
        // UTC second when the report was generated by the electronic position
        // system (EPFS) (0-59, or 60 if time stamp is not available, which
        // should also be the default value, or 61 if positioning system is in
        // manual input mode, or 62 if electronic position fixing system
        // operates in estimated (dead reckoning) mode, or 63 if the positioning
        // system is inoperative)
        message->timeStamp = (payload[17] & 0x7E) >> 1;
        message->available[f_timeStamp] = (message->timeStamp != 60);
        DEBUG_VAR(message->timeStamp,"%u");

        // special manoeuvre indicator (Warning was previously part of the reserved For Regional Application field):
        // 2 bits from 143 to 144
        message->specialManoeuvreIndicator = ((payload[17] & 0x1) << 1) | ((payload[18] & 0x80) >> 7);
        message->available[f_specialManoeuvreIndicator] = (message->specialManoeuvreIndicator != 0);
        DEBUG_VAR(message->specialManoeuvreIndicator,"%u");

        // (spare) 3 bits from 145 to 147, should be zero

        // RAIM-flag: 1 bit (148)
        // Receiver autonomous integrity monitoring (RAIM) flag of electronic
        // position fixing device; 0 = RAIM not in use = default; 1 = RAIM in
        // use. See Table 47
        message->RAIMFlag = (payload[18] & 0x8) >> 3;
        message->available[f_RAIMFlag] = true;
        DEBUG_VAR_BOOL(message->RAIMFlag);

        // communicationState: 19 bits from 149 to 168 (Warning was previously 18 bits)
        setValue(payload,149,19,(unsigned char *)&message->communicationState);
        message->available[f_communicationState] = true;
        DEBUG_VAR(message->communicationState,"0x%X");
    } else {
        error = ENOMEM;
        ERROR_MSG("failed to allocate %d byte for a new AI Message 2",sizeof(AISMessage2));
    }
    return message;
}

#undef MODULE_FLAG
#define MODULE_FLAG FLAG_AIS_MSG_3

inline AISMsg3 AISMessage3::build(const unsigned char *payload,int &error) {
    error = EXIT_SUCCESS;
    AISMsg3 message = AISMsg3(new (std::nothrow) AISMessage3);
    if (!message.isNull()) {
        // Messages 1, 2, 3: Position reports
        // cf. R-REC-M.1371-4-201004-E p 101-102 for details

        const unsigned char *cursor = payload;
        // ID 6 bits from 0 to 5

        // repeatIndicator: 2 bits from 6 to 7 b00000011 = 0x3
        message->repeatIndicator = (*cursor) & 0x3;
        message->available[f_repeatIndicator] = true;
        DEBUG_VAR(message->repeatIndicator,"%u");

        // User ID (MMSI): 30 bits from 8 to 37
        setValue(payload,8,30,(unsigned char *)&message->userID);
        message->available[f_userID] = true;
        DEBUG_VAR(message->userID,"%u");

        // navigationStatus: 4 bits from 38 to 41 => the last 2 bits of the 4th byte and the 2 first bits of the 5th.
        message->navigationStatus = ((payload[4] &  0x3) << 2) | ((payload[5] & 0xC0) >> 6);
        message->available[f_navigationStatus] = (message->navigationStatus != 15);
        DEBUG_VAR(message->navigationStatus,"%u");

        // Rate of Turn: 8 bits from 42 to 49
        const char rawROTValue = ((payload[5] & 0x3F) << 2) | (payload[6] & 0xC0) >> 6 ;
        bool available;
        RateOfTurn(rawROTValue,message->rateOfTurn,available);
        message->available[f_rateOfTurn] = available;
        DEBUG_VAR(message->rateOfTurn,"%f");

        // Speed Over Ground: 10 bits from 50 to 59 => the last 6 bits of the 6th byte and the 4 first bits of the 7th byte.
        const unsigned short rawSOGValue = (payload[6] & 0x30) << 4 | (payload[6] & 0xF) << 4 | (payload[7] & 0xF0) >> 4;
        message->speedOverGround = (float)rawSOGValue / 10.0;
        message->available[f_speedOverGround] = (rawSOGValue != 1023);
        DEBUG_VAR(message->rateOfTurn,"%f");

        // positionAccuracy: 1 bit, bit 60 (on the 7th byte)
        message->positionAccuracy = ((payload[7] & 0x8) != 0);
        message->available[f_positionAccuracy] = true;
        DEBUG_VAR_BOOL(message->positionAccuracy);

        // Longitude: 28 bits, from 61 to 88, in 1/10 000 min
        long rawLongValue = 0x0;
        setValue(payload,61,28,(unsigned char *)&rawLongValue);
        message->longitude = setLongitude(rawLongValue,available);
        message->available[f_longitude] = available;
        DEBUG_VAR(message->longitude,"%f");

        // Latitude: 27 bits, from 89 to 115, in 1/10 000 min
        long rawLatValue = 0x0;
        setValue(payload,89,27,(unsigned char *)&rawLatValue);
        message->latitude = setLatitude(rawLatValue,available);
        message->available[f_latitude] = available;
        DEBUG_VAR(message->latitude,"%f");

        // courseOverGround: 12 bits, from 116 to 127
        const long rawCOGValue = (payload[14] & 0x0F) << 8 | payload[15];
        if (message->available[f_courseOverGround] = (rawCOGValue != 0xE10)) {
            message->courseOverGround = (float)rawCOGValue/10.0;
        } else {
            message->courseOverGround = 360.0;
        }
        DEBUG_VAR(message->courseOverGround,"%f");

        // True heading: 9 bits, from 128 to 136
        message->trueHeading = ((payload[16] >> 7) << 8) | (payload[16] << 1) | (payload[17] & 0x80) >> 7;
        message->available[f_trueHeading] = (message->trueHeading != 511);
        DEBUG_VAR(message->trueHeading,"%u");

        // timeStamp: 6 bits, from 137 to 142
        // UTC second when the report was generated by the electronic position
        // system (EPFS) (0-59, or 60 if time stamp is not available, which
        // should also be the default value, or 61 if positioning system is in
        // manual input mode, or 62 if electronic position fixing system
        // operates in estimated (dead reckoning) mode, or 63 if the positioning
        // system is inoperative)
        message->timeStamp = (payload[17] & 0x7E) >> 1;
        message->available[f_timeStamp] = (message->timeStamp != 60);
        DEBUG_VAR(message->timeStamp,"%u");

        // special manoeuvre indicator (Warning was previously part of the reserved For Regional Application field):
        // 2 bits from 143 to 144
        message->specialManoeuvreIndicator = ((payload[17] & 0x1) << 1) | ((payload[18] & 0x80) >> 7);
        message->available[f_specialManoeuvreIndicator] = (message->specialManoeuvreIndicator != 0);
        DEBUG_VAR(message->specialManoeuvreIndicator,"%u");

        // (spare) 3 bits from 145 to 147, should be zero

        // RAIM-flag: 1 bit (148)
        // Receiver autonomous integrity monitoring (RAIM) flag of electronic
        // position fixing device; 0 = RAIM not in use = default; 1 = RAIM in
        // use. See Table 47
        message->RAIMFlag = (payload[18] & 0x8) >> 3;
        message->available[f_RAIMFlag] = true;
        DEBUG_VAR(message->RAIMFlag,"%u");

        // communicationState: 19 bits from 149 to 168 (Warning was previously 18 bits)
        setValue(payload,149,19,(unsigned char *)&message->communicationState);
        message->available[f_communicationState] = true;
        DEBUG_VAR(message->communicationState,"0x%X");
    } else {
        error = ENOMEM;
        ERROR_MSG("failed to allocate %d byte for a new AI Message 3",sizeof(AISMessage3));
    }
    return message;
}

#undef MODULE_FLAG
#define MODULE_FLAG FLAG_AIS_MSG_4

inline AISMsg4 AISMessage4::build(const unsigned char *payload,int &error) {
    error = EXIT_SUCCESS;
    AISMsg4 message = AISMsg4(new (std::nothrow) AISMessage4);
    if (!message.isNull()) {
        // Messages 4: Base station report
        // cf. R-REC-M.1371-4-201004-E p 103 for details

        const unsigned char *cursor = payload;
        // ID 6 bits from 0 to 5

        // repeatIndicator: 2 bits, from 6 to 7 b00000011 = 0x3
        message->repeatIndicator = payload[0] & 0x3;
        message->available[f_repeatIndicator] = true;
        DEBUG_VAR(message->repeatIndicator,"%u");

        // User ID (MMSI): 30 bits, from 8 to 37
        setValue(payload,8,30,(unsigned char *)&message->userID);
        message->available[f_userID] = true;
        DEBUG_VAR(message->userID,"%u");

        // UTC Year: 14 bits, from 38 to 51, 1-9999; 0 = UTC year not available = default
        message->UTC_year =  ((payload[4] & 0x3) << 12) |  ((payload[5] & 0xF0) << 4) | ((payload[5] & 0x0F) << 4) | ((payload[6] & 0xF0) >> 4);
        message->available[f_UTC_year] = (message->UTC_year != 0);
        DEBUG_VAR(message->UTC_year,"%u");

        // UTC month: 4 bits, from 52 to 55
        message->UTC_month = payload[6] & 0x0F;
        message->available[f_UTC_month] = (message->UTC_month != 0);
        DEBUG_VAR(message->UTC_month,"%u");

        // UTC day: 5 bits, from 56 to 60
        message->UTC_day = (payload[7] & 0xF8) >> 3;
        message->available[f_UTC_day] = (message->UTC_day != 0);
        DEBUG_VAR(message->UTC_day,"%u");

        // UTC hour: 5 bits, from 61 to 65
        message->UTC_hour = ((payload[7] & 0x7) << 2) | ((payload[8] & 0xC0) >> 6);
        message->available[f_UTC_hour] =  (message->UTC_hour != 24);
        DEBUG_VAR(message->UTC_hour,"%u");

        // UTC minute: 6 bits, from 66 to 71
        message->UTC_minute = payload[8] & 0x3F;
        message->available[f_UTC_minute] = (message->UTC_minute != 60);
        DEBUG_VAR(message->UTC_minute,"%u");

        // UTC second: 6 bits, from 72 to 77
        message->UTC_second = (payload[9] & 0xFC) >> 2;
        message->available[f_UTC_second] = (message->UTC_second != 60);
        DEBUG_VAR(message->UTC_second,"%u");

        // Position Accuracy: 1 bit (78)
        message->positionAccuracy = (payload[9] & 0x2) >> 1;
        message->available[f_positionAccuracy] = true;
        DEBUG_VAR_BOOL(message->positionAccuracy);

        // Longitude: 28 bits, from 79 to 106, in 1/10 000 min
        long rawLongValue = 0x0;
        bool  available;
        setValue(payload,79,28,(unsigned char *)&rawLongValue);
        message->longitude = setLongitude(rawLongValue,available);
        message->available[f_longitude] = available;
        DEBUG_VAR(message->longitude,"%f");

        // Latitude: 27 bits, from 107 to 134, in 1/10 000 min
        long rawLatValue = 0x0;
        setValue(payload,107,27,(unsigned char *)&rawLatValue);
        message->latitude = setLatitude(rawLatValue,available);
        message->available[f_latitude] = available;
        DEBUG_VAR(message->latitude,"%f");

        // Type of electronic position fixing device: 4 bits, from 135 to 138
        message->typeOfPositionDevice = (payload[16] & 0x3) << 2 | (payload[17] & 0xC0) >> 6;
        message->available[f_typeOfPositionDevice] = (message->typeOfPositionDevice != 0);
        DEBUG_VAR(message->typeOfPositionDevice,"%u");

        // Transmission control for long-range broadcast message: 1 bit (139)
        message->transMissionControlForLongRangeBroadcastMessage = (payload[17] & 0x20) >> 5;
        message->available[f_transMissionControlForLongRangeBroadcastMessage] = (message->transMissionControlForLongRangeBroadcastMessage != 0);
        DEBUG_VAR_BOOL(message->transMissionControlForLongRangeBroadcastMessage);

        // (spare): 9 bits, from 140 to 148

        // RAIM-flag: 1 bit (148)
        message->RAIMFlag = (payload[18] & 0x08) >> 3;
        message->available[f_RAIMFlag] = true;
        DEBUG_VAR(message->RAIMFlag,"%u");

        // communicationState: 19 bits from 149 to 168
        setValue(payload,149,19,(unsigned char *)&message->communicationState);
        message->available[f_communicationState] = true;
        DEBUG_VAR(message->communicationState,"0x%X");
    } else {
        error = ENOMEM;
        ERROR_MSG("failed to allocate %d byte for a new AIS Message 4",sizeof(AISMessage4));
    }
    return message;
}

#undef MODULE_FLAG
#define MODULE_FLAG FLAG_AIS_MSG_5

inline AISMsg5 AISMessage5::build(const unsigned char *payload,int &error) {
    error = EXIT_SUCCESS;
    AISMsg5 message = AISMsg5(new (std::nothrow) AISMessage5);
    if (!message.isNull()) {
        // repeatIndicator: 2 bits, from 6 to 7 b00000011 = 0x3
        message->repeatIndicator = payload[0] & 0x3;
        message->available[f_repeatIndicator] = true;
        DEBUG_VAR(message->repeatIndicator,"%u");

        // User ID (MMSI): 30 bits, from 8 to 37
        setValue(payload,8,30,(unsigned char *)&message->userID);
        message->available[f_userID] = true;
        DEBUG_VAR(message->userID,"%u");

        // AIS Version Indicator: 2 bits from 38 to 39
        // 0 = station compliant with Recommendation ITU-R M.1371-1
        // 1 = station compliant with Recommendation ITU-R M.1371-3
        // 2-3 = station compliant with future editions
        message->AISVersionIndicator = payload[4] & 0x3;
        message->available[f_AISVersionIndicator] = true;
        DEBUG_VAR(message->AISVersionIndicator,"%u");

        // IMO number: 30 bits, from 40 to 69
        setValue(payload,40,30,(unsigned char *)&message->IMONumber);
        message->available[f_IMONumber] = (message->IMONumber != 0);
        DEBUG_VAR(message->IMONumber,"%u");

        // Call sign: 42 bits, from 70 to 111
        error = setString(payload,70,42,message->callSign);
        message->available[f_callSign] = ((EXIT_SUCCESS == error) && (strcmp(message->callSign,"@@@@@@@") != 0));
        DEBUG_VAR(message->callSign,"%s");

        // Name: 120 bits, from 112 to 231
        error = setString(payload,112,120,message->name);
        message->available[f_name] = ((EXIT_SUCCESS == error) && (strcmp(message->name,"@@@@@@@@@@@@@@@@@@@@") != 0));
        DEBUG_VAR(message->name,"%s");

        // Type of ship and cargo type: 8 bits from 232 to 239
        // 0 = not available or no ship = default
        // 1-99 = as defined in § 3.3.2
        // 100-199 = reserved, for regional use
        // 200-255 = reserved, for future use
        // Not applicable to SAR aircraft
        message->typeOfShipAndCargoType = payload[29];
        message->available[f_typeOfShipAndCargoType] = (message->typeOfShipAndCargoType != 0);
        DEBUG_VAR(message->typeOfShipAndCargoType,"%u");

        // Overall dimension / reference for position: 30 bits, from 240 to 269
        // Reference point for reported position.
        // Also indicates the dimension of ship (m) (see Fig. 42 and § 3.3.3)
        // For SAR aircraft, the use of this field may be decided by the
        // responsible administration. If used it should indicate the maximum
        // dimensions of the craft.
        //  A: 9 bits, from 240 to 248
        message->overallDimensionReferenceForPosition.A = payload[30] << 1 | ((payload[31] &  0x80) >> 7);
        DEBUG_VAR(message->overallDimensionReferenceForPosition.A,"%u");
        //  B: 9 bits, from 249 to 257
        message->overallDimensionReferenceForPosition.B = (payload[31] & 0x7F) << 2| ((payload[32] & 0xC0) >> 6);
        DEBUG_VAR(message->overallDimensionReferenceForPosition.B,"%u");
        //  C: 6 bits, from 258 to 263
        message->overallDimensionReferenceForPosition.C = payload[32] & 0x3F;
        DEBUG_VAR(message->overallDimensionReferenceForPosition.C,"%u");
        //  D: 6 bits, from 264 269
        message->overallDimensionReferenceForPosition.D = (payload[33] & 0xFC) >> 2;
        DEBUG_VAR(message->overallDimensionReferenceForPosition.D,"%u");
        message->available[f_overallDimensionReferenceForPosition] = (message->overallDimensionReferenceForPosition.A != 0)
                                                                        && (message->overallDimensionReferenceForPosition.B != 0)
                                                                        && (message->overallDimensionReferenceForPosition.C != 0)
                                                                        && (message->overallDimensionReferenceForPosition.D != 0);

        // Type of electronic position fixing device: 4 bits from 270 to 273
        message->typeOfPositionDevice = (payload[33] & 0x3) << 2 | (payload[34] & 0xC0) >> 6;
        message->available[f_typeOfPositionDevice] = (message->typeOfPositionDevice != 0);
        DEBUG_VAR(message->typeOfPositionDevice,"%u");

        // Estimated time of arrival; MMDDHHMM UTC : 20 bits, from 274 to 293
        message->ETA.month = (payload[34] & 0x3C) >> 2;// Bits 19-16:
        DEBUG_VAR(message->ETA.month,"%u");
        message->ETA.day = ((payload[34] & 0x3) << 3) | ((payload[35] & 0xE0) >> 5); // Bits 15-11
        DEBUG_VAR(message->ETA.day,"%u");
        message->ETA.hour = payload[35] & 0x1F;
        DEBUG_VAR(message->ETA.hour,"%u");
        message->ETA.minute = payload[36] >> 2;
        DEBUG_VAR(message->ETA.minute,"%u");
        message->available[f_ETA] = message->ETA.has_month()
                                        || message->ETA.has_day()
                                        || message->ETA.has_hour()
                                        || message->ETA.has_minute();

        // Maximum present static draught: 8 bits, from 294 to 301
        // In 1/10 m, 255 = draught 25.5 m or greater, 0 = not available = default;
        // in accordance with IMO Resolution A.851
        // Not applicable to SAR aircraft, should be set to 0
        message->maximumPresentStaticDraught = (float)(((payload[36] & 0x3) << 6) | ((payload[37] & 0xFC) >> 2))/10.0;
        message->available[f_maximumPresentStaticDraught] = (message->maximumPresentStaticDraught != 0.0);
        DEBUG_VAR(message->maximumPresentStaticDraught,"%u");

        // Destination: 120 bits, from 302 to 421
        error = setString(payload,302,120,message->destination);
        message->available[f_destination] = (strcmp(message->destination,"@@@@@@@@@@@@@@@@@@@@") != 0);
        DEBUG_VAR(message->destination,"%s");

        // Data terminal equipment (DTE) ready
        // (0 = available, 1 = not available = default)
        message->DTE = (payload[52] & 0x2) >> 1;
        message->available[f_DTE] = (0 == message->DTE);
        DEBUG_VAR_BOOL(message->DTE);

    } else {
        error = ENOMEM;
        ERROR_MSG("failed to allocate %d byte for a new AI Message 5",sizeof(AISMessage5));
    }
    return message;
}

#undef MODULE_FLAG
#define MODULE_FLAG FLAG_AIS_MSG_6

inline AISMsg6 AISMessage6::build(const unsigned char *payload,const size_t payloadSize,int &error) {
    error = EXIT_SUCCESS;
    AISMsg6 message = AISMsg6(new (std::nothrow) AISMessage6);
    if (!message.isNull()) {
        // repeatIndicator: 2 bits, from 6 to 7 b00000011 = 0x3
        message->repeatIndicator = payload[0] & 0x3;
        message->available[f_repeatIndicator] = true;
        DEBUG_VAR(message->repeatIndicator,"%u");

        // Source ID (MMSI): 30 bits, from 8 to 37
        setValue(payload,8,30,(unsigned char *)&message->sourceID);
        message->available[f_sourceID] = true;
        DEBUG_VAR(message->sourceID,"%u");

        // sequenceNumber: 2bits, from 38 to 39
        message->sequenceNumber = payload[4] & 0x3;
        message->available[f_sequenceNumber] = true;
        DEBUG_VAR(message->sequenceNumber,"%u");

        // Destination ID: 30 bits, from 40 to 69
        setValue(payload,40,30,(unsigned char *)&message->destinationID);
        message->available[f_destinationID] = true;
        DEBUG_VAR(message->destinationID,"%u");

        // Retransmit flag: 1bit (70)
        message->retransmitFlag = (payload[8] & 0x2) >> 1;
        DEBUG_VAR_BOOL(message->retransmitFlag);

        // Binary Data / application ID: 16 bits, from 72 to 87
        /*message->binaryData.applicationIdentifier.designedAreaCode = (payload[9] << 2) | ((payload[10] & 0xC0) >> 6);
        DEBUG_VAR(message->binaryData.applicationIdentifier.designedAreaCode,"%u");
        message->binaryData.applicationIdentifier.functionIdentifier = payload[10] & 0x3F;
        DEBUG_VAR(message->binaryData.applicationIdentifier.functionIdentifier,"%u");*/
        message->binaryData.applicationIdentifier.id = (payload[9] << 8) | payload[10];
        DEBUG_VAR(message->binaryData.applicationIdentifier.id,"%u");

        // binary data application data copy, x bits starting from 88 to...
        message->binaryData.applicationDataSize = payloadSize - 88; // in bits
        const div_t numberOfBytes = div(message->binaryData.applicationDataSize,8);
        memcpy((void*)message->binaryData.applicationData,(void*)(payload+88),numberOfBytes.quot);
        const Byte lastByteMask = 0xFFFFFFFF << (8 - numberOfBytes.rem);
        Byte *lastByte = message->binaryData.applicationData + numberOfBytes.quot;
        *lastByte = payload[numberOfBytes.quot + 88] & lastByteMask;
        message->available[f_binaryData] = true;

    } else {
        error = ENOMEM;
        ERROR_MSG("failed to allocate %d byte for a new AI Message 6",sizeof(AISMessage6));
    }
    return message;
}

#undef MODULE_FLAG
#define MODULE_FLAG FLAG_AIS_MSG_7

inline AISMsg7 AISMessage7::build(const unsigned char *payload,const size_t payloadSize,int &error) {
    error = EXIT_SUCCESS;
    AISMsg7 message = AISMsg7(new (std::nothrow) AISMessage7);
    if (!message.isNull()) {
        // repeatIndicator: 2 bits, from 6 to 7 b00000011 = 0x3
        message->repeatIndicator = payload[0] & 0x3;
        message->available[f_repeatIndicator] = true;
        DEBUG_VAR(message->repeatIndicator,"%u");

        // Source ID (MMSI): 30 bits, from 8 to 37
        setValue(payload,8,30,(unsigned char *)&message->sourceID);
        message->available[f_sourceID] = true;
        DEBUG_VAR(message->sourceID,"%u");

        // (spare 2 bits)

        // Destination ID1: 30 bits, from 40 to 69
        setValue(payload,40,30,(unsigned char *)&message->destinationID1);
        message->available[f_destinationID1] = true;
        DEBUG_VAR(message->destinationID1,"%u");

        // Sequence number for ID1: 2 bits, from 70 to 71
        message->sequenceNumberForID1 = payload[8] & 0x3;
        message->available[f_sequenceNumberForID1] = true;
        DEBUG_VAR(message->sequenceNumberForID1,"%u");

        if (payloadSize > 72) {
            // Destination ID2: 30 bits, from 72 to 101
            setValue(payload,72,30,(unsigned char *)&message->destinationID2);
            message->available[f_destinationID2] = true;
            DEBUG_VAR(message->destinationID2,"%u");

            // Sequence number for ID2: 2 bits, from 70 to 71
            message->sequenceNumberForID2 = payload[12] & 0x3;
            message->available[f_sequenceNumberForID2] = true;
            DEBUG_VAR(message->sequenceNumberForID2,"%u");

            if (payloadSize > 104) {
                // Destination ID3: 30 bits, from 104 to 133
                setValue(payload,72,30,(unsigned char *)&message->destinationID3);
                message->available[f_destinationID3] = true;
                DEBUG_VAR(message->destinationID3,"%u");

                // Sequence number for ID3: 2 bits, from 134 to 135
                message->sequenceNumberForID3 = payload[16] & 0x3;
                message->available[f_sequenceNumberForID3] = true;
                DEBUG_VAR(message->sequenceNumberForID3,"%u");

                if (payloadSize > 136) {
                    // Destination ID4: 30 bits, from 136 to 165
                    setValue(payload,136,30,(unsigned char *)&message->destinationID4);
                    message->available[f_destinationID4] = true;
                    DEBUG_VAR(message->destinationID4,"%u");

                    // Sequence number for ID4: 2 bits, from 166 to 167
                    message->sequenceNumberForID4 = payload[20] & 0x3;
                    message->available[f_sequenceNumberForID4] = true;
                    DEBUG_VAR(message->sequenceNumberForID4,"%u");
                } //(payloadSize > 136)
            } //(payloadSize > 104)
        } //(payloadSize > 72)
    } else {
        error = ENOMEM;
        ERROR_MSG("failed to allocate %d byte for a new AIS Message 7",sizeof(AISMessage7));
    }
    return message;
}

#undef MODULE_FLAG
#define MODULE_FLAG FLAG_AIS_MSG_8

inline AISMsg8 AISMessage8::build(const unsigned char *payload,const size_t payloadSize,int &error) {
    error = EXIT_SUCCESS;
    AISMsg8 message = AISMsg8(new (std::nothrow) AISMessage8);
    if (!message.isNull()) {
        // Messages 8: Binary broadcast message
        // cf. R-REC-M.1371-4-201004-E p 110 for details

        const unsigned char *cursor = payload;
        // ID 6 bits from 0 to 5

        // repeatIndicator: 2 bits, from 6 to 7 b00000011 = 0x3
        message->repeatIndicator = payload[0] & 0x3;
        message->available[f_repeatIndicator] = true;
        DEBUG_VAR(message->repeatIndicator,"%u");

        // User ID (MMSI): 30 bits, from 8 to 37
        setValue(payload,8,30,(unsigned char *)&message->sourceID);
        message->available[f_sourceID] = true;
        DEBUG_VAR(message->sourceID,"%u");

        // spare 2 bits, 38 & 39

        if (payloadSize > 40) {
            // Binary Data / application ID: 16 bits, from 40 to 55
            /*
            message->binaryData.applicationIdentifier.designedAreaCode = (payload[5] << 2) | ((payload[6] & 0xC0) >> 6);
            DEBUG_VAR(message->binaryData.applicationIdentifier.designedAreaCode,"%u");
            message->binaryData.applicationIdentifier.functionIdentifier = payload[6] & 0x3F;
            DEBUG_VAR(message->binaryData.applicationIdentifier.functionIdentifier,"%u");
            */
            message->binaryData.applicationIdentifier.id = (payload[5] << 8) | payload[6];
            DEBUG_VAR(message->binaryData.applicationIdentifier.id,"%u");

            // binary data application data copy, x bits starting from 56 to...
            message->binaryData.applicationDataSize = payloadSize - 56; // in bits
            DEBUG_VAR(message->binaryData.applicationDataSize,"%u bits");
            const div_t numberOfBytes = div(message->binaryData.applicationDataSize,8);
            const unsigned char *binaryDataStartAddress = payload+(56/8);
            DEBUG_MSG("% u bytes and %u bits to copy",numberOfBytes.quot,numberOfBytes.rem);
            const size_t size(numberOfBytes.quot + ((0 == numberOfBytes.rem)?0:1));
            DEBUG_DUMP_MEMORY(binaryDataStartAddress,size);
            memcpy((void*)message->binaryData.applicationData,(void*)binaryDataStartAddress,size);
            DEBUG_DUMP_MEMORY(message->binaryData.applicationData,size);
            if (numberOfBytes.rem != 0) {
                const size_t lastByteOffset = numberOfBytes.quot + 1;
                const Byte lastByteMask = 0xFFFFFFFF << (8 - numberOfBytes.rem);
                DEBUG_VAR(message->binaryData.applicationData[lastByteOffset],"0x%X");
                message->binaryData.applicationData[lastByteOffset] &= lastByteMask;
                DEBUG_VAR(message->binaryData.applicationData[lastByteOffset],"0x%X");
#if 0
                const size_t lastByteOffset = numberOfBytes.quot + 1;
                const Byte lastByteMask = 0xFFFFFFFF << (8 - numberOfBytes.rem);
                const Byte lastPayloadByteValue = binaryDataStartAddress[lastByteOffset];
                DEBUG_VAR(lastPayloadByteValue,"0x%X");
                DEBUG_VAR(lastByteMask,"0x%X");
                /*Byte *lastByte = message->binaryData.applicationData + numberOfBytes.quot + 1;
                DEBUG_VAR(*lastByte,"0x%X");
                *lastByte = lastPayloadByteValue & lastByteMask;
                DEBUG_VAR(*lastByte,"0x%X");*/
                message->binaryData.applicationData[lastByteOffset] = lastPayloadByteValue & lastByteMask;
                DEBUG_VAR(message->binaryData.applicationData[lastByteOffset],"0x%X");
#endif
            }
            message->available[f_binaryData] = true;
            DEBUG_DUMP_MEMORY(message->binaryData.applicationData,((message->binaryData.applicationDataSize)/8)+((0 == numberOfBytes.rem)?0:1));
        }
    } else {
        error = ENOMEM;
        ERROR_MSG("failed to allocate %d byte for a new AIS Message 8",sizeof(AISMessage8));
    }
    return message;
}

#undef MODULE_FLAG
#define MODULE_FLAG FLAG_AIS_MSG_9

inline AISMsg9 AISMessage9::build(const unsigned char *payload,int &error) {
    error = EXIT_SUCCESS;
    AISMsg9 message = AISMsg9(new (std::nothrow) AISMessage9);
    if (!message.isNull()) {
        // Message 9: Standard SAR aircraft position report
        // cf R-REC-M.1371-4-201004-E p111

        const unsigned char *cursor = payload;
        // ID 6 bits from 0 to 5

        // repeatIndicator: 2 bits, from 6 to 7 b00000011 = 0x3
        message->repeatIndicator = payload[0] & 0x3;
        message->available[f_repeatIndicator] = true;
        DEBUG_VAR(message->repeatIndicator,"%u");

        // User ID (MMSI): 30 bits, from 8 to 37
        setValue(payload,8,30,(unsigned char *)&message->userID);
        message->available[f_userID] = true;
        DEBUG_VAR(message->userID,"%u");

        // Altitude: 12 bits, from 38 to 49
        message->altitude = ((payload[4] & 0x3) << 8) | (payload[5] << 2) | ((payload[6] & 0xC0) >> 6);
        message->available[f_altitude] = (message->altitude != 4095);
        DEBUG_VAR(message->altitude,"%u");

        // Speed Over Ground: 10 bits, from 50 to 59
        const unsigned int SOG = ((payload[6] & 0x3F) << 4) | ((payload[7] & 0xF0) >> 4);
        DEBUG_VAR(SOG,"%u");
        message->speedOverGround = SOG / 10.0;
        message->available[f_speedOverGround] = (1023 == SOG);
        DEBUG_VAR(message->speedOverGround,"%f");

        // Position accuracy: 1 bit, 60
        message->positionAccuracy = (payload[7] & 0x8) >> 3;
        message->available[f_positionAccuracy] = true;
        DEBUG_VAR_BOOL(message->positionAccuracy);

        // Longitude: 28 bits, from 61 to 88, in 1/10 000 min
        long rawLongValue = 0x0;
        bool available;
        setValue(payload,61,28,(Byte*)&rawLongValue);
        message->longitude = setLongitude(rawLongValue,available);
        message->available[f_longitude] = available;
        DEBUG_VAR(message->longitude,"%f");

        // Latitude: 27 bits, from 89 to 115, in 1/10 000 min
        long rawLatValue = 0x0;
        setValue(payload,89,27,(Byte*)&rawLatValue);
        message->latitude = setLatitude(rawLatValue,available);
        message->available[f_latitude] = available;
        DEBUG_VAR(message->latitude,"%f");

        // Course Over Ground: 12 bits, from 116 to 127
        const long rawCOGValue = (payload[14] & 0xF) << 8 | payload[15];
        if (message->available[f_courseOverGround] = (rawCOGValue != 0xE10)) {
            message->courseOverGround = (float)rawCOGValue/10.0;
        } else {
            message->courseOverGround = 360.0;
        }
        DEBUG_VAR(message->courseOverGround,"%f");

        // timeStamp: 6 bits, from 128 to 133
        // UTC second when the report was generated by the electronic position
        // system (EPFS) (0-59, or 60 if time stamp is not available, which
        // should also be the default value, or 61 if positioning system is in
        // manual input mode, or 62 if electronic position fixing system
        // operates in estimated (dead reckoning) mode, or 63 if the positioning
        // system is inoperative)
        message->timeStamp = (payload[16] & 0xFC) >> 2;
        message->available[f_timeStamp] = (message->timeStamp != 60);
        DEBUG_VAR(message->timeStamp,"%u");

        // Altitude Sensor: 1 bit, 134
        message->altitudeSensor = (payload[16] & 0x2) >> 1;
        message->available[f_altitude] = true;
        DEBUG_VAR_BOOL(message->altitudeSensor);

        // (spare) 7 bits (135->141)

        // DTE: 1 bit, 142
        message->DTE = (payload[17] & 0x2) >> 1;
        message->available[f_DTE] = true;
        DEBUG_VAR(message->DTE,"%u");

        // (spare) 3 bits (143->145)

        // assigned mode flag: 1bit, 146
       message->assignedModeFlag = (payload[18] & 0x20) >> 6;
       message->available[f_assignedModeFlag] = true;
       DEBUG_VAR(message->assignedModeFlag,"%u");

       // RAIM flag: 1 bit, 147
       message->RAIMFlag = (payload[18] & 0x10) >> 5;
       message->available[f_RAIMFlag] = true;
       DEBUG_VAR(message->RAIMFlag,"%u");

       // Communication state selector flag: 1 bit, 148
       message->communicationStateSelectionFlag = (payload[18] & 0x8) >> 4;
       message->available[f_communicationStateSelectionFlag] = true;
       DEBUG_VAR_BOOL(message->communicationStateSelectionFlag);

       // communicationState: 19 bits from 149 to 168
       setValue(payload,149,19,(unsigned char *)&message->communicationState);
       message->available[f_communicationState] = true;
       DEBUG_VAR(message->communicationState,"0x%X");
    } else {
        error = ENOMEM;
        ERROR_MSG("failed to allocate %d byte for a new AIS Message 8",sizeof(AISMessage8));
    }
    return message;
}

#undef MODULE_FLAG
#define MODULE_FLAG FLAG_AIS_MSG_10

inline AISMsg10 AISMessage10::build(const unsigned char *payload,int &error) {
    error = EXIT_SUCCESS;
    AISMsg10 message = AISMsg10(new (std::nothrow) AISMessage10);
    if (!message.isNull()) {
        // Messages 10: UTC and date inquiry
        // cf. R-REC-M.1371-4-201004-E p 112 for details

        const unsigned char *cursor = payload;
        // ID 6 bits from 0 to 5

        // repeatIndicator: 2 bits, from 6 to 7 b00000011 = 0x3
        message->repeatIndicator = payload[0] & 0x3;
        message->available[f_repeatIndicator] = true;
        DEBUG_VAR(message->repeatIndicator,"%u");

        // Source ID (MMSI): 30 bits, from 8 to 37
        setValue(payload,8,30,(unsigned char *)&message->sourceID);
        message->available[f_sourceID] = true;
        DEBUG_VAR(message->sourceID,"%u");

        // (spare) 2 bits, 38 & 39

        // Destination ID (MMSI): 30 bits, from 40 to 69
        setValue(payload,40,30,(unsigned char *)&message->destinationID);
        message->available[f_destinationID] = true;
        DEBUG_VAR(message->destinationID,"%u");

    } else {
        error = ENOMEM;
        ERROR_MSG("failed to allocate %d byte for a new AIS Message 10",sizeof(AISMessage10));
    }
    return message;
}

#undef MODULE_FLAG
#define MODULE_FLAG FLAG_AIS_MSG_11

inline AISMsg11 AISMessage11::build(const unsigned char *payload,int &error) {
    error = EXIT_SUCCESS;
    AISMsg11 message = AISMsg11(new (std::nothrow) AISMessage11);
    if (!message.isNull()) {
        // Messages 11: Base station report
        // cf. R-REC-M.1371-4-201004-E p 103 for details

        const unsigned char *cursor = payload;
        // ID 6 bits from 0 to 5

        // repeatIndicator: 2 bits, from 6 to 7 b00000011 = 0x3
        message->repeatIndicator = payload[0] & 0x3;
        message->available[f_repeatIndicator] = true;
        DEBUG_VAR(message->repeatIndicator,"%u");

        // User ID (MMSI): 30 bits, from 8 to 37
        setValue(payload,8,30,(unsigned char *)&message->userID);
        message->available[f_userID] = true;
        DEBUG_VAR(message->userID,"%u");

        // UTC Year: 14 bits, from 38 to 51, 1-9999; 0 = UTC year not available = default
        message->UTC_year =  ((payload[4] & 0x3) << 12) |  ((payload[5] & 0xF0) << 4) | ((payload[5] & 0x0F) << 4) | ((payload[6] & 0xF0) >> 4);
        message->available[f_UTC_year] = (message->UTC_year != 0);
        DEBUG_VAR(message->UTC_year,"%u");

        // UTC month: 4 bits, from 52 to 55
        message->UTC_month = payload[6] & 0x0F;
        message->available[f_UTC_month] = (message->UTC_month != 0);
        DEBUG_VAR(message->UTC_month,"%u");

        // UTC day: 5 bits, from 56 to 60
        message->UTC_day = (payload[7] & 0xF8) >> 3;
        message->available[f_UTC_day] = (message->UTC_day != 0);
        DEBUG_VAR(message->UTC_day,"%u");

        // UTC hour: 5 bits, from 61 to 65
        message->UTC_hour = ((payload[7] & 0x7) << 2) | ((payload[8] & 0xC0) >> 6);
        message->available[f_UTC_hour] =  (message->UTC_hour != 24);
        DEBUG_VAR(message->UTC_hour,"%u");

        // UTC minute: 6 bits, from 66 to 71
        message->UTC_minute = payload[8] & 0x3F;
        message->available[f_UTC_minute] = (message->UTC_minute != 60);
        DEBUG_VAR(message->UTC_minute,"%u");

        // UTC second: 6 bits, from 72 to 77
        message->UTC_second = (payload[9] & 0xFC) >> 2;
        message->available[f_UTC_second] = (message->UTC_second != 60);
        DEBUG_VAR(message->UTC_second,"%u");

        // Position Accuracy: 1 bit (78)
        message->positionAccuracy = (payload[9] & 0x2) >> 1;
        message->available[f_positionAccuracy] = true;
        DEBUG_VAR(message->positionAccuracy,"%u");

        // Longitude: 28 bits, from 79 to 106, in 1/10 000 min
        long rawLongValue = 0x0;
        bool available;
        setValue(payload,79,28,(unsigned char *)&rawLongValue);
        message->longitude = setLongitude(rawLongValue,available);
        message->available[f_longitude] = available;
        DEBUG_VAR(message->longitude,"%f");

        // Latitude: 27 bits, from 107 to 134, in 1/10 000 min
        long rawLatValue = 0x0;
        setValue(payload,107,27,(unsigned char *)&rawLatValue);
        message->latitude = setLatitude(rawLatValue,available);
        message->available[f_latitude] = available;
        DEBUG_VAR(message->latitude,"%f");

        // Type of electronic position fixing device: 4 bits, from 135 to 138
        message->typeOfPositionDevice = (payload[16] & 0x3) << 2 | (payload[17] & 0xC0) >> 6;
        message->available[f_typeOfPositionDevice] = (message->typeOfPositionDevice != 0);
        DEBUG_VAR(message->typeOfPositionDevice,"%f");

        // Transmission control for long-range broadcast message: 1 bit (139)
        message->transMissionControlForLongRangeBroadcastMessage = (payload[17] & 0x20) >> 5;
        message->available[f_transMissionControlForLongRangeBroadcastMessage] = (message->transMissionControlForLongRangeBroadcastMessage != 0);
        DEBUG_VAR_BOOL(message->transMissionControlForLongRangeBroadcastMessage);

        // (spare): 9 bits, from 140 to 148

        // communicationState: 19 bits from 149 to 168
        setValue(payload,149,19,(unsigned char *)&message->communicationState);
        message->available[f_communicationState] = true;
        DEBUG_VAR(message->communicationState,"0x%X");
    } else {
        error = ENOMEM;
        ERROR_MSG("failed to allocate %d byte for a new AIS Message 11",sizeof(AISMessage11));
    }
    return message;
}

#undef MODULE_FLAG
#define MODULE_FLAG FLAG_AIS_MSG_12

inline AISMsg12 AISMessage12::build(const unsigned char *payload,const size_t payloadSize,int &error) {
    error = EXIT_SUCCESS;
    AISMsg12 message = AISMsg12(new (std::nothrow) AISMessage12);
    if (!message.isNull()) {
        // Messages 11: Base station report
       // cf. R-REC-M.1371-4-201004-E p 103 for details

       const unsigned char *cursor = payload;
       // ID 6 bits from 0 to 5

       // repeatIndicator: 2 bits, from 6 to 7 b00000011 = 0x3
       message->repeatIndicator = payload[0] & 0x3;
       message->available[f_repeatIndicator] = true;
       DEBUG_VAR(message->repeatIndicator,"%u");

       // Source ID (MMSI): 30 bits, from 8 to 37
       setValue(payload,8,30,(unsigned char *)&message->sourceID);
       message->available[f_sourceID] = true;
       DEBUG_VAR(message->sourceID,"%u");

       // Sequence number, 2 bits, from 38 to 39
       message->sequenceNumber = payload[4] & 0x3;
       DEBUG_VAR(message->sequenceNumber,"%u");

       // Destination ID (MMSI): 30 bits, from 40 to 69
       setValue(payload,40,30,(unsigned char *)&message->destinationID);
       message->available[f_destinationID] = true;
       DEBUG_VAR(message->destinationID,"%u");

       // Retransmit flag: 1 bit (70)
       message->retransmissionFlag = (payload[8] & 0x2) >> 1;
       message->available[f_retransmissionFlag] = true;
       DEBUG_VAR(message->retransmissionFlag,"%u");

       // (spare): 1 bit (71)

       // Safety related text: up to 936 bits starting from 72 to... the end of this message
       const size_t testSize = payloadSize - 71;
       error = setString(payload,72,testSize,message->safetyRelatedText);
       message->available[f_safetyRelatedText] = (EXIT_SUCCESS == error);
       DEBUG_VAR(message->safetyRelatedText,"%s");

    } else {
        error = ENOMEM;
        ERROR_MSG("failed to allocate %d byte for a new AIS Message 11",sizeof(AISMessage11));
    }
    return message;
}

#undef MODULE_FLAG
#define MODULE_FLAG FLAG_AIS_MSG_13

inline AISMsg13 AISMessage13::build(const unsigned char *payload,const size_t payloadSize,int &error) {
    error = EXIT_SUCCESS;
    AISMsg13 message = AISMsg13(new (std::nothrow) AISMessage13);
    if (!message.isNull()) {
        // repeatIndicator: 2 bits, from 6 to 7 b00000011 = 0x3
        message->repeatIndicator = payload[0] & 0x3;
        message->available[f_repeatIndicator] = true;
        DEBUG_VAR(message->repeatIndicator,"%u");

        // Source ID (MMSI): 30 bits, from 8 to 37
        setValue(payload,8,30,(unsigned char *)&message->sourceID);
        message->available[f_sourceID] = true;
        DEBUG_VAR(message->sourceID,"%u");

        // (spare 2 bits)

        // Destination ID1: 30 bits, from 40 to 69
        setValue(payload,40,30,(unsigned char *)&message->destinationID1);
        message->available[f_destinationID1] = true;
        DEBUG_VAR(message->destinationID1,"%u");

        // Sequence number for ID1: 2 bits, from 70 to 71
        message->sequenceNumberForID1 = payload[8] & 0x3;
        message->available[f_sequenceNumberForID1] = true;
        DEBUG_VAR(message->sequenceNumberForID1,"%u");

        if (payloadSize > 72) {
            // Destination ID2: 30 bits, from 72 to 101
            setValue(payload,72,30,(unsigned char *)&message->destinationID2);
            message->available[f_destinationID2] = true;
            DEBUG_VAR(message->destinationID2,"%u");

            // Sequence number for ID2: 2 bits, from 70 to 71
            message->sequenceNumberForID2 = payload[12] & 0x3;
            message->available[f_sequenceNumberForID2] = true;
            DEBUG_VAR(message->sequenceNumberForID2,"%u");

            if (payloadSize > 104) {
                // Destination ID3: 30 bits, from 104 to 133
                setValue(payload,72,30,(unsigned char *)&message->destinationID3);
                message->available[f_destinationID3] = true;
                DEBUG_VAR(message->destinationID3,"%u");

                // Sequence number for ID3: 2 bits, from 134 to 135
                message->sequenceNumberForID3 = payload[16] & 0x3;
                message->available[f_sequenceNumberForID3] = true;
                DEBUG_VAR(message->sequenceNumberForID3,"%u");

                if (payloadSize > 136) {
                    // Destination ID4: 30 bits, from 136 to 165
                    setValue(payload,136,30,(unsigned char *)&message->destinationID4);
                    message->available[f_destinationID4] = true;
                    DEBUG_VAR(message->destinationID4,"%u");

                    // Sequence number for ID4: 2 bits, from 166 to 167
                    message->sequenceNumberForID4 = payload[20] & 0x3;
                    message->available[f_sequenceNumberForID4] = true;
                    DEBUG_VAR(message->sequenceNumberForID4,"%u");
                } //(payloadSize > 136)
            } //(payloadSize > 104)
        } //(payloadSize > 72)
    } else {
        error = ENOMEM;
        ERROR_MSG("failed to allocate %d byte for a new AIS Message 13",sizeof(AISMessage13));
    }
    return message;
}

#undef MODULE_FLAG
#define MODULE_FLAG FLAG_AIS_MSG_14

inline AISMsg14 AISMessage14::build(const unsigned char *payload,const size_t payloadSize,int &error) {
    error = EXIT_SUCCESS;
    AISMsg14 message = AISMsg14(new (std::nothrow) AISMessage14);
    if (!message.isNull()) {
        // repeatIndicator: 2 bits, from 6 to 7 b00000011 = 0x3
        message->repeatIndicator = payload[0] & 0x3;
        message->available[f_repeatIndicator] = true;
        DEBUG_VAR(message->repeatIndicator,"%u");

        // Source ID (MMSI): 30 bits, from 8 to 37
        setValue(payload,8,30,(unsigned char *)&message->sourceID);
        message->available[f_sourceID] = true;
        DEBUG_VAR(message->sourceID,"%u");

        // (spare) 2 bits, from 38 to 39

        // Safety related text: up to 968 bits starting from 40 to... the end of this message
       const size_t textSize = payloadSize - 39;
       error = setString(payload,40,textSize,message->safetyRelatedText);
       message->available[f_safetyRelatedText] = (EXIT_SUCCESS == error);
       DEBUG_VAR(message->safetyRelatedText,"%s");
    } else {
        error = ENOMEM;
        ERROR_MSG("failed to allocate %d byte for a new AIS Message 14",sizeof(AISMessage14));
    }
    return message;
}

#undef MODULE_FLAG
#define MODULE_FLAG FLAG_AIS_MSG_15

inline AISMsg15 AISMessage15::build(const unsigned char *payload,const size_t payloadSize,int &error) {
    error = EXIT_SUCCESS;
    AISMsg15 message = AISMsg15(new (std::nothrow) AISMessage15);
    if (!message.isNull()) {
        // repeatIndicator: 2 bits, from 6 to 7 b00000011 = 0x3
        message->repeatIndicator = payload[0] & 0x3;
        message->available[f_repeatIndicator] = true;
        DEBUG_VAR(message->repeatIndicator,"%u");

        // Source ID (MMSI): 30 bits, from 8 to 37
        setValue(payload,8,30,(unsigned char *)&message->sourceID);
        message->available[f_sourceID] = true;
        DEBUG_VAR(message->sourceID,"%u");

        // (spare) 2 bits, from 38 to 39

        // Destination ID1: 30 bits, from 40 to 69
        setValue(payload,40,30,(unsigned char *)&message->destinationID1);
        message->available[f_destinationID1] = true;
        DEBUG_VAR(message->destinationID1,"%u");

        // Message ID1.1: 6 bits, from 70 to 75
        message->messageID1_1 = ((payload[8] & 0x3) << 4) | ((payload[9] & 0xF0) >> 4);
        message->available[f_messageID1_1] = true;
        DEBUG_VAR(message->messageID1_1,"%u");

        // Slot offset 1.1: 12 bits, from 76 to 87
        message->slotOffset1_1 = ((payload[9] & 0xF) << 8) | payload[10];
        message->available[f_slotOffset1_1] = true;
        DEBUG_VAR(message->slotOffset1_1,"%u");

        // (spare) 2 bits, from 88 to 89

        if (payloadSize >= 90) {
            // Message ID1.2: 6 bits, from 90 to 95
            message->messageID1_2 = payload[11] & 0x3F;
            message->available[f_messageID1_2] = (message->messageID1_2 != 0);
            DEBUG_VAR(message->messageID1_2,"%u");

            // Slot offset 1.2: 12 bits, from 96 to 107
            message->slotOffset1_2 = (payload[12] << 4) | ((payload[13] & 0xF0) >> 4);
            message->available[f_slotOffset1_2] = (message->slotOffset1_2 != 0);
            DEBUG_VAR(message->slotOffset1_2,"%u");

            // (spare) 2 bits, from 108 to 109

            if (payloadSize >= 110) {
                // Destination ID 2: 30 bits, from 110 to 139
                setValue(payload,110,30,(unsigned char *)&message->destinationID2);
                message->available[f_destinationID2] = true;
                DEBUG_VAR(message->destinationID2,"%u");

                // Message ID 2.1: 6 bits, from 140 to 145
                message->messageID2_1 = ((payload[17] & 0xF) << 2) | ((payload[18] & 0xC0) >> 6);
                message->available[f_messageID2_1] = true;
                DEBUG_VAR(message->messageID2_1,"%u");

                // Slot offset 2.1: 12 bits, from 146 to 157
                message->slotOffset2_1 = ((payload[18] & 0x3F) << 6) | ((payload[19] & 0xFC) >> 2);
                message->available[f_slotOffset2_1] = true;
                DEBUG_VAR(message->slotOffset2_1,"%u");
            } //(payloadSize >= 110)
        } //(payloadSize >= 90)
    } else {
        error = ENOMEM;
        ERROR_MSG("failed to allocate %d byte for a new AIS Message 15",sizeof(AISMessage15));
    }
    return message;
}

#undef MODULE_FLAG
#define MODULE_FLAG FLAG_AIS_MSG_16

inline AISMsg16 AISMessage16::build(const unsigned char *payload,const size_t payloadSize,int &error) {
    error = EXIT_SUCCESS;
    AISMsg16 message = AISMsg16(new (std::nothrow) AISMessage16);
    if (!message.isNull()) {
        // repeatIndicator: 2 bits, from 6 to 7 b00000011 = 0x3
        message->repeatIndicator = payload[0] & 0x3;
        message->available[f_repeatIndicator] = true;
        DEBUG_VAR(message->repeatIndicator,"%u");

        // Source ID (MMSI): 30 bits, from 8 to 37
        setValue(payload,8,30,(unsigned char *)&message->sourceID);
        message->available[f_sourceID] = true;
        DEBUG_VAR(message->sourceID,"%u");

        // (spare) 2 bits, from 38 to 39

        // Destination ID A: 30 bits, from 40 to 69
        setValue(payload,40,30,(unsigned char *)&message->destinationIdA);
        message->available[f_destinationIdA] = true;
        DEBUG_VAR(message->destinationIdA,"%u");

        // Offset A: 12 bits, from 70 to 81
        message->offsetA = ((payload[8] & 0x3) << 10) | (payload[9] << 2) | ((payload[10] & 0xC0) >> 6);
        message->available[f_offsetA] = true;
        DEBUG_VAR(message->offsetA,"%u");

        // increment A: 10 bits, from 82 to 91
        message->incrementA = ((payload[10] & 0x3F) << 4) | ((payload[11] & 0xF0) >> 4);
        message->available[f_incrementA] = true;
        DEBUG_VAR(message->incrementA,"%u");

        // spare: 4 bits or...
        if (payloadSize > 95) {
            // Destination ID B: 30 bits, from 92 to 121
            setValue(payload,92,30,(unsigned char *)&message->destinationIdB);
            message->available[f_destinationIdB] = true;
            DEBUG_VAR(message->destinationIdB,"%u");

            // Offset B: 12 bits, from 122 to 133
            message->offsetB = ((payload[15] & 0x3F) << 6) | ((payload[16] & 0xFC)>> 2);
            message->available[f_offsetB] = true;
            DEBUG_VAR(message->offsetB,"%u");

            // increment B: 10 bits, from 134 to 143
            message->incrementB = ((payload[16] & 0x03) << 8)| (payload[17]);
            message->available[f_incrementB] = true;
            DEBUG_VAR(message->incrementB,"%u");
        } //(payloadSize > 95)
    } else {
        error = ENOMEM;
        ERROR_MSG("failed to allocate %d byte for a new AIS Message 16",sizeof(AISMessage16));
    }
    return message;
}

#undef MODULE_FLAG
#define MODULE_FLAG FLAG_AIS_MSG_17

inline AISMsg17 AISMessage17::build(const unsigned char *payload,const size_t payloadSize,int &error) {
    error = EXIT_SUCCESS;
    AISMsg17 message = AISMsg17(new (std::nothrow) AISMessage17);
    if (!message.isNull()) {
        // repeatIndicator: 2 bits, from 6 to 7 b00000011 = 0x3
        message->repeatIndicator = payload[0] & 0x3;
        message->available[f_repeatIndicator] = true;
        DEBUG_VAR(message->repeatIndicator,"%u");

        // Source ID (MMSI): 30 bits, from 8 to 37
        setValue(payload,8,30,(unsigned char *)&message->sourceID);
        message->available[f_sourceID] = true;
        DEBUG_VAR(message->sourceID,"%u");

        // (spare) 2 bits, from 38 to 39

        // Longitude: 28 bits, from 40 to 57, in 1/10 000 min
        long rawLongValue = 0x0;
        bool available;
        setValue(payload,40,28,(unsigned char *)&rawLongValue);
        message->longitude = setLongitude(rawLongValue,available);
        message->available[f_longitude] = available;
        DEBUG_VAR(message->longitude,"%f");

        // Latitude: 27 bits, from 58 to 74, in 1/10 000 min
        long rawLatValue = 0x0;
        setValue(payload,68,27,(unsigned char *)&rawLatValue);
        message->latitude = setLatitude(rawLatValue,available);
        message->available[f_latitude] = available;
        DEBUG_VAR(message->latitude,"%f");

        // (spare) 5 bits, from 75 to 79
        message->sizeInBits = payloadSize - 80;
        if (message->sizeInBits) {
            const unsigned char sizeInBytes(message->sizeInBits/8);
            message->data = new (std::nothrow) Byte [sizeInBytes];
            if (message->data) {
                memcpy(message->data,payload+79,sizeInBytes);
            } else {
                error = ENOMEM;
                ERROR_MSG("failed to allocate %d bytes for data of a new AIS Message 17",sizeInBytes);
            }
        }
    } else {
        error = ENOMEM;
        ERROR_MSG("failed to allocate %d bytes for a new AIS Message 17",sizeof(AISMessage16));
    }
    return message;
}

#undef MODULE_FLAG
#define MODULE_FLAG FLAG_AIS_MSG_18

inline AISMsg18 AISMessage18::build(const unsigned char *payload,int &error) {
    error = EXIT_SUCCESS;
    AISMsg18 message = AISMsg18(new (std::nothrow) AISMessage18);
    if (!message.isNull()) {
        // repeatIndicator: 2 bits, from 6 to 7 b00000011 = 0x3
        message->repeatIndicator = payload[0] & 0x3;
        message->available[f_repeatIndicator] = true;
        DEBUG_VAR(message->repeatIndicator,"%u");

        // Source ID (MMSI): 30 bits, from 8 to 37
        setValue(payload,8,30,(unsigned char *)&message->userID);
        message->available[f_userID] = true;
        DEBUG_VAR(message->userID,"%u");

        // (spare) 8 bits, from 38 to 45

        // Speed over ground: 10 bits, from 46 to 55
        // in 1/10 knot steps (0-102.2 knots)
        // 1 023 = not available, 1 022 = 102.2 knots or higher
        const unsigned short rawSOGValue = ((payload[5] & 0x3) << 8) | payload[6];
        if (message->available[f_speedOverGround] = (rawSOGValue != 1023)) {
            message->speedOverGround = (float)rawSOGValue / 10.0;
        } else {
            message->speedOverGround = rawSOGValue;
        }
        DEBUG_VAR(message->speedOverGround,"%f");

        // Position accuracy: 1 bit, 56
        message->positionAccuracy = (payload[7] & 0x80) >> 8;
        message->available[f_positionAccuracy] = true;
        DEBUG_VAR_BOOL(message->positionAccuracy);

        // Longitude: 28 bits, from 57 to 84, in 1/10 000 min
        long rawLongValue = 0x0;
        bool available;
        setValue(payload,57,28,(unsigned char *)&rawLongValue);
        message->longitude = setLongitude(rawLongValue,available);
        message->available[f_longitude] = available;
        DEBUG_VAR(message->longitude,"%f");

        // Latitude: 27 bits, from 85 to 111, in 1/10 000 min
        long rawLatValue = 0x0;
        setValue(payload,85,27,(unsigned char *)&rawLatValue);
        message->latitude = setLatitude(rawLatValue,available);
        message->available[f_latitude] = available;
        DEBUG_VAR(message->latitude,"%f");

        // courseOverGround: 12 bits, from 112 to 123
        const long rawCOGValue = (payload[14] << 4) | ((payload[15] & 0xF0) >> 4);
        if (message->available[f_courseOverGround] = (rawCOGValue != 0xE10)) {
            message->courseOverGround = (float)rawCOGValue/10.0;
        } else {
            message->courseOverGround = 360.0;
        }
        DEBUG_VAR(message->courseOverGround,"%f");

        // True heading: 9 bits, from 124 to 132
        message->trueHeading = ((payload[15] & 0x0F) << 5)| ((payload[16] & 0xF8) >> 3);
        message->available[f_trueHeading] = (message->trueHeading != 511);
        DEBUG_VAR(message->trueHeading,"%u");

        // timeStamp: 6 bits, from 133 to 138
        // UTC second when the report was generated by the electronic position
        // system (EPFS) (0-59, or 60 if time stamp is not available, which
        // should also be the default value, or 61 if positioning system is in
        // manual input mode, or 62 if electronic position fixing system
        // operates in estimated (dead reckoning) mode, or 63 if the positioning
        // system is inoperative)
        message->timeStamp = ((payload[16] & 0x7) << 3) | ((payload[17] & 0xE0) >> 5);
        message->available[f_timeStamp] = (message->timeStamp != 60);
        DEBUG_VAR(message->timeStamp,"%u");

        // (spare) 2 bits, 139 & 140

        // Class B unit flag: 1 bit, 141
        message->classBUnitFlag = (payload[17] & 0x4) >> 2;
        message->available[f_classBUnitFlag] = true;
        DEBUG_VAR_BOOL(message->classBUnitFlag);

        // Class B display flag: 1 bit, 142
        message->classBDisplayFlag = (payload[17] & 0x2) >> 1;
        message->available[f_classBDisplayFlag] = true;
        DEBUG_VAR_BOOL(message->classBDisplayFlag);

        // Class B DSC flag: 1 bit, 143
        message->classBDSCFlag = payload[17] & 0x1;
        message->available[f_classBDSCFlag] = true;
        DEBUG_VAR_BOOL(message->classBDSCFlag);

        // Class B Message 22 flag: 1 bit; 145
        message->classBMessage22Flag = (payload[18] & 0x40) >> 6;
        message->available[f_classBMessage22Flag] = true;
        DEBUG_VAR_BOOL(message->classBMessage22Flag);

        // Class B band flag: 1bit, 144
        // 0 = Capable of operating over the upper 525 kHz band of the marine band
        // 1 = Capable of operating over the whole marine band
        message->classBBandFlag = (payload[18] & 0x80) >> 7;
        // (irrelevant if “Class B Message 22 flag” is 0)
        message->available[f_classBBandFlag] = (message->classBMessage22Flag != 0);
        DEBUG_VAR_BOOL(message->classBBandFlag);

        // Mode flag: 1 bit, 146
        message->modeFlag = (payload[18] & 0x20) >> 5;
        message->available[f_modeFlag] = true;
        DEBUG_VAR_BOOL(message->modeFlag);

        // RAIM (Receiver autonomous integrity monitoring): 1 bit, 147
        // flag of electronic position fixing device;
        // 0 = RAIM not in use = default; 1 = RAIM in use
        message->RAIMFlag = (payload[18] & 0x10) >> 4;
        message->available[f_RAIMFlag] = true;
        DEBUG_VAR(message->RAIMFlag,"%u");

        // Communication state selector flag: 1 bit, 148
        message->communicationStateSelectorFlag = (payload[18] & 0x08) >> 3;
        message->available[f_communicationStateSelectorFlag] = true;
        DEBUG_VAR(message->communicationStateSelectorFlag,"%u");

        // communicationState: 19 bits from 149 to 168
        setValue(payload,149,19,(unsigned char *)&message->communicationState);
        message->available[f_communicationState] = true;
        DEBUG_VAR(message->communicationState,"0x%X");
    } else {
        error = ENOMEM;
        ERROR_MSG("failed to allocate %d byte for a new AIS Message 18",sizeof(AISMessage18));
    }
    return message;
}

#undef MODULE_FLAG
#define MODULE_FLAG FLAG_AIS_MSG_19

inline AISMsg19 AISMessage19::build(const unsigned char *payload,int &error) {
    error = EXIT_SUCCESS;
    AISMsg19 message = AISMsg19(new (std::nothrow) AISMessage19);
    if (!message.isNull()) {
        // repeatIndicator: 2 bits, from 6 to 7 b00000011 = 0x3
        message->repeatIndicator = payload[0] & 0x3;
        message->available[f_repeatIndicator] = true;
        DEBUG_VAR(message->repeatIndicator,"%u");

        // User ID (MMSI): 30 bits, from 8 to 37
        setValue(payload,8,30,(unsigned char *)&message->userID);
        message->available[f_userID] = true;
        DEBUG_VAR(message->userID,"%u");

        // (spare) 8 bits, from 38 to 45

        // Speed over ground: 10 bits, from 46 to 55
        // in 1/10 knot steps (0-102.2 knots)
        // 1 023 = not available, 1 022 = 102.2 knots or higher
        const unsigned short rawSOGValue = ((payload[5] & 0x3) << 8) | payload[6];
        if (message->available[f_speedOverGround] = (rawSOGValue != 1023)) {
            message->speedOverGround = (float)rawSOGValue / 10.0;
        } else {
            message->speedOverGround = rawSOGValue;
        }
        DEBUG_VAR(message->speedOverGround,"%f");

        // Position accuracy: 1 bit, 56
        message->positionAccuracy = (payload[7] & 0x80) >> 8;
        message->available[f_positionAccuracy] = true;
        DEBUG_VAR_BOOL(message->positionAccuracy);

        // Longitude: 28 bits, from 57 to 84, in 1/10 000 min
        long rawLongValue = 0x0;
        bool available;
        setValue(payload,57,28,(unsigned char *)&rawLongValue);
        message->longitude = setLongitude(rawLongValue,available);
        message->available[f_longitude] = available;
        DEBUG_VAR(message->longitude,"%f");

        // Latitude: 27 bits, from 85 to 111, in 1/10 000 min
        long rawLatValue = 0x0;
        setValue(payload,85,27,(unsigned char *)&rawLatValue);
        message->latitude = setLatitude(rawLatValue,available);
        message->available[f_latitude] = available;
        DEBUG_VAR(message->latitude,"%f");

        // courseOverGround: 12 bits, from 112 to 123
        const long rawCOGValue = (payload[14] & 0x0F) << 4 | ((payload[15] & 0xF0) >> 4);
        if (message->available[f_courseOverGround] = (rawCOGValue != 0xE10)) {
            message->courseOverGround = (float)rawCOGValue/10.0;
        } else {
            message->courseOverGround = 360.0;
        }
        DEBUG_VAR(message->courseOverGround,"%f");

        // True heading: 9 bits, from 124 to 132
        message->trueHeading = ((payload[15] & 0x0F) << 5)| ((payload[16] & 0xF8) >> 3);
        message->available[f_trueHeading] = (message->trueHeading != 511);
        DEBUG_VAR(message->trueHeading,"%u");

        // timeStamp: 6 bits, from 133 to 138
        // UTC second when the report was generated by the electronic position
        // system (EPFS) (0-59, or 60 if time stamp is not available, which
        // should also be the default value, or 61 if positioning system is in
        // manual input mode, or 62 if electronic position fixing system
        // operates in estimated (dead reckoning) mode, or 63 if the positioning
        // system is inoperative)
        message->timeStamp = ((payload[16] & 0x7) << 3) | ((payload[17] & 0xE0) >> 5);
        message->available[f_timeStamp] = (message->timeStamp != 60);
        DEBUG_VAR(message->timeStamp,"%u");

        // (spare) 4 bits, from 133 to 136

        // name: 120 bits, maximum 20 characters 6-bit ASCII, from 137 to 256
        setString(payload,137,120,message->name);
        message->available[f_name] = (strcmp(message->name,"@@@@@@@@@@@@@@@@@@@@") != 0);
        DEBUG_VAR(message->name,"%s");

        // Type of ship and cargo type: 8 bits, from 257 to 264
        message->typeOfShipAndCargoType = ((payload[32] & 0x1) << 7) | (payload[33] & 0xFE);
        message->available[f_typeOfShipAndCargoType] = (message->typeOfShipAndCargoType != 0);
        DEBUG_VAR(message->typeOfShipAndCargoType,"%u");

        // Dimension of ship/reference for position: 30 bits, from 265 to 294
        message->dimensionOfShipReferenceForPosition.A = ((payload[33] & 0x1) << 8) | payload[34];
        DEBUG_VAR(message->dimensionOfShipReferenceForPosition.A,"%u");
        message->dimensionOfShipReferenceForPosition.B = (payload[35] << 1) | (payload[36] & 0x80) >> 7;
        DEBUG_VAR(message->dimensionOfShipReferenceForPosition.B,"%u");
        message->dimensionOfShipReferenceForPosition.C = (payload[36] & 0x7E) >> 1;
        DEBUG_VAR(message->dimensionOfShipReferenceForPosition.C,"%u");
        message->dimensionOfShipReferenceForPosition.D = ((payload[36] & 0x1) << 5) | ((payload[37] & 0xF8) >> 3);
        DEBUG_VAR(message->dimensionOfShipReferenceForPosition.D,"%u");
        message->available[f_dimensionOfShipReferenceForPosition] = (message->dimensionOfShipReferenceForPosition.A != 0)
                                                                    && (message->dimensionOfShipReferenceForPosition.B != 0)
                                                                    && (message->dimensionOfShipReferenceForPosition.C != 0)
                                                                    && (message->dimensionOfShipReferenceForPosition.D != 0);

        // Type of electronic position fixing device: 4 bits, from 295 to 298
        message->typeOfPositionDevice = (payload[37] & 0x7) | ((payload[38] & 0x1) >> 4);
        message->available[f_typeOfPositionDevice] = (message->typeOfPositionDevice != 0);
        DEBUG_VAR(message->typeOfPositionDevice,"%u");

        // RAIM-flag: 1 bit, 299
        // Data terminal ready (0 = available 1 = not available; = default): 1 bit, 300
        // Assigned mode flag: 1 bit, 301
        // (spare): 4 bits to 311

    } else {
        error = ENOMEM;
        ERROR_MSG("failed to allocate %d byte for a new AIS Message 19",sizeof(AISMessage19));
    }
    return message;

}

#undef MODULE_FLAG
#define MODULE_FLAG FLAG_AIS_MSG_20

inline AISMsg20 AISMessage20::build(const unsigned char *payload,const size_t payloadSize,int &error) {
    error = EXIT_SUCCESS;
    AISMsg20 message = AISMsg20(new (std::nothrow) AISMessage20);
    if (!message.isNull()) {
        // repeatIndicator: 2 bits, from 6 to 7 b00000011 = 0x3
        message->repeatIndicator = payload[0] & 0x3;
        message->available[f_repeatIndicator] = true;
        DEBUG_VAR(message->repeatIndicator,"%u");

        // Source ID (MMSI): 30 bits, from 8 to 37
        setValue(payload,8,30,(unsigned char *)&message->sourceID);
        message->available[f_sourceID] = true;
        DEBUG_VAR(message->sourceID,"%u");

        // (spare) 2 bit, 38 & 39

        // Offset Number 1: 12 bits, from 40 to 51
        message->offsetNumber1 = (payload[5] << 4) | (payload[6] >> 4);
        message->available[f_offsetNumber1] = (message->offsetNumber1 != 0);
        DEBUG_VAR(message->offsetNumber1,"%u");

        // Number of slots 1: 4 bits, from 52 to 55
        message->numberOfSlot1 = payload[6] & 0x0F;
        message->available[f_numberOfSlot1] = (message->numberOfSlot1 != 0);
        DEBUG_VAR(message->numberOfSlot1,"%u");

        // timeOut 1: 3 bits, from 56 to 58
        message->timeOut1 = (payload[7] & 0xE0) >> 5;
        message->available[f_timeOut1] = (message->timeOut1 != 0);
        DEBUG_VAR(message->timeOut1,"%u");

        // increment 1: 11 bits, from 59 to 70
        message->increment1 = ((payload[7] & 0x1F) << 6) | ((payload[8] & 0xFC) >> 2);
        message->available[f_increment1] = true;
        DEBUG_VAR(message->increment1,"%u");

        if (payloadSize > 72) {
            // Offset Number 2: 12 bits, from 71 to 82
            message->offsetNumber2 = ((payload[8] & 0x3) << 10) | (payload[9] << 2) | ((payload[10] & 0xC0) >> 6);
            message->available[f_offsetNumber2] = (message->offsetNumber2 != 0);
            DEBUG_VAR(message->offsetNumber2,"%u");

            // Number of slot 2: 4 bits, from 83 to 86
            message->numberOfSlot2 = (payload[10] & 0x3C) >> 2;
            message->available[f_numberOfSlot2] = (message->numberOfSlot2 != 0);
            DEBUG_VAR(message->numberOfSlot2,"%u");

            // timeOut 2: 3 bits, from 87 to 89
            message->timeOut2 = ((payload[10] & 0x03) << 1) | ((payload[11] & 0x80) >> 7);
            message->available[f_timeOut2] = (message->timeOut2 != 0);
            DEBUG_VAR(message->timeOut2,"%u");

           // increment 2: 11 bits, from 90 to 100
           message->increment2 = ((payload[11] & 0x7F) << 4) | ((payload[12] & 0xF0) >> 4);
           message->available[f_increment2] = true;
           DEBUG_VAR(message->increment2,"%u");

           if (payloadSize > 101) {
               // Offset Number 3: 12 bits, from 101 to 112
               message->offsetNumber3 = ((payload[12] & 0x0F) << 8) | payload[13];
               message->available[f_offsetNumber3] = (message->offsetNumber3 != 0);
               DEBUG_VAR(message->offsetNumber3,"%u");

               // Number of slot 3: 4 bits, from 113 to 116
               message->numberOfSlot3 = (payload[14] & 0xF0) >> 4;
               message->available[f_numberOfSlot3] = (message->numberOfSlot3 != 0);
               DEBUG_VAR(message->numberOfSlot3,"%u");

               // timeOut 3: 3 bits, from 117 to 119
               message->timeOut3 = (payload[14] & 0x0E) >> 1;
               message->available[f_timeOut3] = (message->timeOut3 != 0);
               DEBUG_VAR(message->timeOut3,"%u");

               // increment 3: 11 bits, from 120 to 130
               message->increment3 = ((payload[14] & 0x01) << 10) | (payload[15] << 2)| ((payload[16] & 0xC0) >> 6);
               message->available[f_increment3] = true;
               DEBUG_VAR(message->increment3,"%u");

               if (payloadSize > 130) {
                   // Offset Number 4: 12 bits, from 131 to 142
                   message->offsetNumber4 = ((payload[16] & 0x3F) << 6) | ((payload[17] & 0xFC) >> 2);
                   message->available[f_offsetNumber4] = (message->offsetNumber4 != 0);
                   DEBUG_VAR(message->offsetNumber4,"%u");

                   // Number of slot 4: 4 bits, from 143 to 146
                   message->numberOfSlot4 = ((payload[17] & 0x03) << 2) | ((payload[18] & 0xC0) >> 6);
                   message->available[f_numberOfSlot4] = (message->numberOfSlot4 != 0);
                   DEBUG_VAR(message->numberOfSlot4,"%u");

                   // timeOut 4: 3 bits, from 147 to 149
                   message->timeOut4 = (payload[18] & 0x38) >> 3;
                   message->available[f_timeOut4] = (message->timeOut4 != 0);
                   DEBUG_VAR(message->timeOut4,"%u");

                   // increment 4: 11 bits, from 150 to 160
                  message->increment4 = ((payload[18] & 0x07) << 8) | payload[19];
                  message->available[f_increment4] = true;
                  DEBUG_VAR(message->increment4,"%u");
               } //(payloadSize > 130)
           } //(payloadSize > 101)
        } //(payloadSize > 72)
    } else {
        error = ENOMEM;
        ERROR_MSG("failed to allocate %d byte for a new AIS Message 20",sizeof(AISMessage20));
    }
    return message;
}

#undef MODULE_FLAG
#define MODULE_FLAG FLAG_AIS_MSG_21

inline AISMsg21 AISMessage21::build(const unsigned char *payload,const size_t payloadSize,int &error) {
    error = EXIT_SUCCESS;
    AISMsg21 message = AISMsg21(new (std::nothrow) AISMessage21);
    if (!message.isNull()) {
        // repeatIndicator: 2 bits, from 6 to 7 b00000011 = 0x3
        message->repeatIndicator = payload[0] & 0x3;
        message->available[f_repeatIndicator] = true;
        DEBUG_VAR(message->repeatIndicator,"%u");

        // Source ID (MMSI): 30 bits, from 8 to 37
        setValue(payload,8,30,(unsigned char *)&message->sourceID);
        message->available[f_sourceID] = true;
        DEBUG_VAR(message->sourceID,"%u");

        // type of Aids to Navigation: 5 bits, from 38 to 42
        message->typeOfAidsToNavigation = ((payload[4] & 0x03) << 3) | ((payload[5] & 0xE0) >> 5);
        message->available[f_typeOfAidsToNavigation] = true;
        DEBUG_VAR(message->typeOfAidsToNavigation,"%u");

        // name of Aids to Navigation: 120 bits, from 43 to 162
        error = setString(payload,43,120,message->nameOfAidsToNavigation);
        message->available[f_nameOfAidsToNavigation] = ((EXIT_SUCCESS == error) && (strcmp(message->nameOfAidsToNavigation,"@@@@@@@@@@@@@@@@@@@@") != 0));
        DEBUG_VAR(message->nameOfAidsToNavigation,"%s");

        // position accuracy: 1 bit, 163
        message->positionAccuracy = ((payload[20] & 0x10) >> 4);
        message->available[f_positionAccuracy] = true;
        DEBUG_VAR(message->positionAccuracy,"%u");

        // Longitude: 28 bits, from 164 to 191, in 1/10 000 min
        long rawLongValue = 0x0;
        bool available(false);
        setValue(payload,164,28,(unsigned char *)&rawLongValue);
        message->longitude = setLongitude(rawLongValue,available);
        message->available[f_longitude] = available;
        DEBUG_VAR(message->longitude,"%f");

        // Latitude: 27 bits, from 192 to 218, in 1/10 000 min
        long rawLatValue = 0x0;
        setValue(payload,192,27,(unsigned char *)&rawLatValue);
        message->latitude = setLatitude(rawLatValue,available);
        message->available[f_latitude] = available;
        DEBUG_VAR(message->latitude,"%f");

        // dimension / reference for position: 30 bits, from 219 to 248
        // Reference point for reported position;
        // also indicates the dimension of an AtoN (m)
        //  A: 9 bits, from 219 to 227
        message->dimRefPointForPosition.A = ((payload[27] & 0x1F) << 4)| ((payload[28] & 0xF0) >> 4);
        DEBUG_VAR(message->dimRefPointForPosition.A,"%u");
        //  B: 9 bits, from 228 to 236
        message->dimRefPointForPosition.B = ((payload[28] & 0x0F) << 5)| ((payload[29] & 0xF8) >> 3);
        DEBUG_VAR(message->dimRefPointForPosition.B,"%u");
        //  C: 6 bits, from 237 to 242
        message->dimRefPointForPosition.C = ((payload[29] & 0x07) << 3)| ((payload[30] & 0xE0) >> 5);
        DEBUG_VAR(message->dimRefPointForPosition.C,"%u");
        //  D: 6 bits, from 243 to 248
        message->dimRefPointForPosition.D = ((payload[30] & 0x1F) << 1) | ((payload[31] & 0x80) >> 7);
        DEBUG_VAR(message->dimRefPointForPosition.D,"%u");
        message->available[f_dimRefPointForPosition] = (message->dimRefPointForPosition.A != 0)
                                                        && (message->dimRefPointForPosition.B != 0)
                                                        && (message->dimRefPointForPosition.C != 0)
                                                        && (message->dimRefPointForPosition.D != 0);

        // Type of electronic position fixing device: 4 bits from 249 to 252
        message->typeOfPositionDevice = ((payload[31] & 0x78) >> 3);
        message->available[f_typeOfPositionDevice] = (message->typeOfPositionDevice != 0);
        DEBUG_VAR(message->typeOfPositionDevice,"%u");

        // time stamp: 6 bits, from 253 to 258
        message->timeStamp = ((payload[31] & 0x07) << 3) | ( (payload[32] & 0xE0) >> 5);
        message->available[f_timeStamp] = (message->timeStamp != 60);
        DEBUG_VAR(message->timeStamp,"%u");

        // Off-position indicator: 1 bit, 259
        message->offPositionIndicator = (payload[32] & 0x10) >> 4;
        message->available[f_offPositionIndicator] = true;
        DEBUG_VAR(message->offPositionIndicator,"%u");

        // AtoN status: 8 bits, from 260 to 267
        message->AtoNStatus = ((payload[32] & 0x0F) << 4) | ((payload[33] & 0xF0) >> 4);
        message->available[f_AtoNStatus] = true;
        DEBUG_VAR(message->AtoNStatus,"%u");

        // RAIM Flag: 1 bit, 268
        message->RAIMFlag = (payload[33] & 0x08) >> 3;
        message->available[f_RAIMFlag] = true;
        DEBUG_VAR(message->RAIMFlag,"%u");

        // Virtual AtoN flag: 1 bit, 269
        message->virtualAtoNFlag = (payload[33] & 0x04) >> 2;
        message->available[f_virtualAtoNFlag] = true;
        DEBUG_VAR(message->virtualAtoNFlag,"%u");

        // Assigned mode flag: 1 bit, 270
        message->assignedModeFlag = (payload[33] & 0x02) >> 1;
        message->available[f_assignedModeFlag] = true;
        DEBUG_VAR(message->assignedModeFlag,"%u");

        // (spare)! 1 bit, 271
        //DEBUG_VAR(payload[33],"0x%X");

        if (payloadSize > 272) {
            // Name of Aid-to-Navigation Extension: 0 to 84 bits, from 271 to ...

            const unsigned char remainingBits = payloadSize - 272;
            const unsigned char stringLengthInChars = remainingBits / 6;
            const unsigned char stringLengthInBits = stringLengthInChars * 6;

            // This parameter of up to 14 additional 6-bit-ASCII characters for a
            // 2-slot message may be combined with the parameter “Name of Aid-to-
            // Navigation” at the end of that parameter, when more than 20 characters
            // are needed for the name of the AtoN. This parameter should be omitted
            // when no more than 20 characters for the name of the A-to-N are
            // needed in total. Only the required number of characters should be
            // transmitted, i.e. no @-character should be used
            error = setString(payload,272,stringLengthInBits,message->nameOfAidToNavigationExtension);
            message->available[f_nameOfAidToNavigationExtension] = (EXIT_SUCCESS == error);
            DEBUG_VAR(message->nameOfAidToNavigationExtension,"%s");
        } // (payloadSize > 272)
    } else {
        error = ENOMEM;
        ERROR_MSG("failed to allocate %d byte for a new AIS Message 21",sizeof(AISMessage21));
    }
    return message;
}

#undef MODULE_FLAG
#define MODULE_FLAG FLAG_AIS_MSG_22

inline AISMsg22 AISMessage22::build(const unsigned char *payload,int &error) {
    error = EXIT_SUCCESS;
    AISMsg22 message = AISMsg22(new (std::nothrow) AISMessage22);
    if (!message.isNull()) {
        // repeatIndicator: 2 bits, from 6 to 7 b00000011 = 0x3
        message->repeatIndicator = payload[0] & 0x3;
        message->available[f_repeatIndicator] = true;
        DEBUG_VAR(message->repeatIndicator,"%u");

        // Source ID (MMSI): 30 bits, from 8 to 37
        setValue(payload,8,30,(unsigned char *)&message->stationID);
        message->available[f_stationID] = true;
        DEBUG_VAR(message->stationID,"%u");

        // (spare) 2 bit, 38 & 39

        // channel A: 12 bits, from 40 to 51
        message->channelA = (payload[5] << 4) | ((payload[6] & 0xF0) >> 4);
        message->available[f_channelA] = true;
        DEBUG_VAR(message->channelA,"%u");

        // channel B: 12 bits, from 52 to 63
        message->channelB = ((payload[6] & 0x0F) << 8) | (payload[7]);
        message->available[f_channelB] = true;
        DEBUG_VAR(message->channelB,"%u");

        // Tx/Rx mode: 4 bits, from 64 to 67
        message->txRxMode = (payload[8] & 0xF0) >> 4;
        message->available[f_txRxMode] = true;
        DEBUG_VAR(message->txRxMode,"%u");

        // Power: 1 bit, 68
        message->power = (payload[8] & 0x8) >> 3;
        message->available[f_power] = true;
        DEBUG_VAR(message->power,"%u");

        // longitude 1: 18 bits, from 69 to 86
        // Longitude of area to which the assignment applies;
        // upper right corner (North-East); in 1/10 min,
        // or 18 MSBs of addressed station ID 1
        // (±180o, East = positive, West = negative) 181 = not available
        const unsigned long rawLong1Value = ((payload[8] & 0x07) << 15) | (payload[9] << 7) | ((payload[10] & 0xFE) >> 1);

        // latitude 1: 17 bits, from 87 to 103
        // Latitude of area to which the assignment applies;
        // upper right corner (North-East); in 1/10 min,
        // or 12 LSBs of addressed station ID 1, followed by 5 zero bits
        // (±90o, North = positive, South = negative) 91° = not available
        const unsigned long rawLat1Value = ((payload[10] & 0x1) << 16) | ((payload[11]) << 8) | payload[12];
        if ((payload[12] & 0x1F) == 0) {
            message->addressedStationID1 = (rawLong1Value << 12) | ((rawLat1Value & 0x1FFE0) >> 5);
            message->available[f_addressedStationID1] = true;
            DEBUG_VAR(message->addressedStationID1,"%u");
        } else {
            if (rawLong1Value & (1 << 17)) {
                message->longitude1 = - (float) ((~rawLong1Value) & (0xFFFFFFFF >> 14)) / 600.0; // 1/10 min to degrees
            } else {
                message->longitude1 = (float)rawLong1Value / 600.0; // 1/10 min to degrees
            }
            message->available[f_longitude1] = (rawLong1Value != 1810);
            DEBUG_VAR(message->longitude1,"%f");

            if (rawLat1Value & (1 << 16)) {
                message->latitude1 = - (float) ((~rawLat1Value) & (0xFFFFFFFF >> 15)) / 600.0; // 1/10 min to degrees
            } else {
                message->latitude1 = (float)rawLat1Value / 600.0; // 1/10 min to degrees
            }
            message->available[f_latitude1] = (rawLat1Value != 910);
            DEBUG_VAR(message->latitude1,"%f");
        }

        // longitude 2: 18 bits, from 104 to 111
        const unsigned long rawLong2Value = (payload[13] << 10) | (payload[14] << 2) | ((payload[15] & 0xC0) >> 6);

        // latitude 1: 17 bits, from 112 to 128
        const unsigned long rawLat2Value = ((payload[15] & 0x3F) << 11) | (payload[16] << 3) | ((payload[17] & 0xE0) >> 5);

        const Word isAddressedStationID2 = ((payload[16] & 0x3) << 3)| ((payload[17] & 0xE0) >> 5);
        if (0 == isAddressedStationID2) {
            message->addressedStationID2 = (rawLong2Value << 12) | ((rawLat2Value & 0x1FFE0) >> 5);
            message->available[f_addressedStationID2] = true;
            DEBUG_VAR(message->addressedStationID2,"%u");
        } else {
            if (rawLong2Value & (1 << 17)) {
                message->longitude2 = - (float) ((~rawLong1Value) & (0xFFFFFFFF >> 14)) / 600.0; // 1/10 min to degrees
            } else {
                message->longitude2 = (float)rawLong1Value / 600.0; // 1/10 min to degrees
            }
            message->available[f_longitude2] = (rawLong2Value != 1810);
            DEBUG_VAR(message->longitude2,"%f");

            if (rawLat2Value & (1 << 16)) {
                message->latitude2 = - (float) ((~rawLat2Value) & (0xFFFFFFFF >> 15)) / 600.0; // 1/10 min to degrees
            } else {
                message->latitude2 = (float)rawLat2Value / 600.0; // 1/10 min to degrees
            }
            message->available[f_latitude2] = (rawLat2Value != 910);
            DEBUG_VAR(message->latitude2,"%f");
        }

        // Addressed  broadcast message indicator: 1 bit, 129
        message->addressedOrBroadcastMsgIndicator = (payload[17] & 0x10) >> 4;
        message->available[f_addressedOrBroadcastMsgIndicator] = true;
        DEBUG_VAR(message->addressedOrBroadcastMsgIndicator,"%u");

        // Channel A bandwidth: 1 bit, 130
        message->channelABandwith = (payload[17] & 0x08) >> 3;
        message->available[f_channelABandwith] = true;
        DEBUG_VAR(message->channelABandwith,"%u");

        // Channel B bandwidth: 1 bit, 131
        message->channelBBandwith = (payload[17] & 0x04) >> 2;
        message->available[f_channelBBandwith] = true;
        DEBUG_VAR(message->channelBBandwith,"%u");

        // Transitional zone size: 3 bits, from 132 to 134
        // The transitional zone size in nautical miles should be calculated by
        // adding 1 to this parameter value. The default parameter value should
        // be 4, which translates to 5 nautical miles
        const unsigned char transitionalZoneSizeRawValue = (((payload[17] & 0x03) << 1) | ((payload[18] & 0x80) >> 7));
        DEBUG_VAR(transitionalZoneSizeRawValue,"%u");
        message->transitionalZoneSize =  transitionalZoneSizeRawValue + 1;
        message->available[f_transitionalZoneSize] = true;
        DEBUG_VAR(message->transitionalZoneSize,"%u nm");
    } else {
        error = ENOMEM;
        ERROR_MSG("failed to allocate %d byte for a new AIS Message 22",sizeof(AISMessage22));
    }
    return message;
}

#undef MODULE_FLAG
#define MODULE_FLAG FLAG_AIS_MSG_23

inline AISMsg23 AISMessage23::build(const unsigned char *payload,int &error) {
    error = EXIT_SUCCESS;
    AISMsg23 message = AISMsg23(new (std::nothrow) AISMessage23);
    if (!message.isNull()) {
        // repeatIndicator: 2 bits, from 6 to 7 b00000011 = 0x3
        message->repeatIndicator = payload[0] & 0x3;
        message->available[f_repeatIndicator] = true;
        DEBUG_VAR(message->repeatIndicator,"%u");

        // Source ID (MMSI): 30 bits, from 8 to 37
        setValue(payload,8,30,(unsigned char *)&message->stationID);
        message->available[f_stationID] = true;
        DEBUG_VAR(message->stationID,"%u");

        // (spare): 2 bits, 38 & 39

        // longitude 1: 18 bits, from 40 to 57
        const unsigned long rawLong1Value = (payload[5] << 10) | (payload[6] << 2) | ((payload[7] & 0xC0) >> 6);
        if (rawLong1Value & (1 << 17)) {
            message->longitude1 = - (float) ((~rawLong1Value) & (0xFFFFFFFF >> 14)) / 600.0; // 1/10 min to degrees
        } else {
            message->longitude1 = (float)rawLong1Value / 600.0; // 1/10 min to degrees
        }
        message->available[f_longitude1] = (rawLong1Value != 1810);
        DEBUG_VAR(message->longitude1,"%f");

        // latitude 1: 17 bits, from 58 to 74
        const unsigned long rawLat1Value = ((payload[7] & 0x3F) << 11) | (payload[8] << 3)| ((payload[9] & 0xE0) >> 5);
        if (rawLat1Value & (1 << 16)) {
            message->latitude1 = - (float) ((~rawLat1Value) & (0xFFFFFFFF >> 15)) / 600.0; // 1/10 min to degrees
        } else {
            message->latitude1 = (float)rawLat1Value / 600.0; // 1/10 min to degrees
        }
        message->available[f_latitude1] = (rawLat1Value != 910);
        DEBUG_VAR(message->latitude1,"%f");

        // longitude 2: 18 bits, from 75 to 92
        const unsigned long rawLong2Value = ((payload[9] & 0x1F) << 13) | ((payload[10]) << 5) | ((payload[11] & 0xF8) >> 3);
        if (rawLong2Value & (1 << 17)) {
            message->longitude2 = - (float) ((~rawLong2Value) & (0xFFFFFFFF >> 14)) / 600.0; // 1/10 min to degrees
        } else {
            message->longitude2 = (float)rawLong2Value / 600.0; // 1/10 min to degrees
        }
        message->available[f_longitude2] = (rawLong2Value != 1810);
        DEBUG_VAR(message->longitude2,"%f");

        // latitude 2: 17 bits, from 93 to 109
        const unsigned long rawLat2Value = ((payload[11] & 0x07) << 10) | (payload[12] << 2) | ((payload[13] & 0xFC) >> 2);
        if (rawLat2Value & (1 << 16)) {
            message->latitude2 = - (float) ((~rawLat2Value) & (0xFFFFFFFF >> 15)) / 600.0; // 1/10 min to degrees
        } else {
            message->latitude2 = (float)rawLat2Value / 600.0; // 1/10 min to degrees
        }
        message->available[f_latitude2] = (rawLat2Value != 910);
        DEBUG_VAR(message->latitude2,"%f");

        // Station type: 4 bits, from 110 to 113
        message->stationType = ((payload[13] & 0x3) << 2) | ((payload[14] & 0xC0) >> 6);
        message->available[f_stationType] = true;
        DEBUG_VAR(message->stationType,"%u");

        // Type of ship and cargo type: 8 bits, from 114 to 121
        message->typeOfShipAndCargoType = ((payload[14] & 0x3F) << 2) | ((payload[15] & 0xC0) >> 6);
        message->available[f_typeOfShipAndCargoType] = (message->typeOfShipAndCargoType <= 10);
        DEBUG_VAR(message->typeOfShipAndCargoType,"%u");

        // (spare): 22 bits, from 122 to 143

        // Tx/Rx mode: 2 bits, from 144 to 145
        message->txRxMode = (payload[18] & 0xC0) >> 6;
        message->available[f_txRxMode] = (message->txRxMode != 3);
        DEBUG_VAR(message->txRxMode,"%u");

        // Reporting interval: 4 bits, from 146 to 149
        message->reportingInterval = (payload[18] & 0x3C) >> 2;
        message->available[f_reportingInterval] = (message->reportingInterval < 12);
        DEBUG_VAR(message->reportingInterval,"%u");

        // Quiet time: 4 bits, from 150 to 153
        message->quietTime = ((payload[18] & 0x03) << 2) | ((payload[18] & 0xC0) >> 6);
        message->available[f_quietTime] = true;
        DEBUG_VAR(message->quietTime,"%u");

    } else {
        error = ENOMEM;
        ERROR_MSG("failed to allocate %d byte for a new AIS Message 23",sizeof(AISMessage23));
    }
    return message;
}

#undef MODULE_FLAG
#define MODULE_FLAG FLAG_AIS_MSG_24

inline int AISMessage24A::build(const unsigned char *payload) {
    int error = EXIT_SUCCESS;
    // Name: 120 bits from 40 to 159
    error = setString(payload,40,120,name);
    available[f_name] = (EXIT_SUCCESS == error);
    DEBUG_VAR(name,"%s");
    return error;
}

inline int AISMessage24B::build(const unsigned char *payload) {
    int error = EXIT_SUCCESS;

    // Type of ship and cargo type: 8 bits, from 40 to 47
    typeOfShipAndCargoType = payload[5];

    // vendorID: 42 bits from 48 to 89
    //  manufacturerID: 18 bits, from 48 to 65 or 42 bits, from 48 to 90 if not set
    setString(payload,48,42,vendorID.fields.manufacturerID);
    if (available[f_vendorID] = vendorID.is_set()) {
        // set the right values for the last 2 fields
        //  Unit Model Code: 4 bits, from 66 to 69
        vendorID.fields.unitModelcode = (payload[8] & 0x3C) >> 2;

        //  Unit Serial Number: 20 bits, from 70 to 89
        setValue(payload,70,20,(Byte*)&vendorID.fields.unitSerialNumber);
    }

    // call sign: 42 bits from 90 to 131
    error = setString(payload,90,42,callSign);
    available[f_callSign] = (EXIT_SUCCESS == true);
    DEBUG_VAR(callSign,"%s");


    // Dimensions of ship in metres and reference point for reported position
    // (see Fig. 41 and § 3.3.3). Or, for an unregistered daughter vessel, use
    // the MMSI of the associated mother ship in this data field.
    // For SAR aircraft, the use of this field may be decided by the
    // responsible administration. If used it should indicate the maximum
    // dimensions of the craft. As default should A = B = C = D be set to “0”

    // TODO: how to know if theses bits are coding dimension or a MMSI ?

    //  A: 9 bits, from 132 to 140
    dimension.A = ((payload[16] & 0xF) << 5)| ((payload[17] & 0xF8) >> 3);
    DEBUG_VAR(dimension.A,"%u");
    //  B: 9 bits, from 141 to 149
    dimension.B = ((payload[17] & 0x7) << 6) | (payload[18] & 0xFC) >> 2;
    DEBUG_VAR(dimension.B,"%u");
    //  C: 6 bits, from 150 to 155
    dimension.C = ((payload[18] & 0x3) << 4) | ((payload[19] & 0xF0) >> 4);
    DEBUG_VAR(dimension.C,"%u");
    //  D: 6 bits, from 264 269
    dimension.D = ((payload[19] & 0xF) << 2) | ((payload[20] & 0xC0) >> 6);
    DEBUG_VAR(dimension.D,"%u");
    available[f_dimension] = (dimension.A != 0) && (dimension.B != 0) && (dimension.C != 0) && (dimension.D != 0);

    return error;
}

inline AISMsg24 AISMessage24::build(const unsigned char *payload,int &error) {
    error = EXIT_SUCCESS;
    AISMsg24 message = 0;

    // common part

    // repeatIndicator: 2 bits, from 6 to 7 b00000011 = 0x3
    const unsigned char repeatIndicator = payload[0] & 0x3;
    DEBUG_VAR(repeatIndicator,"%u");

    // User ID (MMSI): 30 bits, from 8 to 37
    MMSI userID;
    setValue(payload,8,30,(unsigned char *)&userID);
    DEBUG_VAR(userID,"%u");

    // part Number: 2 bits from 38 to 39
    const unsigned char partNumber = payload[4] & 0x3;
    DEBUG_VAR(partNumber,"%u");
    switch(partNumber) {
        case 0:
            message = new (std::nothrow) AISMessage24A;
            if (!message.isNull()) {
                message->repeatIndicator = repeatIndicator;
                message->available[f_repeatIndicator] = true;
                message->userID = userID;
                message->available[f_userID] = true;
                message->partNumber = partNumber;
                message->available[f_partNumber] = true;
                error = message->build(payload);
            } else {
                error = ENOMEM;
                ERROR_MSG("failed to allocate %d byte for a new AIS Message 24A",sizeof(AISMessage24A));
            }
            break;
        case 1:
            message = new (std::nothrow) AISMessage24B;
            if (!message.isNull()) {
                message->repeatIndicator = repeatIndicator;
                message->available[f_repeatIndicator] = true;
                message->userID = userID;
                message->available[f_userID] = true;
                message->partNumber = partNumber;
                message->available[f_partNumber] = true;
                error = message->build(payload);
            } else {
                error = ENOMEM;
                ERROR_MSG("failed to allocate %d byte for a new AI Message 24B",sizeof(AISMessage24B));
            }
            break;
        default:
            ERROR_MSG("AISMessage24 has an invalid part number (%d)",partNumber);
            error = EINVAL;
            break;
    }
    return message;
}

#undef MODULE_FLAG
#define MODULE_FLAG FLAG_AIS_MSG_25

inline AISMsg25 AISMessage25::build(const unsigned char *payload,int &error) {
    error = EXIT_SUCCESS;
    AISMsg25 message = AISMsg25(new (std::nothrow) AISMessage25);
    if (!message.isNull()) {
        // repeatIndicator: 2 bits, from 6 to 7 b00000011 = 0x3
        message->repeatIndicator = payload[0] & 0x3;
        message->available[f_repeatIndicator] = true;
        DEBUG_VAR(message->repeatIndicator,"%u");

        // Source ID (MMSI): 30 bits, from 8 to 37
        setValue(payload,8,30,(unsigned char *)&message->sourceID);
        message->available[f_sourceID] = true;
        DEBUG_VAR(message->sourceID,"%u");

        // Destination indicator: 1 bit, 38
        message->destinationIndicator = (payload[4] & 0x02) >> 1;
        message->available[f_destinationIndicator] = true;
        DEBUG_VAR(message->destinationIndicator,"%u");

        // Binary Data Flag: 1 bit, 39
        message->binaryDataFlag = payload[4] & 0x01;
        message->available[f_binaryDataFlag] = true;
        DEBUG_VAR(message->binaryDataFlag,"%u");

        if (message->destinationIndicator) {
            // Destination ID: 30 bits, from 40 to 69
            message->destinationID = (payload[5] << 22) | (payload[6] << 14) | (payload[7] << 6)| (payload[8] >> 2);
            message->available[f_destinationID] = true;
            DEBUG_VAR(message->destinationID,"%u");
            if (message->binaryDataFlag) {
                // Application identifier: 16 bits, from 70 to 85
                message->applicationIdentifier = ((payload[8] & 0x03) << 14) | (payload[9] << 6) | ((payload[10] & 0xFC) >> 2);
                message->available[f_applicationIdentifier] = true;
                DEBUG_VAR(message->applicationIdentifier,"%u");

                // Application binary data: 82 bits, from 86 to 167
                setValue(payload,86,82,(unsigned char *)message->binaryData);
                message->available[f_binaryData] = true;
                message->binaryDataSize = 82;
                message->available[f_binaryDataSize] = true;
                DEBUG_VAR(message->binaryDataSize,"%u");
                DEBUG_DUMP_MEMORY(message->binaryData,(82/8)+1);
            } else {
                // Application binary data: 98 bits, from 70 to 167
                setValue(payload,70,98,(unsigned char *)message->binaryData);
                message->available[f_binaryData] = true;
                message->binaryDataSize = 98;
                message->available[f_binaryDataSize] = true;
                DEBUG_VAR(message->binaryDataSize,"%u");
                DEBUG_DUMP_MEMORY(message->binaryData,(98/8)+1);
            }
        } else {
            if (message->binaryDataFlag) {
                // Application identifier: 16 bits, from 40 to 55
                message->applicationIdentifier = (payload[5] << 8) | payload[6];
                message->available[f_applicationIdentifier] = true;
                DEBUG_VAR(message->applicationIdentifier,"%u");

                // Application binary data: 112 bits, from 56 to 167
                setValue(payload,56,112,(unsigned char *)message->binaryData);
                message->available[f_binaryData] = true;
                message->binaryDataSize = 112;
                message->available[f_binaryDataSize] = true;
                DEBUG_VAR(message->binaryDataSize,"%u");
                DEBUG_DUMP_MEMORY(message->binaryData,112/8);
            } else {
                // Application binary data: 128 bits, from 40 to 167
                setValue(payload,40,128,(unsigned char *)message->binaryData);
                message->available[f_binaryData] = true;
                message->binaryDataSize = 128;
                message->available[f_binaryDataSize] = true;
                DEBUG_VAR(message->binaryDataSize,"%u");
                DEBUG_DUMP_MEMORY(message->binaryData,(128/8));
            }
        }
    } else {
        error = ENOMEM;
        ERROR_MSG("failed to allocate %d byte for a new AIS Message 25",sizeof(AISMessage25));
    }
    return message;
}

#undef MODULE_FLAG
#define MODULE_FLAG FLAG_AIS_MSG_26

inline int AISMessage26::buildNoBinaryDataFlag(const unsigned char *payload,const size_t payloadSize) {
    int error(EXIT_SUCCESS);
    const unsigned int binaryDataStartOffset = 40;

    if (payloadSize > 1004 ) {
        // binary data: 1004 bits, from 40 to 1043
        binaryDataSize = 1004;
        available[f_binaryDataSize] = true;
        DEBUG_VAR(binaryDataSize,"%u");
        // communicationStateSelectorFlag: 1 bit, 1044
        communicationStateSelectorFlag = (payload[130] & 0x08) >> 3;
        if (communicationStateSelectorFlag) {
            // communicationState: 19 bits, from 1045 to 1063
            if (payloadSize >= 1064) {
                communicationState = ((payload[130] & 0x07) << 16) | (payload[131] << 8) | payload[132];
            } else {
                error = EINVAL;
                ERROR_MSG("Message 26 is to short to contains %u bits of binary data and 19 bits of communication state (%u)",binaryDataSize,payloadSize);
            }
        }
    } else if (payloadSize > 780) {
        // binary data: 780 bits, from 40 to 819
        binaryDataSize = 780;
        available[f_binaryDataSize] = true;
        DEBUG_VAR(binaryDataSize,"%u");
        // communicationStateSelectorFlag: 1 bit, 820
        communicationStateSelectorFlag = (payload[102] & 0x08) >> 3;
        if (communicationStateSelectorFlag) {
            // communicationState: 19 bits, from 821 to 839
            if (payloadSize >= 840) {
                communicationState = ((payload[102] & 0x07) << 16) | (payload[103] << 8) | payload[104];
            } else {
                error = EINVAL;
                ERROR_MSG("Message 26 is to short to contains %u bits of binary data and 19 bits of communication state (%u)",binaryDataSize,payloadSize);
            }
        }
    } else if (payloadSize > 560) {
        // binary data: 556 bits, from 40 to 595
        binaryDataSize = 556;
        available[f_binaryDataSize] = true;
        DEBUG_VAR(binaryDataSize,"%u");
        // communicationStateSelectorFlag: 1 bit, 596
        communicationStateSelectorFlag = (payload[74] & 0x08) >> 3;
        if (communicationStateSelectorFlag) {
            // communicationState: 19 bits, from 597 to 615
            if (payloadSize >= 616) {
                communicationState = ((payload[74] & 0x07) << 16) | (payload[75] << 8) | payload[76];
            } else {
                error = EINVAL;
                ERROR_MSG("Message 26 is to short to contains %u bits of binary data and 19 bits of communication state (%u)",binaryDataSize,payloadSize);
            }
        }
    } else if (payloadSize > 340) {
        // binary data: 332 bits, from 40 to 371
        binaryDataSize = 332;
        available[f_binaryDataSize] = true;
        DEBUG_VAR(binaryDataSize,"%u");
        // communicationStateSelectorFlag: 1 bit, 372
        communicationStateSelectorFlag = (payload[46] & 0x08) >> 3;
        if (communicationStateSelectorFlag) {
            // communicationState: 19 bits, from 373 to 391
            if (payloadSize >= 392) {
                communicationState = ((payload[46] & 0x07) << 16) | (payload[47] << 8) | payload[48];
            } else {
                error = EINVAL;
                ERROR_MSG("Message 26 is to short to contains %u bits of binary data and 19 bits of communication state (%u)",binaryDataSize,payloadSize);
            }
        }
    } else {
        // binary data: 108 bits, from 40 to 147
        binaryDataSize = 108;
        available[f_binaryDataSize] = true;
        DEBUG_VAR(binaryDataSize,"%u");
        // communicationStateSelectorFlag: 1 bit, 148
        communicationStateSelectorFlag = (payload[18] & 0x08) >> 3;
        if (communicationStateSelectorFlag) {
           // communicationState: 19 bits, from 149 to 167
            if (payloadSize >= 168) {
                communicationState = ((payload[18] & 0x07) << 16) | (payload[19] << 8) | payload[20];
            } else {
                error = EINVAL;
                ERROR_MSG("Message 26 is to short to contains %u bits of binary data and 19 bits of communication state (%u)",binaryDataSize,payloadSize);
            }
        }
    }

    if (EXIT_SUCCESS == error) {
        const size_t endOffset = binaryDataSize + binaryDataStartOffset;
        if (endOffset < payloadSize) {
            setValue(payload,binaryDataStartOffset,binaryDataSize,(unsigned char *)binaryData);
            available[f_binaryData] = true;
            DEBUG_DUMP_MEMORY(binaryData,binaryDataSize/8);
        } else {
            error = EINVAL;
            ERROR_MSG("Message 26 is to short to contain %d binary data bytes (%d >= %d)",endOffset,payloadSize);
        }
    } // error already printed
    return error;
}

inline int AISMessage26::buildBinaryDataFlag(const unsigned char *payload,const size_t payloadSize) {
    int error(EXIT_SUCCESS);
    // Application identifier: 16 bits, from 40 to 55
    applicationIdentifier = ((payload[8] & 0x03) << 14) | (payload[9] << 6) | ((payload[10] & 0xFC) >> 2);
    available[f_applicationIdentifier] = true;
    DEBUG_VAR(applicationIdentifier,"%u");

    const unsigned int binaryDataStartOffset = 56;
    if (payloadSize > 990 ) {
        // binary data: 988 bits, from 56 to 1043
        binaryDataSize = 988;
        available[f_binaryDataSize] = true;
        DEBUG_VAR(binaryDataSize,"%u");
        // communicationStateSelectorFlag: 1 bit, 1044
        communicationStateSelectorFlag = (payload[130] & 0x08) >> 3;
        if (communicationStateSelectorFlag) {
            // communicationState: 19 bits, from 1045 to 1063
            if (payloadSize >= 1064) {
                communicationState = ((payload[130] & 0x07) << 16) | (payload[131] << 8) | payload[132];
            } else {
                error = EINVAL;
                ERROR_MSG("Message 26 is to short to contains %u bits of binary data and 19 bits of communication state (%u)",binaryDataSize,payloadSize);
            }
        }
    } else if (payloadSize > 800 ) {
        // binary data: 764 bits, from 56 to 819
        binaryDataSize = 764;
        available[f_binaryDataSize] = true;
        DEBUG_VAR(binaryDataSize,"%u");
        // communicationStateSelectorFlag: 1 bit, 820
        communicationStateSelectorFlag = (payload[102] & 0x08) >> 3;
        if (communicationStateSelectorFlag) {
            // communicationState: 19 bits, from 821 to 839
            if (payloadSize >= 840) {
                communicationState = ((payload[102] & 0x07) << 16) | (payload[103] << 8) | payload[104];
            } else {
                error = EINVAL;
                ERROR_MSG("Message 26 is to short to contains %u bits of binary data and 19 bits of communication state (%u)",binaryDataSize,payloadSize);
            }
        }
    } else if (payloadSize > 550 ) {
        // binary data: 540 bits, from 56 to 595
        binaryDataSize = 540;
        available[f_binaryDataSize] = true;
        DEBUG_VAR(binaryDataSize,"%u");
        // communicationStateSelectorFlag: 1 bit, 596
        communicationStateSelectorFlag = (payload[74] & 0x08) >> 3;
        if (communicationStateSelectorFlag) {
            // communicationState: 19 bits, from 597 to 615
            if (payloadSize >= 616) {
                communicationState = ((payload[74] & 0x07) << 16) | (payload[75] << 8) | payload[76];
            } else {
                error = EINVAL;
                ERROR_MSG("Message 26 is to short to contains %u bits of binary data and 19 bits of communication state (%u)",binaryDataSize,payloadSize);
            }
        }
    } else if (payloadSize > 320 ) {
        // binary data: 316 bits, from 56 to 371
        binaryDataSize = 316;
        available[f_binaryDataSize] = true;
        DEBUG_VAR(binaryDataSize,"%u");
        // communicationStateSelectorFlag: 1 bit, 372
        communicationStateSelectorFlag = (payload[46] & 0x08) >> 3;
        if (communicationStateSelectorFlag) {
            // communicationState: 19 bits, from 373 to 391
            if (payloadSize >= 392) {
                communicationState = ((payload[46] & 0x07) << 16) | (payload[47] << 8) | payload[48];
            } else {
                error = EINVAL;
                ERROR_MSG("Message 26 is to short to contains %u bits of binary data and 19 bits of communication state (%u)",binaryDataSize,payloadSize);
            }
        }
    } else {
        // binary data: 92 bits, from 56 to 147
        binaryDataSize = 92;
        available[f_binaryDataSize] = true;
        DEBUG_VAR(binaryDataSize,"%u");
        // communicationStateSelectorFlag: 1 bit, 148
        communicationStateSelectorFlag = (payload[18] & 0x08) >> 3;
        if (communicationStateSelectorFlag) {
           // communicationState: 19 bits, from 149 to 167
            if (payloadSize >= 168) {
                communicationState = ((payload[18] & 0x07) << 16) | (payload[19] << 8) | payload[20];
            } else {
                error = EINVAL;
                ERROR_MSG("Message 26 is to short to contains %u bits of binary data and 19 bits of communication state (%u)",binaryDataSize,payloadSize);
            }
        } //(communicationStateSelectorFlag)
    }

    if (EXIT_SUCCESS == error) {
        const size_t endOffset = binaryDataSize + binaryDataStartOffset;
        if (endOffset < payloadSize) {
            setValue(payload,binaryDataStartOffset,binaryDataSize,(unsigned char *)binaryData);
            available[f_binaryData] = true;
            DEBUG_DUMP_MEMORY(binaryData,binaryDataSize/8);
        } else {
            error = EINVAL;
            ERROR_MSG("Message 26 is to short to contain %d binary data bytes (%d >= %d)",endOffset,payloadSize);
        }
    } // error already printed
    return error;
}

inline int AISMessage26::buildDestinationIndicatorNoBinaryDataFlag(const unsigned char *payload,const size_t payloadSize) {
    int error(EXIT_SUCCESS);
    const unsigned int binaryDataStartOffset = 70;

    if (payloadSize > 970 ) {
        // binary data: 974 bits, from 70 to 1043
        binaryDataSize = 974;
        available[f_binaryDataSize] = true;
        DEBUG_VAR(binaryDataSize,"%u");
        // communicationStateSelectorFlag: 1 bit, 1044
        communicationStateSelectorFlag = (payload[130] & 0x08) >> 3;
        if (communicationStateSelectorFlag) {
            // communicationState: 19 bits, from 1045 to 1063
            if (payloadSize >= 1064) {
                communicationState = ((payload[130] & 0x07) << 16) | (payload[131] << 8) | payload[132];
            } else {
                error = EINVAL;
                ERROR_MSG("Message 26 is to short to contains %u bits of binary data and 19 bits of communication state (%u)",binaryDataSize,payloadSize);
            }
        }
    } else if (payloadSize > 700 ) {
        // binary data: 750 bits, from 70 to 819
        binaryDataSize = 750;
        available[f_binaryDataSize] = true;
        DEBUG_VAR(binaryDataSize,"%u");
        // communicationStateSelectorFlag: 1 bit, 820
        communicationStateSelectorFlag = (payload[102] & 0x08) >> 3;
        if (communicationStateSelectorFlag) {
            // communicationState: 19 bits, from 821 to 839
            if (payloadSize >= 840) {
                communicationState = ((payload[102] & 0x07) << 16) | (payload[103] << 8) | payload[104];
            } else {
                error = EINVAL;
                ERROR_MSG("Message 26 is to short to contains %u bits of binary data and 19 bits of communication state (%u)",binaryDataSize,payloadSize);
            }
        }
    } else if (payloadSize > 500) {
        // binary data: 526 bits, from 70 to 595
        binaryDataSize = 526;
        available[f_binaryDataSize] = true;
        DEBUG_VAR(binaryDataSize,"%u");
        // communicationStateSelectorFlag: 1 bit, 596
        communicationStateSelectorFlag = (payload[74] & 0x08) >> 3;
        if (communicationStateSelectorFlag) {
            // communicationState: 19 bits, from 597 to 615
            if (payloadSize >= 616) {
                communicationState = ((payload[74] & 0x07) << 16) | (payload[75] << 8) | payload[76];
            } else {
                error = EINVAL;
                ERROR_MSG("Message 26 is to short to contains %u bits of binary data and 19 bits of communication state (%u)",binaryDataSize,payloadSize);
            }
        }
    } else if (payloadSize > 300) {
        // binary data: 302 bits, from 70 to 371
        binaryDataSize = 286;
        available[f_binaryDataSize] = true;
        DEBUG_VAR(binaryDataSize,"%u");
        // communicationStateSelectorFlag: 1 bit, 372
        communicationStateSelectorFlag = (payload[46] & 0x08) >> 3;
        if (communicationStateSelectorFlag) {
            // communicationState: 19 bits, from 373 to 391
            if (payloadSize >= 392) {
                communicationState = ((payload[46] & 0x07) << 16) | (payload[47] << 8) | payload[48];
            } else {
                error = EINVAL;
                ERROR_MSG("Message 26 is to short to contains %u bits of binary data and 19 bits of communication state (%u)",binaryDataSize,payloadSize);
            }
        }
    } else {
        // binary data: 78 bits, from 70 to 147
        binaryDataSize = 78;
        available[f_binaryDataSize] = true;
        DEBUG_VAR(binaryDataSize,"%u");
        // communicationStateSelectorFlag: 1 bit, 148
        communicationStateSelectorFlag = (payload[18] & 0x08) >> 3;
        if (communicationStateSelectorFlag) {
           // communicationState: 19 bits, from 149 to 167
            if (payloadSize >= 168) {
                communicationState = ((payload[18] & 0x07) << 16) | (payload[19] << 8) | payload[20];
            } else {
                error = EINVAL;
                ERROR_MSG("Message 26 is to short to contains %u bits of binary data and 19 bits of communication state (%u)",binaryDataSize,payloadSize);
            }
        }
    }

    if (EXIT_SUCCESS == error) {
        const size_t endOffset = binaryDataSize + binaryDataStartOffset;
        if (endOffset < payloadSize) {
            setValue(payload,binaryDataStartOffset,binaryDataSize,(unsigned char *)binaryData);
            available[f_binaryData] = true;
            DEBUG_DUMP_MEMORY(binaryData,binaryDataSize/8);
        } else {
            error = EINVAL;
            ERROR_MSG("Message 26 is to short to contain %d binary data bytes (%d >= %d)",endOffset,payloadSize);
        }
    } // error already printed
    return error;
}

inline int AISMessage26::buildDestinationIndicatorBinaryDataFlag(const unsigned char *payload,const size_t payloadSize) {
    int error(EXIT_SUCCESS);
    // Application identifier: 16 bits, from 70 to 85
    applicationIdentifier = ((payload[8] & 0x03) << 14) | (payload[9] << 6) | ((payload[10] & 0xFC) >> 2);
    available[f_applicationIdentifier] = true;
    DEBUG_VAR(applicationIdentifier,"%u");

    const unsigned int binaryDataStartOffset = 86;
    if (payloadSize > 1000 ) {
        // binary data: 958 bits, from 86 to 1043
        binaryDataSize = 958;
        available[f_binaryDataSize] = true;
        DEBUG_VAR(binaryDataSize,"%u");
        // communicationStateSelectorFlag: 1 bit, 1044
        communicationStateSelectorFlag = (payload[130] & 0x08) >> 3;
        if (communicationStateSelectorFlag) {
            // communicationState: 19 bits, from 1045 to 1063
            if (payloadSize >= 1064) {
                communicationState = ((payload[130] & 0x07) << 16) | (payload[131] << 8) | payload[132];
            } else {
                error = EINVAL;
                ERROR_MSG("Message 26 is to short to contains %u bits of binary data and 19 bits of communication state (%u)",binaryDataSize,payloadSize);
            }
        }
    } else if (payloadSize > 800 ) {
        // binary data: 734 bits, from 86 to 819
        binaryDataSize = 734;
        available[f_binaryDataSize] = true;
        DEBUG_VAR(binaryDataSize,"%u");
        // communicationStateSelectorFlag: 1 bit, 820
        communicationStateSelectorFlag = (payload[102] & 0x08) >> 3;
        if (communicationStateSelectorFlag) {
            // communicationState: 19 bits, from 821 to 839
            if (payloadSize >= 840) {
                communicationState = ((payload[102] & 0x07) << 16) | (payload[103] << 8) | payload[104];
            } else {
                error = EINVAL;
                ERROR_MSG("Message 26 is to short to contains %u bits of binary data and 19 bits of communication state (%u)",binaryDataSize,payloadSize);
            }
        }
    } else if (payloadSize > 500 ) {
        // binary data: 510 bits, from 86 to 595
        binaryDataSize = 510;
        available[f_binaryDataSize] = true;
        DEBUG_VAR(binaryDataSize,"%u");
        // communicationStateSelectorFlag: 1 bit, 596
        communicationStateSelectorFlag = (payload[74] & 0x08) >> 3;
        if (communicationStateSelectorFlag) {
            // communicationState: 19 bits, from 597 to 615
            if (payloadSize >= 616) {
                communicationState = ((payload[74] & 0x07) << 16) | (payload[75] << 8) | payload[76];
            } else {
                error = EINVAL;
                ERROR_MSG("Message 26 is to short to contains %u bits of binary data and 19 bits of communication state (%u)",binaryDataSize,payloadSize);
            }
        }
    } else if (payloadSize > 300 ) {
        // binary data: 286 bits, from 86 to 371
        binaryDataSize = 286;
        available[f_binaryDataSize] = true;
        DEBUG_VAR(binaryDataSize,"%u");
        // communicationStateSelectorFlag: 1 bit, 372
        communicationStateSelectorFlag = (payload[46] & 0x08) >> 3;
        if (communicationStateSelectorFlag) {
            // communicationState: 19 bits, from 373 to 391
            if (payloadSize >= 392) {
                communicationState = ((payload[46] & 0x07) << 16) | (payload[47] << 8) | payload[48];
            } else {
                error = EINVAL;
                ERROR_MSG("Message 26 is to short to contains %u bits of binary data and 19 bits of communication state (%u)",binaryDataSize,payloadSize);
            }
        }
    } else {
        // binary data: 62 bits, from 86 to 147
        binaryDataSize = 62;
        available[f_binaryDataSize] = true;
        DEBUG_VAR(binaryDataSize,"%u");
        // communicationStateSelectorFlag: 1 bit, 148
        communicationStateSelectorFlag = (payload[18] & 0x08) >> 3;
        if (communicationStateSelectorFlag) {
           // communicationState: 19 bits, from 149 to 167
            if (payloadSize >= 168) {
                communicationState = ((payload[18] & 0x07) << 16) | (payload[19] << 8) | payload[20];
            } else {
                error = EINVAL;
                ERROR_MSG("Message 26 is to short to contains %u bits of binary data and 19 bits of communication state (%u)",binaryDataSize,payloadSize);
            }
        } //(communicationStateSelectorFlag)
    }

    if (EXIT_SUCCESS == error) {
        const size_t endOffset = binaryDataSize + binaryDataStartOffset;
        if (endOffset < payloadSize) {
            setValue(payload,binaryDataStartOffset,binaryDataSize,(unsigned char *)binaryData);
            available[f_binaryData] = true;
            DEBUG_DUMP_MEMORY(binaryData,binaryDataSize/8);
        } else {
            error = EINVAL;
            ERROR_MSG("Message 26 is to short to contain %d binary data bytes (%d >= %d)",endOffset,payloadSize);
        }
    } // error already printed
    return error;
}

inline AISMsg26 AISMessage26::build(const unsigned char *payload,const size_t payloadSize,int &error) {
    error = EXIT_SUCCESS;
    AISMsg26 message = AISMsg26(new (std::nothrow) AISMessage26);
    if (!message.isNull()) {
        // repeatIndicator: 2 bits, from 6 to 7 b00000011 = 0x3
        message->repeatIndicator = payload[0] & 0x3;
        message->available[f_repeatIndicator] = true;
        DEBUG_VAR(message->repeatIndicator,"%u");

        // Source ID (MMSI): 30 bits, from 8 to 37
        setValue(payload,8,30,(unsigned char *)&message->sourceID);
        message->available[f_sourceID] = true;
        DEBUG_VAR(message->sourceID,"%u");

        // Destination indicator: 1 bit, 38
        message->destinationIndicator = (payload[4] & 0x02) >> 1;
        message->available[f_destinationIndicator] = true;
        DEBUG_VAR(message->destinationIndicator,"%u");

        // Binary Data Flag: 1 bit, 39
        message->binaryDataFlag = payload[4] & 0x01;
        message->available[f_binaryDataFlag] = true;
        DEBUG_VAR(message->binaryDataFlag,"%u");

        if (message->destinationIndicator) {
            // Destination ID: 30 bits, from 40 to 69
            message->destinationID = (payload[5] << 22) | (payload[6] << 14) | (payload[7] << 6)| (payload[8] >> 2);
            message->available[f_destinationID] = true;
            DEBUG_VAR(message->destinationID,"%u");

            if (message->binaryDataFlag) {
                error = message->buildDestinationIndicatorBinaryDataFlag(payload,payloadSize);
            } else { // (message->binaryDataFlag)
                error = message->buildDestinationIndicatorNoBinaryDataFlag(payload,payloadSize);
            }
        } else  { //(message->destinationIndicator)
            if (message->binaryDataFlag) {
                error = message->buildBinaryDataFlag(payload,payloadSize);
            } else {
                error = message->buildNoBinaryDataFlag(payload,payloadSize);
            }
//#error to be continued...
        }
    } else {
        error = ENOMEM;
        ERROR_MSG("failed to allocate %d byte for a new AIS Message 26",sizeof(AISMessage25));
    }
    return message;
}

#undef MODULE_FLAG
#define MODULE_FLAG FLAG_AIS_MSG_27

inline AISMsg27 AISMessage27::build(const unsigned char *payload,int &error) {
    error = EXIT_SUCCESS;
    AISMsg27 message = AISMsg27(new (std::nothrow) AISMessage27);
    if (!message.isNull()) {
        // Messages 11: Base station report
        // cf. R-REC-M.1371-4-201004-E p 135 for details

        const unsigned char *cursor = payload;
        // ID 6 bits from 0 to 5

        // repeatIndicator: 2 bits, from 6 to 7 b00000011 = 0x3
        message->repeatIndicator = payload[0] & 0x3;
        message->available[f_repeatIndicator] = true;
        DEBUG_VAR(message->repeatIndicator,"%u");

        // User ID (MMSI): 30 bits, from 8 to 37
        setValue(payload,8,30,(unsigned char *)&message->userID);
        message->available[f_userID] = true;
        DEBUG_VAR(message->userID,"%u");

        // Position accuracy: 1bit, 38
        message->positionAccuracy = (payload[4] & 0x2) >> 1;
        message->available[f_positionAccuracy] = true;
        DEBUG_VAR(message->positionAccuracy,"%u");

        // RAIM flag: 1 bit, 39
        message->RAIMFlag = payload[4] & 0x1;
        message->available[f_RAIMFlag] = true;

        // Navigational status: 4 bits, from 40 to 43
        message->navigationStatus = (payload[5] & 0xF0) >> 4;
        message->available[f_navigationStatus] = (message->navigationStatus != 15);
        DEBUG_VAR(message->navigationStatus,"%u");

        // Longitude: 18 bits, from 44 to 61, in 1/10 min (±180o, East = positive, West = negative
        long rawLongValue = 0x0;
        setValue(payload,44,18,(Byte*)&rawLongValue);
        if (rawLongValue & (1 << 17)) {
            message->longitude = - (float) ((~rawLongValue) & (0xFFFFFFFF >> 14)) / 600.0; // 1/10 min to degrees
        } else {
            message->longitude = (float)rawLongValue / 600.0; // 1/10 min to degrees
        }
        message->available[f_longitude] = true;
        DEBUG_VAR(message->longitude,"%f");

        // Latitude: 17 bits, from 62 to 78, in 1/10 min (±90o, North = positive, South = negative
        long rawLatValue = 0x0;
        setValue(payload,62,17,(unsigned char *)&rawLatValue);
        if (rawLatValue & (1 << 16)) {
            message->latitude = - (float) ((~rawLatValue) & (0xFFFFFFFF >> 15)) / 600.0; // 1/10 min to degrees
        } else {
            message->latitude = (float)rawLatValue / 600.0; // 1/10 min to degrees
        }
        message->available[f_latitude] = true;
        DEBUG_VAR(message->latitude,"%f");

        // Speed Over Ground: 6 bits, from 79 to 84, Knots (0-62); 63 = not available = default
        message->speedOverGround = ((payload[9] & 0x1) << 5) | ((payload[10] & 0xF8) >> 3);
        message->available[f_courseOverGround] = (message->speedOverGround != 63);
        DEBUG_VAR(message->speedOverGround,"%f");

        // Course Over Ground: 9 bits, from 85 to 93, Degrees (0-359); 511 = not available = default
        message->courseOverGround = ((payload[10] & 0x7) << 6)| ((payload[11] & 0xFC) >> 2);
        message->available[f_courseOverGround] = (message->courseOverGround != 511);
        DEBUG_VAR(message->courseOverGround,"%f");

        // Status of current GNSS position: 1 bit (94),
        // 0 = Position is the current GNSS position; 1 = Reported position is not the current GNSS position = default
        message->statusOfCurrentGNSSPosition = (payload[11] & 0x2) >> 1;
        message->available[f_statusOfCurrentGNSSPosition] = true;
        DEBUG_VAR(message->statusOfCurrentGNSSPosition,"%u");

    } else {
        error = ENOMEM;
        ERROR_MSG("failed to allocate %d byte for a new AIS Message 27",sizeof(AISMessage27));
    }
    return message;

}

#undef MODULE_FLAG
#define MODULE_FLAG FLAG_AIS_ALARMS

//$AIALR,142904.00,001,A,V,AIS: Tx malfunction*56
//$AIALR,142905.00,002,A,V,AIS: Antenna VSWR exceeds limit*59
//$AIALR,142906.00,003,A,V,AIS: Rx channel 1 malfunction*02
//$AIALR,142906.00,004,A,V,AIS: Rx channel 2 malfunction*06
//$AIALR,142907.00,006,A,V,AIS: General failure*2A
//$AIALR,142907.00,025,A,V,AIS: External EPFS lost*04
//$AIALR,142908.00,026,A,V,AIS: No sensor position in use*7B
//$AIALR,142908.00,029,A,V,AIS: No valid SOG information*7E
//$AIALR,142908.00,030,A,V,AIS: No valid COG information*66
//       00000000001111111111222222222233333333334444444444
//       01234567890123456789012345678901234567890123456789
static inline time_t getTime(const char *payload) {
    time_t currentTime(0);
    struct tm baseTime;

    time(&currentTime);
    if (gmtime_r(&currentTime,&baseTime) == NULL) {
        memset(&baseTime,0,sizeof(baseTime));
        ERROR_MSG("gmtime_r error");
    }
    baseTime.tm_hour = (payload[0] - '0') * 10 + (payload[1] - '0');
    DEBUG_VAR(baseTime.tm_hour,"%d");
    baseTime.tm_min = (payload[2] - '0') * 10 + (payload[3] - '0');
    DEBUG_VAR(baseTime.tm_min,"%d");
    baseTime.tm_sec = (payload[4] - '0') * 10 + (payload[5] - '0');
    DEBUG_VAR(baseTime.tm_sec,"%d");
    return mktime(&baseTime);
}

Alarm AISAlarm::build(const char *payload,int &error) {
    Alarm message = Alarm(new (std::nothrow) AISAlarm);
    if (!message.isNull()) {
        message->timeOfAlarmCondictionChange = getTime(payload);
        DEBUG_VAR(message->timeOfAlarmCondictionChange,"%u");
        message->id = (payload[10] - '0') * 100 + (payload[11] - '0') * 10 + (payload[12] - '0');
        DEBUG_VAR(message->id,"%u");
        message->condition = payload[14];
        DEBUG_VAR(message->condition,"%u");
        message->state = payload[16];
        DEBUG_VAR(message->state,"%u");
        message->description = payload + 18;
        const unsigned end = message->description.rfind('*');
        if (end != std::string::npos) {
            message->description.erase(end);
        }
        DEBUG_VAR(message->description.c_str(),"%s");
    } else {
        error = ENOMEM;
        ERROR_MSG("failed to allocate %d byte for a new AIS alarm message",sizeof(AISAlarm));
    }
    return message;
}

#undef MODULE_FLAG
#define MODULE_FLAG FLAG_AIS_RUNTIME

int AISMessage::AIVDMParser(const char *payload6bits) {
	int error(EXIT_SUCCESS);
	/* The maximum number of Message Data bits, that can be contained in an AIS radio message,
	 * is 1008 bits. This number of bits requires 168 6-bits symbols.
	 * 1008 / 8 = 126 bytes
	 */
	unsigned char payload8bits[128];
	size_t payloadSizeInBits(0);
	std::memset(payload8bits,0,sizeof(payload8bits));

	error = decodePayload(payload6bits,payload8bits,payloadSizeInBits);
	if (EXIT_SUCCESS == error) {
		const unsigned char messageID = (payload8bits[0] & 0xFC)>>2;// bits 0->5 (b11111100 ) message ID
		// cf. R-REC-M.1371-4-201004-E p 98 table 43 for the defined messages list
		DEBUG_VAR(messageID,"%u");
		switch(messageID) {
            case  1: { // Position Report Class A
                    AISMsg1 msg = AISMessage1::build(payload8bits,error);
                    if (EXIT_SUCCESS == error) {
                        // publish it
                        message1(msg,customParameter);
                    }
                }
                break;
            case  2: { // Position Report Class A (Assigned schedule)
                    AISMsg2 msg = AISMessage2::build(payload8bits,error);
                    if (EXIT_SUCCESS == error) {
                        // publish it
                        message2(msg,customParameter);
                    }
                }
                break;
            case  3: { //	Position Report Class A (Response to interrogation)
                    AISMsg3 msg = AISMessage3::build(payload8bits,error);
                    if (EXIT_SUCCESS == error) {
                        // publish it
                        message3(msg,customParameter);
                    }
                }
                break;
            case  4: { //	Base Station Report
                    AISMsg4 msg = AISMessage4::build(payload8bits,error);
                    if (EXIT_SUCCESS == error) {
                        // publish it
                        message4(msg,customParameter);
                    }
                }
                break;
            case  5: { //	Static and Voyage Related Data
                    AISMsg5 msg = AISMessage5::build(payload8bits,error);
                    if (EXIT_SUCCESS == error) {
                        // publish it
                        message5(msg,customParameter);
                    }
                }
                break;
            case  6: { //	Binary Addressed Message
                    AISMsg6 msg = AISMessage6::build(payload8bits,payloadSizeInBits,error);
                    if (EXIT_SUCCESS == error) {
                        // publish it
                        message6(msg,customParameter);
                    }
                }
                break;
            case  7: { //	Binary Acknowledge
                    AISMsg7 msg = AISMessage7::build(payload8bits,payloadSizeInBits,error);
                    if (EXIT_SUCCESS == error) {
                        // publish it
                        message7(msg,customParameter);
                    }
                }
                break;
            case  8: { //	Binary Broadcast Message
                    AISMsg8 msg = AISMessage8::build(payload8bits,payloadSizeInBits,error);
                    if (EXIT_SUCCESS == error) {
                        // publish it
                        message8(msg,customParameter);
                    }
                }
                break;
            case  9: { //	Standard SAR Aircraft Position Report
                    AISMsg9 msg = AISMessage9::build(payload8bits,error);
                    if (EXIT_SUCCESS == error) {
                        // publish it
                        message9(msg,customParameter);
                    }
                }
                break;
            case 10: { //	UTC and Date Inquiry
                    AISMsg10 msg = AISMessage10::build(payload8bits,error);
                    if (EXIT_SUCCESS == error) {
                        // publish it
                        message10(msg,customParameter);
                    }
                }
                break;
            case 11: { //	UTC and Date Response
                    AISMsg11 msg = AISMessage11::build(payload8bits,error);
                    if (EXIT_SUCCESS == error) {
                        // publish it
                        message11(msg,customParameter);
                    }
                }
                break;
            case 12: { //	Addressed Safety Related Message
                    AISMsg12 msg = AISMessage12::build(payload8bits,payloadSizeInBits,error);
                    if (EXIT_SUCCESS == error) {
                        // publish it
                        message12(msg,customParameter);
                    }
                }
                break;
            case 13: { //	Safety Related Acknowledgment
                    AISMsg13 msg = AISMessage13::build(payload8bits,payloadSizeInBits,error);
                    if (EXIT_SUCCESS == error) {
                        // publish it
                        message13(msg,customParameter);
                    }
                }
                break;
            case 14: { //	Safety Related Broadcast Message
                    AISMsg14 msg = AISMessage14::build(payload8bits,payloadSizeInBits,error);
                    if (EXIT_SUCCESS == error) {
                        // publish it
                        message14(msg,customParameter);
                    }
                }
                break;
            case 15: { //	Interrogation
                    AISMsg15 msg = AISMessage15::build(payload8bits,payloadSizeInBits,error);
                    if (EXIT_SUCCESS == error) {
                        // publish it
                        message15(msg,customParameter);
                    }
                }
                break;
            case 16: { //	Assignment Mode Command
                    AISMsg16 msg = AISMessage16::build(payload8bits,payloadSizeInBits,error);
                    if (EXIT_SUCCESS == error) {
                        // publish it
                        message16(msg,customParameter);
                    }
                //INFO_MSG("message %u (%s) not yet decoded",messageID,payload6bits);
                }
                break;
            case 17: { //	DGNSS Binary Broadcast Message
                    AISMsg17 msg = AISMessage17::build(payload8bits,payloadSizeInBits,error);
                    if (EXIT_SUCCESS == error) {
                        // publish it
                        message17(msg,customParameter);
                    }
                //INFO_MSG("message %u (%s) not yet decoded",messageID,payload6bits);
                }
                break;
            case 18: { //	Standard Class B CS Position Report
                    AISMsg18 msg = AISMessage18::build(payload8bits,error);
                    if (EXIT_SUCCESS == error) {
                        // publish it
                        message18(msg,customParameter);
                    }
                    //INFO_MSG("message %u (%s) not yet decoded",messageID,payload6bits);
                }
                break;
            case 19: { //	Extended Class B Equipment Position Report
                    AISMsg19 msg = AISMessage19::build(payload8bits,error);
                    if (EXIT_SUCCESS == error) {
                        // publish it
                        message19(msg,customParameter);
                    }
                //INFO_MSG("message %u (%s) not yet decoded",messageID,payload6bits);
                }
                break;
            case 20: { //	Data Link Management
                    AISMsg20 msg = AISMessage20::build(payload8bits,payloadSizeInBits,error);
                    if (EXIT_SUCCESS == error) {
                        // publish it
                        message20(msg,customParameter);
                    }
                    //INFO_MSG("message %u (%s) not yet decoded",messageID,payload6bits);
                }
                break;
            case 21: { //	Aid-to-Navigation Report
                    AISMsg21 msg = AISMessage21::build(payload8bits,payloadSizeInBits,error);
                    if (EXIT_SUCCESS == error) {
                        // publish it
                        message21(msg,customParameter);
                    }
                    //INFO_MSG("message %u (%s) not yet decoded",messageID,payload6bits);
                }
                break;
            case 22: { //	Channel Management
                    AISMsg22 msg = AISMessage22::build(payload8bits,error);
                    if (EXIT_SUCCESS == error) {
                        // publish it
                        message22(msg,customParameter);
                    }
                    //INFO_MSG("message %u (%s) not yet decoded",messageID,payload6bits);
                }
                break;
            case 23: { //	Group Assignment Command
                    AISMsg23 msg = AISMessage23::build(payload8bits,error);
                    if (EXIT_SUCCESS == error) {
                        // publish it
                        message23(msg,customParameter);
                    }
                    //INFO_MSG("message %u (%s) not yet decoded",messageID,payload6bits);
                }
                break;
            case 24: { //	Static Data Report
                    AISMsg24 msg = AISMessage24::build(payload8bits,error);
                    if (EXIT_SUCCESS == error) {
                        // publish it
                        message24(msg,customParameter);
                        if (msg->is_AISMessage24A()) {
                            AISMsg24A msg24A = msg.down_cast<AISMessage24A>();
                            message24A(msg24A,customParameter);
                        } else if (msg->is_AISMessage24B()) {
                            AISMsg24B msg24AB = msg.down_cast<AISMessage24B>();
                            message24B(msg24AB,customParameter);
                        }
                    } // error already printed
                }
                break;
            case 25: { //	Single Slot Binary Message,
                    AISMsg25 msg = AISMessage25::build(payload8bits,error);
                    if (EXIT_SUCCESS == error) {
                        // publish it
                        message25(msg,customParameter);
                    }
                    //INFO_MSG("message %u (%s) not yet decoded",messageID,payload6bits);
                }
                break;
            case 26: { //	Multiple Slot Binary Message With Communications State
                    AISMsg26 msg = AISMessage26::build(payload8bits,payloadSizeInBits,error);
                    if (EXIT_SUCCESS == error) {
                        // publish it
                        message26(msg,customParameter);
                    }
                    //INFO_MSG("message %u (%s) not yet decoded",messageID,payload6bits);
                }
                break;
            case 27: { //	Position Report For Long-Range Applications
                    AISMsg27 msg = AISMessage27::build(payload8bits,error);
                    if (EXIT_SUCCESS == error) {
                        // publish it
                        message27(msg,customParameter);
                    }
                }
                break;
            default:
                error = EINVAL;
                break;
		} //switch(messageID)
	} // (decodePayload(message,messageData) == error)

	return error;
}

inline int AISMessage::AIVDOParser(const char *payload6bits) {
    return AIVDMParser(payload6bits);
}

static inline unsigned int getFormatterMnemonicCode(const char *message) {
	// warning: this function doesn't manage query sentences nor proprietary sentence
	// address field: aaccc where ccc is the formatter mnemonic code
	const char *p = message + 3; // to skip the start of sequence character and the talker
	const unsigned int formatter = *((unsigned int*)p) & 0x00FFFFFF;
	return formatter;
}

static inline unsigned char getFieldNumberValue(const char *message,const char **newPosition) {
	unsigned char n = 0;
	register const char *fieldSeparatorPosition = message;
	while(*fieldSeparatorPosition != ',') {
		n = (n * 10) + (*fieldSeparatorPosition) - '0';
		fieldSeparatorPosition++;
	}
	*newPosition = fieldSeparatorPosition;
	return n;
}

static inline unsigned char getFieldCharacterValue(const char *message,const char **newPosition, const char defaultValue = '\0') {
	char character = defaultValue;
	register const char *fieldSeparatorPosition = message;
	if (*fieldSeparatorPosition != ',') {
		character = *fieldSeparatorPosition;
		fieldSeparatorPosition++;
	}
	*newPosition = fieldSeparatorPosition;
	return character;
}

int AIVDOMessage::AIVDMParser(const char *message) {
    DEBUG_MSG("VDM Messages are not published to VDO subscribers");
    return EXIT_SUCCESS;
}

int AIVDMMessage::AIVDOParser(const char *message) {
    DEBUG_MSG("VDO Messages are not published to VDM subscribers");
    return EXIT_SUCCESS;
}

/*
 * AIS's VDM & VDO messages structure
 *
 * !AIVDO,1,1,,,900082PH@0Oc9q4KcP3008h205A4,0*07
 * !AIVDM,1,1,,B,14W1qJ0OivOalFlKVgbR4AO<2@Ed,0*6A
 *
 * Field 1, !AIVDM, identifies this as an AIVDM packet.
 * Field 2 (1 in this example) is the count of fragments in the currently accumulating message.
 *          The payload size of each sentence is limited by NMEA 0183�s 82-character maximum,
 *          so it is sometimes required to split a payload over several fragment sentences.
 * Field 3 (1 in this example) is the fragment number of this sentence. It will be one-based.
 *          A sentence with a fragment count of 1 and a fragment number of 1 is complete in itself.
 * Field 4 (empty in this example) is a sequential message ID for multi-sentence messages.
 *         The "message sequential identifier" is a number from 0 to 9 that is sequentially assigned as needed.
 *         This identifier is incremented for each new multi-sentence message. The count resets to 0, after 9 is used.
 *         For radio broadcast messages requiring multiple sentences, each sentence of the message contains the same sequential
 *         identification number. The purpose of this number is to link the separate sentences containing portions of the same
 *         radio message. This allows for the possibility that other sentences might be interleaved with the message sentences
 *         that contain the complete message contents. This number also links an ABK-sentence acknowledgment to the appropriate BBM-sentence.
 * Field 5 (B in this example) is a radio channel code. AIS uses the high side of the duplex from two VHF radio channels:
 *         AIS Channel A is 161.975Mhz (87B); AIS Channel B is 162.025Mhz (88B).
 * Field 6 (177KQJ5000G?tO`K>RA1wUbN0TKH in this example) is the data payload.
 * Field 7 (0) is the number of fill bits requires to pad the data payload to a 6 bit boundary, ranging from 0 to 5.
 *         Equivalently, subtracting 5 from this tells how many least significant bits of the last 6-bit nibble in the
 *         data payload should be ignored. Note that this pad byte has a tricky interaction with the <[ITU-1371]>
 *         requirement for byte alignment in over-the-air AIS messages; see the detailed discussion of message lengths
 *         and alignment in a later section.
 * The *-separated suffix (*5C) is the NMEA 0183 data-integrity checksum for the sentence, preceded by "*".
 *         It is computed on the entire sentence including the AIVDM tag but excluding the leading "!".
 */
int AISMessage::parseBangMessage(const char *message) {
	int error = EXIT_SUCCESS;
	const unsigned int formatterMnemonicCode = getFormatterMnemonicCode(message);
	register const char *currentPosition = strchr(message + 4,',');
	if (currentPosition != NULL) {
		if (VHF_DATA_LINK_MSG_FORMATTER == formatterMnemonicCode) {
            ++currentPosition;
            const unsigned char countOfFragments = getFieldNumberValue(currentPosition,&currentPosition); /* Field 2 */
            ++currentPosition;
            const unsigned char fragmentNumber = getFieldNumberValue(currentPosition,&currentPosition); /* Field 3 */
            ++currentPosition;
            const unsigned char sequentialMessageID = getFieldNumberValue(currentPosition,&currentPosition); /* Field 4 */
            ++currentPosition;
            const char radioChannel = getFieldCharacterValue(currentPosition,&currentPosition); /* Field 5 */
            ++currentPosition;
            if (1 == countOfFragments) {
                error = AIVDMParser(currentPosition);
            } else {
                //TODO: better multi part messages handling (threadsafe...)
                static AISFragmentsMgr fragmentsMgr;
                if (('A' == radioChannel) || ('B' == radioChannel)) {
                    const char *fullPayload = fragmentsMgr.addSegment(radioChannel,sequentialMessageID,countOfFragments,fragmentNumber,currentPosition);
                    if (fullPayload) {
                        error = AIVDMParser(fullPayload);
                    }
                } else {
                    ERROR_MSG("invalid radio channel code (%c) in %s",radioChannel,message);
                    error = EINVAL;
                }
            }
		} else if (VHF_DATA_LINK_OWN_VESSEL_MSG_FORMATTER == formatterMnemonicCode) {
            ++currentPosition;
            const unsigned char countOfFragments = getFieldNumberValue(currentPosition,&currentPosition); /* Field 2 */
            ++currentPosition;
            const unsigned char fragmentNumber = getFieldNumberValue(currentPosition,&currentPosition); /* Field 3 */
            ++currentPosition;
            const unsigned char sequentialMessageID = getFieldNumberValue(currentPosition,&currentPosition); /* Field 4 */
            ++currentPosition;
            const char radioChannel = getFieldCharacterValue(currentPosition,&currentPosition); /* Field 5 */
            ++currentPosition;
            if (1 == countOfFragments) {
                error = AIVDOParser(currentPosition);
            } else {
                //TODO: better multi part messages handling (threadsafe...)
                static AISFragmentsMgr fragmentsMgr;
                if (('A' == radioChannel) || ('B' == radioChannel)) {
                    const char *fullPayload = fragmentsMgr.addSegment(radioChannel,sequentialMessageID,countOfFragments,fragmentNumber,currentPosition);
                    if (fullPayload) {
                        error = AIVDOParser(fullPayload);
                    }
                } else {
                    ERROR_MSG("invalid radio channel code (%c) in %s",radioChannel,message);
                    error = EINVAL;
                }
            }
		} else {
		    NOTICE_MSG("Message %s not handled",message);
		}
	} else {
		ERROR_MSG("message %s is invalid: no field separator found !",message);
		error = EBADMSG;
	}
	return error;
}

int AISMessage::parseDollardMessage(const char *message) {
    int error = EXIT_SUCCESS;
    const unsigned int formatterMnemonicCode = getFormatterMnemonicCode(message);
    register const char *currentPosition = strchr(message + 4,',');
    if (currentPosition != NULL) {
        if (SET_ALARM_STATE_FORMATTER == formatterMnemonicCode) {
            currentPosition++;
            Alarm msg = AISAlarm::build(currentPosition,error);
            if (EXIT_SUCCESS == error) {
                // publish it
                alarm(msg,customParameter);
            }
        } else {
            NOTICE_MSG("Message %s not handled",message);
        }
    } else {
        ERROR_MSG("message %s is invalid: no field separator found !",message);
        error = EBADMSG;
    }
    return error;
}

