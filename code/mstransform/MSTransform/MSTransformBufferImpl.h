//# MSTransformBufferImpl.h: This file contains the interface definition of the MSTransformBufferImpl.h class.
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

#ifndef MSTransformBufferImpl_H_
#define MSTransformBufferImpl_H_

// Where VisBufferImpl2 interface is defined
#include <msvis/MSVis/VisBufferImpl2.h>

// Class containing the actual transformation logic
#include <mstransform/MSTransform/MSTransformManager.h>

namespace casa {

class DataCubeHolderBase
{

public:

	DataCubeHolderBase() {}
	virtual ~DataCubeHolderBase() {}
	virtual void setMatrixIndex(uInt matrixIndex) = 0;
	virtual void setVectorIndex(uInt vectorIndex) = 0;
	uInt getMatrixIndex() {return matrixIndex_p;}
	uInt getVectorIndex() {return vectorIndex_p;}
	IPosition & getMatrixShape() {return matrixShape_p;}
	IPosition & getVectorShape() {return vectorShape_p;}

protected:

	uInt matrixIndex_p;
	uInt vectorIndex_p;
	IPosition matrixShape_p;
	IPosition vectorShape_p;
};

template <class T> class DataCubeHolder : public DataCubeHolderBase
{

public:

	DataCubeHolder(Cube<T> &dataCube) {cube_p.reference(dataCube);}
	~DataCubeHolder() {}

	Matrix<T> & getMatrix() {return matrix_p;}
	Vector<T> & getVector() {return vector_p;}

	void setMatrixIndex(uInt matrixIndex)
	{
		matrix_p.resize(); // Resize to 0 to avoid shape conformance problems
		matrixIndex_p = matrixIndex;
		matrix_p.reference(cube_p.xyPlane(matrixIndex));
		matrixShape_p = matrix_p.shape();
	}

	void setVectorIndex(uInt vectorIndex)
	{
		vector_p.resize(); // Resize to 0 to avoid shape conformance problems
		vectorIndex_p = vectorIndex;
		vector_p.reference(matrix_p.row(vectorIndex));
		vectorShape_p = vector_p.shape();
	}

protected:

	Cube<T> cube_p;
	Matrix<T> matrix_p;
	Vector<T> vector_p;
};

class DataCubeMap
{

public:

	DataCubeMap() {dataCubeMap_p.clear();}
	~DataCubeMap() {dataCubeMap_p.clear();}

	void add(MS::PredefinedColumns key,DataCubeHolderBase* dataCubeHolder){dataCubeMap_p[key] = dataCubeHolder;}

	void setWindowShape(IPosition windowShape) {windowShape_p = windowShape;}
	IPosition & getWindowShape() {return windowShape_p;}

	template <class T> Vector<T> & getVector(MS::PredefinedColumns key)
	{
		DataCubeHolder<T> *flagCubeHolder = static_cast< DataCubeHolder<T>* >(dataCubeMap_p[key]);
		return flagCubeHolder->getVector();
	}

	template <class T> Matrix<T> & getMatrix(MS::PredefinedColumns key)
	{
		DataCubeHolder<T> *flagCubeHolder = static_cast< DataCubeHolder<T>* >(dataCubeMap_p[key]);
		return flagCubeHolder->getVector();
	}

	void setMatrixIndex(uInt rowIndex)
	{
		for (dataCubeMapIter_p = dataCubeMap_p.begin();dataCubeMapIter_p!= dataCubeMap_p.end();dataCubeMapIter_p++)
		{
			dataCubeMapIter_p->second->setMatrixIndex(rowIndex);
		}
	}

	void setVectorIndex(uInt vectorIndex)
	{
		for (dataCubeMapIter_p = dataCubeMap_p.begin();dataCubeMapIter_p!= dataCubeMap_p.end();dataCubeMapIter_p++)
		{
			dataCubeMapIter_p->second->setVectorIndex(vectorIndex);
		}
	}

	IPosition & getMatrixShape()
	{
		return dataCubeMap_p.begin()->second->getMatrixShape();
	}

	IPosition & getVectorShape()
	{
		return dataCubeMap_p.begin()->second->getVectorShape();
	}


protected:

	IPosition windowShape_p;
	std::map<MS::PredefinedColumns, DataCubeHolderBase*> dataCubeMap_p;
	std::map<MS::PredefinedColumns, DataCubeHolderBase*>::iterator dataCubeMapIter_p;
};

typedef void (casa::MSTransformBufferImpl::*TransformFunction)(	vi::VisBuffer2 *vb,
																DataCubeMap &inputDataMap,
																DataCubeMap &outputDataMap) const;

typedef void (casa::MSTransformBufferImpl::*TransformKernel)(	vi::VisBuffer2 *vb,
																DataCubeMap &inputDataMap,
																DataCubeMap &outputDataMap,
																IPosition &inputPos,
																IPosition &outputPos,
																IPosition &kernelShape) const;

typedef void (casa::MSTransformBufferImpl::*TransformKernel1D)(	vi::VisBuffer2 *vb,
																DataCubeMap &inputDataMap,
																DataCubeMap &outputDataMap,
																uInt &inputPos,
																uInt &outputPos,
																uInt &kernelSize) const;

class MSTransformBufferImpl : public vi::VisBufferImpl2
{

public:

	MSTransformBufferImpl(MSTransformManager *manager);
	~MSTransformBufferImpl() {};

	void resetState();
	void setRowIdOffset(uInt rowOffset) {rowIdOffset_p = rowOffset;}
	void shiftRowIdOffset(Int nRows) {rowIdOffset_p += nRows;}

	void generateWeights() const;

	// Re-indexable Vectors
    const Vector<Int> & dataDescriptionIds () const; // [nR]
    const Vector<Int> & spectralWindows () const; // [nR]
    const Vector<Int> & observationId () const; // [nR]
    const Vector<Int> & arrayId () const; // [nR]
    const Vector<Int> & fieldId () const; // [nR]
    const Vector<Int> & stateId () const; // [nR]
    const Vector<Int> & antenna1 () const; // [nR]
    const Vector<Int> & antenna2 () const; // [nR]

	// Not-Re-indexable Vectors
    const Vector<Int> & scan () const; // [nR]
    const Vector<Int> & processorId () const; // [nR]
    const Vector<Int> & feed1 () const; // [nR]
    const Vector<Int> & feed2 () const; // [nR]
    const Vector<Double> & time () const; // [nR]
    const Vector<Double> & timeCentroid () const; // [nR]
    const Vector<Double> & timeInterval () const; // [nR]

    // Average-able vectors
    const Vector<Double> & exposure () const; // [nR]
    const Vector<Bool> & flagRow () const; // [nR]

    const Matrix<Double> & uvw () const; // [3,nR]
    const Matrix<Float> & weight () const; // [nC, nR]
    const Matrix<Float> & sigma () const; // [nC, nR]
    const Cube<Bool> & flagCube () const; // [nC,nF,nR]
    const Cube<Complex> & visCube () const; // [nC,nF,nR]
    const Cube<Complex> & visCubeCorrected () const; // [nC,nF,nR]
    const Cube<Complex> & visCubeModel () const; // [nC,nF,nR]
    const Cube<Float> & visCubeFloat () const; // [nC,nF,nR]
    const Cube<Float> & weightSpectrum () const; // [nC,nF,nR]
    const Cube<Float> & sigmaSpectrum () const; // [nC,nF,nR]
    const Array<Bool> & flagCategory () const; // [nC,nF,nCategories,nR]

	IPosition getShape () const;
	Int nRows () const;
	Int nChannels () const;
	Int nCorrelations () const;
	Int nAntennas () const;

	// For plotms
    const Vector<Float> & feedPa (Double time) const; // [nA]
    Float parang0(Double time) const;
    const Vector<Float> & parang(Double time) const; // [nA]
    MDirection azel0(Double time) const;
    const Vector<MDirection> & azel(Double time) const; // [nA]
    Double hourang(Double time) const;

    Vector<Int> getCorrelationTypes () const;
    const Vector<Int> & correlationTypes () const;
    Vector<Stokes::StokesTypes> getCorrelationTypesDefined () const;
    Vector<Stokes::StokesTypes> getCorrelationTypesSelected () const;

    Double getFrequency (Int rowInBuffer, Int frequencyIndex, Int frame = FrameNotSpecified) const;
    const Vector<Double> & getFrequencies (Int rowInBuffer,Int frame = FrameNotSpecified) const;
    Int getChannelNumber (Int rowInBuffer, Int frequencyIndex) const;
    const Vector<Int> & getChannelNumbers (Int rowInBuffer) const;

    const Vector<uInt> & rowIds () const;

    // Rotate visibility phase for given vector (dim = nrow of vb) of phases (metres)
    void phaseCenterShift(const Vector<Double>& phase);
    // Rotate visibility phase for phase center offsets (arcsecs)
    void phaseCenterShift(Double dx, Double dy);

    const MDirection& phaseCenter () const;
    const MFrequency::Types & freqRefFrameType () const;

protected:

    MFrequency::Convert generateFreqRefTranEngine (Double time,Int outputRefFrame,Bool toObservedFrame) const;

    void transformDataCube(	vi::VisBuffer2 *vb,
    						DataCubeMap &inputDataCubeMap,
    						DataCubeMap &outputDataCubeMap,
    						TransformFunction funcPointer) const;

    void channelAverage(	vi::VisBuffer2 *vb,
    						DataCubeMap &inputDataCubeMap,
    						DataCubeMap &outputDataCubeMap) const;

    void decimationWindow(	vi::VisBuffer2 *vb,
    						DataCubeMap &inputDataCubeMap,
    						DataCubeMap &outputDataCubeMap,
    						TransformKernel1D kernelPointer) const;

    void flagAverageKernel(	vi::VisBuffer2 *vb,
    						DataCubeMap &inputDataCubeMap,
    						DataCubeMap &outputDataCubeMap,
    						uInt &inputPos,
    						uInt &outputPos,
    						uInt &kernelSize) const;

private:

	MSTransformManager *manager_p;
	ArrayColumn<Double> spwFrequencies_p;
	map<uInt,uInt> inputOutputSPWIndexMap_p;
	uInt rowIdOffset_p;

	// OTF frequency transformation
	MDirection phaseCenter_p;
	MPosition observatoryPosition_p;
	ArrayMeasColumn<MFrequency> spwRefRame_p;

	// Phase shifting
	Bool applyPhaseShifting_p;
	Double dx_p, dy_p;

	// NONE datacol handling
	Bool noneDataCol_p;

	mutable Vector<Int> observationId_p;
	mutable Vector<Int> arrayId_p;
	mutable Vector<Int> scan_p;
	mutable Vector<Int> stateId_p;
	mutable Vector<Int> fieldId_p;
	mutable Vector<Int> dataDescriptionIds_p;
	mutable Vector<Int> spectralWindows_p;
	mutable Vector<Int> processorId_p;
	mutable Vector<Int> antenna1_p;
	mutable Vector<Int> antenna2_p;
	mutable Vector<Int> feed1_p;
	mutable Vector<Int> feed2_p;
	mutable Vector<Bool> flagRow_p;
	mutable Vector<Double> time_p;
	mutable Vector<Double> timeCentroid_p;
	mutable Vector<Double> timeInterval_p;
	mutable Vector<Double> exposure_p;
	mutable Matrix< Double> uvw_p;
	mutable Matrix<Float> weight_p;
	mutable Matrix<Float> sigma_p;
	mutable Cube<Bool> flagCube_p;
	mutable Cube<Complex> visCube_p;
	mutable Cube<Complex> visCubeCorrected_p;
	mutable Cube<Complex> visCubeModel_p;
	mutable Cube<Float> visCubeFloat_p;
	mutable Cube<Float> weightSpectrum_p;
	mutable Cube<Float> sigmaSpectrum_p;
	mutable Array<Bool> flagCategory_p;
	mutable Vector<Float> feedPa_p;
	mutable Vector<Float> parang_p;
	mutable Vector<MDirection> azel_p;
	mutable Vector<Double> frequencies_p;
	mutable Vector<Int> channelNumbers_p;
	mutable Vector<uInt> rowIds_p;
	mutable IPosition shape_p;
	mutable uInt nRows_p;
	mutable uInt nChannels_p;
	mutable uInt nCorrelations_p;
	mutable uInt nAntennas_p;
	mutable MFrequency::Types freqRefFrameType_p;

	mutable Bool observationIdOk_p;
	mutable Bool arrayIdOk_p;
	mutable Bool scanOk_p;
	mutable Bool stateIdOk_p;
	mutable Bool fieldIdOk_p;
	mutable Bool dataDescIdOk_p;
	mutable Bool spectralWindowsOk_p;
	mutable Bool processorIdOk_p;
	mutable Bool antenna1Ok_p;
	mutable Bool antenna2Ok_p;
	mutable Bool feed1Ok_p;
	mutable Bool feed2Ok_p;
	mutable Bool flagRowOk_p;
	mutable Bool timeOk_p;
	mutable Bool timeCentroidOk_p;
	mutable Bool timeIntervalOk_p;
	mutable Bool exposureOk_p;
	mutable Bool uvwOk_p;
	mutable Bool weightOk_p;
	mutable Bool sigmaOk_p;
	mutable Bool flagCubeOk_p;
	mutable Bool visCubeOk_p;
	mutable Bool visCubeCorrectedOk_p;
	mutable Bool visCubeModelOk_p;
	mutable Bool visCubeFloatOk_p;
	mutable Bool weightSpectrumOk_p;
	mutable Bool sigmaSpectrumOk_p;
	mutable Bool flagCategoryOk_p;
	mutable Bool feedPaOk_p;
	mutable Bool parangOk_p;
	mutable Bool azelOk_p;
	mutable Bool frequenciesOk_p;
	mutable Bool channelNumbersOk_p;
	mutable Bool rowIdsOk_p;
	mutable Bool shapeOk_p;
	mutable Bool nRowsOk_p;
	mutable Bool nChannelsOk_p;
	mutable Bool nCorrelationsOk_p;
	mutable Bool nAntennasOk_p;
	mutable Bool freqRefFrameTypeOk_p;

	mutable Bool observationIdTransformed_p;
	mutable Bool arrayIdTransformed_p;
	mutable Bool scanTransformed_p;
	mutable Bool stateIdTransformed_p;
	mutable Bool fieldIdTransformed_p;
	mutable Bool dataDescIdTransformed_p;
	mutable Bool spectralWindowsTransformed_p;
	mutable Bool processorIdTransformed_p;
	mutable Bool antenna1Transformed_p;
	mutable Bool antenna2Transformed_p;
	mutable Bool feed1Transformed_p;
	mutable Bool feed2Transformed_p;
	mutable Bool flagRowTransformed_p;
	mutable Bool uvwTransformed_p;
	mutable Bool weightTransformed_p;
	mutable Bool sigmaTransformed_p;
	mutable Bool timeTransformed_p;
	mutable Bool timeCentroidTransformed_p;
	mutable Bool timeIntervalTransformed_p;
	mutable Bool exposureTransformed_p;
	mutable Bool feedPaTransformed_p;
	mutable Bool parangTransformed_p;
	mutable Bool azelTransformed_p;
	mutable Bool frequenciesTransformed_p;
	mutable Bool channelNumbersTransformed_p;
	mutable Bool rowIdsTransformed_p;

};

} //# NAMESPACE CASA - END


#endif /* MSTransformBufferImpl_H_ */

