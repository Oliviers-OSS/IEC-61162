AUTOMAKE_OPTIONS = gnu
#PKG_CONFIG_PATH=(top_builddir)
INCLUDES = -I$(top_srcdir)/include -I$(top_srcdir)  
#INCLUDES += `pkg-config --errors-to-stdout --cflags dbgflags`  
INCLUDES += -I/home/oc/GitRepositories/cxxtest

READ_ONLY_DATA_DIR=$(abs_srcdir)/../..
READ_WRITE_DATA_DIR=/tmp

AM_CPPFLAGS = -DLOGGER=consoleLogger -DSRCDIR="$(abs_srcdir)"
AM_CPPFLAGS += $(DBGFLAGS_CFLAGS) $(SQLITE3_CFLAGS) $(LIBSIGC_CFLAGS)
#AM_CPPFLAGS += -DDATADIR=$(datadir) -DLOCALSTATEDIR=$(localstatedir) -D_GNU_SOURCE
AM_CPPFLAGS += -DDATADIR=$(READ_ONLY_DATA_DIR) -DLOCALSTATEDIR=$(READ_WRITE_DATA_DIR) -D_GNU_SOURCE
AM_CXXFLAGS = -std=c++0x

noinst_PROGRAMS = InternalDatabaseWriter_tests

InternalDatabaseWriter_tests_SOURCES = tests.cpp InternalDatabaseWriter_tests.h ../../tools.h ../../tools.cpp ../../Configuration.h ../../Configuration.cpp ../../InternalDatabaseWriter.h ../../InternalDatabaseWriter.cpp ../../debug.h ../../debug.cpp ../../locks.h ../../locks.cpp
InternalDatabaseWriter_tests_LDFLAGS = -L$(top_builddir)/libIEC61162
InternalDatabaseWriter_tests_LDFLAGS += $(DBGFLAGS_LIBS) $(SQLITE3_LIBS) $(LIBSIGC_LIBS)
InternalDatabaseWriter_tests_LDFLAGS += -lrt -lpthread 
InternalDatabaseWriter_tests_LDFLAGS += `PKG_CONFIG_PATH=$(top_builddir) pkg-config --errors-to-stdout --libs IEC61162`

tests.cpp: InternalDatabaseWriter_tests.h
	/home/oc/GitRepositories/cxxtest/bin/cxxtestgen --xunit-printer --xunit-file=InternalDatabaseWriter_tests_report.xml -o tests.cpp InternalDatabaseWriter_tests.h 