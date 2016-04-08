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

	DataCubeHolder(Cube<T> &dataCube) {cube_p.reference(dataCube);}
	DataCubeHolder(const Cube<T> &dataCube) {cube_p.reference(dataCube);}

	Matrix<T> & getMatrix() {return matrix_p;}
	Vector<T> & getVector() {return vector_p;}

	void setMatrixIndex(uInt matrixIndex)
	{
		matrix_p.resize(); // Resize to 0 to avoid shape conformance problems
		matrixIndex_p = matrixIndex;
		matrix_p.reference(cube_p.xyPlane(matrixIndex));
		matrixShape_p = matrix_p.shape();

		return;
	}

	void setVectorIndex(uInt vectorIndex)
	{
		vector_p.resize(); // Resize to 0 to avoid shape conformance problems
		vectorIndex_p = vectorIndex;
		vector_p.reference(matrix_p.row(vectorIndex));
		vectorShape_p = vector_p.shape();

		return;
	}


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

	Bool present(MS::PredefinedColumns key);

	void setWindowShape(IPosition windowShape);
	IPosition & getWindowShape();

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

	void setMatrixIndex(uInt rowIndex);
	void setVectorIndex(uInt vectorIndex);

	IPosition & getMatrixShape();
	IPosition & getVectorShape();

	size_t nelements();


protected:

	IPosition windowShape_p;
	std::map<MS::PredefinedColumns, DataCubeHolderBase*> dataCubeMap_p;
	std::map<MS::PredefinedColumns, DataCubeHolderBase*>::iterator dataCubeMapIter_p;
};


//////////////////////////////////////////////////////////////////////////
// Convenience methods
//////////////////////////////////////////////////////////////////////////

inline Float weightToSigma (Float weight)
{
	return weight > FLT_MIN ? 1.0 / std::sqrt (weight) : -1.0;
}

inline Float sigmaToWeight (Float sigma)
{
	return sigma > FLT_MIN ? 1.0 / (sigma * sigma) : 0.0;
}

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


#endif /* UtilsTVI_H_ */

