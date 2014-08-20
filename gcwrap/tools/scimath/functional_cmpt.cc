
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

#include <functional_cmpt.h>

#include <casa/Logging/LogIO.h>
#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/QuantumHolder.h>
#include <scimath/Functionals/Gaussian1D.h>
#include <scimath/Functionals/Gaussian2D.h>
#include <scimath/Functionals/Polynomial.h>
#include <scimath/Functionals/PowerLogarithmicPolynomial.h>

#include <stdcasa/StdCasa/CasacSupport.h>

//DEBUG
#include <casa/Arrays/ArrayIO.h>

using namespace std;
using namespace casa;

#define _ORIGIN *_log << LogOrigin("functional_cmpt.cc", __FUNCTION__, __LINE__);

#define _FUNC(BODY) \
        try { \
        	_ORIGIN; \
        	if (! _isAttached()) { \
        		throw AipsError("Not attached to a functional"); \
        	} \
			BODY \
        } \
        catch (const AipsError& x) { \
        	*_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST; \
        	RETHROW(x); \
        }


#define _FUNC2(BODY) \
        try { \
                _ORIGIN; \
                BODY \
        } \
        catch (const AipsError& x) { \
        	*_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST; \
        	RETHROW(x); \
        }


namespace casac {

functional::functional() : _log(new LogIO())
{

}

functional::functional(
	Gaussian1D<Double>*& function //, Gaussian1D<AutoDiff<Double> >*& first
) :  _log(new LogIO()), _functional(function) //, _firstDeriv(first)
{}

functional::functional(
	Gaussian2D<Double>*& function
) :  _log(new LogIO()), _functional(function)
{}

functional::functional(
	PowerLogarithmicPolynomial<Double>*& function
) :  _log(new LogIO()), _functional(function)
{}

functional::functional(
	Polynomial<Double>*& function
) :  _log(new LogIO()), _functional(function)
{}

/*
functional::functional(
	Function<Double, Double>*& function
) :  _log(new LogIO()), _functional(function)
{}
*/

functional::~functional() {}
/*
::casac::functional *
functional::open(const std::string& name, const int order, const std::vector<double>& params)
{

    // TODO : IMPLEMENT ME HERE !
}

*/
variant* functional::f(const variant& v) {
	_FUNC(
		switch(v.type()) {
		case variant::INT:
			// allow fall through
		case variant::DOUBLE:
			if (_functional->ndim() > 1) {
				throw AipsError(
					"Functional has " + String::toString(_functional->ndim())
					+ " dimensions and so value must be an array of at least this many values"
				);
			}
			return new variant(_functional->operator ()(v.toDouble()));
		case variant::INTVEC:
			// allow fall through
		case variant::DOUBLEVEC: {
			vector<double> f;
			vector<double> x = v.toDoubleVec();
			if (x.size() % _functional->ndim() != 0) {
				throw AipsError(
					"Functional has " + String::toString(_functional->ndim())
				+ " dimensions and so value must be an array with length of an integral multiple of many values"
				);
			}
			switch(_functional->ndim()) {
			case 1: {
				for (vector<double>::const_iterator iter=x.begin(); iter!=x.end(); iter++) {
					f.push_back(_functional->operator ()(*iter));
				}
			}
			break;
			case 2: {
				if (x.size() == 2) {
					return new variant(_functional->operator()(x[0], x[1]));
				}
				for (vector<double>::const_iterator iter=x.begin(); iter!=x.end(); iter++) {
					Double x = *iter;
					iter++;
					f.push_back(_functional->operator ()(x, *iter));
				}
			}
			}
			return new variant(f);
		}
		default:
			throw AipsError("Unpermitted type for value");
		}
	);
}

int functional::ndim() {
	_FUNC(
		return _functional->ndim();
	)
}

bool
functional::done() {
    _log.reset(0);
    _functional.reset(0);
    return true;
}

::casac::functional* functional::gaussian1d(
	const double height, const double center, const double fwhm
) {
	_FUNC2(
		Gaussian1D<Double> *g = new Gaussian1D<Double>(height, center, fwhm);
		/*
		Gaussian1D<AutoDiff<Double> > *dg = new Gaussian1D<AutoDiff<Double> >();
		AutoDiff<Double> da(height, 3, Gaussian1D<Double>::HEIGHT);
		dg->setHeight(da);
		dg->setWidth(AutoDiff<Double>(fwhm, 3, Gaussian1D<Double>::WIDTH));
		dg->setCenter(AutoDiff<Double>(center, 3, Gaussian1D<Double>::CENTER));
		*/
		return new functional(g);
	);
}

functional* functional::gaussian2d(
	const double amplitude, const vector<double>& center,
	const vector<double>& fwhm, const variant& pa
) {
	_FUNC2 (
		vector<double> mycenter = center;
		vector<double> myfwhm = fwhm;
		if (center.size() == 1 and center[0] == -1) {
			mycenter = vector<double>(2, 0);
		}
		if (center.size() != 2) {
			throw AipsError("Center must have exactly two elements");
		}
		if (fwhm.size() == 1 and fwhm[0] == -1) {
			myfwhm = vector<double>(2, 1);
		}
		if (fwhm.size() != 2) {
			throw AipsError ("fwhm must have exactly two elelemnts");
		}
		casa::Quantity mypa = casaQuantity(pa);
		if (mypa.getUnit().empty()) {
			mypa.setUnit("rad");
		}
		else if (! mypa.isConform("rad")) {
			throw AipsError("pa units must be angular");
		}
		Gaussian2D<Double> *g = new Gaussian2D<Double>(amplitude, mycenter, myfwhm, mypa.getValue("rad"));
		return new functional(g);

	);

}

::casac::functional* functional::polynomial(
	const vector<double>& parms
) {
	_FUNC2(
		Polynomial<Double> *poly = new Polynomial<Double>(parms.size());
		poly->setCoefficients(Vector<Double>(parms));
		return new functional(poly);
	);
}

::casac::functional* functional::powerlogpoly(
	const vector<double>& parms
) {
	_FUNC2(
		PowerLogarithmicPolynomial<Double> *plp = new PowerLogarithmicPolynomial<Double>(parms);
		return new functional(plp);
	);
}



/*

::casac::functional *
functional::poly(const int order, const std::vector<double>& params)
{

    // TODO : IMPLEMENT ME HERE !
}

::casac::functional *
functional::oddpoly(const int order, const std::vector<double>& params)
{

    // TODO : IMPLEMENT ME HERE !
}

::casac::functional *
functional::evenpoly(const int order, const std::vector<double>& params)
{

    // TODO : IMPLEMENT ME HERE !
}

::casac::functional *
functional::chebyshev(const int order, const std::vector<double>& params, const double xmin, const double xmax, const std::string& ooimode, const double def)
{

    // TODO : IMPLEMENT ME HERE !
}

::casac::functional *
functional::butterworth(const int minorder, const int maxorder, const double mincut, const double maxcut, const double center, const double peak)
{

    // TODO : IMPLEMENT ME HERE !
}

::casac::functional *
functional::combi()
{

    // TODO : IMPLEMENT ME HERE !
}

::casac::functional *
functional::compound()
{

    // TODO : IMPLEMENT ME HERE !
}

::casac::functional *
functional::compiled(const std::string& code, const std::vector<double>& params)
{

    // TODO : IMPLEMENT ME HERE !
}

::casac::record*
functional::id()
{

    // TODO : IMPLEMENT ME HERE !
}

std::string
functional::tool_type()
{

    // TODO : IMPLEMENT ME HERE !
}

std::string
functional::name()
{

    // TODO : IMPLEMENT ME HERE !
}

bool functional::tool_done(const bool kill)
{

    // TODO : IMPLEMENT ME HERE !
}
*/

bool functional::_isAttached() const {
	return _functional.get() != 0;
}
} // casac namespace

