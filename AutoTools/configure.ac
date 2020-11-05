#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([inodeum], [0.64], [tony.buckley000@gmail.com])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/um_main.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AM_PROG_CC_C_O

# Checks for specific libraries and headers.
not_inst=""
searchlibs="m:cos:math.h pcap:pcap_findalldevs:pcap.h pthread:pthread_create:pthread.h"

for i in ${searchlibs}
do
    l_lib=""
    lib=`echo ${i} | awk -F: '{print $1}'`
    func=`echo ${i} | awk -F: '{print $2}'`
    AC_SEARCH_LIBS([${func}], [${lib}], [l_lib=yes], [not_inst="${not_inst}  $lib"])

    if test "x${l_lib}" = xyes; then
	hdr=`echo ${i} | awk -F: '{print $3}'`
    	AC_CHECK_HEADERS([${hdr}], [], [not_inst="${not_inst}  $lib ($hdr)"])
    fi
done

if test "x${not_inst}" != x; then
  echo "------------------------------------------"
  echo " Unable to find one or more libraries and/or header files on this system.  "
  echo " The following will need to be installed to build Inodeum:  "
  echo ${not_inst}
  echo " "
  echo " For example, to install 'pthread' and 'pcap' on Ubuntu or Debian   "
  echo " based platforms try the following:  "
  echo "[sudo] apt-get install libpthread-stubs0-dev libpcap-dev  "
  echo "------------------------------------------"

  (exit 1); exit 1;
fi

# Check for packages
PKG_PROG_PKG_CONFIG

not_inst=""
PKG_CHECK_MODULES([GTK], [gtk+-3.0], [], [not_inst="${not_inst} gtk+3.0"])
PKG_CHECK_MODULES([KEYR], [libsecret-1], [], [not_inst="${not_inst} libsecret-1"])
PKG_CHECK_MODULES([SSL], [openssl], [], [not_inst="${not_inst} openssl"])
PKG_CHECK_MODULES([CAIRO], [cairo], [], [not_inst="${not_inst} cairo"])

if test "x${not_inst}" != x; then
  echo "------------------------------------------"
  echo " Unable to find one or more package files on this system.  "
  echo " The following will need to be installed to build Inodeum:  "
  echo ${not_inst}
  echo " "
  echo " As a guide you may any of the following to build Inodeum:  "
  echo "      [sudo] apt-get update                   # Get latest "
  echo "      [sudo] apt-get install build-essential  # Base dev system "
  echo "      [sudo] apt-get install libgtk-3-dev  "
  echo "                             libsecret-1-dev  "
  echo "                             libcairo2-dev*  "
  echo "      [sudo] apt-get install libpcap-dev  "
  echo "                             libssl-dev  "
  echo " "
  echo " To view a list of installed packages enter:  pkg-config --list-all  "
  echo "------------------------------------------"

  (exit 1); exit 1;
fi

# General check for other header files.
AC_HEADER_RESOLV
AC_CHECK_HEADERS([arpa/inet.h ctype.h dirent.h errno.h fcntl.h \
                  libgen.h netdb.h net/if.h netinet/in.h \
                  resolv.h stdint.h stdio.h stdlib.h string.h \
                  sys/ioctl.h sys/socket.h sys/stat.h sys/sysinfo.h sys/types.h \
                  time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_INT64_T
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_MKTIME
AC_FUNC_REALLOC
AC_FUNC_STRTOD
AC_CHECK_FUNCS([clock_gettime gethostbyname inet_ntoa memset mkdir socket \
                strcasecmp strchr strerror strstr strtol sysinfo])

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 data/Makefile
                 data/icons/Makefile])
AC_OUTPUT

echo \
"-------------------------------------------------

 ${PACKAGE_NAME} Version ${PACKAGE_VERSION}

 Prefix: '${prefix}'.
 Compiler: '${CC} ${CFLAGS} ${CPPFLAGS}'

 Now type 'make @<:@<target>@:>@'
   where the optional <target> is:
     all                - build all binaries
     install            - install everything

--------------------------------------------------"

