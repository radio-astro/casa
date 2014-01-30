/*
 * VisBufferImpl.cc
 *
 *  Created on: Jul 3, 2012
 *      Author: jjacobs
 */


#include <casa/aipstype.h>
#include <msvis/MSVis/VisibilityIterator2.h>
#include <msvis/MSVis/VisBufferImpl.h>
#include <msvis/MSVis/VisBufferImpl.h>
#include <msvis/MSVis/VisBufferAsyncWrapper.h>
#include <msvis/MSVis/UtilJ.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/MaskedArray.h>
#include <casa/Arrays/MaskArrMath.h>
#include <casa/Arrays/ArrayUtil.h>
#include <casa/OS/Path.h>
#include <components/ComponentModels/ComponentList.h>
#include <casa/Utilities/Assert.h>
#include <casa/Utilities/GenSort.h>
#include <casa/OS/Timer.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <synthesis/TransformMachines/FTMachine.h>
#include <msvis/MSVis/UtilJ.h>


#define CheckVisIter() checkVisIter (__func__, __FILE__, __LINE__)
#define CheckVisIter1(s) checkVisIter (__func__, __FILE__, __LINE__,s)
#define CheckVisIterBase() checkVisIterBase (__func__, __FILE__, __LINE__)

namespace casa {

namespace vi {

//////////////////////////////////////////////////////////
//
// Auxiliary Classes are contained in the "vb" namespace.
//
// These include VbCacheItemBase, VbCacheItem, VisBufferCache
// and VisBufferState.

class VbCacheItemBase {

    // Provides a common base class for all of the cached value classes.
    // This is required because the actualy value classes use a template
    // to capture the underlying value type.

    friend class VisBufferImpl;

public:

    VbCacheItemBase () : vb_p (0) {}

    virtual ~VbCacheItemBase () {}

    virtual void clear () = 0;
    virtual void fill () const = 0;
    virtual Bool isPresent () const = 0;

protected:

    virtual void copy (const VbCacheItemBase * other, Bool markAsCached = False) = 0;

    VisBufferImpl * getVb () const
    {
        return vb_p;
    }

    virtual void
    initialize (VisBufferImpl * vb)
    {
        vb_p = vb;
        vb_p->registerCacheItem (this);
    }

    virtual void setAsPresent () = 0;

private:

    VisBufferImpl * vb_p; // [use]

};

typedef std::vector<VbCacheItemBase *> CacheRegistry;

template <typename T>
class VbCacheItem : public VbCacheItemBase {

    friend class VisBufferImpl;

public:

    typedef T DataType;
    typedef void (VisBufferImpl::* Filler) (T &) const;

    VbCacheItem ()
    : isPresent_p (False)
    {}

    virtual void
    clear ()
    {
        item_p = T ();
        isPresent_p = False;
    }


    virtual void
    fill () const
    {
        (getVb() ->* filler_p) (item_p);
    }

    const T &
    get () const
    {
        if (! isPresent_p){
            fill ();
            isPresent_p = True;
        }

        return item_p;
    }

    T &
    getRef ()
    {
        if (! isPresent_p){
            fill ();
            isPresent_p = True;
        }

        return item_p;
    }


    void
    initialize (VisBufferImpl * vb, Filler filler)
    {
        VbCacheItemBase::initialize (vb);
        filler_p = filler;
    }

    Bool
    isPresent () const
    {
        return isPresent_p;
    }

    virtual void
    set (const T & newItem)
    {
        ThrowIf (! getVb()->isWritable (), "This VisBuffer is readonly");

        item_p = newItem;
        isPresent_p = True;
    }

    template <typename U>
    void
    set (const U & newItem)
    {
        ThrowIf (! getVb()->isWritable (), "This VisBuffer is readonly");

        item_p = newItem;
        isPresent_p = True;
    }


protected:

    virtual void
    copy (const VbCacheItemBase * otherRaw, Bool markAsCached)
    {
        // Convert generic pointer to one pointint to this
        // cache item type.

        const VbCacheItem * other = dynamic_cast <const VbCacheItem *> (otherRaw);
        Assert (other != 0);

        // Capture the cached status of the other item

        isPresent_p = other->isPresent_p;

        // If the other item was cached then copy it over
        // otherwise clear out this item.

        if (isPresent_p){
            item_p = other->item_p;
        }
        else {
            item_p = T ();

            if (markAsCached){
                isPresent_p = True;
            }
        }
    }

    void
    setAsPresent ()
    {
        isPresent_p = True;
    }

private:

    Filler       filler_p;
    mutable Bool isPresent_p;
    mutable T    item_p;
};


class VisBufferCache {

    // Holds the cached values for a VisBuffer object.

public:

    VisBufferCache (VisBufferImpl * vb)
    {

        // Initialize the cache items.  This will also register them
        // with the vb object to allow it to iterate over the cached
        // values.

        antenna1_p.initialize (vb, & VisBufferImpl::fillAntenna1);
        antenna2_p.initialize (vb, &VisBufferImpl::fillAntenna2);
        arrayId_p.initialize (vb, &VisBufferImpl::fillArrayId);
        //chanAveBounds_p.initialize (vb, &VisBufferImpl::fillChanAveBounds);
        channel_p.initialize (vb, &VisBufferImpl::fillChannel);
        cjones_p.initialize (vb, &VisBufferImpl::fillJonesC);
        correctedVisCube_p.initialize (vb, &VisBufferImpl::fillCubeCorrected);
        correctedVisibility_p.initialize (vb, &VisBufferImpl::fillVisibilityCorrected);
        corrType_p.initialize (vb, &VisBufferImpl::fillCorrType);
        dataDescriptionId_p.initialize (vb, &VisBufferImpl::fillDataDescriptionId);
        dataDescriptionIds_p.initialize (vb, &VisBufferImpl::fillDataDescriptionIds);
        direction1_p.initialize (vb, &VisBufferImpl::fillDirection1);
        direction2_p.initialize (vb, &VisBufferImpl::fillDirection2);
        exposure_p.initialize (vb, &VisBufferImpl::fillExposure);
        feed1_p.initialize (vb, &VisBufferImpl::fillFeed1);
        feed1Pa_p.initialize (vb, &VisBufferImpl::fillFeedPa1);
        feed2_p.initialize (vb, &VisBufferImpl::fillFeed2);
        feed2Pa_p.initialize (vb, &VisBufferImpl::fillFeedPa2);
        fieldId_p.initialize (vb, &VisBufferImpl::fillFieldId);
        flag_p.initialize (vb, &VisBufferImpl::fillFlag);
        flagCategory_p.initialize (vb, &VisBufferImpl::fillFlagCategory);
        flagCube_p.initialize (vb, &VisBufferImpl::fillFlagCube);
        flagRow_p.initialize (vb, &VisBufferImpl::fillFlagRow);
        floatDataCube_p.initialize (vb, &VisBufferImpl::fillFloatData);
        frequency_p.initialize (vb, &VisBufferImpl::fillFrequency);
        imagingWeight_p.initialize (vb, &VisBufferImpl::fillImagingWeight);
        ////lsrFrequency_p.initialize (vb, &VisBufferImpl::fillLsrFrequency);
        modelVisCube_p.initialize (vb, &VisBufferImpl::fillCubeModel);
        modelVisibility_p.initialize (vb, &VisBufferImpl::fillVisibilityModel);
        nChannel_p.initialize (vb, &VisBufferImpl::fillNChannel);
        nCorr_p.initialize (vb, &VisBufferImpl::fillNCorr);
        ////nCat_p.initialize (vb, &VisBufferImpl::fillNCat);
        nRow_p.initialize (vb, &VisBufferImpl::fillNRow);
        observationId_p.initialize (vb, &VisBufferImpl::fillObservationId);
        phaseCenter_p.initialize (vb, &VisBufferImpl::fillPhaseCenter);
        polFrame_p.initialize (vb, &VisBufferImpl::fillPolFrame);
        processorId_p.initialize (vb, &VisBufferImpl::fillProcessorId);
        ////rowIds_p.initialize (vb, &VisBufferImpl::fillRowIds);
        scan_p.initialize (vb, &VisBufferImpl::fillScan);
        sigma_p.initialize (vb, &VisBufferImpl::fillSigma);
        sigmaMat_p.initialize (vb, &VisBufferImpl::fillSigmaMat);
        spectralWindow_p.initialize (vb, &VisBufferImpl::fillSpectralWindow);
        stateId_p.initialize (vb, &VisBufferImpl::fillStateId);
        time_p.initialize (vb, &VisBufferImpl::fillTime);
        timeCentroid_p.initialize (vb, &VisBufferImpl::fillTimeCentroid);
        timeInterval_p.initialize (vb, &VisBufferImpl::fillTimeInterval);
        uvw_p.initialize (vb, &VisBufferImpl::fillUvw);
        uvwMat_p.initialize (vb, &VisBufferImpl::fillUvwMat);
        visCube_p.initialize (vb, &VisBufferImpl::fillCubeObserved);
        visibility_p.initialize (vb, &VisBufferImpl::fillVisibilityObserved);
        weight_p.initialize (vb, &VisBufferImpl::fillWeight);
        weightMat_p.initialize (vb, &VisBufferImpl::fillWeightMat);
        weightSpectrum_p.initialize (vb, &VisBufferImpl::fillWeightSpectrum);
    }

    // The values that are potentially cached.

    VbCacheItem <Vector<Int> > antenna1_p;
    VbCacheItem <Vector<Int> > antenna2_p;
    VbCacheItem <Int> arrayId_p;
    VbCacheItem <Vector<Int> > channel_p;
    VbCacheItem <Vector<SquareMatrix<Complex, 2> > > cjones_p;
    VbCacheItem <Cube<Complex> > correctedVisCube_p;
    VbCacheItem <Matrix<CStokesVector> > correctedVisibility_p;
    VbCacheItem <Vector<Int> > corrType_p;
    VbCacheItem <Int> dataDescriptionId_p;
    VbCacheItem <Vector<Int> > dataDescriptionIds_p;
    VbCacheItem <Vector<MDirection> > direction1_p; //where the first antenna/feed is pointed to
    VbCacheItem <Vector<MDirection> > direction2_p; //where the second antenna/feed is pointed to
    VbCacheItem <Vector<Double> > exposure_p;
    VbCacheItem <Vector<Int> > feed1_p;
    VbCacheItem <Vector<Float> > feed1Pa_p;
    VbCacheItem <Vector<Int> > feed2_p;
    VbCacheItem <Vector<Float> > feed2Pa_p;
    VbCacheItem <Int> fieldId_p;
    VbCacheItem <Matrix<Bool> > flag_p;
    VbCacheItem <Array<Bool> > flagCategory_p;
    VbCacheItem <Cube<Bool> > flagCube_p;
    VbCacheItem <Vector<Bool> > flagRow_p;
    VbCacheItem <Cube<Float> > floatDataCube_p;
    VbCacheItem <Vector<Double> > frequency_p;
    VbCacheItem <Matrix<Float> > imagingWeight_p;
    //VbCacheItem <Vector<Double> > lsrFrequency_p;
    VbCacheItem <Cube<Complex> > modelVisCube_p;
    VbCacheItem <Matrix<CStokesVector> > modelVisibility_p;
    VbCacheItem <Int> nChannel_p;
    VbCacheItem <Int> nCorr_p;
    //    VbCacheItem <Int> nCat_p;
    VbCacheItem <Int> nRow_p;
    VbCacheItem <Vector<Int> > observationId_p;
    VbCacheItem <MDirection> phaseCenter_p;
    VbCacheItem <Int> polFrame_p;
    VbCacheItem <Vector<Int> > processorId_p;
    VbCacheItem <Vector<uInt> > rowIds_p;
    VbCacheItem <Vector<Int> > scan_p;
    VbCacheItem <Vector<Float> > sigma_p;
    VbCacheItem <Matrix<Float> > sigmaMat_p;
    VbCacheItem <Int> spectralWindow_p;
    VbCacheItem <Vector<Int> > stateId_p;
    VbCacheItem <Vector<Double> > time_p;
    VbCacheItem <Vector<Double> > timeCentroid_p;
    VbCacheItem <Vector<Double> > timeInterval_p;
    VbCacheItem <Vector<RigidVector<Double, 3> > > uvw_p;
    VbCacheItem <Matrix<Double> > uvwMat_p;
    VbCacheItem <Cube<Complex> > visCube_p;
    VbCacheItem <Matrix<CStokesVector> > visibility_p;
    VbCacheItem <Vector<Float> > weight_p;
    VbCacheItem <Matrix<Float> > weightMat_p;
    VbCacheItem <Cube<Float> > weightSpectrum_p;

    template <typename T, typename U>
    static void
    sortCorrelationItem (vi::VbCacheItem<T> & dataItem, IPosition & blc, IPosition & trc,
                         IPosition & mat, U & tmp, Bool sort)
    {

        T & data = dataItem.getRef ();
        U p1, p2, p3;

        if (dataItem.isPresent() && data.nelements() > 0) {

          blc(0) = trc(0) = 1;
          p1.reference(data (blc, trc).reform(mat));

          blc(0) = trc(0) = 2;
          p2.reference(data (blc, trc).reform(mat));

          blc(0) = trc(0) = 3;
          p3.reference(data (blc, trc).reform(mat));

          if (sort){ // Sort correlations: (PP,QQ,PQ,QP) -> (PP,PQ,QP,QQ)

              tmp = p1;
              p1 = p2;
              p2 = p3;
              p3 = tmp;
          }
          else {      // Unsort correlations: (PP,PQ,QP,QQ) -> (PP,QQ,PQ,QP)

              tmp = p3;
              p3 = p2;
              p2 = p1;
              p1 = tmp;
          }
        }
    }

};

class VisBufferState {

public:

    VisBufferState ()
    : corrSorted_p (False),
      dirtyComponents_p (),
      isAttached_p (False),
      isNewMs_p (False),
      isNewArrayId_p (False),
      isNewFieldId_p (False),
      isNewSpectralWindow_p (False),
      isWritable_p (False),
      lastPointTableRow_p (-1),
      vi_p (0),
      visModelData_p (VisModelDataI::create())
    {}

  ~VisBufferState () { delete visModelData_p;}

    Bool corrSorted_p; // Have correlations been sorted by sortCorr?
    VbDirtyComponents dirtyComponents_p;
    Bool isAttached_p;
    Bool isNewMs_p;
    Bool isNewArrayId_p;
    Bool isNewFieldId_p;
    Bool isNewSpectralWindow_p;
    Bool isWritable_p;
    mutable Int lastPointTableRow_p;
    Int msId_p;
    String msName_p;
    Bool newMs_p;
    ROVisibilityIterator2 * vi_p; // [use]
    mutable VisModelDataI * visModelData_p;

    CacheRegistry cacheRegistry_p;
};


using namespace vi;

////////////////////////////////////////////////////////////
//
// Basic VisBufferImpl Methods
// ========================
//
// Other sections contain the accessor and filler methods

VisBufferImpl::VisBufferImpl ()
: cache_p (0), state_p (0)
{
    construct ();
}

VisBufferImpl::VisBufferImpl (const VisBufferImpl & vb)
: cache_p (0), state_p (0)
{
    construct ();

    operator= (vb);
}

VisBufferImpl::~VisBufferImpl ()
{
    delete cache_p;
    delete state_p;
}


VisBufferImpl &
VisBufferImpl::operator= (const VisBufferImpl & other)
{
    if (this != &other) {

        assign (other);

    }

    return *this;
}

Bool
VisBufferImpl::areCorrelationsInCanonicalOrder () const
{
  Vector<Int> corrs(corrType());

  // Only a meaningful question is all 4 corrs present

  Bool result = corrs.nelements () == 4 &&
                (corrs (1) == Stokes::LL || corrs (1) == Stokes::YY);

  return result;
}

void
VisBufferImpl::assign (const VisBufferImpl & other, Bool copy)
{
    Assert (dynamic_cast<const VisBufferImpl *> (& other) != 0); // Can't cope with parent

    ThrowIf (other.state_p->corrSorted_p, "Cannot assign a VisBuffer that has had correlations sorted!");

    if (this != &other) {

        ThrowIf (state_p->isAttached_p,
                 "Cannot assign to a VisBuffer that is attached to a VisibilityIterator2");

        stateCopy (other);

        if (getViP() == 0){

            // The source VI is not associated with a VI.
            // In this case copy over all the information and any data
            // that is not present will be marked as present and given
            // values with zero length

            cacheCopy (other, True);
        }
        else if (copy){
            cacheCopy (other, False); // Copy anything that is cached
        }
        else{
            cacheClear ();
        }
    }
}

void
VisBufferImpl::attachToVisibilityIterator2 (ROVisibilityIterator2 & vi)
{
    ThrowIf (state_p->isAttached_p, "VisBuffer already attached to VisibilityIterator2");

    state_p->isAttached_p = True;
    state_p->vi_p = & vi;
}

void
VisBufferImpl::averageFlagInfoChannels (const Matrix<Int> & averagingBounds,
                                     Int nChannelsOut, Bool useWeightSpectrum)
{

    Array<Bool> & flagCategories = cache_p->flagCategory_p.getRef();

    Bool processCategories = flagCategories.nelements() != 0;
    IPosition categoriesShape (flagCategories.shape());
    Int nChannelsIn = categoriesShape(1);
    categoriesShape (1) = nChannelsOut; // make it match the new # of channels

    ThrowIf (nChannelsIn < nChannelsOut,
             String::format ("Can't average %d channels to yield %d channels.",
                            nChannelsIn, nChannelsOut));

    Cube<Bool> & flagCube = cache_p->flagCube_p.getRef();
    IPosition cubeShape (flagCube.shape());
    Assert (nChannelsIn == cubeShape (1)); // ought to be the same
    cubeShape (1) = nChannelsOut;  // make it match the new # of channels

    const Vector<Int>& channels = cache_p->channel_p.get();

    if(nChannelsIn == nChannelsOut && channels.nelements() > 0 && channels [0] == 0)
        return;    // No-op.

    Array<Bool> newFlagCategories (categoriesShape, True);
    Cube<Bool> newFlagCube (cubeShape, False);

    Cube<Float> weightSpectrum = cache_p->weightSpectrum_p.get ();
    Matrix<Float> weightMatrix = cache_p->weightMat_p.get();

    Int nCorrelations = nCorr();
    Int nCategories = categoriesShape (2);

    for(Int row = 0; row < nRow(); ++row){

        Int channelIn = 0;

        for(Int channelOut = 0; channelOut < nChannelsOut; ++channelOut){

            while(channels [channelIn] >= averagingBounds(channelOut, 0) &&
                  channels [channelIn] <= averagingBounds(channelOut, 1) &&
                  channelIn < nChannelsIn) {

                for(Int correlation = 0; correlation < nCorrelations; ++correlation){

                    // Process the flag cube

                    Double wt = useWeightSpectrum ? weightSpectrum (correlation, channelOut, row)
                                                  : weightMatrix (correlation, row);

                    if( ! flagCube (correlation, channelIn, row) && wt > 0.0){
                        newFlagCube (correlation, channelOut, row) = False;
                    }

                    if (processCategories){ // only if they exist

                        for(Int category = 0; category < nCategories; ++ category){

                            if(!flagCategories (IPosition(4, correlation, channelIn, category, row))){
                                newFlagCategories (IPosition(4, correlation, channelOut, category, row)) = False;
                            }
                        }
                    }
                }
                ++ channelIn;
            }
        }
    }

    // Install the new values

    cache_p->flagCube_p.set (newFlagCube);

    if (processCategories){
        cache_p->flagCategory_p.set (newFlagCategories);
    }
}

template<class T>
void
VisBufferImpl::averageVisCubeChannels (T & dataCache, Int nChannelsOut,
                                    const Matrix<Int>& averagingBounds)
{
    if (! dataCache.isPresent()){
        return; // Nothing to do
    }

    typename T::DataType & data = dataCache.getRef ();

    IPosition csh(data.shape());
    Int nChannelsIn = csh(1);

    ThrowIf (nChannelsIn < nChannelsOut,
             String::format ("Can't average %d channels to %d channels!\n"
                            "Data already averaged?",
                            nChannelsIn, nChannelsOut));

    csh(1) = nChannelsOut;

    const Vector<Int> & chans (channel());
    Bool areShifting = ! (chans.nelements() > 0 && chans[0] == 0);

    if (nChannelsIn == nChannelsOut && ! areShifting){
        return;                     // No-op
    }

    typename T::DataType newCube(csh, 0.0);

    Int nCor = nCorr();

    Bool doSpWt = getVi()->existsWeightSpectrum();

    // Make sure weightSpectrum() is unaveraged.

    if(doSpWt && (areShifting || weightSpectrum().shape()(1) < nChannelsIn)){
        cache_p->weightSpectrum_p.fill(); // fill the unaveraged weightt spectrum
    }

    Vector<Double> weightTotals (nCor);
    const Cube<Float> & weights = weightSpectrum ();

    for(Int row = 0; row < nRow(); ++row){

        if(flagRow()(row)){
            continue; // row flagged so skip it
        }

        Int channelIn = 0;

        for(Int channelOut = 0; channelOut < nChannelsOut; ++channelOut){

            weightTotals = 0;

            while(chans[channelIn] >= averagingBounds (channelOut, 0) &&
                  chans[channelIn] <= averagingBounds (channelOut, 1) &&
                  channelIn < nChannelsIn){

                for(Int icor = 0; icor < nCor; ++icor){

                    if(flagCube()(icor, channelIn, row)){
                        continue; // skip flagged correlation
                    }

                    typename T::DataType::value_type datum = data(icor, channelIn, row);
                    Double weight = 1.0;

                    if (doSpWt){ // weight the data point
                        weight = weights (icor, channelIn, row);
                        datum *= weight;
                    }

                    newCube(icor, channelOut, row) += datum;

                    weightTotals [icor] += weight;
                }
                ++channelIn;
            }

            for(Int icor = 0; icor < nCor; ++icor){
                if(weightTotals[icor] > 0.0){
                    newCube(icor, channelOut, row) *= 1.0 / weightTotals[icor];
                }
            }
        }
    }

    dataCache.set (newCube);
}

Int
VisBufferImpl::averageChannelFrequencies (Int nChannelsOut,
                                       const Vector<Int> & channels,
                                       const Matrix<Int> & averagingBounds)
{
    // Collapse the frequency values themselves, and count the number of
    // selected channels.
    // TBD: move this up to bounds calculation loop?

    Vector<Double> newFrequencies (nChannelsOut,0.0);
    Vector<Int> newChannels (nChannelsOut,0);
    const Vector<Double> & oldFrequencies = frequency(); // Ensure frequencies pre-filled

    Int nChannelsIn = channels.nelements();
    Int channelIn=0;
    Int nChannelsAveraged = 0;

    for(Int channelOut = 0; channelOut < nChannelsOut; ++channelOut){

        Int n = 0;

        while(channels[channelIn] >= averagingBounds(channelOut, 0) &&
              channels[channelIn] <= averagingBounds(channelOut, 1) &&
              channelIn < nChannelsIn){

            ++n;
            newFrequencies [channelOut] += (oldFrequencies[channelIn] - newFrequencies[channelOut]) / n;
            newChannels [channelOut] += channels [channelIn];
            channelIn++;
        }

        if (n>0) {
            newChannels[channelOut] /= n;
            nChannelsAveraged += n;
        }
    }

    // Install the new values

    cache_p->frequency_p.set (newFrequencies);
    cache_p->channel_p.set (newChannels);
    cache_p->nChannel_p.set (nChannelsOut);

    return nChannelsAveraged;
}

void
VisBufferImpl::averageChannels (const Matrix<Int>& averagingBounds)
{
    //  Only do something if there is something to do
    if (averagingBounds.nelements() <= 0)
        return;

    Int nChannelsOut = averagingBounds.nrow();

    Vector<Int> channels  = channel(); // Ensure channels pre-filled


    const Bool useWeightSpectrum = getVi()->existsWeightSpectrum();

    // Apply averaging to whatever data is present

    averageVisCubeChannels (cache_p->visCube_p, nChannelsOut, averagingBounds);
    averageVisCubeChannels (cache_p->modelVisCube_p, nChannelsOut, averagingBounds);
    averageVisCubeChannels (cache_p->correctedVisCube_p, nChannelsOut, averagingBounds);
    averageVisCubeChannels (cache_p->floatDataCube_p,  nChannelsOut, averagingBounds);

    uInt nCorrelations = nCorr();
    uInt nRows = nRow();
    Matrix<Float> rowWtFac(nCorrelations, nRows);

    uInt nChannelsSelected = flagCube().shape()(1);   // # of selected channels

    Matrix<Float> rowWeightFactors(nCorrelations, nRows);
    computeRowWeightFactors (rowWeightFactors, useWeightSpectrum);

    averageFlagInfoChannels (averagingBounds,nChannelsOut, useWeightSpectrum);

    Int nChannelsAveraged = averageChannelFrequencies (nChannelsOut, channels,
                                                       averagingBounds);

    adjustWeightFactorsAndFlags (rowWeightFactors, useWeightSpectrum, nRows,
                                 nCorrelations, nChannelsOut);

    adjustWeightAndSigmaMatrices (nChannelsAveraged, nChannelsOut, nRows,
                                  nCorrelations, nChannelsSelected, rowWeightFactors);

}

void
VisBufferImpl::adjustWeightAndSigmaMatrices (Int nChannelsAveraged, Int nChannelsOut, Int nRows,
                                          Int nCorrelations, Int nChannelsSelected,
                                          const Matrix <Float> & rowWeightFactors)
{
    Int nChannelsOriginally = getVi()->msColumns().spectralWindow().numChan()(spectralWindow());
        // Row weight and sigma currently refer to the number of channels in the
        // MS, regardless of any selection.

    Float selChanFac = static_cast<Float>(nChannelsAveraged) / nChannelsOriginally;
        // This is slightly fudgy because it ignores the unselected part of
        // weightSpectrum.  Unfortunately now that selection is applied to
        // weightSpectrum, we can't get at the unselected channel weights.

    Matrix <Float> & sigmaMat = cache_p->sigmaMat_p.getRef ();
    Matrix <Float> & weightMat = cache_p->weightMat_p.getRef ();

    for(Int row = 0; row < nRows; ++row){
        for(Int icor = 0; icor < nCorrelations; ++icor){

            Float factor = rowWeightFactors (icor, row);

            if (nChannelsAveraged < nChannelsOriginally){
                weightMat (icor, row) *= selChanFac * factor;
            }

            if(factor > 0.0){          // Unlike WEIGHT, SIGMA is for a single chan.
                sigmaMat (icor, row) /= sqrt (nChannelsSelected * factor / nChannelsOut);
            }
        }
    }
}


void
VisBufferImpl::adjustWeightFactorsAndFlags (Matrix <Float> & rowWeightFactors,
                                            Bool useWeightSpectrum,
                                            Int nRows,
                                            Int nCorrelations,
                                            Int nChannelsOut)
{
    if(useWeightSpectrum){

        const Cube<Float>& weights (weightSpectrum());
        Cube<Bool> & flagCube = cache_p->flagCube_p.getRef();

        for(Int row = 0; row < nRows; ++row){

            for(Int correlation = 0; correlation < nCorrelations; ++correlation){

                Float originalFactor = rowWeightFactors(correlation, row);
                rowWeightFactors(correlation, row) = 0.0;

                for(Int channelOut = 0; channelOut < nChannelsOut; ++channelOut){

                    Float oswt = weights (correlation, channelOut, row);       // output spectral
                    // weight
                    if(oswt > 0.0)
                        rowWeightFactors(correlation, row) += oswt;
                    else
                        flagCube (correlation, channelOut, row) = True;
                }

                if(originalFactor > 0.0)
                    rowWeightFactors(correlation, row) /= originalFactor;
            }
        }
    }
}
void
VisBufferImpl::computeRowWeightFactors (Matrix <Float> & rowWeightFactors, Bool useWeightSpectrum)
{
    Int nCorrelations = nCorr();
    Int nRows = nRow();

    Int nChannels = flagCube().shape()(1);   // # of selected channels

    rowWeightFactors = (useWeightSpectrum) ? 0.0   // initialize for loops
                                           : 1.0;  // result is 1.0

    if (! useWeightSpectrum){
        return;
    }

    // Get the total weight spectrum

    rowWeightFactors = 0.0;

    const Cube<Float>& weights (weightSpectrum());   // while it is unaveraged.

    for(Int row = 0; row < nRows; ++row){

        for(Int icor = 0; icor < nCorrelations; ++icor){

            for(Int channelIn = 0; channelIn < nChannels; ++channelIn){

                rowWeightFactors(icor, row) += weights (icor, channelIn, row);
                    // Presumably the input row weight was set without taking flagging
                    // into account.
            }
        }
    }
}

void
VisBufferImpl::checkVisIter (const char * func, const char * file, int line, const char * extra) const
{
  checkVisIterBase (func, file, line, extra);
}

void
VisBufferImpl::checkVisIterBase (const char * func, const char * file, int line, const char * extra) const
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

VisBufferImpl *
VisBufferImpl::clone () const
{
    return new VisBufferImpl (* this);
}

void
VisBufferImpl::cacheCopy (const VisBufferImpl & other, Bool markAsCached)
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
VisBufferImpl::cacheClear (Bool markAsCached)
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
VisBufferImpl::construct ()
{
    state_p = new VisBufferState ();

    // Initialize all non-object member variables

    state_p->corrSorted_p = False; // Have correlations been sorted by sortCorr?
    state_p->isAttached_p = False;
    state_p->lastPointTableRow_p = -1;
    state_p->newMs_p = True;
    state_p->vi_p = 0;

    cache_p = new VisBufferCache (this);
}

void
VisBufferImpl::detachFromVisibilityIterator2 ()
{
    ThrowIf (! state_p->isAttached_p,
             "Cannot detach VisBuffer from VI since it is not attached.");

#warning "Uncomment next line after cutover"
    //// getViP()->detachVisBuffer (* this);

    state_p->isAttached_p = False;
    state_p->vi_p = 0;
}


void
VisBufferImpl::dirtyComponentsAdd (const VbDirtyComponents & dirtyComponents)
{
    state_p->dirtyComponents_p = state_p->dirtyComponents_p + dirtyComponents;
}

void
VisBufferImpl::dirtyComponentsAdd (VisBufferComponents::EnumType component)
{
    state_p->dirtyComponents_p = state_p->dirtyComponents_p + VbDirtyComponents::singleton (component);
}


void
VisBufferImpl::dirtyComponentsClear ()
{
    state_p->dirtyComponents_p = VbDirtyComponents::none();
}

VbDirtyComponents
VisBufferImpl::dirtyComponentsGet () const
{
    return state_p->dirtyComponents_p;
}

void
VisBufferImpl::dirtyComponentsSet (const VbDirtyComponents & dirtyComponents)
{
    state_p->dirtyComponents_p = dirtyComponents;
}

void
VisBufferImpl::dirtyComponentsSet (VisBufferComponents::EnumType component)
{
    state_p->dirtyComponents_p = VbDirtyComponents::singleton (component);
}

Int
VisBufferImpl::msId () const
{
    return state_p->msId_p;
}

String
VisBufferImpl::msName (Bool stripPath) const
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
VisBufferImpl::getVi () const
{
    return state_p->vi_p;
}

ROVisibilityIterator2 *
VisBufferImpl::getViP () const
{
    return state_p->vi_p;
}

void
VisBufferImpl::invalidate ()
{
    cacheClear (False); // empty cached values
}

Bool
VisBufferImpl::isNewArrayId () const
{
    return state_p->isNewArrayId_p;
}

Bool
VisBufferImpl::isNewFieldId () const
{
    return state_p->isNewFieldId_p;
}

Bool
VisBufferImpl::isNewMs() const
{
    return state_p->isNewMs_p;
}

Bool
VisBufferImpl::isNewSpectralWindow () const
{
    return state_p->isNewSpectralWindow_p;
}

Bool
VisBufferImpl::isWritable () const
{
    return state_p->isWritable_p;
}

void
VisBufferImpl::normalize(const Bool & /* phaseOnly */)
{
#warning "*** Carefully go over recode of this method (VisBufferImpl::normalize)"

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
VisBufferImpl::registerCacheItem (VbCacheItemBase * item)
{
    state_p->cacheRegistry_p.push_back (item);
}

void
VisBufferImpl::resetWeightsUsingSigma ()
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
VisBufferImpl::setIterationInfo (Int msId, const String & msName, Bool isNewMs,
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
VisBufferImpl::sortCorrelationsAux (bool makeSorted)
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

    if (! areCorrelationsInCanonicalOrder() && ! state_p->corrSorted_p) {

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

        state_p->corrSorted_p = makeSorted;
    }

}

// Sort correlations: (PP,QQ,PQ,QP) -> (PP,PQ,QP,QQ)
void
VisBufferImpl::sortCorr()
{
    sortCorrelationsAux (False);
}


void
VisBufferImpl::stateCopy (const VisBufferImpl & other)
{
    // Copy state from the other buffer

    state_p->corrSorted_p = other.state_p->corrSorted_p;
    state_p->dirtyComponents_p = other.state_p->dirtyComponents_p;
    state_p->isAttached_p = False;  // attachment isn't copyabled
    state_p->isNewArrayId_p = other.state_p->isNewArrayId_p;
    state_p->isNewFieldId_p = other.state_p->isNewFieldId_p;
    state_p->isNewMs_p = other.state_p->isNewMs_p;
    state_p->isNewSpectralWindow_p = other.state_p->isNewSpectralWindow_p;
    state_p->lastPointTableRow_p = other.state_p->lastPointTableRow_p;
    state_p->newMs_p = other.state_p->newMs_p;
    state_p->vi_p = other.getViP ();
    state_p->visModelData_p = other.state_p->visModelData_p;
}


// Unsort correlations: (PP,PQ,QP,QQ) -> (PP,QQ,PQ,QP)
void
VisBufferImpl::unSortCorr()
{
    sortCorrelationsAux (False);
}

template <typename Coord>
void
VisBufferImpl::updateCoord (Coord & item,
                         const Coord & otherItem)
{
    if (otherItem.isPresent()){
        item.set (otherItem.get());
    }
    else{
        item.get (); // force fetching of the info
    }
}


void
VisBufferImpl::updateCoordInfo (const VisBufferImpl * vb, const  Bool dirDependent)
{
    updateCoord (cache_p->antenna1_p, vb->cache_p->antenna1_p);
    updateCoord (cache_p->antenna2_p, vb->cache_p->antenna2_p);
    updateCoord (cache_p->arrayId_p, vb->cache_p->arrayId_p);
    updateCoord (cache_p->dataDescriptionId_p, vb->cache_p->dataDescriptionId_p);
    updateCoord (cache_p->fieldId_p, vb->cache_p->fieldId_p);
    updateCoord (cache_p->spectralWindow_p, vb->cache_p->spectralWindow_p);
    updateCoord (cache_p->time_p, vb->cache_p->time_p);
    updateCoord (cache_p->frequency_p, vb->cache_p->frequency_p);
    updateCoord (cache_p->nRow_p, vb->cache_p->nRow_p);

    setIterationInfo (vb->msId(), vb->msName (), vb->isNewMs (),
                      vb->isNewArrayId (), vb->isNewFieldId (), vb->isNewSpectralWindow ());

    updateCoord (cache_p->feed1_p, vb->cache_p->feed1_p);
    updateCoord (cache_p->feed2_p, vb->cache_p->feed2_p);

    if(dirDependent){
        updateCoord (cache_p->feed1Pa_p, vb->cache_p->feed1Pa_p);
        updateCoord (cache_p->feed2Pa_p, vb->cache_p->feed2Pa_p);
        updateCoord (cache_p->direction1_p, vb->cache_p->direction1_p);
        updateCoord (cache_p->direction2_p, vb->cache_p->direction2_p);
    }
}


void
VisBufferImpl::validate ()
{
    cacheClear (True); // empty values but mark as cached.
}



//      +---------------+
//      |               |
//      |  Calculators  |
//      |               |
//      +---------------+


MDirection
VisBufferImpl::azel0(Double time) const
{
  return getViP()->azel0(time);
}

void
VisBufferImpl::azel0Vec(Double time, Vector<Double>& azelVec) const
{
  MDirection azelMeas = azel0(time);

  azelVec.resize(2);

  azelVec = azelMeas.getAngle("deg").getValue();
}

Vector<MDirection>
VisBufferImpl::azel(Double time) const
{
  return getViP()->azel(time);
}

void
VisBufferImpl::azelMat(Double time, Matrix<Double>& azelMat) const
{
  Vector<MDirection> azelMeas = azel(time);

  azelMat.resize(2, azelMeas.nelements());

  for (uInt iant = 0; iant < azelMeas.nelements(); ++iant) {
    azelMat.column(iant) = (azelMeas(iant).getAngle("deg").getValue());
  }
}

Vector<Float>
VisBufferImpl::feed_pa(Double time) const
{
  return getViP()->feed_pa(time);
}

Double
VisBufferImpl::hourang(Double time) const
{
  return getViP()->hourang(time);
}

Float
VisBufferImpl::parang0(Double time) const
{
  return getViP()->parang0(time);
}

Vector<Float>
VisBufferImpl::parang(Double time) const
{
  return getViP()->parang(time);
}

//      +-------------+
//      |             |
//      |  Accessors  |
//      |             |
//      +-------------+

Vector<Int>
VisBufferImpl::antenna1 () const
{
    return cache_p->antenna1_p.get ();
}

const Vector<Int> &
VisBufferImpl::antenna2 () const
{
    return cache_p->antenna2_p.get ();
}

Int
VisBufferImpl::arrayId () const
{
    return cache_p->arrayId_p.get ();
}

const Vector<Int> &
VisBufferImpl::channel () const
{
    return cache_p->channel_p.get ();
}

const Vector<SquareMatrix<Complex, 2> > &
VisBufferImpl::cjones () const
{
    return cache_p->cjones_p.get ();
}

const Cube<Complex> &
VisBufferImpl::correctedVisCube () const
{
    return cache_p->correctedVisCube_p.get ();
}

void
VisBufferImpl::setCorrectedVisCube (const Cube<Complex> & value)
{
    cache_p->correctedVisCube_p.set (value);
}

const Matrix<CStokesVector> &
VisBufferImpl::correctedVisibility () const
{
    return cache_p->correctedVisibility_p.get ();
}

void
VisBufferImpl::setCorrectedVisibility (const Matrix<CStokesVector> & value)
{
    cache_p->correctedVisibility_p.set (value);
}

const Vector<Int> &
VisBufferImpl::corrType () const
{
    return cache_p->corrType_p.get ();
}

Int
VisBufferImpl::dataDescriptionId () const
{
    return cache_p->dataDescriptionId_p.get ();
}

const Vector<MDirection> &
VisBufferImpl::direction1 () const
{
    return cache_p->direction1_p.get ();
}

const Vector<MDirection> &
VisBufferImpl::direction2 () const
{
    return cache_p->direction2_p.get ();
}

const Vector<Double> &
VisBufferImpl::exposure () const
{
    return cache_p->exposure_p.get ();
}

const Vector<Int> &
VisBufferImpl::feed1 () const
{
    return cache_p->feed1_p.get ();
}

const Vector<Float> &
VisBufferImpl::feed1_pa () const
{
    return cache_p->feed1Pa_p.get ();
}

const Vector<Int> &
VisBufferImpl::feed2 () const
{
    return cache_p->feed2_p.get ();
}

const Vector<Float> &
VisBufferImpl::feed2_pa () const
{
    return cache_p->feed2Pa_p.get ();
}

Int
VisBufferImpl::fieldId () const
{
    return cache_p->fieldId_p.get ();
}

const Matrix<Bool> &
VisBufferImpl::flag () const
{
    return cache_p->flag_p.get ();
}

void
VisBufferImpl::setFlag (const Matrix<Bool>& value)
{
    cache_p->flag_p.set (value);
}

const Array<Bool> &
VisBufferImpl::flagCategory () const
{
    return cache_p->flagCategory_p.get ();
}

void
VisBufferImpl::setFlagCategory (const Array<Bool>& value)
{
    cache_p->flagCategory_p.set (value);
}

const Cube<Bool> &
VisBufferImpl::flagCube () const
{
    return cache_p->flagCube_p.get ();
}

void
VisBufferImpl::setFlagCube (const Cube<Bool>& value)
{
    cache_p->flagCube_p.set (value);
}

const Vector<Bool> &
VisBufferImpl::flagRow () const
{
    return cache_p->flagRow_p.get ();
}

void
VisBufferImpl::setFlagRow (const Vector<Bool>& value)
{
    cache_p->flagRow_p.set (value);
}

const Cube<Float> &
VisBufferImpl::floatDataCube () const
{
    return cache_p->floatDataCube_p.get ();
}

void
VisBufferImpl::setFloatDataCube (const Cube<Float> & value)
{
    cache_p->floatDataCube_p.set (value);
}

const Vector<Double> &
VisBufferImpl::frequency () const
{
    return cache_p->frequency_p.get ();
}

const Matrix<Float> &
VisBufferImpl::imagingWeight () const
{
    return cache_p->imagingWeight_p.get ();
}

//const Vector<Double> &
//VisBufferImpl::lsrFrequency () const
//{
//    return cache_p->lsrFrequency_p.get ();
//}

const Cube<Complex> &
VisBufferImpl::modelVisCube () const
{
    return cache_p->modelVisCube_p.get ();
}

void
VisBufferImpl::setModelVisCube (const Complex & value)
{
    cache_p->modelVisCube_p.set (value);
}

void
VisBufferImpl::setModelVisCube (const Cube<Complex> & value)
{
    cache_p->modelVisCube_p.set (value);
}

const Matrix<CStokesVector> &
VisBufferImpl::modelVisibility () const
{
    return cache_p->modelVisibility_p.get ();
}

void
VisBufferImpl::setModelVisibility (Matrix<CStokesVector> & value)
{
    cache_p->modelVisibility_p.set (value);
}

void
VisBufferImpl::setModelVisCube(const Vector<Float>& stokesIn)
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
VisBufferImpl::nChannel () const
{
    return cache_p->nChannel_p.get ();
}

Int
VisBufferImpl::nCorr () const
{
    return cache_p->nCorr_p.get();
}

Int
VisBufferImpl::nRow () const
{
    return cache_p->nRow_p.get ();
}

const Vector<Int> &
VisBufferImpl::observationId () const
{
    return cache_p->observationId_p.get ();
}

const MDirection&
VisBufferImpl::phaseCenter () const
{
    return cache_p->phaseCenter_p.get ();
}

Int
VisBufferImpl::polFrame () const
{
    return cache_p->polFrame_p.get ();
}

const Vector<Int> &
VisBufferImpl::processorId () const
{
    return cache_p->processorId_p.get ();
}

const Vector<uInt> &
VisBufferImpl::rowIds () const
{
    return cache_p->rowIds_p.get ();
}

const Vector<Int> &
VisBufferImpl::scan () const
{
    return cache_p->scan_p.get ();
}

const Vector<Float> &
VisBufferImpl::sigma () const
{
    return cache_p->sigma_p.get ();
}

const Matrix<Float> &
VisBufferImpl::sigmaMat () const
{
    return cache_p->sigmaMat_p.get ();
}

Int
VisBufferImpl::spectralWindow () const
{
    return cache_p->spectralWindow_p.get ();
}

const Vector<Int> &
VisBufferImpl::stateId () const
{
    return cache_p->stateId_p.get ();
}

const Vector<Double> &
VisBufferImpl::time () const
{
    return cache_p->time_p.get ();
}

const Vector<Double> &
VisBufferImpl::timeCentroid () const
{
    return cache_p->timeCentroid_p.get ();
}

const Vector<Double> &
VisBufferImpl::timeInterval () const
{
    return cache_p->timeInterval_p.get ();
}

const Vector<RigidVector<Double, 3> > &
VisBufferImpl::uvw () const
{
    return cache_p->uvw_p.get ();
}

const Matrix<Double> &
VisBufferImpl::uvwMat () const
{
    return cache_p->uvwMat_p.get ();
}

const Cube<Complex> &
VisBufferImpl::visCube () const
{
    return cache_p->visCube_p.get ();
}

void
VisBufferImpl::setVisCube (const Complex & value)
{
    cache_p->visCube_p.set (value);
}

void
VisBufferImpl::setVisCube (const Cube<Complex> & value)
{
    cache_p->visCube_p.set (value);
}

const Matrix<CStokesVector> &
VisBufferImpl::visibility () const
{
    return cache_p->visibility_p.get ();
}

void
VisBufferImpl::setVisibility (Matrix<CStokesVector> & value)
{
    cache_p->visibility_p.set (value);
}

const Vector<Float> &
VisBufferImpl::weight () const
{
    return cache_p->weight_p.get ();
}

void
VisBufferImpl::setWeight (const Vector<Float>& value)
{
    cache_p->weight_p.set (value);
}

const Matrix<Float> &
VisBufferImpl::weightMat () const
{
    return cache_p->weightMat_p.get ();
}

void
VisBufferImpl::setWeightMat (const Matrix<Float>& value)
{
    cache_p->weightMat_p.set (value);
}

const Cube<Float> &
VisBufferImpl::weightSpectrum () const
{
    return cache_p->weightSpectrum_p.get ();
}

void
VisBufferImpl::setWeightSpectrum (const Cube<Float>& value)
{
    cache_p->weightSpectrum_p.set (value);
}

//      +-----------+
//      |           |
//      |  Fillers  |
//      |           |
//      +-----------+


void
VisBufferImpl::fillAntenna1 (Vector<Int>& value) const
{
  CheckVisIter ();

  getViP()->antenna1 (value);
}


void
VisBufferImpl::fillAntenna2 (Vector<Int>& value) const
{
  CheckVisIter ();

  getViP()->antenna2 (value);
}

void
VisBufferImpl::fillArrayId (Int& value) const
{
  CheckVisIter ();

  value = getViP()->arrayId ();
}

void
VisBufferImpl::fillChannel (Vector<Int>& value) const
{
  CheckVisIter ();

  getViP()->channel (value);
}

void
VisBufferImpl::fillCorrType (Vector<Int>& value) const
{
  CheckVisIter ();

  getViP()->corrType (value);
}

void
VisBufferImpl::fillCubeCorrected (Cube <Complex> & value) const
{
    CheckVisIter ();

    getViP()->visibility (value, VisibilityIterator2::Corrected);
}


void
VisBufferImpl::fillCubeModel (Cube <Complex> & value) const
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
VisBufferImpl::fillCubeObserved (Cube <Complex> & value) const
{
    CheckVisIter ();

    getViP()->visibility (value, VisibilityIterator2::Observed);
}

void
VisBufferImpl::fillDataDescriptionId  (Int& value) const
{
  CheckVisIter ();

  value = getViP()->dataDescriptionId ();
}

void
VisBufferImpl::fillDirection1 (Vector<MDirection>& value) const
{
  CheckVisIterBase ();
  // fill state_p->feed1_pa cache, antenna, feed and time will be filled automatically

  feed1_pa();

  fillDirectionAux (value, antenna1 (), feed1 (), feed1_pa ());

  value.resize(antenna1 ().nelements()); // could also use nRow()
}

void
VisBufferImpl::fillDirection2 (Vector<MDirection>& value) const
{
  CheckVisIterBase ();
  // fill state_p->feed1_pa cache, antenna, feed and time will be filled automatically

  feed2_pa();

  fillDirectionAux (value, antenna2 (), feed2 (), feed2_pa ());

  value.resize(antenna2 ().nelements()); // could also use nRow()
}

void
VisBufferImpl::fillDirectionAux (Vector<MDirection>& value,
                             const Vector<Int> & antenna,
                             const Vector<Int> &feed,
			     const Vector<Float> & feedPa) const
{
  value.resize (antenna.nelements()); // could also use nRow()

  const ROMSPointingColumns & mspc = getViP()->msColumns().pointing();
  state_p->lastPointTableRow_p = mspc.pointingIndex (antenna (0),
                                            time()(0), state_p->lastPointTableRow_p);
  if (getViP()->allBeamOffsetsZero() && state_p->lastPointTableRow_p < 0) {

    // No true pointing information found; use phase center from the field table

    value.set(phaseCenter());
    state_p->lastPointTableRow_p = 0;
    return;
  }

  for (uInt row = 0; row < antenna.nelements(); ++row) {

    DebugAssert(antenna (row) >= 0 && feed (row) >= 0, AipsError);

    Int pointIndex1 = mspc.pointingIndex(antenna (row), time()(row), state_p->lastPointTableRow_p);

    if (pointIndex1 >= 0) {
      state_p->lastPointTableRow_p = pointIndex1;
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
VisBufferImpl::fillExposure (Vector<Double>& value) const
{
  CheckVisIter ();

  getViP()->exposure (value);
}

void
VisBufferImpl::fillFeed1 (Vector<Int>& value) const
{
  CheckVisIter ();

  getViP()->feed1 (value);
}

void
VisBufferImpl::fillFeed2 (Vector<Int>& value) const
{
  CheckVisIter ();

  getViP()->feed2 (value);
}

void
VisBufferImpl::fillFeedPa1 (Vector <Float> & feedPa) const
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
VisBufferImpl::fillFeedPa2 (Vector <Float> & feedPa) const
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
VisBufferImpl::fillFeedPaAux (Vector <Float> & feedPa,
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
VisBufferImpl::fillFieldId (Int& value) const
{
  CheckVisIter ();

  value = getViP()->fieldId ();
}

void
VisBufferImpl::fillFlag (Matrix<Bool>& value) const
{
  CheckVisIter ();

  getViP()->flag (value);
}

void
VisBufferImpl::fillFlagCategory (Array<Bool>& value) const
{
  CheckVisIter();

  getViP()->flagCategory (value);
}

void
VisBufferImpl::fillFlagCube (Cube<Bool>& value) const
{
  CheckVisIter ();

  getViP()->flag (value);
}

void
VisBufferImpl::fillFlagRow (Vector<Bool>& value) const
{
  CheckVisIter ();

  getViP()->flagRow (value);
}

void
VisBufferImpl::fillFloatData (Cube<Float>& value) const
{
  CheckVisIter ();

  getViP()->floatData (value);
}

void
VisBufferImpl::fillImagingWeight (Matrix<Float> & value) const
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
VisBufferImpl::fillFrequency (Vector<Double>& value) const
{
  CheckVisIter ();

  getViP()->frequency (value);
}

void
VisBufferImpl::fillJonesC (Vector<SquareMatrix<Complex, 2> >& /*value*/) const
{
  CheckVisIter ();

#warning "Uncomment later"
////  getViP()->jonesC (value);
}


void
VisBufferImpl::fillNChannel (Int& value) const
{
  CheckVisIter ();

  //  state_p->nChannel_p=getViP()->channelGroupSize (value);
  value = channel().nelements();
}

void
VisBufferImpl::fillNCorr (Int& value) const
{
  CheckVisIter ();

  value  = corrType().nelements();
}

void
VisBufferImpl::fillNRow (Int& value) const
{
  CheckVisIter ();

  value = getViP()->nRow ();
}

void
VisBufferImpl::fillObservationId (Vector<Int>& value) const
{
  CheckVisIter();

  getViP()->observationId (value);
}

void
VisBufferImpl::fillPhaseCenter (MDirection& value) const
{
  CheckVisIter ();

  value = getViP()->phaseCenter ();
}

void
VisBufferImpl::fillPolFrame (Int& value) const
{
  CheckVisIter ();

  value = getViP()->polFrame ();
}

void
VisBufferImpl::fillProcessorId (Vector<Int>& value) const
{
  CheckVisIter();

  getViP()->processorId (value);
}

void
VisBufferImpl::fillScan (Vector<Int>& value) const
{
  CheckVisIter ();

  getViP()->scan (value);
}

void
VisBufferImpl::fillSigma (Vector<Float>& value) const
{
  CheckVisIter ();

  getViP()->sigma (value);
}

void
VisBufferImpl::fillSigmaMat (Matrix<Float>& value) const
{
  CheckVisIter ();

  getViP()->sigmaMat (value);
}

void
VisBufferImpl::fillSpectralWindow (Int& value) const
{
  CheckVisIter ();

  value = getViP()->spectralWindow ();
}

void
VisBufferImpl::fillStateId (Vector<Int>& value) const
{
  CheckVisIter();

  getViP()->stateId (value);
}


void
VisBufferImpl::fillTime (Vector<Double>& value) const
{
  CheckVisIter ();

  getViP()->time (value);
}

void
VisBufferImpl::fillTimeCentroid (Vector<Double>& value) const
{
  CheckVisIter ();

  getViP()->timeCentroid (value);
}

void
VisBufferImpl::fillTimeInterval (Vector<Double>& value) const
{
  CheckVisIter ();

  getViP()->timeInterval (value);
}

void
VisBufferImpl::fillUvw (Vector<RigidVector<Double, 3> >& value) const
{
  CheckVisIter ();

  getViP()->uvw (value);
}

void
VisBufferImpl::fillUvwMat (Matrix<Double>& value) const
{
  CheckVisIter ();

  getViP()->uvwMat (value);
}

void
VisBufferImpl::fillVisibilityCorrected (Matrix<CStokesVector>& value) const
{
    CheckVisIter ();

    getViP()->visibility (value, VisibilityIterator2::Corrected);
}

void
VisBufferImpl::fillVisibilityModel (Matrix<CStokesVector>& value) const
{
    CheckVisIter ();

    getViP()->visibility (value, VisibilityIterator2::Model);
}

void
VisBufferImpl::fillVisibilityObserved (Matrix<CStokesVector>& value) const
{
    CheckVisIter ();

    getViP()->visibility (value, VisibilityIterator2::Observed);
}


void
VisBufferImpl::fillWeight (Vector<Float>& value) const
{
  CheckVisIter ();

  getViP()->weight (value);
}

void
VisBufferImpl::fillWeightMat (Matrix<Float>& value) const
{
  CheckVisIter ();

  getViP()->weightMat (value);
}

void
VisBufferImpl::fillWeightSpectrum (Cube<Float>& value) const
{
  CheckVisIter ();

  getViP()->weightSpectrum (value);
}

} // end namespace vb

} // end namespace casa
