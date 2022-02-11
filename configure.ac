dnl Process this file with autoconf to produce a configure script.

AC_INIT([gavl], [2.0.0])
AC_CONFIG_SRCDIR([include/gavl/gavl.h])
AM_INIT_AUTOMAKE

AM_CONFIG_HEADER(include/config.h)

GAVL_VERSION=$VERSION

GAVL_VERSION_MAJOR=`echo $VERSION | cut -d . -f 1`
GAVL_VERSION_MINOR=`echo $VERSION | cut -d . -f 2`
GAVL_VERSION_MICRO=`echo $VERSION | cut -d . -f 3 | cut -d p -f 1`

LTVERSION_CURRENT="2"
LTVERSION_REVISION="0"
LTVERSION_AGE="0"

SAVE_CFLAGS=$CFLAGS

AC_SUBST(GAVL_VERSION)
AC_SUBST(GAVL_VERSION_MAJOR)
AC_SUBST(GAVL_VERSION_MINOR)
AC_SUBST(GAVL_VERSION_MICRO)

AC_SUBST(LTVERSION_CURRENT)
AC_SUBST(LTVERSION_REVISION)
AC_SUBST(LTVERSION_AGE)

AC_CANONICAL_HOST

AC_DISABLE_STATIC

AC_PROG_LIBTOOL
AM_PROG_LIBTOOL

AC_PROG_MAKE_SET

CFLAGS=$SAVE_CFLAGS

dnl
dnl Doxygen
dnl

test_doxygen="true"
have_doxygen="false"

AC_ARG_WITH(doxygen,
    AC_HELP_STRING([--without-doxygen],
    [disable documentation generation]),
    test_doxygen="false")

if test "x$test_doxygen" = "xtrue"; then
  AC_CHECK_PROG(DOXYGEN, doxygen, "doxygen")
  if test "x$DOXYGEN" != "x"; then
    have_doxygen="true"
  fi
fi

AM_CONDITIONAL(HAVE_DOXYGEN, test "x$have_doxygen" = "xtrue")



dnl
dnl Check for compiler flags
dnl  

AC_C_ATTRIBUTE_ALIGNED

AC_C_BIGENDIAN(,,AC_MSG_ERROR("Cannot detect endianess"))

LIBGAVL_CFLAGS=""
LIBGAVL_LIBS=""
LIBGAVL_LDFLAGS=""

APPS_LDFLAGS=""

dnl
dnl Check for mingw
dnl

case "$host" in
	*-*-mingw*|*cygwin*)
	os_win32=yes		
	AC_MSG_WARN([host is $host])
	;;
	*)
	os_win32=no
	;;
esac
AM_CONDITIONAL(OS_WIN32, [test "$os_win32" = "yes"])

if test "$os_win32" = "yes"; then
LIBGAVL_LIBS="-lws2_32 $LIBGAVL_LIBS"

dnl it appears that -no-undefined is necessary in mingw in order to build dll
dnl maybe we should check here if the user only wants static libs

LIBGAVL_LDFLAGS="-no-undefined $LIBGAVL_LDFLAGS"
fi

dnl
dnl gnutls
dnl

PKG_CHECK_MODULES(GNUTLS, gnutls >= "3.5.18",, AC_MSG_ERROR(["gnutls not found"]))

LIBGAVL_CFLAGS="$LIBGAVL_CFLAGS $GNUTLS_CFLAGS"
LIBGAVL_LIBS="$LIBGAVL_LIBS $GNUTLS_LIBS"


dnl
dnl Check for Linker flags
dnl

GMERLIN_CHECK_LDFLAGS

dnl
dnl Check for library functions
dnl 

AC_CHECK_HEADERS([sys/select.h sys/sendfile.h ifaddrs.h])

AC_CHECK_DECLS([MSG_NOSIGNAL, SO_NOSIGPIPE],,,
               [#include <sys/types.h>
	       #include <sys/socket.h>])

AC_CHECK_FUNCS([memalign posix_memalign strdup vasprintf isatty sched_setaffinity])

AC_SYS_LARGEFILE
AC_FUNC_FSEEKO

AC_CHECK_HEADERS([drm/drm_fourcc.h])
AC_CHECK_HEADERS([libdrm/drm_fourcc.h])

dnl
dnl clock_gettime
dnl

AH_TEMPLATE([HAVE_CLOCK_GETTIME],
            [clock_gettime available?])
AH_TEMPLATE([HAVE_CLOCK_MONOTONIC],
            [CLOCK_MONOTONIC available?])

HAVE_CLOCK_GETTIME="true"
HAVE_CLOCK_MONOTONIC="false"

OLD_LIBS=$LIBS
AC_SEARCH_LIBS([clock_gettime], [rt], , HAVE_CLOCK_GETTIME="false")
RT_LIBS=$LIBS
AC_SUBST(RT_LIBS)
LIBS=$OLD_LIBS

if test "x$HAVE_CLOCK_GETTIME" = "xtrue"; then
  AC_DEFINE([HAVE_CLOCK_GETTIME])

OLD_LIBS=$LIBS
LIBS=$RT_LIBS

AC_MSG_CHECKING(for CLOCK_MONOTONIC)

AC_TRY_COMPILE([
  #include <time.h>
  #include <errno.h>
],[
  struct timespec tp;
  clock_gettime(CLOCK_MONOTONIC, &tp);
  ],
  [
    # program could be run
    AC_MSG_RESULT(yes)
    HAVE_CLOCK_MONOTONIC="true"

  ],
    # program could not be run
    AC_MSG_RESULT(no)
)

if test "x$HAVE_CLOCK_MONOTONIC" = "xtrue"; then
  AC_DEFINE([HAVE_CLOCK_MONOTONIC])
fi

fi



dnl
dnl lrint, lrintf
dnl

AC_C99_FUNC_LRINT
AC_C99_FUNC_LRINTF

AC_CHECK_HEADERS(sys/times.h byteswap.h)

dnl
dnl Check for libpng (only needed for some tests)
dnl

GMERLIN_CHECK_LIBPNG

dnl
dnl Build optimization flags
dnl

LQT_OPT_CFLAGS(["-O3 -fomit-frame-pointer"])

dnl
dnl Check for SIMD
dnl

GAVL_CHECK_SIMD($host_cpu, "$OPT_CFLAGS")

dnl
dnl X11
dnl

GMERLIN_CHECK_X11

dnl
dnl Check for libva
dnl

GMERLIN_CHECK_LIBVA

if test "x$have_libva_x11" = "xtrue"; then
LIBGAVL_CFLAGS="$LIBGAVL_CFLAGS $X_CFLAGS"
LIBGAVL_LIBS="$LIBGAVL_LIBS $X_LIBS"
fi

if test "x$have_libva" = "xtrue"; then
LIBGAVL_CFLAGS="$LIBGAVL_CFLAGS $LIBVA_CFLAGS"
LIBGAVL_LIBS="$LIBGAVL_LIBS $LIBVA_LIBS"
fi

dnl
dnl OpenGL
dnl

GMERLIN_CHECK_OPENGL

dnl
dnl Video4linux2
dnl

GMERLIN_CHECK_V4L2

dnl
dnl Now, build the final CFLAGS.
dnl 

CFLAGS="$CFLAGS $OPT_CFLAGS"

LQT_TRY_CFLAGS([-Wall], CFLAGS="$CFLAGS -Wall")
LQT_TRY_CFLAGS([-Wmissing-declarations], CFLAGS="$CFLAGS -Wmissing-declarations" )
LQT_TRY_CFLAGS([-Wdeclaration-after-statement], CFLAGS="$CFLAGS -Wdeclaration-after-statement")
dnl LQT_TRY_CFLAGS([-mfpmath=387], CFLAGS="$CFLAGS -mfpmath=387")
LQT_TRY_CFLAGS([-fvisibility=hidden], CFLAGS="$CFLAGS -fvisibility=hidden")

dnl
dnl Stuff from libsamplerate
dnl

AC_ARG_ENABLE(cpu-clip,
    AC_HELP_STRING([--disable-cpu-clip], [disable tricky cpu specific clipper]),
    ac_arg_cpu_clip="N", ac_arg_cpu_clip="Y")

if test x$ac_arg_cpu_clip = "xY" ; then
        AC_C_CLIP_MODE
else
        echo "checking processor clipping capabilities... disabled"
        ac_cv_c_clip_positive=0
        ac_cv_c_clip_negative=0
        fi

AC_DEFINE_UNQUOTED(CPU_CLIPS_POSITIVE, ${ac_cv_c_clip_positive},
        [Target processor clips on positive float to int conversion.])
AC_DEFINE_UNQUOTED(CPU_CLIPS_NEGATIVE, ${ac_cv_c_clip_negative},
        [Target processor clips on negative float to int conversion.])

AC_CHECK_SIZEOF(int,0)
AC_CHECK_SIZEOF(long,0)
AC_CHECK_SIZEOF(float,4)
AC_CHECK_SIZEOF(double,8)

dnl UD_CHECK_IEEE

dnl Absolute src path for doxygen

TOP_SRCDIR=`cd ${srcdir} && pwd` 
AC_SUBST(TOP_SRCDIR)

dnl
dnl Output variables
dnl

AM_CPPFLAGS='-I$(top_srcdir)/include -I$(top_srcdir)/include/gavl -I$(top_builddir)/include'
AC_SUBST(AM_CPPFLAGS)

LIBGAVL_CFLAGS="$LIBGAVL_CFLAGS -D__GAVL__ -D_FILE_OFFSET_BITS=64"

AC_SUBST(LIBGAVL_CFLAGS)

AC_SUBST(LIBGAVL_LIBS)	
AC_SUBST(LIBGAVL_LDFLAGS)	
AC_SUBST(APPS_LDFLAGS)

AC_OUTPUT(Makefile \
gavl.spec \
gavl.pc \
doc/Makefile \
doc/Doxyfile \
src/Makefile \
m4/Makefile \
utils/Makefile \
include/Makefile \
include/gavl/Makefile \
include/gavl/gavl_version.h \
gavl/Makefile \
gavl/libgdither/Makefile \
gavl/libsamplerate/Makefile \
gavl/c/Makefile \
gavl/gavf/Makefile \
gavl/hq/Makefile \
gavl/mmx/Makefile \
gavl/mmxext/Makefile \
gavl/3dnow/Makefile \
gavl/sse/Makefile \
gavl/sse2/Makefile \
gavl/sse3/Makefile \
gavl/ssse3/Makefile )

