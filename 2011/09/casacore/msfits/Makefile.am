AM_DEFAULT_SOURCE_EXT = .cc

lib_LTLIBRARIES = libcasa_msfits.la
libcasa_msfits_la_SOURCES = \
        MSFits/FitsIDItoMS.cc \
        MSFits/MSFitsIDI.cc \
        MSFits/MSFitsInput.cc \
        MSFits/MSFitsOutput.cc \
        MSFits/SDAntennaHandler.cc \
        MSFits/SDDataDescHandler.cc \
        MSFits/SDFeedHandler.cc \
        MSFits/SDFieldHandler.cc \
        MSFits/SDFITSHandler.cc \
        MSFits/SDHistoryHandler.cc \
        MSFits/SDMainHandler.cc \
        MSFits/SDObservationHandler.cc \
        MSFits/SDPointingHandler.cc \
        MSFits/SDPolarizationHandler.cc \
        MSFits/SDSourceHandler.cc \
        MSFits/SDSpWinHandler.cc \
        MSFits/SDSysCalHandler.cc \
        MSFits/SDWeatherHandler.cc 

# Dependencies for libcasa_msfits
AM_CPPFLAGS = -DCASA_USECASAPATH -DCASACORE_NEEDS_RETHROW \
        -I.. \
        -I$(top_srcdir)/casa \
        -I$(top_srcdir)/tables \
        -I$(top_srcdir)/scimath \
        -I$(top_srcdir)/measures \
        -I$(top_srcdir)/fits \
        -I$(top_srcdir)/ms

libcasa_msfits_la_LIBADD = \
        $(top_builddir)/casa/casa/libcasa_casa.la \
        $(top_builddir)/tables/tables/libcasa_tables.la \
        $(top_builddir)/scimath/scimath/libcasa_scimath.la \
        $(top_builddir)/measures/measures/libcasa_measures.la \
        $(top_builddir)/fits/fits/libcasa_fits.la \
        $(top_builddir)/ms/ms/libcasa_ms.la

libcasa_msfits_la_DEPENDENCIES = \
        $(top_builddir)/casa/casa/libcasa_casa.la \
        $(top_builddir)/tables/tables/libcasa_tables.la \
        $(top_builddir)/scimath/scimath/libcasa_scimath.la \
        $(top_builddir)/measures/measures/libcasa_measures.la \
        $(top_builddir)/fits/fits/libcasa_fits.la \
        $(top_builddir)/ms/ms/libcasa_ms.la



check_PROGRAMS = \
        MSFits/test/tfits2ms \
        MSFits/test/tMSConcat \
        MSFits/test/tMSSelection 

TESTS = $(check_PROGRAMS)
LDADD = libcasa_msfits.la
