#include "globals.h"
#include "IEC61162/IEC-61162.h"
#include <typeinfo>

using namespace IEC61162;

#define MODULE_FLAG FLAG_RUNTIME

static inline unsigned short getTalker(const char *message) {
	// address field: aaccc where aa is the talker
	unsigned short talker = 0x0;
	const char *p = message + 1;
	if ('P' == *p) { // proprietary sentence ID
		talker = PROPRIETARY_MSG_IDENTIFIER;
	} else {
		unsigned short *t = (unsigned short *)p;
		talker = *t;
	}
	return talker;
}

// $AIALR,000002.00,025,A,V,AIS: External EPFS lost*0F
// $GPZDA,143017.00,07,06,2013*67
// $GPGGA,143017.00,5004.0689,N,00955.4682,E,1,,,1000,M,,*39
// $GPVTG,94.3,T,,,50.0,N,,*43
inline int Message::parseDollardMessage(const char *message) {
	int error = EXIT_SUCCESS;
	const unsigned short talker = getTalker(message);
	if (PROPRIETARY_MSG_IDENTIFIER == talker) {
		// publish the sentence
		std::string *msg = new (std::nothrow) std::string(message);
		if (msg != 0) {
			raw_sentence proprietary_message(msg);
			proprietary_sentence(proprietary_message,customParameter);
		} else {
			ERROR_MSG("failed to allocate a std::string to publish the proprietary message %s",message);
			error = ENOMEM;
		}
	} else if (AIS_MSG_IDENTIFIER == talker) {
	    AISMessage *aisMsg = dynamic_cast<AISMessage *>(this);
        if (aisMsg != 0) {
            error = aisMsg->parseDollardMessage(message);
        } else {
            // not an error: AIS messages are managed ONLY when using the AISMessage class
            INFO_MSG("Unhandled AIS Message %s received and discarded",message);
        }
	} else {
    	error = EINVAL;
    }
	return error;
}

// !AIVDO,1,1,,,900082PH@0Oc9q4KcP3009@205A4,0*2E
// !AIVDM,1,1,,B,13Hj5J7000Od<fdKQJ3Iw`S>28FK,0*3F

inline int Message::parseBangMessage(const char *message) {
	int error = EXIT_SUCCESS;
    const unsigned short talker = getTalker(message);

    if (AIS_MSG_IDENTIFIER == talker) {
        AISMessage *aisMsg = dynamic_cast<AISMessage *>(this);
        if (aisMsg != 0) {
            error = aisMsg->parseBangMessage(message);
        } else {
            // not an error: AIS messages are managed ONLY when using the AISMessage class
            INFO_MSG("Unhandled AIS Message %s received and discarded",message);
        }
    } else {
    	error = EINVAL;
    }

	return error;
}

/*
 * the absolute value calculated by exclusive-OR'ing the eight data bits
 * (no start bit or stop bits) of each character in the sentence between,
 * but excluding, the start of sequence character and the checksum delimiter
 */
static inline unsigned int computeChecksum(const char *message) {
	unsigned int checksum = 0;
	const char *p = message;
	  while(*p != '*') {
		  checksum = checksum ^ *p;
		  p++;
	  }
	return checksum;
}

static inline unsigned char getCharactersValue(const char character) {
	unsigned char value  = 0;
	if (isdigit(character)) {
		value = character - '0';
	} else {
		value = character - 'A' + 10 ;
	}
	return value;
}

static inline unsigned int getMessagesChecksum(const char *checksumPosition) {
	register const char *p = checksumPosition;
	unsigned int checksum = getCharactersValue(*p);
	p++;
	checksum = (checksum << 4) + getCharactersValue(*p);
	return checksum;
}

Message::Message(callbacksParameter parameter /*= NULL*/)
	:customParameter(parameter) {
}

Message::~Message() {
}

int Message::parse(const char *message) {
	int error(EXIT_SUCCESS);

	// IEC 61162 Data Transmission format protocol / NMEA 0183 defines messages as follow:
	// start of sequence character ('$' or '!')
	// address field: talker (2 characters) followed by the sentence for formatter mnemonic code identifying the data type and the string format of the successive fields
	// field separator (',') between on or more data fields
	// checksum delimiter ('*')
	// checksum field
	// <CR> <LF> (0x0D 0x0A)

	const size_t n = strlen(message);
	if (n > 0) {
        // check the end of sequence character (size, checksum and End of message indicator)
        if (n <= MAX_MESSAGE_LENGTH) {
            if ((0x0D == message[n-2]) && (0x0A == message[n-1])) {
                const char *checksumDelimiterPos = strrchr(message,'*');
                if (checksumDelimiterPos != NULL) {
                    const unsigned int computedChecksum = computeChecksum(message+1); // +1 to skip the start of sequence character
                    const unsigned int messagesChecksum = getMessagesChecksum(checksumDelimiterPos+1);
                    if (computedChecksum == messagesChecksum) {
                        switch(message[0]) {
                            case '$':
                                error = parseDollardMessage(message); // error already printed
                                break;
                            case '!':
                                error = parseBangMessage(message); // error already printed
                                break;
                            default:
                                ERROR_MSG("sentence start %c in the message (%s) is unknown",message[0],message);
                                error = EPROTO;
                                break;
                        } //switch(message[0])
                    } else {
                        ERROR_MSG("invalid checksum (computed = 0x%X, in the message (%s): 0x%X)",computedChecksum,message,messagesChecksum);
                        error = EBADMSG;
                    }
                } else {
                    ERROR_MSG("checksum delimiter (*) not found in %s",message);
                    error = EBADMSG;
                }
            } else {
                ERROR_MSG("bad message %s ending (0x%.2X%.2X)",message,message[n-2],message[n-1]);
                error = EPROTO;
            }
        } else {
            ERROR_MSG("message %s is too length: %d (82 allowed only, including start & end of sequence characters)",message,n);
            error = EMSGSIZE;
        }
	} else {
		ERROR_MSG("message is NULL");
		error = ENODATA;
	}

	return error;
};
