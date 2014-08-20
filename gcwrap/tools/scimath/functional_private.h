/* Private parts of functional component */

private:


std::auto_ptr<casa::LogIO> _log;
std::auto_ptr<casa::Function<casa::Double, casa::Double> > _functional;

functional(
	casa::Polynomial<casa::Double>*& function
);

functional(
	casa::Gaussian1D<casa::Double>*& function
);

functional(
	casa::Gaussian2D<casa::Double>*& function
);

functional(
	casa::PowerLogarithmicPolynomial<casa::Double>*& function
);

/*
functional(
	casa::Function<casa::Double, casa::Double>*& function
);
*/

bool _isAttached() const;
