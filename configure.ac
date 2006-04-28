AC_INIT([avogadro], [.00-cvs], [Donald E. Curtis <dcurtis@cs.uiowa.edu>])
AC_PREREQ([2.50])

AC_DEFINE(APP_NAME, "Avogadro", "Proper Name of the Package")

AC_CANONICAL_SYSTEM

AM_CONFIG_HEADER(config.h)
AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)

AC_PROG_CXX
AC_PROG_INSTALL

AC_HEADER_STDC
AC_HEADER_SYS_WAIT
#AC_CHECK_HEADERS(signal.h fcntl.h sys/socket.h sys/un.h errno.h fcntl.h)
#
#AC_MSG_CHECKING([for sun_len in sys/un.h])
    #AC_EGREP_HEADER([sun_len], [sys/un.h],
      #[AC_MSG_RESULT([yes])
         #AC_DEFINE([HAVE_SOCKADDR_UN_SUN_LEN], [1],
             #[Define if sockaddr_un in sys/un.h contains a sun_len component])],
              #AC_MSG_RESULT([no]))
#
#AM_PATH_GLIB(1.2.10,,AC_MSG_ERROR([Cannot find proper glib version]))
#AM_PATH_GTK(1.2.10,,AC_MSG_ERROR([Cannot find proper gtk version]))
#
#AC_PATH_PROG(GLADECONFIG, libglade-config)
#if test -n "$GLADECONFIG"; then
  #LIBGLADE_LIBS=`libglade-config --libs`
  #AC_SUBST(LIBGLADE_LIBS)
  #LIBGLADE_INCLUDES=`libglade-config --cflags`
  #AC_SUBST(LIBGLADE_INCLUDES)
#else
  #AC_MSG_ERROR("GTK-Gamess Requires libglade-config ( part of the libglade development libraries ).")
#fi
#
#AC_PATH_PROG(XML2CONFIG, xml2-config)
#if test -n "$XML2CONFIG"; then
  #LIBXML2_LIBS=`xml2-config --libs`
  #AC_SUBST(LIBXML2_LIBS)
  #LIBXML2_INCLUDES=`xml2-config --cflags`
  #AC_SUBST(LIBXML2_INCLUDES)
#else
  #AC_MSG_ERROR("GTK-Gamess Requires xml2-config ( part of the xml2 development libraries ).")
#fi
#
	#dnl ****************************
	#dnl *** Checks for socklen_t ***
	#dnl ****************************
#
 	#AC_MSG_CHECKING([for socklen_t])
	#ac_cv_socklen_t=""
	#AC_TRY_COMPILE([
##include <sys/types.h>
##include <sys/socket.h>
	#],[
#socklen_t a=0;
#getsockname(0,(struct sockaddr*)0, &a);
	#],
		#ac_cv_socklen_t="socklen_t",
		#AC_TRY_COMPILE([
##include <sys/types.h>
##include <sys/socket.h>
		#],[
#int a=0;
#getsockname(0,(struct sockaddr*)0, &a);
		#],
			#ac_cv_socklen_t="int",
			#ac_cv_socklen_t="size_t"
		#)
	#)
#
	#AC_MSG_RESULT($ac_cv_socklen_t)
	#if test "$ac_cv_socklen_t" != "socklen_t"; then
		#AC_DEFINE_UNQUOTED(socklen_t, $ac_cv_socklen_t,
			#[Define the real type of socklen_t])
	#fi
#
##AC_ARG_WITH(tmp,[  --with-tmp=TMP          Temp Directory (default = /tmp)],
            ##tmp_directory="$withval", tmp_directory="")
#
##if test x${tmp_directory} != x ; then
  ##TMP_DEFINE="-DTMP_DIR=\\\"$tmp_directory\\\""
  ##AC_SUBST(TMP_DEFINE)
##fi
#
## AM_INIT_AUTOMAKE

AC_CONFIG_FILES([Makefile src/Makefile])


AC_OUTPUT





