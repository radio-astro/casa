
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

#include <iostream>
#include <xmlcasa/scimath/functional_cmpt.h>

using namespace std;

namespace casac {

functional::functional()
{

}

functional::~functional()
{

}

::casac::functional *
functional::open(const std::string& name, const int order, const std::vector<double>& params)
{

    // TODO : IMPLEMENT ME HERE !
}

::casac::variant*
functional::f(const std::vector<double>& x)
{

    // TODO : IMPLEMENT ME HERE !
}

::casac::variant*
functional::cf(const std::vector<casac::complex>& x)
{

    // TODO : IMPLEMENT ME HERE !
}

::casac::variant*
functional::fdf(const std::vector<double>& x)
{

    // TODO : IMPLEMENT ME HERE !
}

::casac::variant*
functional::cfdf(const std::vector<casac::complex>& x)
{

    // TODO : IMPLEMENT ME HERE !
}

std::string
functional::type()
{

    // TODO : IMPLEMENT ME HERE !
}

int
functional::npar()
{

    // TODO : IMPLEMENT ME HERE !
}

int
functional::ndim()
{

    // TODO : IMPLEMENT ME HERE !
}

int
functional::order()
{

    // TODO : IMPLEMENT ME HERE !
}

::casac::record*
functional::state()
{

    // TODO : IMPLEMENT ME HERE !
}

std::vector<double>
functional::parameters()
{

    // TODO : IMPLEMENT ME HERE !
}

bool
functional::setparameters(const std::vector<double>& par)
{

    // TODO : IMPLEMENT ME HERE !
}

double
functional::par(const int n)
{

    // TODO : IMPLEMENT ME HERE !
}

std::vector<double>
functional::setpar(const int n, const double v)
{

    // TODO : IMPLEMENT ME HERE !
}

std::vector<bool>
functional::masks()
{

    // TODO : IMPLEMENT ME HERE !
}

bool
functional::setmasks(const std::vector<bool>& mask)
{

    // TODO : IMPLEMENT ME HERE !
}

double
functional::mask(const int n)
{

    // TODO : IMPLEMENT ME HERE !
}

std::vector<bool>
functional::setmask(const int n, const bool v)
{

    // TODO : IMPLEMENT ME HERE !
}

bool
functional::done()
{

    // TODO : IMPLEMENT ME HERE !
}

::casac::functional *
functional::gaussian1d(const ::casac::record& height, const double center, const double width)
{

    // TODO : IMPLEMENT ME HERE !
}

::casac::functional *
functional::gaussian2d(const std::vector<double>& params)
{

    // TODO : IMPLEMENT ME HERE !
}

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

bool
functional::tool_done(const bool kill)
{

    // TODO : IMPLEMENT ME HERE !
}

} // casac namespace

