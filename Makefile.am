ACLOCAL_AMFLAGS=-I m4
SUBDIRS=libIEC61162 exampleProgram 
EXTRA_DIST = IEC61162.pc.in

if TESTS_PROGRAMS_BUILD_ENABLED
    SUBDIRS += TestPrograms
endif

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = IEC61162.pc

cppcheck: 
	cppcheck --enable=all -I$(top_srcdir) -I$(top_srcdir)/include --verbose $(SUBDIRS)
	