
/***
 * Framework independent header file for imager...
 *
 * Implement the imager component here.
 * 
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 ***/

#ifndef _imager_cmpt__H__
#define _imager_cmpt__H__

#include <vector>
#include <xmlcasa/casac.h>
#include <xmlcasa/Quantity.h>
#include <xmlcasa/conversions.h>
#include <xmlcasa/record.h>
#include <xmlcasa/variant.h>
#include <xmlcasa/synthesis/imager_forward.h>

// put includes here

namespace casac {

/**
 * imager component class 
 *
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 **/
class imager
{
  private:

	

  public:

    imager();
    virtual ~imager();

    bool advise(int& pixels, ::casac::record& cell, int& facets, std::string& phasecenter, const bool takeadvice = true, const double amplitudeloss = 0.05, const ::casac::variant& fieldofview = ::casac::initialize_variant("1.0deg"));

    bool approximatepsf(const std::string& psf = "", const bool async = false);

    bool boxmask(const std::string& mask = "", const std::vector<int>& blc = ::casac::initialize_vector( 4, (int) 0, (int) 0, (int) 0, (int) 0 ), const std::vector<int>& trc = std::vector<int> (0), const double value = 1.0);

  bool calcuvw(const std::vector<int>& fields = std::vector<int> (1, -1), const std::string& refcode = "", const bool reuse=true);

    bool clean(const std::string& algorithm = "clark", const int niter = 1000, const double gain = 0.1, const ::casac::variant& threshold = ::casac::initialize_variant("0.0Jy"), const bool displayprogress = false, const std::vector<std::string>& model = std::vector<std::string> (1, ""), const std::vector<bool>& keepfixed = std::vector<bool> (1, false), const std::string& complist = "", const std::vector<std::string>& mask = std::vector<std::string> (1, ""), const std::vector<std::string>& image = std::vector<std::string> (1, ""), const std::vector<std::string>& residual = std::vector<std::string> (1, ""), const std::vector<std::string>& psfimage = std::vector<std::string> (1, ""), const bool interactive = false, const int npercycle = 100, const std::string& masktemplate = "", const bool async = false);

    bool clipimage(const std::string& image = "", const ::casac::variant& threshold = ::casac::initialize_variant("0.0Jy"));

    bool clipvis(const ::casac::variant& threshold = ::casac::initialize_variant("0.0Jy"));

    bool close();

    bool correct(const bool doparallactic = true, const Quantity& timestep = Quantity(std::vector<double> (1, 10),"s"));

    bool defineimage(const int nx = 128, const int ny = -1, const ::casac::variant& cellx = ::casac::initialize_variant("1.0"), const ::casac::variant& celly = ::casac::initialize_variant(""), const std::string& stokes = "I", const ::casac::variant& phasecenter = ::casac::initialize_variant("0"), const std::string& mode = "mfs", const int nchan = -1, const ::casac::variant& start = ::casac::initialize_variant("0"), const ::casac::variant& step = ::casac::initialize_variant("1"), const std::vector<int>& spw = std::vector<int> (1, 0), const ::casac::variant& restfreq = ::casac::initialize_variant(""), const std::string& outframe = "", const std::string& veltype = "radio", const int facets = 1, const ::casac::variant& movingsource = ::casac::initialize_variant(""), const ::casac::variant& distance = ::casac::initialize_variant("0.0"));

    bool done();

    bool drawmask(const std::string& image = "", const std::string& mask = "");

    bool exprmask(const std::string& mask = "", const double expr = 1.0);

    bool feather(const std::string& image = "", const std::string& highres = "", const std::string& lowres = "", const std::string& lowpsf = "", const bool async = false);

    bool filter(const std::string& type = "gaussian", const ::casac::variant& bmaj = ::casac::initialize_variant("1arcsec"), const ::casac::variant& bmin = ::casac::initialize_variant("1arcsec"), const ::casac::variant& bpa = ::casac::initialize_variant("0deg"), const bool async = false);

    bool fitpsf(::casac::record& bmaj, ::casac::record& bmin, ::casac::record& bpa, const std::string& psf, const bool async = false);

    bool fixvis(const std::vector<int>& fields = std::vector<int> (1, -1), const std::vector<std::string>& phasedirs = std::vector<std::string> (1, ""), const std::string& refcode = "", const std::vector<double>& distances = std::vector<double> (1, 0.0), const std::string& datacolumn = "all");

    bool ft(const std::vector<std::string>& model = std::vector<std::string> (1, ""), const std::string& complist = "", const bool incremental = false, const bool async = false);

    bool linearmosaic(const std::vector<std::string>& images = std::vector<std::string> (1, ""), const std::string& mosaic = "", const std::string& fluxscale = "", const std::string& sensitivity = "", const std::vector<int>& fieldids = std::vector<int> (1, 0), const bool usedefaultvp = true, const std::string& vptable = "", const bool async = false);

    bool make(const std::string& image = "", const bool async = false);

    bool makeimage(const std::string& type = "observed", const std::string& image = "", const std::string& compleximage = "", const bool verbose = true, const bool async = false);

    bool makemodelfromsd(const std::string& sdimage = "", const std::string& modelimage = "", const std::string& sdpsf = "", const std::string& maskimage = "");

    bool mask(const std::string& image = "", const std::string& mask = "", const ::casac::variant& threshold = ::casac::initialize_variant("0.0Jy"), const bool async = false);

    bool mem(const std::string& algorithm = "entropy", const int niter = 20, const ::casac::variant& sigma = ::casac::initialize_variant("0.001Jy"), const ::casac::variant& targetflux = ::casac::initialize_variant("1.0Jy"), const bool constrainflux = false, const bool displayprogress = false, const std::vector<std::string>& model = std::vector<std::string> (1, ""), const std::vector<bool>& keepfixed = std::vector<bool> (1, false), const std::string& complist = "", const std::vector<std::string>& prior = std::vector<std::string> (1, ""), const std::vector<std::string>& mask = std::vector<std::string> (1, ""), const std::vector<std::string>& image = std::vector<std::string> (1, ""), const std::vector<std::string>& residual = std::vector<std::string> (1, ""), const bool async = false);

    bool nnls(const std::vector<std::string>& model = std::vector<std::string> (1, ""), const std::vector<bool>& keepfixed = std::vector<bool> (1, false), const std::string& complist = "", const int niter = 0, const double tolerance = 1e-06, const std::vector<std::string>& fluxmask = std::vector<std::string> (1, ""), const std::vector<std::string>& datamask = std::vector<std::string> (1, ""), const std::vector<std::string>& image = std::vector<std::string> (1, ""), const std::vector<std::string>& residual = std::vector<std::string> (1, ""), const bool async = false);

    bool open(const std::string& thems = "", const bool compress = false, const bool usescratch = false);

    bool pb(const std::string& inimage = "", const std::string& outimage = "", const std::string& incomps = "", const std::string& outcomps = "", const std::string& operation = "apply", const std::string& pointingcenter = "N. pole", const ::casac::variant& parangle = ::casac::initialize_variant("0.0deg"), const std::string& pborvp = "pb", const bool async = false);

    bool pixon(const std::string& algorithm = "test", const Quantity& sigma = Quantity(std::vector<double> (1, 0.001),"Jy"), const std::string& model = "", const bool async = false);

    bool plotsummary();

    bool plotuv(const bool rotate = false);

    bool plotvis(const std::string& type = "all", const int increment = 1);

  // Dumps core 2/3 of the time.  Fix, add to plotms or replace with something else.
  // bool plotweights(const bool gridded = false, const int increment = 1);

    bool regionmask(const std::string& mask = "", const ::casac::record& region = ::casac::initialize_record(""), const ::casac::variant& boxes = ::casac::initialize_variant(""), const ::casac::variant& circles = ::casac::initialize_variant(""), const double value = 1.0);

    bool regiontoimagemask(const std::string& mask = "", const ::casac::record& region = ::casac::initialize_record(""), const ::casac::variant& boxes = ::casac::initialize_variant(""), const ::casac::variant& circles = ::casac::initialize_variant(""), const double value = 1.0);

    bool residual(const std::vector<std::string>& model = std::vector<std::string> (1, ""), const std::string& complist = "", const std::vector<std::string>& image = std::vector<std::string> (1, ""), const bool async = false);

    bool restore(const std::vector<std::string>& model = std::vector<std::string> (1, ""), const std::string& complist = "", const std::vector<std::string>& image = std::vector<std::string> (1, ""), const std::vector<std::string>& residual = std::vector<std::string> (1, ""), const bool async = false);

    bool sensitivity(::casac::record& pointsource, double& relative, double& sumweights, const bool async = false);

    bool setbeam(const ::casac::variant& bmaj = ::casac::initialize_variant("1.0arcsec"), const ::casac::variant& bmin = ::casac::initialize_variant("1.0arcsec"), const ::casac::variant& bpa = ::casac::initialize_variant("0deg"), const bool async = false);

    bool setdata(const std::string& mode = "none", const std::vector<int>& nchan = std::vector<int> (1, 1), const std::vector<int>& start = std::vector<int> (1, 0), const std::vector<int>& step = std::vector<int> (1, 1), const Quantity& mstart = Quantity(std::vector<double> (1, 0.0),"km/s"), const Quantity& mstep = Quantity(std::vector<double> (1, 0.0),"km/s"), const std::vector<int>& spwid = std::vector<int> (1, 0), const std::vector<int>& fieldid = std::vector<int> (1, 0), const std::string& msselect = "", const bool async = false);

    bool selectvis(const std::string& vis = "", const std::vector<int>& nchan = std::vector<int> (1, -1), const std::vector<int>& start = std::vector<int> (1, 0), const std::vector<int>& step = std::vector<int> (1, 1), const ::casac::variant& spw = ::casac::initialize_variant("-1"), const ::casac::variant& field = ::casac::initialize_variant("-1"), const ::casac::variant& baseline = ::casac::initialize_variant("-1"), const ::casac::variant& time = ::casac::initialize_variant(""), const ::casac::variant& scan = ::casac::initialize_variant(""), const ::casac::variant& uvrange = ::casac::initialize_variant(""), const std::string& taql = "", const bool usescratch = false, const bool datainmemory = false);

    bool setimage(const int nx = 128, const int ny = 128, const Quantity& cellx = Quantity(std::vector<double> (1, 1.0),"arcsec"), const Quantity& celly = Quantity(std::vector<double> (1, 1.0),"arcsec"), const std::string& stokes = "I", const bool doshift = false, const std::string& phasecenter = "00h00m00.00 +090d00m00.000 B1950", const Quantity& shiftx = Quantity(std::vector<double> (1, 0.0),"arcsec"), const Quantity& shifty = Quantity(std::vector<double> (1, 0.0),"arcsec"), const std::string& mode = "mfs", const int nchan = 1, const int start = 0, const int step = 1, const std::string& mstart = "0 km/s", const std::string& mstep = "1 km/s", const std::vector<int>& spwid = std::vector<int> (1, 0), const int fieldid = 0, const int facets = 1, const Quantity& distance = Quantity(std::vector<double> (1, 0.0),"m"));

    bool setjy(const ::casac::variant& field = ::casac::initialize_variant(""), const ::casac::variant& spw = ::casac::initialize_variant(""), const std::string& modimage = "", const std::vector<double>& fluxdensity = ::casac::initialize_vector( 4, (double) 0.0, (double) 0.0, (double) 0.0, (double) 0.0 ), const std::string& standard = "SOURCE");

    bool setmfcontrol(const double cyclefactor = 1.5, const double cyclespeedup = -1, const int stoplargenegatives = 2, const int stoppointmode = -1, const double minpb = 0.1, const std::string& scaletype = "NONE", const double constpb = 0.4, const std::vector<std::string>& fluxscale = std::vector<std::string> (1, ""));

    bool setoptions(const std::string& ftmachine = "ft", const int cache = -1, const int tile = 16, const std::string& gridfunction = "SF", const ::casac::variant& location = ::casac::initialize_variant(""), const double padding = 1.0, const std::string& freqinterp = "nearest", const int wprojplanes = 1, const std::string& epjtablename = "", const bool applypointingoffsets = false, const bool dopbgriddingcorrections = true, const std::string& cfcachedirname = "", const double pastep = 1.0, const double pblimit = 0.05, const int imagetilevol = 0);

    bool setscales(const std::string& scalemethod = "nscales", const int nscales = 5, const std::vector<double>& uservector = ::casac::initialize_vector( 3, (double) 0.0, (double) 3.0, (double) 10.0 ));

    bool setsmallscalebias(const float inbias = 0.6);

    bool settaylorterms(const int ntaylorterms = 2, const double reffreq = 1.4e+09);

    bool setsdoptions(const double scale = 1.0, const double weight = 1.0, const int convsupport = -1, const std::string& pointingcolumntouse = "DIRECTION");

    bool setvp(const bool dovp = false, const bool usedefaultvp = true, const std::string& vptable = "", const bool dosquint = false, const ::casac::variant& parangleinc = ::casac::initialize_variant("360deg"), const ::casac::variant& skyposthreshold = ::casac::initialize_variant("180deg"), const std::string& telescope = "", const bool verbose = true);

    bool smooth(const std::vector<std::string>& model = std::vector<std::string> (1, ""), const std::vector<std::string>& image = std::vector<std::string> (1, ""), const bool usefit = true, const ::casac::variant& bmaj = ::casac::initialize_variant("5.arcsec"), const ::casac::variant& bmin = ::casac::initialize_variant("5.arcsec"), const ::casac::variant& bpa = ::casac::initialize_variant("0deg"), const bool normalize = true, const bool async = false);

    bool stop();

    bool summary();

    bool uvrange(const double uvmin = 0.0, const double uvmax = 0.0);

    bool weight(const std::string& type = "natural", const std::string& rmode = "none", const ::casac::variant& noise = ::casac::initialize_variant("0.0Jy"), const double robust = 0.0, const ::casac::variant& fieldofview = ::casac::initialize_variant("0.0arcsec"), const int npixels = 0, const bool mosaic = false, const bool async = false);

    private:

	#include <xmlcasa/synthesis/imager_private.h>

};

} // casac namespace
#endif

