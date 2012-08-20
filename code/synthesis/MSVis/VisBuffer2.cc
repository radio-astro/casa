/*
 * VisBuffer2.cc
 *
 *  Created on: Jul 3, 2012
 *      Author: jjacobs
 */


#include <casa/aipstype.h>
#include <synthesis/MSVis/VisBuffer2.h>
#include <synthesis/MSVis/VisBufferImpl2.h>
#include <synthesis/MSVis/VisibilityIterator2.h>
#include <synthesis/MSVis/UtilJ.h>

namespace casa {

using namespace vi;

VisBuffer2::VisBuffer2(Type type)
: vb_p (0)
{
    construct (0, type);
}

VisBuffer2::VisBuffer2 (ROVisibilityIterator2 * vi, Type type)
: vb_p (0)
{

    construct (vi, type);
}


VisBuffer2::~VisBuffer2()
{
    delete vb_p;
}

void
VisBuffer2::construct (ROVisibilityIterator2 * vi, Type type)
{
    // Create the proper underlying implementation
    // (until async gets added just do a simple one).

    switch (type){

    case Plain:

        vb_p = new VisBufferImpl2 (vi);
        break;

    case Asynchronous:
        Assert (False);
        break;

    default:

        ThrowIf (True, utilj::format ("Unknown VisBuffer type id (%d)", type));
        break;

    }
}

const ROVisibilityIterator2 *
VisBuffer2::getVi () const
{
    return vb_p->getVi  ();
}

void
VisBuffer2::invalidate()
{
    vb_p->invalidate ();
}

void
VisBuffer2::dirtyComponentsAdd (const VbDirtyComponents & additionalDirtyComponents)
{
    vb_p->dirtyComponentsAdd  (additionalDirtyComponents);
}

void
VisBuffer2::dirtyComponentsAdd (VisBufferComponents::EnumType component)
{
    vb_p->dirtyComponentsAdd  (component);
}

void
VisBuffer2::dirtyComponentsClear ()
{
    vb_p->dirtyComponentsClear  ();
}

VbDirtyComponents
VisBuffer2::dirtyComponentsGet () const
{
    return vb_p->dirtyComponentsGet  ();
}

void
VisBuffer2::dirtyComponentsSet (const VbDirtyComponents & dirtyComponents)
{
    vb_p->dirtyComponentsSet  (dirtyComponents);
}

void
VisBuffer2::dirtyComponentsSet (VisBufferComponents::EnumType component)
{
    vb_p->dirtyComponentsSet  (component);
}

Vector<Float>
VisBuffer2::feed_pa(Double time) const
{
    return vb_p->feed_pa (time);
}

Float
VisBuffer2::parang0(Double time) const
{
    return vb_p->parang0 (time);
}

Vector<Float>
VisBuffer2::parang(Double time) const
{
    return vb_p->parang (time);
}

MDirection
VisBuffer2::azel0(Double time) const
{
    return vb_p->azel0 (time);
}

Vector<MDirection>
VisBuffer2::azel(Double time) const
{
    return vb_p->azel (time);
}

Double
VisBuffer2::hourang(Double time) const
{
    return vb_p->hourang (time);
}

void
VisBuffer2::sortCorr ()
{
    vb_p->sortCorr  ();
}

void
VisBuffer2::unSortCorr()
{
    vb_p->unSortCorr ();
}

void
VisBuffer2::normalize(Bool phaseOnly)
{
    vb_p->normalize (phaseOnly);
}

void
VisBuffer2::resetWeightsUsingSigma ()
{
    vb_p->resetWeightsUsingSigma  ();
}

void
VisBuffer2::copyCoordinateInfo(const VisBuffer2 & other, Bool includeDirections)
{
    vb_p->copyCoordinateInfo (other.vb_p, includeDirections);
}

Bool
VisBuffer2::isNewArrayId () const
{
    return vb_p->isNewArrayId  ();
}

Bool
VisBuffer2::isNewFieldId () const
{
    return vb_p->isNewFieldId  ();
}

Bool
VisBuffer2::isNewMs() const
{
    return vb_p->isNewMs ();
}

Bool
VisBuffer2::isNewSpectralWindow () const
{
    return vb_p->isNewSpectralWindow  ();
}

Bool
VisBuffer2::isWritable () const
{
    return vb_p->isWritable  ();
}

Int
VisBuffer2::msId() const
{
    return vb_p->msId ();
}

String
VisBuffer2::msName (Bool stripPath) const
{
    return vb_p->msName  (stripPath);
}

//  +-------------+
//  |             |
//  |  Accessors  |
//  |             |
//  +-------------+

const Vector<Int> &
VisBuffer2::antenna1 () const
{
    return vb_p->antenna1();
}

const Vector<Int> &
VisBuffer2::antenna2 () const
{
    return vb_p->antenna2();
}

Int
VisBuffer2::arrayId (Int row) const
{
    return vb_p->arrayId();
}

const Vector<SquareMatrix<Complex, 2> > &
VisBuffer2::cjones () const
{
    return vb_p->cjones();
}

const Vector<Int> &
VisBuffer2::correlationType (Int row) const
{
    return vb_p->corrType();
}

Int
VisBuffer2::dataDescriptionId (Int row) const
{
    return vb_p->dataDescriptionId();
}

const Vector<MDirection> &
VisBuffer2::direction1 () const
{
    return vb_p->direction1();
}

const Vector<MDirection> &
VisBuffer2::direction2 () const
{
    return vb_p->direction2();
}

const Vector<Double> &
VisBuffer2::exposure () const
{
    return vb_p->exposure();
}

const Vector<Int> &
VisBuffer2::feed1 () const
{
    return vb_p->feed1();
}

const Vector<Float> &
VisBuffer2::feed1_pa () const
{
    return vb_p->feed1_pa();
}

const Vector<Int> &
VisBuffer2::feed2 () const
{
    return vb_p->feed2();
}

const Vector<Float> &
VisBuffer2::feed2_pa () const
{
    return vb_p->feed2_pa();
}

Int
VisBuffer2::fieldId (Int row) const
{
    return vb_p->fieldId();
}

const Matrix<Bool> &
VisBuffer2::flag () const
{
    return vb_p->flag();
}

void
VisBuffer2::setFlag (const Matrix<Bool>& flags)
{
    vb_p->setFlag(flags);
}

const Array<Bool> &
VisBuffer2::flagCategory () const
{
    return vb_p->flagCategory();
}

void
VisBuffer2::setFlagCategory (const Array<Bool>& flagCategories)
{
    vb_p->setFlagCategory(flagCategories);
}

const Cube<Bool> &
VisBuffer2::flagCube () const
{
    return vb_p->flagCube();
}

void
VisBuffer2::setFlagCube (const Cube<Bool>& flagCube)
{
    vb_p->setFlagCube(flagCube);
}

const Vector<Bool> &
VisBuffer2::flagRow () const
{
    return vb_p->flagRow();
}

void
VisBuffer2::setFlagRow (const Vector<Bool>& flagRow)
{
    vb_p->setFlagRow(flagRow);
}


const Matrix<Float> &
VisBuffer2::imagingWeight () const
{
    return vb_p->imagingWeight();
}


Int
VisBuffer2::nCorrelations (Int row) const
{
    return vb_p->nCorr();
}

Int
VisBuffer2::nRows () const
{
    return vb_p->nRow();
}

const Vector<Int> &
VisBuffer2::observationId () const
{
    return vb_p->observationId();
}

const MDirection&
VisBuffer2::phaseCenter (Int row) const
{
    return vb_p->phaseCenter();
}

Int
VisBuffer2::polarizationFrame (Int row) const
{
    return vb_p->polFrame();
}

const Vector<Int> &
VisBuffer2::processorId () const
{
    return vb_p->processorId();
}

const Vector<uInt> &
VisBuffer2::rowIds () const
{
    return vb_p->rowIds();
}

const Vector<Int> &
VisBuffer2::scan () const
{
    return vb_p->scan();
}

const Vector<Float> &
VisBuffer2::sigma () const
{
    return vb_p->sigma();
}

const Matrix<Float> &
VisBuffer2::sigmaMat () const
{
    return vb_p->sigmaMat();
}

Int
VisBuffer2::spectralWindow (Int row) const
{
    return vb_p->spectralWindow();
}

const Vector<Int> &
VisBuffer2::stateId () const
{
    return vb_p->stateId();
}

const Vector<Double> &
VisBuffer2::time () const
{
    return vb_p->time();
}

const Vector<Double> &
VisBuffer2::timeCentroid () const
{
    return vb_p->timeCentroid();
}

const Vector<Double> &
VisBuffer2::timeInterval () const
{
    return vb_p->timeInterval();
}

const Vector<RigidVector<Double, 3> > &
VisBuffer2::uvw () const
{
    return vb_p->uvw();
}

const Matrix<Double> &
VisBuffer2::uvwMat () const
{
    return vb_p->uvwMat();
}


const Cube<Complex> &
VisBuffer2::visCube () const
{
    return vb_p->visCube();
}

void
VisBuffer2::setVisCube(const Complex & c)
{
    vb_p->setVisCube(c);
}

void
VisBuffer2::setVisCube (const Cube<Complex> & visCube)
{
    vb_p->setVisCube(visCube);
}

const Matrix<CStokesVector> &
VisBuffer2::vis() const
{
    return vb_p->visibility();
}

void
VisBuffer2::setVis (Matrix<CStokesVector> & stokesVector)
{
    vb_p->setVisibility(stokesVector);
}

const Cube<Float> &
VisBuffer2::visCubeFloat () const
{
    return vb_p->floatDataCube();
}

void
VisBuffer2::setVisCubeFloat (const Cube<Float> & floatDataCube)
{
    vb_p->setFloatDataCube(floatDataCube);
}


const Cube<Complex> &
VisBuffer2::visCubeCorrected () const
{
    return vb_p->correctedVisCube();
}

void
VisBuffer2::setVisCubeCorrected (const Cube<Complex> & correctedVisCube)
{
    vb_p->setCorrectedVisCube(correctedVisCube);
}

const Matrix<CStokesVector> &
VisBuffer2::visCorrected () const
{
    return vb_p->correctedVisibility();
}

void
VisBuffer2::setVisCorrected (const Matrix<CStokesVector> & correctedVisibility)
{
    vb_p->setCorrectedVisibility(correctedVisibility);
}

const Cube<Complex> &
VisBuffer2::visCubeModel () const
{
    return vb_p->modelVisCube();
}

void
VisBuffer2::setVisCubeModel(const Complex & c)
{
    vb_p->setModelVisCube(c);
}

void
VisBuffer2::setVisCubeModel(const Cube<Complex>& vis)
{
    vb_p->setModelVisCube(vis);
}

void
VisBuffer2::setVisCubeModel(const Vector<Float>& stokes)
{
    vb_p->setModelVisCube(stokes);
}

const Matrix<CStokesVector> &
VisBuffer2::visModel () const
{
    return vb_p->modelVisibility();
}

void
VisBuffer2::setVisModel (Matrix<CStokesVector> & modelVisibility)
{
    vb_p->setModelVisibility(modelVisibility);
}

const Vector<Float> &
VisBuffer2::weight () const
{
    return vb_p->weight();
}

void
VisBuffer2::setWeight (const Vector<Float>& weight)
{
    vb_p->setWeight(weight);
}

const Matrix<Float> &
VisBuffer2::weightMat () const
{
    return vb_p->weightMat();
}

void
VisBuffer2::setWeightMat (const Matrix<Float>& weightMat)
{
    vb_p->setWeightMat(weightMat);
}

const Cube<Float> &
VisBuffer2::weightSpectrum () const
{
    return vb_p->weightSpectrum();
}


void
VisBuffer2::setWeightSpectrum (const Cube<Float> & weightSpectrum)
{
    vb_p->setWeightSpectrum(weightSpectrum);
}



} // end namespace casa
