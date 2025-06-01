dnl
dnl Standardized linker flags:
dnl We use --as-needed for executables and
dnl --no-undefined for libraries
dnl

AC_DEFUN([GMERLIN_CHECK_LDFLAGS],[

GMERLIN_LIB_LDFLAGS=""
GMERLIN_EXE_LDFLAGS=""

AC_MSG_CHECKING(if linker supports --no-undefined)
OLD_LDFLAGS=$LDFLAGS
LDFLAGS="$LDFLAGS -Wl,--no-undefined"

AC_LINK_IFELSE([AC_LANG_SOURCE([[int main() { return 0; } ]])],
            [GMERLIN_LIB_LDFLAGS="-Wl,--no-undefined $GMERLIN_LIB_LDFLAGS"; AC_MSG_RESULT(Supported)],
            [AC_MSG_RESULT(Unsupported)])
LDFLAGS=$OLD_LDFLAGS

AC_MSG_CHECKING(if linker supports --as-needed)
OLD_LDFLAGS=$LDFLAGS
LDFLAGS="$LDFLAGS -Wl,--as-needed"
AC_LINK_IFELSE([AC_LANG_SOURCE([[int main() { return 0; }]])],
            [GMERLIN_EXE_LDFLAGS="-Wl,--as-needed $GMERLIN_EXE_LDFLAGS"; AC_MSG_RESULT(Supported)],
            [AC_MSG_RESULT(Unsupported)])
LDFLAGS=$OLD_LDFLAGS

AC_SUBST(GMERLIN_LIB_LDFLAGS)
AC_SUBST(GMERLIN_EXE_LDFLAGS)

])

dnl
dnl PNG
dnl 

AC_DEFUN([GMERLIN_CHECK_LIBPNG],[

AH_TEMPLATE([HAVE_LIBPNG], [Enable png codec])
 
have_libpng=false
PNG_REQUIRED="1.2.2"

AC_ARG_ENABLE(libpng,
[AS_HELP_STRING([--disable-libpng],[Disable libpng (default: autodetect)])],
[case "${enableval}" in
   yes) test_libpng=true ;;
   no)  test_libpng=false ;;
esac],[test_libpng=true])

if test x$test_libpng = xtrue; then

PKG_CHECK_MODULES(PNG, libpng, have_libpng="true", have_libpng="false")
fi

AC_SUBST(PNG_CFLAGS)
AC_SUBST(PNG_LIBS)
AC_SUBST(PNG_REQUIRED)

AM_CONDITIONAL(HAVE_LIBPNG, test x$have_libpng = xtrue)

if test x$have_libpng = xtrue; then
AC_DEFINE(HAVE_LIBPNG)
fi

])



dnl
dnl OpenGL
dnl
AC_DEFUN([GMERLIN_CHECK_OPENGL],[
AH_TEMPLATE([HAVE_GL],[OpenGL available])
AH_TEMPLATE([HAVE_GLX],[GLX available])
AH_TEMPLATE([HAVE_EGL],[EGL available])

dnl
dnl Search for OpenGL libraries
dnl

OLD_LIBS=$LIBS

have_GL="true"
AC_SEARCH_LIBS([glBegin], [GL], [], AC_MSG_ERROR([OpenGL not found]), [])

if test "x$have_GL" = "xtrue"; then
AC_LINK_IFELSE([AC_LANG_SOURCE([[#include <GL/gl.h>
				 int main()
				 {
				 if(0)
				   glBegin(GL_QUADS); return 0;
				 } ]])],
	       [],AC_MSG_ERROR([Linking OpenGL program failed]))
fi

GL_LIBS=$LIBS

LIBS="$OLD_LIBS"

dnl
dnl Check for EGL
dnl

OLD_LIBS=$LIBS

have_EGL="true"
AC_SEARCH_LIBS([eglGetCurrentDisplay], [GL EGL], [], AC_MSG_ERROR([EGL not found]), [])

if test "x$have_GL" = "xtrue"; then
AC_LINK_IFELSE([AC_LANG_SOURCE([[#include <EGL/egl.h>
				 int main()
				 {
				 if(0)
				   eglGetCurrentDisplay();
				 return 0;
				 }
				 ]])],
               [],AC_MSG_ERROR([Linking EGL program failed]))
fi

EGL_LIBS=$LIBS

LIBS="$OLD_LIBS"

if test "x$have_GL" = "xtrue"; then
AC_DEFINE(HAVE_GL)

if test "x$have_EGL" = "xtrue"; then
AC_DEFINE(HAVE_EGL)
fi

fi

AM_CONDITIONAL(HAVE_GL, test x$have_GL = xtrue)
AM_CONDITIONAL(HAVE_EGL, test x$have_EGL = xtrue)

AC_SUBST(GL_CFLAGS)
AC_SUBST(GL_LIBS)
AC_SUBST(EGL_CFLAGS)
AC_SUBST(EGL_LIBS)

])

dnl
dnl GLU
dnl

AC_DEFUN([GMERLIN_CHECK_GLU],[
AH_TEMPLATE([HAVE_GLU],[GLU available])

OLD_CFLAGS=$CFLAGS
OLD_LIBS=$LIBS

have_GLU="true"
AC_SEARCH_LIBS([gluLookAt], [GLU], [], [have_GLU="false"], [])

if test "x$have_GLU" = "xtrue"; then
AC_TRY_LINK([#include <GL/glu.h>],[
if(0) gluLookAt(0, 0, 0, 0, 0, 0, 0, 0, 0); return 0;
],[],[have_GLU="false"])
fi

GLU_CFLAGS=$CFLAGS
GLU_LIBS=$LIBS

CFLAGS="$OLD_CFLAGS"
LIBS="$OLD_LIBS"

if test "x$have_GLU" = "xtrue"; then
AC_DEFINE(HAVE_GLU)
fi

AM_CONDITIONAL(HAVE_GLU, test x$have_GLU = xtrue)

AC_SUBST(GLU_CFLAGS)
AC_SUBST(GLU_LIBS)

])

dnl
dnl Video4linux2
dnl

AC_DEFUN([GMERLIN_CHECK_V4L2],[

AH_TEMPLATE([HAVE_V4L2], [Enable v4l2])
	     
have_v4l2=false
AC_ARG_ENABLE(v4l2,
              AS_HELP_STRING(--disable-v4l2, [Disable Video4Linux (default: autodetect)]),
              [case "${enableval}" in
                 yes) test_v4l2=true ;;
                 no) test_v4l2=false ;;
               esac],
	       test_v4l2=true)

if test x$test_v4l2 = xtrue; then
AC_CHECK_HEADERS(linux/videodev2.h, have_v4l2=true)
fi

AM_CONDITIONAL(HAVE_V4L2, test x$have_v4l2 = xtrue)

if test x$have_v4l2 = xtrue; then
AC_DEFINE(HAVE_V4L2)
fi


])

dnl
dnl X11
dnl

AC_DEFUN([GMERLIN_CHECK_X11],[

have_x="false"

X_CFLAGS=""
X_LIBS=""

AH_TEMPLATE([HAVE_XLIB],
            [Do we have xlib installed?])

AC_PATH_X

if test x$no_x != xyes; then
  if test "x$x_includes" != "x"; then
    X_CFLAGS="-I$x_includes"
  elif test -d /usr/X11R6/include; then 
    X_CFLAGS="-I/usr/X11R6/include"
  else
    X_CFLAGS=""
  fi

  if test "x$x_libraries" != "x"; then
    X_LIBS="-L$x_libraries -lX11"
  else
    X_LIBS="-lX11"
  fi
  have_x="true"
else
  PKG_CHECK_MODULES(X, x11 >= 1.0.0, have_x=true, have_x=false)
fi

if test x$have_x = xtrue; then
  X_LIBS="$X_LIBS -lXext"
  AC_DEFINE([HAVE_XLIB])
fi


AC_SUBST(X_CFLAGS)
AC_SUBST(X_LIBS)
AM_CONDITIONAL(HAVE_X11, test x$have_x = xtrue)

])


dnl
dnl libva
dnl

AC_DEFUN([GMERLIN_CHECK_LIBVA],[

AH_TEMPLATE([HAVE_LIBVA],
            [Do we have libva installed?])
AH_TEMPLATE([HAVE_LIBVA_X11],
            [Do we have libva (x11) installed?])

have_libva="false"
have_libva_x11="false"

LIBVA_CFLAGS=""
LIBVA_LIBS=""

AC_ARG_ENABLE(libva,
[AS_HELP_STRING([--disable-libva],[Disable libva (default: autodetect)])],
[case "${enableval}" in
   yes) test_libva=true ;;
   no)  test_libva=false ;;
esac],[test_libva=true])

if test x$have_x != xtrue; then
test_libva="false"
fi

if test x$test_libva = xtrue; then
PKG_CHECK_MODULES(LIBVA_BASE, libva, have_libva="true", have_libva="false")
fi

if test "x$have_libva" = "xtrue"; then
LIBVA_CFLAGS=$LIBVA_BASE_CFLAGS
LIBVA_LIBS=$LIBVA_BASE_LIBS

if test x$have_x = xtrue; then
PKG_CHECK_MODULES(LIBVA_X11, libva-x11, have_libva_x11="true", have_libva_x11="false")
fi

if test "x$have_libva_x11" = "xtrue"; then
LIBVA_CFLAGS="$LIBVA_CFLAGS $LIBVA_X11_CFLAGS"
LIBVA_LIBS="$LIBVA_LIBS $LIBVA_X11_LIBS"
else
have_libva="false"
fi

fi

AC_SUBST(LIBVA_LIBS)
AC_SUBST(LIBVA_CFLAGS)

AM_CONDITIONAL(HAVE_LIBVA, test x$have_libva = xtrue)
AM_CONDITIONAL(HAVE_LIBVA_X11, test x$have_libva_x11 = xtrue)

if test "x$have_libva" = "xtrue"; then
AC_DEFINE([HAVE_LIBVA])
fi

if test "x$have_libva_x11" = "xtrue"; then
AC_DEFINE([HAVE_LIBVA_X11])
fi

])
