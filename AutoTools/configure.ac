#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([inodeum], [0.50], [tony.buckley000@gmail.com])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/um_main.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AM_PROG_CC_C_O

# Checks for specific libraries and headers.
not_inst=""
searchlibs="crypto:OPENSSl_init:openssl/bio.h " \
	   "m:cos:math.h " \
	   "pcap:pcap_findalldevs:pcap.h " \
	   "pthread:pthread_create:pthread.h " \
	   "ssl:SSL_library_init:openssl/ssl.h " \
	   "cairo:cairo_stroke:cairo.h " \
	   "gtk-3:gtk_init:gtk/gtk.h " \
	   "gnome-keyring:gnome_keyring_list_item_ids_sync:gnome-keyring.h"

for i in ${searchlibs}
do
    l_lib=""
    lib=`echo ${i} | awk -F: '{print $1}'`
    func=`echo ${i} | awk -F: '{print $2}'`
    AC_SEARCH_LIBS([${func}], [${lib}], [l_lib=yes], [not_inst="${not_inst} $lib"])

    if test "x${l_lib}" = xyes; then
	hdr=`echo ${i} | awk -F: '{print $3}'`
    	AC_CHECK_HEADERS([${hdr}], [], [not_inst="${not_inst}  $lib"])
    fi
done

if test "x${not_inst}" != x; then
  echo "------------------------------------------"
  echo " Unable to find one or more libraries and/or header files on this system.  "
  echo " The following will need to be installed to build Inodeum:  "
  echo ${not_inst}

  echo " "
  echo " For example on Ubuntu or Debian based platforms try the following:  "
  echo "[sudo] apt-get install ${not_inst}  "
  echo "------------------------------------------"
fi

if test "x${l_gtk}" != xyes || test "x{h_gtk]" = "xno"; then
  echo " "
  echo "------------------------------------------"
  echo " It appears that the GTK development libraries and/or header files  "
  echo " are not pressent on this system.  "
  echo " To install try:    [sudo] apt-get install libgtk-3-dev  "
  echo " Note that depending on what development tools are present  "
  echo " you may need to run:    [sudo] apt-get build-dep libgtk-3-dev "
  echo "------------------------------------------"
  (exit 1); exit 1;
fi

# General check for other header files.
LDFLAGS=`pkg-config --libs gtk+-3.0 gnome-keyring-1 cairo glib-2.0`;
CPPFLAGS=`pkg-config --cflags gtk+-3.0 gnome-keyring-1 glib-2.0`;
AC_HEADER_RESOLV
AC_CHECK_HEADERS([arpa/inet.h cairo-xlib.h ctype.h dirent.h errno.h fcntl.h \
                  gdk/gdkkeysyms.h glib.h gnome-keyring-memory.h \
                  gtk/gtk.h libgen.h netdb.h net/if.h  netinet/in.h \
                  openssl/err.h openssl/pem.h openssl/x509.h openssl/x509_vfy.h \
                  resolv.h stdint.h stdio.h stdlib.h string.h \
                  sys/ioctl.h sys/socket.h sys/stat.h sys/sysinfo.h sys/types.h \
                  time.h unistd.h])

# pkg-config module checks for cflags and linker flags
PKG_PROG_PKG_CONFIG

PKG_CHECK_MODULES([X], [gtk+-3.0 gnome-keyring-1 cairo])

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

