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

#ifndef UtilsTVI_CC_
#define UtilsTVI_CC_

namespace casa { //# NAMESPACE CASA - BEGIN

namespace vi { //# NAMESPACE VI - BEGIN


//////////////////////////////////////////////////////////////////////////
// DataCubeHolderBase class
//////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
uInt DataCubeHolderBase::getMatrixIndex()
{
	return matrixIndex_p;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
uInt DataCubeHolderBase::getVectorIndex()
{
	return vectorIndex_p;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
IPosition & DataCubeHolderBase::getMatrixShape()
{
	return matrixShape_p;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
IPosition & DataCubeHolderBase::getVectorShape()
{
	return vectorShape_p;
}


//////////////////////////////////////////////////////////////////////////
// DataCubeHolder class
//////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template<class T> DataCubeHolder<T>::DataCubeHolder(Cube<T> &dataCube)
{
	cube_p.reference(dataCube);
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template<class T> DataCubeHolder<T>::DataCubeHolder(const Cube<T> &dataCube)
{
	cube_p.reference(dataCube);
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template<class T> Matrix<T> & DataCubeHolder<T>::getMatrix()
{
	return matrix_p;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template<class T> Vector<T> & DataCubeHolder<T>::getVector()
{
	return vector_p;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template<class T> void DataCubeHolder<T>::setMatrixIndex(uInt matrixIndex)
{
	matrix_p.resize(); // Resize to 0 to avoid shape conformance problems
	matrixIndex_p = matrixIndex;
	matrix_p.reference(cube_p.xyPlane(matrixIndex));
	matrixShape_p = matrix_p.shape();

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template<class T> void DataCubeHolder<T>::setVectorIndex(uInt vectorIndex)
{
	vector_p.resize(); // Resize to 0 to avoid shape conformance problems
	vectorIndex_p = vectorIndex;
	vector_p.reference(matrix_p.row(vectorIndex));
	vectorShape_p = vector_p.shape();

	return;
}


//////////////////////////////////////////////////////////////////////////
// DataCubeMap class
//////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
DataCubeMap::DataCubeMap()
{
	dataCubeMap_p.clear();
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
DataCubeMap::~DataCubeMap()
{
	dataCubeMap_p.clear();
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void DataCubeMap::add(MS::PredefinedColumns key,DataCubeHolderBase* dataCubeHolder)
{
	dataCubeMap_p[key] = dataCubeHolder;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void DataCubeMap::add(MS::PredefinedColumns key,DataCubeHolderBase &dataCubeHolder)
{
	dataCubeMap_p[key] = &dataCubeHolder;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void DataCubeMap::setWindowShape(IPosition windowShape)
{
	windowShape_p = windowShape;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
IPosition & DataCubeMap::getWindowShape()
{
	return windowShape_p;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template <class T> Vector<T> & DataCubeMap::getVector(MS::PredefinedColumns key)
{
	DataCubeHolder<T> *flagCubeHolder = static_cast< DataCubeHolder<T>* >(dataCubeMap_p[key]);
	return flagCubeHolder->getVector();
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template <class T> Matrix<T> & DataCubeMap::getMatrix(MS::PredefinedColumns key)
{
	DataCubeHolder<T> *flagCubeHolder = static_cast< DataCubeHolder<T>* >(dataCubeMap_p[key]);
	return flagCubeHolder->getVector();
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void DataCubeMap::setMatrixIndex(uInt rowIndex)
{
	for (dataCubeMapIter_p = dataCubeMap_p.begin();dataCubeMapIter_p!= dataCubeMap_p.end();dataCubeMapIter_p++)
	{
		dataCubeMapIter_p->second->setMatrixIndex(rowIndex);
	}

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void DataCubeMap::setVectorIndex(uInt vectorIndex)
{
	for (dataCubeMapIter_p = dataCubeMap_p.begin();dataCubeMapIter_p!= dataCubeMap_p.end();dataCubeMapIter_p++)
	{
		dataCubeMapIter_p->second->setVectorIndex(vectorIndex);
	}

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
IPosition & DataCubeMap::getMatrixShape()
{
	return dataCubeMap_p.begin()->second->getMatrixShape();
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
IPosition & DataCubeMap::getVectorShape()
{
	return dataCubeMap_p.begin()->second->getVectorShape();
}


//////////////////////////////////////////////////////////////////////////
// Convenience methods
//////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
inline Float weightToSigma (Float weight)
{
    return weight > FLT_MIN ? 1.0 / std::sqrt (weight) : -1.0;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
inline Float sigmaToWeight (Float sigma)
{
    return sigma > FLT_MIN ? 1.0 / (sigma * sigma) : 0.0;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void accumulateWeightCube (const Cube<Float> &weightCube, const Cube<Bool> &flags, Matrix<Float> &result)
{
	IPosition shape = weightCube.shape();
	uInt nRows = shape(2);
	uInt nCorrelations = shape (0);

	result.resize(nCorrelations,nRows,False);

	Vector<Float> currentVector;
	for (uInt row=0; row < nRows; row++)
	{
		currentVector.reference(result.column(row));
		accumulateWeightMatrix (weightCube.xyPlane(row), flags.xyPlane(row),currentVector);
	}

    return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void accumulateWeightMatrix (const Matrix<Float> &weightMatrix, const Matrix<Bool> &flags, Vector<Float> &result)
{
    IPosition shape = weightMatrix.shape();
    result.resize(shape(0),False);
    Vector<uInt> samples(shape(0),0);
    uInt nCorrelations = shape (0);
    uInt nChannels = shape (1);

    for (uInt correlation = 0; correlation < nCorrelations; correlation++)
    {
        int nSamples = 0;
        float sum = 0;
        bool accumulatorFlag = True;

        for (uInt channel=0; channel< nChannels; channel++)
        {
            Bool inputFlag = flags(correlation,channel);
            // True/True or False/False
            if (accumulatorFlag == inputFlag)
            {
                nSamples ++;
                sum += weightMatrix (correlation, channel);
            }
            // True/False: Reset accumulation when accumulator switches from flagged to unflagged
            else if ( accumulatorFlag and ! inputFlag )
            {
                accumulatorFlag = False;
                nSamples = 1;
                sum = weightMatrix (correlation, channel);
            }
            // else ignore case where accumulator is valid and data is not

        }

        result (correlation) = sum / (nSamples != 0 ? nSamples : 1);
    }

    return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void accumulateFlagCube (const Cube<Bool> &flagCube, Vector<Bool> &flagRow)
{
	// Get original shape
	IPosition shape = flagCube.shape();
	size_t nCorr = shape(0);
	size_t nChan = shape(1);
	size_t nRows = shape(2);

	// Reshape flag cube to match the input shape
	flagRow.resize(nRows,False);
	flagRow = False;

	Bool rowFlagValue = False;
	for (size_t row_i =0;row_i<nRows;row_i++)
	{
		rowFlagValue = True;
		for (size_t chan_i =0;chan_i<nChan;chan_i++)
		{
			if (rowFlagValue)
			{
				for (size_t corr_i =0;corr_i<nCorr;corr_i++)
				{
					if (not flagCube(corr_i,chan_i,row_i))
					{
						rowFlagValue = False;
						break;
					}
				}
			}
			else
			{
				break;
			}
		}
		flagRow(row_i) = rowFlagValue;
	}

	return;
}


} //# NAMESPACE VI - END

} //# NAMESPACE CASA - END


#endif /* UtilsTVI_CC_ */

