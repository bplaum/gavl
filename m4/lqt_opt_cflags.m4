#
# Automatic OPT_CFLAGS generation by Burkhard Plaum (2005-05-24)
#

dnl LQT_TRY_CFLAGS (CFLAGS, [ACTION-IF-WORKS], [ACTION-IF-FAILS])
dnl check if $CC supports a given set of cflags
AC_DEFUN([LQT_TRY_CFLAGS],
    [AC_MSG_CHECKING([if $CC supports $1 flags])
    SAVE_CFLAGS="$CFLAGS"
    CFLAGS="$1"
    AC_TRY_COMPILE([],[],[lqt_try_cflags_ok=yes],[lqt_try_cflags_ok=no])
    CFLAGS="$SAVE_CFLAGS"
    AC_MSG_RESULT([$lqt_try_cflags_ok])
    if test x"$lqt_try_cflags_ok" = x"yes"; then
        ifelse([$2],[],[:],[$2])
    else
        ifelse([$3],[],[:],[$3])
    fi])

dnl LQT_OPT_CFLAGS([ADDITIONAL_OPT_FLAGS])
dnl Get proper optimization flags. 
dnl
dnl ADDITIONAL_OPT_FLAGS:  Additional optimization flags (e.g. -O3 --fast-math)
dnl On output, the Variable LQT_OPT_CFLAGS will be set to the compiler flags
dnl Furthermore, is debuggind was requested, the variable LQT_DEBUG will be
dnl set to "true"

AC_DEFUN([LQT_OPT_CFLAGS],[

dnl
dnl Debugging Support
dnl

LQT_DEBUG=false

AC_ARG_ENABLE(debug,
[AC_HELP_STRING([--enable-debug],[Enable debugging, disable optimization])],
[case "${enableval}" in
   yes) LQT_DEBUG=true ;;
   no)  LQT_DEBUG=false ;;
esac],[LQT_DEBUG=false])


dnl
dnl Extra cflags from the commandline. Can have the special values "none" or "auto"
dnl

AC_ARG_WITH(cpuflags, 
AC_HELP_STRING([--with-cpuflags],[Set CPU specific compiler flags. Default is auto, which
               does autodetection. Specify none for compiling the most portable binaries]),
               lqt_cpuflags="$withval", lqt_cpuflags="auto")

if test x$lqt_cpuflags = xnone; then
lqt_cpuflags=""
fi

dnl Let gcc detect the architecture

if test "x$lqt_cpuflags" = "xauto"; then
  lqt_cpuflags="-march=native -mtune=native"
fi

dnl
dnl Build the final flags
dnl

lqt_additional_opt_flags=ifelse([$1],[],[],[$1])

lqt_test_flags=$lqt_cpuflags

if test x$LQT_DEBUG = xtrue; then
  lqt_test_cflags="$lqt_test_flags -g -DDEBUG"
else
  lqt_test_cflags="$lqt_test_flags $lqt_additional_opt_flags"
fi

OPT_CFLAGS=""
for i in $lqt_test_cflags; do
  LQT_TRY_CFLAGS($i, OPT_CFLAGS="$OPT_CFLAGS $i") ;
done

])
