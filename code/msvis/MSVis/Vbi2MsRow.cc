/*
 * Vbi2MsRow.cc
 *
 *  Created on: Aug 22, 2013
 *      Author: jjacobs
 */

#include <casa/Exceptions.h>
#include <measures/Measures/MDirection.h>
#include <msvis/MSVis/Vbi2MsRow.h>
#include <msvis/MSVis/VisibilityIterator2.h>
#include <synthesis/TransformMachines/VisModelData.h>
#include <msvis/MSVis/VisBufferImpl2.h>
#include <msvis/MSVis/VisBufferImpl2Internal.h>

#define AssertWritable() ThrowIf (! isWritable (), "Cannot write to this MsRow object")

using casa::vi::VisBufferImpl2;

namespace casa {

namespace ms {


Vbi2MsRow::Vbi2MsRow (Int row, const VisBufferImpl2 * vb)
: MsRow (row, vb),
  vbi2_p (const_cast<VisBufferImpl2 *> (vb))
{}


Vbi2MsRow::Vbi2MsRow (Int row, VisBufferImpl2 * vb)
: MsRow (row, vb),
  vbi2_p (vb)
{}

////////////////////////////////////////////////////////////
//
// Scalar getters
//

Int
Vbi2MsRow::antenna1 () const
{
    return vbi2_p->antenna1 () (row ());
}

Int
Vbi2MsRow::antenna2 () const
{
    return vbi2_p->antenna2 () (row ());
}

Int
Vbi2MsRow::arrayId () const
{
    return vbi2_p->arrayId () (row ());
}

Int
Vbi2MsRow::correlationType () const
{
    return vbi2_p->correlationTypes () (row ());
}


template <typename T, typename U>
void
Vbi2MsRow::copyIf (Bool copyThis, Vbi2MsRow * other,
                   void (Vbi2MsRow::* setter) (T),
                   U (Vbi2MsRow::* getter) () const)
{
    if (copyThis){
        (this ->* setter) ((other ->* getter) ());
    }
}

void
Vbi2MsRow::copy (Vbi2MsRow * other,
                 const VisBufferComponents2 & componentsToCopy)
{
    // Copy the data contents of the other row into this one

    setAntenna1 (other->antenna1 ());
    setAntenna2 (other->antenna2 ());
    setArrayId (other->arrayId ());
    setDataDescriptionId (other->dataDescriptionId ());
    setExposure (other->exposure ());
    setFeed1 (other->feed1 ());
    setFeed2 (other->feed2 ());
    setFieldId (other->fieldId ());
    setFlags (other->flags ());
    setInterval (other->interval ());
    setObservationId (other->observationId ());
    setProcessorId (other->processorId ());
    setRowFlag (other->isRowFlagged());
    setRowId (other->rowId());
    setScanNumber (other->scanNumber ());
    setSigma (other->sigma ());
    setSpectralWindow (other->spectralWindow());
    setStateId (other->stateId ());
    setTime (other->time ());
    setTimeCentroid (other->timeCentroid ());
    setUvw (other->uvw ());
    setWeight (other->weight ());

    // Optionally copied fields

    copyIf (componentsToCopy.contains (vi::VisibilityCubeCorrected), other,
            & Vbi2MsRow::setCorrected, & Vbi2MsRow::corrected );
    copyIf (componentsToCopy.contains (vi::VisibilityCubeModel), other,
            & Vbi2MsRow::setModel, & Vbi2MsRow::model );
    copyIf (componentsToCopy.contains (vi::VisibilityCubeObserved), other,
            & Vbi2MsRow::setObserved, & Vbi2MsRow::observed );
    copyIf (componentsToCopy.contains (vi::WeightSpectrum), other,
            & Vbi2MsRow::setWeightSpectrum, & Vbi2MsRow::weightSpectrum );
    copyIf (componentsToCopy.contains (vi::WeightSpectrumCorrected), other,
            & Vbi2MsRow::setWeightSpectrumCorrected, & Vbi2MsRow::weightSpectrumCorrected );
}


Int
Vbi2MsRow::dataDescriptionId () const
{
    return vbi2_p->dataDescriptionIds () (row ());
}

Int
Vbi2MsRow::feed1 () const
{
    return vbi2_p->feed1 () (row ());
}

Int
Vbi2MsRow::feed2 () const
{
    return vbi2_p->feed2 () (row ());
}

Int
Vbi2MsRow::fieldId () const
{
    return vbi2_p->fieldId () (row ());
}

Bool
Vbi2MsRow::isRowFlagged () const
{
    return vbi2_p->flagRow () (row ());
}

const Matrix<Bool>
Vbi2MsRow::flags () const
{
    return vbi2_p->flagCube ().xyPlane (row ());
}

Matrix<Bool>
Vbi2MsRow::flagsMutable ()
{
    return vbi2_p->flagCube ().xyPlane (row ());
}

Bool
Vbi2MsRow::isFlagged (Int correlation, Int channel) const
{
    return vbi2_p->flagCube () (correlation,channel, row ());
}


Int
Vbi2MsRow::observationId () const
{
    return vbi2_p->observationId () (row ());
}

Int
Vbi2MsRow::processorId () const
{
    return vbi2_p->processorId () (row ());
}

Int
Vbi2MsRow::scanNumber () const
{
    return vbi2_p->scan () (row ());
}

Int
Vbi2MsRow::stateId () const
{
    return vbi2_p->stateId () (row ());
}

Double
Vbi2MsRow::exposure () const
{
    return vbi2_p->exposure () (row ());
}

Double
Vbi2MsRow::interval () const
{
    return vbi2_p->timeInterval () (row ());
}

Int
Vbi2MsRow::rowId () const
{
    return vbi2_p->rowIds() (row());
}

Float
Vbi2MsRow::sigma (Int correlation) const
{
    return vbi2_p->sigma () (correlation, row ());
}

Int
Vbi2MsRow::spectralWindow () const
{
    return vbi2_p->spectralWindows() (row());
}


Double
Vbi2MsRow::time () const
{
    return vbi2_p->time () (row ());
}
Double
Vbi2MsRow::timeCentroid () const
{
    return vbi2_p->timeCentroid () (row ());
}

Float
Vbi2MsRow::weight (Int correlation) const
{
    return vbi2_p->weight () (correlation, row ());
}


////////////////////////////////////////////////////////////
//
// Scalar setters
//

void
Vbi2MsRow::setAntenna1 (Int value)
{
    AssertWritable();

    vbi2_p->cache_p->antenna1_p.getRef (False) (row ()) = value;
}

void
Vbi2MsRow::setAntenna2 (Int value)
{
    AssertWritable();

    vbi2_p->cache_p->antenna2_p.getRef (False)(row ()) = value;
}

void
Vbi2MsRow::setArrayId (Int value)
{
    AssertWritable();

    vbi2_p->cache_p->arrayId_p.getRef (False)(row ()) = value;
}

void
Vbi2MsRow::setCorrelationType (Int value)
{
    AssertWritable();

    vbi2_p->cache_p->corrType_p.getRef (False)(row ()) = value;
}



void
Vbi2MsRow::setDataDescriptionId (Int value)
{
    AssertWritable();

    vbi2_p->cache_p->dataDescriptionIds_p.getRef (False)(row ()) = value;
}

void
Vbi2MsRow::setFeed1 (Int value)
{
    AssertWritable();

    vbi2_p->cache_p->feed1_p.getRef (False)(row ()) = value;
}

void
Vbi2MsRow::setFeed2 (Int value)
{
    AssertWritable();

    vbi2_p->cache_p->feed2_p.getRef (False)(row ()) = value;
}

void
Vbi2MsRow::setFieldId (Int value)
{
    AssertWritable();

    vbi2_p->cache_p->fieldId_p.getRef (False)(row ()) = value;
}

void
Vbi2MsRow::setRowId (Int value)
{
    AssertWritable();

    vbi2_p->cache_p->rowIds_p.getRef (False) (row ()) = value;
}

void
Vbi2MsRow::setRowFlag (Bool isFlagged)
{
    AssertWritable ();

    vbi2_p->cache_p->flagRow_p.getRef (False)(row ()) = isFlagged;
}

void
Vbi2MsRow::setFlags (Bool isFlagged, Int correlation, Int channel)
{
    AssertWritable ();

    vbi2_p->cache_p->flagCube_p.getRef (False)(correlation,channel, row ()) = isFlagged;
}

void
Vbi2MsRow::setFlags (const Matrix<Bool> & flags)
{
    AssertWritable ();

    vbi2_p->cache_p->flagCube_p.getRef(False).xyPlane (row()) = flags;
}

void
Vbi2MsRow::setObservationId (Int value)
{
    AssertWritable();

    vbi2_p->cache_p->observationId_p.getRef (False)(row ()) = value;
}

void
Vbi2MsRow::setProcessorId (Int value)
{
    AssertWritable();

    vbi2_p->cache_p->processorId_p.getRef (False)(row ()) = value;
}

void
Vbi2MsRow::setScanNumber (Int value)
{
    AssertWritable();

    vbi2_p->cache_p->scan_p.getRef (False)(row ()) = value;
}

void
Vbi2MsRow::setSpectralWindow (Int value)
{
    vbi2_p->cache_p->spectralWindows_p.getRef (False)(row ()) = value;
}

void
Vbi2MsRow::setStateId (Int value)
{
    AssertWritable();

    vbi2_p->cache_p->stateId_p.getRef (False)(row ()) = value;
}

void
Vbi2MsRow::setExposure (Double value)
{
    AssertWritable();

    vbi2_p->cache_p->exposure_p.getRef (False)(row ()) = value;
}

void
Vbi2MsRow::setInterval (Double value)
{
    AssertWritable();

    vbi2_p->cache_p->timeInterval_p.getRef (False)(row ()) = value;
}

void
Vbi2MsRow::setSigma (Int correlation, Float value)
{
    AssertWritable();

    vbi2_p->cache_p->sigma_p.getRef (False)(correlation, row ()) = value;
}

void
Vbi2MsRow::setTime (Double value)
{
    AssertWritable();

    vbi2_p->cache_p->time_p.getRef (False)(row ()) = value;
}

void
Vbi2MsRow::setTimeCentroid (Double value)
{
    AssertWritable();

    vbi2_p->cache_p->timeCentroid_p.getRef (False)(row ()) = value;
}

void
Vbi2MsRow::setWeight (Int correlation, Float value)
{
    AssertWritable();

    vbi2_p->cache_p->weight_p.getRef (False)(correlation, row ()) = value;
}

////////////////////////////////////////////////////////////
//
// Nonscalar getters
//

const Complex &
Vbi2MsRow::corrected (Int correlation, Int channel) const
{
    return vbi2_p->visCubeCorrected () (correlation, channel, row ());
}


Float
Vbi2MsRow::weightSpectrum (Int correlation, Int channel) const
{
    return vbi2_p->weightSpectrum () (correlation, channel, row ());
}


Float
Vbi2MsRow::weightSpectrumCorrected (Int correlation, Int channel) const
{
    return vbi2_p->weightSpectrumCorrected () (correlation, channel, row ());
}


const Complex &
Vbi2MsRow::model (Int correlation, Int channel) const
{
    return vbi2_p->visCubeModel () (correlation, channel, row ());
}


const Complex &
Vbi2MsRow::observed (Int correlation, Int channel) const
{
    return vbi2_p->visCube () (correlation, channel, row ());
}

const Vector<Double>
Vbi2MsRow::uvw () const
{
    return vbi2_p->uvw ().column (row());
}

void
Vbi2MsRow::setUvw (const Vector<Double> & value)
{
    vbi2_p->cache_p->uvw_p.getRef(False).column (row()) = value;
}

const Matrix<Complex>
Vbi2MsRow::corrected () const
{
    return vbi2_p->visCubeCorrected().xyPlane (row());
}

Matrix<Complex>
Vbi2MsRow::correctedMutable ()
{
    return vbi2_p->visCubeCorrected().xyPlane (row());
}

void
Vbi2MsRow::setCorrected (const Matrix<Complex> & value)
{
    vbi2_p->cache_p->correctedVisCube_p.getRef(False).xyPlane (row()) = value;
}

const Matrix<Complex>
Vbi2MsRow::model () const
{
    return vbi2_p->visCubeModel ().xyPlane (row());
}

Matrix<Complex>
Vbi2MsRow::modelMutable ()
{
    return vbi2_p->visCubeModel ().xyPlane (row());
}

void
Vbi2MsRow::setModel (const Matrix<Complex> & value)
{
    vbi2_p->cache_p->modelVisCube_p.getRef(False).xyPlane (row()) = value;
}

const Matrix<Complex>
Vbi2MsRow::observed () const
{
    return vbi2_p->visCube ().xyPlane (row());
}

Matrix<Complex>
Vbi2MsRow::observedMutable ()
{
    return vbi2_p->visCube ().xyPlane (row());
}

void
Vbi2MsRow::setObserved (const Matrix<Complex> & value)
{
    vbi2_p->cache_p->visCube_p.getRef(False).xyPlane (row()) = value;
}

const Vector<Float>
Vbi2MsRow::sigma () const
{
    return vbi2_p->sigma ().column (row());
}

void
Vbi2MsRow::setSigma (const Vector<Float> & value)
{
    vbi2_p->cache_p->sigma_p.getRef(False).column (row()) = value;
}

const Vector<Float>
Vbi2MsRow::weight () const
{
    return vbi2_p->weight ().column (row());
}

void
Vbi2MsRow::setWeight (const Vector<Float> & value)
{
    vbi2_p->cache_p->weight_p.getRef(False).column (row()) = value;
}

void
Vbi2MsRow::setWeightSpectrum (const Matrix<Float> & value)
{
    vbi2_p->cache_p->weightSpectrum_p.getRef(False).xyPlane (row()) = value;
}

const Matrix<Float>
Vbi2MsRow::weightSpectrum () const
{
    return vbi2_p->weightSpectrum ().xyPlane (row());
}

const Matrix<Float>
Vbi2MsRow::weightSpectrumCorrected () const
{
    return vbi2_p->weightSpectrumCorrected ().xyPlane (row());
}

void
Vbi2MsRow::setWeightSpectrumCorrected (const Matrix<Float> & value)
{
    vbi2_p->cache_p->weightSpectrumCorrected_p.getRef(False).xyPlane (row()) = value;
}

////////////////////////////////////////////////////////////
//
// Nonscalar setters
//

void
Vbi2MsRow::setCorrected (Int correlation, Int channel, const Complex & value)
{
    AssertWritable();

    vbi2_p->cache_p->correctedVisCube_p.getRef (False)(correlation, channel, row ()) = value;
}

void
Vbi2MsRow::setWeightSpectrum (Int correlation, Int channel, Float value)
{
    AssertWritable();

    vbi2_p->cache_p->weightSpectrum_p.getRef (False)(correlation, channel, row ()) = value;
}


void
Vbi2MsRow::setWeightSpectrumCorrected (Int correlation, Int channel, Float value)
{
    AssertWritable();

    vbi2_p->cache_p->weightSpectrumCorrected_p.getRef (False)(correlation, channel, row ()) = value;
}


void
Vbi2MsRow::setModel (Int correlation, Int channel, const Complex & value)
{
    AssertWritable();

    vbi2_p->cache_p->modelVisCube_p.getRef (False)(correlation, channel, row ()) = value;
}

void
Vbi2MsRow::setObserved (Int correlation, Int channel, const Complex & value)
{
    AssertWritable();

    vbi2_p->cache_p->visCube_p.getRef (False)(correlation, channel, row ()) = value;
}



}
}


