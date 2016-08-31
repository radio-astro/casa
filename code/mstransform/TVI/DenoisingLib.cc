//# DenoisingLib.h: This file contains the interface definition of the MSTransformManager class.
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

#include <mstransform/TVI/DenoisingLib.h>

namespace casa { //# NAMESPACE CASA - BEGIN

namespace denoising { //# NAMESPACE DENOISING - BEGIN

// -----------------------------------------------------------------------
// Wrap CASA Vector with a gsl_vector structure
// -----------------------------------------------------------------------
void GslVectorWrap(Vector<Double> casa_vector, gsl_vector &wrapper)
{
	wrapper.size = casa_vector.size();
	wrapper.stride = casa_vector.steps()(0);
	wrapper.data = casa_vector.data();
	wrapper.owner = false;

	return;
}

// -----------------------------------------------------------------------
//
// Wrap CASA Matrix with a gsl_matrix structure
//
// GSL Matrices are stored in row-major order, meaning that
// each row of elements forms a contiguous block in memory.
// This is the standard “C-language ordering” of two-dimensional arrays.
//
// CASA Matrices are however stored in column-major order
// so the elements of each column forms a contiguous block in memory.
//
// Therefore we have to swap rows-cols in order to match.
//
// Note that FORTRAN stores arrays in column-major order.
// -----------------------------------------------------------------------
void GslMatrixWrap(Matrix<Double> &casa_matrix, gsl_matrix &wrapper, size_t ncols)
{
    ThrowIf (not casa_matrix.contiguousStorage(),
             "Cannot map a non-contiguous CASA matrix to GSL matrix");

	wrapper.size1 = casa_matrix.ncolumn(); // Number of rows
	wrapper.size2 = ncols > 0? ncols : casa_matrix.nrow(); // Number of columns
	wrapper.tda = casa_matrix.nrow();
	wrapper.block = NULL;
	wrapper.data = casa_matrix.data();
	wrapper.owner = false;

	return;
}

//////////////////////////////////////////////////////////////////////////
// GslMultifitLinearBase class
//////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
GslMultifitLinearBase::GslMultifitLinearBase()
{
	model_p = NULL;
	ndata_p = 0;
	ncomponents_p = 0;
	max_ncomponents_p = 0;

	gsl_coeff_p = NULL;
	gsl_covariance_p = NULL;
	gsl_workspace_p = NULL;

	errno_p = 0;
	chisq_p = 0;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
GslMultifitLinearBase::GslMultifitLinearBase(GslLinearModelBase<Double> &model)
{
	setModel(model);

	errno_p = 0;
	chisq_p = 0;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
GslMultifitLinearBase::~GslMultifitLinearBase()
{
	freeGslResources();
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void GslMultifitLinearBase::allocGslResources()
{
	gsl_covariance_p = gsl_matrix_alloc (ncomponents_p, ncomponents_p);
	gsl_workspace_p = gsl_multifit_linear_alloc (ndata_p, ncomponents_p);
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void GslMultifitLinearBase::freeGslResources()
{
	if (gsl_covariance_p != NULL) gsl_matrix_free (gsl_covariance_p);
	if (gsl_workspace_p != NULL) gsl_multifit_linear_free (gsl_workspace_p);
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void GslMultifitLinearBase::setModel(GslLinearModelBase<Double> &model)
{
	model_p = &model;
	ndata_p = model_p->ndata();
	max_ncomponents_p = model_p->ncomponents();

	ncomponents_p = max_ncomponents_p;
	GslMatrixWrap(model_p->getModelMatrix(),gsl_model_p);

	data_p.resize(ndata_p,1,False);

	allocGslResources();
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void GslMultifitLinearBase::resetNComponents(size_t ncomponents)
{
    ThrowIf (ncomponents > max_ncomponents_p,
             "Maximum number of components is " + max_ncomponents_p);

	ncomponents_p = ncomponents;
	GslMatrixWrap(model_p->getModelMatrix(),gsl_model_p,ncomponents_p);

	freeGslResources();
	allocGslResources();
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void GslMultifitLinearBase::resetModel(GslLinearModelBase<Double> &model)
{
	freeGslResources();
	setModel(model);
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void GslMultifitLinearBase::setData(Vector<Double> &data)
{
    ThrowIf (data.size() != ndata_p,"Size of data does not match model");

    data_p.column(0).reference(data);
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void GslMultifitLinearBase::setData(Vector<Float> &data)
{
	ThrowIf (data.size() != ndata_p,"Size of data does not match model");

	for (size_t idx=0;idx<ndata_p;idx++)
	{
		data_p(idx,0) = data(idx);
	}
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void GslMultifitLinearBase::setData(Vector<Complex> &data)
{
	ThrowIf (data.size() != ndata_p,"Size of data does not match model");

	if (data_p.ncolumn() != 2) data_p.resize(ndata_p,2,False);

	for (size_t idx=0;idx<ndata_p;idx++)
	{
		data_p(idx,0) = real(data(idx));
		data_p(idx,1) = imag(data(idx));
	}
}

// -----------------------------------------------------------------------
//
// Perform least-squares fits to a general linear model, y = X c where
// y is a vector of n observations, X is an n by p matrix of predictor variables,
// and the elements of the vector c are the p unknown best-fit parameters which
// are to be estimated.
//
// NOTE: gsl_multifit_linear expects that the model matrix is organized as follows
// X = [ 1   , x_1  , x_1^2 , ..., x_1^order;
//       1   , x_2  , x_2^2 , ..., x_2^order;
//       1   , x_3  , x_3^2 , ..., x_3^order;
//       ... , ...  , ...   , ..., ...
//       1   , x_n  , x_n^2 , ..., x_n^order]
//
// -----------------------------------------------------------------------
gsl_vector* GslMultifitLinearBase::calcFitCoeffCore(Vector<Double> data)
{
	// Wrap data vector in a gsl_vector
	gsl_vector data_gsl;
	GslVectorWrap(data,data_gsl);

	// Allocate vector of coeff
	gsl_vector* coeff = gsl_vector_alloc (ncomponents_p);

	// Perform coeff calculation
	errno_p = gsl_multifit_linear (&gsl_model_p, &data_gsl,
			coeff, gsl_covariance_p, &chisq_p, gsl_workspace_p);

	return coeff;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
Vector<Complex> GslMultifitLinearBase::calcFitCoeff(Vector<Complex> &data)
{
	// Store input data as double
	setData(data);

	// Call fit method to calculate real/imag coefficients
	gsl_vector *real_coeff = calcFitCoeffCore(data_p.column(0));
	gsl_vector *imag_coeff = calcFitCoeffCore(data_p.column(1));

	// Get imag coefficients
	Vector<Complex> fitCoeff(ncomponents_p);
	for (size_t coeff_idx=0;coeff_idx<ncomponents_p;coeff_idx++)
	{
		fitCoeff(coeff_idx) = Complex(	gsl_vector_get(real_coeff,coeff_idx),
										gsl_vector_get(imag_coeff,coeff_idx));
	}

	gsl_vector_free (real_coeff);
	gsl_vector_free (imag_coeff);

	return fitCoeff;
}

//////////////////////////////////////////////////////////////////////////
// GslMultifitWeightedLinear class
//////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
GslMultifitWeightedLinear::GslMultifitWeightedLinear() :
		GslMultifitLinearBase()
{

}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
GslMultifitWeightedLinear::GslMultifitWeightedLinear(GslLinearModelBase<Double> &model) :
		GslMultifitLinearBase()
{
	setModel(model);
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
GslMultifitWeightedLinear::~GslMultifitWeightedLinear()
{

}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void GslMultifitWeightedLinear::setModel(GslLinearModelBase<Double> &model)
{
	GslMultifitLinearBase::setModel(model);
	weights_p.resize(ndata_p,False);
	GslVectorWrap(weights_p,gls_weights_p);
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void GslMultifitWeightedLinear::setWeights(Vector<Float> &weights)
{
	// Dim check
	ThrowIf (weights.size() != ndata_p,"Size of weights does not match model");

	// Fill in
	for (size_t idx=0;idx<ndata_p;idx++)
	{
		weights_p(idx) = weights(idx);
	}
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void GslMultifitWeightedLinear::setFlags(Vector<Bool> &flags, Bool goodIsTrue)
{
	// Dim check
	ThrowIf (flags.size() != ndata_p,"Size of flags does not match model");

	// Fill in
	if (goodIsTrue)
	{
		for (size_t idx=0;idx<ndata_p;idx++)
		{
			weights_p(idx) = flags(idx);
		}
	}
	else
	{
		for (size_t idx=0;idx<ndata_p;idx++)
		{
			weights_p(idx) = !flags(idx);
		}
	}
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void GslMultifitWeightedLinear::setWeightsAndFlags(Vector<Float> &weights, Vector<Bool> &flags, Bool goodIsTrue)
{
	// Dim check
	ThrowIf (weights.size() != ndata_p,"Size of weights does not match model");
	ThrowIf (flags.size() != ndata_p,"Size of flags does not match model");

	// Fill in
	if (goodIsTrue)
	{
		for (size_t idx=0;idx<ndata_p;idx++)
		{
			weights_p(idx) = weights(idx)*flags(idx);
		}
	}
	else
	{
		for (size_t idx=0;idx<ndata_p;idx++)
		{
			weights_p(idx) = weights(idx)*!flags(idx);
		}
	}
}

// -----------------------------------------------------------------------
//
// Perform least-squares fits to a general linear weighted model, y = X c where
// y is a vector of n observations, with weights w, X is an n by p matrix of
// predictor variables, and the elements of the vector c are the p unknown best-fit
// parameters which are to be estimated.
//
// NOTE: gsl_multifit_linear expects that the model matrix is organized as follows
// X = [ 1   , x_1  , x_1^2 , ..., x_1^order;
//       1   , x_2  , x_2^2 , ..., x_2^order;
//       1   , x_3  , x_3^2 , ..., x_3^order;
//       ... , ...  , ...   , ..., ...
//       1   , x_n  , x_n^2 , ..., x_n^order]
//
// NOTE: More than one data series can use the same weights / workspace
//       Therefore input data is a matrix where each row represents a data series
//
// -----------------------------------------------------------------------
gsl_vector* GslMultifitWeightedLinear::calcFitCoeffCore(Vector<Double> data)
{
	// Wrap data vector in a gsl_vector
	gsl_vector data_gsl;
	GslVectorWrap(data,data_gsl);

	// Allocate vector of coeff
	gsl_vector* coeff = gsl_vector_alloc (ncomponents_p);

	// Perform coeff calculation
	errno_p = gsl_multifit_wlinear (&gsl_model_p, &gls_weights_p, &data_gsl,
			coeff, gsl_covariance_p, &chisq_p, gsl_workspace_p);

	return coeff;
}

} //# NAMESPACE DENOISING - END

} //# NAMESPACE CASA - END

