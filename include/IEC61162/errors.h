/*
 * errors.h
 *
 *  Created on: 8 mars 2013
 *      Author: oc
 */

#ifndef _ERRORS_H_
#define _ERRORS_H_

#include <cerrno>

#define FACILITY_IEC61162   (1<<0)
#define FACILITY_AIS_MSG    (1<<1)

// error code memory layout
//  8 bits 31->24 facility
// 24 bits 00->23 error code
// 33222222222211111111110000000000
// 10987654321098765432109876543210

#define ERROR_CODE(facility,error)  ((facility << 22) | (error & 0xFFFFFF))
#define GET_ERROR(code)             ((code) & 0xFFFFFF)
#define GET_FACILITY(code)          (((code) & FF000000) >> 24)

// values from 0 to 0xFF are defined in errno.h
#define BAD_CHECKSUM                0x100
#define BAD_SIZE                    0x101
#define BAD_FORMAT                  0x102
#define PROTOCOL_ERROR              0x103
#define OUT_OF_BOUNDARY             0x104
#define OUT_OF_VALIDITY_DOMAIN      0x105
#define INVALID_MESSAGE             0x106
#define UNKNOWN_MESSAGE             0x107

#endif /* _ERRORS_H_ */
