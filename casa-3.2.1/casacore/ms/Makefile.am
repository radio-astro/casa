AM_DEFAULT_SOURCE_EXT = .cc

lib_LTLIBRARIES = libcasa_ms.la
libcasa_ms_la_SOURCES = \
        MeasurementSets/MeasurementSet.cc \
        MeasurementSets/MS1ToMS2Converter.cc \
        MeasurementSets/MSAntenna.cc \
        MeasurementSets/MSAntennaColumns.cc \
        MeasurementSets/MSAntennaGram.cc \
        MeasurementSets/MSAntennaIndex.cc \
        MeasurementSets/MSAntennaParse.cc \
        MeasurementSets/MSArrayGram.cc \
        MeasurementSets/MSArrayParse.cc \
        MeasurementSets/MSColumns.cc \
        MeasurementSets/MSConcat.cc \
        MeasurementSets/MSCorrGram.cc \
        MeasurementSets/MSCorrParse.cc \
        MeasurementSets/MSDataDescColumns.cc \
        MeasurementSets/MSDataDescIndex.cc \
        MeasurementSets/MSDataDescription.cc \
        MeasurementSets/MSDerivedValues.cc \
        MeasurementSets/MSDoppler.cc \
        MeasurementSets/MSDopplerColumns.cc \
        MeasurementSets/MSDopplerIndex.cc \
        MeasurementSets/MSDopplerUtil.cc \
        MeasurementSets/MSFeed.cc \
        MeasurementSets/MSFeedColumns.cc \
        MeasurementSets/MSFeedIndex.cc \
        MeasurementSets/MSField.cc \
        MeasurementSets/MSFieldColumns.cc \
        MeasurementSets/MSFieldGram.cc \
        MeasurementSets/MSFieldIndex.cc \
        MeasurementSets/MSFieldParse.cc \
        MeasurementSets/MSFlagCmd.cc \
        MeasurementSets/MSFlagCmdColumns.cc \
        MeasurementSets/MSFlagger.cc \
        MeasurementSets/MSFreqOffColumns.cc \
        MeasurementSets/MSFreqOffIndex.cc \
        MeasurementSets/MSFreqOffset.cc \
        MeasurementSets/MSHistory.cc \
        MeasurementSets/MSHistoryColumns.cc \
        MeasurementSets/MSHistoryHandler.cc \
        MeasurementSets/MSIter.cc \
        MeasurementSets/MSLister.cc \
        MeasurementSets/MSMainColumns.cc \
        MeasurementSets/MSObsColumns.cc \
        MeasurementSets/MSObservation.cc \
        MeasurementSets/MSObsIndex.cc \
        MeasurementSets/MSParse.cc \
        MeasurementSets/MSPointing.cc \
        MeasurementSets/MSPointingColumns.cc \
        MeasurementSets/MSPointingIndex.cc \
        MeasurementSets/MSPolarization.cc \
        MeasurementSets/MSPolColumns.cc \
        MeasurementSets/MSPolIndex.cc \
        MeasurementSets/MSProcessor.cc \
        MeasurementSets/MSProcessorColumns.cc \
        MeasurementSets/MSRange.cc \
        MeasurementSets/MSReader.cc \
        MeasurementSets/MSScanGram.cc \
        MeasurementSets/MSScanParse.cc \
        MeasurementSets/MSSelection.cc \
        MeasurementSets/MSSelectionError.cc \
        MeasurementSets/MSSelectionKeywords.cc \
        MeasurementSets/MSSelectionTools.cc \
        MeasurementSets/MSSelector.cc \
        MeasurementSets/MSSimulator.cc \
        MeasurementSets/MSSource.cc \
        MeasurementSets/MSSourceColumns.cc \
        MeasurementSets/MSSourceIndex.cc \
        MeasurementSets/MSSpectralWindow.cc \
        MeasurementSets/MSSpwGram.cc \
        MeasurementSets/MSSpwIndex.cc \
        MeasurementSets/MSSpWindowColumns.cc \
        MeasurementSets/MSSpWindowIndex.cc \
        MeasurementSets/MSSpwParse.cc \
        MeasurementSets/MSState.cc \
        MeasurementSets/MSStateColumns.cc \
        MeasurementSets/MSSummary.cc \
        MeasurementSets/MSSysCal.cc \
        MeasurementSets/MSSysCalColumns.cc \
        MeasurementSets/MSSysCalIndex.cc \
        MeasurementSets/MSTable2.cc \
        MeasurementSets/MSTableImpl.cc \
        MeasurementSets/MSTableIndex.cc \
        MeasurementSets/MSTileLayout.cc \
        MeasurementSets/MSTimeGram.cc \
        MeasurementSets/MSTimeParse.cc \
        MeasurementSets/MSUvDistGram.cc \
        MeasurementSets/MSUvDistParse.cc \
        MeasurementSets/MSValidIds.cc \
        MeasurementSets/MSWeather.cc \
        MeasurementSets/MSWeatherColumns.cc \
        MeasurementSets/MSWeatherIndex.cc \
        MeasurementSets/NewMSSimulator.cc \
        MeasurementSets/StokesConverter.cc 

# Dependencies for libcasa_ms
AM_CPPFLAGS = -DCASA_USECASAPATH -DCASACORE_NEEDS_RETHROW \
        -I.. \
        -I$(top_srcdir)/casa \
        -I$(top_srcdir)/tables \
        -I$(top_srcdir)/scimath \
        -I$(top_srcdir)/measures

libcasa_ms_la_LIBADD = \
        $(top_builddir)/casa/casa/libcasa_casa.la \
        $(top_builddir)/tables/tables/libcasa_tables.la \
        $(top_builddir)/scimath/scimath/libcasa_scimath.la \
        $(top_builddir)/measures/measures/libcasa_measures.la

libcasa_ms_la_DEPENDENCIES = \
        $(top_builddir)/casa/casa/libcasa_casa.la \
        $(top_builddir)/tables/tables/libcasa_tables.la \
        $(top_builddir)/scimath/scimath/libcasa_scimath.la \
        $(top_builddir)/measures/measures/libcasa_measures.la

SUBDIRS = MeasurementSets .



check_PROGRAMS = \
        MeasurementSets/test/tMeasurementSet \
        MeasurementSets/test/tMSAntennaGram \
        MeasurementSets/test/tMSAntennaGram2 \
        MeasurementSets/test/tMSColumns \
        MeasurementSets/test/tMSCorrGram \
        MeasurementSets/test/tMSDataDescBuffer \
        MeasurementSets/test/tMSDerivedValues \
        MeasurementSets/test/tMSFieldBuffer \
        MeasurementSets/test/tMSFieldGram \
        MeasurementSets/test/tMSMainBuffer \
        MeasurementSets/test/tMSPolBuffer \
        MeasurementSets/test/tMSReader \
        MeasurementSets/test/tMSScanGram \
        MeasurementSets/test/tMSSimulator \
        MeasurementSets/test/tMSSpwGram \
        MeasurementSets/test/tMSSummary \
        MeasurementSets/test/tMSTimeGram \
        MeasurementSets/test/tMSUvDistGram \
        MeasurementSets/test/tStokesConverter 

TESTS = $(check_PROGRAMS)
LDADD = libcasa_ms.la
