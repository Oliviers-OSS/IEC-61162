/*
 * tools.h
 *
 *  Created on: 13 nov. 2012
 *      Author: oc
 */

#ifndef TOOLS_H_
#define TOOLS_H_

//#include "globals.h"
#include <cstdarg>
#include <string>
//#include <pthread.h>
//#include <stdexcept>

//#define MODULE_FLAG FLAG_LOCKS
/**
 * @brief vsprintf function for the std:string class.
 * @remark see vsprintf(3) for details.
 */
int vsprintf(std::string &s, const char *format,va_list params);

/**
 * @brief sprintf function for the std:string class
 * @remark see sprintf(3) for details.
 */
int sprintf(std::string &s, const char *format,...);


#undef MODULE_FLAG
#endif /* TOOLS_H_ */
