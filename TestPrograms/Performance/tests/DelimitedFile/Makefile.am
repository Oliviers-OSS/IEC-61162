AUTOMAKE_OPTIONS = gnu
#PKG_CONFIG_PATH=(top_builddir)
INCLUDES = -I$(top_srcdir)/include -I$(top_srcdir)
INCLUDES += `pkg-config --errors-to-stdout --cflags sigc++-2.0`  
INCLUDES += `pkg-config --errors-to-stdout --cflags dbgflags` 
INCLUDES += `PKG_CONFIG_PATH=$(top_builddir) pkg-config --errors-to-stdout --cflags IEC61162` 
INCLUDES += -I/home/oc/GitRepositories/cxxtest


AM_CPPFLAGS=-DLOGGER=consoleLogger -DSRCDIR="$(abs_srcdir)"
AM_CXXFLAGS = -std=c++0x

noinst_PROGRAMS = DelimitedFile_tests
DelimitedFile_tests_SOURCES = tests.cpp DelimitedFile_tests.h tools.h tools.cpp ../../DelimitedFile.h locks.h locks.cpp  
DelimitedFile_tests_LDFLAGS = -L$(top_builddir)/libIEC61162/.libs
DelimitedFile_tests_LDFLAGS += `pkg-config --errors-to-stdout --libs sigc++-2.0`
DelimitedFile_tests_LDFLAGS += `pkg-config --errors-to-stdout --libs dbgflags`
DelimitedFile_tests_LDFLAGS += -lrt -lpthread 
#DelimitedFile_tests_LDFLAGS += `PKG_CONFIG_PATH=$(top_builddir) pkg-config --errors-to-stdout --libs IEC61162`
DelimitedFile_tests_DEPENDENCIES = $(top_builddir)/libIEC61162/.libs/libIEC61162.so

tests.cpp: DelimitedFile_tests.h
	/home/oc/GitRepositories/cxxtest/bin/cxxtestgen --xunit-printer --xunit-file=DelimitedFile_tests_report.xml -o tests.cpp DelimitedFile_tests.h 
