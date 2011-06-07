set (datafiles
badEstimatesFormat.txt
badFixedFormat.txt
gaussian_model_with_noise.fits
goodEstimatesFormat.txt
jyperbeammap.fits
fixtures/goodAsciiRegionFile.txt
)

foreach (file ${datafiles})
    configure_file (${CMAKE_CURRENT_SOURCE_DIR}/${file} ${CMAKE_CURRENT_BINARY_DIR}/${file} COPYONLY)
endforeach (file)

set (tests
tAsciiAnnotationFileParser
tFitterEstimatesFileParser
)

foreach (test ${tests})
	add_executable (${test}  ${test}.cc)
	target_link_libraries (${test} casa_images)
	add_test (${test} ${CMAKE_SOURCE_DIR}/cmake/cmake_assay ./${test})
endforeach (test)
