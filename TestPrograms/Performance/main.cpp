/*
 * main.cpp
 *
 *  Created on: 30 avr. 2013
 *      Author: oc
 */

#include "globals.h"
#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <memory>
#include <sys/time.h>
#include <sys/resource.h>
#include <IEC61162/IEC-61162.h>
#include "DelimitedFile.h"

#define TIME_OUT_NSEC   200000000   // 200 ms
#define TIME_OUT_SEC    0

File2ParserMsg fileToParserMessages;
unsigned int nbValidMessage(0);
unsigned int nbBadMessage(0);

using namespace std;

void* parseMsg(void *param) {
    int error(EXIT_SUCCESS);
    IEC61162::AISMessage msg;
    const struct timespec timeout = {TIME_OUT_SEC,TIME_OUT_NSEC};
    do {
        std::string rawMsg;
        DEBUG_MSG("waiting for msg...");
        error = fileToParserMessages.getMessage(rawMsg,timeout);
        DEBUG_VAR(error,"%d");
        if (EXIT_SUCCESS == error) {
            const int parseError =  msg.parse(rawMsg.c_str());
            DEBUG_MSG("%s => error = %d",rawMsg.c_str(),parseError);
            if (EXIT_SUCCESS == parseError) {
                ++nbValidMessage;
                DEBUG_VAR(nbValidMessage,"%u");
            } else {
                ++nbBadMessage;
                DEBUG_VAR(nbBadMessage,"%u");
            }
        } else {
            ERROR_MSG("getMessage error %d",error);
        }
    } while(EXIT_SUCCESS == error);
    INFO_MSG("parseMsg ended");
    return NULL;
}

/*
static inline int readFile(const char *filename) {
    int error(EXIT_SUCCESS);
    ifstream file(filename);
    string line;

    if (file.is_open()) {
        while(file.good()) {
            getline (file,line);
            fileToParserMessages.putMessage(line);
        }
        file.close();
        DEBUG_MSG("read file ended");
    } else {
        error = errno;
        ERROR_MSG("open %s error %d (%m)",filename,error);
    }

    return error;
}*/

static inline int readFile(const char *filename) {
    int error(EXIT_SUCCESS);
    try {
        DelimitedFile<char,4096> file(filename);
        char line[127];
        ssize_t size;
        do {
            size = file.getLineEx(line,sizeof(line),"\n","\r\n");
            if (size) {
                fileToParserMessages.putMessage(line);
            }
        } while( size > 0);
    } //
    catch(DelimitedFile<char,4096>::exception &e) {
        ERROR_MSG("exception caught: %s",e.what());
        error = e.code();
    }
    INFO_MSG("end of file reading...");
    return error;
}

static inline void computeElapsedTime(const struct timespec &endTime,const struct timespec &startTime,struct timespec &elapsedTime) {
    if ((startTime.tv_sec != 0) && (startTime.tv_nsec != 0)) {
        elapsedTime.tv_sec = endTime.tv_sec - startTime.tv_sec;
        elapsedTime.tv_nsec = endTime.tv_nsec - startTime.tv_nsec - TIME_OUT_NSEC;
        if (elapsedTime.tv_nsec < 0) {
            elapsedTime.tv_sec--;
            elapsedTime.tv_nsec += 1000000000UL;
        }
    } else {
        elapsedTime.tv_sec = elapsedTime.tv_nsec = 0;
    }
}

//Table 1-4: Contents of the stat files (as of 2.6.30-rc7)
//..............................................................................
// Field          Content
//  pid           process id
//  tcomm         filename of the executable
//  state         state (R is running, S is sleeping, D is sleeping in an
//                uninterruptible wait, Z is zombie, T is traced or stopped)
//  ppid          process id of the parent process
//  pgrp          pgrp of the process
//  sid           session id
//  tty_nr        tty the process uses
//  tty_pgrp      pgrp of the tty
//  flags         task flags
//  min_flt       number of minor faults
//  cmin_flt      number of minor faults with child's
//  maj_flt       number of major faults
//  cmaj_flt      number of major faults with child's
//  utime         user mode jiffies
//  stime         kernel mode jiffies
//  cutime        user mode jiffies with child's
//  cstime        kernel mode jiffies with child's
//  priority      priority level
//  nice          nice level
//  num_threads   number of threads
//  it_real_value (obsolete, always 0)
//  start_time    time the process started after system boot
//  vsize         virtual memory size
//  rss           resident set memory size
//  rsslim        current limit in bytes on the rss
//  start_code    address above which program text can run
//  end_code      address below which program text can run
//  start_stack   address of the start of the stack
//  esp           current value of ESP
//  eip           current value of EIP
//  pending       bitmap of pending signals
//  blocked       bitmap of blocked signals
//  sigign        bitmap of ignored signals
//  sigcatch      bitmap of catched signals
//  wchan         address where process went to sleep
//  0             (place holder)
//  0             (place holder)
//  exit_signal   signal to send to parent thread on exit
//  task_cpu      which CPU the task is scheduled on
//  rt_priority   realtime priority
//  policy        scheduling policy (man sched_setscheduler)
//  blkio_ticks   time spent waiting for block IO
//  gtime         guest time of the task in jiffies
//  cgtime        guest time of the task children in jiffies

static inline void displayProcessStatusData() {
    ifstream stats("/proc/self/status");
    if (stats.is_open()) {
        string line;
        while(stats.good()) {
            getline (stats,line);
            cout << line << endl;
        }
        stats.close();
    }
}

int main(int argc, char *argv[]) {
    int error(EXIT_SUCCESS);
    pthread_attr_t attr;
    //const char *filename= TO_STRING(SRCDIR)"/AIS_Msg.txt";
    //const char *filename= TO_STRING(SRCDIR)"/sample";
    //const char *filename= TO_STRING(SRCDIR)"/nmea-sample";
    //const char *filename= TO_STRING(SRCDIR)"/dst-ouessant";
    const char *filename= TO_STRING(SRCDIR)"/log_corsen_1159195516234";
    //const char *filename= TO_STRING(SRCDIR)"/logs";
    //const char *filename= TO_STRING(SRCDIR)"/log_simuJPN310p-13112006";
    if (argc > 1) {
        filename = argv[1];
    }
    error = pthread_attr_init(&attr);
    if (0 == error) {
        error = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
        if (0 == error) {
            pthread_t parser;
            error = pthread_create(&parser,&attr,parseMsg,NULL);
            if (0 == error) {

                struct timespec startTime;
                if ( clock_gettime(CLOCK_MONOTONIC, &startTime) != 0) {
                    error = errno;
                    ERROR_MSG("clock_gettime startTime error %d (%m)",error);
                    startTime.tv_sec = startTime.tv_nsec = 0;
                }

                error = readFile(filename);

                error = pthread_join(parser,NULL);
                if (error != 0) {
                    ERROR_MSG("pthread_join error %d",error);
                }

                struct timespec endTime;
                if ( clock_gettime(CLOCK_MONOTONIC, &endTime) != 0) {
                    error = errno;
                    ERROR_MSG("clock_gettime endTime error %d (%m)",error);
                }

                struct timespec elapsedTime;
                computeElapsedTime(endTime,startTime,elapsedTime);

                struct rusage usage;
                if (getrusage(RUSAGE_SELF,&usage) == 0) {
                    displayProcessStatusData();
                    cout << "Elapsed time = " << elapsedTime.tv_sec << " sec " <<  elapsedTime.tv_nsec << " nsec" << endl;
                    cout << "User CPU time used = " << usage.ru_utime.tv_sec << " sec " << usage.ru_utime.tv_usec << " usec" << endl;
                    cout << "System CPU time used = " << usage.ru_stime.tv_sec << " sec " << usage.ru_stime.tv_usec << " usec" << endl;
                    cout << "Number of valid messages = " << nbValidMessage << endl;
                    cout << "Number of invalid Messages = " << nbBadMessage << endl;
                    cout << "Total Number of Messages = " << nbValidMessage + nbBadMessage << endl;
                } else {
                    error = errno;
                    ERROR_MSG("getrusage error %d (%m)",error);
                }
            } else {
                ERROR_MSG("pthread_create error %d",error);
            }
        } else {
            ERROR_MSG("pthread_attr_setdetachstate error %d",error);
        }
    } else {
        ERROR_MSG("pthread_attr_init error %d",error);
    }
    return error;
}
