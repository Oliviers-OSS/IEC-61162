/*
 * SourceDevice.h
 *
 *  Created on: 7 mai 2013
 *      Author: oc
 */

#ifndef _SOURCE_DEVICE_H_
#define _SOURCE_DEVICE_H_

class SourceDevice {
public:
    SourceDevice() {
    }
    virtual ~SourceDevice() {
    }
    virtual int getMessage(char *buffer,const size_t bufferSize, size_t &filedSize) = 0;
};


#endif /* _SOURCE_DEVICE_H_ */
