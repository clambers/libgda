m4_define([_OSLC_CHECK_INTERNAL],
  [AC_BEFORE([AC_PROG_LIBTOOL], [$0])
   AC_BEFORE([AM_PROG_LIBTOOL], [$0])
   AC_BEFORE([LT_INIT], [$0])

   AC_ARG_ENABLE([oslc],
     [AS_HELP_STRING([--disable-oslc], [disable OSLC provider])],
     [],
     [enable_oslc=yes])

   AS_IF([test "x$enable_oslc" != xno],
     [OSLC_CFLAGS="$(GIO_CFLAGS)"
      OSLC_LIBS="$(GIO_LIBS)"])

   AM_CONDITIONAL([OSLC], [test "x$enable_oslc" = xyes])
   AC_SUBST([OSLC_CFLAGS])
   AC_SUBST([OSLC_LIBS])])

AC_DEFUN([OSLC_CHECK],
  [_OSLC_CHECK_INTERNAL()])
