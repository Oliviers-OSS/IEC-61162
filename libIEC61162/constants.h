/*
 * constants.h
 *
 *  Created on: 8 mars 2013
 *      Author: oc
 */

#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_

#define MAX_MESSAGE_LENGTH 83 //82
// Talker identifiers extended with the AIS talker id and proprietary sentence field
// cf. NMEA0183.pdf p2-3
#define PROPRIETARY_MSG_IDENTIFIER	*((unsigned short*)"P")  // Proprietary sentences
#define AIS_MSG_IDENTIFIER			*((unsigned short*)"AI") // AIS sentences
#define NO_MSG_IDENTIFIER			*((unsigned short*)"--")
#if 0
*((unsigned short*)"AG") // Autopilot - General
*((unsigned short*)"AP") // Autopilot - Magnetic
*((unsigned short*)"CD") // Communications – Digital Selective Calling (DSC)
*((unsigned short*)"CR") // Communications – Receiver / Beacon Receiver
*((unsigned short*)"CS") // Communications – Satellite
*((unsigned short*)"CT") // Communications – Radio-Telephone (MF/HF)
*((unsigned short*)"CV") // Communications – Radio-Telephone (VHF)
*((unsigned short*)"CX") // Communications – Scanning Receiver
*((unsigned short*)"DF") // Direction Finder
*((unsigned short*)"EC") // Electronic Chart Display & Information System (ECDIS)
*((unsigned short*)"EP") // Emergency Position Indicating Beacon (EPIRB)
*((unsigned short*)"ER") // Engine Room Monitoring Systems
*((unsigned short*)"GP") // Global Positioning System (GPS)
*((unsigned short*)"HC") // Heading – Magnetic Compass
*((unsigned short*)"HE") // Heading – North Seeking Gyro
*((unsigned short*)"HN") // Heading – Non North Seeking Gyro
*((unsigned short*)"II") // Integrated Instrumentation
*((unsigned short*)"IN") // Integrated Navigation
*((unsigned short*)"LC") // Loran C
*((unsigned short*)"RA") // RADAR and/or ARPA
*((unsigned short*)"SD") // Sounder, Depth
*((unsigned short*)"SN") // Electronic Positioning System, other/general
*((unsigned short*)"SS") // Sounder, Scanning
*((unsigned short*)"TI") // Turn Rate Indicator
*((unsigned short*)"VD") // Velocity Sensor, Doppler, other/general
*((unsigned short*)"DM") // Velocity Sensor, Speed Log, Water, Magnetic
*((unsigned short*)"VW") // Velocity Sensor, Speed Log, Water, Mechanical
*((unsigned short*)"WI") // Weather Instruments
*((unsigned short*)"YX") // Transducer
*((unsigned short*)"ZA") // Timekeeper – Atomic Clock
*((unsigned short*)"ZC") // Timekeeper – Chronometer
*((unsigned short*)"ZQ") // Timekeeper – Quartz
*((unsigned short*)"ZV") // Timekeeper – Radio Update, WWV or WWVH
#endif

#define SET_ALARM_STATE_FORMATTER 				*((unsigned int*)"ALR")
#define VHF_DATA_LINK_MSG_FORMATTER 			*((unsigned int*)"VDM")
#define VHF_DATA_LINK_OWN_VESSEL_MSG_FORMATTER  *((unsigned int*)"VDO")
#define VHF_DATA_LINK_TEXT_MSG_FORMATTER        *((unsigned int*)"TXT") //p10


#endif /* _CONSTANTS_H_ */
