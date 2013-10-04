AM_DEFAULT_SOURCE_EXT = .cc

lib_LTLIBRARIES = libcasa_images.la
libcasa_images_la_SOURCES = \
        Images/ComponentImager.cc \
        Images/FITSImage.cc \
        Images/HDF5Image2.cc \
        Images/ImageAnalysis.cc \
        Images/ImageExprGram.cc \
        Images/ImageExprParse.cc \
        Images/ImageFFT.cc \
        Images/ImageFITS2Converter.cc \
        Images/ImageInfo.cc \
        Images/ImageMomentsProgress.cc \
        Images/ImageOpener.cc \
        Images/ImagePolarimetry.cc \
        Images/ImagePolProxy.cc \
        Images/ImageProfileFit.cc \
        Images/ImageProxy.cc \
        Images/ImageUtilities.cc \
        Images/LELImageCoord.cc \
        Images/MaskSpecifier.cc \
        Images/MIRIADImage.cc \
        Images/PagedImage2.cc \
        Regions/AipsIOReaderWriter.cc \
        Regions/ImageRegion.cc \
        Regions/RegionHandler.cc \
        Regions/RegionHandlerHDF5.cc \
        Regions/RegionHandlerMemory.cc \
        Regions/RegionHandlerTable.cc \
        Regions/RegionManager.cc \
        Regions/RFReaderWriter.cc \
        Regions/WCBox.cc \
        Regions/WCComplement.cc \
        Regions/WCCompound.cc \
        Regions/WCConcatenation.cc \
        Regions/WCDifference.cc \
        Regions/WCExtension.cc \
        Regions/WCIntersection.cc \
        Regions/WCLELMask.cc \
        Regions/WCPolygon.cc \
        Regions/WCRegion.cc \
        Regions/WCRegion2.cc \
        Regions/WCUnion.cc \
        Wnbt/ComponentUpdate.cc 

# Dependencies for libcasa_images
AM_CPPFLAGS = -DCASA_USECASAPATH -DCASACORE_NEEDS_RETHROW \
        -I.. \
        -I$(top_srcdir)/casa \
        -I$(top_srcdir)/tables \
        -I$(top_srcdir)/scimath \
        -I$(top_srcdir)/measures \
        -I$(top_srcdir)/fits \
        -I$(top_srcdir)/coordinates \
        -I$(top_srcdir)/components \
        -I$(top_srcdir)/lattices \
        -I$(top_srcdir)

libcasa_images_la_LIBADD = \
        $(top_builddir)/casa/casa/libcasa_casa.la \
        $(top_builddir)/tables/tables/libcasa_tables.la \
        $(top_builddir)/scimath/scimath/libcasa_scimath.la \
        $(top_builddir)/measures/measures/libcasa_measures.la \
        $(top_builddir)/fits/fits/libcasa_fits.la \
        $(top_builddir)/coordinates/coordinates/libcasa_coordinates.la \
        $(top_builddir)/components/components/libcasa_components.la \
        $(top_builddir)/lattices/lattices/libcasa_lattices.la \
        $(top_builddir)/mirlib/libcasa_mirlib.la

libcasa_images_la_DEPENDENCIES = \
        $(top_builddir)/casa/casa/libcasa_casa.la \
        $(top_builddir)/tables/tables/libcasa_tables.la \
        $(top_builddir)/scimath/scimath/libcasa_scimath.la \
        $(top_builddir)/measures/measures/libcasa_measures.la \
        $(top_builddir)/fits/fits/libcasa_fits.la \
        $(top_builddir)/coordinates/coordinates/libcasa_coordinates.la \
        $(top_builddir)/components/components/libcasa_components.la \
        $(top_builddir)/lattices/lattices/libcasa_lattices.la \
        $(top_builddir)/mirlib/libcasa_mirlib.la

SUBDIRS = Images .



check_PROGRAMS = \
        Images/test/dImageHistograms \
        Images/test/dImageInterface \
        Images/test/dImageMoments \
        Images/test/dImageProfileFit \
        Images/test/dImageStatistics \
        Images/test/dImageSummary \
        Images/test/dPagedImage \
        Images/test/fits2image \
        Images/test/image2fits \
        Images/test/tComponentImager \
        Images/test/tExtendImage \
        Images/test/tFITSImage \
        Images/test/tHDF5Image \
        Images/test/tImageConcat \
        Images/test/tImageConvolver \
        Images/test/tImageDecomposer \
        Images/test/tImageExpr2Gram \
        Images/test/tImageExpr3Gram \
        Images/test/tImageExprGram \
        Images/test/tImageExprParse_addDir \
        Images/test/tImageFFT \
        Images/test/tImageFit1D \
        Images/test/tImageInfo \
        Images/test/tImagePolarimetry \
        Images/test/tImageRegrid \
        Images/test/tImageSourceFinder \
        Images/test/tImageUtilities \
        Images/test/tLELSpectralIndex \
        Images/test/tMIRIADImage \
        Images/test/tPagedImage \
        Images/test/tPagedImage2 \
        Images/test/tRebinImage \
        Images/test/tSepImConvolver \
        Images/test/tSubImage \
        Images/test/tTempImage \
        Regions/test/tRegionHandler \
        Regions/test/tWCBox \
        Regions/test/tWCExtension \
        Regions/test/tWCLELMask \
        Regions/test/tWCUnion \
        Wnbt/test/tComponentUpdate 

TESTS = $(check_PROGRAMS)
LDADD = libcasa_images.la
