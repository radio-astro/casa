/*
 * VisBuffer2Adapter.h
 *
 *  Created on: Oct 9, 2012
 *      Author: jjacobs
 */

#ifndef VISBUFFER2ADAPTER_H_
#define VISBUFFER2ADAPTER_H_

#include <synthesis/MSVis/VisBuffer.h>
#include <synthesis/MSVis/VisBuffer2.h>
#include <casa/BasicSL/String.h>
#include <synthesis/MSVis/UtilJ.h>
#include <synthesis/MSVis/VisibilityIterator2.h>
#include <synthesis/MSVis/VisibilityIteratorImpl2.h>

using casa::utilj::toStdError;

#define IllegalOperation() \
    Throw (String::format ("Illegal operation: %s", __func__));

#define CheckWritability() \
    ThrowIf (vb2Rw_p == 0, String::format ("VB is not writable in %s", __func__));

namespace casa {

namespace vi {

class VisBuffer2Adapter : public VisBuffer {

public:

    VisBuffer2Adapter (VisBuffer2 * vb) : vb2_p (vb) , vb2Rw_p (vb)
    {
        construct();
    }
    VisBuffer2Adapter (const VisBuffer2 * vb) : msColumns_p (0), vb2_p (vb), vb2Rw_p (0)
    {
        construct();
    }

    void
    construct ()
    {
        const VisibilityIteratorImpl2 * vi =
            dynamic_cast<const VisibilityIteratorImpl2 *> (vb2_p->getVi()->getImpl());

        msColumns_p = vi->msColumnsKluge();

        vi->allSpectralWindowsSelected (spectralWindows_p, nChannels_p);

    }

    ~VisBuffer2Adapter () {}

    virtual VisBuffer & assign(const VisBuffer &, Bool = True) {IllegalOperation();}

    virtual VisBuffer * clone () const {IllegalOperation();}

    virtual void invalidate(){IllegalOperation();}

//#if 0

    // <group>
    // Access functions
    //
    virtual Int & nCorr() { nCorr_p = vb2_p->nCorrelations(); return nCorr_p;}
    virtual Int nCorr() const { return vb2_p-> nCorrelations(); }

    virtual Int & nChannel() { nChannelsScalar_p = vb2_p-> nChannels(); return nChannelsScalar_p;}
    virtual Int nChannel() const { return vb2_p-> nChannels(); }

    virtual Vector<Int>& channel() { IllegalOperation (); }
    virtual const Vector<Int>& channel() const { IllegalOperation(); }

    virtual Int & nRow() { nRows_p = vb2_p->nRows (); return nRows_p;}

    virtual Vector<Int>& antenna1() { return const_cast<Vector<Int>&> (vb2_p-> antenna1());}
    virtual const Vector<Int>& antenna1() const { return vb2_p-> antenna1(); }

    virtual Vector<Int>& antenna2() { return const_cast<Vector<Int>&> (vb2_p-> antenna2());}
    virtual const Vector<Int>& antenna2() const { return vb2_p-> antenna2(); }

    virtual Vector<Int>& feed1() { return const_cast<Vector<Int>&> (vb2_p-> feed1());}
    virtual const Vector<Int>& feed1() const { return vb2_p-> feed1(); }

    virtual Vector<Int>& feed2() { return const_cast<Vector<Int>&> (vb2_p-> feed2());}
    virtual const Vector<Int>& feed2() const { return vb2_p-> feed2(); }

    // feed1_pa() and feed2_pa() return an array of parallactic angles
    // (each corresponds to the first receptor of the feed) one for each
    // row in the current buffer. In contrast, feed_pa() calculates
    // the angles for each antenna. These methods are implemented for
    // VisBuffer only to benefit from caching of the feed and antenna IDs.

    virtual Vector<Float>& feed1_pa() { IllegalOperation (); }
    virtual const Vector<Float>& feed1_pa() const { return vb2_p-> feedPa1(); }

    virtual Vector<Float>& feed2_pa() { IllegalOperation (); }
    virtual const Vector<Float>& feed2_pa() const { return vb2_p-> feedPa2(); }

    virtual Vector<SquareMatrix<Complex, 2> >& CJones() { IllegalOperation (); }
    virtual const Vector<SquareMatrix<Complex, 2> >& CJones() const { return vb2_p-> cjones(); }

    // Note that feed_pa is a function instead of a cached value
    virtual Vector<Float> feed_pa(Double time) const{ return vb2_p-> feedPa (time); }

    // direction1() and direction2() return arrays of directions where
    // the first and the second antenna/feed are pointed to. One value for
    // each row in the current buffer.
    virtual Vector<MDirection>& direction1() { IllegalOperation (); }
    virtual const Vector<MDirection>& direction1()  const { return vb2_p-> direction1(); }

    virtual Vector<MDirection>& direction2() { IllegalOperation (); }
    virtual const Vector<MDirection>& direction2()  const { return vb2_p-> direction2(); }


    // NOMINAL parallactic angle (feed p.a. offset NOT included)
    virtual Float parang0(Double time) const{ return vb2_p-> parang0(time); }
    virtual Vector<Float> parang(Double time) const{ return vb2_p-> parang(time); }

    // Note that azel is a function instead of a cached value
    virtual MDirection azel0(Double time) const{ return vb2_p-> azel0(time); }
    virtual Vector<Double>& azel0Vec(Double , Vector<Double>& ) const{ IllegalOperation(); }
    virtual Vector<MDirection> azel(Double time) const{ return vb2_p-> azel(time); }
    virtual Matrix<Double>& azelMat(Double , Matrix<Double>& ) const{ IllegalOperation(); }

    // Hour angle for specified time
    virtual Double hourang(Double time) const{ return vb2_p-> hourang(time); }

    virtual Int fieldId() const { return vb2_p-> fieldId()(0); }

    virtual Int& fieldIdRef() { IllegalOperation(); }

    virtual Int& arrayIdRef() { IllegalOperation(); }

    virtual Int arrayId() const { return vb2_p-> arrayId()(0); }

    // Return flag for each channel & row
    virtual Matrix<Bool>& flag() { IllegalOperation(); }
    virtual const Matrix<Bool>& flag() const { IllegalOperation(); }

    // Return flag for each polarization, channel and row
    virtual Cube<Bool>& flagCube() { return const_cast<Cube<Bool>&> (vb2_p-> flagCube());}
    virtual const Cube<Bool>& flagCube() const { return vb2_p-> flagCube(); }

    virtual Vector<Bool>& flagRow() { return const_cast<Vector<Bool>&> (vb2_p-> flagRow());}
    virtual const Vector<Bool>& flagRow() const { return vb2_p-> flagRow(); }

    // Return flags for each polarization, channel, category, and row.
    virtual Array<Bool>& flagCategory() { IllegalOperation (); }
    virtual const Array<Bool>& flagCategory() const { return vb2_p-> flagCategory(); }

    virtual Vector<Int>& scan() { return const_cast<Vector<Int>&> (vb2_p-> scan());}
    virtual const Vector<Int>& scan() const { return vb2_p-> scan(); }

    // scalar version for convenience, when scan known constant for
    // entire iteration/buffer.
    virtual Int scan0() { IllegalOperation(); }

    virtual Vector<Int>& processorId() { return const_cast<Vector<Int>&> (vb2_p-> processorId());}
    virtual const Vector<Int>& processorId() const { return vb2_p-> processorId(); }

    virtual Vector<Int>& observationId() { return const_cast<Vector<Int>&> (vb2_p-> observationId());}
    virtual const Vector<Int>& observationId() const { return vb2_p-> observationId(); }

    virtual Vector<Int>& stateId() { return const_cast<Vector<Int>&> (vb2_p-> stateId());}
    virtual const Vector<Int>& stateId() const { return vb2_p-> stateId(); }

    // Gets SPECTRAL_WINDOW/CHAN_FREQ (in Hz, acc. to the MS def'n v.2).
    virtual Vector<Double>& frequency() {
        return const_cast<Vector<Double>&> (vb2_p->getFrequencies(0));
    }
    virtual const Vector<Double>& frequency() const
    {
        return vb2_p->getFrequencies(0);
    }

    //if ignoreconv=True..frequency is served as is in the data frame
    virtual void lsrFrequency(const Int & spw, Vector<Double>& freq, Bool & convert,
                              const Bool ignoreConv = False) const
    {
        const VisibilityIteratorImpl2 * vi =
            dynamic_cast<const VisibilityIteratorImpl2 *> (vb2_p->getVi()->getImpl());

        Int frame = -1;
        if (ignoreConv){
            frame = vi->getObservatoryFrequencyType ();
        }
        else{
            frame = MFrequency::LSRK;
        }

        convert = frame != MFrequency::LSRK; // make this parameter write-only

        freq = vi->getFrequencies (this->time()(0), frame, spw, vi->msId());
    }

    virtual Int numberCoh () const { IllegalOperation(); }

    virtual MDirection & phaseCenter() { IllegalOperation (); }
    virtual MDirection phaseCenter() const { return vb2_p-> phaseCenter(); }

    virtual Int polFrame() const { return vb2_p-> polarizationFrame(); }

    virtual Vector<Int>& corrType() { return const_cast<Vector<Int> &> (vb2_p-> correlationTypes()); }
    virtual const Vector<Int>& corrType() const { return vb2_p-> correlationTypes(); }

    virtual Vector<Float>& sigma() { IllegalOperation (); }
    virtual const Vector<Float>& sigma() const { IllegalOperation(); }

    virtual Matrix<Float>& sigmaMat() { IllegalOperation (); }
    virtual const Matrix<Float>& sigmaMat() const { return vb2_p-> sigma(); }

    virtual Int & spectralWindow() { spectralWindow_p = vb2_p->spectralWindows()(0); return spectralWindow_p;}
    virtual Int spectralWindow() const { return vb2_p-> spectralWindows()(0); }
    virtual Int polarizationId() const { return vb2_p-> polarizationId(); }
    virtual Int& dataDescriptionIdRef() { IllegalOperation(); }
    virtual Int dataDescriptionId() const { return vb2_p-> dataDescriptionIds()(0); }
    virtual Vector<Double>& time() { IllegalOperation (); }
    virtual const Vector<Double>& time() const { return vb2_p-> time(); }

    virtual Vector<Double>& timeCentroid() { IllegalOperation (); }
    virtual const Vector<Double>& timeCentroid() const { return vb2_p-> timeCentroid(); }

    virtual Vector<Double>& timeInterval() { IllegalOperation (); }
    virtual const Vector<Double>& timeInterval() const { return vb2_p-> timeInterval(); }

    virtual Vector<Double>& exposure() { IllegalOperation (); }
    virtual const Vector<Double>& exposure() const { return vb2_p-> exposure(); }

    virtual Vector<RigidVector<Double, 3> >& uvw()
    {
        if (uvw_p.empty()){

            fillUvw ();

        }

        return uvw_p;
    }

    virtual const Vector<RigidVector<Double, 3> >& uvw() const
    {
        if (uvw_p.empty()){

            fillUvw ();

        }

        return uvw_p;
    }

    void
    fillUvw() const {

        const Matrix<Double> & u = vb2_p->uvw();

        Int nRows = u.shape()(1);
        uvw_p.resize (nRows);

        for (Int i = 0; i < nRows; i++){
            RigidVector<Double,3> t;
            for (Int j = 0; j < 3; j++){
                t (j) = u (j, i);
            }
            uvw_p (i) = t;
        }
    }


    virtual Matrix<Double>& uvwMat() { IllegalOperation (); }
    virtual const Matrix<Double>& uvwMat() const { return vb2_p-> uvw(); }

    virtual Matrix<CStokesVector>& visibility() { IllegalOperation (); }
    virtual const Matrix<CStokesVector>& visibility() const { IllegalOperation (); }

    virtual Matrix<CStokesVector>& modelVisibility() { IllegalOperation (); }
    virtual const Matrix<CStokesVector>& modelVisibility() const { IllegalOperation (); }

    virtual Matrix<CStokesVector>& correctedVisibility() { IllegalOperation (); }
    virtual const Matrix<CStokesVector>& correctedVisibility() const { IllegalOperation (); }

    virtual Cube<Complex>& visCube() { IllegalOperation (); }
    virtual const Cube<Complex>& visCube() const { return vb2_p-> visCube(); }

    virtual Cube<Complex>& modelVisCube() { return const_cast<Cube<Complex> &> (vb2_p->visCubeModel()); }

    virtual Cube<Complex>& modelVisCube(const Bool & ) { IllegalOperation (); }

    virtual const Cube<Complex>& modelVisCube() const { return vb2_p->visCubeModel(); }

    virtual Cube<Complex>& correctedVisCube() { IllegalOperation(); }
    virtual const Cube<Complex>& correctedVisCube() const { return vb2_p-> visCubeCorrected(); }

    // Return visCube(), modelVisCube(), or correctedVisCube(),
    // according to whichcol.  They throw an exception if whichcol is
    // unsupported, but note the encouraged default.
    // TODO: Optionally return DATA if whichcol is unavailable.
    Cube<Complex>& dataCube(const MS::PredefinedColumns whichcol=MS::DATA);
    const Cube<Complex>& dataCube(const MS::PredefinedColumns
                                  whichcol=MS::DATA) const;

    virtual Cube<Float>& floatDataCube() { IllegalOperation(); }
    virtual const Cube<Float>& floatDataCube() const { return vb2_p-> visCubeFloat (); }

    // Returns the weights for each row averaged over the parallel hand correlations.
    virtual Vector<Float>& weight() { IllegalOperation(); }
    virtual const Vector<Float>& weight() const { IllegalOperation(); }

    // Returns the nPol_p x curNumRow_p weight matrix.
    virtual Matrix<Float>& weightMat() { IllegalOperation(); }
    virtual const Matrix<Float>& weightMat() const { return vb2_p-> weight(); }

    // Is a valid WEIGHT_SPECTRUM available?
    virtual Bool existsWeightSpectrum() const { IllegalOperation(); }

    virtual Cube<Float>& weightSpectrum() { IllegalOperation();; }
    virtual const Cube<Float>& weightSpectrum() const { return vb2_p-> weightSpectrum(); }

    virtual Matrix<Float>& imagingWeight(){ IllegalOperation(); }
    virtual const Matrix<Float>& imagingWeight() const{ return vb2_p-> imagingWeight(); }

    virtual Cube<Float>& weightCube() { IllegalOperation(); }
    //</group>

    //<group>
    // Utility functions to provide coordinate or column ranges of the
    // data in the VisBuffer. Flagging is applied before computing the ranges.
    //
    // Generic accessor to column ranges of integer type, as specified by
    // enumerations defined in class MSCalEnums. Throws an exception
    // if the enum is not for a recognized integer column.
    virtual Vector<Int> vecIntRange(const MSCalEnums::colDef & ) const{ IllegalOperation(); }

    // Antenna id. range (includes both ANTENNA1 and ANTENNA2 columns)
    virtual Vector<Int> antIdRange() const{ IllegalOperation(); }

    // Time range
    virtual Bool timeRange(MEpoch & , MVEpoch & , MVEpoch & ) const
    { IllegalOperation(); }

    // Return the row Ids from the original ms. If the ms used is a subset of
    // another ms then rowIds() return the row ids of the original ms.
    virtual Vector<uInt>& rowIds(){ IllegalOperation(); }

    virtual const Vector<uInt>& rowIds() const { return vb2_p-> rowIds(); };

    //</group>

    // Frequency average the buffer (visibility() column only)
    virtual void freqAverage(){ IllegalOperation(); }

    // Frequency average the buffer (visCube and [if present] modelVisCube)
    void freqAveCubes(){ IllegalOperation(); }

    // Average channel axis according to chanavebounds, for whichever of DATA,
    // MODEL_DATA, CORRECTED_DATA, FLOAT_DATA, FLAG, and WEIGHT_SPECTRUM are
    // present.  It will only treat the first 5 as present if they have already
    // been loaded into the buffer!
    void channelAve(const Matrix<Int>& ){ IllegalOperation(); }

    // Average channel axis by factor.
    template<class T> void chanAveVisCube(Cube<T>& , Int ){ IllegalOperation(); }

    // Accumulate channel axis by factor, without applying WEIGHT_SPECTRUM even
    // if it is present.
    // It is primarily intended for averaging WEIGHT_SPECTRUM itself.
    template<class T> void chanAccCube(Cube<T>& , Int ){ IllegalOperation(); }

    // This defaults to no conceptual side effects, but usually it is more
    // efficient to let it leave weightSpectrum() in a channel averaged state.
    // restoreWeightSpectrum has no effect if !existsWeightSpectrum().
    virtual void chanAveFlagCube(Cube<Bool>& , const Int ,
                         const Bool  = True){ IllegalOperation(); }

    // Doesn't do anything if flagcat is degenerate.
    void chanAveFlagCategory(Array<Bool>& flagcat, const Int nChanOut);

    // Form Stokes parameters from correlations
    //  (these are preliminary versions)
    virtual void formStokes(){ IllegalOperation(); }
    virtual void formStokesWeightandFlag(){ IllegalOperation(); }
    virtual void formStokes(Cube<Complex>& ){ IllegalOperation(); }
    virtual void formStokes(Cube<Float>& ){ IllegalOperation(); }    // Will throw up if asked to do all 4.

    // Sort/unsort the correlations, if necessary
    //  (Rudimentary handling of non-canonically sorted correlations--use with care!)
    virtual void sortCorr(){ IllegalOperation(); }
    virtual void unSortCorr(){ IllegalOperation(); }

    // Normalize the visCube by the modelVisCube
    //   (and optionally also divide visCube_p by its normalized amp)
    virtual void normalize(const Bool &  = False){ CheckWritability(); vb2Rw_p-> normalize(); }

    // Fill weightMat according to sigma column
    virtual void resetWeightMat(){ IllegalOperation(); }

    // Rotate visibility phase for given vector (dim = nrow of vb) of phases (metres)
    virtual void phaseCenterShift(const Vector<Double>& ){ IllegalOperation(); }
    // Rotate visibility phase for phase center offsets (arcsecs)
    virtual void phaseCenterShift(Double , Double ){ IllegalOperation(); }

    // Update coordinate info - useful for copied VisBuffers that need
    // to retain some state for later reference.
    // Presently this fills antenna, array, field and spectralWindow ids, time,
    // frequency and number of rows.
    // if dirDependent is set to False the expensive direction dependent calculation of parallactic or direction of
    // antenna from pointing table is avoided
    //Add more as needed.
    virtual void updateCoordInfo(const VisBuffer *  = NULL, const Bool =True){ IllegalOperation(); }

    // Set the visibility to a constant, note that this only changes the buffer,
    // no values are written back to tables from here.
    virtual void setVisCube(Complex c){ CheckWritability(); vb2Rw_p-> setVisCube(c); }
    virtual void setModelVisCube(Complex c){ CheckWritability(); vb2Rw_p-> setVisCubeModel(c); }
    virtual void setCorrectedVisCube(Complex ){ IllegalOperation(); }

    // Set the visibility, note that this only changes the buffer,
    // no values are written back to tables from here.
    virtual void setVisCube(const Cube<Complex>& vis){ CheckWritability(); vb2Rw_p-> setVisCube(vis); }
    virtual void setModelVisCube(const Cube<Complex>& vis){ CheckWritability(); vb2Rw_p-> setVisCubeModel(vis); }
    virtual void setCorrectedVisCube(const Cube<Complex>& vis){ CheckWritability(); vb2Rw_p-> setVisCubeCorrected(vis); }

    // Like the above, but for FLOAT_DATA, keeping it as real floats.
    virtual void setFloatDataCube(const Cube<Float>& fcube){ CheckWritability(); vb2Rw_p-> setVisCubeFloat(fcube); }

    // Set model according to a Stokes vector
    virtual void setModelVisCube(const Vector<Float>& /*stokes*/){ IllegalOperation (); }

    // Reference external model visibilities
    virtual void refModelVis(const Matrix<CStokesVector>& ){ IllegalOperation(); }

    // Remove scratch cols data from vb
    virtual void removeScratchCols(){ IllegalOperation(); }

    // Access the current ROMSColumns object via VisIter
    virtual const ROMSColumns & msColumns() const
    {
        Assert (msColumns_p != 0);
        return * msColumns_p;
    }

    virtual  Int numberAnt () const{ return vb2_p-> nAntennas (); }

    // Get all selected spectral windows not just the one in the actual buffer
    virtual void allSelectedSpectralWindows(Vector<Int>& spectralWindows, Vector<Int>& nChannels)
    {
        spectralWindows.assign (spectralWindows_p);
        nChannels.assign (nChannels_p);
    }

    virtual void getChannelSelection(Block< Vector<Int> >& ,
				   Block< Vector<Int> >& ,
				   Block< Vector<Int> >& ,
				   Block< Vector<Int> >& ,
				   Block< Vector<Int> >& ) const { IllegalOperation(); }
    void allSelectedSpectralWindows(Vector<Int>& spws, Vector<Int>& nvischan) const ;

    // Return the actual msid, useful if using multiple ms to monitor which
    // ms in the  list is being dealt with
    virtual Int msId() const { return vb2_p-> msId(); }

    //checked if the ms has changed since the last chunk processed
    virtual Bool newMS() const { return vb2_p-> isNewMs(); }
    //get the name of the ms the buffer is at empty string if no visiter is attached
    virtual String msName(Bool =False) const{ return vb2_p-> msName(); }

    virtual Bool newArrayId () const{ return vb2_p-> isNewArrayId (); }
    virtual Bool newFieldId () const{ return vb2_p-> isNewFieldId (); }
    virtual Bool newSpectralWindow () const{ return vb2_p-> isNewSpectralWindow (); }

    //


private:


    const ROMSColumns * msColumns_p; // [use]
    Vector<Int> nChannels_p;
    Int nChannelsScalar_p;
    Int nCorr_p;
    Int nRows_p;
    Int spectralWindow_p;
    Vector<Int> spectralWindows_p;
    mutable Vector<RigidVector<Double, 3> > uvw_p;
    const vi::VisBuffer2 * vb2_p; // [use]
    vi::VisBuffer2 * vb2Rw_p; // [use]

};

} // end namespace vi

} // end namespace casa


#endif /* VISBUFFER2ADAPTER_H_ */
