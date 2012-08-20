/*
 * VisBufferImpl.cc
 *
 *  Created on: Jul 3, 2012
 *      Author: jjacobs
 */


//#include <casa/Arrays/ArrayLogical.h>
//#include <casa/Arrays/ArrayMath.h>
//#include <casa/Arrays/ArrayUtil.h>
//#include <casa/Arrays/MaskArrMath.h>
//#include <casa/Arrays/MaskedArray.h>
#include <casa/OS/Path.h>
#include <casa/OS/Timer.h>
#include <casa/Utilities/Assert.h>
#include <casa/Utilities/GenSort.h>
#include <casa/aipstype.h>
#include <components/ComponentModels/ComponentList.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <synthesis/MSVis/UtilJ.h>
#include <synthesis/MSVis/UtilJ.h>
#include <synthesis/MSVis/VisBufferAsyncWrapper2.h>
#include <synthesis/MSVis/VisBufferImpl2.h>
#include <synthesis/MSVis/VisibilityIterator2.h>
#include <synthesis/TransformMachines/FTMachine.h>


#define CheckVisIter() checkVisIter (__func__, __FILE__, __LINE__)
#define CheckVisIter1(s) checkVisIter (__func__, __FILE__, __LINE__,s)
#define CheckVisIterBase() checkVisIterBase (__func__, __FILE__, __LINE__)

namespace casa {

namespace vi {


VisBufferCache::VisBufferCache (VisBufferImpl2 * vb)
{

    ThrowIf (vb == NULL, "VisBufferCacheImpl not connected to VisBufferImpl2");

    // Initialize the cache items.  This will also register them
    // with the vb object to allow it to iterate over the cached
    // values.

    antenna1_p.initialize (vb, & VisBufferImpl2::fillAntenna1);
    antenna2_p.initialize (vb, &VisBufferImpl2::fillAntenna2);
    arrayId_p.initialize (vb, &VisBufferImpl2::fillArrayId);
    cjones_p.initialize (vb, &VisBufferImpl2::fillJonesC);
    correctedVisCube_p.initialize (vb, &VisBufferImpl2::fillCubeCorrected);
    correctedVisibility_p.initialize (vb, &VisBufferImpl2::fillVisibilityCorrected);
    corrType_p.initialize (vb, &VisBufferImpl2::fillCorrType);
    dataDescriptionId_p.initialize (vb, &VisBufferImpl2::fillDataDescriptionId);
    direction1_p.initialize (vb, &VisBufferImpl2::fillDirection1);
    direction2_p.initialize (vb, &VisBufferImpl2::fillDirection2);
    exposure_p.initialize (vb, &VisBufferImpl2::fillExposure);
    feed1_p.initialize (vb, &VisBufferImpl2::fillFeed1);
    feed1Pa_p.initialize (vb, &VisBufferImpl2::fillFeedPa1);
    feed2_p.initialize (vb, &VisBufferImpl2::fillFeed2);
    feed2Pa_p.initialize (vb, &VisBufferImpl2::fillFeedPa2);
    fieldId_p.initialize (vb, &VisBufferImpl2::fillFieldId);
    flag_p.initialize (vb, &VisBufferImpl2::fillFlag);
    flagCategory_p.initialize (vb, &VisBufferImpl2::fillFlagCategory);
    flagCube_p.initialize (vb, &VisBufferImpl2::fillFlagCube);
    flagRow_p.initialize (vb, &VisBufferImpl2::fillFlagRow);
    floatDataCube_p.initialize (vb, &VisBufferImpl2::fillFloatData);
    frequency_p.initialize (vb, &VisBufferImpl2::fillFrequency);
    imagingWeight_p.initialize (vb, &VisBufferImpl2::fillImagingWeight);
    modelVisCube_p.initialize (vb, &VisBufferImpl2::fillCubeModel);
    modelVisibility_p.initialize (vb, &VisBufferImpl2::fillVisibilityModel);
    nChannel_p.initialize (vb, &VisBufferImpl2::fillNChannel);
    nCorr_p.initialize (vb, &VisBufferImpl2::fillNCorr);
    ////nCat_p.initialize (vb, &VisBufferImpl2::fillNCat);
    nRow_p.initialize (vb, &VisBufferImpl2::fillNRow);
    observationId_p.initialize (vb, &VisBufferImpl2::fillObservationId);
    phaseCenter_p.initialize (vb, &VisBufferImpl2::fillPhaseCenter);
    polFrame_p.initialize (vb, &VisBufferImpl2::fillPolFrame);
    processorId_p.initialize (vb, &VisBufferImpl2::fillProcessorId);
    ////rowIds_p.initialize (vb, &VisBufferImpl2::fillRowIds);
    scan_p.initialize (vb, &VisBufferImpl2::fillScan);
    sigma_p.initialize (vb, &VisBufferImpl2::fillSigma);
    sigmaMat_p.initialize (vb, &VisBufferImpl2::fillSigmaMat);
    spectralWindow_p.initialize (vb, &VisBufferImpl2::fillSpectralWindow);
    stateId_p.initialize (vb, &VisBufferImpl2::fillStateId);
    time_p.initialize (vb, &VisBufferImpl2::fillTime);
    timeCentroid_p.initialize (vb, &VisBufferImpl2::fillTimeCentroid);
    timeInterval_p.initialize (vb, &VisBufferImpl2::fillTimeInterval);
    uvw_p.initialize (vb, &VisBufferImpl2::fillUvw);
    uvwMat_p.initialize (vb, &VisBufferImpl2::fillUvwMat);
    visCube_p.initialize (vb, &VisBufferImpl2::fillCubeObserved);
    visibility_p.initialize (vb, &VisBufferImpl2::fillVisibilityObserved);
    weight_p.initialize (vb, &VisBufferImpl2::fillWeight);
    weightMat_p.initialize (vb, &VisBufferImpl2::fillWeightMat);
    weightSpectrum_p.initialize (vb, &VisBufferImpl2::fillWeightSpectrum);
}

void
VbCacheItemBase::initialize (VisBufferImpl2 * vb)
{
    vb_p = vb;
    vb_p->registerCacheItem (this);
}


using namespace vi;

////////////////////////////////////////////////////////////
//
// Basic VisBufferImpl2 Methods
// ========================
//
// Other sections contain the accessor and filler methods

VisBufferImpl2::VisBufferImpl2 ()
: cache_p (0), state_p (0)
{
    construct (0);
}

VisBufferImpl2::VisBufferImpl2(ROVisibilityIterator2 * iter)
: cache_p (0),
  state_p (0)
{
  construct ( iter);
}

VisBufferImpl2::~VisBufferImpl2 ()
{
    delete cache_p;
    delete state_p;
}

Bool
VisBufferImpl2::areCorrelationsInCanonicalOrder () const
{
  Vector<Int> corrs(corrType());

  // Only a meaningful question is all 4 corrs present

  Bool result = corrs.nelements () == 4 &&
                (corrs (1) == Stokes::LL || corrs (1) == Stokes::YY);

  return result;
}

Bool
VisBufferImpl2::areCorrelationsSorted() const
{
    return state_p->areCorrelationsSorted_p = False;
}

void
VisBufferImpl2::associateWithVisibilityIterator2 (const ROVisibilityIterator2 & vi)
{
    ThrowIf (state_p->isAttached_p, "VisBuffer is attached to VisibilityIterator2");

    state_p->isAttached_p = False;
    state_p->viC_p = & vi;
    state_p->vi_p = 0;
}

void
VisBufferImpl2::checkVisIter (const char * func, const char * file, int line, const char * extra) const
{
  checkVisIterBase (func, file, line, extra);
}

void
VisBufferImpl2::checkVisIterBase (const char * func, const char * file, int line, const char * extra) const
{
  if (! state_p->isAttached_p) {
    throw AipsError (String ("VisBuffer not attached to a VisibilityIterator2 while filling this field in (") +
                     func + extra + ")", file, line);
  }

  if (getViP() == NULL){
    throw AipsError (String ("VisBuffer's VisibilityIterator2 is NULL while filling this field in (") +
                     func + extra + ")", file, line);
  }
}

VisBufferImpl2 *
VisBufferImpl2::clone () const
{
    return new VisBufferImpl2 (* this);
}

void
VisBufferImpl2::cacheCopy (const VisBufferImpl2 & other, Bool markAsCached)
{
    CacheRegistry::iterator dst;
    CacheRegistry::const_iterator src;

    for (dst = state_p->cacheRegistry_p.begin(), src = other.state_p->cacheRegistry_p.begin();
         dst != state_p->cacheRegistry_p.end();
         dst ++, src ++)
    {
        (*dst)->copy (* src, markAsCached);
    }
}

void
VisBufferImpl2::cacheClear (Bool markAsCached)
{
    for (CacheRegistry::iterator i = state_p->cacheRegistry_p.begin();
         i != state_p->cacheRegistry_p.end();
         i++){

        (*i)->clear ();

        if (markAsCached){
            (*i)->setAsPresent ();
        }
    }
}

void
VisBufferImpl2::construct (ROVisibilityIterator2 * vi)
{
    state_p = new VisBufferState ();

    // Initialize all non-object member variables

    state_p->areCorrelationsSorted_p = False; // Have correlations been sorted by sortCorr?
    state_p->isAttached_p = vi != 0;
    state_p->pointingTableLastRow_p = -1;
    state_p->newMs_p = True;
    state_p->vi_p = vi;
    state_p->viC_p = vi;

    cache_p = new VisBufferCache (this);
}

void
VisBufferImpl2::copyCoordinateInfo (const VisBufferBase2 * vb, const  Bool dirDependent)
{
    cache_p->antenna1_p.set (vb->antenna1 ());
    cache_p->antenna2_p.set (vb->antenna2 ());
    cache_p->arrayId_p.set (vb->arrayId ());
    cache_p->dataDescriptionId_p.set (vb->dataDescriptionId ());
    cache_p->fieldId_p.set (vb->fieldId ());
    cache_p->spectralWindow_p.set (vb->spectralWindow ());
    cache_p->time_p.set (vb->time ());
    cache_p->frequency_p.set (vb->frequency ());
    cache_p->nRow_p.set (vb->nRow ());

    setIterationInfo (vb->msId(), vb->msName (), vb->isNewMs (),
                      vb->isNewArrayId (), vb->isNewFieldId (), vb->isNewSpectralWindow ());

    cache_p->feed1_p.set (vb->feed1 ());
    cache_p->feed2_p.set (vb->feed2 ());

    if(dirDependent){
        cache_p->feed1Pa_p.set (vb->feed1_pa ());
        cache_p->feed2Pa_p.set (vb->feed2_pa ());
        cache_p->direction1_p.set (vb->direction1 ());
        cache_p->direction2_p.set (vb->direction2 ());
    }
}


void
VisBufferImpl2::detachFromVisibilityIterator2 ()
{
    ThrowIf (! state_p->isAttached_p,
             "Cannot detach VisBuffer from VI since it is not attached.");

#warning "Uncomment next line after cutover"
    //// getViP()->detachVisBuffer (* this);

    state_p->isAttached_p = False;
    state_p->vi_p = 0;
}


void
VisBufferImpl2::dirtyComponentsAdd (const VbDirtyComponents & dirtyComponents)
{
    state_p->dirtyComponents_p = state_p->dirtyComponents_p + dirtyComponents;
}

void
VisBufferImpl2::dirtyComponentsAdd (VisBufferComponents::EnumType component)
{
    state_p->dirtyComponents_p = state_p->dirtyComponents_p + VbDirtyComponents::singleton (component);
}


void
VisBufferImpl2::dirtyComponentsClear ()
{
    state_p->dirtyComponents_p = VbDirtyComponents::none();
}

VbDirtyComponents
VisBufferImpl2::dirtyComponentsGet () const
{
    return state_p->dirtyComponents_p;
}

void
VisBufferImpl2::dirtyComponentsSet (const VbDirtyComponents & dirtyComponents)
{
    state_p->dirtyComponents_p = dirtyComponents;
}

void
VisBufferImpl2::dirtyComponentsSet (VisBufferComponents::EnumType component)
{
    state_p->dirtyComponents_p = VbDirtyComponents::singleton (component);
}

Int
VisBufferImpl2::msId () const
{
    return state_p->msId_p;
}

String
VisBufferImpl2::msName (Bool stripPath) const
{
    String result;

    if(stripPath){

      Path path(state_p->msName_p);
      result = path.baseName();
    }
    else{
        result = state_p->msName_p;
    }

    return result;
}

const ROVisibilityIterator2 *
VisBufferImpl2::getVi () const
{
    return state_p->viC_p;
}

ROVisibilityIterator2 *
VisBufferImpl2::getViP () const
{
    return state_p->vi_p;
}

VisModelData
VisBufferImpl2::getVisModelData () const
{
    return state_p->visModelData_p;
}

void
VisBufferImpl2::invalidate ()
{
    cacheClear (False); // empty cached values
}

Bool
VisBufferImpl2::isNewArrayId () const
{
    return state_p->isNewArrayId_p;
}

Bool
VisBufferImpl2::isNewFieldId () const
{
    return state_p->isNewFieldId_p;
}

Bool
VisBufferImpl2::isNewMs() const
{
    return state_p->isNewMs_p;
}

Bool
VisBufferImpl2::isNewSpectralWindow () const
{
    return state_p->isNewSpectralWindow_p;
}

Bool
VisBufferImpl2::isWritable () const
{
    return state_p->isWritable_p;
}

void
VisBufferImpl2::normalize(const Bool & /* phaseOnly */)
{
#warning "*** Carefully go over recode of this method (VisBufferImpl2::normalize)"

    // NB: phase-only now handled by SolvableVisCal
    //   (we will remove phaseOnly parameter later)
    // NB: Handles pol-dep weights in chan-indep way
    // TBD: Handle channel-dep weights?

    Bool dataMissing = ! cache_p->visCube_p.isPresent() ||
                       ! cache_p->modelVisCube_p.isPresent () ||
                       ! cache_p->weightMat_p.isPresent();

    ThrowIf (dataMissing, "Failed to normalize data by model!");

    Int nCor = nCorr();
    Float amp(1.0);
    Vector<Float> ampCorr(nCor);
    Vector<Int> count (nCor);

    const Vector<Bool> & rowFlagged = cache_p->flagRow_p.get ();
    const Matrix<Bool> & flagged = cache_p->flag_p.get ();

    Cube<Complex> & visCube = cache_p->visCube_p.getRef();
    Cube<Complex> & modelCube = cache_p->modelVisCube_p.getRef();
    Matrix<Float> & weightMat = cache_p->weightMat_p.getRef();

    for (Int row = 0; row < nRow(); row++) {

        if (rowFlagged (row)){
            weightMat.column(row) = 0.0f; // Zero weight on this flagged row
            continue;
        }

        ampCorr = 0.0f;
        count = 0;

        for (Int channel = 0; channel < nChannel(); channel ++) {

            if (flagged (channel, row)){
                continue;  // data is flagged so skip over it
            }

            for (Int correlation = 0; correlation < nCor; correlation ++) {

                amp = abs(visCube(correlation, channel, row));
                if (amp > 0.0f) {

                    // Normalize visibility datum by corresponding model data point.


                    DComplex vis = visCube(correlation, channel, row);
                    DComplex mod = modelCube(correlation, channel, row);

                    visCube (correlation, channel, row) = Complex (vis / mod);
                    modelCube (correlation, channel, row) = Complex(1.0);

                    ampCorr(correlation) += amp;
                    count (correlation)++;
                }
                else { // zero data if model is zero
                    visCube (correlation, channel, row) = 0.0;
                }
            }
        }

        for (Int correlation = 0; correlation < nCor; correlation++) {
            if (count (correlation) > 0) {
                weightMat(correlation, row) *= square (ampCorr(correlation) / count (correlation));
            }
            else {
                weightMat(correlation, row) = 0.0f;
            }
        }
    }
}

void
VisBufferImpl2::registerCacheItem (VbCacheItemBase * item)
{
    state_p->cacheRegistry_p.push_back (item);
}

void
VisBufferImpl2::resetWeightsUsingSigma ()
{
    const Matrix <Float> & sigma = sigmaMat ();

    IPosition ip (sigma.shape());

    Matrix <Float> & weight = cache_p->weightMat_p.getRef ();
    weight.resize(ip);

    Int nPol(ip(0));
    Int nRow(ip(1));

    // Weight is inverse square of sigma (or zero[?])

    Float * w = weight.data();
    const Float * s = sigma.data();

    for (Int row = 0; row < nRow; ++row){
        for (Int pol = 0; pol < nPol; ++pol, ++w, ++s){
            if (*s > 0.0f) {
                *w = 1.0f / square(*s);
            } else {
                *w = 0.0f;
            }
        }
    }

    // Scale by (unselected!) # of channels (to stay aligned with original nominal weights)

    Int nchan = getViP()->msColumns().spectralWindow().numChan()(spectralWindow());

    weight *= Float(nchan);

    cache_p->weightMat_p.setAsPresent ();
}

void
VisBufferImpl2::setIterationInfo (Int msId, const String & msName, Bool isNewMs,
                              Bool isNewArrayId, Bool isNewFieldId,
                              Bool isNewSpectralWindow)
{
    state_p->msId_p = msId;
    state_p->msName_p = msName;
    state_p->isNewMs_p = isNewMs;
    state_p->isNewMs_p = isNewMs;
    state_p->isNewArrayId_p = isNewArrayId;
    state_p->isNewFieldId_p = isNewFieldId;
    state_p->isNewSpectralWindow_p = isNewSpectralWindow;
}


// Sort correlations: (PP,QQ,PQ,QP) -> (PP,PQ,QP,QQ)
void
VisBufferImpl2::sortCorrelationsAux (bool makeSorted)
{

    // This method is for temporarily sorting the correlations
    //  into canonical order if the MS data is out-of-order
    // NB: Always sorts the weightMat()
    // NB: Only works on the visCube-style data
    // NB: It only sorts the data columns which are already present
    //     (so make sure the ones you need are already read!)
    // NB: It is the user's responsibility to run unSortCorr
    //     after using the sorted data to put it back in order
    // NB: corrType_p is NOT changed to match the sorted
    //     correlations (it is expected that this sort is
    //     temporary, and that we will run unSortCorr
    // NB: This method does nothing if no sort required

    // If nominal order is non-canonical (only for nCorr=4)
    //   and data not yet sorted

    if (! areCorrelationsInCanonicalOrder() && ! state_p->areCorrelationsSorted_p) {

        // First sort the weights

        weightMat();    // (ensures it is filled)

        Vector<Float> wtmp(nRow());
        Vector<Float> w1, w2, w3;
        IPosition wblc(1, 0, 0), wtrc(3, 0, nRow() - 1), vec(1, nRow());

        VisBufferCache::sortCorrelationItem (cache_p->weightMat_p, wblc, wtrc, vec, wtmp, makeSorted);

        // Now sort the complex data cubes

        Matrix<Complex> tmp(nChannel(), nRow());
        Matrix<Complex> p1, p2, p3;
        IPosition blc(3, 0, 0, 0);
        IPosition trc(3, 0, nChannel() - 1, nRow() - 1);
        IPosition mat(2, nChannel(), nRow());

        // Sort the various visCubes, if present

        VisBufferCache::sortCorrelationItem (cache_p->visCube_p, blc, trc, mat, tmp, makeSorted);

        VisBufferCache::sortCorrelationItem (cache_p->modelVisCube_p, blc, trc, mat, tmp, makeSorted);

        VisBufferCache::sortCorrelationItem (cache_p->correctedVisCube_p, blc, trc, mat, tmp, makeSorted);

        // Finally sort the float data cube

        Matrix<Float> tmp2 (nChannel(), nRow());

        VisBufferCache::sortCorrelationItem (cache_p->floatDataCube_p, blc, trc, mat, tmp2, makeSorted);

        // Record the sort state

        state_p->areCorrelationsSorted_p = makeSorted;
    }

}

// Sort correlations: (PP,QQ,PQ,QP) -> (PP,PQ,QP,QQ)
void
VisBufferImpl2::sortCorr()
{
    sortCorrelationsAux (False);
}


void
VisBufferImpl2::stateCopy (const VisBufferImpl2 & other)
{
    // Copy state from the other buffer

    state_p->areCorrelationsSorted_p = other.areCorrelationsSorted ();
    state_p->dirtyComponents_p = other.dirtyComponentsGet ();
    state_p->isAttached_p = False;  // attachment isn't copyabled
    state_p->isNewArrayId_p = other.isNewArrayId ();
    state_p->isNewFieldId_p = other.isNewFieldId ();
    state_p->isNewMs_p = other.isNewMs ();
    state_p->isNewSpectralWindow_p = other.isNewSpectralWindow ();
    state_p->pointingTableLastRow_p = -1; // This will slow pointing table lookup
                                          // but probably not important in cases
                                          // where a vb is being copied (?).
    state_p->newMs_p = other.isNewMs ();
    state_p->viC_p = other.getVi ();
    state_p->vi_p = 0; // just to be safe
    state_p->visModelData_p = other.getVisModelData ();
}


// Unsort correlations: (PP,PQ,QP,QQ) -> (PP,QQ,PQ,QP)
void
VisBufferImpl2::unSortCorr()
{
    sortCorrelationsAux (False);
}



void
VisBufferImpl2::validate ()
{
    cacheClear (True); // empty values but mark as cached.
}



//      +---------------+
//      |               |
//      |  Calculators  |
//      |               |
//      +---------------+


MDirection
VisBufferImpl2::azel0(Double time) const
{
  return getViP()->azel0(time);
}

void
VisBufferImpl2::azel0Vec(Double time, Vector<Double>& azelVec) const
{
  MDirection azelMeas = azel0(time);

  azelVec.resize(2);

  azelVec = azelMeas.getAngle("deg").getValue();
}

Vector<MDirection>
VisBufferImpl2::azel(Double time) const
{
  return getViP()->azel(time);
}

void
VisBufferImpl2::azelMat(Double time, Matrix<Double>& azelMat) const
{
  Vector<MDirection> azelMeas = azel(time);

  azelMat.resize(2, azelMeas.nelements());

  for (uInt iant = 0; iant < azelMeas.nelements(); ++iant) {
    azelMat.column(iant) = (azelMeas(iant).getAngle("deg").getValue());
  }
}

Vector<Float>
VisBufferImpl2::feed_pa(Double time) const
{
  return getViP()->feed_pa(time);
}

Double
VisBufferImpl2::hourang(Double time) const
{
  return getViP()->hourang(time);
}

Float
VisBufferImpl2::parang0(Double time) const
{
  return getViP()->parang0(time);
}

Vector<Float>
VisBufferImpl2::parang(Double time) const
{
  return getViP()->parang(time);
}

//      +-------------+
//      |             |
//      |  Accessors  |
//      |             |
//      +-------------+

const Vector<Int> &
VisBufferImpl2::antenna1 () const
{
    return cache_p->antenna1_p.get ();
}

const Vector<Int> &
VisBufferImpl2::antenna2 () const
{
    return cache_p->antenna2_p.get ();
}

Int
VisBufferImpl2::arrayId () const
{
    return cache_p->arrayId_p.get ();
}

const Vector<SquareMatrix<Complex, 2> > &
VisBufferImpl2::cjones () const
{
    return cache_p->cjones_p.get ();
}

const Cube<Complex> &
VisBufferImpl2::correctedVisCube () const
{
    return cache_p->correctedVisCube_p.get ();
}

void
VisBufferImpl2::setCorrectedVisCube (const Cube<Complex> & value)
{
    cache_p->correctedVisCube_p.set (value);
}

const Matrix<CStokesVector> &
VisBufferImpl2::correctedVisibility () const
{
    return cache_p->correctedVisibility_p.get ();
}

void
VisBufferImpl2::setCorrectedVisibility (const Matrix<CStokesVector> & value)
{
    cache_p->correctedVisibility_p.set (value);
}

const Vector<Int> &
VisBufferImpl2::corrType () const
{
    return cache_p->corrType_p.get ();
}

Int
VisBufferImpl2::dataDescriptionId () const
{
    return cache_p->dataDescriptionId_p.get ();
}

const Vector<MDirection> &
VisBufferImpl2::direction1 () const
{
    return cache_p->direction1_p.get ();
}

const Vector<MDirection> &
VisBufferImpl2::direction2 () const
{
    return cache_p->direction2_p.get ();
}

const Vector<Double> &
VisBufferImpl2::exposure () const
{
    return cache_p->exposure_p.get ();
}

const Vector<Int> &
VisBufferImpl2::feed1 () const
{
    return cache_p->feed1_p.get ();
}

const Vector<Float> &
VisBufferImpl2::feed1_pa () const
{
    return cache_p->feed1Pa_p.get ();
}

const Vector<Int> &
VisBufferImpl2::feed2 () const
{
    return cache_p->feed2_p.get ();
}

const Vector<Float> &
VisBufferImpl2::feed2_pa () const
{
    return cache_p->feed2Pa_p.get ();
}

Int
VisBufferImpl2::fieldId () const
{
    return cache_p->fieldId_p.get ();
}

const Matrix<Bool> &
VisBufferImpl2::flag () const
{
    return cache_p->flag_p.get ();
}

void
VisBufferImpl2::setFlag (const Matrix<Bool>& value)
{
    cache_p->flag_p.set (value);
}

const Array<Bool> &
VisBufferImpl2::flagCategory () const
{
    return cache_p->flagCategory_p.get ();
}

void
VisBufferImpl2::setFlagCategory (const Array<Bool>& value)
{
    cache_p->flagCategory_p.set (value);
}

const Cube<Bool> &
VisBufferImpl2::flagCube () const
{
    return cache_p->flagCube_p.get ();
}

void
VisBufferImpl2::setFlagCube (const Cube<Bool>& value)
{
    cache_p->flagCube_p.set (value);
}

const Vector<Bool> &
VisBufferImpl2::flagRow () const
{
    return cache_p->flagRow_p.get ();
}

void
VisBufferImpl2::setFlagRow (const Vector<Bool>& value)
{
    cache_p->flagRow_p.set (value);
}

const Cube<Float> &
VisBufferImpl2::floatDataCube () const
{
    return cache_p->floatDataCube_p.get ();
}

void
VisBufferImpl2::setFloatDataCube (const Cube<Float> & value)
{
    cache_p->floatDataCube_p.set (value);
}

const Vector<Double> &
VisBufferImpl2::frequency () const
{
    return cache_p->frequency_p.get ();
}

const Matrix<Float> &
VisBufferImpl2::imagingWeight () const
{
    return cache_p->imagingWeight_p.get ();
}

//const Vector<Double> &
//VisBufferImpl2::lsrFrequency () const
//{
//    return cache_p->lsrFrequency_p.get ();
//}

const Cube<Complex> &
VisBufferImpl2::modelVisCube () const
{
    return cache_p->modelVisCube_p.get ();
}

void
VisBufferImpl2::setModelVisCube (const Complex & value)
{
    cache_p->modelVisCube_p.set (value);
}

void
VisBufferImpl2::setModelVisCube (const Cube<Complex> & value)
{
    cache_p->modelVisCube_p.set (value);
}

const Matrix<CStokesVector> &
VisBufferImpl2::modelVisibility () const
{
    return cache_p->modelVisibility_p.get ();
}

void
VisBufferImpl2::setModelVisibility (Matrix<CStokesVector> & value)
{
    cache_p->modelVisibility_p.set (value);
}

void
VisBufferImpl2::setModelVisCube(const Vector<Float>& stokesIn)
{

  enum {I, Q, U, V};

  Vector<Float> stokes (4, 0.0);

  stokes [I] = 1.0;  // Stokes parameters, nominally unpolarized, unit I

  for (uInt i = 0; i < stokesIn.nelements(); ++i){
      stokes [i] = stokesIn [i];
  }

  // Convert to correlations, according to basis

  Vector<Complex> stokesFinal (4, Complex(0.0)); // initially all zero

  if (polFrame() == MSIter::Circular){
    stokesFinal(0) = Complex(stokes [I] + stokes [V]);
    stokesFinal(1) = Complex(stokes [Q], stokes [U]);
    stokesFinal(2) = Complex(stokes [Q], -stokes [U]);
    stokesFinal(3) = Complex(stokes [I] - stokes [V]);
  }
  else if (polFrame() == MSIter::Linear) {
    stokesFinal(0) = Complex(stokes [I] + stokes [Q]);
    stokesFinal(1) = Complex(stokes [U], stokes [V]);
    stokesFinal(2) = Complex(stokes [U], -stokes [V]);
    stokesFinal(3) = Complex(stokes [I] - stokes [Q]);
  }
  else {
    throw(AipsError("Model-setting only works for CIRCULAR and LINEAR bases, for now."));
  }

  // A map onto the actual correlations in the VisBuffer

  Vector<Int> corrmap = corrType();
  corrmap -= corrmap(0);

  ThrowIf (max(corrmap) >= 4,  "HELP! The correlations in the data are not normal!");

  // Set the modelVisCube accordingly

  Cube<Complex> visCube (getViP()->visibilityShape(), 0.0);

  for (Int icorr = 0; icorr < nCorr(); ++icorr){
    if (abs(stokesFinal(corrmap(icorr))) > 0.0) {
      visCube (Slice (icorr, 1, 1), Slice(), Slice()).set(stokesFinal (corrmap (icorr)));
    }
  }

  cache_p->modelVisCube_p.set (visCube);

  // Lookup flux density calibrator scaling, and apply it per channel...
  //  TBD
}


Int
VisBufferImpl2::nChannel () const
{
    return cache_p->nChannel_p.get ();
}

Int
VisBufferImpl2::nCorr () const
{
    return cache_p->nCorr_p.get();
}

Int
VisBufferImpl2::nRow () const
{
    return cache_p->nRow_p.get ();
}

const Vector<Int> &
VisBufferImpl2::observationId () const
{
    return cache_p->observationId_p.get ();
}

const MDirection&
VisBufferImpl2::phaseCenter () const
{
    return cache_p->phaseCenter_p.get ();
}

Int
VisBufferImpl2::polFrame () const
{
    return cache_p->polFrame_p.get ();
}

const Vector<Int> &
VisBufferImpl2::processorId () const
{
    return cache_p->processorId_p.get ();
}

const Vector<uInt> &
VisBufferImpl2::rowIds () const
{
    return cache_p->rowIds_p.get ();
}

const Vector<Int> &
VisBufferImpl2::scan () const
{
    return cache_p->scan_p.get ();
}

const Vector<Float> &
VisBufferImpl2::sigma () const
{
    return cache_p->sigma_p.get ();
}

const Matrix<Float> &
VisBufferImpl2::sigmaMat () const
{
    return cache_p->sigmaMat_p.get ();
}

Int
VisBufferImpl2::spectralWindow () const
{
    return cache_p->spectralWindow_p.get ();
}

const Vector<Int> &
VisBufferImpl2::stateId () const
{
    return cache_p->stateId_p.get ();
}

const Vector<Double> &
VisBufferImpl2::time () const
{
    return cache_p->time_p.get ();
}

const Vector<Double> &
VisBufferImpl2::timeCentroid () const
{
    return cache_p->timeCentroid_p.get ();
}

const Vector<Double> &
VisBufferImpl2::timeInterval () const
{
    return cache_p->timeInterval_p.get ();
}

const Vector<RigidVector<Double, 3> > &
VisBufferImpl2::uvw () const
{
    return cache_p->uvw_p.get ();
}

const Matrix<Double> &
VisBufferImpl2::uvwMat () const
{
    return cache_p->uvwMat_p.get ();
}

const Cube<Complex> &
VisBufferImpl2::visCube () const
{
    return cache_p->visCube_p.get ();
}

void
VisBufferImpl2::setVisCube (const Complex & value)
{
    cache_p->visCube_p.set (value);
}

void
VisBufferImpl2::setVisCube (const Cube<Complex> & value)
{
    cache_p->visCube_p.set (value);
}

const Matrix<CStokesVector> &
VisBufferImpl2::visibility () const
{
    return cache_p->visibility_p.get ();
}

void
VisBufferImpl2::setVisibility (Matrix<CStokesVector> & value)
{
    cache_p->visibility_p.set (value);
}

const Vector<Float> &
VisBufferImpl2::weight () const
{
    return cache_p->weight_p.get ();
}

void
VisBufferImpl2::setWeight (const Vector<Float>& value)
{
    cache_p->weight_p.set (value);
}

const Matrix<Float> &
VisBufferImpl2::weightMat () const
{
    return cache_p->weightMat_p.get ();
}

void
VisBufferImpl2::setWeightMat (const Matrix<Float>& value)
{
    cache_p->weightMat_p.set (value);
}

const Cube<Float> &
VisBufferImpl2::weightSpectrum () const
{
    return cache_p->weightSpectrum_p.get ();
}

void
VisBufferImpl2::setWeightSpectrum (const Cube<Float>& value)
{
    cache_p->weightSpectrum_p.set (value);
}

//      +-----------+
//      |           |
//      |  Fillers  |
//      |           |
//      +-----------+


void
VisBufferImpl2::fillAntenna1 (Vector<Int>& value) const
{
  CheckVisIter ();

  getViP()->antenna1 (value);
}


void
VisBufferImpl2::fillAntenna2 (Vector<Int>& value) const
{
  CheckVisIter ();

  getViP()->antenna2 (value);
}

void
VisBufferImpl2::fillArrayId (Int& value) const
{
  CheckVisIter ();

  value = getViP()->arrayId ();
}

void
VisBufferImpl2::fillCorrType (Vector<Int>& value) const
{
  CheckVisIter ();

  getViP()->corrType (value);
}

void
VisBufferImpl2::fillCubeCorrected (Cube <Complex> & value) const
{
    CheckVisIter ();

    getViP()->visibility (value, VisibilityIterator2::Corrected);
}


void
VisBufferImpl2::fillCubeModel (Cube <Complex> & value) const
{
    CheckVisIter ();

    String modelkey=String("definedmodel_field_")+String::toString(fieldId());
    Bool hasmodkey=getViP()->ms().keywordSet().isDefined(modelkey);

    if (hasmodkey || !(getViP()->ms().tableDesc().isColumn("MODEL_DATA"))){

        //cerr << "HASMOD " << state_p->visModelData_p.hasModel(msId(), fieldId(), spectralWindow()) << endl;

        if (state_p->visModelData_p.hasModel (msId(), fieldId(), spectralWindow()) == -1){

            if(hasmodkey){

                String whichrec=getViP()->ms().keywordSet().asString(modelkey);
                Record modrec(getViP()->ms().keywordSet().asRecord(whichrec));
#warning "Uncomment later"
                ///// state_p->visModelData_p.addModel(modrec, Vector<Int>(1, msId()), *this);
            }
        }

#warning "Uncomment later"
        ///// state_p->visModelData_p.getModelVis(*this);
    }
    else{

        // Get the model data from the measurement set via the VI

        getViP()->visibility (value, VisibilityIterator2::Model);
    }
}

void
VisBufferImpl2::fillCubeObserved (Cube <Complex> & value) const
{
    CheckVisIter ();

    getViP()->visibility (value, VisibilityIterator2::Observed);
}

void
VisBufferImpl2::fillDataDescriptionId  (Int& value) const
{
  CheckVisIter ();

  value = getViP()->dataDescriptionId ();
}

void
VisBufferImpl2::fillDirection1 (Vector<MDirection>& value) const
{
  CheckVisIterBase ();
  // fill state_p->feed1_pa cache, antenna, feed and time will be filled automatically

  feed1_pa();

  fillDirectionAux (value, antenna1 (), feed1 (), feed1_pa ());

  value.resize(antenna1 ().nelements()); // could also use nRow()
}

void
VisBufferImpl2::fillDirection2 (Vector<MDirection>& value) const
{
  CheckVisIterBase ();
  // fill state_p->feed1_pa cache, antenna, feed and time will be filled automatically

  feed2_pa();

  fillDirectionAux (value, antenna2 (), feed2 (), feed2_pa ());

  value.resize(antenna2 ().nelements()); // could also use nRow()
}

void
VisBufferImpl2::fillDirectionAux (Vector<MDirection>& value,
                             const Vector<Int> & antenna,
                             const Vector<Int> &feed,
			     const Vector<Float> & feedPa) const
{
  value.resize (antenna.nelements()); // could also use nRow()

  const ROMSPointingColumns & mspc = getViP()->msColumns().pointing();
  state_p->pointingTableLastRow_p = mspc.pointingIndex (antenna (0),
                                            time()(0), state_p->pointingTableLastRow_p);
  if (getViP()->allBeamOffsetsZero() && state_p->pointingTableLastRow_p < 0) {

    // No true pointing information found; use phase center from the field table

    value.set(phaseCenter());
    state_p->pointingTableLastRow_p = 0;
    return;
  }

  for (uInt row = 0; row < antenna.nelements(); ++row) {

    DebugAssert(antenna (row) >= 0 && feed (row) >= 0, AipsError);

    Int pointIndex1 = mspc.pointingIndex(antenna (row), time()(row), state_p->pointingTableLastRow_p);

    if (pointIndex1 >= 0) {
      state_p->pointingTableLastRow_p = pointIndex1;
      value(row) = mspc.directionMeas(pointIndex1, timeInterval()(row));
    } else {
      value(row) = phaseCenter(); // nothing found, use phase center
    }

    if (!getViP()->allBeamOffsetsZero()) {

      RigidVector<Double, 2> beamOffset =
        getViP()->getBeamOffsets()(0, antenna (row), feed (row));

      if (downcase (getViP()->antennaMounts()(antenna (row))) == "alt-az") {

        SquareMatrix<Double, 2> xform(SquareMatrix<Double, 2>::General);
        // SquareMatrix' default constructor is a bit strange.
        // We will probably need to change it in the future
        Double cpa = cos(feedPa(row));
        Double spa = sin(feedPa(row));
        xform(0, 0) = cpa;
        xform(1, 1) = cpa;
        xform(0, 1) = -spa;
        xform(1, 0) = spa;
        beamOffset *= xform; // parallactic angle rotation
      }

      // x direction is flipped to convert az-el type frame to ra-dec
      value(row).shift(-beamOffset(0), beamOffset(1), True);
    }
  }
}

void
VisBufferImpl2::fillExposure (Vector<Double>& value) const
{
  CheckVisIter ();

  getViP()->exposure (value);
}

void
VisBufferImpl2::fillFeed1 (Vector<Int>& value) const
{
  CheckVisIter ();

  getViP()->feed1 (value);
}

void
VisBufferImpl2::fillFeed2 (Vector<Int>& value) const
{
  CheckVisIter ();

  getViP()->feed2 (value);
}

void
VisBufferImpl2::fillFeedPa1 (Vector <Float> & feedPa) const
{
  CheckVisIterBase ();

  // fill feed, antenna and time caches, if not filled before

  feed1 ();
  antenna1 ();
  time ();

  feedPa.resize(antenna1().nelements()); // could also use nRow()

  fillFeedPaAux (feedPa, antenna1 (), feed1 ());
}


void
VisBufferImpl2::fillFeedPa2 (Vector <Float> & feedPa) const
{
  CheckVisIterBase ();

  // fill feed, antenna and time caches, if not filled before
  feed2();
  antenna2();
  time();

  feedPa.resize(antenna2().nelements()); // could also use nRow()

  fillFeedPaAux (feedPa, antenna2(), feed2 ());
}

void
VisBufferImpl2::fillFeedPaAux (Vector <Float> & feedPa,
                              const Vector <Int> & antenna,
                              const Vector <Int> & feed) const
{
  for (uInt row = 0; row < feedPa.nelements(); ++row) {

    const Vector<Float>& antennaPointingAngle = feed_pa (time ()(row));
        // caching inside ROVisibilityIterator2, if the time doesn't change.
        // Otherwise we should probably fill both buffers for feed1 and feed2
        // simultaneously to speed up things.

    Assert(antenna (row) >= 0 && antenna (row) < (int) antennaPointingAngle.nelements());

    feedPa (row) = antennaPointingAngle (antenna (row));
      //
      // state_p->feed_pa returns only the first feed position angle
      // we need to add an offset if this row correspods to a
      // different feed

    if (feed (row)){  // Skip when feed(row) is zero

      float feedsAngle = getViP()->receptorAngles()(0, antenna (row), feed (row));
      float feed0Angle = getViP()->receptorAngles()(0, antenna (row), 0);

      feedPa (row) += feedsAngle - feed0Angle;
    }
  }
}

void
VisBufferImpl2::fillFieldId (Int& value) const
{
  CheckVisIter ();

  value = getViP()->fieldId ();
}

void
VisBufferImpl2::fillFlag (Matrix<Bool>& value) const
{
  CheckVisIter ();

  getViP()->flag (value);
}

void
VisBufferImpl2::fillFlagCategory (Array<Bool>& value) const
{
  CheckVisIter();

  getViP()->flagCategory (value);
}

void
VisBufferImpl2::fillFlagCube (Cube<Bool>& value) const
{
  CheckVisIter ();

  getViP()->flag (value);
}

void
VisBufferImpl2::fillFlagRow (Vector<Bool>& value) const
{
  CheckVisIter ();

  getViP()->flagRow (value);
}

void
VisBufferImpl2::fillFloatData (Cube<Float>& value) const
{
  CheckVisIter ();

  getViP()->floatData (value);
}

void
VisBufferImpl2::fillImagingWeight (Matrix<Float> & value) const
{
    const VisImagingWeight & weightGenerator = getViP()->getImagingWeightGenerator ();

    ThrowIf (weightGenerator.getType () == "none",
             "Bug check... imaging weights not set");

    value.resize (flag().shape ());

    if (weightGenerator.getType () == "uniform") {

        weightGenerator.weightUniform (value, flag (), uvwMat (), frequency(), weight (), msId (), fieldId ());

    } else if (weightGenerator.getType () == "radial") {

        weightGenerator.weightRadial (value, flag (), uvwMat (), frequency (), weight ());

    } else {

        weightGenerator.weightNatural (value, flag (), weight ());
    }

    if (weightGenerator.doFilter ()) {

        weightGenerator.filter (value, flag (), uvwMat (), frequency(), weight ());
    }
}


void
VisBufferImpl2::fillFrequency (Vector<Double>& value) const
{
  CheckVisIter ();

  getViP()->frequency (value);
}

void
VisBufferImpl2::fillJonesC (Vector<SquareMatrix<Complex, 2> >& value) const
{
  CheckVisIter ();

  getViP()->jonesC (value);
}


void
VisBufferImpl2::fillNChannel (Int& value) const
{
  CheckVisIter ();

  value = visCube().shape ()(1);
}

void
VisBufferImpl2::fillNCorr (Int& value) const
{
  CheckVisIter ();

  value  = corrType().nelements();
}

void
VisBufferImpl2::fillNRow (Int& value) const
{
  CheckVisIter ();

  value = getViP()->nRows ();
}

void
VisBufferImpl2::fillObservationId (Vector<Int>& value) const
{
  CheckVisIter();

  getViP()->observationId (value);
}

void
VisBufferImpl2::fillPhaseCenter (MDirection& value) const
{
  CheckVisIter ();

  value = getViP()->phaseCenter ();
}

void
VisBufferImpl2::fillPolFrame (Int& value) const
{
  CheckVisIter ();

  value = getViP()->polFrame ();
}

void
VisBufferImpl2::fillProcessorId (Vector<Int>& value) const
{
  CheckVisIter();

  getViP()->processorId (value);
}

void
VisBufferImpl2::fillScan (Vector<Int>& value) const
{
  CheckVisIter ();

  getViP()->scan (value);
}

void
VisBufferImpl2::fillSigma (Vector<Float>& value) const
{
  CheckVisIter ();

  getViP()->sigma (value);
}

void
VisBufferImpl2::fillSigmaMat (Matrix<Float>& value) const
{
  CheckVisIter ();

  getViP()->sigmaMat (value);
}

void
VisBufferImpl2::fillSpectralWindow (Int& value) const
{
  CheckVisIter ();

  value = getViP()->spectralWindow ();
}

void
VisBufferImpl2::fillStateId (Vector<Int>& value) const
{
  CheckVisIter();

  getViP()->stateId (value);
}


void
VisBufferImpl2::fillTime (Vector<Double>& value) const
{
  CheckVisIter ();

  getViP()->time (value);
}

void
VisBufferImpl2::fillTimeCentroid (Vector<Double>& value) const
{
  CheckVisIter ();

  getViP()->timeCentroid (value);
}

void
VisBufferImpl2::fillTimeInterval (Vector<Double>& value) const
{
  CheckVisIter ();

  getViP()->timeInterval (value);
}

void
VisBufferImpl2::fillUvw (Vector<RigidVector<Double, 3> >& value) const
{
  CheckVisIter ();

  getViP()->uvw (value);
}

void
VisBufferImpl2::fillUvwMat (Matrix<Double>& value) const
{
  CheckVisIter ();

  getViP()->uvwMat (value);
}

void
VisBufferImpl2::fillVisibilityCorrected (Matrix<CStokesVector>& value) const
{
    CheckVisIter ();

    getViP()->visibility (value, VisibilityIterator2::Corrected);
}

void
VisBufferImpl2::fillVisibilityModel (Matrix<CStokesVector>& value) const
{
    CheckVisIter ();

    getViP()->visibility (value, VisibilityIterator2::Model);
}

void
VisBufferImpl2::fillVisibilityObserved (Matrix<CStokesVector>& value) const
{
    CheckVisIter ();

    getViP()->visibility (value, VisibilityIterator2::Observed);
}


void
VisBufferImpl2::fillWeight (Vector<Float>& value) const
{
  CheckVisIter ();

  getViP()->weight (value);
}

void
VisBufferImpl2::fillWeightMat (Matrix<Float>& value) const
{
  CheckVisIter ();

  getViP()->weightMat (value);
}

void
VisBufferImpl2::fillWeightSpectrum (Cube<Float>& value) const
{
  CheckVisIter ();

  getViP()->weightSpectrum (value);
}

} // end namespace vb

} // end namespace casa
