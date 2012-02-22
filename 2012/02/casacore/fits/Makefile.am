AM_DEFAULT_SOURCE_EXT = .cc

lib_LTLIBRARIES = libcasa_fits.la
libcasa_fits_la_SOURCES = \
        FITS/BasicFITS.cc \
        FITS/BinTable.cc \
        FITS/blockio.cc \
        FITS/CopyRecord.cc \
        FITS/fits.cc \
        FITS/FITSDateUtil.cc \
        FITS/FITSError.cc \
        FITS/FITSHistoryUtil.cc \
        FITS/fitsio.cc \
        FITS/FITSKeywordUtil.cc \
        FITS/FITSMultiTable.cc \
        FITS/FITSSpectralUtil.cc \
        FITS/FITSTable.cc \
        FITS/FITSTable2.cc \
        FITS/FITSTable3.cc \
        FITS/FITSTimedTable.cc \
        FITS/hdu2.cc \
        FITS/FITSReader.cc \
        FITS/SDFITSTable.cc 

# Dependencies for libcasa_fits
AM_CPPFLAGS = -DCASA_USECASAPATH -DCASACORE_NEEDS_RETHROW \
        -I.. \
        -I$(top_srcdir)/casa \
        -I$(top_srcdir)/tables \
        -I$(top_srcdir)/measures

libcasa_fits_la_LIBADD = \
        $(top_builddir)/casa/casa/libcasa_casa.la \
        $(top_builddir)/tables/tables/libcasa_tables.la \
        $(top_builddir)/measures/measures/libcasa_measures.la

libcasa_fits_la_DEPENDENCIES = \
        $(top_builddir)/casa/casa/libcasa_casa.la \
        $(top_builddir)/tables/tables/libcasa_tables.la \
        $(top_builddir)/measures/measures/libcasa_measures.la



check_PROGRAMS = \
        FITS/test/t_priArr_imgExt \
        FITS/test/tBinTable \
        FITS/test/tFITS \
        FITS/test/tfits1 \
        FITS/test/tfits2 \
        FITS/test/tfits3 \
        FITS/test/tfits4 \
        FITS/test/tfits5 \
        FITS/test/tfits_ascTbl \
        FITS/test/tfits_ascTbl2 \
        FITS/test/tfits_binTbl1 \
        FITS/test/tfits_binTbl2 \
        FITS/test/tfits_imgExt2 \
        FITS/test/tfits_priGrp \
        FITS/test/tFITSDateUtil \
        FITS/test/tFITSHistoryUtil \
        FITS/test/tFITSKeywordUtil \
        FITS/test/tfitsread_data \
        FITS/test/tfitsskip \
        FITS/test/tfitsskip_all \
        FITS/test/tfitsskip_hdu \
        FITS/test/tfitsreader \
        FITS/test/tFITSSpectralUtil 

TESTS = $(check_PROGRAMS)
LDADD = libcasa_fits.la
