#######################################
# The list of libraries we are building seperated by spaces.
# The 'lib_' indicates that these build products will be installed
# in the $(libdir) directory. For example /usr/lib
lib_LTLIBRARIES = libIEC61162.la

#######################################
# Build information for each library

 
# Sources for libIEC61162
libIEC61162_la_PUBLICS_INCLUDES = IEC61162/IEC-61162.h IEC61162/basicsmartpointer.h IEC61162/errors.h IEC61162/AISDataTypes.h 
libIEC61162_la_PUBLICS_INCLUDES += IEC61162/AISMessage01.h IEC61162/AISMessage02.h IEC61162/AISMessage03.h IEC61162/AISMessage04.h IEC61162/AISMessage05.h IEC61162/AISMessage06.h IEC61162/AISMessage07.h IEC61162/AISMessage08.h IEC61162/AISMessage09.h
libIEC61162_la_PUBLICS_INCLUDES += IEC61162/AISMessage10.h IEC61162/AISMessage11.h IEC61162/AISMessage12.h IEC61162/AISMessage13.h IEC61162/AISMessage14.h
libIEC61162_la_PUBLICS_INCLUDES += IEC61162/AISMessage24.h IEC61162/AISMessage27.h
libIEC61162_la_PRIVATES_INCLUDES = globals.h debug.h constants.h ModuleVersionInfo.h
libIEC61162_la_INCLUDES = $(libIEC61162_la_PUBLICS_INCLUDES) $(libIEC61162_la_PRIVATES_INCLUDES) 
libIEC61162_la_SOURCES = $(libIEC61162_la_INCLUDES) libIEC61162.cpp AISMessages.cpp debug.cpp

# Here we are setting the logger used for traces 
#libIEC61162_la_CPPFLAGS += -DLOGGER=consoleLogger

# Compiler options. Here we are adding the include directory
# to be searched for headers included in the source code.
libIEC61162_la_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/include $(LIBSIGC_CFLAGS)

# Linker options libIEC61162
libIEC61162_la_LDFLAGS = $(LIBSIGC_LIBS)
libIEC61162_la_LDFLAGS += -version-info ${LIB_VERSION}:${LIB_REVISION}:${LIB_INTERFACE_AGE}
libIEC61162_la_LDFLAGS += -Wl,--version-script=$(top_srcdir)/libIEC61162/libIEC61162.map
	
