AM_DEFAULT_SOURCE_EXT = .cc

lib_LTLIBRARIES = libcasa_lattices.la
libcasa_lattices_la_SOURCES = \
        LatticeMath/Fit2D.cc \
        LatticeMath/LatticeFit.cc \
        Lattices/FITSMask.cc \
        Lattices/LatticeAddNoise.cc \
        Lattices/LatticeBase.cc \
        Lattices/LatticeCleanProgress.cc \
        Lattices/LatticeExprNode.cc \
        Lattices/LatticeFFT.cc \
        Lattices/LatticeHistProgress.cc \
        Lattices/LatticeHistSpecialize.cc \
        Lattices/LatticeIndexer.cc \
        Lattices/LatticeLocker.cc \
        Lattices/LatticeNavigator.cc \
        Lattices/LatticeProgress.cc \
        Lattices/LatticeRegion.cc \
        Lattices/LatticeStatsBase.cc \
        Lattices/LatticeStepper.cc \
        Lattices/LattRegionHolder.cc \
        Lattices/LattStatsProgress.cc \
        Lattices/LattStatsSpecialize.cc \
        Lattices/LCBox.cc \
        Lattices/LCComplement.cc \
        Lattices/LCConcatenation.cc \
        Lattices/LCDifference.cc \
        Lattices/LCEllipsoid.cc \
        Lattices/LCExtension.cc \
        Lattices/LCHDF5Mask.cc \
        Lattices/LCIntersection.cc \
        Lattices/LCLELMask.cc \
        Lattices/LCMask.cc \
        Lattices/LCPagedMask.cc \
        Lattices/LCPixelSet.cc \
        Lattices/LCPolygon.cc \
        Lattices/LCRegion.cc \
        Lattices/LCRegion2.cc \
        Lattices/LCRegionFixed.cc \
        Lattices/LCRegionMulti.cc \
        Lattices/LCRegionSingle.cc \
        Lattices/LCSlicer.cc \
        Lattices/LCStretch.cc \
        Lattices/LCUnion.cc \
        Lattices/LELArrayBase.cc \
        Lattices/LELAttribute.cc \
        Lattices/LELBinary2.cc \
        Lattices/LELCoordinates.cc \
        Lattices/LELFunction2.cc \
        Lattices/LELLattCoord.cc \
        Lattices/LELLattCoordBase.cc \
        Lattices/LELRegion.cc \
        Lattices/LELUnary2.cc \
        Lattices/PixelCurve1D.cc \
        Lattices/TiledLineStepper.cc \
        Lattices/TiledShape.cc \
        Lattices/TileStepper.cc 

# Dependencies for libcasa_lattices
AM_CPPFLAGS = -DCASA_USECASAPATH -DCASACORE_NEEDS_RETHROW \
        -I.. \
        -I$(top_srcdir)/casa \
        -I$(top_srcdir)/tables \
        -I$(top_srcdir)/scimath

libcasa_lattices_la_LIBADD = \
        $(top_builddir)/casa/casa/libcasa_casa.la \
        $(top_builddir)/tables/tables/libcasa_tables.la \
        $(top_builddir)/scimath/scimath/libcasa_scimath.la

libcasa_lattices_la_DEPENDENCIES = \
        $(top_builddir)/casa/casa/libcasa_casa.la \
        $(top_builddir)/tables/tables/libcasa_tables.la \
        $(top_builddir)/scimath/scimath/libcasa_scimath.la



check_PROGRAMS = \
        LatticeMath/test/tFit2D \
        LatticeMath/test/tLatticeFit \
        Lattices/test/dLattice \
        Lattices/test/dPagedArray \
        Lattices/test/tArrayLattice \
        Lattices/test/tCurvedLattice2D \
        Lattices/test/tExtendLattice \
        Lattices/test/tHDF5Iterator \
        Lattices/test/tHDF5Lattice \
        Lattices/test/tLatticeAddNoise \
        Lattices/test/tLatticeApply \
        Lattices/test/tLatticeApply2 \
        Lattices/test/tLatticeCache \
        Lattices/test/tLatticeConcat \
        Lattices/test/tLatticeConvolver \
        Lattices/test/tLatticeExpr \
        Lattices/test/tLatticeExpr2 \
        Lattices/test/tLatticeExpr2Node \
        Lattices/test/tLatticeExpr3 \
        Lattices/test/tLatticeExpr3Node \
        Lattices/test/tLatticeExprNode \
        Lattices/test/tLatticeFFT \
        Lattices/test/tLatticeFractile \
        Lattices/test/tLatticeHistograms \
        Lattices/test/tLatticeIndexer \
        Lattices/test/tLatticeIterator \
        Lattices/test/tLatticeLocker \
        Lattices/test/tLatticeRegion \
        Lattices/test/tLatticeSlice1D \
        Lattices/test/tLatticeStatistics \
        Lattices/test/tLatticeStepper \
        Lattices/test/tLatticeTwoPtCorr \
        Lattices/test/tLatticeUtilities \
        Lattices/test/tLCComplement \
        Lattices/test/tLCConcatenation \
        Lattices/test/tLCDifference \
        Lattices/test/tLCExtension \
        Lattices/test/tLCIntersection \
        Lattices/test/tLCLELMask \
        Lattices/test/tLCMask \
        Lattices/test/tLCPagedMask \
        Lattices/test/tLCPixelSet \
        Lattices/test/tLCPolygon \
        Lattices/test/tLCRegion \
        Lattices/test/tLCSlicer \
        Lattices/test/tLCStretch \
        Lattices/test/tLCUnion \
        Lattices/test/tLEL \
        Lattices/test/tLELAttribute \
        Lattices/test/tLELMedian \
        Lattices/test/tPagedArray \
        Lattices/test/tPixelCurve1D \
        Lattices/test/tRebinLattice \
        Lattices/test/tSubLattice \
        Lattices/test/tTempLattice \
        Lattices/test/tTiledLineStepper \
        Lattices/test/tTiledShape \
        Lattices/test/tTileStepper 

TESTS = $(check_PROGRAMS)
LDADD = libcasa_lattices.la
