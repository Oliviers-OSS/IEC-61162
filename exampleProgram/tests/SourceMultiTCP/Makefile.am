AUTOMAKE_OPTIONS = gnu
#PKG_CONFIG_PATH=(top_builddir)
INCLUDES = -I$(top_srcdir)/include -I$(top_srcdir)  
#INCLUDES += `pkg-config --errors-to-stdout --cflags dbgflags`  
INCLUDES += -I/home/oc/GitRepositories/cxxtest

AM_CPPFLAGS = -DLOGGER=consoleLogger -DSRCDIR="$(abs_srcdir)" -D_GNU_SOURCE
AM_CPPFLAGS += $(DBGFLAGS_CFLAGS)
AM_CXXFLAGS = -std=c++0x

noinst_PROGRAMS = SourceMultiTCP_tests
SourceMultiTCP_tests_SOURCES = tests.cpp SourceMultiTCP_tests.h ../../tools.h ../../tools.cpp ../../SourceMultiTCP.h ../../SourceMultiTCP.cpp ../../debug.h ../../debug.cpp ../../SourceTCP.h ../../SourceTCP.cpp ../../locks.h ../../locks.cpp
SourceMultiTCP_tests_LDFLAGS = -L$(top_builddir)/libIEC61162
#SourceMultiTCP_tests_LDFLAGS += `pkg-config --errors-to-stdout --libs dbgflags`
SourceMultiTCP_tests_LDFLAGS += $(DBGFLAGS_LIBS)
SourceMultiTCP_tests_LDFLAGS += -lrt -lpthread 

tests.cpp: SourceMultiTCP_tests.h
	/home/oc/GitRepositories/cxxtest/bin/cxxtestgen --xunit-printer --xunit-file=SourceMultiTCP_tests_report.xml -o tests.cpp SourceMultiTCP_tests.h 
