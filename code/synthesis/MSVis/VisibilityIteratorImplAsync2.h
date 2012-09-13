/*
 * VisibilityIteratorAsync.h
 *
 *  Created on: Nov 1, 2010
 *      Author: jjacobs
 */

#if ! defined (VisibilityIteratorAsync2_H_)
#define VisibilityIteratorAsync2_H_

#include <set>
using std::set;

#include <synthesis/MSVis/AsynchronousInterface.h>
#include <synthesis/MSVis/UtilJ.h>
#include <synthesis/MSVis/VisibilityIteratorImpl2.h>

#define NotImplementedROVIA throw utilj::AipsErrorTrace (String ("Method not legal in ROVIA: ") + __PRETTY_FUNCTION__, __FILE__, __LINE__)
#define NotPrefetched throw utilj::AipsErrorTrace (String ("Column not prefetched for async I/O: ") + __PRETTY_FUNCTION__, __FILE__, __LINE__)

namespace casa {

class VisBufferAsync;
class VisBufferAsyncWrapper;

namespace asyncio {

class AsynchronousInterface;
class VlaDatum;
class VlaData;
class VLAT;
class WriteData;

} // end namespace asyncio

namespace vi {

class ViReadImplAsync2 : public VisibilityIteratorReadImpl2 {
    // This needs to be changed back to ROVisibilityIterator at some point
    // after feasibility testing

    friend class Rovia_Test;
    friend class ROVisIterator;
    friend class VisIterator;
    friend class ViWriteImplAsync2;

public:

    typedef casa::asyncio::PrefetchColumns PrefetchColumns;

//    static VisibilityIteratorReadImpl *
//    create (const ROVisibilityIterator &,
//            const PrefetchColumns & prefetchColumns,
//            Int nReadAheadBuffers = -1);

//    VisibilityIteratorReadImpl2 (ROVisibilityIterator2 * rovi,
//                                 const Block<MeasurementSet> & mss,
//                                 const Block<Int> & sortColumns,
//                                 Bool addDefaultSortCols,
//                                 Double timeInterval,
//                                 Bool createVb = True);

//    ViReadImplAsync2 (const Block<MeasurementSet> & mss,
//                     const PrefetchColumns & prefetchColumns,
//                     const Block<Int> & sortColumns,
//                     const Bool addDefaultSortCols,
//                     Double timeInterval,
//                     Bool writable)
//    : VisibilityIteratorReadImpl2 (mss, prefetchColumns, sortColumns,
//                                   addDefaultSortCols, timeInterval,
//                                   writable)
//    {} // Empty body for the moment.

    ViReadImplAsync2 (const PrefetchColumns & prefetchColumns,
                     const VisibilityIteratorReadImpl2 & other,
                     Bool writable);


    ~ViReadImplAsync2 () {} // empty body for now

    VisibilityIteratorReadImpl2 * clone () const;

    void attachVisBuffer (VisBuffer & vb);
    void detachVisBuffer (VisBuffer & vb);
    void getChannelSelection(Block< Vector<Int> >& ,
                             Block< Vector<Int> >& ,
                             Block< Vector<Int> >& ,
                             Block< Vector<Int> >& ,
                             Block< Vector<Int> >& );
    PrefetchColumns getPrefetchColumns () const;
    VisBuffer2 * getVisBuffer ();


//    Int getDataDescriptionId () const;
//    const MeasurementSet & getMeasurementSet();
//    const Int getMeasurementSetId ();
//    Int getNAntennas () const;
    MEpoch getEpoch () const;
//    Vector<Float> getReceptor0Angle ();

    /////void linkWithRovi (VisibilityIteratorReadImpl * rovi);

    bool more () const;
    bool moreChunks () const;
    ViReadImplAsync2 & nextChunk ();
    void origin ();
    void originChunks ();
    virtual void advance ();

    void setPrefetchColumns (const PrefetchColumns & prefetchColumns);

    VisibilityIteratorReadImpl2 & selectChannel(Int nGroup=1,
                                        Int start=0,
                                        Int width=0,
                                        Int increment=1,
                                        Int spectralWindow=-1);

    VisibilityIteratorReadImpl2 & selectChannel(const Block< Vector<Int> >& blockNGroup,
                                        const Block< Vector<Int> >& blockStart,
                                        const Block< Vector<Int> >& blockWidth,
                                        const Block< Vector<Int> >& blockIncr,
                                        const Block< Vector<Int> >& blockSpw);


    VisibilityIteratorReadImpl2 &
    selectVelocity(Int ,
                   const MVRadialVelocity& ,
                   const MVRadialVelocity& ,
                   MRadialVelocity::Types ,
                   MDoppler::Types , Bool );
    void setInterval(Double timeInterval);
    void setRowBlocking(Int nRow);

    // These functions generate a list of the IDs (from PrefetchColumnIDs enum)
    // of the columns to prefetch.  For the variable arg calls, terminate with a
    // -1.

    static int getDefaultNBuffers ();

    // The functions below make no sense (at first glance) for asynchronous operation and are implemented
    // to throw an AipsError if called.  ROVIA is designed to have all the data accessed through the
    // associated VisBufferAsync.  Any method which tries to access data through the ROVIA makes no
    // sense.  Also anything that tries to change the characteristics of underlying ROVI is not currently
    // permitted.  During integration some methods may be found to be more important to the use of ROVIA
    // and a way may be found to implement them in ROVIA.

    Bool allBeamOffsetsZero() const; // { NotPrefetched; }
    void allSelectedSpectralWindows(Vector<Int>& , Vector<Int>& ) { NotPrefetched; }
    void antenna1(Vector<Int>& ) const { NotPrefetched; }
    void antenna2(Vector<Int>& ) const { NotPrefetched; }
    const Vector<String> & antennaMounts() const; // { NotPrefetched; }
    Vector<MDirection> azel(Double ) const { NotImplementedROVIA; }
    MDirection azel0(Double ) const { NotImplementedROVIA; }
    void channel(Vector<Int>& ) const { NotPrefetched; }
    Int channelGroupSize() const { NotImplementedROVIA; }
    Int channelIndex() const { NotImplementedROVIA; }
    Vector<SquareMatrix<Complex,2> >& CJones(Vector<SquareMatrix<Complex,2> >& ) const { NotPrefetched; }
    void corrType(Vector<Int>& ) const { NotPrefetched; }
    Int dataDescriptionId() const { NotPrefetched; }
    Bool existsWeightSpectrum() const { NotImplementedROVIA; }
    void exposure(Vector<Double>& /*expo*/) const { NotPrefetched; }
    void feed1(Vector<Int>& ) const { NotPrefetched; }
    void feed2(Vector<Int>& ) const { NotPrefetched; }
    //Vector<Float> feed_pa(Double ) const { NotImplementedROVIA; }
    Int fieldId() const { NotPrefetched; }
    void flagCategory(Array<Bool>& /*flagCategories*/) const { NotPrefetched; }
    void floatData(Cube<Float>& /*fcube*/) const { NotPrefetched; }
    void getFloatDataColumn (Cube<Float>& /*data*/) const { NotImplementedROVIA; }
    void getFloatDataColumn(const Slicer& /*slicer*/, Cube<Float>& /*data*/) const { NotImplementedROVIA; }
    void getInterpolatedFloatDataFlagWeight() const { NotImplementedROVIA; }
    void getInterpolatedVisFlagWeight(DataColumn /*whichOne*/) const { NotImplementedROVIA; }
    Int arrayId() const { NotPrefetched; }
    String fieldName() const { NotImplementedROVIA; }
    String sourceName() const { NotImplementedROVIA; }
    void flag(Cube<Bool>& ) const { NotPrefetched; }
    void flag(Matrix<Bool>& ) const { NotPrefetched; }
    void flagRow(Vector<Bool>& ) const { NotPrefetched; }
    void frequency(Vector<Double>& ) const { NotPrefetched; }
    const Cube<RigidVector<Double, 2> >& getBeamOffsets() const;// { NotImplementedROVIA; }
    Int getDataDescriptionId () const { NotPrefetched; }
    MEpoch getMEpoch () const { NotImplementedROVIA; }
    void getMeasurementSet() const { NotImplementedROVIA; }
    Int getMeasurementSetId() const { NotImplementedROVIA; }
    Int getNAntennas () const { NotImplementedROVIA; }
    Vector<Float> getReceptor0Angle () { NotImplementedROVIA; }
    Vector<uInt> getRowIds () const { NotImplementedROVIA; }
    Double hourang(Double ) const { NotImplementedROVIA; }
    void lsrFrequency(Vector<Double>& ) const { NotImplementedROVIA; }
    void lsrFrequency(const Int& , Vector<Double>& , Bool& ) { NotImplementedROVIA; }
    const casa::MeasurementSet& ms() const  { NotImplementedROVIA; }
    void msColumns() const;
    Int msId() const;
    Int nCorr() const { NotPrefetched; }
    Int nRow() const { NotPrefetched; }
    Int nRowChunk() const;
    Int nSubInterval() const { NotImplementedROVIA; }
    Bool newArrayId() const { NotImplementedROVIA; }
    Bool newFieldId() const { NotImplementedROVIA; }
    Bool newMS() const   { return msIter_p.more();}
    Bool newSpectralWindow() const { NotImplementedROVIA; }
    Int numberCoh() { NotPrefetched; }
    Int numberDDId() { NotPrefetched; }
    Int numberPol() { NotPrefetched; }
    Int numberSpw();
    void observationId(Vector<Int>& /*obsIDs*/) const { NotPrefetched; }
    Vector<Float> parang(Double ) const { NotImplementedROVIA; }
    const casa::Float & parang0(Double ) const { NotImplementedROVIA; }
    const MDirection & phaseCenter() const { NotPrefetched; }
    Int polFrame() const  { NotPrefetched; }
    void processorId(Vector<Int>& /*procIDs*/) const { NotPrefetched; }
    Int polarizationId() const { NotPrefetched; }
    const casa::Cube<double>& receptorAngles() const; // { NotImplementedROVIA; }
    void rowIds(Vector<uInt>& ) const { NotImplementedROVIA; }
    void scan(Vector<Int>& ) const { NotPrefetched; }
    void sigma(Vector<Float>& ) const { NotPrefetched; }
    void sigmaMat(Matrix<Float>& ) const { NotPrefetched; }
    void slurp() const { NotImplementedROVIA; }
    Int spectralWindow() const { NotPrefetched; }
    void stateId(Vector<Int>& /*stateIds*/) const { NotPrefetched; }
    void time(Vector<Double>& ) const { NotPrefetched; }
    void timeCentroid(Vector<Double>& /*t*/) const { NotPrefetched; }
    void timeInterval(Vector<Double>& ) const { NotPrefetched; }
    Vector<RigidVector<Double,3> >& uvw(Vector<RigidVector<Double,3> >& ) const { NotPrefetched; }
    void uvwMat(Matrix<Double>& ) const { NotPrefetched; }
    VisibilityIteratorReadImpl2& velInterpolation(const String& ) { NotImplementedROVIA; }
    void visibility(Cube<Complex>& , DataColumn ) const { NotPrefetched; }
    void visibility(Matrix<CStokesVector>& , DataColumn ) const { NotPrefetched; }
    IPosition visibilityShape() const { NotImplementedROVIA; }
    void weight(Vector<Float>& ) const { NotPrefetched; }
    void weightMat(Matrix<Float>& ) const { NotPrefetched; }
    void weightSpectrum(Cube<Float>& ) const { NotPrefetched; }

    static String prefetchColumnName (Int id); // for debug only

protected:

    // Use the factory method "create" instead of calling the constructor
    // directly.  This allows disabling the feature.

//    ViReadImplAsync (const MeasurementSet & ms,
//                     const PrefetchColumns & prefetchColumns,
//                     const Block<Int> & sortColumns,
//                     Double timeInterval=0,
//                     Int nReadAheadBuffers = 2);
//    ViReadImplAsync (const MeasurementSet & ms,
//                     const PrefetchColumns & prefetchColumns,
//                     const Block<Int> & sortColumns,
//                     const Bool addDefaultSortCols,
//                     Double timeInterval=0,
//                     Int nReadAheadBuffers = 2);
//
//    // Same as previous constructor, but with multiple MSs to iterate over.
//
//    ViReadImplAsync (const Block<MeasurementSet> & mss,
//                     const PrefetchColumns & prefetchColumns,
//                     const Block<Int> & sortColumns,
//                     Double timeInterval=0,
//                     Int nReadAheadBuffers = 2);


    ViReadImplAsync2 (const ROVisibilityIterator & rovi,
                     const PrefetchColumns & prefetchColumns,
                     Int nReadAheadBuffers = -1);

    PrefetchColumns augmentPrefetchColumns (const PrefetchColumns & prefetchColumnsBase);
    void construct(const Block<MeasurementSet> & mss,
                   const PrefetchColumns & prefetchColumns,
                   const Block<Int> & sortColumns,
                   const Bool addDefaultSortCols,
                   Double timeInterval,
                   Bool writable);


    void fillVisBuffer();
    const MeasurementSet & getMs() const;
    void readComplete ();
    void saveMss (const Block<MeasurementSet> & mss);
    void saveMss (const MeasurementSet & ms);
    //void startVlat ();

private:

    asyncio::AsynchronousInterface * interface_p; // [own]
    Int                              msId_p;
    PrefetchColumns                  prefetchColumns_p;
    Stack<VisBufferAsyncWrapper *>   vbaWrapperStack_p;
    VisBufferAsync *                 visBufferAsync_p;
    asyncio::VlaData *               vlaData_p; // [use]
    asyncio::VLAT *                  vlat_p; // [use]

    void dumpPrefetchColumns () const;
    void updateMsd ();

    ViReadImplAsync2 (const ViReadImplAsync2 & MSI);
    ViReadImplAsync2 & operator=(const ViReadImplAsync2 &MSI);

}; // end class ViReadImplAsync


class ViWriteImplAsync2 : public VisibilityIteratorWriteImpl2 {

public:

    typedef casa::asyncio::PrefetchColumns PrefetchColumns;

    ViWriteImplAsync2 (VisibilityIterator2 * vi);
    ViWriteImplAsync2 (const PrefetchColumns & prefetchColumns,
                      const VisibilityIteratorWriteImpl2 & other,
                      VisibilityIterator2 * vi);

    ~ViWriteImplAsync2 (){} // empty body for now

    VisibilityIteratorWriteImpl2 * clone () const;

    void putModel(const RecordInterface& rec, Bool iscomponentlist=True, Bool incremental=False);


    // Set/modify the flags in the data.
    // This will flag all channels in the original data that contributed to
    // the output channel in the case of channel averaging.
    // All polarizations have the same flag value.
    void setFlag(const Matrix<Bool>& flag);
    // Set/modify the flags in the data.
    // This sets the flags as found in the MS, Cube(npol,nchan,nrow),
    // where nrow is the number of rows in the current iteration (given by
    // nRow()).
    void setFlag(const Cube<Bool>& flag);

    void setFlagCategory (const Array<Bool> & flagCategory);

    // Set/modify the flag row column; dimension Vector(nrow)
    void setFlagRow(const Vector<Bool>& rowflags);
    // Set/modify the visibilities.
    // This is possibly only for a 'reference' MS which has a new DATA column.
    // The first axis of the matrix should equal the selected number of channels
    // in the original MS.
    // If the MS does not contain all polarizations, only the parallel
    // hand polarizations are used.
    void setVis(const Matrix<CStokesVector>& vis, DataColumn whichOne);
    // Set/modify the visibilities
    // This sets the data as found in the MS, Cube(npol,nchan,nrow).
    void setVis(const Cube<Complex>& vis, DataColumn whichOne);
    // Set the visibility and flags, and interpolate from velocities if needed
    void setVisAndFlag(const Cube<Complex>& vis, const Cube<Bool>& flag,
                       DataColumn whichOne);
    // Set/modify the weights
    void setWeight(const Vector<Float>& wt);
    // Set/modify the weightMat
    void setWeightMat(const Matrix<Float>& wtmat);
    // Set/modify the weightSpectrum
    void setWeightSpectrum(const Cube<Float>& wtsp);
    // Set/modify the Sigma
    void setSigma(const Vector<Float>& sig);
    // Set/modify the ncorr x nrow SigmaMat.
    void setSigmaMat(const Matrix<Float>& sigmat);

protected:

    ViReadImplAsync2 * getReadImpl();
    void queueWriteData (const asyncio::WriteData & data);

private:


}; // end class ViWriteImplAsync2


typedef ViReadImplAsync2 ROVIA;

} // end namespace vi

} // end namespace casa

#endif // ViReadImplAsync
