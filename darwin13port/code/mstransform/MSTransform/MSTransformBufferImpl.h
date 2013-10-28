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
#include <synthesis/MSVis/VisBufferImpl2.h>

// Class containing the actual transformation logic
#include <mstransform/MSTransform/MSTransformManager.h>

namespace casa {

// Forward declarations
class MSTransformManager;

class MSTransformBufferImpl : public vi::VisBufferImpl2
{

public:

	MSTransformBufferImpl(MSTransformManager *manager);
	~MSTransformBufferImpl() {};

	void resetState();

	// Re-indexable Vectors
    const Vector<Int> & dataDescriptionIds () const; // [nR]
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
    const Array<Bool> & flagCategory () const; // [nC,nF,nCategories,nR]

	IPosition getShape () const;
	Int nRows () const;
	Int nChannels () const;
	Int nCorrelations () const;
	Int nAntennas () const;


private:

	MSTransformManager *manager_p;

	mutable Vector<Int> observationId_p;
	mutable Vector<Int> arrayId_p;
	mutable Vector<Int> scan_p;
	mutable Vector<Int> stateId_p;
	mutable Vector<Int> fieldId_p;
	mutable Vector<Int> dataDescriptionIds_p;
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
	mutable Array<Bool> flagCategory_p;
	mutable IPosition shape_p;
	mutable uInt nRows_p;
	mutable uInt nChannels_p;
	mutable uInt nCorrelations_p;
	mutable uInt nAntennas_p;

	mutable Bool observationIdOk_p;
	mutable Bool arrayIdOk_p;
	mutable Bool scanOk_p;
	mutable Bool stateIdOk_p;
	mutable Bool fieldIdOk_p;
	mutable Bool dataDescIdOk_p;
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
	mutable Bool flagCategoryOk_p;
	mutable Bool shapeOk_p;
	mutable Bool nRowsOk_p;
	mutable Bool nChannelsOk_p;
	mutable Bool nCorrelationsOk_p;
	mutable Bool nAntennasOk_p;

	mutable Bool observationIdTransformed_p;
	mutable Bool arrayIdTransformed_p;
	mutable Bool scanTransformed_p;
	mutable Bool stateIdTransformed_p;
	mutable Bool fieldIdTransformed_p;
	mutable Bool dataDescIdTransformed_p;
	mutable Bool processorIdTransformed_p;
	mutable Bool antenna1Transformed_p;
	mutable Bool antenna2Transformed_p;
	mutable Bool feed1Transformed_p;
	mutable Bool feed2Transformed_p;
	mutable Bool flagRowTransformed_p;
	mutable Bool timeTransformed_p;
	mutable Bool timeCentroidTransformed_p;
	mutable Bool timeIntervalTransformed_p;
	mutable Bool exposureTransformed_p;

};

} //# NAMESPACE CASA - END


#endif /* MSTransformBufferImpl_H_ */

