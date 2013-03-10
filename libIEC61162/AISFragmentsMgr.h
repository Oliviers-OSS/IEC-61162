/*
 * AISFragmentsMgr.h
 *
 *  Created on: 6 avr. 2013
 *      Author: oc
 */

#ifndef AISFRAGMENTSMGR_H_
#define AISFRAGMENTSMGR_H_

#include <vector>
#include <string>
#include <ctime>
#include <cerrno>
#include <cassert>

#define MODULE_FLAG FLAG_AIS_RUNTIME
#define FRAGMENTS_FRAME_TIME_OUT (5*60) // in seconds

class AISFragmentsMgr {
    struct Fragments {
        // The maximum number of character in a sentence is 1008 bits
        // i.e. 1008 / 8 = 126 bytes and could be divided in fragments
        // of 3+1 = 4 to 82 bytes, including the starting the terminating
        // delimiters, i.e. 82 - 3 = 79 characters.
        // In fact less because only the payload is stored.
        // We suppose there will be almost (2048 / 79 = 25.9) 26 Segments.

        time_t firstFragmentreceivedDate;
        size_t totalSize;
        unsigned char numberOfSegmentsStored;
        unsigned char numberOfSegmentsTotal;
        char payloads[2048];
        unsigned int segmentsBitmap;

        Fragments()
            :firstFragmentreceivedDate(0),totalSize(0),numberOfSegmentsStored(0),numberOfSegmentsTotal(0),segmentsBitmap(0) {
        }

        ~Fragments() {
        }

        void reset(const time_t currentTime,const unsigned char countOfFragments) {
            firstFragmentreceivedDate = currentTime;
            totalSize = 0;
            numberOfSegmentsStored= 0;
            numberOfSegmentsTotal = countOfFragments;
            memset(payloads,0,sizeof(payloads));
            segmentsBitmap = 0x0;
        }

        void resetAttributs() {
            firstFragmentreceivedDate = 0;
            //totalSize = 0;
            //numberOfSegmentsStored= 0;
            //numberOfSegmentsTotal = 0;
        }

        void shrink() {
            register char *readCursor = payloads;
            register char *writeCursor = payloads;
            const char *endPayloads = payloads + sizeof(payloads);
            while(readCursor < endPayloads) {
                if (*readCursor != '\0') {
                    *writeCursor = *readCursor;
                    writeCursor++;
                }
                readCursor++;
            }
            // end the string
            *writeCursor = '\0';
        }

        const char *add(const unsigned char countOfFragments, const unsigned char fragmentNumber, const char *messageSegment) {
            char *fullSentence = NULL;

            const time_t currentTime = time(NULL);
            const unsigned int fragmentID(1 << fragmentNumber);
            if ( ( (currentTime - firstFragmentreceivedDate ) >= FRAGMENTS_FRAME_TIME_OUT)
                || (countOfFragments != numberOfSegmentsTotal)
                || ((segmentsBitmap & fragmentID) == fragmentID) ) {
                // new fragmented message
                reset(currentTime,countOfFragments);
            }
            segmentsBitmap |= fragmentID;

            const unsigned char offset = 79 * (fragmentNumber -1); // first fragment Number is one and first cell is 0
            assert(offset < sizeof(payloads));
            register char *writeCursor = payloads + offset;
            char endOfFragment = ',';
            if (countOfFragments == fragmentNumber) {
                endOfFragment = '*'; // to keep the last padding value
            }

            while(*messageSegment != endOfFragment) {
                *writeCursor = *messageSegment;
                writeCursor++;
                messageSegment++;
            }
            numberOfSegmentsStored++;

            // sentence completed ?
            if (numberOfSegmentsStored == countOfFragments) {
                shrink(); // concatenate all the segments
                fullSentence = payloads;
                DEBUG_VAR(fullSentence,"%s");
                resetAttributs(); // else if another message arrives during the grace time frame, it will be concatenated to this one.
            }
            return fullSentence;
        }
    } fragments[2][10];
public:

    AISFragmentsMgr() {
    }

    ~AISFragmentsMgr() {
    }

    const char * addSegment(const char radioChannel, const unsigned char sequentialMessageID, const unsigned char countOfFragments, const unsigned char fragmentNumber, const char *messageSegment) {
        assert(sequentialMessageID<10);
        assert( ('A' == radioChannel) || ('B' == radioChannel));
        assert(fragmentNumber > 0);
        return fragments[radioChannel - 'A'][sequentialMessageID].add(countOfFragments,fragmentNumber,messageSegment);
    }
};
#undef MODULE_FLAG
#endif /* AISFRAGMENTSMGR_H_ */
