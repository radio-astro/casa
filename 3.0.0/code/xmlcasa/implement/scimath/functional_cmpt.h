
/***
 * Framework independent header file for functional...
 *
 * Implement the functional component here.
 * 
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 ***/

#ifndef _functional_cmpt__H__
#define _functional_cmpt__H__
#ifndef _functional_cpnt__H__
#define _functional_cpnt__H__

#include <vector>
#include <xmlcasa/casac.h>
#include <xmlcasa/variant.h>
#include <xmlcasa/conversions.h>
#include <xmlcasa/record.h>
#include <xmlcasa/scimath/functional_forward.h>

// put includes here

namespace casac {

/**
 * functional component class 
 *
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 **/
class functional
{
  private:
	

  public:

    functional();
    virtual ~functional();

    ::casac::functional * 
	open(const std::string& name = "gaussian1d", 
	     const int order = 0, 
	     const std::vector<double>& params 
	         = ::casac::initialize_vector( 3, (double) 1.0, (double) 0.0, (double) 1.0 ));


    ::casac::variant* f(const std::vector<double>& x = std::vector<double> (1, -1));

    ::casac::variant* cf(const std::vector<casac::complex>& x);

    ::casac::variant* fdf(const std::vector<double>& x = std::vector<double> (1, -1));

    ::casac::variant* cfdf(const std::vector<casac::complex>& x);

    std::string type();

    int npar();

    int ndim();

    int order();

    ::casac::record* state();

    std::vector<double> parameters();

    bool setparameters(const std::vector<double>& par = std::vector<double> (1, -1));

    double par(const int n = 1);

    std::vector<double> setpar(const int n = 1, const double v = 1);

    std::vector<bool> masks();

    bool setmasks(const std::vector<bool>& mask);

    double mask(const int n = 1);

    std::vector<bool> setmask(const int n = 1, const bool v = true);

    bool done();

    ::casac::functional * gaussian1d(const ::casac::record& height = ::casac::initialize_record("            1.0      "), const double center = 0, const double width = 1);

    ::casac::functional * gaussian2d(const std::vector<double>& params = std::vector<double> (1, -1));

    ::casac::functional * poly(const int order = 0, const std::vector<double>& params = std::vector<double> (1, -1));

    ::casac::functional * oddpoly(const int order = 0, const std::vector<double>& params = std::vector<double> (1, -1));

    ::casac::functional * evenpoly(const int order = 0, const std::vector<double>& params = std::vector<double> (1, -1));

    ::casac::functional * chebyshev(const int order = 0, const std::vector<double>& params = std::vector<double> (1, 0), const double xmin = -1, const double xmax = -1, const std::string& ooimode = "'constant'", const double def = 0);

    ::casac::functional * butterworth(const int minorder = 1, const int maxorder = 1, const double mincut = -1.0, const double maxcut = 1.0, const double center = 0.0, const double peak = 1.0);

    ::casac::functional * combi();

    ::casac::functional * compound();

    ::casac::functional * compiled(const std::string& code = "''", const std::vector<double>& params = std::vector<double> (1, -1));

    ::casac::record* id();

    std::string tool_type();

    std::string name();

    bool tool_done(const bool kill = false);

    private:

    #include <xmlcasa/scimath/functional_private.h>
    

};

} // casac namespace
#endif
#endif

