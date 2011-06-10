/*
 * VisibilityIteratorAsync.h
 *
 *  Created on: Nov 1, 2010
 *      Author: jjacobs
 */

#if ! defined (VisibilityIteratorAsync_H_)
#define VisibilityIteratorAsync_H_

#include <set>
using std::set;

#include "VisibilityIterator.h"
#include "UtilJ.h"


#define NotImplementedROVIA throw utilj::AipsErrorTrace (String ("Method not legal in ROVIA: ") + __PRETTY_FUNCTION__, __FILE__, __LINE__)

namespace casa {


class VisBufferAsync;
class VisBufferAsyncWrapper;
class ROVisibilityIteratorAsyncImpl;
class VlaDatum;

namespace asyncio {


typedef enum   {Ant1,
                Ant2,
                ArrayId,
                Channel,
                Cjones,
                CorrType,
                Corrected,
                CorrectedCube,
                Direction1,
                Direction2,
                Exposure,
                Feed1,
                Feed1_pa,
                Feed2,
                Feed2_pa,
                FieldId,
                Flag,
                FlagCategory,
                FlagCube,
                FlagRow,
                Freq,
                ImagingWeight,
                LSRFreq,
                Model,
                ModelCube,
                NChannel,
                NCorr,
                NRow,
                ObservationId,
                Observed,
                ObservedCube,
                PhaseCenter,
                PolFrame,
                ProcessorId,
                Scan,
                Sigma,
                SigmaMat,
                SpW,
                StateId,
                Time,
                TimeCentroid,
                TimeInterval,
                Weight,
                WeightMat,
                WeightSpectrum,
                Uvw,
                UvwMat,
                N_PrefetchColumnIds // must be last
            } PrefetchColumnIds;

class PrefetchColumns : public std::set<PrefetchColumnIds>  {};


} // end namespace asyncio


class ROVisibilityIteratorAsync : public ROVisibilityIterator {
    // This needs to be changed back to ROVisibilityIterator at some point
    // after feasibility testing

public:



    typedef casa::asyncio::PrefetchColumns PrefetchColumns;

    // ROVisibilityIteratorAsync ();

    static ROVisibilityIterator *
    create (const MeasurementSet & ms,
            const PrefetchColumns & prefetchColumns,
            const Block<Int> & sortColumns,
            Double timeInterval=0,
            Int nReadAheadBuffers = -1);

    static ROVisibilityIterator *
    create (const MeasurementSet & ms,
            const PrefetchColumns & prefetchColumns,
            const Block<Int> & sortColumns,
            const Bool addDefaultSortCols,
            Double timeInterval=0,
            Int nReadAheadBuffers = -1);

    static ROVisibilityIterator *
    create (const Block<MeasurementSet> & mss,
            const PrefetchColumns & prefetchColumns,
            const Block<Int> & sortColumns,
            Double timeInterval=0,
            Int nReadAheadBuffers = -1);

    static ROVisibilityIterator *
    create (const Block<MeasurementSet> & mss,
            const PrefetchColumns & prefetchColumns,
            const Block<Int> & sortColumns,
            const Bool addDefaultSortCols,
            Double timeInterval=0,
            Int nReadAheadBuffers = -1);

    static ROVisibilityIterator *
    create (const ROVisibilityIterator &,
            const PrefetchColumns & prefetchColumns,
            Int nReadAheadBuffers = -1);

    ~ROVisibilityIteratorAsync ();

    ROVisibilityIteratorAsync & operator++ ();
    ROVisibilityIteratorAsync & operator++ (int);

    void attachVisBuffer (VisBuffer & vb);
    void detachVisBuffer (VisBuffer & vb);
    void getChannelSelection(Block< Vector<Int> >& ,
                             Block< Vector<Int> >& ,
                             Block< Vector<Int> >& ,
                             Block< Vector<Int> >& ,
                             Block< Vector<Int> >& );
    PrefetchColumns getPrefetchColumns () const;
    VisBuffer * getVisBuffer ();


//    Int getDataDescriptionId () const;
//    const MeasurementSet & getMeasurementSet();
//    const Int getMeasurementSetId ();
//    Int getNAntennas () const;
//    MEpoch getMEpoch () const;
//    Vector<Float> getReceptor0Angle ();

    void linkWithRovi (ROVisibilityIterator * rovi);
    bool more () const;
    bool moreChunks () const;
    ROVisibilityIteratorAsync & nextChunk ();
    void origin ();
    void originChunks ();

    void setPrefetchColumns (const PrefetchColumns & prefetchColumns);

    ROVisibilityIterator& selectChannel(Int nGroup=1,
                                        Int start=0,
                                        Int width=0,
                                        Int increment=1,
                                        Int spectralWindow=-1);

    ROVisibilityIterator& selectChannel(Block< Vector<Int> >& blockNGroup,
                                        Block< Vector<Int> >& blockStart,
                                        Block< Vector<Int> >& blockWidth,
                                        Block< Vector<Int> >& blockIncr,
                                        Block< Vector<Int> >& blockSpw);


    ROVisibilityIterator&
    selectVelocity(Int ,
                   const MVRadialVelocity& ,
                   const MVRadialVelocity& ,
                   MRadialVelocity::Types ,
                   MDoppler::Types , Bool );
    void setInterval(Double timeInterval);
    void setRowBlocking(Int nRow);


    //

    // These functions generate a list of the IDs (from PrefetchColumnIDs enum)
    // of the columns to prefetch.  For the variable arg calls, terminate with a
    // -1.

    static int getDefaultNBuffers ();
    static String getAipsRcBase();
    static Bool isAsynchronousIoEnabled ();
    static PrefetchColumns prefetchColumnsAll ();
    static PrefetchColumns prefetchAllColumnsExcept (int firstColumnId, ...);
    static PrefetchColumns prefetchColumns (int firstColumnId, ...);

    // The functions below make no sense (at first glance) for asynchronous operation and are implemented
    // to throw an AipsError if called.  ROVIA is designed to have all the data accessed through the
    // associated VisBufferAsync.  Any method which tries to access data through the ROVIA makes no
    // sense.  Also anything that tries to change the characteristics of underlying ROVI is not currently
    // permitted.  During integration some methods may be found to be more important to the use of ROVIA
    // and a way may be found to implement them in ROVIA.

    Bool allBeamOffsetsZero() const { NotImplementedROVIA; }
    void allSelectedSpectralWindows(Vector<Int>& , Vector<Int>& ) { NotImplementedROVIA; }
    Vector<Int>& antenna1(Vector<Int>& ) const { NotImplementedROVIA; }
    Vector<Int>& antenna2(Vector<Int>& ) const { NotImplementedROVIA; }
    const Vector<String>& antennaMounts() const { NotImplementedROVIA; }
    Vector<MDirection> azel(Double ) const { NotImplementedROVIA; }
    MDirection azel0(Double ) const { NotImplementedROVIA; }
    Vector<Int>& channel(Vector<Int>& ) const { NotImplementedROVIA; }
    Int channelGroupSize() const { NotImplementedROVIA; }
    Int channelIndex() const { NotImplementedROVIA; }
    Vector<SquareMatrix<Complex,2> >& CJones(Vector<SquareMatrix<Complex,2> >& ) const { NotImplementedROVIA; }
    Vector<Int>& corrType(Vector<Int>& ) const { NotImplementedROVIA; }
    Int dataDescriptionId() const { NotImplementedROVIA; }
    Bool existsWeightSpectrum() const { NotImplementedROVIA; }
    Vector<Double>& exposure(Vector<Double>& /*expo*/) const { NotImplementedROVIA; }
    Vector<Int>& feed1(Vector<Int>& ) const { NotImplementedROVIA; }
    Vector<Int>& feed2(Vector<Int>& ) const { NotImplementedROVIA; }
    Vector<Float> feed_pa(Double ) const { NotImplementedROVIA; }
    Int fieldId() const { NotImplementedROVIA; }
    Array<Bool>& flagCategory(Array<Bool>& /*flagCategories*/) const { NotImplementedROVIA; }
    Cube<Float>& floatData(Cube<Float>& /*fcube*/) const { NotImplementedROVIA; }
    void getFloatDataColumn (Cube<Float>& /*data*/) const { NotImplementedROVIA; }
    void getFloatDataColumn(const Slicer& /*slicer*/, Cube<Float>& /*data*/) const { NotImplementedROVIA; }
    void getInterpolatedFloatDataFlagWeight() const { NotImplementedROVIA; }
    void getInterpolatedVisFlagWeight(DataColumn /*whichOne*/) const { NotImplementedROVIA; }
    Int arrayId() const { NotImplementedROVIA; }
    String fieldName() const { NotImplementedROVIA; }
    String sourceName() const { NotImplementedROVIA; }
    Cube<Bool>& flag(Cube<Bool>& ) const { NotImplementedROVIA; }
    Matrix<Bool>& flag(Matrix<Bool>& ) const { NotImplementedROVIA; }
    Vector<Bool>& flagRow(Vector<Bool>& ) const { NotImplementedROVIA; }
    Vector<Double>& frequency(Vector<Double>& ) const { NotImplementedROVIA; }
    const Cube<RigidVector<Double, 2> >& getBeamOffsets() const { NotImplementedROVIA; }
    Int getDataDescriptionId () const { NotImplementedROVIA; }
    MEpoch getMEpoch () const { NotImplementedROVIA; }
    const MeasurementSet & getMeasurementSet() const { NotImplementedROVIA; }
    Int getMeasurementSetId() const { NotImplementedROVIA; }
    Int getNAntennas () const { NotImplementedROVIA; }
    Vector<Float> getReceptor0Angle () { NotImplementedROVIA; }
    Vector<uInt> getRowIds () const { NotImplementedROVIA; }
    Double hourang(Double ) const { NotImplementedROVIA; }
    Vector<Double>& lsrFrequency(Vector<Double>& ) const { NotImplementedROVIA; }
    void lsrFrequency(const Int& , Vector<Double>& , Bool& ) { NotImplementedROVIA; }
    const MeasurementSet& ms() const  { NotImplementedROVIA; }
    const ROMSColumns& msColumns() const { NotImplementedROVIA; }
    Int msId() const  { NotImplementedROVIA; }
    Int nCorr() const { NotImplementedROVIA; }
    Int nRow() const { NotImplementedROVIA; }
    Int nRowChunk() const { NotImplementedROVIA; }
    Int nSubInterval() const { NotImplementedROVIA; }
    Bool newArrayId() const { NotImplementedROVIA; }
    Bool newFieldId() const { NotImplementedROVIA; }
    Bool newMS() const   { return msIter_p.more();}
    Bool newSpectralWindow() const { NotImplementedROVIA; }
    Int numberCoh() { NotImplementedROVIA; }
    Int numberDDId() { NotImplementedROVIA; }
    Int numberPol() { NotImplementedROVIA; }
    Int numberSpw() { NotImplementedROVIA; }
    Vector<Int>& observationId(Vector<Int>& /*obsIDs*/) const { NotImplementedROVIA; }
    Vector<Float> parang(Double ) const { NotImplementedROVIA; }
    const Float& parang0(Double ) const { NotImplementedROVIA; }
    const MDirection& phaseCenter() const { NotImplementedROVIA; }
    Int polFrame() const  { NotImplementedROVIA; }
    Vector<Int>& processorId(Vector<Int>& /*procIDs*/) const { NotImplementedROVIA; }
    Int polarizationId() const { NotImplementedROVIA; }
    const Cube<Double>& receptorAngles() const { NotImplementedROVIA; }
    Vector<uInt>& rowIds(Vector<uInt>& ) const { NotImplementedROVIA; }
    Vector<Int>& scan(Vector<Int>& ) const { NotImplementedROVIA; }
    Vector<Float>& sigma(Vector<Float>& ) const { NotImplementedROVIA; }
    Matrix<Float>& sigmaMat(Matrix<Float>& ) const { NotImplementedROVIA; }
    void slurp() const { NotImplementedROVIA; }
    Int spectralWindow() const { NotImplementedROVIA; }
    Vector<Int>& stateId(Vector<Int>& /*stateIds*/) const { NotImplementedROVIA; }
    Vector<Double>& time(Vector<Double>& ) const { NotImplementedROVIA; }
    Vector<Double>& timeCentroid(Vector<Double>& /*t*/) const { NotImplementedROVIA; }
    Vector<Double>& timeInterval(Vector<Double>& ) const { NotImplementedROVIA; }
    Vector<RigidVector<Double,3> >& uvw(Vector<RigidVector<Double,3> >& ) const { NotImplementedROVIA; }
    Matrix<Double>& uvwMat(Matrix<Double>& ) const { NotImplementedROVIA; }
    ROVisibilityIterator& velInterpolation(const String& ) { NotImplementedROVIA; }
    Cube<Complex>& visibility(Cube<Complex>& , DataColumn ) const { NotImplementedROVIA; }
    Matrix<CStokesVector>& visibility(Matrix<CStokesVector>& , DataColumn ) const { NotImplementedROVIA; }
    IPosition visibilityShape() const { NotImplementedROVIA; }
    Vector<Float>& weight(Vector<Float>& ) const { NotImplementedROVIA; }
    Matrix<Float>& weightMat(Matrix<Float>& ) const { NotImplementedROVIA; }
    Cube<Float>& weightSpectrum(Cube<Float>& ) const { NotImplementedROVIA; }

    static String prefetchColumnName (Int id); // for debug only

protected:

    // Use the factory method "create" instead of calling the constructor
    // directly.  This allows disabling the feature.

    ROVisibilityIteratorAsync (const MeasurementSet & ms,
                               const PrefetchColumns & prefetchColumns,
                               const Block<Int> & sortColumns,
                               Double timeInterval=0,
                               Int nReadAheadBuffers = 2);
    ROVisibilityIteratorAsync (const MeasurementSet & ms,
                               const PrefetchColumns & prefetchColumns,
                               const Block<Int> & sortColumns,
                               const Bool addDefaultSortCols,
                               Double timeInterval=0,
                               Int nReadAheadBuffers = 2);

    // Same as previous constructor, but with multiple MSs to iterate over.

    ROVisibilityIteratorAsync (const Block<MeasurementSet> & mss,
                               const PrefetchColumns & prefetchColumns,
                               const Block<Int> & sortColumns,
                               Double timeInterval=0,
                               Int nReadAheadBuffers = 2);
    ROVisibilityIteratorAsync (const Block<MeasurementSet> & mss,
                               const PrefetchColumns & prefetchColumns,
                               const Block<Int> & sortColumns,
                               const Bool addDefaultSortCols,
                               Double timeInterval=0,
                               Int nReadAheadBuffers = 2);

    ROVisibilityIteratorAsync (const ROVisibilityIterator & rovi,
                               const PrefetchColumns & prefetchColumns,
                               Int nReadAheadBuffers = -1);

    void advance ();
    void construct (const PrefetchColumns & prefetchColumns, Int nReadAheadBuffers);
    void fillVisBuffer();
    void readComplete ();
    void saveMss (const Block<MeasurementSet> & mss);
    void saveMss (const MeasurementSet & ms);
    void startVlat ();

private:

    Int chunkNumber_p;
    ROVisibilityIteratorAsyncImpl * impl_p;
    ROVisibilityIterator * linkedVisibilityIterator_p; // [use]
    PrefetchColumns prefetchColumns_p;
    Int subChunkNumber_p;
    VisBufferAsync * visBufferAsync_p;
    Stack<VisBufferAsyncWrapper *> vbaWrapperStack_p;

    void dumpPrefetchColumns () const;

    ROVisibilityIteratorAsync (const ROVisibilityIteratorAsync & MSI);
    ROVisibilityIteratorAsync & operator=(const ROVisibilityIteratorAsync &MSI);

};

namespace asyncio {

class ChannelSelection {

public:

    ChannelSelection () {}

    ChannelSelection (const Block< Vector<Int> > & blockNGroup,
                      const Block< Vector<Int> > & blockStart,
                      const Block< Vector<Int> > & blockWidth,
                      const Block< Vector<Int> > & blockIncr,
                      const Block< Vector<Int> > & blockSpw);

    ChannelSelection (const ChannelSelection & other);
    ChannelSelection & operator= (const ChannelSelection & other);


    void
    get (Block< Vector<Int> > & blockNGroup,
         Block< Vector<Int> > & blockStart,
         Block< Vector<Int> > & blockWidth,
         Block< Vector<Int> > & blockIncr,
         Block< Vector<Int> > & blockSpw) const;

private:

    Block< Vector<Int> > blockNGroup_p;
    Block< Vector<Int> > blockStart_p;
    Block< Vector<Int> > blockWidth_p;
    Block< Vector<Int> > blockIncr_p;
    Block< Vector<Int> > blockSpw_p;
};


class RoviaModifier {
public:

    friend std::ostream & operator<< (std::ostream & o, const RoviaModifier & m);

    virtual ~RoviaModifier () {}
    virtual void apply (ROVisibilityIterator *) const = 0;

private:

    virtual void print (std::ostream & o) const = 0;

};

class SelectChannelModifier : public RoviaModifier {

public:

    SelectChannelModifier (Int nGroup, Int start, Int width, Int increment, Int spectralWindow);
    SelectChannelModifier (const Block< Vector<Int> > & blockNGroup,
                           const Block< Vector<Int> > & blockStart,
                           const Block< Vector<Int> > & blockWidth,
                           const Block< Vector<Int> > & blockIncr,
                           const Block< Vector<Int> > & blockSpw);

    void apply (ROVisibilityIterator *) const;

private:

    Bool channelBlocks_p;
    ChannelSelection channelSelection_p;
    Int increment_p;
    Int nGroup_p;
    Int spectralWindow_p;
    Int start_p;
    Int width_p;

    void print (std::ostream & o) const;
    String toCsv (const Block< Vector<Int> > & bv) const;
    String toCsv (const Vector<Int> & v) const;

};

class SetIntervalModifier : public RoviaModifier {

public:

    SetIntervalModifier  (Double timeInterval);
    void apply (ROVisibilityIterator *) const;

private:

    Double timeInterval_p;

    void print (std::ostream & o) const;
};


class SetRowBlockingModifier : public RoviaModifier {

public:

    SetRowBlockingModifier (Int nRows);
    void apply (ROVisibilityIterator *) const;

private:

    Int nRows_p;
    Int nGroup_p;
    Int spectralWindow_p;
    Int start_p;
    Int width_p;

    void print (std::ostream & o) const;
};

class RoviaModifiers {

public:

    ~RoviaModifiers ();

    void add (RoviaModifier *);
    void apply (ROVisibilityIterator *);
    void clear ();
    void transfer (RoviaModifiers & other);

private:

    typedef vector<RoviaModifier *> Data;
    Data data_p;

};

class SelectVelocityModifier : public RoviaModifier {

public:

    SelectVelocityModifier (Int nChan, const MVRadialVelocity& vStart, const MVRadialVelocity& vInc,
                            MRadialVelocity::Types rvType, MDoppler::Types dType, Bool precise);
    void apply (ROVisibilityIterator *) const;

private:

    MDoppler::Types dType_p;
    Int nChan_p;
    Bool precise_p;
    MRadialVelocity::Types rvType_p;
    MVRadialVelocity vInc_p;
    MVRadialVelocity vStart_p;

    virtual void print (std::ostream & o) const;

};

} // end namespace asyncio

typedef ROVisibilityIteratorAsync ROVIA;

} // end namespace casa

#endif // ROVisibilityIteratorAsync
