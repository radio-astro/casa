set (datafiles
imageStats.fits
imagetestimage.fits
jyperpixelimage.fits
mexinputtest.fits
qualityimage.fits
ngc5921.clean.fits
ngc5921.clean.no_freq.no_stokes.fits
test_image.fits
test_image.im/table.dat
test_image.im/table.f0
test_image.im/table.f0_TSM0
test_image.im/table.info
test_image.im/table.lock
test_image.im/logtable/table.dat
test_image.im/logtable/table.f0
test_image.im/logtable/table.info
test_image.im/logtable/table.lock
test_image2dconvolver_convolved.fits
test_image2dconvolver.fits
test_image2dconvolver_multibeam.im/logtable/table.dat
test_image2dconvolver_multibeam.im/logtable/table.info
test_image2dconvolver_multibeam.im/logtable/table.info
test_image2dconvolver_multibeam.im/logtable/table.f0
test_image2dconvolver_multibeam.im/logtable/table.lock
test_image2dconvolver_multibeam.im/table.dat
test_image2dconvolver_multibeam.im/table.f0
test_image2dconvolver_multibeam.im/table.f0_TSM0
test_image2dconvolver_multibeam.im/table.info
test_image2dconvolver_multibeam.im/table.lock

)

foreach (file ${datafiles})
    configure_file (${CMAKE_CURRENT_SOURCE_DIR}/${file} ${CMAKE_CURRENT_BINARY_DIR}/${file} COPYONLY)
endforeach (file)


set (tests
dImageHistograms
dImageInterface
dImageMoments
dImageStatistics
dImageSummary
dPagedImage
tAntennaResponses
tComponentImager
tExtendImage
tFITSImage
tFITSImgParser
tFITSErrorImage
tFITSExtImage
tFITSExtImageII
tFITSQualityImage
tHDF5Image
tImage2DConvolver
tImageConcat
tImageConvolver
tImageDecomposer
tImageEmpty
tImageExpr2Gram
tImageExpr3Gram
tImageExprGram
tImageExprParse_addDir
tImageFFT
tImageFit1D
tImageInfo
tImageMetaData
tImageMoments
tImagePolarimetry
tImageRegrid
tImageSourceFinder
tImageStatistics
tImageUtilities
tLELSpectralIndex
tMIRIADImage
tPagedImage2
tPagedImage
tRebinImage
tSepImConvolver
tSubImage
tTempImage
tImageBeamSet
)

foreach (test ${tests})
	add_executable (${test}  ${test}.cc)
	target_link_libraries (${test} casa_images)
	add_test (${test} ${CMAKE_SOURCE_DIR}/cmake/cmake_assay ./${test})
endforeach (test)
