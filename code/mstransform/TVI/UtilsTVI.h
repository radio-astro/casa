//# UtilsTVI.h: This file contains the interface definition of the MSTransformManager class.
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

#ifndef UtilsTVI_H_
#define UtilsTVI_H_

// casacore containers
#include <casacore/casa/Arrays/Cube.h>
#include <casacore/casa/Arrays/Matrix.h>
#include <casacore/casa/Arrays/Vector.h>
#include <casacore/casa/Containers/Record.h>

// Measurement Set
#include <casacore/ms/MeasurementSets/MeasurementSet.h>

// Standard Lib
#include <map>
#include <float.h>

// NOTE: See implementation include below


namespace casa { //# NAMESPACE CASA - BEGIN

namespace vi { //# NAMESPACE VI - BEGIN


//////////////////////////////////////////////////////////////////////////
// DataCubeHolderBase class
//////////////////////////////////////////////////////////////////////////

class DataCubeHolderBase
{

public:

	DataCubeHolderBase() {}
	virtual ~DataCubeHolderBase() {}
	virtual void setMatrixIndex(uInt matrixIndex) = 0;
	virtual void setVectorIndex(uInt vectorIndex) = 0;
	uInt getMatrixIndex();
	uInt getVectorIndex();
	IPosition & getMatrixShape();
	IPosition & getVectorShape();

protected:

	uInt matrixIndex_p;
	uInt vectorIndex_p;
	IPosition matrixShape_p;
	IPosition vectorShape_p;
};

//////////////////////////////////////////////////////////////////////////
// DataCubeHolder class
//////////////////////////////////////////////////////////////////////////

template <class T> class DataCubeHolder : public DataCubeHolderBase
{

public:

	DataCubeHolder(Cube<T> &dataCube);
	DataCubeHolder(const Cube<T> &dataCube);

	Matrix<T> & getMatrix();
	Vector<T> & getVector();

	void setMatrixIndex(uInt matrixIndex);
	void setVectorIndex(uInt vectorIndex);


protected:

	Cube<T> cube_p;
	Matrix<T> matrix_p;
	Vector<T> vector_p;
};

//////////////////////////////////////////////////////////////////////////
// DataCubeMap class
//////////////////////////////////////////////////////////////////////////

class DataCubeMap
{

public:

	DataCubeMap();
	~DataCubeMap();

	void add(MS::PredefinedColumns key,DataCubeHolderBase* dataCubeHolder);
	void add(MS::PredefinedColumns key,DataCubeHolderBase &dataCubeHolder);

	void setWindowShape(IPosition windowShape);
	IPosition & getWindowShape();

	template <class T> Vector<T> & getVector(MS::PredefinedColumns key);
	template <class T> Matrix<T> & getMatrix(MS::PredefinedColumns key);

	void setMatrixIndex(uInt rowIndex);
	void setVectorIndex(uInt vectorIndex);

	IPosition & getMatrixShape();
	IPosition & getVectorShape();


protected:

	IPosition windowShape_p;
	std::map<MS::PredefinedColumns, DataCubeHolderBase*> dataCubeMap_p;
	std::map<MS::PredefinedColumns, DataCubeHolderBase*>::iterator dataCubeMapIter_p;
};


//////////////////////////////////////////////////////////////////////////
// Convenience methods
//////////////////////////////////////////////////////////////////////////

inline Float weightToSigma (Float weight);

inline Float sigmaToWeight (Float sigma);

void accumulateWeightCube (	const Cube<Float> &weightCube,
							const Cube<Bool> &flags,
							Matrix<Float> &result);

void accumulateWeightMatrix (	const Matrix<Float> &weightMatrix,
								const Matrix<Bool> &flags,
								Vector<Float> &result);

void accumulateFlagCube (	const Cube<Bool> &flagCube,
							Vector<Bool> &flagRow);



} //# NAMESPACE VI - END

} //# NAMESPACE CASA - END

// Include implementation file to avoid compile problems
// due to the fact that the compiler does not now which
// version of the template it has to instantiate
#include <mstransform/TVI/UtilsTVI.cc>


#endif /* UtilsTVI_H_ */

