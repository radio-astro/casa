AM_DEFAULT_SOURCE_EXT = .cc

lib_LTLIBRARIES = libcasa_coordinates.la
libcasa_coordinates_la_SOURCES = \
        Coordinates/Coordinate.cc \
        Coordinates/CoordinateSystem.cc \
        Coordinates/CoordinateUtil.cc \
        Coordinates/Direction2Coordinate.cc \
        Coordinates/DirectionCoordinate.cc \
        Coordinates/FITSCoordinateUtil.cc \
        Coordinates/GaussianConvert.cc \
        Coordinates/LinearCoordinate.cc \
        Coordinates/LinearXform.cc \
        Coordinates/LinearXform2.cc \
        Coordinates/ObsInfo.cc \
        Coordinates/Projection.cc \
        Coordinates/Spectral2Coordinate.cc \
        Coordinates/SpectralCoordinate.cc \
        Coordinates/StokesCoordinate.cc \
        Coordinates/TabularCoordinate.cc 

# Dependencies for libcasa_coordinates
AM_CPPFLAGS = -DCASA_USECASAPATH -DCASACORE_NEEDS_RETHROW \
        -I.. \
        -I$(top_srcdir)/casa \
        -I$(top_srcdir)/scimath \
        -I$(top_srcdir)/measures \
        -I$(top_srcdir)/fits

libcasa_coordinates_la_LIBADD = \
        $(top_builddir)/casa/casa/libcasa_casa.la \
        $(top_builddir)/scimath/scimath/libcasa_scimath.la \
        $(top_builddir)/measures/measures/libcasa_measures.la \
        $(top_builddir)/fits/fits/libcasa_fits.la

libcasa_coordinates_la_DEPENDENCIES = \
        $(top_builddir)/casa/casa/libcasa_casa.la \
        $(top_builddir)/scimath/scimath/libcasa_scimath.la \
        $(top_builddir)/measures/measures/libcasa_measures.la \
        $(top_builddir)/fits/fits/libcasa_fits.la



check_PROGRAMS = \
        Coordinates/test/dCoordinates \
        Coordinates/test/dRemoveAxes \
        Coordinates/test/dWorldMap \
        Coordinates/test/tCoordinate \
        Coordinates/test/tCoordinateSystem \
        Coordinates/test/tCoordinateUtil \
        Coordinates/test/tDirectionCoordinate \
        Coordinates/test/tFrequencyAligner \
        Coordinates/test/tGaussianConvert \
        Coordinates/test/tLinearCoordinate \
        Coordinates/test/tLinearXform \
        Coordinates/test/tObsInfo \
        Coordinates/test/tProjection \
        Coordinates/test/tSpectralCoordinate \
        Coordinates/test/tStokesCoordinate \
        Coordinates/test/tTabularCoordinate 

TESTS = $(check_PROGRAMS)
LDADD = libcasa_coordinates.la
