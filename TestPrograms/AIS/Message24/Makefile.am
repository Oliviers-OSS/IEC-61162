AUTOMAKE_OPTIONS = gnu
#PKG_CONFIG_PATH=(top_builddir)
INCLUDES = -I$(top_srcdir)/include -I$(top_srcdir)
INCLUDES += `pkg-config --errors-to-stdout --cflags sigc++-2.0`  
INCLUDES += `pkg-config --errors-to-stdout --cflags dbgflags` 
INCLUDES += `PKG_CONFIG_PATH=$(top_builddir) pkg-config --errors-to-stdout --cflags IEC61162` 
INCLUDES += -I/home/oc/GitRepositories/cxxtest


AM_CPPFLAGS=-DLOGGER=consoleLogger
AM_CXXFLAGS=-std=c++0x

noinst_PROGRAMS = AISMessage24_test
AISMessage24_test_suite = AISMessage24_test.h 
AISMessage24_test_SOURCES = tests.cpp  AISMessage24_test.cpp $(AISMessage24_test_suite)
AISMessage24_test_LDFLAGS = -L$(top_builddir)/libIEC61162
AISMessage24_test_LDFLAGS += `pkg-config --errors-to-stdout --libs sigc++-2.0`
AISMessage24_test_LDFLAGS += `pkg-config --errors-to-stdout --libs dbgflags` 
AISMessage24_test_LDFLAGS += `PKG_CONFIG_PATH=$(top_builddir) pkg-config --errors-to-stdout --libs IEC61162`
AISMessage24_test_DEPENDENCIES = $(top_builddir)/libIEC61162/.libs/libIEC61162.so

tests.cpp: $(AISMessage24_test_suite)
	/home/oc/GitRepositories/cxxtest/bin/cxxtestgen --xunit-printer --xunit-file=IEC61162_AISMessage24_tests_report.xml -o tests.cpp $(AISMessage24_test_suite) 
