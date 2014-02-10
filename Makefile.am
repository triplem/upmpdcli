
# This should be from configure'd CFLAGS 
AM_CPPFLAGS = -DDEBUG -g -std=c++0x \
            -DDATADIR=\"${pkgdatadir}\" -DCONFIGDIR=\"${sysconfdir}\"

lib_LTLIBRARIES = libupnpp.la

libupnpp_la_SOURCES = \
    libupnpp/cdircontent.cxx \
    libupnpp/cdircontent.hxx \
    libupnpp/cdirectory.cxx \
    libupnpp/cdirectory.hxx \
    libupnpp/description.cxx \
    libupnpp/description.hxx \
    libupnpp/device.cxx \
    libupnpp/device.hxx \
    libupnpp/discovery.cxx \
    libupnpp/discovery.hxx \
    libupnpp/expatmm.hxx \
    libupnpp/getsyshwaddr.c \
    libupnpp/getsyshwaddr.h \
    libupnpp/ixmlwrap.cxx \
    libupnpp/ixmlwrap.hxx \
    libupnpp/log.cxx \
    libupnpp/log.hxx \
    libupnpp/md5.cxx \
    libupnpp/md5.hxx \
    libupnpp/ptmutex.hxx \
    libupnpp/soaphelp.cxx \
    libupnpp/soaphelp.hxx \
    libupnpp/upnpp_p.hxx \
    libupnpp/upnpplib.cxx \
    libupnpp/upnpplib.hxx \
    libupnpp/vdir.cxx \
    libupnpp/vdir.hxx \
    libupnpp/workqueue.hxx

libupnpp_la_LIBADD = -lixml -lupnp -lexpat -lpthread -lrt

bin_PROGRAMS = upmpdcli #upexplorer 

#upexplorer_SOURCES = upexplo/upexplo.cxx
#upexplorer_LDADD = libupnpp.la -lixml -lupnp -lexpat -lpthread -lrt
#upexplorer_LDFLAGS = 

upmpdcli_SOURCES = \
     upmpd/conftree.cxx \
     upmpd/conftree.hxx \
     upmpd/mpdcli.cxx \
     upmpd/mpdcli.hxx \
     upmpd/upmpd.cxx \
     upmpd/upmpdutils.cxx \
     upmpd/upmpdutils.hxx

upmpdcli_LDADD = libupnpp.la -lmpdclient 


dist_pkgdata_DATA = upmpd/description.xml upmpd/AVTransport.xml \
                  upmpd/RenderingControl.xml
dist_sysconf_DATA = upmpd/upmpdcli.conf

