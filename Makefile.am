lib_LTLIBRARIES = libupnpp.la

libupnpp_la_SOURCES = \
    libupnpp/cdircontent.cxx \
    libupnpp/cdirectory.cxx \
    libupnpp/description.cxx \
    libupnpp/discovery.cxx \
    libupnpp/ixmlwrap.cxx \
    libupnpp/upnpplib.cxx

bin_PROGRAMS = upexplorer

upexplorer_SOURCES = upexplo/upexplo.cxx
upexplorer_LDADD = libupnpp.la
upexplorer_LDFLAGS = -lixml -lupnp -lexpat -lpthread 
