AUTOMAKE_OPTIONS = gnu
#PKG_CONFIG_PATH=(top_builddir)
INCLUDES = -I$(top_srcdir)/include -I$(top_srcdir)  
INCLUDES += `pkg-config --errors-to-stdout --cflags dbgflags`  
INCLUDES += -I/home/oc/GitRepositories/cxxtest

AM_CPPFLAGS=-DLOGGER=consoleLogger -DSRCDIR="$(abs_srcdir)"
AM_CXXFLAGS = -std=c++0x

noinst_PROGRAMS = Configuration_tests
READ_ONLY_DATA_DIR=$(abs_srcdir)/../..
READ_WRITE_DATA_DIR=/tmp
Configuration_tests_SOURCES = tests.cpp Configuration_tests.h ../../tools.h ../../tools.cpp ../../Configuration.h ../../Configuration.cpp ../../debug.h ../../debug.cpp ../../locks.h ../../locks.cpp
Configuration_tests_CPPFLAGS = -DDATADIR=$(READ_ONLY_DATA_DIR) -DLOCALSTATEDIR=$(READ_WRITE_DATA_DIR) -DPROGRAM_NAME=$(noinst_PROGRAMS) -D_GNU_SOURCE
Configuration_tests_LDFLAGS = -L$(top_builddir)/libIEC61162
Configuration_tests_LDFLAGS += `pkg-config --errors-to-stdout --libs dbgflags`
Configuration_tests_LDFLAGS += -lrt -lpthread 
#Configuration_tests_LDFLAGS += `PKG_CONFIG_PATH=$(top_builddir) pkg-config --errors-to-stdout --libs IEC61162`

tests.cpp: Configuration_tests.h
	/home/oc/GitRepositories/cxxtest/bin/cxxtestgen --xunit-printer --xunit-file=Configuration_tests_report.xml -o tests.cpp Configuration_tests.h 