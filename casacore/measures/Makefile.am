AM_DEFAULT_SOURCE_EXT = .cc

lib_LTLIBRARIES = libcasa_measures.la
libcasa_measures_la_SOURCES = \
        Measures/Aberration.cc \
        Measures/EarthField.cc \
        Measures/EarthMagneticMachine.cc \
        Measures/MBaseline.cc \
        Measures/MCBase.cc \
        Measures/MCBaseline.cc \
        Measures/MCDirection.cc \
        Measures/MCDoppler.cc \
        Measures/MCEarthMagnetic.cc \
        Measures/MCEpoch.cc \
        Measures/MCFrame.cc \
        Measures/MCFrequency.cc \
        Measures/MConvertBase.cc \
        Measures/MCPosition.cc \
        Measures/MCRadialVelocity.cc \
        Measures/MCuvw.cc \
        Measures/MDirection.cc \
        Measures/MDoppler.cc \
        Measures/MEarthMagnetic.cc \
        Measures/MeasComet.cc \
        Measures/MeasData.cc \
        Measures/MeasFrame.cc \
        Measures/MeasIERS.cc \
        Measures/MeasJPL.cc \
        Measures/MeasMath.cc \
        Measures/MeasTable.cc \
        Measures/Measure.cc \
        Measures/MeasureHolder.cc \
        Measures/MeasuresProxy.cc \
        Measures/MEpoch.cc \
        Measures/MFrequency.cc \
        Measures/MPosition.cc \
        Measures/MRadialVelocity.cc \
        Measures/MRBase.cc \
        Measures/Muvw.cc \
        Measures/Nutation.cc \
        Measures/ParAngleMachine.cc \
        Measures/Precession.cc \
        Measures/SofaTest.cc \
        Measures/SolarPos.cc \
        Measures/Stokes.cc \
        Measures/UVWMachine.cc \
        Measures/VelocityMachine.cc \
        TableMeasures/TableMeasColumn.cc \
        TableMeasures/TableMeasDescBase.cc \
        TableMeasures/TableMeasOffsetDesc.cc \
        TableMeasures/TableMeasRefDesc.cc \
        TableMeasures/TableMeasType.cc \
        TableMeasures/TableMeasValueDesc.cc \
        TableMeasures/TableQuantumDesc.cc 

# Dependencies for libcasa_measures
AM_CPPFLAGS = -DCASA_USECASAPATH -DCASACORE_NEEDS_RETHROW \
        -I.. \
        -I$(top_srcdir)/casa \
        -I$(top_srcdir)/tables \
        -I$(top_srcdir)/scimath

libcasa_measures_la_LIBADD = \
        $(top_builddir)/casa/casa/libcasa_casa.la \
        $(top_builddir)/tables/tables/libcasa_tables.la \
        $(top_builddir)/scimath/scimath/libcasa_scimath.la

libcasa_measures_la_DEPENDENCIES = \
        $(top_builddir)/casa/casa/libcasa_casa.la \
        $(top_builddir)/tables/tables/libcasa_tables.la \
        $(top_builddir)/scimath/scimath/libcasa_scimath.la



check_PROGRAMS = \
        Measures/test/dM1950_2000 \
        Measures/test/dMeasure \
        Measures/test/tEarthField \
        Measures/test/tEarthMagneticMachine \
        Measures/test/tIAU2000 \
        Measures/test/tMBaseline \
        Measures/test/tMEarthMagnetic \
        Measures/test/tMeasComet \
        Measures/test/tMeasJPL \
        Measures/test/tMeasMath \
        Measures/test/tMeasure \
        Measures/test/tMeasureHolder \
        Measures/test/tMuvw \
        Measures/test/tParAngleMachine \
        Measures/test/tRecordTransformable \
        Measures/test/tStokes \
        Measures/test/tUVWMachine \
        Measures/test/tVelocityMachine \
        TableMeasures/test/tTableMeasures \
        TableMeasures/test/tTableQuantum 

TESTS = $(check_PROGRAMS)
LDADD = libcasa_measures.la
