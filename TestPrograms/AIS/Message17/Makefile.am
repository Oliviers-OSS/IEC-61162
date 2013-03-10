AUTOMAKE_OPTIONS = gnu
#PKG_CONFIG_PATH=(top_builddir)
INCLUDES = -I$(top_srcdir)/include -I$(top_srcdir)
INCLUDES += `pkg-config --errors-to-stdout --cflags sigc++-2.0`  
INCLUDES += `pkg-config --errors-to-stdout --cflags dbgflags` 
INCLUDES += `PKG_CONFIG_PATH=$(top_builddir) pkg-config --errors-to-stdout --cflags IEC61162` 
INCLUDES += -I/home/oc/GitRepositories/cxxtest


AM_CPPFLAGS=-DLOGGER=consoleLogger
AM_CXXFLAGS=-std=c++0x

noinst_PROGRAMS = AISMessage17_test
AISMessage17_test_suite = AISMessage17_test.h 
AISMessage17_test_SOURCES = tests.cpp  AISMessage17_test.cpp $(AISMessage17_test_suite)
AISMessage17_test_LDFLAGS = -L$(top_builddir)/libIEC61162
AISMessage17_test_LDFLAGS += `pkg-config --errors-to-stdout --libs sigc++-2.0`
AISMessage17_test_LDFLAGS += `pkg-config --errors-to-stdout --libs dbgflags` 
AISMessage17_test_LDFLAGS += `PKG_CONFIG_PATH=$(top_builddir) pkg-config --errors-to-stdout --libs IEC61162`
AISMessage17_test_DEPENDENCIES = $(top_builddir)/libIEC61162/.libs/libIEC61162.so

tests.cpp: $(AISMessage17_test_suite)
	/home/oc/GitRepositories/cxxtest/bin/cxxtestgen --xunit-printer --xunit-file=IEC61162_AISMessage17_tests_report.xml -o tests.cpp $(AISMessage17_test_suite) 
