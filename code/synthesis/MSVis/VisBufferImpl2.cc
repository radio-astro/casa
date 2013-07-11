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
#include <casa/Utilities.h>
#include <casa/aipstype.h>
#include <typeinfo>
#include <components/ComponentModels/ComponentList.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <synthesis/MSVis/UtilJ.h>
#include <synthesis/MSVis/VisBufferAsyncWrapper2.h>
#include <synthesis/MSVis/VisBufferComponents2.h>
#include <synthesis/MSVis/VisBufferComponents2.h>
#include <synthesis/MSVis/VisBufferImpl2.h>
#include <synthesis/MSVis/VisBuffer2Adapter.h>
#include <synthesis/MSVis/VisibilityIterator2.h>
#include <synthesis/TransformMachines/VisModelData.h>
#include <synthesis/TransformMachines/FTMachine.h>


#define CheckVisIter() checkVisIter (__func__, __FILE__, __LINE__)
#define CheckVisIter1(s) checkVisIter (__func__, __FILE__, __LINE__,s)
#define CheckVisIterBase() checkVisIterBase (__func__, __FILE__, __LINE__)

#include <functional>

using namespace std;

namespace casa {

namespace vi {

//////////////////////////////////////////////////////////
//
// Auxiliary Classes are contained in the "vb" namespace.
//
// These include VbCacheItemBase, VbCacheItem, VisBufferCache
// and VisBufferState.


// Possible array shapes of data coming from the main table cells.

typedef enum {NoCheck, Nr, NfNr, NcNr, NcNfNr, NcNfNcatNr, I3Nr, N_ShapePatterns} ShapePattern;

class VisBufferCache;

class VbCacheItemBase {

    // Provides a common base class for all of the cached value classes.
    // This is required because the actualy value classes use a template
    // to capture the underlying value type.

    friend class VisBufferImpl2;

public:

    VbCacheItemBase () : vbComponent_p (Unknown), vb_p (0) {}

    virtual ~VbCacheItemBase () {}

    virtual void clear (Bool clearStatusOnly = False) = 0;
    virtual void clearDirty () = 0;
    virtual void copyRowElement (Int sourceRow, Int destinationRow) = 0;
    virtual void fill () const = 0;
    VisBufferComponent2
    getComponent () const
    {
        return vbComponent_p;
    }
    virtual Bool isArray () const = 0;
    virtual Bool isDirty () const = 0;
    virtual Bool isPresent () const = 0;
    virtual Bool isShapeOk () const = 0;
    virtual void resize (Bool /*copyValues*/) {}
    virtual void resizeRows (Int /*newNRows*/) {}
    virtual void setDirty () = 0;
    virtual String shapeErrorMessage () const = 0;

protected:

    virtual void copy (const VbCacheItemBase * other, Bool fetchIfNeeded) = 0;

    VisBufferImpl2 * getVb () const
    {
        return vb_p;
    }

    virtual void initialize (VisBufferCache * cache, VisBufferImpl2 * vb, VisBufferComponent2 component,
                             Bool isKey = True);

    Bool isKey () const { return isKey_p;}

    virtual void setAsPresent (Bool isPresent = True) const = 0;
    void setIsKey (Bool isKey)
    {
        isKey_p = isKey;
    }

private:

    Bool isKey_p;
    VisBufferComponent2 vbComponent_p;
    VisBufferImpl2 * vb_p; // [use]

};

typedef std::vector<VbCacheItemBase *> CacheRegistry;

template <typename T, Bool IsComputed = False>
class VbCacheItem : public VbCacheItemBase {

    friend class VisBufferImpl2;

public:

    typedef T DataType;
    typedef void (VisBufferImpl2::* Filler) (T &) const;

    VbCacheItem ()
    : VbCacheItemBase (), isPresent_p (False)
    {}

    virtual void
    clear (Bool clearStatusOnly)
    {
        if (! clearStatusOnly) {
            clearValue (item_p);
        }
        setAsPresent (False);
        clearDirty ();
    }

    virtual void
    clearDirty ()
    {
        isDirty_p = False;
    }

    virtual void copyRowElement (Int /*sourceRow*/, Int /*destinationRow*/) {} // noop


    virtual void
    fill () const
    {
        const VisBufferImpl2 * vb = getVb();

        ThrowIf (! vb->isAttached (),
                 String::format ("Can't fill VisBuffer component %s: Not attached to VisibilityIterator",
                                 VisBufferComponents2::name (getComponent()).c_str()));

        ThrowIf (! IsComputed && ! vb->isFillable (),
                 String::format ("Cannot fill VisBuffer component %s: %s",
                                 VisBufferComponents2::name (getComponent()).c_str(),
                                 vb->getFillErrorMessage ().c_str()));

        (vb ->* filler_p) (item_p);
    }

    const T &
    get () const
    {
        if (! isPresent()){
            fill ();
            setAsPresent ();
            isDirty_p = False;
        }

        return item_p;
    }

    T &
    getRef ()
    {
        if (! isPresent()){
            fill ();
            setAsPresent();
        }

        // Caller is getting a modifiabled reference to the
        // datum (otherwise they would use "get"): assume
        // that it will be used to modify the datum and mark
        // it as dirty.

        isDirty_p = True;

        return item_p;
    }

    void
    initialize (VisBufferCache * cache, VisBufferImpl2 * vb, Filler filler,
                VisBufferComponent2 component = Unknown,
                Bool isKey = True)
    {
        VbCacheItemBase::initialize (cache, vb, component, isKey);
        filler_p = filler;
    }

    Bool isArray () const
    {
        return False;
    }

    Bool
    isDirty () const
    {
        return isDirty_p;
    }

    Bool
    isPresent () const
    {
        return isPresent_p;
    }

    virtual Bool
    isShapeOk () const
    {
        return True;
    }

    virtual void
    set (const T & newItem)
    {
        ThrowIf (! getVb()->isWritable (), "This VisBuffer is readonly");

        ThrowIf (isKey() && ! getVb()->isRekeyable (),
                 "This VisBuffer is does not allow row key values to be changed.");

        // Set operations to a rekeyable VB are allowed to change the shapes of the
        // values.  When T derives from Array, the assign method will use Array::assign
        // which resizes the destination value to match the source value.  For nonkeyable
        // VBs, the normal operator= method is used which for Arrays will throw an
        // exception when a shape incompatibility exists between the source and destination.

        if (isKey ()){
            assign (item_p, newItem);
        }
        else{
            item_p = newItem;
        }

        ThrowIf (! isShapeOk (), shapeErrorMessage() );

        setAsPresent();
        isDirty_p = True;
    }


    template <typename U>
    void
    set (const U & newItem)
    {
        ThrowIf (! getVb()->isWritable (), "This VisBuffer is readonly");

        ThrowIf (isKey () && ! getVb()->isRekeyable (),
                 "This VisBuffer is does not allow row key values to be changed.");

        item_p = newItem;

        ThrowIf (! isShapeOk (), shapeErrorMessage() );

        setAsPresent();
        isDirty_p = True;
    }

    template <typename U>
    void
    setSpecial (const U & newItem)
    {
        // For internal use for items which aren't really demand-fetched

        item_p = newItem;
        setAsPresent();
        isDirty_p = False;
    }

    virtual void
    setDirty ()
    {
        isDirty_p = True;
    }

    virtual String
    shapeErrorMessage () const
    {
        Throw ("Scalar shapes should not have shape errors.");
    }

protected:

    void
    assign (T & lhs, const T & rhs)
    {
        lhs = rhs;
    }

    template <typename E>
    static void clearValue (Array <E> & value){
        value.resize();
    }

    static void clearValue (Int & value){
        value = 0;
    }

    static void clearValue (MDirection & value){
        value = MDirection ();
    }


//    virtual void
//    copy (const VbCacheItemBase * otherRaw, Bool markAsCached)
//    {
//        // Convert generic pointer to one pointint to this
//        // cache item type.
//
//        const VbCacheItem * other = dynamic_cast <const VbCacheItem *> (otherRaw);
//        Assert (other != 0);
//
//        // Capture the cached status of the other item
//
//        isPresent_p = other->isPresent_p;
//
//        // If the other item was cached then copy it over
//        // otherwise clear out this item.
//
//        if (isPresent_p){
//            item_p = other->item_p;
//        }
//        else {
//            item_p = T ();
//
//            if (markAsCached){
//                isPresent_p = True;
//            }
//        }
//    }

    virtual void
    copy (const VbCacheItemBase * otherRaw, Bool fetchIfNeeded)
    {
        const VbCacheItem<T, IsComputed> * other =
            dynamic_cast <const VbCacheItem<T, IsComputed> *> (otherRaw);
        copyAux (other, fetchIfNeeded);
    }

    void
    copyAux (const VbCacheItem<T, IsComputed> * other, bool fetchIfNeeded)
    {
        if (other->isPresent()){

            item_p = other->item_p;
            setAsPresent ();
            isDirty_p = False;
        }
        else if (fetchIfNeeded){
            set (other->get());
        }
        else {

            setAsPresent (False);
            isDirty_p = False;
        }
    }

    T &
    getItem () const
    {
        return item_p;
    }

    void
    setAsPresent (Bool isPresent = True) const
    {
        isPresent_p = isPresent;
    }

private:

    Filler       filler_p;
    mutable Bool isDirty_p;
    mutable Bool isPresent_p;
    mutable T    item_p;
};

template <typename T, Bool IsComputed = False>
class VbCacheItemArray : public VbCacheItem<T, IsComputed> {
public:

    typedef typename VbCacheItem<T>::Filler Filler;

    virtual ~VbCacheItemArray () {}



    virtual void copyRowElement (Int sourceRow, Int destinationRow)
    {
        copyRowElementAux (this->getItem(), sourceRow, destinationRow);
    }

    void
    initialize (VisBufferCache * cache,
                VisBufferImpl2 * vb,
                Filler filler,
                VisBufferComponent2 component,
                ShapePattern shapePattern,
                Bool isKey = True)
    {
        VbCacheItem<T, IsComputed>::initialize (cache, vb, filler, component, isKey);
        shapePattern_p = shapePattern;
    }


    virtual Bool
    isShapeOk () const
    {
        // Check to see if the shape of this data item is consistent
        // with the expected shape.

        Bool result = shapePattern_p == NoCheck ||
                      this->getItem().shape() == this->getVb()->getValidShape (shapePattern_p);

        return result;
    }


    Bool isArray () const
    {
        return True;
    }

    void
    resize (Bool copyValues)
    {
        if (shapePattern_p != NoCheck){

            IPosition desiredShape = this->getVb()->getValidShape (shapePattern_p);

            this->getItem().resize (desiredShape, copyValues);

            if (! copyValues){
                this->getItem() = typename T::value_type();
            }

        }
    }

    void
    resizeRows (Int newNRows)
    {
        IPosition shape = this->getItem().shape();

        if (shapePattern_p != NoCheck){

            // Change the last dimension to be the new number of rows,
            // then resize, copying values.

            shape (shape.nelements() - 1) = newNRows;

            this->getItem().resize (shape, True);

            this->setDirty();
        }
    }

    virtual void
    set (const T & newItem)
    {
        ThrowIf (! this->getVb()->isWritable (), "This VisBuffer is readonly");

        ThrowIf (this->isKey() && ! this->getVb()->isRekeyable (),
                 "This VisBuffer is does not allow row key values to be changed.");

        // Now check for a conformant shape.

        IPosition itemShape = newItem.shape();
        Bool parameterShapeOk = shapePattern_p == NoCheck ||
                                itemShape == this->getVb()->getValidShape (shapePattern_p);
        ThrowIf (! parameterShapeOk,
                 "Invalid parameter shape:: " + shapeErrorMessage (& itemShape));

        VbCacheItem<T,IsComputed>::set (newItem);
    }

    template <typename U>
    void
    set (const U & newItem)
    {
        VbCacheItem<T,IsComputed>::set (newItem);
    }

    virtual String
    shapeErrorMessage (const IPosition * badShape = 0) const
    {

        ThrowIf (shapePattern_p == NoCheck,
                 "No shape error message for NoCheck type array");

        ThrowIf (isShapeOk () && badShape == 0,
                 "Shape is OK so no error message.");

        String badShapeString = (badShape != 0) ? badShape->toString()
                                                : this->getItem().shape().toString();

        ostringstream os;

        os << "VisBuffer::ShapeError: "
           << VisBufferComponents2::name (this->getComponent())
           << " should have shape "
           << this->getVb()->getValidShape(shapePattern_p).toString()
           << " but had shape "
           << badShapeString;

        return os.str();
    }

protected:

    void
    assign (T & dst, const T & src)
    {
        dst.assign (src);
    }

    static void
    copyRowElementAux (Cube<typename T::value_type> & cube, Int sourceRow, Int destinationRow)
    {
        IPosition shape = cube.shape();
        Int nI = shape(1);
        Int nJ = shape(0);

        for (Int i = 0; i < nI; i++){
            for (Int j = 0; j < nJ; j++){
                cube (j, i, destinationRow) = cube (j, i, sourceRow);
            }
        }
    }

    static void
    copyRowElementAux (Matrix<typename T::value_type> & matrix, Int sourceRow, Int destinationRow)
    {
        IPosition shape = matrix.shape();
        Int nJ = shape(0);

        for (Int j = 0; j < nJ; j++){
            matrix (j, destinationRow) = matrix (j, sourceRow);
        }
    }

    static void
    copyRowElementAux (Array<typename T::value_type> & array, Int sourceRow, Int destinationRow)
    {
        IPosition shape = array.shape();
        Assert (shape.nelements() == 4);

        Int nH = shape(2);
        Int nI = shape(1);
        Int nJ = shape(0);

        for (Int h = 0; h < nH; h++){
            for (Int i = 0; i < nI; i++){
                for (Int j = 0; j < nJ; j++){
                    array (IPosition (4, j, i, h, destinationRow)) =
                        array (IPosition (4, j, i, h, sourceRow));
                }
            }
        }
    }

    static void
    copyRowElementAux (Vector<typename T::value_type> & vector, Int sourceRow, Int destinationRow)
    {
        vector (destinationRow) = vector (sourceRow);
    }

private:

    ShapePattern shapePattern_p;
};

class VisBufferCache {

    // Holds the cached values for a VisBuffer object.

public:

    VisBufferCache (VisBufferImpl2 * vb);
    void initialize (VisBufferImpl2 * vb);
    void registerItem (VbCacheItemBase * item);

    // The values that are potentially cached.

    VbCacheItemArray <Vector<Int> > antenna1_p;
    VbCacheItemArray <Vector<Int> > antenna2_p;
    VbCacheItemArray <Vector<Int> > arrayId_p;
    VbCacheItemArray <Vector<SquareMatrix<Complex, 2> >, True> cjones_p;
    VbCacheItemArray <Cube<Complex> > correctedVisCube_p;
//    VbCacheItemArray <Matrix<CStokesVector> > correctedVisibility_p;
    VbCacheItemArray <Vector<Int> > corrType_p;
    VbCacheItem <Int> dataDescriptionId_p;
    VbCacheItemArray <Vector<Int> > dataDescriptionIds_p;
    VbCacheItemArray <Vector<MDirection> > direction1_p; //where the first antenna/feed is pointed to
    VbCacheItemArray <Vector<MDirection> > direction2_p; //where the second antenna/feed is pointed to
    VbCacheItemArray <Vector<Double> > exposure_p;
    VbCacheItemArray <Vector<Int> > feed1_p;
    VbCacheItemArray <Vector<Float> > feed1Pa_p;
    VbCacheItemArray <Vector<Int> > feed2_p;
    VbCacheItemArray <Vector<Float> > feed2Pa_p;
    VbCacheItemArray <Vector<Int> > fieldId_p;
//    VbCacheItemArray <Matrix<Bool> > flag_p;
    VbCacheItemArray <Array<Bool> > flagCategory_p;
    VbCacheItemArray <Cube<Bool> > flagCube_p;
    VbCacheItemArray <Vector<Bool> > flagRow_p;
    VbCacheItemArray <Cube<Float> > floatDataCube_p;
    VbCacheItemArray <Matrix<Float> > imagingWeight_p;
    VbCacheItemArray <Cube<Complex> > modelVisCube_p;
//    VbCacheItemArray <Matrix<CStokesVector> > modelVisibility_p;
    VbCacheItem <Int> nAntennas_p;
    VbCacheItem <Int> nChannels_p;
    VbCacheItem <Int> nCorrelations_p;
    VbCacheItem <Int> nRows_p;
    VbCacheItemArray <Vector<Int> > observationId_p;
    VbCacheItem <MDirection> phaseCenter_p;
    VbCacheItem <Int> polFrame_p;
    VbCacheItem <Int> polarizationId_p;
    VbCacheItemArray <Vector<Int> > processorId_p;
    VbCacheItemArray <Vector<uInt> > rowIds_p;
    VbCacheItemArray <Vector<Int> > scan_p;
    VbCacheItemArray <Matrix<Float> > sigma_p;
    //VbCacheItemArray <Matrix<Float> > sigmaMat_p;
    VbCacheItemArray <Vector<Int> > spectralWindows_p;
    VbCacheItemArray <Vector<Int> > stateId_p;
    VbCacheItemArray <Vector<Double> > time_p;
    VbCacheItemArray <Vector<Double> > timeCentroid_p;
    VbCacheItemArray <Vector<Double> > timeInterval_p;
    VbCacheItemArray <Matrix<Double> > uvw_p;
    VbCacheItemArray <Cube<Complex> > visCube_p;
//    VbCacheItemArray <Matrix<CStokesVector> > visibility_p;
    VbCacheItemArray <Matrix<Float> > weight_p;
    //VbCacheItemArray <Matrix<Float> > weightMat_p;
    VbCacheItemArray <Cube<Float> > weightSpectrum_p;

    CacheRegistry registry_p;

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

    template<typename T>
    class FrequencyCache {
    public:

        typedef Vector<T> (VisibilityIterator2::* Updater) (Double, Int) const;

        FrequencyCache (Updater updater) : updater_p (updater) {}

        Int frame_p;
        Double time_p;
        Updater updater_p;
        Vector<T> values_p;

        void
        flush ()
        {
            time_p = -1;
        }

        void
        updateCacheIfNeeded (const VisibilityIterator2 * rovi, Double time,
                             Int frame = VisBuffer2::FrameNotSpecified)
        {
            if (time == time_p && frame == frame_p){
                return;
            }

            time_p = time;
            frame_p = frame;

            values_p.assign ((rovi ->* updater_p) (time_p, frame_p));
        }
    };

    VisBufferState ()
    : areCorrelationsSorted_p (False),
      channelNumbers_p (& VisibilityIterator2::getChannels),
      dirtyComponents_p (),
      frequencies_p (& VisibilityIterator2::getFrequencies),
      isAttached_p (False),
      isFillable_p (False),
      isNewMs_p (False),
      isNewArrayId_p (False),
      isNewFieldId_p (False),
      isNewSpectralWindow_p (False),
      isRekeyable_p (False),
      isWritable_p (False),
      pointingTableLastRow_p (-1),
      validShapes_p (N_ShapePatterns),
      vi_p (0),
      viC_p (0),
      visModelData_p (),
      weightScaling_p (0)
    {}

    Bool areCorrelationsSorted_p; // Have correlations been sorted by sortCorr?
    FrequencyCache<Int> channelNumbers_p;
    Vector<Int> correlations_p;
    VisBufferComponents2 dirtyComponents_p;
    FrequencyCache<Double> frequencies_p;
    Bool isAttached_p;
    Bool isFillable_p;
    Bool isNewMs_p;
    Bool isNewArrayId_p;
    Bool isNewFieldId_p;
    Bool isNewSpectralWindow_p;
    Bool isRekeyable_p;
    Bool isWritable_p;
    mutable Int pointingTableLastRow_p;
    Int msId_p;
    String msName_p;
    Bool newMs_p;
    Subchunk subchunk_p;
    Vector<IPosition> validShapes_p;
    VisibilityIterator2 * vi_p; // [use]
    const VisibilityIterator2 * viC_p; // [use]
    mutable VisModelData visModelData_p;
    CountedPtr <WeightScaling> weightScaling_p;
};


VisBufferCache::VisBufferCache (VisBufferImpl2 * vb)
{
    ThrowIf (vb == NULL, "VisBufferCacheImpl not connected to VisBufferImpl2");
}

void
VisBufferCache::initialize (VisBufferImpl2 * vb)
{

    // Initialize the cache items.  This will also register them
    // with the vb object to allow it to iterate over the cached
    // values.

    antenna1_p.initialize (this, vb, & VisBufferImpl2::fillAntenna1, Antenna1, Nr);
    antenna2_p.initialize (this, vb, &VisBufferImpl2::fillAntenna2, Antenna2, Nr);
    arrayId_p.initialize (this, vb, &VisBufferImpl2::fillArrayId, ArrayId, Nr);
    cjones_p.initialize (this, vb, &VisBufferImpl2::fillJonesC, JonesC, NoCheck);
    correctedVisCube_p.initialize (this, vb, &VisBufferImpl2::fillCubeCorrected,
                                   VisibilityCubeCorrected, NcNfNr, False);
//    correctedVisibility_p.initialize (this, vb, &VisBufferImpl2::fillVisibilityCorrected,
//                                      VisibilityCorrected, NcNfNr, False);
    corrType_p.initialize (this, vb, &VisBufferImpl2::fillCorrType, CorrType, NoCheck);
    //dataDescriptionId_p.initialize (this, vb, &VisBufferImpl2::fillDataDescriptionId, DataDescriptionId);
    dataDescriptionIds_p.initialize (this, vb, &VisBufferImpl2::fillDataDescriptionIds, DataDescriptionIds, Nr);
    direction1_p.initialize (this, vb, &VisBufferImpl2::fillDirection1, Direction1, NoCheck);
    direction2_p.initialize (this, vb, &VisBufferImpl2::fillDirection2, Direction2, NoCheck);
    exposure_p.initialize (this, vb, &VisBufferImpl2::fillExposure, Exposure, Nr);
    feed1_p.initialize (this, vb, &VisBufferImpl2::fillFeed1, Feed1, Nr);
    feed1Pa_p.initialize (this, vb, &VisBufferImpl2::fillFeedPa1, FeedPa1, NoCheck);
    feed2_p.initialize (this, vb, &VisBufferImpl2::fillFeed2, Feed2, Nr);
    feed2Pa_p.initialize (this, vb, &VisBufferImpl2::fillFeedPa2, FeedPa2, NoCheck);
    fieldId_p.initialize (this, vb, &VisBufferImpl2::fillFieldId, FieldId, Nr);
    //flag_p.initialize (this, vb, &VisBufferImpl2::fillFlag, Flag, NoCheck, False);
    flagCategory_p.initialize (this, vb, &VisBufferImpl2::fillFlagCategory, FlagCategory, NoCheck, False);
        // required column but not used in casa, make it a nocheck for shape validation
    flagCube_p.initialize (this, vb, &VisBufferImpl2::fillFlagCube, FlagCube, NcNfNr, False);
    flagRow_p.initialize (this, vb, &VisBufferImpl2::fillFlagRow, FlagRow, Nr, False);
    floatDataCube_p.initialize (this, vb, &VisBufferImpl2::fillFloatData, FloatData, NcNfNr, False);
    imagingWeight_p.initialize (this, vb, &VisBufferImpl2::fillImagingWeight, ImagingWeight, NoCheck);
    modelVisCube_p.initialize (this, vb, &VisBufferImpl2::fillCubeModel, VisibilityCubeModel, NcNfNr, False);
//    modelVisibility_p.initialize (this, vb, &VisBufferImpl2::fillVisibilityModel, VisibilityModel, NoCheck, False);
    nAntennas_p.initialize (this, vb, &VisBufferImpl2::fillNAntennas, NAntennas);
    nChannels_p.initialize (this, vb, &VisBufferImpl2::fillNChannel, NChannels);
    nCorrelations_p.initialize (this, vb, &VisBufferImpl2::fillNCorr, NCorrelations);
    nRows_p.initialize (this, vb, &VisBufferImpl2::fillNRow, NRows);
    observationId_p.initialize (this, vb, &VisBufferImpl2::fillObservationId, ObservationId, Nr);
    phaseCenter_p.initialize (this, vb, &VisBufferImpl2::fillPhaseCenter, PhaseCenter);
    polFrame_p.initialize (this, vb, &VisBufferImpl2::fillPolFrame, PolFrame);
    polarizationId_p.initialize (this, vb, &VisBufferImpl2::fillPolarizationId, PolarizationId);
    processorId_p.initialize (this, vb, &VisBufferImpl2::fillProcessorId, ProcessorId, Nr);
    rowIds_p.initialize (this, vb, &VisBufferImpl2::fillRowIds, RowIds, Nr);
    scan_p.initialize (this, vb, &VisBufferImpl2::fillScan, Scan, Nr);
    sigma_p.initialize (this, vb, &VisBufferImpl2::fillSigma, Sigma, NcNr, False);
    //sigmaMat_p.initialize (this, vb, &VisBufferImpl2::fillSigmaMat, SigmaMat);
    spectralWindows_p.initialize (this, vb, &VisBufferImpl2::fillSpectralWindows, SpectralWindows, Nr);
    stateId_p.initialize (this, vb, &VisBufferImpl2::fillStateId, StateId, Nr);
    time_p.initialize (this, vb, &VisBufferImpl2::fillTime, casa::vi::Time, Nr);
    timeCentroid_p.initialize (this, vb, &VisBufferImpl2::fillTimeCentroid, TimeCentroid, Nr);
    timeInterval_p.initialize (this, vb, &VisBufferImpl2::fillTimeInterval, TimeInterval, Nr);
    uvw_p.initialize (this, vb, &VisBufferImpl2::fillUvw, Uvw, I3Nr);
    visCube_p.initialize (this, vb, &VisBufferImpl2::fillCubeObserved, VisibilityCubeObserved, NcNfNr, False);
    //visibility_p.initialize (this, vb, &VisBufferImpl2::fillVisibilityObserved, VisibilityObserved, NoCheck, False);
    weight_p.initialize (this, vb, &VisBufferImpl2::fillWeight, Weight, NcNr, False);
    weightSpectrum_p.initialize (this, vb, &VisBufferImpl2::fillWeightSpectrum,
                                 WeightSpectrum, NcNfNr, False);

}

void
VisBufferCache::registerItem (VbCacheItemBase * item)
{
    registry_p.push_back (item);
}

void
VbCacheItemBase::initialize (VisBufferCache * cache, VisBufferImpl2 * vb, VisBufferComponent2 component, Bool isKey)
{
    vbComponent_p = component;
    vb_p = vb;
    cache->registerItem (this);
    setIsKey (isKey);
}


using namespace vi;

////////////////////////////////////////////////////////////
//
// Basic VisBufferImpl2 Methods
// ========================
//
// Other sections contain the accessor and filler methods

VisBufferImpl2::VisBufferImpl2 (VisBufferOptions options)
: cache_p (0), state_p (0)
{
    construct (0, options);
}

VisBufferImpl2::VisBufferImpl2(VisibilityIterator2 * iter, VisBufferOptions options)
: cache_p (0),
  state_p (0)
{
  construct (iter, options);
}

VisBufferImpl2::~VisBufferImpl2 ()
{
    delete cache_p;
    delete state_p;
}

Bool
VisBufferImpl2::areCorrelationsInCanonicalOrder () const
{
  Vector<Int> corrs(correlationTypes());

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
VisBufferImpl2::associateWithVisibilityIterator2 (const VisibilityIterator2 & vi)
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

void
VisBufferImpl2::cacheClear (Bool markAsCached)
{
    for (CacheRegistry::iterator i = cache_p->registry_p.begin();
         i != cache_p->registry_p.end();
         i++){

        (*i)->clear (false);

        if (markAsCached){
            (*i)->setAsPresent ();
        }
    }
}

void
VisBufferImpl2::cacheResizeAndZero (const VisBufferComponents2 & exclusions)
{
    for (CacheRegistry::iterator i = cache_p->registry_p.begin();
         i != cache_p->registry_p.end();
         i++){

        if (exclusions.contains((*i)->getComponent())){
            continue; // this one is excluded
        }

        //


       (*i)->resize (False);

       (*i)->setAsPresent();
    }
}



void
VisBufferImpl2::construct (VisibilityIterator2 * vi, VisBufferOptions options)
{
    state_p = new VisBufferState ();

    // Initialize all non-object member variables

    state_p->areCorrelationsSorted_p = False; // Have correlations been sorted by sortCorr?

    state_p->isAttached_p = vi != 0;
    state_p->pointingTableLastRow_p = -1;
    state_p->newMs_p = True;
    state_p->vi_p = vi;
    state_p->viC_p = vi;

    // Handle the options

    state_p->isWritable_p = False;
    state_p->isRekeyable_p = False;

    if (options & VbRekeyable){

        state_p->isWritable_p = True; // rekeyable implies writable
        state_p->isRekeyable_p = True;
    }
    else if (options & VbWritable){
        state_p->isWritable_p = True;
    }

    cache_p = new VisBufferCache (this);
    cache_p->initialize (this);
}

void
VisBufferImpl2::copy (const VisBuffer2 & otherRaw, Bool fetchIfNeeded)
{
    copyComponents (otherRaw, VisBufferComponents2::all(), True, fetchIfNeeded);
}

void
VisBufferImpl2::copyComponents (const VisBuffer2 & otherRaw,
                                const VisBufferComponents2 & components,
                                Bool allowShapeChange,
                                Bool fetchIfNeeded)
{
    const VisBufferImpl2 * other = dynamic_cast<const VisBufferImpl2 *> (& otherRaw);

    ThrowIf (other == 0,
             String::format ("Copy between %s and VisBufferImpl2 not implemented.",
                             typeid (otherRaw).name()));

    if (! hasShape()){

        // If this VB is shapeless, then assume the shape of the source VB.

        setShape (otherRaw.nCorrelations(), otherRaw.nChannels(), otherRaw.nRows(), True);
    }
    else if (allowShapeChange && getShape() != otherRaw.getShape()){

        setShape (otherRaw.nCorrelations(), otherRaw.nChannels(), otherRaw.nRows(), True);
    }

    Bool wasFillable = isFillable();
    setFillable (True);

    for (CacheRegistry::iterator dst = cache_p->registry_p.begin(),
                                 src = other->cache_p->registry_p.begin();
         dst != cache_p->registry_p.end();
         dst ++, src ++){

        if (components.contains ((* src)->getComponent())){

            try {

                (* dst)->copy (* src, fetchIfNeeded);
            }
            catch (AipsError & e){

                String m =  String::format ("While copying %s",
                                            VisBufferComponents2::name ((* src)->getComponent()).c_str());
                Rethrow (e, m);
            }
        }
    }

    setFillable (wasFillable);
}

void
VisBufferImpl2::copyCoordinateInfo (const VisBuffer2 * vb, Bool dirDependent,
                                    Bool allowShapeChange, Bool fetchIfNeeded)
{

    VisBufferComponents2 components =
        VisBufferComponents2::these (Antenna1, Antenna2, ArrayId, DataDescriptionIds,
                                     FieldId, SpectralWindows, casa::vi::Time,
                                     NRows, Feed1, Feed2, Unknown);

    copyComponents (* vb, components, allowShapeChange, fetchIfNeeded);

    setIterationInfo (vb->msId(),
                      vb->msName (),
                      vb->isNewMs (),
                      vb->isNewArrayId (),
                      vb->isNewFieldId (),
                      vb->isNewSpectralWindow (),
                      vb->getSubchunk (),
                      vb->getCorrelationNumbers(),
                      vb->getWeightScaling());

    if(dirDependent){

        VisBufferComponents2 components =
                VisBufferComponents2::these (Direction1, Direction2, FeedPa1, FeedPa2, Unknown);

        copyComponents (* vb, components, allowShapeChange, fetchIfNeeded);
    }
}

void
VisBufferImpl2::copyRow (Int sourceRow, Int destinationRow)
{
    for (CacheRegistry::iterator i = cache_p->registry_p.begin();
         i != cache_p->registry_p.end();
         i++){

        if (! (* i)->isPresent()){
            continue;
        }

        (* i)->copyRowElement (sourceRow, destinationRow);

    }

}

void
VisBufferImpl2::deleteRows (const Vector<Int> & rowsToDelete)
{

    // Deletes the specified rows.  This involves copying the undeleted
    // rows down to fill in the holes left by the deletions.  The current
    // approach preserves the order of the undeleted rows; however, this
    // causes all rows after the first deletion to be copied.  If preserving
    // the order is not important then any deletion could be filled by copying
    // a row from the end of the VB into the holes.  For the moment the simpler
    // approach is implemented.

    if (rowsToDelete.empty()){
        return;
    }

    Int deleteIndex = 0;
    Int rowToDelete = rowsToDelete (0);
    Int destinationRow = 0;

    for (Int sourceRow = 0; sourceRow < nRows(); sourceRow ++){

        if (sourceRow == rowToDelete){

            // This row is being deleted so don't copy it down.
            // Advance the deletion index and get the next row
            // to be deleted.

            deleteIndex ++;

            if (deleteIndex < (int) rowsToDelete.nelements()){

                Int oldRowToDelete = rowToDelete;

                rowToDelete = rowsToDelete (deleteIndex);

                Assert (oldRowToDelete < rowToDelete);
            }
        }
        else if (destinationRow != sourceRow){

            // Copy this row down to the next empty row.  Advance
            // the destination row.

            copyRow (sourceRow, destinationRow);

            destinationRow ++;
        }
        else{
            destinationRow ++;
        }
    }

    Int newNRows = nRows() - rowsToDelete.nelements();

    setShape (nCorrelations (), nChannels (), newNRows, False);

    resizeRows (newNRows);
}

void
VisBufferImpl2::resizeRows (Int newNRows)
{

    // Resize each member of the cache to use the new number of rows.
    // The resizing will preserve the unaffected data values.

    for (CacheRegistry::iterator i = cache_p->registry_p.begin();
         i != cache_p->registry_p.end();
         i++){

        if (! (* i)->isPresent()){
            continue;
        }

        (* i)->resizeRows (newNRows);
    }
}

void
VisBufferImpl2::dirtyComponentsAdd (const VisBufferComponents2 & additionalDirtyComponents)
{
    // Loop through all of the cached VB components and mark them dirty if
    // they're in the set of addition dirty components

    for (CacheRegistry::iterator i = cache_p->registry_p.begin();
         i != cache_p->registry_p.end();
         i++){

        if (additionalDirtyComponents.contains ((* i)->getComponent())){

            (* i)->setDirty ();
        }
    }
}

void
VisBufferImpl2::dirtyComponentsAdd (VisBufferComponent2 component)
{
    dirtyComponentsAdd (VisBufferComponents2::singleton (component));
}


void
VisBufferImpl2::dirtyComponentsClear ()
{
    for (CacheRegistry::iterator i = cache_p->registry_p.begin();
         i != cache_p->registry_p.end();
         i++){

        (* i)->clearDirty ();
    }
}

VisBufferComponents2
VisBufferImpl2::dirtyComponentsGet () const
{
    VisBufferComponents2 dirtyComponents;

    for (CacheRegistry::iterator i = cache_p->registry_p.begin();
         i != cache_p->registry_p.end();
         i++){

        if ((*i)->isDirty ()){

            VisBufferComponent2 component = (* i)->getComponent ();

            if (component != Unknown){
                dirtyComponents = dirtyComponents +
                                  VisBufferComponents2::singleton (component);
            }
        }
    }

    return dirtyComponents;
}

void
VisBufferImpl2::dirtyComponentsSet (const VisBufferComponents2 & dirtyComponents)
{
    // Clear the dirty state for the cache

    cacheClear ();

    // Add in the specified components to the newly cleared cache.

    dirtyComponentsAdd (dirtyComponents);
}

void
VisBufferImpl2::dirtyComponentsSet (VisBufferComponent2 component)
{
    dirtyComponentsSet (VisBufferComponents2::singleton (component));
}


Bool
VisBufferImpl2::isAttached () const
{
    return state_p->isAttached_p;
}

Bool
VisBufferImpl2::isFillable () const
{
    return state_p->isFillable_p;
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

Double
VisBufferImpl2::getFrequency (Int rowInBuffer, Int frequencyIndex, Int frame) const
{
    if (frame == FrameNotSpecified){
        frame = getVi()->getReportingFrameOfReference();
    }

    Double t = time() (rowInBuffer);

    state_p->frequencies_p.updateCacheIfNeeded (getVi(), t, frame);

    return state_p->frequencies_p.values_p (frequencyIndex);
}

const Vector<Double> &
VisBufferImpl2::getFrequencies (Int rowInBuffer, Int frame) const
{
    if (frame == FrameNotSpecified){
        frame = getVi()->getReportingFrameOfReference();
    }

    Double t = time() (rowInBuffer);

    state_p->frequencies_p.updateCacheIfNeeded (getVi(), t, frame);

    return state_p->frequencies_p.values_p;
}

Int
VisBufferImpl2::getChannelNumber (Int rowInBuffer, Int frequencyIndex) const
{
    Double t = time() (rowInBuffer);

    state_p->channelNumbers_p.updateCacheIfNeeded (getVi(), t, 0);

    return state_p->channelNumbers_p.values_p (frequencyIndex);
}

const Vector<Int> &
VisBufferImpl2::getChannelNumbers (Int rowInBuffer) const
{
    Double t = time() (rowInBuffer);

    state_p->channelNumbers_p.updateCacheIfNeeded (getVi(), t, 0);

    return state_p->channelNumbers_p.values_p;
}

Vector<Int>
VisBufferImpl2::getCorrelationNumbers () const
{
    return state_p->correlations_p;
}


String
VisBufferImpl2::getFillErrorMessage () const
{
    return "VisBuffer not positioned to a subchunk";
}


Subchunk
VisBufferImpl2::getSubchunk () const
{
    return state_p->subchunk_p;
}

IPosition
VisBufferImpl2::getValidShape (Int i) const
{
    ThrowIf (i < 0 || i >= (int) state_p->validShapes_p.nelements(),
             String::format ("Invalid shape requested: %d", i));

    return state_p->validShapes_p (i);
}

const VisibilityIterator2 *
VisBufferImpl2::getVi () const
{
    return state_p->viC_p;
}

VisibilityIterator2 *
VisBufferImpl2::getViP () const
{
    return state_p->vi_p;
}

VisModelData
VisBufferImpl2::getVisModelData () const
{
    return state_p->visModelData_p;
}

IPosition
VisBufferImpl2::getShape () const
{
    IPosition shape;

    if (hasShape()){
        shape = IPosition (3, cache_p->nCorrelations_p.get(), cache_p->nChannels_p.get(),
                           cache_p->nRows_p.get());
    }
    else{
        shape = IPosition ();
    }

    return shape;
}


Bool
VisBufferImpl2::hasShape () const
{
    Bool hasShape = cache_p->nCorrelations_p.isPresent() && cache_p->nCorrelations_p.get() > 0;
    hasShape = hasShape && cache_p->nChannels_p.isPresent() && cache_p->nChannels_p.get() > 0;
    hasShape = hasShape && cache_p->nRows_p.isPresent() && cache_p->nRows_p.get() > 0;

    return hasShape;
}

void
VisBufferImpl2::invalidate ()
{
    cacheClear (False); // empty cached values
    setFillable (False); // buffer is in limbo
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
VisBufferImpl2::isRekeyable () const
{
    return state_p->isRekeyable_p;
}


Bool
VisBufferImpl2::isWritable () const
{
    return state_p->isWritable_p;
}

void
VisBufferImpl2::normalize()
{
    // NB: Handles pol-dep weights in chan-indep way

    // Check for missing data

    ThrowIf (! cache_p->visCube_p.isPresent(),
             "Cannot normalize; visCube is missing.");
    ThrowIf (! cache_p->modelVisCube_p.isPresent (),
             "Cannot normalize; modelVisCube is missing.");
    ThrowIf (! cache_p->weight_p.isPresent(),
             "Cannot normalize; weightMap is missing.");

    // Get references to the cached values to be used in the
    // normalization.

    Int nCorrelations = this->nCorrelations ();

    const Vector<Bool> & rowFlagged = cache_p->flagRow_p.get ();
    const Cube<Bool> & flagged = cache_p->flagCube_p.get ();

    Cube<Complex> & visCube = cache_p->visCube_p.getRef();
    Cube<Complex> & modelCube = cache_p->modelVisCube_p.getRef();
    Matrix<Float> & weight = cache_p->weight_p.getRef();

    // Normalize each row.

    for (Int row = 0; row < nRows (); row++) {

        if (rowFlagged (row)){

            weight.column(row) = 0.0f; // Zero weight on this flagged row
            continue;
        }

        normalizeRow (row, nCorrelations, flagged, visCube,
                      modelCube, weight);

    }
}

void
VisBufferImpl2::normalizeRow (Int row, Int nCorrelations, const Cube<Bool> & flagged,
                              Cube<Complex> & visCube, Cube<Complex> & modelCube,
                              Matrix<Float> & weightMat)
{
    Vector<Float> amplitudeSum = Vector<Float> (nCorrelations, 0.0f);
    Vector<Int> count = Vector<Int> (nCorrelations, 0);

    for (Int channel = 0; channel < nChannels (); channel ++) {

        for (Int correlation = 0; correlation < nCorrelations; correlation ++) {

            if (flagged (correlation, channel, row)){
                continue;  // data is flagged so skip over it
            }

            // If the model amplitude is zero, zero out the observed value.
            // Otherwise divide the observed value by the model value and
            // set the model value to 1.0.

            Float amplitude = abs (modelCube(correlation, channel, row));

            if (amplitude <= 0.0f) { // zero data if model is zero

                visCube (correlation, channel, row) = 0.0;
                continue;
            }

            // Normalize visibility datum by corresponding model data point.

            DComplex vis = visCube(correlation, channel, row);
            DComplex mod = modelCube(correlation, channel, row);

            visCube (correlation, channel, row) = Complex (vis / mod);
            modelCube (correlation, channel, row) = Complex(1.0);

            // Count and sum up the nonzero model amplitudes for this correlation.

            amplitudeSum (correlation) += amplitude;
            count (correlation) ++;
        }
    }

    // Adjust the weight matrix entries for this row appropriately

    for (Int correlation = 0; correlation < nCorrelations; correlation++) {

        if (count (correlation) > 0) {

            weightMat (correlation, row) *= square (amplitudeSum (correlation) /
                                                    count (correlation));
        }
        else {

            weightMat (correlation, row) = 0.0f;
        }
    }
}

void
VisBufferImpl2::registerCacheItem (VbCacheItemBase * item)
{
    cache_p->registry_p.push_back (item);
}

void
VisBufferImpl2::resetWeightsUsingSigma ()
{
    const Matrix <Float> & sigma = this->sigma ();

    IPosition ip (sigma.shape());

    Matrix <Float> & weight = cache_p->weight_p.getRef ();
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

    Int nchan = getViP()->subtableColumns().spectralWindow().numChan()(spectralWindows()(0));

    weight *= Float(nchan);

    cache_p->weight_p.setAsPresent ();
}

void
VisBufferImpl2::setIterationInfo (Int msId,
                                  const String & msName,
                                  Bool isNewMs,
                                  Bool isNewArrayId,
                                  Bool isNewFieldId,
                                  Bool isNewSpectralWindow,
                                  const Subchunk & subchunk,
                                  const Vector<Int> & correlations,
                                  CountedPtr <WeightScaling> weightScaling)
{
    // Set the iteration attributes into this VisBuffer

    state_p->msId_p = msId;
    state_p->msName_p = msName;
    state_p->isNewMs_p = isNewMs;
    state_p->isNewMs_p = isNewMs;
    state_p->isNewArrayId_p = isNewArrayId;
    state_p->isNewFieldId_p = isNewFieldId;
    state_p->isNewSpectralWindow_p = isNewSpectralWindow;
    state_p->subchunk_p = subchunk;
    state_p->correlations_p.assign (correlations);
    state_p->weightScaling_p = weightScaling;
}

void
VisBufferImpl2::setFillable (Bool isFillable)
{
    state_p->isFillable_p = isFillable;
}



void
VisBufferImpl2::configureNewSubchunk (Int msId,
                                      const String & msName,
                                      Bool isNewMs,
                                      Bool isNewArrayId,
                                      Bool isNewFieldId,
                                      Bool isNewSpectralWindow,
                                      const Subchunk & subchunk,
                                      Int nRows,
                                      Int nChannels,
                                      Int nCorrelations,
                                      const Vector<Int> & correlations,
                                      CountedPtr<WeightScaling> weightScaling)
{
    // Prepare this VisBuffer for the new subchunk

    cacheClear();

    setIterationInfo (msId, msName, isNewMs, isNewArrayId, isNewFieldId,
                      isNewSpectralWindow, subchunk, correlations, weightScaling);

    setFillable (True); // New subchunk, so it's fillable

    state_p->frequencies_p.flush();
    state_p->channelNumbers_p.flush();

    cache_p->nRows_p.setSpecial (nRows);
    cache_p->nChannels_p.setSpecial (nChannels);
    cache_p->nCorrelations_p.setSpecial (nCorrelations);

    setupValidShapes ();
}

void
VisBufferImpl2::setRekeyable (Bool isRekeyable)
{
    state_p->isRekeyable_p = isRekeyable;
}

void
VisBufferImpl2::setShape (Int nCorrelations, Int nChannels, Int nRows,
                          Bool clearTheCache)
{
    ThrowIf (hasShape() && ! isRekeyable(),
             "Operation setShape is illegal on nonrekeyable VisBuffer");

    if (clearTheCache){
        cacheClear (False); // leave values alone so that array could be reused
    }

    cache_p->nCorrelations_p.setSpecial(nCorrelations);
    cache_p->nChannels_p.setSpecial(nChannels);
    cache_p->nRows_p.setSpecial(nRows);

    setupValidShapes ();
}

void
VisBufferImpl2::setupValidShapes ()
{
    state_p->validShapes_p [Nr] = IPosition (1, nRows());
    state_p->validShapes_p [NfNr] = IPosition (2, nChannels(), nRows());
    state_p->validShapes_p [NcNr] = IPosition (2, nCorrelations(), nRows());
    state_p->validShapes_p [NcNfNr] = IPosition (3, nCorrelations(), nChannels(), nRows());
    state_p->validShapes_p [I3Nr] = IPosition (2, 3, nRows());
    //state_p->validShapes [NcNfNcatNr] = IPosition (4, nCorrelations(), nChannels(), nCategories(), nRows());
    //   flag_category is not used in CASA, so no need to implement checking.
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

    // Sort if nominal order is non-canonical (only for nCorr=4).
    // Also do not sort or unsort if that is the current sort state.

    if (! areCorrelationsInCanonicalOrder() &&
        (state_p->areCorrelationsSorted_p != makeSorted)) {

        // First sort the weights

        weight();    // (ensures it is filled)

        Vector<Float> wtmp(nRows ());
        Vector<Float> w1, w2, w3;
        IPosition wblc (1, 0, 0);
        IPosition wtrc (2, 0, nRows () - 1);
        IPosition vec (1, nRows ());

        VisBufferCache::sortCorrelationItem (cache_p->weight_p, wblc, wtrc, vec, wtmp, makeSorted);

        // Now sort the complex data cubes

        Matrix<Complex> tmp(nChannels (), nRows ());
        Matrix<Complex> p1, p2, p3;
        IPosition blc(3, 0, 0, 0);
        IPosition trc(3, 0, nChannels () - 1, nRows () - 1);
        IPosition mat(2, nChannels (), nRows ());

        // Sort the various visCubes, if present

        VisBufferCache::sortCorrelationItem (cache_p->visCube_p, blc, trc, mat, tmp, makeSorted);

        VisBufferCache::sortCorrelationItem (cache_p->modelVisCube_p, blc, trc, mat, tmp, makeSorted);

        VisBufferCache::sortCorrelationItem (cache_p->correctedVisCube_p, blc, trc, mat, tmp, makeSorted);

        // Finally sort the float data cube

        Matrix<Float> tmp2 (nChannels (), nRows ());

        VisBufferCache::sortCorrelationItem (cache_p->floatDataCube_p, blc, trc, mat, tmp2, makeSorted);

        // Record the sort state

        state_p->areCorrelationsSorted_p = makeSorted;
    }

}

// Sort correlations: (PP,QQ,PQ,QP) -> (PP,PQ,QP,QQ)
void
VisBufferImpl2::sortCorr()
{
    sortCorrelationsAux (True);
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

void
VisBufferImpl2::validateShapes () const
{
    // Check that all of the modified array columns have the proper shape.
    // Throw an exception if any columns have improper shape.  This is
    // intended to be a last chance sanity-check before the data is written to disk
    // or made available for use.

    String message;

    for (CacheRegistry::iterator i = cache_p->registry_p.begin();
         i != cache_p->registry_p.end();
         i++){

        if ((*i)->isDirty() && ! (*i)->isShapeOk ()){

            message += (*i)->shapeErrorMessage () + "\n";
        }
    }

    ThrowIf (! message.empty(), message);
}

void
VisBufferImpl2::writeChangesBack ()
{
    ThrowIf (! state_p->isAttached_p,
             "Call to writeChangesBack on unattached VisBuffer.");

    VisibilityIterator2 * rwvi = dynamic_cast <VisibilityIterator2 *> (getViP());

    ThrowIf (rwvi == 0, "Can't write to a read-only VisibilityIterator.");

    if (isRekeyable()){

        validateShapes ();
    }

    rwvi->writeBackChanges (this);
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

const Vector<MDirection> &
VisBufferImpl2::azel(Double time) const
{
  return getViP()->azel(time);
}

const Vector<Float> &
VisBufferImpl2::feedPa(Double time) const
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

const Vector<Float> &
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

void
VisBufferImpl2::setAntenna1 (const Vector<Int> & value)
{
    cache_p->antenna1_p.set (value);
}

const Vector<Int> &
VisBufferImpl2::antenna2 () const
{
    return cache_p->antenna2_p.get ();
}

void
VisBufferImpl2::setAntenna2 (const Vector<Int> & value)
{
    cache_p->antenna2_p.set (value);
}

const Vector<Int> &
VisBufferImpl2::arrayId () const
{
    return cache_p->arrayId_p.get ();
}

void
VisBufferImpl2::setArrayId (const Vector<Int> &value)
{
    cache_p->arrayId_p.set (value);
}

const Vector<SquareMatrix<Complex, 2> > &
VisBufferImpl2::cjones () const
{
    return cache_p->cjones_p.get ();
}

const Vector<Int> &
VisBufferImpl2::correlationTypes () const
{
    return cache_p->corrType_p.get ();
}

//Int
//VisBufferImpl2::dataDescriptionId () const
//{
//    return cache_p->dataDescriptionId_p.get ();
//}

const Vector<Int> &
VisBufferImpl2::dataDescriptionIds () const
{
	return cache_p->dataDescriptionIds_p.get ();
}

//void
//VisBufferImpl2::setDataDescriptionId (Int value)
//{
//    cache_p->dataDescriptionId_p.set (value);
//}

void
VisBufferImpl2::setDataDescriptionIds (const Vector<Int> & value)
{
    cache_p->dataDescriptionIds_p.set (value);
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

void
VisBufferImpl2::setExposure (const Vector<Double> & value)
{
    cache_p->exposure_p.set (value);
}

const Vector<Int> &
VisBufferImpl2::feed1 () const
{
    return cache_p->feed1_p.get ();
}

void
VisBufferImpl2::setFeed1 (const Vector<Int> & value)
{
    cache_p->feed1_p.set (value);
}

const Vector<Float> &
VisBufferImpl2::feedPa1 () const
{
    return cache_p->feed1Pa_p.get ();
}

const Vector<Int> &
VisBufferImpl2::feed2 () const
{
    return cache_p->feed2_p.get ();
}

void
VisBufferImpl2::setFeed2 (const Vector<Int> & value)
{
    cache_p->feed2_p.set (value);
}

const Vector<Float> &
VisBufferImpl2::feedPa2 () const
{
    return cache_p->feed2Pa_p.get ();
}

const Vector<Int> &
VisBufferImpl2::fieldId () const
{
    return cache_p->fieldId_p.get ();
}

void
VisBufferImpl2::setFieldId (const Vector<Int> & value)
{
    cache_p->fieldId_p.set (value);
}

//const Matrix<Bool> &
//VisBufferImpl2::flag () const
//{
//    return cache_p->flag_p.get ();
//}

//void
//VisBufferImpl2::setFlag (const Matrix<Bool>& value)
//{
//    cache_p->flag_p.set (value);
//}

const Array<Bool> &
VisBufferImpl2::flagCategory () const
{
    return cache_p->flagCategory_p.get();
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

Cube<Bool> &
VisBufferImpl2::flagCubeRef ()
{
    return cache_p->flagCube_p.getRef ();
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

Vector<Bool> &
VisBufferImpl2::flagRowRef ()
{
    return cache_p->flagRow_p.getRef ();
}

void
VisBufferImpl2::setFlagRow (const Vector<Bool>& value)
{
    cache_p->flagRow_p.set (value);
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

Int
VisBufferImpl2::nAntennas () const
{
    return cache_p->nAntennas_p.get ();
}

Int
VisBufferImpl2::nChannels () const
{
    return cache_p->nChannels_p.get ();
}

Int
VisBufferImpl2::nCorrelations () const
{
    return cache_p->nCorrelations_p.get();
}

Int
VisBufferImpl2::nRows () const
{
    return cache_p->nRows_p.get ();
}

const Vector<Int> &
VisBufferImpl2::observationId () const
{
    return cache_p->observationId_p.get ();
}

void
VisBufferImpl2::setObservationId (const Vector<Int> & value)
{
    cache_p->observationId_p.set (value);
}

const MDirection&
VisBufferImpl2::phaseCenter () const
{
    return cache_p->phaseCenter_p.get ();
}

Int
VisBufferImpl2::polarizationFrame () const
{
    return cache_p->polFrame_p.get ();
}

Int
VisBufferImpl2::polarizationId () const
{
    return cache_p->polarizationId_p.get ();
}

const Vector<Int> &
VisBufferImpl2::processorId () const
{
    return cache_p->processorId_p.get ();
}

void
VisBufferImpl2::setProcessorId (const Vector<Int> & value)
{
    cache_p->processorId_p.set (value);
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

void
VisBufferImpl2::setScan (const Vector<Int> & value)
{
    cache_p->scan_p.set (value);
}

const Matrix<Float> &
VisBufferImpl2::sigma () const
{
    return cache_p->sigma_p.get ();
}

void
VisBufferImpl2::setSigma (const Matrix<Float> & sigma)
{
    cache_p->sigma_p.set (sigma);
}

//const Matrix<Float> &
//VisBufferImpl2::sigmaMat () const
//{
//    return cache_p->sigmaMat_p.get ();
//}

//Int
//VisBufferImpl2::spectralWindow () const
//{
//    return cache_p->spectralWindow_p.get ();
//}

const Vector<Int> &
VisBufferImpl2::spectralWindows () const
{
	return cache_p->spectralWindows_p.get ();
}

void
VisBufferImpl2::setSpectralWindows (const Vector<Int> & spectralWindows)
{
    cache_p->spectralWindows_p.set (spectralWindows);
}


const Vector<Int> &
VisBufferImpl2::stateId () const
{
    return cache_p->stateId_p.get ();
}

void
VisBufferImpl2::setStateId (const Vector<Int> & value)
{
    cache_p->stateId_p.set (value);
}

const Vector<Double> &
VisBufferImpl2::time () const
{
    return cache_p->time_p.get ();
}

void
VisBufferImpl2::setTime (const Vector<Double> & value)
{
    cache_p->time_p.set (value);
}

const Vector<Double> &
VisBufferImpl2::timeCentroid () const
{
    return cache_p->timeCentroid_p.get ();
}

void
VisBufferImpl2::setTimeCentroid (const Vector<Double> & value)
{
    cache_p->timeCentroid_p.set (value);
}

const Vector<Double> &
VisBufferImpl2::timeInterval () const
{
    return cache_p->timeInterval_p.get ();
}

void
VisBufferImpl2::setTimeInterval (const Vector<Double> & value)
{
    cache_p->timeInterval_p.set (value);
}

const Matrix<Double> &
VisBufferImpl2::uvw () const
{
    return cache_p->uvw_p.get ();
}

void
VisBufferImpl2::setUvw (const Matrix<Double> & value)
{
    cache_p->uvw_p.set (value);
}

const Cube<Complex> &
VisBufferImpl2::visCubeCorrected () const
{
    return cache_p->correctedVisCube_p.get ();
}

Cube<Complex> &
VisBufferImpl2::visCubeCorrectedRef ()
{
    return cache_p->correctedVisCube_p.getRef ();
}

void
VisBufferImpl2::setVisCubeCorrected (const Cube<Complex> & value)
{
    cache_p->correctedVisCube_p.set (value);
}

//const Matrix<CStokesVector> &
//VisBufferImpl2::visCorrected () const
//{
//    return cache_p->correctedVisibility_p.get ();
//}

//void
//VisBufferImpl2::setVisCorrected (const Matrix<CStokesVector> & value)
//{
//    cache_p->correctedVisibility_p.set (value);
//}

const Cube<Float> &
VisBufferImpl2::visCubeFloat () const
{
    return cache_p->floatDataCube_p.get ();
}

void
VisBufferImpl2::setVisCubeFloat (const Cube<Float> & value)
{
    cache_p->floatDataCube_p.set (value);
}

const Cube<Complex> &
VisBufferImpl2::visCubeModel () const
{
    return cache_p->modelVisCube_p.get ();
}

Cube<Complex> &
VisBufferImpl2::visCubeModelRef ()
{
    return cache_p->modelVisCube_p.getRef ();
}

void
VisBufferImpl2::setVisCubeModel (const Complex & value)
{
    cache_p->modelVisCube_p.set (value);
}

void
VisBufferImpl2::setVisCubeModel (const Cube<Complex> & value)
{
    cache_p->modelVisCube_p.set (value);
}

//const Matrix<CStokesVector> &
//VisBufferImpl2::visModel () const
//{
//    return cache_p->modelVisibility_p.get ();
//}

//void
//VisBufferImpl2::setVisModel (Matrix<CStokesVector> & value)
//{
//    cache_p->modelVisibility_p.set (value);
//}

//void
//VisBufferImpl2::setVisCubeModel(const Vector<Float>& stokesIn)
//{
//
//  enum {I, Q, U, V};
//
//  Vector<Float> stokes (4, 0.0);
//
//  stokes [I] = 1.0;  // Stokes parameters, nominally unpolarized, unit I
//
//  for (uInt i = 0; i < stokesIn.nelements(); ++i){
//      stokes [i] = stokesIn [i];
//  }
//
//  // Convert to correlations, according to basis
//
//  Vector<Complex> stokesFinal (4, Complex(0.0)); // initially all zero
//
//  if (polarizationFrame() == MSIter::Circular){
//    stokesFinal(0) = Complex(stokes [I] + stokes [V]);
//    stokesFinal(1) = Complex(stokes [Q], stokes [U]);
//    stokesFinal(2) = Complex(stokes [Q], -stokes [U]);
//    stokesFinal(3) = Complex(stokes [I] - stokes [V]);
//  }
//  else if (polarizationFrame() == MSIter::Linear) {
//    stokesFinal(0) = Complex(stokes [I] + stokes [Q]);
//    stokesFinal(1) = Complex(stokes [U], stokes [V]);
//    stokesFinal(2) = Complex(stokes [U], -stokes [V]);
//    stokesFinal(3) = Complex(stokes [I] - stokes [Q]);
//  }
//  else {
//    throw(AipsError("Model-setting only works for CIRCULAR and LINEAR bases, for now."));
//  }
//
//  // A map onto the actual correlations in the VisBuffer
//
//  Vector<Int> corrmap = correlationTypes ();
//  corrmap -= corrmap(0);
//
//  ThrowIf (max(corrmap) >= 4,  "HELP! The correlations in the data are not normal!");
//
//  // Set the modelVisCube accordingly
//
//  Cube<Complex> visCube (getViP()->visibilityShape(), 0.0);
//
//  for (Int icorr = 0; icorr < nCorrelations (); ++icorr){
//    if (abs(stokesFinal(corrmap(icorr))) > 0.0) {
//      visCube (Slice (icorr, 1, 1), Slice(), Slice()).set(stokesFinal (corrmap (icorr)));
//    }
//  }
//
//  cache_p->modelVisCube_p.set (visCube);
//
//  // Lookup flux density calibrator scaling, and apply it per channel...
//  //  TBD
//}


const Cube<Complex> &
VisBufferImpl2::visCube () const
{
    return cache_p->visCube_p.get ();
}

Cube<Complex> &
VisBufferImpl2::visCubeRef ()
{
    return cache_p->visCube_p.getRef ();
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

//const Matrix<CStokesVector> &
//VisBufferImpl2::vis () const
//{
//    return cache_p->visibility_p.get ();
//}

//void
//VisBufferImpl2::setVis (Matrix<CStokesVector> & value)
//{
//    cache_p->visibility_p.set (value);
//}

const Matrix<Float> &
VisBufferImpl2::weight () const
{
    return cache_p->weight_p.get ();
}

void
VisBufferImpl2::setWeight (const Matrix<Float>& value)
{
    cache_p->weight_p.set (value);
}

//const Matrix<Float> &
//VisBufferImpl2::weightMat () const
//{
//    return cache_p->weightMat_p.get ();
//}

//void
//VisBufferImpl2::setWeightMat (const Matrix<Float>& value)
//{
//    cache_p->weightMat_p.set (value);
//}

const Cube<Float> &
VisBufferImpl2::weightSpectrum () const
{
    return cache_p->weightSpectrum_p.get ();
}

Cube<Float> &
VisBufferImpl2::weightSpectrumRef ()
{
    return cache_p->weightSpectrum_p.getRef();
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
VisBufferImpl2::fillArrayId (Vector<Int>& value) const
{
  CheckVisIter ();

  getViP()->arrayIds (value);
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

    getViP()->visibilityCorrected (value);
}


void
VisBufferImpl2::fillCubeModel (Cube <Complex> & value) const
{
    CheckVisIter ();

    String modelkey=String("definedmodel_field_")+String::toString(fieldId());
    Bool hasmodkey=getViP()->ms().keywordSet().isDefined(modelkey);

    if (hasmodkey || !(getViP()->ms().tableDesc().isColumn("MODEL_DATA"))){

        //cerr << "HASMOD " << state_p->visModelData_p.hasModel(msId(), fieldId(), spectralWindow()) << endl;

        if (state_p->visModelData_p.hasModel (msId(), fieldId()(0), spectralWindows()(0)) == -1){

            if(hasmodkey){

                String whichrec=getViP()->ms().keywordSet().asString(modelkey);
                Record modrec(getViP()->ms().keywordSet().asRecord(whichrec));
                state_p->visModelData_p.addModel(modrec, Vector<Int>(1, msId()), VisBuffer2Adapter (this));
            }
        }

        {
            // Horrible kluge here.  The model field itself is mutable within this class so
            // that the filling operation can modify it even though the accessor method
            // is const.  This is a reasonable use of mutable since the user is unaware that
            // the data is cached and filled on demand.  However, the getModelVis method
            // repeatedly modifies the VisBuffer that it gets passed and there doesn't appear
            // to be any easy way to rework it so that it can get by with const access (it
            // modifies the modelVis field and then passes the modified VisBuffer on down
            // to other virtual methods which can potentially access the modified model
            // field.

            VisBuffer2Adapter vb2a (const_cast <VisBufferImpl2 *> (this));
            state_p->visModelData_p.getModelVis (vb2a);
        }
    }
    else{

        // Get the model data from the measurement set via the VI

        getViP()->visibilityModel (value);
    }
}

void
VisBufferImpl2::fillCubeObserved (Cube <Complex> & value) const
{
    CheckVisIter ();

    getViP()->visibilityObserved (value);
}

void
VisBufferImpl2::fillDataDescriptionId  (Int& value) const
{
  CheckVisIter ();

  value = getViP()->dataDescriptionId ();
}

void
VisBufferImpl2::fillDataDescriptionIds  (Vector<Int>& value) const
{
  CheckVisIter ();

  getViP()->dataDescriptionIds (value);
}


void
VisBufferImpl2::fillDirection1 (Vector<MDirection>& value) const
{
  CheckVisIterBase ();
  // fill state_p->feed1_pa cache, antenna, feed and time will be filled automatically

  feedPa1 ();

  fillDirectionAux (value, antenna1 (), feed1 (), feedPa1 ());

  value.resize(antenna1 ().nelements()); // could also use nRow()
}

void
VisBufferImpl2::fillDirection2 (Vector<MDirection>& value) const
{
  CheckVisIterBase ();
  // fill state_p->feed1_pa cache, antenna, feed and time will be filled automatically

  feedPa2 ();

  fillDirectionAux (value, antenna2 (), feed2 (), feedPa2 ());

  value.resize(antenna2 ().nelements()); // could also use nRow()
}

void
VisBufferImpl2::fillDirectionAux (Vector<MDirection>& value,
                                  const Vector<Int> & antenna,
                                  const Vector<Int> &feed,
                                  const Vector<Float> & feedPa) const
{
    value.resize (antenna.nelements()); // could also use nRow()

    const ROMSPointingColumns & mspc = getViP()->subtableColumns ().pointing();
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
        }
        else {

            value(row) = phaseCenter(); // nothing found, use phase center
        }

        if (!getViP()->allBeamOffsetsZero()) {

            RigidVector<Double, 2> beamOffset = getViP()->getBeamOffsets()(0, antenna (row),
                                                                           feed (row));

            if (downcase (getViP()->antennaMounts()(antenna (row))) == "alt-az") {

                SquareMatrix<Double, 2> xform(SquareMatrix<Double, 2>::General);

                Double cpa = cos(feedPa(row));
                Double spa = sin(feedPa(row));

                xform(0, 0) = cpa;
                xform(1, 1) = cpa;
                xform(0, 1) = -spa;
                xform(1, 0) = spa;

                beamOffset *= xform; // parallactic angle rotation
            }

            value(row).shift(-beamOffset(0), beamOffset(1), True);
                // x direction is flipped to convert az-el type frame to ra-dec
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

  // Fill feed, antenna and time caches, if not filled before.

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

    const Vector<Float>& antennaPointingAngle = this->feedPa (time ()(row));

    Assert(antenna (row) >= 0 && antenna (row) < (int) antennaPointingAngle.nelements());

    feedPa (row) = antennaPointingAngle (antenna (row));

    if (feed (row) != 0){  // Skip when feed(row) is zero

      // feedPa returns only the first feed position angle so
      // we need to add an offset if this row correspods to a
      // different feed

      float feedsAngle = getViP()->receptorAngles()(0, antenna (row), feed (row));
      float feed0Angle = getViP()->receptorAngles()(0, antenna (row), 0);

      feedPa (row) += feedsAngle - feed0Angle;
    }
  }
}

void
VisBufferImpl2::fillFieldId (Vector<Int>& value) const
{
  CheckVisIter ();

  getViP()->fieldIds (value);
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
             "Bug check: Imaging weight generator not set");

#warning "Rework logic so that called code is not expecting a flag matrix."

    value.resize (IPosition (2, nChannels(), nRows()));

    Matrix<Bool> flagMat = flagCube().yzPlane(0);
    std::logical_and<Bool> andOp;

    Vector<Float> wts (nRows (), 0);

    wts = weight().row(0);
    wts += weight().row(nCorrelations() - 1);
    wts *= 0.5f;

    for (Int i = 1; i < nCorrelations(); ++ i){

        Matrix<Bool> flagPlane = flagCube().yzPlane(i);
        arrayTransform<Bool,Bool,Bool,std::logical_and<Bool> > (flagMat, flagPlane, flagMat, andOp);
    }

    if (weightGenerator.getType () == "uniform") {

        weightGenerator.weightUniform (value, flagMat, uvw (), getFrequencies (0), wts, msId (), fieldId ()(0));

    } else if (weightGenerator.getType () == "radial") {

        weightGenerator.weightRadial (value, flagMat, uvw (), getFrequencies (0), wts);

    } else {

        weightGenerator.weightNatural (value, flagMat, wts);
    }

    if (weightGenerator.doFilter ()) {

        weightGenerator.filter (value, flagMat, uvw (), getFrequencies (0), wts);
    }
}

void
VisBufferImpl2::fillJonesC (Vector<SquareMatrix<Complex, 2> >& value) const
{
  CheckVisIter ();

  getViP()->jonesC (value);
}

void
VisBufferImpl2::fillNAntennas (Int & value) const
{
  CheckVisIter ();

  value = getVi()->getNAntennas();
}

void
VisBufferImpl2::fillNChannel (Int &) const
{
  CheckVisIter ();

  // This value enters the VB from a route that doesn't involve
  // filling; however the framework requires that this method exist
  // so it's implemented as a no-op.
}

void
VisBufferImpl2::fillNCorr (Int &) const
{
  CheckVisIter ();

  // This value enters the VB from a route that doesn't involve
  // filling; however the framework requires that this method exist
  // so it's implemented as a no-op.
}

void
VisBufferImpl2::fillNRow (Int&) const
{
  CheckVisIter ();

  // This value enters the VB from a route that doesn't involve
  // filling; however the framework requires that this method exist
  // so it's implemented as a no-op.
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
VisBufferImpl2::fillPolarizationId (Int& value) const
{
  CheckVisIter ();

  value = getViP()->polarizationId ();
}

void
VisBufferImpl2::fillProcessorId (Vector<Int>& value) const
{
  CheckVisIter();

  getViP()->processorId (value);
}

void
VisBufferImpl2::fillRowIds (Vector<uInt>& value) const
{
  CheckVisIter ();

  getViP()->getRowIds(value);
}


void
VisBufferImpl2::fillScan (Vector<Int>& value) const
{
  CheckVisIter ();

  getViP()->scan (value);
}

void
VisBufferImpl2::fillSigma (Matrix<Float>& value) const
{
  CheckVisIter ();

  getViP()->sigma (value);
}

//void
//VisBufferImpl2::fillSigmaMat (Matrix<Float>& value) const
//{
//  CheckVisIter ();
//
//  getViP()->sigmaMat (value);
//}

//void
//VisBufferImpl2::fillSpectralWindow (Int& value) const
//{
//  CheckVisIter ();
//
//  value = getViP()->spectralWindow ();
//}

void
VisBufferImpl2::fillSpectralWindows (Vector<Int>& value) const
{
  CheckVisIter ();

  getViP()->spectralWindows (value);
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
VisBufferImpl2::fillUvw (Matrix<Double>& value) const
{
  CheckVisIter ();

  getViP()->uvw (value);
}

//void
//VisBufferImpl2::fillVisibilityCorrected (Matrix<CStokesVector>& value) const
//{
//    CheckVisIter ();
//
//    getViP()->visibilityCorrected (value);
//}

//void
//VisBufferImpl2::fillVisibilityModel (Matrix<CStokesVector>& value) const
//{
//    CheckVisIter ();
//
//    getViP()->visibilityModel (value);
//}

//void
//VisBufferImpl2::fillVisibilityObserved (Matrix<CStokesVector>& value) const
//{
//    CheckVisIter ();
//
//    getViP()->visibilityObserved (value);
//}


void
VisBufferImpl2::fillWeight (Matrix<Float>& value) const
{
  CheckVisIter ();

  getViP()->weight (value);
}


void
VisBufferImpl2::fillWeightSpectrum (Cube<Float>& value) const
{
  CheckVisIter ();

  getViP()->weightSpectrum (value);
}

Float
VisBufferImpl2::getWeightScaled (Int row) const
{
    Float sum = 0;
    Int n = nCorrelations();

    for (Int correlation = 0; correlation < n; ++ correlation){

        sum += getWeightScaled (correlation, row);
    }

    return sum / n;
}

Float
VisBufferImpl2::getWeightScaled (Int correlation, Int row) const
{
    if (flagRow () (row)){
        return 0;
    }

    if (weightSpectrumPresent()){

        Float sum;
        Int n = nChannels ();

        for (Int channel = 0; channel < n; ++ channel){

            sum += getWeightScaled (correlation, channel, row);

        }

        return sum / n;
    }
    else {

        Float theWeight = weight () (correlation, row);

        if (! state_p->weightScaling_p.null()){
            theWeight = (* state_p->weightScaling_p) (theWeight);
        }

        return theWeight;
    }
}

Float
VisBufferImpl2::getWeightScaled (Int correlation, Int channel, Int row) const
{
    // Get the weight from the weightSpectrum if it is present (either it was
    // read from the MS or it was set by the user); otherwise get the weight
    // from the weight column.

    Float theWeight = 0;

    if (weightSpectrumPresent ()){

        theWeight = weightSpectrum () (correlation, channel, row);
    }
    else{
        theWeight = weight () (correlation, row);
    }

    // If there is a scaling function, the apply that to the weight

    if (! state_p->weightScaling_p.null()){
        theWeight = (* state_p->weightScaling_p) (theWeight);
    }

    return theWeight;
}

CountedPtr<WeightScaling>
VisBufferImpl2::getWeightScaling () const
{
    return state_p->weightScaling_p;
}

Bool
VisBufferImpl2::weightSpectrumPresent () const
{
    Bool present = cache_p->weightSpectrum_p.isPresent() ||
                   (isAttached() && getVi()->existsWeightSpectrum());

    return present;
}


} // end namespace vi

} // end namespace casa
