/*
 * tools.cpp
 *
 *  Created on: 13 nov. 2012
 *      Author: oc
 */

#include "debug.h"
#include "tools.h"

#include <cstdio>
#include <cstddef>
#include <cerrno>
#include <cstdlib>
#include <dbgflags-1/debug_macros.h>

#ifdef FLAG_RUNTIME
#define MODULE_FLAG FLAG_RUNTIME
#endif //FLAG_RUNTIME

int vsprintf(std::string &s, const char *format,va_list params) {
	int error(EXIT_SUCCESS);
	char *buffer(0);
	size_t size(0);
	FILE *fd = open_memstream(&buffer,&size);
	if (fd != NULL) {
		vfprintf(fd,format,params);
		fclose(fd);
		fd = NULL;
		s.reserve(size);
		s = buffer;
		free(buffer);
		buffer = 0;
	} else {
		error = errno;
		ERROR_MSG("open_memstream error %d (%m)",error);
	}

	return ((EXIT_SUCCESS == error)? size : -error);
}

int sprintf(std::string &s, const char *format,...) {
	va_list params;
	va_start(params,format);
	const int n = vsprintf(s,format,params);
	va_end(params);
	return n;
}
