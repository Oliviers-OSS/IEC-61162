AUTOMAKE_OPTIONS = gnu
#PKG_CONFIG_PATH=(top_builddir)
INCLUDES = -I$(top_srcdir)/include -I$(top_srcdir)  
#INCLUDES += `pkg-config --errors-to-stdout --cflags dbgflags`  
INCLUDES += -I/home/oc/GitRepositories/cxxtest

AM_CPPFLAGS = -DLOGGER=consoleLogger -DSRCDIR="$(abs_srcdir)"
AM_CPPFLAGS += $(DBGFLAGS_CFLAGS)
AM_CXXFLAGS = -std=c++0x

noinst_PROGRAMS = SourceTCP_tests
SourceTCP_tests_SOURCES = tests.cpp SourceTCP_tests.h ../../tools.h ../../tools.cpp ../../SourceTCP.h ../../SourceTCP.cpp ../../debug.h ../../debug.cpp ../../locks.h ../../locks.cpp
SourceTCP_tests_LDFLAGS = -L$(top_builddir)/libIEC61162
#SourceTCP_tests_LDFLAGS += `pkg-config --errors-to-stdout --libs dbgflags`
SourceTCP_tests_LDFLAGS += $(DBGFLAGS_LIBS)
SourceTCP_tests_LDFLAGS += -lrt -lpthread 

tests.cpp: SourceTCP_tests.h
	/home/oc/GitRepositories/cxxtest/bin/cxxtestgen --xunit-printer --xunit-file=SourceTCP_tests_report.xml -o tests.cpp SourceTCP_tests.h 