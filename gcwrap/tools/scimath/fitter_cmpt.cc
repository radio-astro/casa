
/***
 * Framework independent implementation file for functional...
 *
 * Implement the functional component here.
 * 
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 ***/

#include <fitter_cmpt.h>

#include <casa/Containers/Record.h>
#include <casa/Logging/LogIO.h>
#include <components/SpectralComponents/GaussianSpectralElement.h>
#include <components/SpectralComponents/PowerLogPolynomialSpectralElement.h>
#include <components/SpectralComponents/ProfileFit1D.h>

#include <components/SpectralComponents/SpectralListFactory.h>
#include <stdcasa/StdCasa/CasacSupport.h>

using namespace std;
using namespace casa;

namespace casac {

// necessary because our build tools require a default constructor and destructor to be implemented

fitter::fitter() {}

fitter::~fitter() {}

record* fitter::gauss1d(
	const vector<double>& y, const vector<double>& x,
	const vector<bool>& mask, const variant& pampest,
	const variant& pcenterest, const variant& pfwhmest
) {
	LogIO log;
	try {
		vector<double> myampest = toVectorDouble(pampest, "pampest");
		vector<double> mycenterest = toVectorDouble(pcenterest, "pcenterest");
		vector<double> myfwhmest = toVectorDouble(pfwhmest, "pfwhmest");
		ProfileFit1D<Double> fitter;
		fitter.setData(x, y, mask);
		SpectralList list = SpectralListFactory::create(log, pampest, pcenterest, pfwhmest);
		if (list.nelements() == 0) {
			list.add(GaussianSpectralElement());
		}
		uInt ngauss = list.nelements();
		fitter.setElements(list);
		fitter.fit();
		Record ret;
		ret.define("niter", fitter.getNumberIterations());
		vector<Double> model;
		ret.define("model", fitter.getFit());
		ret.define("residual", fitter.getResidual());
		Record gs;
		Vector<Double> amps(ngauss);
		Vector<Double> fwhms(ngauss);
		Vector<Double> centers(ngauss);
		Vector<Double> ampErrs(ngauss);
		Vector<Double> fwhmErrs(ngauss);
		Vector<Double> centerErrs(ngauss);
		SpectralList solutions = fitter.getList();
		for (uInt i=0; i<solutions.nelements(); i++) {
			const GaussianSpectralElement *g = dynamic_cast<const GaussianSpectralElement*>(solutions[i]);
			amps[i] = g->getAmpl();
			fwhms[i] = g->getFWHM();
			centers[i] = g->getCenter();
			ampErrs[i] = g->getAmplErr();
			fwhmErrs[i] = g->getFWHMErr();
			centerErrs[i] = g->getCenterErr();
		}
		gs.define("amps", amps);
		gs.define("fwhms", fwhms);
		gs.define("centers", centers);
		gs.define("ampErrs", ampErrs);
		gs.define("fwhmErrs", fwhmErrs);
		gs.define("centerErrs", centerErrs);
		ret.defineRecord("gs", gs);
		return fromRecord(ret);

	}
	catch (const AipsError& x) {
		RETHROW(x);
	}
	return new record();
}

record* fitter::logtranspoly(
	const vector<double>& y, const vector<double>& x,
	const vector<bool>& mask, const vector<double>& estimates
) {
	LogIO log;
	try {
		ProfileFit1D<Double> fitter;
		Vector<Double> lnx = casa::log(Vector<Double>(x));
		Vector<Double> lny = casa::log(Vector<Double>(y));
		fitter.setData(lnx, lny, mask);
		SpectralList list;
		if (estimates.size() == 0) {
			list.add(LogTransformedPolynomialSpectralElement(vector<Double>(2, 0.0)));
		}
		else {
			list.add(LogTransformedPolynomialSpectralElement(estimates));
		}
		fitter.setElements(list);
		fitter.fit();
		return fromRecord(_recordForUnbounded(fitter));
	}
	catch (const AipsError& x) {
		RETHROW(x);
	}
	return new record();
}

record* fitter::poly(
	const vector<double>& y, const vector<double>& x,
	const vector<bool>& mask, const vector<double>& estimates
) {
	LogIO log;
	try {
		ProfileFit1D<Double> fitter;
		fitter.setData(x, y, mask);
		SpectralList list;
		if (estimates.size() == 0) {
			list.add(PolynomialSpectralElement(vector<Double>(2, 0.0)));
		}
		else {
			list.add(PolynomialSpectralElement(estimates));
		}
		cout << list[0]->get() << endl;
		fitter.setElements(list);
		fitter.fit();
		return fromRecord(_recordForUnbounded(fitter));
	}
	catch (const AipsError& x) {
		RETHROW(x);
	}
	return new record();
}

record* fitter::powerlogpoly(
	const vector<double>& y, const vector<double>& x,
	const vector<bool>& mask, const vector<double>& estimates
) {
	LogIO log;
	try {
		ProfileFit1D<Double> fitter;
		fitter.setData(x, y, mask);
		SpectralList list;
		if (estimates.size() == 0) {
			list.add(PowerLogPolynomialSpectralElement(vector<Double>(2, 0.0)));
		}
		else {
			list.add(PowerLogPolynomialSpectralElement(estimates));
		}
		fitter.setElements(list);
		fitter.fit();
		return fromRecord(_recordForUnbounded(fitter));
	}
	catch (const AipsError& x) {
		RETHROW(x);
	}
	return new record();
}

Record fitter::_recordForUnbounded(
	const ProfileFit1D<Double>& fitter
) const {
	Record ret;
	ret.define("niter", fitter.getNumberIterations());
	vector<Double> model;
	ret.define("model", fitter.getFit());
	ret.define("residual", fitter.getResidual());
	SpectralList solutions = fitter.getList();
	const SpectralElement *el = solutions[0];
	ret.define("solutions", el->get());
	ret.define("errors", el->getError());
	return ret;
}



} // casac namespace

