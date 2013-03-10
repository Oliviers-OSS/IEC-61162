/*
 * String.cpp
 *
 *  Created on: 22 août 2013
 *      Author: oc
 */

#include "String.h"

int String::vFormat(const char *format,va_list optional_arguments) {
    if (buffer) {
        free(buffer);
    }
    return vasprintf(&buffer, format, optional_arguments);
}

int String::Format(const char *format,...) {
    va_list optional_arguments;
    va_start(optional_arguments, format);
    const int n = vFormat(format,  optional_arguments);
    va_end(optional_arguments);
    return n;
}
