/* Private parts of functional component */

private:


std::auto_ptr<casacore::LogIO> _log;
std::auto_ptr<casacore::Function<casacore::Double, casacore::Double> > _functional;

functional(
	casacore::Polynomial<casacore::Double>*& function
);

functional(
	casacore::Gaussian1D<casacore::Double>*& function
);

functional(
	casacore::Gaussian2D<casacore::Double>*& function
);

functional(
	casacore::PowerLogarithmicPolynomial<casacore::Double>*& function
);

/*
functional(
	casacore::Function<casacore::Double, casacore::Double>*& function
);
*/

bool _isAttached() const;
