AM_DEFAULT_SOURCE_EXT = .cc

lib_LTLIBRARIES = libcasa_scimath.la
libcasa_scimath_la_SOURCES = \
        Fitting/FittingProxy.cc \
        Fitting/LSQFit.cc \
        Fitting/LSQFit3.cc \
        Fitting/LSQMatrix.cc \
        Fitting/LSQMatrix2.cc \
        Functionals/FuncExprData.cc \
        Functionals/FuncExpression.cc \
        Functionals/FunctionalProxy.cc \
        Functionals/FunctionFactoryErrors.cc \
        Functionals/SerialHelper.cc \
        Mathematics/FFTW.cc \
        Mathematics/Interpolate2D.cc \
        Mathematics/MathFunc2.cc \
        Mathematics/MatrixSolver.cc \
        Mathematics/MedianSlider.cc \
        Mathematics/NNLSMatrixSolver.cc \
        Mathematics/NumericTraits.cc \
        Mathematics/RigidVector2.cc \
        Mathematics/SCSL.cc \
        Mathematics/SquareMatrix2.cc \
        Mathematics/VanVleck.cc \
        Mathematics/VectorKernel.cc 

# Dependencies for libcasa_scimath
AM_CPPFLAGS = -DCASA_USECASAPATH -DCASACORE_NEEDS_RETHROW \
        -I.. \
        -I$(top_srcdir)/casa

libcasa_scimath_la_LIBADD = \
        $(top_builddir)/casa/casa/libcasa_casa.la

libcasa_scimath_la_DEPENDENCIES = \
        $(top_builddir)/casa/casa/libcasa_casa.la



check_PROGRAMS = \
        Fitting/test/dConstraints \
        Fitting/test/dLSQFit \
        Fitting/test/tFitGaussian \
        Fitting/test/tLinearFitSVD \
        Fitting/test/tLSQaips \
        Fitting/test/tLSQFit \
        Fitting/test/tNonLinearFitLM \
        Functionals/test/dFunction \
        Functionals/test/dGaussianND \
        Functionals/test/tChebyshev \
        Functionals/test/tCombiFunction \
        Functionals/test/tCompoundFunction \
        Functionals/test/tFuncExpression \
        Functionals/test/tFunctionHolder \
        Functionals/test/tFunctionOrder \
        Functionals/test/tFunctionWrapper \
        Functionals/test/tGaussian1D \
        Functionals/test/tGaussian2D \
        Functionals/test/tGaussian3D \
        Functionals/test/tGaussianND \
        Functionals/test/tHyperPlane \
        Functionals/test/tInterpolate1D \
        Functionals/test/tPolynomial \
        Functionals/test/tSampledFunctional \
        Functionals/test/tSimButterworthBandpass \
        Functionals/test/tSinusoid1D \
        Functionals/test/tSPolynomial \
        Mathematics/test/dAutoDiff \
        Mathematics/test/dSparseDiff \
        Mathematics/test/tAutoDiff \
        Mathematics/test/tConvolver \
        Mathematics/test/tFFTServer \
        Mathematics/test/tHistAcc \
        Mathematics/test/tMathFunc \
        Mathematics/test/tMatrixMathLA \
        Mathematics/test/tMedianSlider \
        Mathematics/test/tSparseDiff \
        Mathematics/test/tStatAcc \
        Mathematics/test/tVanVleck 

TESTS = $(check_PROGRAMS)
LDADD = libcasa_scimath.la
