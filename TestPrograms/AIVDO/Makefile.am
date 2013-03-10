AUTOMAKE_OPTIONS = gnu
#PKG_CONFIG_PATH=(top_builddir)
INCLUDES = -I$(top_srcdir)/include -I$(top_srcdir)
INCLUDES += `pkg-config --errors-to-stdout --cflags sigc++-2.0`  
INCLUDES += `pkg-config --errors-to-stdout --cflags dbgflags` 
INCLUDES += `PKG_CONFIG_PATH=$(top_builddir) pkg-config --errors-to-stdout --cflags IEC61162` 
INCLUDES += -I/home/oc/GitRepositories/cxxtest


AM_CPPFLAGS=-DLOGGER=consoleLogger

noinst_PROGRAMS = AIVDO_test
AIVDO_test_suite = AIVDO_Test.h 
AIVDO_test_SOURCES = tests.cpp  AIVDO_Test.cpp $(AIVDO_test_suite)
AIVDO_test_LDFLAGS = -L$(top_builddir)/libIEC61162
AIVDO_test_LDFLAGS += `pkg-config --errors-to-stdout --libs sigc++-2.0`
AIVDO_test_LDFLAGS += `pkg-config --errors-to-stdout --libs dbgflags` 
AIVDO_test_LDFLAGS += `PKG_CONFIG_PATH=$(top_builddir) pkg-config --errors-to-stdout --libs IEC61162`
AIVDO_test_DEPENDENCIES = $(top_builddir)/libIEC61162/.libs/libIEC61162.so

tests.cpp: $(AIVDO_test_suite)
	/home/oc/GitRepositories/cxxtest/bin/cxxtestgen --xunit-printer --xunit-file=IEC61162_AIVDO_tests_report.xml -o tests.cpp $(AIVDO_test_suite) 