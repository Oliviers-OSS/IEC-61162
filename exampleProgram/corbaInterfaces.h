/*
 * corbaInterfaces.h
 *
 *  Created on: 22 août 2013
 *      Author: oc
 */

#ifndef _CORBA_INTERFACES_H_
#define _CORBA_INTERFACES_H_

#include "globals.h"

#if HAVE_TAO

#define MODULE_FLAG FLAG_CORBA

struct ProcessStartParameters {
    int argc;
    char **argv;

    ProcessStartParameters(int c,char*v[]):argc(c),argv(v) {
    }
};

void *ORBMainThread (void *parameter);

inline int startServants(int argc, char *argv[]) {
    int error(EXIT_SUCCESS);

    // use pthread attributs and not only a call to  pthread_detach
    // to be able to change more than one flags if needed (stack'size...)
    pthread_attr_t attr;
    error = pthread_attr_init(&attr);
    if (0 == error) {
        error = pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
        if (0 == error) {
            ProcessStartParameters *params = new (std::nothrow) ProcessStartParameters(argc, argv);
            if (params != 0) {
                pthread_t orbMainThread;
                error = pthread_create(&orbMainThread,&attr,ORBMainThread,params);
            } else {
                error = ENOMEM;
                ERROR_MSG("failed to allocate %u bytes to store process start parameters",sizeof(ProcessStartParameters));
            }
        } else {
            ERROR_MSG("pthread_attr_setdetachstate PTHREAD_CREATE_DETACHED error %d",error);
        }
        error = pthread_attr_destroy(&attr);
        if (0 == error) {
        } else {
            ERROR_MSG("pthread_attr_destroy error %d",error);
        }
    } else {
        ERROR_MSG("pthread_attr_init error %d",error);
    }
    return error;
}

#undef MODULE_FLAG

#else

inline int startServants(int argc, char *argv[]) {
    int error(EXIT_SUCCESS);
    return error;
}

#endif //HAVE_TAO

#endif /* _CORBA_INTERFACES_H_ */
