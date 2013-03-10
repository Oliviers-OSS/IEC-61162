AUTOMAKE_OPTIONS = gnu
#PKG_CONFIG_PATH=(top_builddir)
INCLUDES = -I$(top_srcdir)/include -I$(top_srcdir)  
#INCLUDES += `pkg-config --errors-to-stdout --cflags dbgflags`  
INCLUDES += -I/home/oc/GitRepositories/cxxtest
INCLUDES += -I../..

AM_CPPFLAGS = -DLOGGER=consoleLogger -DSRCDIR="$(abs_srcdir)"
AM_CPPFLAGS += $(DBGFLAGS_CFLAGS)
AM_CXXFLAGS = -std=c++0x

IDL_COMPILER=tao_idl
IDL_COMPILER_OPT=-in 
IDL_COMPILER_GENERATED_HEADERS_FILES = aisS.inl aisS.h aisC.h aisC.inl  
IDL_COMPILER_GENERATED_SOURCES_FILES = aisC.cpp

%S.c %C.h %C.inl %S.inl: %.idl
	$(IDL_COMPILER) $(IDL_COMPILER_OPT) $< 

aisC.h aisC.cpp : ais.idl

noinst_PROGRAMS = CorbaClientAdaptor_tests
CorbaClientAdaptor_tests_SOURCES = tests.cpp CorbaClientAdaptor_tests.h ../../debug.h ../../debug.cpp ../../CorbaClientAdaptor.h
CorbaClientAdaptor_tests_SOURCES += ../../Configuration.h ../../Configuration.cpp 
CorbaClientAdaptor_tests_SOURCES += $(IDL_COMPILER_GENERATED_HEADERS_FILES) $(IDL_COMPILER_GENERATED_SOURCES_FILES)
#CorbaClientAdaptor_tests_LDFLAGS = -L$(top_builddir)/libIEC61162
CorbaClientAdaptor_tests_LDFLAGS = $(DBGFLAGS_LIBS)
CorbaClientAdaptor_tests_LDFLAGS += -lrt -lpthread -lTAO_PortableServer -lTAO -lACE $(TAO_LIBS)

tests.cpp: CorbaClientAdaptor_tests.h
	/home/oc/GitRepositories/cxxtest/bin/cxxtestgen --xunit-printer --xunit-file=CorbaClientAdaptor_tests_report.xml -o tests.cpp CorbaClientAdaptor_tests.h 
	