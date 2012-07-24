set (datafiles
fixtures/goodAsciiRegionFile.txt
)

foreach (file ${datafiles})
    configure_file (${CMAKE_CURRENT_SOURCE_DIR}/${file} ${CMAKE_CURRENT_BINARY_DIR}/${file} COPYONLY)
endforeach (file)

set (tests
tAnnAnnulus
tAnnCenterBox
tAnnCircle
tAnnEllipse
tAnnLine
tAnnPolygon
tAnnRectBox
tAnnRotBox
tAnnSymbol
tAnnText
tAnnVector
tRegionTextList
)

foreach (test ${tests})
    add_executable (${test} ${test}.cc)
    target_link_libraries (${test} casa_images)
    add_test (${test} ${CMAKE_SOURCE_DIR}/cmake/cmake_assay ./${test})
endforeach (test)
