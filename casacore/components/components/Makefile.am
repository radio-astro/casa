AM_DEFAULT_SOURCE_EXT = .cc

lib_LTLIBRARIES = libcasa_components.la
libcasa_components_la_SOURCES = \
        ComponentModels/ComponentList.cc \
        ComponentModels/ComponentShape.cc \
        ComponentModels/ComponentType.cc \
        ComponentModels/ComponentType2.cc \
        ComponentModels/ConstantSpectrum.cc \
        ComponentModels/DiskShape.cc \
        ComponentModels/FluxStandard.cc \
        ComponentModels/GaussianShape.cc \
        ComponentModels/PointShape.cc \
        ComponentModels/SkyCompBase.cc \
        ComponentModels/SkyComponent.cc \
        ComponentModels/SkyCompRep.cc \
        ComponentModels/SpectralIndex.cc \
        ComponentModels/SpectralModel.cc \
        ComponentModels/TwoSidedShape.cc \
        SpectralComponents/Spectral2Element.cc \
        SpectralComponents/SpectralElement.cc \
        SpectralComponents/SpectralEstimate.cc \
        SpectralComponents/SpectralFit.cc \
        SpectralComponents/SpectralList.cc 

# Dependencies for libcasa_components
AM_CPPFLAGS = -DCASA_USECASAPATH -DCASACORE_NEEDS_RETHROW \
        -I.. \
        -I$(top_srcdir)/casa \
        -I$(top_srcdir)/tables \
        -I$(top_srcdir)/scimath \
        -I$(top_srcdir)/measures \
        -I$(top_srcdir)/coordinates

libcasa_components_la_LIBADD = \
        $(top_builddir)/casa/casa/libcasa_casa.la \
        $(top_builddir)/tables/tables/libcasa_tables.la \
        $(top_builddir)/scimath/scimath/libcasa_scimath.la \
        $(top_builddir)/measures/measures/libcasa_measures.la \
        $(top_builddir)/coordinates/coordinates/libcasa_coordinates.la

libcasa_components_la_DEPENDENCIES = \
        $(top_builddir)/casa/casa/libcasa_casa.la \
        $(top_builddir)/tables/tables/libcasa_tables.la \
        $(top_builddir)/scimath/scimath/libcasa_scimath.la \
        $(top_builddir)/measures/measures/libcasa_measures.la \
        $(top_builddir)/coordinates/coordinates/libcasa_coordinates.la



check_PROGRAMS = \
        ComponentModels/test/dPointShape \
        ComponentModels/test/dSkyCompBase \
        ComponentModels/test/dSpectralModel \
        ComponentModels/test/dTwoSidedShape \
        ComponentModels/test/tComponentList \
        ComponentModels/test/tComponentType \
        ComponentModels/test/tConstantSpectrum \
        ComponentModels/test/tDiskShape \
        ComponentModels/test/tGaussianComponent \
        ComponentModels/test/tGaussianShape \
        ComponentModels/test/tPointComponent \
        ComponentModels/test/tPointShape \
        ComponentModels/test/tSkyCompRep \
        ComponentModels/test/tSpectralIndex \
        SpectralComponents/test/tProfileFit1D \
        SpectralComponents/test/tSpectralFit 

TESTS = $(check_PROGRAMS)
LDADD = libcasa_components.la
