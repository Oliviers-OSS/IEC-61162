dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(IEC-61162, 0.0,oliviers.open.source.softwares@gmail.com,libIEC-61162,https://github.com/Oliviers-OSS/IEC-61162)
LIB_VERSION=0
LIB_REVISION=0
LIB_INTERFACE_AGE=0
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([libIEC61162/libIEC61162.cpp])
AM_CONFIG_HEADER([config.h])
AC_CANONICAL_SYSTEM

dnl Initialize automake
AM_INIT_AUTOMAKE([dist-bzip2])

dnl Initialize Libtool
LT_INIT

dnl Check if Libtool is present
dnl Libtool is used for building share libraries 
dnl AC_PROG_LIBTOOL

dnl Checks for programs.
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB
#AC_CHECK_PROG(CXXTESTGEN,cxxtestgen,[cxxtestgen found],[cxxtestgen not found],[/opt/cxxtest/bin])
#AC_DEFINE(TOTO, "$CXXTESTGEN", [tmp])

dnl this allows us specify individual linking flags for each target
AM_PROG_CC_C_O 

dnl Checks for libraries.
PKG_CHECK_MODULES([LIBSIGC], [sigc++-2.0],[AC_DEFINE([HAVE_LIBSIGC], [1],[Define to 1 if sigc++-2.0 is available])],[AC_MSG_ERROR([cannot find the mandatory libsigc++ 2.0 library"],-1)])
PKG_CHECK_MODULES([DBGFLAGS], [dbgflags],[AC_DEFINE([HAVE_DBGFLAGS], [1], [Define to 1 if Dbgflags is available and must be used])],[AC_MSG_NOTICE([dbgflags not found, stubs will be used for logging])])
dnl optionals libraries used only in the example program
PKG_CHECK_MODULES([ACE], [ACE],[AC_DEFINE([HAVE_ACE], [1], [Define to 1 if the ACE Framework is available])],[AC_MSG_NOTICE([ACE not found, CORBA interface will not disabled in the example program])])
PKG_CHECK_MODULES([ACE_TAO], [TAO],[TAO_Enabled="yes"],[AC_MSG_NOTICE([ACE-TAO not found, CORBA interface will not disabled in the example program])])
if test "$TAO_Enabled" = "yes"; then
	AC_DEFINE([HAVE_TAO], [1], [Define to 1 if the ACE-TAO ORB is available])
    AM_CONDITIONAL(EXAMPLE_PROGRAM_SERVANT_ENABLED,true)
fi
PKG_CHECK_MODULES([ACE_TAO_CosNaming],[TAO_CosNaming],[AC_DEFINE([HAVE_TAO_COSNAMING], [1], [Define to 1 if the ACE-TAO ORB CosNaming is available])],[AC_MSG_NOTICE([ACE-TAO CosNaming  not found, registration of the CORBA interface will not disabled in the example program])])
PKG_CHECK_MODULES([SPATIALITE],[spatialite],[AC_DEFINE([HAVE_SPATIALITE], [1], [Define to 1 if the Spatialite package is available to use it in the example program])],[AC_MSG_NOTICE([Spatialite not found (used only by the example program when available])])
PKG_CHECK_MODULES([SQLITE3],[sqlite3],[AC_DEFINE([HAVE_SQLITE3], [1], [Define to 1 if the SQLite package is available to use it in the example program])],[AC_MSG_NOTICE([SQLite3 not found (used only by the example program when available])])

dnl Checks for header files.
AC_HEADER_STDBOOL
AC_CHECK_HEADERS([fenv.h fcntl.h sys/time.h arpa/inet.h netdb.h netinet/in.h sys/socket.h])

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_MKTIME
AC_CHECK_FUNCS([strrchr memset strchr strstr strtoul pow sqrt clock_gettime getpagesize strncasecmp socket ])

dnl set library version
AC_SUBST(LIB_VERSION)
AC_SUBST(LIB_REVISION)
AC_SUBST(LIB_INTERFACE_AGE)

dnl debug
AC_ARG_ENABLE(debug,
        [  --enable-debug enable debug mode ],
        [case "${enableval}" in
         yes) enable_debug="yes";;
          no) enable_debug="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-debug) ;;
         esac],
        [enable_debug="no"]
)
if test "$enable_debug" = "yes"; then
        AC_DEFINE(_DEBUG_, 1, [Defined if debug mode is enabled (it's easier to check).])        
fi
if test "$enable_debug" = "no"; then
        AC_DEFINE(NDEBUG, 1, [Defined if debug mode is disabled.])
fi

dnl trace call / profiling
AC_ARG_ENABLE(profiling-hooks, [  --enable-profiling-hooks enable profiling hooks ],
       [case "${enableval}" in
         yes) enable_profiling_hooks="yes";;
          no) enable_profiling_hooks="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-profiling-hooks) ;;
         esac],
        [enable_profiling_hooks="no"]
)
if test "$enable_profiling_hooks" = "yes"; then
        AC_DEFINE(PROFILING_HOOKS_ENABLED, 1, [Defined if profiling hooks are enabled])
fi

dnl tests programs
AC_ARG_ENABLE(tests-programs-build,[  --enable-tests-programs-build enable building of the library's tests programs (default is no) ],
        [case "${enableval}" in
           yes) testsProgramsBuild="yes";;
            no) testsProgramsBuild="no" ;;
             *) AC_MSG_ERROR(bad value ${enableval} for --enable-tests-programs-build) ;;
        esac],
        [testsProgramsBuild="no"]
)
AM_CONDITIONAL(TESTS_PROGRAMS_BUILD_ENABLED,[test "$testsProgramsBuild" = "yes"])

dnl developpement mode 
AC_ARG_ENABLE(dev-mode,[  --enable-dev-mode enable developpement mode (for internal use only, default is no) ], 
        [case "${enableval}" in
           yes) devModeBuild="yes";;
            no) devModeBuild="no" ;;
             *) AC_MSG_ERROR(bad value ${enableval} for --enable-dev-mode) ;;
        esac],
        [devModeBuild="no"]
)
AM_CONDITIONAL(DEV_MODE_BUILD_ENABLED,[test "$devModeBuild" = "yes"])

AC_ARG_ENABLE(CORBA-kind,
              AS_HELP_STRING([--enable-CORBA-kind],[enable use of the kind attribut in CORBA Naming Service by the example program (default is yes)]),
	      [case "${enableval}" in
	       yes) enable_CORBA_kind="yes";;
	        no) enable_CORBA_kind="no";;
		 *) AC_MSG_ERROR(bad value ${enableval} for --enable-CORBA-kind) ;;
	       esac],
	       [enable_CORBA_kind="yes"]
)
if test "$enable_CORBA_kind" = "no"; then
   AC_DEFINE(DISABLE_CORBA_NS_KIND,1,[Definable if the CORBA Naming Service Kind attribut must be empty.])
fi

AC_CONFIG_FILES(Makefile
                IEC61162.pc
                libIEC61162/Makefile
                TestPrograms/Makefile
                TestPrograms/proprietarySentence/Makefile
                TestPrograms/AIS/Makefile
                TestPrograms/AIS/BasicRuntimeFunctions/Makefile
                TestPrograms/AIS/FragmentsMgr/Makefile
                TestPrograms/AIS/Message01/Makefile
                TestPrograms/AIS/Message02/Makefile
                TestPrograms/AIS/Message03/Makefile
                TestPrograms/AIS/Message04/Makefile
                TestPrograms/AIS/Message05/Makefile
                TestPrograms/AIS/Message06/Makefile
                TestPrograms/AIS/Message07/Makefile
                TestPrograms/AIS/Message08/Makefile
                TestPrograms/AIS/Message09/Makefile
                TestPrograms/AIS/Message10/Makefile
                TestPrograms/AIS/Message11/Makefile
                TestPrograms/AIS/Message12/Makefile
                TestPrograms/AIS/Message13/Makefile
                TestPrograms/AIS/Message14/Makefile
                TestPrograms/AIS/Message15/Makefile
                TestPrograms/AIS/Message16/Makefile
                TestPrograms/AIS/Message17/Makefile
                TestPrograms/AIS/Message18/Makefile
                TestPrograms/AIS/Message19/Makefile
                TestPrograms/AIS/Message20/Makefile
                TestPrograms/AIS/Message21/Makefile
                TestPrograms/AIS/Message22/Makefile
                TestPrograms/AIS/Message23/Makefile
                TestPrograms/AIS/Message24/Makefile
                TestPrograms/AIS/Message25/Makefile
                TestPrograms/AIS/Message26/Makefile
                TestPrograms/AIS/Message27/Makefile
                TestPrograms/AIS/Alarm/Makefile
                TestPrograms/AIVDM/Makefile
                TestPrograms/AIVDO/Makefile                
                TestPrograms/Performance/Makefile
                TestPrograms/Performance/tests/Makefile
                TestPrograms/Performance/tests/InterThreadsMsgQueue/Makefile
                TestPrograms/Performance/tests/DelimitedFile/Makefile
                exampleProgram/Makefile
                exampleProgram/tests/Makefile
                exampleProgram/tests/Configuration/Makefile
                exampleProgram/tests/SourceTCP/Makefile
                exampleProgram/tests/InternalDatabaseWriter/Makefile
                exampleProgram/tests/SourceMultiTCP/Makefile
                exampleProgram/tests/String/Makefile
                exampleProgram/tests/client/Makefile
                exampleProgram/tests/CorbaClientAdaptor/Makefile
                )
                
AC_OUTPUT
