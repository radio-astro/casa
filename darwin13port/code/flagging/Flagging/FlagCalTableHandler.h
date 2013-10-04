//# FlagCalTableHandler.h: This file contains the interface definition of the FlagCalTableHandler class.
//#
//#  CASA - Common Astronomy Software Applications (http://casa.nrao.edu/)
//#  Copyright (C) Associated Universities, Inc. Washington DC, USA 2011, All rights reserved.
//#  Copyright (C) European Southern Observatory, 2011, All rights reserved.
//#
//#  This library is free software; you can redistribute it and/or
//#  modify it under the terms of the GNU Lesser General Public
//#  License as published by the Free software Foundation; either
//#  version 2.1 of the License, or (at your option) any later version.
//#
//#  This library is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY, without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//#  Lesser General Public License for more details.
//#
//#  You should have received a copy of the GNU Lesser General Public
//#  License along with this library; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
//#  MA 02111-1307  USA
//# $Id: $

#ifndef FlagCalTableHandler_H_
#define FlagCalTableHandler_H_

#include <flagging/Flagging/FlagDataHandler.h>
#include <ms/MeasurementSets/MSSelectionTools.h>
#include <synthesis/CalTables/NewCalTable.h>
#include <synthesis/CalTables/CTInterface.h>
#include <synthesis/CalTables/CTIter.h>
#include <synthesis/CalTables/CalBuffer.h>

namespace casa { //# NAMESPACE CASA - BEGIN

namespace vi {

class WeightScaling;

}


class DummyBufferImpl: public vi::VisBuffer2
{
public:
	DummyBufferImpl() {}
	~DummyBufferImpl() {}

	virtual void copy (const VisBuffer2 & other, Bool fetchIfNeeded) {}
    virtual void copyComponents (const VisBuffer2 & other,
				 const VisBufferComponents2 & components,
				 Bool allowShapeChange = False,
				 Bool fetchIfNeeded = True) {}
    virtual void copyCoordinateInfo(const VisBuffer2 * other, Bool includeDirections,
   				    Bool allowShapeChange = False,
                                    Bool fetchIfNeeded = True) {}

    virtual void setShape (Int nCorrelations, Int nChannels, Int nRows, Bool) {}
    virtual const vi::VisibilityIterator2 * getVi () const {return NULL;}

    virtual Bool isAttached () const {return True;}
    virtual Bool isFillable () const {return True;}

    virtual void writeChangesBack () {}
    virtual void dirtyComponentsAdd (const VisBufferComponents2 & additionalDirtyComponents) {}
    virtual void dirtyComponentsAdd (VisBufferComponent2 component) {}
    virtual void dirtyComponentsClear () {}
    virtual VisBufferComponents2 dirtyComponentsGet () const { VisBufferComponents2 dummy; return dummy;}
    virtual void dirtyComponentsSet (const VisBufferComponents2 & dirtyComponents) {}
    virtual void dirtyComponentsSet (VisBufferComponent2 component) {}

    virtual const Matrix<Float> & imagingWeight() const {Matrix<Float> dummy; return dummy;}

    virtual Double getFrequency (Int rowInBuffer, Int frequencyIndex,
                                 Int frame = FrameNotSpecified) const {Double dummy; return dummy;}
    virtual const Vector<Double> & getFrequencies (Int rowInBuffer,
                                                 Int frame = FrameNotSpecified) const {Vector<Double> dummy; return dummy;}
    virtual Int getChannelNumber (Int rowInBuffer, Int frequencyIndex) const {Int dummy; return dummy;}
    virtual const Vector<Int> & getChannelNumbers (Int rowInBuffer) const {Vector<Int> dummy; return dummy;}

    virtual void normalize() {}

    virtual void resetWeightsUsingSigma () {}

    virtual Bool isNewArrayId () const {Bool dummy; return dummy;}
    virtual Bool isNewFieldId () const {Bool dummy; return dummy;}
    virtual Bool isNewMs() const {Bool dummy; return dummy;}
    virtual Bool isNewSpectralWindow () const {Bool dummy; return dummy;}
    virtual Bool isWritable () const {Bool dummy; return dummy;}
    virtual Int msId() const {Int dummy; return dummy;}
    virtual String msName (Bool stripPath = False) const {String dummy; return dummy;}
    virtual vi::Subchunk getSubchunk () const {vi::Subchunk dummy; return dummy;}

    virtual const Vector<Int> & antenna1 () const {Vector<Int> dummy; return dummy;}
    virtual void setAntenna1 (const Vector<Int> & value) {}
    virtual const Vector<Int> & antenna2 () const {Vector<Int> dummy; return dummy;}
    virtual void setAntenna2 (const Vector<Int> & value) {}
    virtual const Vector<Int>& arrayId () const {Vector<Int> dummy; return dummy;}
    virtual void setArrayId (const Vector<Int>&) {}
    virtual Int dataDescriptionId () const {Int dummy; return dummy;}
    virtual void setDataDescriptionId (Int value) {}
    virtual void setDataDescriptionIds(const casa::Vector<int>&){}
    virtual const Vector<MDirection> & direction1 () const {Vector<MDirection> dummy; return dummy;}
    virtual const Vector<MDirection> & direction2 () const {Vector<MDirection> dummy; return dummy;}
    virtual const Vector<Double> & exposure () const {Vector<Double> dummy; return dummy;}
    virtual void setExposure (const Vector<Double> & value) {}
    virtual const Vector<Int> & feed1 () const {Vector<Int> dummy; return dummy;}
    virtual void setFeed1 (const Vector<Int> & value) {}
    virtual const Vector<Int> & feed2 () const {Vector<Int> dummy; return dummy;}
    virtual void setFeed2 (const Vector<Int> & value) {}
    virtual const Vector<Int>& fieldId () const {Vector<Int> dummy; return dummy;}
    virtual void setFieldId (const Vector<Int>&) {}
    virtual const Matrix<Bool> & flag () const {Matrix<Bool> dummy; return dummy;}
    virtual void setFlag (const Matrix<Bool>& value) {}
    virtual const Array<Bool> & flagCategory () const {Array<Bool> dummy; return dummy;}
    virtual void setFlagCategory (const Array<Bool>& value) {}
    virtual const Cube<Bool> & flagCube () const {Cube<Bool> dummy; return dummy;}
    virtual void setFlagCube (const Cube<Bool>& value) {}
    virtual const Vector<Bool> & flagRow () const {Vector<Bool> dummy; return dummy;}
    virtual void setFlagRow (const Vector<Bool>& value) {}
    virtual const Vector<Int> & observationId () const {Vector<Int> dummy; return dummy;}
    virtual void setObservationId (const Vector<Int> & value) {}
    virtual const Vector<Int> & processorId () const {Vector<Int> dummy; return dummy;}
    virtual void setProcessorId (const Vector<Int> & value) {}
    virtual const Vector<Int> & scan () const {Vector<Int> dummy; return dummy;}
    virtual void setScan (const Vector<Int> & value) {}
    virtual const Matrix<Float> & sigma () const {Matrix<Float> dummy; return dummy;}
    virtual void setSigma (const Matrix<Float> & value) {}
    virtual const Matrix<Float> & sigmaMat () const {Matrix<Float> dummy; return dummy;}
    virtual const Vector<Int> & stateId () const {Vector<Int> dummy; return dummy;}
    virtual void setStateId (const Vector<Int> & value) {}
    virtual const Vector<Double> & time () const {Vector<Double> dummy; return dummy;}
    virtual void setTime (const Vector<Double> & value) {}
    virtual const Vector<Double> & timeCentroid () const {Vector<Double> dummy; return dummy;}
    virtual void setTimeCentroid (const Vector<Double> & value) {}
    virtual const Vector<Double> & timeInterval () const {Vector<Double> dummy; return dummy;}
    virtual void setTimeInterval (const Vector<Double> & value) {}
    virtual const Matrix<Double> & uvw () const {Matrix<Double> dummy; return dummy;}
    virtual void setUvw (const Matrix<Double> & value) {}
    virtual const Matrix<Float> & weight () const {Matrix<Float> dummy; return dummy;}
    virtual void setWeight (const Matrix<Float>& value) {}
    virtual const Matrix<Float> & weightMat () const {Matrix<Float> dummy; return dummy;}
    virtual void setWeightMat (const Matrix<Float>& value) {}
    virtual const Cube<Float> & weightSpectrum () const {Cube<Float> dummy; return dummy;}
    virtual void setWeightSpectrum (const Cube<Float>& value) {}
    virtual const Cube<float>& weightSpectrumCorrected() const {Cube<Float> dummy; return dummy;}
    virtual void setWeightSpectrumCorrected(const Cube<float>&) {}
    virtual Float getWeightCorrectedScaled(Int, Int, Int) const { return 0.0f;};

    virtual const Cube<Complex> & visCube () const {Cube<Complex> dummy; return dummy;}
    virtual void setVisCube(const Complex & c) {}
    virtual void setVisCube (const Cube<Complex> &){}
    virtual const Matrix<CStokesVector> & vis () const {Matrix<CStokesVector> dummy; return dummy;}
    virtual void setVis (Matrix<CStokesVector> &) {}
    virtual const Cube<Complex> & visCubeCorrected () const {Cube<Complex> dummy; return dummy;}
    virtual void setVisCubeCorrected (const Cube<Complex> &) {}
    virtual const Matrix<CStokesVector> & visCorrected () const {Matrix<CStokesVector> dummy; return dummy;}
    virtual void setVisCorrected (const Matrix<CStokesVector> &) {}
    virtual const Cube<Float> & visCubeFloat () const {Cube<Float> dummy; return dummy;}
    virtual void setVisCubeFloat (const Cube<Float> &) {}
    virtual const Cube<Complex> & visCubeModel () const {Cube<Complex> dummy; return dummy;}
    virtual void setVisCubeModel(const Complex & c) {}
    virtual void setVisCubeModel(const Cube<Complex>& vis) {}
    virtual void setVisCubeModel(const Vector<Float>& stokes) {}
    virtual const Matrix<CStokesVector> & visModel () const {Matrix<CStokesVector> dummy; return dummy;}
    virtual void setVisModel (Matrix<CStokesVector> &) {}

	virtual MDirection azel0 (Double time) const {MDirection dummy; return dummy;}
	virtual Vector<MDirection> & azel(Double time) const {Vector<MDirection> dummy; return dummy;}
	virtual const Vector<SquareMatrix<Complex, 2> > & cjones () const {Vector<SquareMatrix<Complex, 2> > dummy; return dummy;}
	virtual const Vector<Int> & correlationTypes () const {Vector<Int> dummy; return dummy;}
	virtual Vector<Float> & feedPa(Double time) const {Vector<Float> dummy; return dummy;}
	virtual const Vector<Float> & feedPa1 () const {Vector<Float> dummy; return dummy;}
	virtual const Vector<Float> & feedPa2 () const {Vector<Float> dummy; return dummy;}
	virtual Double hourang(Double time) const {Double dummy; return dummy;}
	virtual Int nAntennas () const {Int dummy; return dummy;}
	virtual Int nChannels () const {Int dummy; return dummy;}
	virtual Int nCorrelations () const {Int dummy; return dummy;}
	virtual Int nRows () const {Int dummy; return dummy;}
	virtual Float parang0(Double time) const {Float dummy; return dummy;}
	virtual Vector<Float> & parang(Double time) const {Vector<Float> dummy; return dummy;}
	virtual const MDirection& phaseCenter () const {MDirection dummy; return dummy;}
	virtual Int polarizationFrame () const {Int dummy; return dummy;}
	virtual Int polarizationId () const {Int dummy; return dummy;}
	virtual const Vector<uInt> & rowIds () const {Vector<uInt> dummy; return dummy;}
	virtual Int spectralWindow () const {Int dummy; return dummy;}
	virtual const Vector<Int> & spectralWindows () const {Vector<Int> dummy; return dummy;}
        virtual void setSpectralWindows (const Vector<Int> & /*spectralWindows*/) {}
        virtual Bool modelDataIsVirtual () const { return false;}


    virtual void setFillable (Bool isFillable) {}
    virtual const Vector<Int> & dataDescriptionIds () const {Vector<Int> dummy; return dummy;}
    virtual Vector<Int> getCorrelationNumbers () const {Vector<Int> dummy; return dummy;}
    virtual void validateShapes () const {}

    virtual Float getWeightScaled (Int row) const { return 0;}
    virtual Float getWeightScaled (Int correlation, Int row) const { return 0;}
    virtual Float getWeightScaled (Int correlation, Int channel, Int row) const { return 0;}
    virtual CountedPtr<vi::WeightScaling> getWeightScaling () const { return 0; }


protected:

    virtual void configureNewSubchunk (Int msId, const String & msName, Bool isNewMs,
                                       Bool isNewArrayId, Bool isNewFieldId,
                                       Bool isNewSpectralWindow, const vi::Subchunk & subchunk,
                                       Int nRows, Int nChannels, Int nCorrelations,
                                       const Vector<Int> & correlations,
                                       CountedPtr <vi::WeightScaling> weightScaling) {}
    virtual void invalidate() {}
    virtual Bool isRekeyable () const {Bool dummy; return dummy;}
    virtual void setRekeyable (Bool isRekeable) {}

};


class CTCache
{

public:
	CTCache(CTIter *calIter);
	~CTCache();

	Int arrayId();
	Int fieldId();
	Int spectralWindow();
	Vector<Int>& scan();
	Vector<Double>& time();
	Vector<Int>& antenna1();
	Vector<Int>& antenna2();
	Cube<Bool>& flagCube();
	Vector<Int>& getChannelNumbers(Int rowInBuffer);
	Vector<Double>& getFrequencies(Int rowInBuffer,Int frame = vi::VisBuffer2::FrameNotSpecified);
	Int nRowChunk();
	Int nRows();
	Int nChannels();
	Int nCorrelations();

	Cube<Complex>& visCube();
	Cube<Complex>& visCubeCorrected();
	Cube<Complex>& visCubeModel();

	// Convenient public methods for compatibility with MS-like interface
	Vector<Int>& observationId();
	Vector<Int>& correlationTypes();

	// Methods for efficient synchronization with CTIter
	void invalidate();

private:

	CTIter *calIter_p;
	Int field0_p;
	Int spw0_p;
	Vector<Int> field_p;
	Vector<Int> spw_p;
	Vector<Int> scan_p;
	Vector<Double> time_p;
	Vector<Int> antenna1_p;
	Vector<Int> antenna2_p;
	Cube<Bool> flagCube_p;
	Vector<Int> channel_p;
	Vector<Double> frequency_p;
	Cube<Complex> cparam_p;	  // This is an actual Cube<Complex>
	Cube<Complex> fparam_p;   // This is in reality a Cube<Float>
	Cube<Complex> snr_p;      // This is in reality a Cube<Float>
	Int nRowChunk_p;
	Int nRow_p;
	Int nChannel_p;
	Int nCorr_p;

	// Convenient members for compatibility with MS-like interface
	Vector<Int> observationId_p;
	Vector<Int> corrType_p;

	// Members for efficient synchronization with CTIter
	Bool CTfieldIdOK_p;
	Bool CTspectralWindowOK_p;
	Bool CTscanOK_p;
	Bool CTtimeOK_p;
	Bool CTantenna1OK_p;
	Bool CTantenna2OK_p;
	Bool CTflagCubeOk_p;
	Bool CTobservationIdOK_p;
	Bool CTcorrTypeOK_p;
	Bool CTchannelOK_p;
	Bool CTfrequencyOK_p;
	Bool CTVisCubeOK_p;
	Bool CTcorrectedVisCubeOK_p;
	Bool CTmodelVisCubeOK_p;
	Bool CTnRowChunkOK_p;
	Bool CTnRowOK_p;
	Bool CTnChannelOK_p;
	Bool CTnCorrOK_p;

	// Convenience methods and members to by-pass const methods issues
	Int& fillSpectralWindow();
	Int& fillFieldId();
	Int& fillnRowChunk();

};


class CTBuffer: public DummyBufferImpl
{

public:

	CTBuffer(CTIter *ctIter) {ctCache_p = new CTCache(ctIter);invalidate();}
	~CTBuffer() {}

	const Vector<Int> & arrayId() const {arrayId_p.assign (Vector<Int> (time().nelements(), ctCache_p->arrayId())); return arrayId_p;}
	const Vector<Int> & fieldId() const {fieldId_p.assign (Vector<Int> (time().nelements(), ctCache_p->fieldId())); return fieldId_p;}
	Int spectralWindow() const {return ctCache_p->spectralWindow();}
	const Vector<Int>& scan() const {return ctCache_p->scan();}
	const Vector<Double>& time() const {return ctCache_p->time();}
	const Vector<Int>& antenna1() const {return ctCache_p->antenna1();}
	const Vector<Int>& antenna2() const {return ctCache_p->antenna2();}
	const Cube<Bool>& flagCube() const {return ctCache_p->flagCube();}
	const Vector<Int>& getChannelNumbers(Int rowInBuffer) const {return ctCache_p->getChannelNumbers(rowInBuffer);}
	const Vector<Double>& getFrequencies(Int rowInBuffer,Int frame = vi::VisBuffer2::FrameNotSpecified) const {return ctCache_p->getFrequencies(rowInBuffer);}
	Int nRows() const {return ctCache_p->nRows();}
	Int nChannels() const {return ctCache_p->nChannels();}
	Int nCorrelations() const {return ctCache_p->nCorrelations();}
	const Vector<Int> &  spectralWindows () const {
	    spectralWindows_p.assign (Vector<Int> (time().nelements(), ctCache_p->spectralWindow())); return spectralWindows_p;}


	const Cube<Complex>& visCube() const {return ctCache_p->visCube();}
	const Cube<Complex>& visCubeCorrected() const {return ctCache_p->visCubeCorrected();}
	const Cube<Complex>& visCubeModel() const {return ctCache_p->visCubeModel();}

	// Convenient public methods for compatibility with MS-like interface
	const Vector<Int>& observationId() const {return ctCache_p->observationId();}
	const Vector<Int>& correlationTypes() const {return ctCache_p->correlationTypes();}

	// Methods for efficient synchronization with CTIter
	void invalidate() {ctCache_p->invalidate();}

        Vector<bool>& flagRowRef() {throw AipsError ("Not implemented " , __FILE__, __LINE__);}
        Cube<bool>& flagCubeRef() {throw AipsError ("Not implemented " , __FILE__, __LINE__);}
        Cube<complex<float> >& visCubeRef() {throw AipsError ("Not implemented " , __FILE__, __LINE__);}
        Cube<complex<float> >& visCubeCorrectedRef() {throw AipsError ("Not implemented " , __FILE__, __LINE__);}
        Cube<complex<float> >& visCubeModelRef() {throw AipsError ("Not implemented " , __FILE__, __LINE__);}
        Cube<float >& weightSpectrumRef() {throw AipsError ("Not implemented " , __FILE__, __LINE__);}
        IPosition getShape () const { return IPosition ();}

private:

        mutable Vector<Int> arrayId_p;
        mutable Vector<Int> fieldId_p;
        mutable Vector<Int> spectralWindows_p;
    CTIter *calIter_p;
	CTCache *ctCache_p;

};

// Flag Data Handler class definition
class FlagCalTableHandler: public FlagDataHandler
{

public:

	// Default constructor
	// NOTE: Time interval 0 groups all time steps together in one chunk.
	FlagCalTableHandler(string msname, uShort iterationApproach = SUB_INTEGRATION, Double timeInterval = 0);

	// Default destructor
	~FlagCalTableHandler();

	// Open CalTable
	bool open();

	// Close CalTable
	bool close();

	// Generate selected CalTable
	bool selectData();

	// Parse MSSelection expression
	bool parseExpression(MSSelection &parser);

	// Generate CalIter
	bool generateIterator();

	// Move to next chunk
	bool nextChunk();

	// Move to next buffer
	bool nextBuffer();

	// Write flag cube into MS
	bool flushFlags();

	// Provide table name (for flag version)
	String getTableName();

	// Check if a given column is available in the cal table
	bool checkIfColumnExists(String column);

	// Signal true when a progress summary has to be printed
	bool summarySignal();

private:

	// Mapping functions
	void generateScanStartStopMap();

	// Convenient private methods for compatibility with MS-like interface
	Block<String> getSortColumns(Block<Int> intCols);

	// CatTable selection objects
	NewCalTable *selectedCalTable_p;
	NewCalTable *originalCalTable_p;
	CTInterface *calTableInterface_p;

	// Calibration Table iterator and buffer
	CTIter *calIter_p;
	CTBuffer *calBuffer_p;

};

} //# NAMESPACE CASA - END

#endif /* FlagCalTableHandler_H_ */
