AUTOMAKE_OPTIONS = gnu
#PKG_CONFIG_PATH=(top_builddir)
INCLUDES = -I$(top_srcdir)/include -I$(top_srcdir)  
#INCLUDES += `pkg-config --errors-to-stdout --cflags dbgflags`  
INCLUDES += -I/home/oc/GitRepositories/cxxtest

AM_CPPFLAGS = -D_GNU_SOURCE
#AM_CXXFLAGS = -std=c++0x

noinst_PROGRAMS = String_tests

String_tests_SOURCES = tests.cpp String_tests.h ../../String.h ../../String.cpp 

tests.cpp: String_tests.h
	/home/oc/GitRepositories/cxxtest/bin/cxxtestgen --xunit-printer --xunit-file=String_tests_report.xml -o tests.cpp String_tests.h 