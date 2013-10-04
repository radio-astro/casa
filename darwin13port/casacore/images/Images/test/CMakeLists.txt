set (datafiles
imageStats.fits
imagetestimage.fits
jyperpixelimage.fits
mexinputtest.fits
qualityimage.fits
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
)

foreach (file ${datafiles})
    configure_file (${CMAKE_CURRENT_SOURCE_DIR}/${file} ${CMAKE_CURRENT_BINARY_DIR}/${file} COPYONLY)
endforeach (file)


set (tests
dImageInterface
#dImageStatistics
dImageSummary
dPagedImage
tExtendImage
tFITSImage
tFITSImgParser
tFITSErrorImage
tFITSExtImage
tFITSExtImageII
tFITSQualityImage
tHDF5Image
tImageConcat
tImageEmpty
tImageExpr2Gram
tImageExpr3Gram
tImageExprGram
tImageExprParse_addDir
tImageInfo
tImageRegrid
tImageStatistics
tImageUtilities
tLELSpectralIndex
tMIRIADImage
tPagedImage2
tPagedImage
tRebinImage
tSubImage
tTempImage
tImageBeamSet
)

foreach (test ${tests})
	add_executable (${test}  ${test}.cc)
	target_link_libraries (${test} casa_images)
	add_test (${test} ${CMAKE_SOURCE_DIR}/cmake/cmake_assay ./${test})
endforeach (test)
