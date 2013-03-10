AUTOMAKE_OPTIONS = gnu
SUBDIRS = tests
PKG_CONFIG_PATH=(top_builddir)
INCLUDES = -I$(top_srcdir)/include -I$(top_srcdir)
INCLUDES += `pkg-config --errors-to-stdout --cflags sigc++-2.0`  
INCLUDES += `pkg-config --errors-to-stdout --cflags dbgflags` 
INCLUDES += `PKG_CONFIG_PATH=$(top_builddir) pkg-config --errors-to-stdout --cflags IEC61162` 


AM_CPPFLAGS = -DLOGGER=consoleLogger -DSRCDIR="$(abs_srcdir)"
AM_CXXFLAGS = -std=c++0x

noinst_PROGRAMS = Performances_test
Performances_test_SOURCES = main.cpp debug.h globals.h InterThreadsMsgQueue.h tools.h tools.cpp DelimitedFile.h locks.h locks.cpp
Performances_test_LDFLAGS = -L$(top_builddir)/libIEC61162
Performances_test_LDFLAGS += `pkg-config --errors-to-stdout --libs sigc++-2.0`
Performances_test_LDFLAGS += `pkg-config --errors-to-stdout --libs dbgflags` 
Performances_test_LDFLAGS += `PKG_CONFIG_PATH=$(top_builddir) pkg-config --errors-to-stdout --libs IEC61162`
Performances_test_LDFLAGS += -lrt -lpthread
Performances_test_DEPENDENCIES = $(top_builddir)/libIEC61162/.libs/libIEC61162.so
