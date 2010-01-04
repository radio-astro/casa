
/***
 * Framework independent header file for imagepol...
 *
 * Implement the imagepol component here.
 * 
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 ***/

#ifndef _imagepol_cmpt__H__
#define _imagepol_cmpt__H__

#include <vector>
#include <xmlcasa/casac.h>

#include <xmlcasa/conversions.h>
#include <xmlcasa/variant.h>
#include <xmlcasa/images/imagepol_forward.h>
#include <xmlcasa/images/image_cmpt.h>

// put includes here

namespace casac {

/**
 * imagepol component class 
 *
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 **/
class imagepol
{
  private:

	

  public:

    imagepol();

    virtual ~imagepol();

    bool imagepoltestimage(const std::string& outfile = "imagepol.iquv", const std::vector<double>& rm = std::vector<double> (1, 0.0), const double pa0 = 0.0, const double sigma = 0.01, const int nx = 32, const int ny = 32, const int nf = 32, const double f0 = 1.4e9, const double bw = 128.0e6);

    bool complexlinpol(const std::string& outfile);

    bool complexfraclinpol(const std::string& outfile);

    ::casac::image * depolratio(const std::string& infile, const bool debias = false, const double clip = 10.0, const double sigma = -1, const std::string& outfile = "");

    bool close();

    bool done();

    bool fourierrotationmeasure(const std::string& complex = "", const std::string& amp = "", const std::string& pa = "", const std::string& real = "", const std::string& imag = "", const bool zerolag0 = false);

    ::casac::image * fraclinpol(const bool debias = false, const double clip = 10.0, const double sigma = -1, const std::string& outfile = "");

    ::casac::image * fractotpol(const bool debias = false, const double clip = 10.0, const double sigma = -1, const std::string& outfile = "");

    ::casac::image * linpolint(const bool debias = false, const double clip = 10.0, const double sigma = -1, const std::string& outfile = "");

    ::casac::image * linpolposang(const std::string& outfile = "");

    bool makecomplex(const std::string& complex, const std::string& real = "", const std::string& imag = "", const std::string& amp = "", const std::string& phase = "");

    bool open(const ::casac::variant& image = ::casac::initialize_variant(""));

    ::casac::image * pol(const std::string& which, const bool debias = false, const double clip = 10.0, const double sigma = -1, const std::string& outfile = "");

    bool rotationmeasure(const std::string& rm = "", const std::string& rmerr = "", const std::string& pa0 = "", const std::string& pa0err = "", const std::string& nturns = "", const std::string& chisq = "", const double sigma = -1, const double rmfg = 0.0, const double rmmax = 0.0, const double maxpaerr = 1e30, const std::string& plotter = "", const int nx = 5, const int ny = 5);

    double sigma(const double clip = 10.0);

    ::casac::image * sigmadepolratio(const std::string& infile, const bool debias = false, const double clip = 10.0, const double sigma = -1, const std::string& outfile = "");

    ::casac::image * sigmafraclinpol(const double clip = 10.0, const double sigma = -1, const std::string& outfile = "");

    ::casac::image * sigmafractotpol(const double clip = 10.0, const double sigma = -1, const std::string& outfile = "");

    double sigmalinpolint(const double clip = 10.0, const double sigma = -1, const std::string& outfile = "");

    ::casac::image * sigmalinpolposang(const double clip = 10.0, const double sigma = -1, const std::string& outfile = "");

    double sigmastokes(const std::string& which, const double clip = 10.0);

    double sigmastokesi(const double clip = 10.0);

    double sigmastokesq(const double clip = 10.0);

    double sigmastokesu(const double clip = 10.0);

    double sigmastokesv(const double clip = 10.0);

    double sigmatotpolint(const double clip = 10.0, const double sigma = -1);

    ::casac::image * stokes(const std::string& which, const std::string& outfile = "");

    ::casac::image * stokesi(const std::string& outfile = "");

    ::casac::image * stokesq(const std::string& outfile = "");

    ::casac::image * stokesu(const std::string& outfile = "");

    ::casac::image * stokesv(const std::string& outfile = "");

    bool summary();

    ::casac::image * totpolint(const bool debias = false, const double clip = 10.0, const double sigma = -1, const std::string& outfile = "");

    private:

		#include <xmlcasa/images/imagepol_private.h>	

};

} // casac namespace
#endif

