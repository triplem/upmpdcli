AC_INIT([upmpdcli], [0.5], [jfd@lesbonscomptes.com],
             [upmpdcli], [http://www.lesbonscomptes.com/upnpp])
AC_PREREQ([2.53])
AC_CONFIG_SRCDIR([libupnpp/upnpplib.hxx])
AC_CONFIG_HEADERS([libupnpp/config.h])

AM_INIT_AUTOMAKE([1.10 no-define subdir-objects foreign])
LT_INIT

AC_PROG_CXX

# libupnp is configured with large file support, and we need to do the same,
# else a difference in off_t size impacts struct File_Info and prevents the
# vdir to work. This does make a difference, for exemple, for Raspbian
# on the Raspberry PI. Use the same directives as libupnp's configure.ac
AC_TYPE_SIZE_T
AC_TYPE_OFF_T
AC_DEFINE([_LARGE_FILE_SOURCE], [], [Large files support])
AC_DEFINE([_FILE_OFFSET_BITS], [64], [File Offset size])

AC_CHECK_LIB([upnp], [UpnpInit], [true], AC_MSG_ERROR([libupnp not found]))
AC_CHECK_LIB([mpdclient], [mpd_connection_new], [true],
                          AC_MSG_ERROR([libmpdclient not found]))
                     
AC_CHECK_FUNCS([getifaddrs] [UpnpSetLogLevel])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
