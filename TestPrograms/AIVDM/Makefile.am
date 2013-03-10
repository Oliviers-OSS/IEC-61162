AUTOMAKE_OPTIONS = gnu
#PKG_CONFIG_PATH=(top_builddir)
INCLUDES = -I$(top_srcdir)/include -I$(top_srcdir)
INCLUDES += `pkg-config --errors-to-stdout --cflags sigc++-2.0`  
INCLUDES += `pkg-config --errors-to-stdout --cflags dbgflags` 
INCLUDES += `PKG_CONFIG_PATH=$(top_builddir) pkg-config --errors-to-stdout --cflags IEC61162` 
INCLUDES += -I/home/oc/GitRepositories/cxxtest


AM_CPPFLAGS=-DLOGGER=consoleLogger

noinst_PROGRAMS = AIVDM_test
AIVDM_test_suite = AIVDM_Test.h 
AIVDM_test_SOURCES = tests.cpp  AIVDM_Test.cpp $(AIVDM_test_suite)
AIVDM_test_LDFLAGS = -L$(top_builddir)/libIEC61162
AIVDM_test_LDFLAGS += `pkg-config --errors-to-stdout --libs sigc++-2.0`
AIVDM_test_LDFLAGS += `pkg-config --errors-to-stdout --libs dbgflags` 
AIVDM_test_LDFLAGS += `PKG_CONFIG_PATH=$(top_builddir) pkg-config --errors-to-stdout --libs IEC61162`
AIVDM_test_DEPENDENCIES = $(top_builddir)/libIEC61162/.libs/libIEC61162.so

tests.cpp: $(AIVDM_test_suite)
	/home/oc/GitRepositories/cxxtest/bin/cxxtestgen --xunit-printer --xunit-file=IEC61162_AIVDM_tests_report.xml -o tests.cpp $(AIVDM_test_suite) 