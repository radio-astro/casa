/*
 * VisBufferImpl2Internal.h
 *
 *  Created on: Aug 22, 2013
 *      Author: jjacobs
 */

#ifndef VISBUFFERIMPL2INTERNAL_H_
#define VISBUFFERIMPL2INTERNAL_H_

#include <cassert>

namespace casa {

namespace ms {
    class Vbi2MsRow;
}

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

    virtual void appendRows (Int nRowsToAdd, Bool truncate = False) = 0;
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

    virtual void appendRows (Int, Bool)
    {
        // Noop for scalars
    }

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
    getRef (Bool fillIfAbsent = True)
    {
        if (! isPresent() && fillIfAbsent){
            fill ();
        }
        setAsPresent();

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
        ThrowIf (True, "Scalar shapes should not have shape errors.");

        return String();
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

    virtual void appendRows (Int nRows, Bool truncate)
    {
        IPosition shape = this->getItem().shape();
        Int nDims = shape.size();

        if (nDims == 0 || shapePattern_p == NoCheck){
            // This item is empty or unfillable so leave it alone.
        }
        else if (truncate){

            // Make any excess rows disappear with a little hack to
            // avoid a copy:
            //
            // The array tmp is copy constructed to reference this item's
            // array, thus sharing its storage.  Then tmp is resized to have
            // the appropriate shape (few rows). Finally the original array
            // is set up to share the storage of tmp. This leave the current
            // array with excess storage but the proper number of elements.
            // When this item's array is freed or resized it will return all
            // of the storage so there is no memory leak;

            shape [nDims - 1] = nRows;

            T tmp = this->getItem();
            tmp.resize (shape, True);
            this->getItem().reference (tmp);
        }
        else{

            IPosition desiredShape = this->getVb()->getValidShape (shapePattern_p);

            desiredShape [nDims - 1] = shape [nDims - 1] + nRows;

            this->getItem().resize (desiredShape, True);
        }

        // Resize the array copying the existing values if needed.

    }

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
        AssertCc (shape.nelements() == 4);

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

    void appendComplete ();
    Int appendRow ();
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
    VbCacheItemArray <Cube<Float> > sigmaSpectrum_p;

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
    : appendCapacity_p (0),
      appendSize_p (0),
      areCorrelationsSorted_p (False),
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

    Int appendCapacity_p;
    Int appendSize_p;
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


}

}


#endif /* VISBUFFERIMPL2INTERNAL_H_ */
