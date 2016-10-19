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

// logger
#include <casacore/casa/Logging/LogIO.h>

// GSL includes
#include <gsl/gsl_multifit.h>

using namespace casacore;

namespace casa { //# NAMESPACE CASA - BEGIN

namespace denoising { //# NAMESPACE DENOISING - BEGIN

void GslVectorWrap(Vector<Double> casa_vector, gsl_vector &wrapper);
void GslMatrixWrap(Matrix<Double> &casa_matrix, gsl_matrix &wrapper, size_t ncols=0);

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
		model_p.resize(ncomponents_p,ndata_p,False);
	}

	size_t ndata() {return ndata_p;}
	size_t ncomponents() {return ncomponents_p;}
	Matrix<T>& getModelMatrix(){return model_p;}
	Vector<T> getModelAt(size_t pos){return model_p.column(pos);}

protected:

	Matrix<T> model_p;
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
			Vector<T> linearComponent;
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

	GslPolynomialModel(const Vector<T> &data_x, size_t order) :
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

	Vector<Float>& getLinearComponentFloat()
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

	Vector<Float> linear_component_float_p; // Float-compatibility

};

//////////////////////////////////////////////////////////////////////////
// GslMultifitLinearBase class
//////////////////////////////////////////////////////////////////////////

class GslMultifitLinearBase
{

public:

	GslMultifitLinearBase();
	GslMultifitLinearBase(GslLinearModelBase<Double> &model);

	~GslMultifitLinearBase();

	void resetModel(GslLinearModelBase<Double> &model);

	void resetNComponents(size_t ncomponents);

	void setDebug(Bool debug) {debug_p = debug;};

	Vector<Complex> calcFitCoeff(Vector<Complex> &data);
	template<class T> Vector<T> calcFitCoeff(Vector<T> &data)
	{
		// Store input data as double
		setData(data);

		// Call fit method to calculate coefficients
		gsl_vector *coeffGSL = calcFitCoeffCore(data_p.column(0));

		// Convert GSL vector into CASA vector
		Vector<T> coeffCASA(ncomponents_p);
		for (size_t coeff_idx=0;coeff_idx<ncomponents_p;coeff_idx++)
		{
			coeffCASA(coeff_idx) = gsl_vector_get(coeffGSL,coeff_idx);
		}

		// Free GSL resources
		gsl_vector_free (coeffGSL);

		return coeffCASA;
	}

	void calcFitModelStd(Vector<Complex> &data, Vector<Complex> &model, Vector<Complex> &std);
	template<class T> void calcFitModelStd(Vector<T> &data, Vector<T> &model, Vector<T> &std)
	{
		// Store input data as double
		setData(data);

		// Call fit method to calculate real/imag coefficients
		gsl_vector *coeff_real = calcFitCoeffCore(data_p.column(0));

		// Get imag coefficients
		gsl_vector xGSL;
		double y, yerr;
		for (size_t data_idx=0;data_idx<ndata_p;data_idx++)
		{
			Vector<Double> xCASA = model_p->getModelAt(data_idx);
			if (xCASA.size() != ncomponents_p) xCASA.resize(ncomponents_p,True);
			GslVectorWrap(xCASA,xGSL);

			y = 0;
			yerr = 0;
			errno_p = gsl_multifit_linear_est (&xGSL, coeff_real, gsl_covariance_p, &y, &yerr);

			if (model.size() > 0) model(data_idx) = y;
			if (std.size() > 0 ) std(data_idx) = yerr;
		}

		// Free GSL resources
		gsl_vector_free (coeff_real);

		return;
	}

	template<class T> Vector<T>  calcFitModel(Vector<T> &data)
	{
		Vector<T> model(ndata_p);
		Vector<T> std;

		calcFitModelStd(data, model, std);

		return model;
	}

	template<class T> Vector<T>  calcFitStd(Vector<T> &data)
	{
		Vector<T> model;
		Vector<T> std(ndata_p);

		calcFitModelStd(data, model, std);

		return std;
	}


protected:

	void allocGslResources();
	void freeGslResources();

	virtual void setModel(GslLinearModelBase<Double> &model);

	void setData(Vector<Float> &data);
	void setData(Vector<Double> &data);
	void setData(Vector<Complex> &data);

	virtual gsl_vector* calcFitCoeffCore(Vector<Double> data);

	// Model
	size_t ndata_p;
	size_t ncomponents_p;
	size_t max_ncomponents_p;
	gsl_matrix gsl_model_p;
	GslLinearModelBase<Double> *model_p;

	// GSL Resources
	gsl_vector *gsl_coeff_p;
	gsl_matrix *gsl_covariance_p;
	gsl_multifit_linear_workspace *gsl_workspace_p;

	// Data
	Matrix<Double> data_p;

	// Fit result
	int errno_p;
	double chisq_p;

	// Misc
	Bool debug_p;
};

//////////////////////////////////////////////////////////////////////////
// GslMultifitWeightedLinear class
//////////////////////////////////////////////////////////////////////////

class GslMultifitWeightedLinear : public GslMultifitLinearBase
{

public:

	GslMultifitWeightedLinear();
	GslMultifitWeightedLinear(GslLinearModelBase<Double> &model);
	~GslMultifitWeightedLinear();

	void setWeights(Vector<Float> &weights);
	void setFlags(Vector<Bool> &flags,Bool goodIsTrue=True);
	void setWeightsAndFlags(Vector<Float> &weights, Vector<Bool> &flags, Bool goodIsTrue=True);

protected:

	void setModel(GslLinearModelBase<Double> &model);
	gsl_vector* calcFitCoeffCore(Vector<Double> data);

	// Weights
	Vector<Double> weights_p;
	gsl_vector gls_weights_p;
};

} //# NAMESPACE DENOISING - END

} //# NAMESPACE CASA - END


#endif /* DenoisingLib_H_ */

