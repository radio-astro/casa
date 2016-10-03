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

#ifndef DenoisingLib_H_
#define DenoisingLib_H_

// casacore containers
#include <casacore/casa/Arrays/Matrix.h>
#include <casacore/casa/Arrays/Vector.h>

// GSL includes
#include <gsl/gsl_multifit.h>


namespace casa { //# NAMESPACE CASA - BEGIN

namespace denoising { //# NAMESPACE DENOISING - BEGIN

void GslVectorWrap(casacore::Vector<casacore::Double> casa_vector, gsl_vector &wrapper);
void GslMatrixWrap(casacore::Matrix<casacore::Double> &casa_matrix, gsl_matrix &wrapper, size_t ncols=0);

//////////////////////////////////////////////////////////////////////////
// GslLinearModelBase class
//////////////////////////////////////////////////////////////////////////

template<class T> class GslLinearModelBase
{

public:

	GslLinearModelBase(size_t ndata, size_t ncomponents)
	{
		ndata_p = ndata;
		ncomponents_p = ncomponents;
		model_p.resize(ncomponents_p,ndata_p,casacore::False);
	}

	size_t ndata() {return ndata_p;}
	size_t ncomponents() {return ncomponents_p;}
	casacore::Matrix<casacore::Double>& getModelMatrix(){return model_p;}

protected:

	casacore::Matrix<T> model_p;
	size_t ndata_p;
	size_t ncomponents_p;
};

//////////////////////////////////////////////////////////////////////////
// GslPolynomialModel class
//////////////////////////////////////////////////////////////////////////

template<class T> class GslPolynomialModel : public GslLinearModelBase<T>
{
	using GslLinearModelBase<T>::model_p;
	using GslLinearModelBase<T>::ndata_p;
	using GslLinearModelBase<T>::ncomponents_p;

public:

	GslPolynomialModel(size_t ndata, size_t order) :
		GslLinearModelBase<T>(ndata,order+1)
	{
		// Initialize model
		model_p.row(0) = 1.0;

		// Populate linear components
		if (order > 0)
		{
			casacore::Vector<T> linearComponent;
			linearComponent.reference(model_p.row(1));
			indgen(linearComponent,-1.0,2.0/(ndata_p-1));
		}

		// Populate higher orders
		for (size_t model_idx=2;model_idx<ncomponents_p;model_idx++)
		{
			for (size_t data_idx=0; data_idx<ndata_p; data_idx++)
			{
				model_p(model_idx,data_idx) = pow(model_p(1,data_idx),model_idx);
			}
		}
	}

	GslPolynomialModel(const casacore::Vector<T> &data_x, size_t order) :
		GslLinearModelBase<T>(data_x.size(),order+1)
	{
		// Calculate scale
		T min,max,mid,scale;
		minMax(min,max,data_x);
		mid = 0.5*(min+max);
		scale = 1.0 / (min-mid);

		// Populate linear components
		model_p.row(0) = 1.0;
		if (order > 0) model_p.row(1) = scale*(data_x-mid);

		// Populate higher orders
		for (size_t model_idx=2;model_idx<ncomponents_p;model_idx++)
		{
			for (size_t data_idx=0; data_idx<ndata_p; data_idx++)
			{
				model_p(model_idx,data_idx) = pow(model_p(1,data_idx),model_idx);
			}
		}
	}

	casacore::Vector<casacore::Float>& getLinearComponentFloat()
	{
		if (linear_component_float_p.size() != ndata_p) initLinearComponentFloat();
		return linear_component_float_p;
	}

private:

	void initLinearComponentFloat()
	{
		linear_component_float_p.resize(ndata_p);
		for (size_t data_idx=0; data_idx<ndata_p; data_idx++)
		{
			linear_component_float_p(data_idx) = model_p(1,data_idx);
		}
	}

	casacore::Vector<casacore::Float> linear_component_float_p; // Float-compatibility

};

//////////////////////////////////////////////////////////////////////////
// GslMultifitLinearBase class_
//////////////////////////////////////////////////////////////////////////

class GslMultifitLinearBase
{

public:

	GslMultifitLinearBase();
	GslMultifitLinearBase(GslLinearModelBase<casacore::Double> &model);

	~GslMultifitLinearBase();

	void resetModel(GslLinearModelBase<casacore::Double> &model);

	void resetNComponents(size_t ncomponents);

	casacore::Vector<casacore::Complex> calcFitCoeff(casacore::Vector<casacore::Complex> &data);
	template<class T> casacore::Vector<T> calcFitCoeff(casacore::Vector<T> &data)
	{
		// Store input data as double
		setData(data);

		// Call fit method to calculate coefficients
		gsl_vector *coeff = calcFitCoeffCore(data_p.column(0));

		// Get coefficients
		casacore::Vector<T> fitCoeff(ncomponents_p);
		for (size_t coeff_idx=0;coeff_idx<ncomponents_p;coeff_idx++)
		{
			fitCoeff(coeff_idx) = gsl_vector_get(coeff,coeff_idx);
		}

		gsl_vector_free (coeff);

		return fitCoeff;
	}


protected:

	void allocGslResources();
	void freeGslResources();

	virtual void setModel(GslLinearModelBase<casacore::Double> &model);

	void setData(casacore::Vector<casacore::Float> &data);
	void setData(casacore::Vector<casacore::Double> &data);
	void setData(casacore::Vector<casacore::Complex> &data);

	virtual gsl_vector* calcFitCoeffCore(casacore::Vector<casacore::Double> data);

	// Model
	size_t ndata_p;
	size_t ncomponents_p;
	size_t max_ncomponents_p;
	gsl_matrix gsl_model_p;
	GslLinearModelBase<casacore::Double> *model_p;

	// GSL Resources
	gsl_vector *gsl_coeff_p;
	gsl_matrix *gsl_covariance_p;
	gsl_multifit_linear_workspace *gsl_workspace_p;

	// Data
	casacore::Matrix<casacore::Double> data_p;

	// Fit result
	int errno_p;
	double chisq_p;
};

//////////////////////////////////////////////////////////////////////////
// GslMultifitWeightedLinear class
//////////////////////////////////////////////////////////////////////////

class GslMultifitWeightedLinear : public GslMultifitLinearBase
{

public:

	GslMultifitWeightedLinear();
	GslMultifitWeightedLinear(GslLinearModelBase<casacore::Double> &model);
	~GslMultifitWeightedLinear();

	void setWeights(casacore::Vector<casacore::Float> &weights);
	void setFlags(casacore::Vector<casacore::Bool> &flags,casacore::Bool goodIsTrue=casacore::True);
	void setWeightsAndFlags(casacore::Vector<casacore::Float> &weights, casacore::Vector<casacore::Bool> &flags, casacore::Bool goodIsTrue=casacore::True);

protected:

	void setModel(GslLinearModelBase<casacore::Double> &model);
	gsl_vector* calcFitCoeffCore(casacore::Vector<casacore::Double> data);

	// Weights
	casacore::Vector<casacore::Double> weights_p;
	gsl_vector gls_weights_p;
};

} //# NAMESPACE DENOISING - END

} //# NAMESPACE CASA - END


#endif /* DenoisingLib_H_ */

