AUTOMAKE_OPTIONS = gnu
#PKG_CONFIG_PATH=(top_builddir)
INCLUDES = -I$(top_srcdir)/include -I$(top_srcdir)
INCLUDES += `pkg-config --errors-to-stdout --cflags sigc++-2.0`  
INCLUDES += `pkg-config --errors-to-stdout --cflags dbgflags` 
INCLUDES += `PKG_CONFIG_PATH=$(top_builddir) pkg-config --errors-to-stdout --cflags IEC61162` 
INCLUDES += -I/home/oc/GitRepositories/cxxtest


AM_CPPFLAGS=-DLOGGER=consoleLogger
AM_CXXFLAGS = -std=c++0x

noinst_PROGRAMS = InterThreadsMsgQueue_tests
InterThreadsMsgQueue_tests_SOURCES = tests.cpp InterThreadsMsgQueue_tests.h ../../tools.h ../../tools.cpp InterThreadsMsgQueue_tests.cpp ../../locks.h ../../locks.cpp
InterThreadsMsgQueue_tests_LDFLAGS = -L$(top_builddir)/libIEC61162
InterThreadsMsgQueue_tests_LDFLAGS += `pkg-config --errors-to-stdout --libs sigc++-2.0`
InterThreadsMsgQueue_tests_LDFLAGS += `pkg-config --errors-to-stdout --libs dbgflags`
InterThreadsMsgQueue_tests_LDFLAGS += -lrt -lpthread 
#InterThreadsMsgQueue_tests_LDFLAGS += `PKG_CONFIG_PATH=$(top_builddir) pkg-config --errors-to-stdout --libs IEC61162`
InterThreadsMsgQueue_tests_DEPENDENCIES = $(top_builddir)/libIEC61162/.libs/libIEC61162.so

tests.cpp: InterThreadsMsgQueue_tests.h
	/home/oc/GitRepositories/cxxtest/bin/cxxtestgen --xunit-printer --xunit-file=InterThreadsMsgQueue_tests_report.xml -o tests.cpp InterThreadsMsgQueue_tests.h 