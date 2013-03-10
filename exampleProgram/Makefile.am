AUTOMAKE_OPTIONS = gnu
SUBDIRS = tests

#######################################
# The list of executables we are building seperated by spaces
# the 'bin_' indicates that these build products will be installed
# in the $(bindir) directory. For example /usr/bin
#bin_PROGRAMS=IEC61162_Demo

# Because a.out is only a sample program we don't want it to be installed.
# The 'noinst_' prefix indicates that the following targets are not to be
# installed.
noinst_PROGRAMS=IEC61162_Demo

#######################################
if DEV_MODE_BUILD_ENABLED
    READ_ONLY_DATA_DIR=$(abs_srcdir)
	READ_WRITE_DATA_DIR=/tmp
else
	READ_ONLY_DATA_DIR=$(datadir)/$(noinst_PROGRAMS)
	READ_WRITE_DATA_DIR=$(localstatedir)   
endif

#######################################
# Build information for each executable. The variable name is derived
# by use the name of the executable with each non alpha-numeric character is
# replaced by '_'. So a.out becomes a_out and the appropriate suffex added.
# '_SOURCES' for example.

# Sources for the example program
IEC61162_Demo_headers = debug.h globals.h Configuration.h tools.h InterThreadsMsgQueue.h String.h locks.h
IEC61162_Demo_headers += SourceDevice.h SourceFile.h DelimitedFile.h SourceSerial.h SourceTCP.h
IEC61162_Demo_headers += InternalDatabaseWriter.h InternalDatabaseReader.h 
IEC61162_Demo_SOURCES = main.cpp debug.cpp Configuration.cpp tools.cpp InternalDatabaseWriter.cpp SourceTCP.cpp InternalDatabaseReader.cpp String.cpp locks.cpp  
IEC61162_Demo_SOURCES += $(noinst_IEC61162_Demo_headers) 

# Linker options for the example program
IEC61162_Demo_LDFLAGS = $(top_srcdir)/libIEC61162/libIEC61162.la 

# Compiler options for the example program
INCLUDES = -I$(top_srcdir)/include -I$(top_srcdir)
#INCLUDES += `pkg-config --errors-to-stdout --cflags sigc++-2.0`  
#INCLUDES += `pkg-config --errors-to-stdout --cflags dbgflags` 
#INCLUDES += `pkg-config --errors-to-stdout --cflags sqlite3`
#INCLUDES += `pkg-config --errors-to-stdout --cflags spatialite`
INCLUDES += `PKG_CONFIG_PATH=$(top_builddir) pkg-config --errors-to-stdout --cflags IEC61162`
IEC61162_Demo_CPPFLAGS = $(LIBSIGC_CFLAGS) $(DBGFLAGS_CFLAGS) $(SQLITE3_CFLAGS) $(SPATIALITE_CFLAGS)
IEC61162_Demo_CPPFLAGS += -DDATADIR=$(READ_ONLY_DATA_DIR) -DLOCALSTATEDIR=$(READ_WRITE_DATA_DIR) -DPROGRAM_NAME=$(noinst_PROGRAMS) -D_GNU_SOURCE
IEC61162_Demo_CXXFLAGS = -std=c++0x
IEC61162_Demo_LDADD = $(LIBSIGC_LIBS) $(DBGFLAGS_LIBS) $(SQLITE3_LIBS) $(SPATIALITE_LIBS)
#IEC61162_Demo_LDFLAGS += `pkg-config --errors-to-stdout --libs sigc++-2.0`
#IEC61162_Demo_LDFLAGS += `pkg-config --errors-to-stdout --libs dbgflags` 
#IEC61162_Demo_LDFLAGS += `pkg-config --errors-to-stdout --libs sqlite3`
#IEC61162_Demo_LDFLAGS += `pkg-config --errors-to-stdout --libs spatialite`
IEC61162_Demo_LDFLAGS += -lrt -lpthread
IEC61162_Demo_LDFLAGS += `PKG_CONFIG_PATH=$(top_builddir) pkg-config --errors-to-stdout --libs IEC61162`

# Dependencies of the example program
IEC61162_Demo_DEPENDENCIES = $(top_builddir)/libIEC61162/.libs/libIEC61162.so

if EXAMPLE_PROGRAM_SERVANT_ENABLED
# CORBA
#IDL_FILES= $(wildcard *.idl)
IDL_COMPILER=tao_idl
IDL_COMPILER_OPT=-in 
IDL_COMPILER_GENERATED_HEADERS_FILES_SERVER = aisS.inl aisS.h 
IDL_COMPILER_GENERATED_HEADERS_FILES_CLIENT = aisC.h aisC.inl  
IDL_COMPILER_GENERATED_SOURCES_FILES_SERVER = aisS.cpp 
IDL_COMPILER_GENERATED_SOURCES_FILES_CLIENT = aisC.cpp

%S.c %C.h %C.inl %S.inl: %.idl
	$(IDL_COMPILER) $(IDL_COMPILER_OPT) $< 
 
#EXTRA_DIST += aisS_T.cpp
noinst_HEADERS = $(IDL_COMPILER_GENERATED_HEADERS_FILES_SERVER) $(IDL_COMPILER_GENERATED_HEADERS_FILES_CLIENT)
IEC61162_Demo_SOURCES += MapServantImpl.h MapServantImpl.cpp corbaInterfaces.h corbaInterfaces.cpp CorbaClientAdaptor.h $(IDL_COMPILER_GENERATED_SOURCES_FILES_SERVER) $(IDL_COMPILER_GENERATED_SOURCES_FILES_CLIENT)
IEC61162_Demo_LDFLAGS += -lTAO -lACE -lTAO_PortableServer -lTAO_CosNaming $(TAO_LIBS)
endif

# additional data files
IEC61162_Demodir=$(READ_ONLY_DATA_DIR)
IEC61162_Demo_DATA=countries.csv Type_of_electronic_position_fixing_device.csv