m4_sinclude([version.m4])

AC_PREREQ(2.61)
AC_INIT([opendnssec], OPENDNSSEC_VERSION, [http://bugs.opendnssec.org/])

AM_INIT_AUTOMAKE
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE(foreign)
ACX_PREFIXHACK

OPENDNSSEC_COMMON

# extensions
AC_USE_SYSTEM_EXTENSIONS

# standard programs
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LIBTOOL
AC_PROG_MAKE_SET
AC_PROG_RANLIB
AC_PROG_CXX

# additional programs
AC_PATH_PROG(XMLLINT, xmllint)
AC_PATH_PROG(XSLTPROC, xsltproc)
AC_PATH_PROG(JAVA, java)
AC_PATH_PROG(CP, cp)
AC_DEFINE_UNQUOTED(CP_COMMAND, "$CP", [System cp(3) command])

# building
ACX_ARG_RPATH
ACX_RUNTIME_PATH_ADD
ACX_PEDANTIC
ACX_STRICT

# compiler flags
CHECK_COMPILER_FLAG(std=c99, [C99FLAG="-std=c99"])
CHECK_COMPILER_FLAG(xc99, [C99FLAG="-xc99"])
CHECK_COMPILER_FLAG_NEEDED($C99FLAG, [#include <stdbool.h>], [CFLAGS="$CFLAGS $C99FLAG"])
CHECK_COMPILER_FLAG_NEEDED(-D__EXTENSIONS__,
[
#include "confdefs.h"
#include <stdlib.h>
#include <unistd.h>
int test() {
	int a;
	char **opts = NULL;
	a = getopt(2, opts, "a");
	return a;
}
], [CFLAGS="-D__EXTENSIONS__ $CFLAGS"])

# checks for header files
AC_HEADER_STDC
AC_HEADER_STDBOOL
AC_HEADER_TIME
AC_CHECK_HEADERS([fcntl.h inttypes.h stdio.h stdlib.h string.h syslog.h unistd.h])
AC_CHECK_HEADERS(getopt.h,, [AC_INCLUDES_DEFAULT])
AC_CHECK_HEADERS([errno.h getopt.h pthread.h signal.h stdarg.h stdint.h strings.h])
AC_CHECK_HEADERS([sys/select.h sys/socket.h sys/stat.h sys/time.h sys/types.h sys/wait.h])
AC_CHECK_HEADERS([libxml/parser.h libxml/relaxng.h libxml/xmlreader.h libxml/xpath.h])

# checks for typedefs, structures, and compiler characteristics
AC_C_CONST
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_UID_T
AC_TYPE_UINT16_T
AC_MSG_CHECKING(for union semun)
AC_TRY_COMPILE([#include <sys/types.h>
                #include <sys/ipc.h>
                #include <sys/sem.h>],
               [union semun arg;],
               [have_union_semun=1],
               [have_union_semun=0]
)
if test $have_union_semun = 1; then
  AC_MSG_RESULT(yes)
  AC_DEFINE(HAVE_UNION_SEMUN, 1, [union semun is defined])
else
  AC_MSG_RESULT(no)
fi

# pthread
ACX_PTHREAD(
  [AC_DEFINE([HAVE_PTHREAD], [1],
    [Define if you have POSIX threads libraries and header files.])
    LDFLAGS="$CFLAGS $PTHREAD_CFLAGS $LDFLAGS $PTHREAD_LIBS $LIBS"])

LIBS="$PTHREAD_LIBS $LIBS"
CFLAGS="$CFLAGS $PTHREAD_CFLAGS"
CC="$PTHREAD_CC"

# checks for library functions
AC_CHECK_FUNCS([arc4random arc4random_uniform])
AC_CHECK_FUNCS([dup2 endpwent select strerror strtol])
AC_CHECK_FUNCS([getpass getpassphrase memset])
AC_CHECK_FUNCS([localtime_r memset strdup strerror strstr strtol strtoul])
AC_CHECK_FUNCS([setregid setreuid])
AC_CHECK_FUNCS([chown stat exit time atoi getpid waitpid sigfillset])
AC_CHECK_FUNCS([malloc calloc realloc free])
AC_CHECK_FUNCS([strlen strncmp strncat strncpy strerror strncasecmp strdup])
AC_CHECK_FUNCS([fgetc fopen fclose ferror fprintf vsnprintf snprintf fflush])
AC_CHECK_FUNCS([openlog closelog syslog])
AC_CHECK_FUNCS([openlog_r closelog_r syslog_r vsyslog_r])
AC_CHECK_FUNCS([chroot getgroups setgroups initgroups])
AC_CHECK_FUNCS([close unlink fcntl socket listen bzero])
AC_CHECK_FUNCS([va_start va_end])
AC_CHECK_FUNCS([xmlInitParser xmlCleanupParser xmlCleanupThreads])
AC_CHECK_FUNCS([pthread_mutex_init pthread_mutex_destroy pthread_mutex_lock pthread_mutex_unlock])
AC_CHECK_FUNCS([pthread_cond_init pthread_cond_signal pthread_cond_destroy pthread_cond_wait pthread_cond_timedwait])
AC_CHECK_FUNCS([pthread_create pthread_detach pthread_self pthread_join pthread_sigmask])

AC_FUNC_CHOWN
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_FUNC_MKTIME
AC_FUNC_REALLOC
AC_FUNC_SELECT_ARGTYPES
AC_FUNC_VPRINTF

AC_REPLACE_FUNCS(strlcat)
AC_REPLACE_FUNCS(strlcpy)
AC_REPLACE_FUNCS(b64_pton)
AC_REPLACE_FUNCS(b64_ntop)

ACX_BROKEN_SETRES
AC_CHECK_STRPTIME

# check for some target-specific stuff
case "$host" in
*-*-darwin*)
    AC_DEFINE(SETEUID_BREAKS_SETUID, 1,
	    [Define if your platform breaks doing a seteuid before a setuid])
    AC_DEFINE(BROKEN_SETREUID, 1, [Define if your setreuid() is broken])
    AC_DEFINE(BROKEN_SETREGID, 1, [Define if your setregid() is broken])
    ;;
*-*-netbsd*)
    AC_DEFINE(_NETBSD_SOURCE, 1, [Enable functions on BSD])
    ;;
esac

# check for the sockaddr_un.sun_len member
AC_CHECK_MEMBER([struct sockaddr_un.sun_len],
    [AC_DEFINE(HAVE_SOCKADDR_SUN_LEN,1,[Have the sockaddr_un.sun_len member.])],
    [],
    [ #include <sys/types.h>
      #include <sys/un.h>
    ])

# timeshift
AC_ARG_ENABLE(timeshift,
	AC_HELP_STRING([--enable-timeshift], [Enable timeshift debug]),
		[enable_timeshift="${enableval}"],
		[enable_timeshift="no"])
AC_MSG_CHECKING(if we should enable timeshift debugging)
if test "x${enable_timeshift}" = "xyes"; then
	AC_MSG_RESULT(yes)
	AC_DEFINE_UNQUOTED(ENFORCER_TIMESHIFT, 1, [timeshift debug])
else
	AC_MSG_RESULT(no)
fi

# common dependencies
ACX_LIBXML2
ACX_LDNS(1,6,12)
ACX_LDNS_NOT(1,6,14, [binary incompatibility, see http://open.nlnetlabs.nl/pipermail/ldns-users/2012-October/000564.html])
ACX_LDNS_NOT(1,6,15, [fail to create NSEC3 bitmap for empty non-terminals, see http://www.nlnetlabs.nl/pipermail/ldns-users/2012-November/000565.html])
ACX_PKCS11_MODULES
ACX_RT
ACX_LIBC
ACX_SSL
ACX_CUNIT

# cunit
AM_CONDITIONAL([WITH_CUNIT], [test "${with_cunit}" != "no" -a -n "$CUNIT_LIBS"])

# libhsm
ACX_DLOPEN
AC_DEFINE_UNQUOTED([HSM_DEFAULT_CONFIG],["$OPENDNSSEC_CONFIG_FILE"],[Default configuration file.])

# enforcer-ng
AC_ARG_ENABLE(enforcerng,
	AC_HELP_STRING([--disable-enforcerng],
		[Disable enforcerng build (default enabled)]),
		[enable_enforcerng="${enableval}"],
		[enable_enforcerng="yes"])
if test "x${enable_enforcerng}" = "xyes"; then
	AC_DEFINE_UNQUOTED(SIGNER_CLI_UPDATE, "$OPENDNSSEC_SIGNER_CLI update", [Path to the OpenDNSSEC signer engine cli])
	ACX_ENFORCER_DATABASE
	ACX_LIBREADLINE
fi
AH_BOTTOM([#include "stdbool-cpp.h"])
AM_CONDITIONAL([ENABLE_ENFORCERNG], [test "${enable_enforcerng}" = "yes"])

# signer
AC_ARG_ENABLE(signer,
	AC_HELP_STRING([--disable-signer],
		[Disable signer build (default enabled)]),
		[enable_signer="${enableval}"],
		[enable_signer="yes"])
AH_BOTTOM([
	/* define before includes as it specifies what standard to use. */
	#if (defined(HAVE_PSELECT) && !defined (HAVE_PSELECT_PROTO)) \
		|| !defined (HAVE_CTIME_R_PROTO) \
		|| defined (STRPTIME_NEEDS_DEFINES)
	#  ifndef _BSD_SOURCE
	#    define _BSD_SOURCE 1
	#  endif
	#  ifndef __EXTENSIONS__
	#    define __EXTENSIONS__ 1
	#  endif
	#  if !defined(_STDC_C99) && !defined(__cplusplus)
	#    define _STDC_C99 1
	#  endif
	#  ifndef _ALL_SOURCE
	#    define _ALL_SOURCE 1
	#  endif
	#endif
])
AM_CONDITIONAL([ENABLE_SIGNER], [test "${enable_signer}" = "yes"])

# check for a green cucumber (but only if we can shift time)
if test "x${enable_timeshift}" = "xyes"; then
	AC_PATH_PROG(CUCUMBER, cucumber)
fi
AM_CONDITIONAL([WITH_CUCUMBER], [test -x "${CUCUMBER}"])

# doxygen
DX_PDF_FEATURE(OFF)
DX_PS_FEATURE(OFF)
DX_INIT_DOXYGEN(${PACKAGE_NAME}, [Doxyfile], [doxygen-doc])


AC_CONFIG_HEADER([common/config.h])

AC_CONFIG_FILES([
	Makefile
	Doxyfile
	common/Makefile
	conf/Makefile
	conf/addns.xml
	conf/conf.xml
	conf/enforcerstate.xml
	conf/kasp.xml
	conf/signconf.xml
	conf/zonelist.xml
	enforcer-ng/Makefile
	enforcer-ng/src/Makefile
	enforcer-ng/src/utils/Makefile
	enforcer-ng/man/Makefile
	enforcer-ng/src/db/test/Makefile
	enforcer-ng/man/ods-enforcer.8
	enforcer-ng/man/ods-enforcerd.8
	libhsm/Makefile
	libhsm/src/Makefile
	libhsm/src/bin/Makefile
	libhsm/src/bin/ods-hsmspeed.1
	libhsm/src/bin/ods-hsmutil.1
	libhsm/src/lib/Makefile
	libhsm/checks/Makefile
	libhsm/checks/conf-softhsm.xml
	libhsm/checks/conf-sca6000.xml
	libhsm/checks/conf-etoken.xml
	libhsm/checks/conf-multi.xml
	libhsm/checks/conf-opensc.xml
	libhsm/checks/conf-ncipher.xml
	libhsm/checks/conf-aepkeyper.xml
	signer/Makefile
	signer/man/Makefile
	signer/man/ods-signer.8
	signer/man/ods-signerd.8
	signer/src/Makefile
	tools/Makefile
	tools/ods-control
	tools/solaris/Makefile
	tools/solaris/ods-enforcerd.init
	tools/solaris/ods-signerd.init
	tools/ods-timing.5
	tools/opendnssec.7
	tools/ods-control.8
	tools/ods-kasp2html
	mm/Makefile
	mm/src/Makefile
])

AC_OUTPUT
