
/***
 * Framework independent header file for deconvolver...
 *
 * Implement the deconvolver component here.
 * 
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 ***/

#ifndef _deconvolver_cmpt__H__
#define _deconvolver_cmpt__H__

#include <vector>
#include <xmlcasa/casac.h>
#include <xmlcasa/conversions.h>
#include <xmlcasa/record.h>
#include <xmlcasa/variant.h>
#include <xmlcasa/synthesis/deconvolver_forward.h>

// put includes here

namespace casac {

/**
 * deconvolver component class 
 *
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 **/
class deconvolver
{
  private:

	

  public:

    deconvolver();
    virtual ~deconvolver();

    bool open(const std::string& dirty = "", const std::string& psf = "", bool suppressWarnings=false);

    bool reopen();

    bool close();

    bool done();

    bool summary();

    bool boxmask(const std::string& mask = "", const std::vector<int>& blc = std::vector<int> (1, -1), const std::vector<int>& trc = std::vector<int> (1, -1), const ::casac::variant& fillvalue = ::casac::initialize_variant("1.0Jy"), const ::casac::variant& outsidevalue = ::casac::initialize_variant("0.0Jy"));

    bool regionmask(const std::string& mask = "", const ::casac::record& region = ::casac::initialize_record(""), const ::casac::variant& boxes = ::casac::initialize_variant(""), const double value = 1.0);

    bool clipimage(const std::string& clippedimage = "", const std::string& inputimage = "", const ::casac::variant& threshold = ::casac::initialize_variant("0.0Jy"));

    bool clarkclean(const int niter = 1000, const double gain = 0.1, const ::casac::variant& threshold = ::casac::initialize_variant("0Jy"), const bool displayprogress = false, const std::string& model = "", const std::string& mask = "", const int histbins = 500, const std::vector<int>& psfpatchsize = ::casac::initialize_vector( 2, (int) 51, (int) 51 ), const double maxextpsf = 0.2, const double speedup = 0.0, const int maxnumpix = 10000, const int maxnummajcycles = -1, const int maxnumminoriter = -1);

    std::string dirtyname();

    std::string psfname();

    bool make(const std::string& image = "", const bool async = false);

    bool convolve(const std::string& convolvemodel = "", const std::string& model = "");

    bool makegaussian(const std::string& gaussianimage = "", const ::casac::variant& bmaj = ::casac::initialize_variant("0rad"), const ::casac::variant& bmin = ::casac::initialize_variant("0rad"), const ::casac::variant& bpa = ::casac::initialize_variant("0deg"), const bool normalize = true, const bool async = false);

    bool state();

    bool updatestate(const std::string& f = "", const std::string& method = "INIT");

    bool clean(const std::string& algorithm = "hogbom", const int niter = 1000, const double gain = 0.1, const ::casac::variant& threshold = ::casac::initialize_variant("0Jy"), const bool displayprogress = false, const std::string& model = "", const std::string& mask = "", const bool async = false);

    bool setscales(const std::string& scalemethod = "nscales", const int nscales = 5, const std::vector<double>& uservector = ::casac::initialize_vector( 3, (double) 0.0, (double) 3.0, (double) 10.0 ));

    bool ft(const std::string& model = "", const std::string& transform = "", const bool async = false);

    bool restore(const std::string& model = "", const std::string& image = "", const ::casac::variant& bmaj = ::casac::initialize_variant("0rad"), const ::casac::variant& bmin = ::casac::initialize_variant("0rad"), const ::casac::variant& bpa = ::casac::initialize_variant("0deg"), const bool async = false);

    bool residual(const std::string& model = "", const std::string& image = "", const bool async = false);

    bool smooth(const std::string& model = "", const std::string& image = "", const ::casac::variant& bmaj = ::casac::initialize_variant("0rad"), const ::casac::variant& bmin = ::casac::initialize_variant("0rad"), const ::casac::variant& bpa = ::casac::initialize_variant("0deg"), const bool normalize = true, const bool async = false);

    bool mem(const std::string& entropy = "entropy", const int niter = 20, const ::casac::variant& sigma = ::casac::initialize_variant("0.001Jy"), const ::casac::variant& targetflux = ::casac::initialize_variant("1.0Jy"), const bool constrainflux = false, const bool displayprogress = false, const std::string& model = "", const std::string& prior = "", const std::string& mask = "", const bool imageplane = false, const bool async = false);

    bool makeprior(const std::string& prior = "", const std::string& templateimage = "", const ::casac::variant& lowclipfrom = ::casac::initialize_variant("0.0Jy"), const ::casac::variant& lowclipto = ::casac::initialize_variant("0.0Jy"), const ::casac::variant& highclipfrom = ::casac::initialize_variant("9e20Jy"), const ::casac::variant& highclipto = ::casac::initialize_variant("9e20Jy"), const std::vector<int>& blc = std::vector<int> (1, -1), const std::vector<int>& trc = std::vector<int> (1, -1), const bool async = false);

    private:

#include <xmlcasa/synthesis/deconvolver_private.h>

};

} // casac namespace
#endif

