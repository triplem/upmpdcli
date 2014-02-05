
# This should be from configure'd CFLAGS 
AM_CPPFLAGS = -DDEBUG -g -std=c++0x

lib_LTLIBRARIES = libupnpp.la

libupnpp_la_SOURCES = \
    libupnpp/cdircontent.cxx \
    libupnpp/cdirectory.cxx \
    libupnpp/description.cxx \
    libupnpp/device.cxx \
    libupnpp/discovery.cxx \
    libupnpp/ixmlwrap.cxx \
    libupnpp/upnpplib.cxx \
    libupnpp/getsyshwaddr.c \
    libupnpp/vdir.cxx \
    libupnpp/soaphelp.cxx \
    libupnpp/md5.cxx

bin_PROGRAMS = upexplorer upmpdcli

upexplorer_SOURCES = upexplo/upexplo.cxx
upexplorer_LDADD = libupnpp.la -lixml -lupnp -lexpat -lpthread 
#upexplorer_LDFLAGS = 

upmpdcli_SOURCES = upmpd/upmpd.cxx upmpd/mpdcli.cxx upmpd/upmpdutils.cxx
upmpdcli_LDADD = libupnpp.la -lixml -lupnp -lexpat -lmpdclient -lpthread 
#upmpdcli_LDFLAGS = 

dist_pkgdata_DATA = upmpd/description.xml

