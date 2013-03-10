#ifndef _IEC_61162_H_
#define _IEC_61162_H_

#include <sigc++/sigc++.h>
#include <string>
#include <IEC61162/basicsmartpointer.h>
#include <IEC61162/errors.h>
#include <IEC61162/AISMessage01.h>
#include <IEC61162/AISMessage02.h>
#include <IEC61162/AISMessage03.h>
#include <IEC61162/AISMessage04.h>
#include <IEC61162/AISMessage05.h>
#include <IEC61162/AISMessage06.h>
#include <IEC61162/AISMessage07.h>
#include <IEC61162/AISMessage08.h>
#include <IEC61162/AISMessage09.h>
#include <IEC61162/AISMessage10.h>
#include <IEC61162/AISMessage11.h>
#include <IEC61162/AISMessage12.h>
#include <IEC61162/AISMessage13.h>
#include <IEC61162/AISMessage14.h>
#include <IEC61162/AISMessage15.h>
#include <IEC61162/AISMessage16.h>
#include <IEC61162/AISMessage17.h>
#include <IEC61162/AISMessage18.h>
#include <IEC61162/AISMessage19.h>
#include <IEC61162/AISMessage20.h>
#include <IEC61162/AISMessage21.h>
#include <IEC61162/AISMessage22.h>
#include <IEC61162/AISMessage23.h>
#include <IEC61162/AISMessage24.h>
#include <IEC61162/AISMessage25.h>
#include <IEC61162/AISMessage26.h>
#include <IEC61162/AISMessage27.h>
#include <IEC61162/AISAlarm.h>

namespace IEC61162 {
	typedef basic_smart_pointer<std::string>	raw_sentence;
	typedef void*                               callbacksParameter;

	class Message {
	protected:
	    int parseDollardMessage(const char *message);
	    int parseBangMessage(const char *message);
		callbacksParameter customParameter;
	public:
		Message(callbacksParameter parameter = NULL);
		virtual ~Message();
		virtual int parse(const char *buffer);

		// Proprietary Sentences
		sigc::signal<void, raw_sentence, callbacksParameter> proprietary_sentence;
	};

	class AISMessage : public Message {
	protected:
	    friend class Message;
	    int parseDollardMessage(const char *message);
	    int parseBangMessage(const char *message);
	    virtual int AIVDMParser(const char *message);
	    virtual int AIVDOParser(const char *message);
	public:

		AISMessage(callbacksParameter parameter = NULL)
		    :Message(parameter) {
		}

		virtual ~AISMessage() {
		}

		// AIS Messages (VDM & VDO)
		sigc::signal<void, AISMsg1,   callbacksParameter> message1;
		sigc::signal<void, AISMsg2,   callbacksParameter> message2;
		sigc::signal<void, AISMsg3,   callbacksParameter> message3;
		sigc::signal<void, AISMsg4,   callbacksParameter> message4;
		sigc::signal<void, AISMsg5,   callbacksParameter> message5;
		sigc::signal<void, AISMsg6,   callbacksParameter> message6;
		sigc::signal<void, AISMsg7,   callbacksParameter> message7;
		sigc::signal<void, AISMsg8,   callbacksParameter> message8;
		sigc::signal<void, AISMsg9,   callbacksParameter> message9;
		sigc::signal<void, AISMsg10,  callbacksParameter> message10;
		sigc::signal<void, AISMsg11,  callbacksParameter> message11;
		sigc::signal<void, AISMsg12,  callbacksParameter> message12;
		sigc::signal<void, AISMsg13,  callbacksParameter> message13;
		sigc::signal<void, AISMsg14,  callbacksParameter> message14;
		sigc::signal<void, AISMsg15,  callbacksParameter> message15;
		sigc::signal<void, AISMsg16,  callbacksParameter> message16;
		sigc::signal<void, AISMsg17,  callbacksParameter> message17;
		sigc::signal<void, AISMsg18,  callbacksParameter> message18;
		sigc::signal<void, AISMsg19,  callbacksParameter> message19;
		sigc::signal<void, AISMsg20,  callbacksParameter> message20;
		sigc::signal<void, AISMsg21,  callbacksParameter> message21;
		sigc::signal<void, AISMsg22,  callbacksParameter> message22;
		sigc::signal<void, AISMsg23,  callbacksParameter> message23;
		sigc::signal<void, AISMsg24,  callbacksParameter> message24;  // to register for msg 24 A & B
		sigc::signal<void, AISMsg24A, callbacksParameter> message24A; // to register for msg 24 A only
		sigc::signal<void, AISMsg24B, callbacksParameter> message24B; // to register for msg 24 B only
		sigc::signal<void, AISMsg25,  callbacksParameter> message25;
		sigc::signal<void, AISMsg26,  callbacksParameter> message26;
		sigc::signal<void, AISMsg27,  callbacksParameter> message27;

		// Alarms
		sigc::signal<void, Alarm,  callbacksParameter> alarm;
	};

	class AIVDMMessage : public AISMessage {
	protected:
	    virtual int AIVDOParser(const char *message);
	public:
	    AIVDMMessage(callbacksParameter parameter = NULL)
            :AISMessage(parameter) {
        }

        virtual ~AIVDMMessage() {
        }
	};

    class AIVDOMessage : public AISMessage {
    protected:
        virtual int AIVDMParser(const char *message);
    public:
        AIVDOMessage(callbacksParameter parameter = NULL)
            :AISMessage(parameter) {
        }

        virtual ~AIVDOMessage() {
        }
    };
};

#endif // _IEC_61162_H_
