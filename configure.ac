dnl Ce fichier est utilisé par autoconf pour produire un script configure.

AC_INIT(configure.ac)
AM_INIT_AUTOMAKE(gshutdown, 0.2)
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE

# Programmes
AC_PROG_CC
AC_PROG_CC_STDC
AC_PROG_INSTALL

# Headers
AC_HEADER_STDC
AC_ISC_POSIX
AC_CHECK_HEADERS([libintl.h stdlib.h string.h sys/wait.h signal.h])

# Typedefs, structures
AC_C_CONST
AC_STRUCT_TM

# Arguments
AM_BINRELOC
AC_ARG_ENABLE(debug,
              [AC_HELP_STRING([--enable-debug],
			      [enable debug mode. It's used under development (default=no)])],
	      enable_debug=$enableval,
	      enable_debug="no")

if test "$enable_debug" = "yes"; then
	CFLAGS="-ansi -Wall -g -DDEBUG"
    #-pedantic 
fi

AC_ARG_ENABLE(libnotify,
              [AC_HELP_STRING([--enable-libnotify],
			      [enable libnotify notifications (default=yes)])],
	      enable_libnotify=$enableval,
	      enable_libnotify="yes")

if test "$enable_libnotify" = "yes"; then
	libnotify="libnotify >= 0.3.2"
	CFLAGS="$CFLAGS -DLIBNOTIFY_ENABLED"
else
	libnotify=""
fi

# gtk+2
pkg_modules="gtk+-2.0 >= 2.0.0
             glib-2.0 >= 2.6
             libglade-2.0 >= 2.0.0
             $libnotify"
PKG_CHECK_MODULES(PACKAGE, [$pkg_modules])
AC_SUBST(PACKAGE_CFLAGS)
AC_SUBST(PACKAGE_LIBS)

# Gettext
ALL_LINGUAS="fr es ca sv pt de da hu ru it no sr sr@Latn pl zh"
GETTEXT_PACKAGE=gshutdown
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [Gettext package])
AM_GLIB_GNU_GETTEXT

AC_OUTPUT([
Makefile
man/Makefile
src/Makefile
po/Makefile.in
gshutdown.spec
gshutdown.lsm
gshutdown.desktop
PKGBUILD
autopackage/default.apspec
])

#echo
#echo "CFLAGS : $CFLAGS"
#echo

