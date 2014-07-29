set (tests
dAutoDiff
dSparseDiff
tAutoDiff
tCombinatorics
tConvolver
tInterpolateArray1D
tFFTServer
tFFTServer2
tGeometry
tHistAcc
tMathFunc
tMatrixMathLA
tMedianSlider
tSmooth
tSparseDiff
tStatAcc
tVanVleck
tVectorKernel
)

foreach (test ${tests})
    add_executable (${test} ${test}.cc)
    target_link_libraries (${test} casa_scimath)
    add_test (${test} ${CMAKE_SOURCE_DIR}/cmake/cmake_assay ./${test})
endforeach (test)
