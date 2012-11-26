#include <casa/aips.h>
#include <synthesis/MSVis/FinalTvi2.h>
#include <synthesis/MSVis/UtilJ.h>
#include <synthesis/MSVis/VisBuffer2.h>

namespace casa {

namespace vi {

FinalTvi2::FinalTvi2 (ViImplementation2 * inputVi, VisibilityIterator2 * vi,
                      MeasurementSet & finalMs, Bool isWritable)
: TransformingVi2 (inputVi),
  ms_p (finalMs)
{
    setVisBuffer (VisBuffer2::factory (vi, VbPlain, isWritable));
}

FinalTvi2::~FinalTvi2 ()
{
}

void
FinalTvi2::writeBackChanges (VisBuffer2 * vb)
{
    Throw ("Not Implemented");
}

void
FinalTvi2::writeFlag (const Matrix<Bool> & flag)
{
    Throw ("Not Implemented");
}

void
FinalTvi2::writeFlag (const Cube<Bool> & flag)
{
    Throw ("Not Implemented");
}

void
FinalTvi2::writeFlagRow (const Vector<Bool> & rowflags)
{
    Throw ("Not Implemented");
}

void
FinalTvi2::writeFlagCategory(const Array<Bool>& fc)
{
    Throw ("Not Implemented");
}

void
FinalTvi2::writeVisCorrected (const Matrix<CStokesVector> & visibilityStokes)
{
    Throw ("Not Implemented");
}
void
FinalTvi2::writeVisModel (const Matrix<CStokesVector> & visibilityStokes)
{
    Throw ("Not Implemented");
}
void
FinalTvi2::writeVisObserved (const Matrix<CStokesVector> & visibilityStokes)
{
    Throw ("Not Implemented");
}

void
FinalTvi2::writeVisCorrected (const Cube<Complex> & vis)
{
    Throw ("Not Implemented");
}
void
FinalTvi2::writeVisModel (const Cube<Complex> & vis)
{
    Throw ("Not Implemented");
}
void
FinalTvi2::writeVisObserved (const Cube<Complex> & vis)
{
    Throw ("Not Implemented");
}

void
FinalTvi2::writeWeight (const Vector<Float> & wt)
{
    Throw ("Not Implemented");
}

void
FinalTvi2::writeWeightMat (const Matrix<Float> & wtmat)
{
    Throw ("Not Implemented");
}

void
FinalTvi2::writeWeightSpectrum (const Cube<Float> & wtsp)
{
    Throw ("Not Implemented");
}

void
FinalTvi2::writeSigma (const Vector<Float> & sig)
{
    Throw ("Not Implemented");
}

void
FinalTvi2::writeSigmaMat (const Matrix<Float> & sigmat)
{
    Throw ("Not Implemented");
}

void
FinalTvi2::writeModel(const RecordInterface& rec, Bool iscomponentlist,
                      Bool incremental)
{
    Throw ("Not Implemented");
}

} // end namespace vi

} //# NAMESPACE CASA - END



