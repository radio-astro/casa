
/***
 * Framework independent header file for image...
 *
 * Implement the image component here.
 * 
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 ***/

#ifndef _image_cmpt__H__
#define _image_cmpt__H__

#include <vector>
#include <xmlcasa/casac.h>
#include <xmlcasa/conversions.h>
#include <xmlcasa/record.h>
#include <xmlcasa/variant.h>
#include <xmlcasa/images/image_forward.h>
#include <xmlcasa/images/image_cmpt.h>
#include <xmlcasa/images/coordsys_cmpt.h>

// put includes here

namespace casac {

/**
 * image component class 
 *
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 **/
class image
{
  private:

	

  public:

    image();
    virtual ~image();

    ::casac::image * newimage(const std::string& infile);

    ::casac::image * newimagefromfile(const std::string& infile);

    ::casac::image * imagecalc(const std::string& outfile = "", const std::string& pixels = "", const bool overwrite = false);

    ::casac::image * imageconcat(const std::string& outfile = "", const ::casac::variant& infiles = ::casac::initialize_variant(""), const int axis = -10, const bool relax = false, const bool tempclose = true, const bool overwrite = false);

    bool fromarray(const std::string& outfile, const ::casac::variant& pixels, const ::casac::record& csys = ::casac::initialize_record(""), const bool linear = false, const bool overwrite = false, const bool log = true);

    bool fromascii(const std::string& outfile = "", const std::string& infile = "", const std::vector<int>& shape = std::vector<int> (1, -1), const std::string& sep = " ", const ::casac::record& csys = ::casac::initialize_record(""), const bool linear = false, const bool overwrite = false);

    bool fromfits(const std::string& outfile = "", const std::string& infile = "", const int whichrep = 0, const int whichhdu = 0, const bool zeroblanks = false, const bool overwrite = false);

    bool fromimage(const std::string& outfile = "", const std::string& infile = "", const ::casac::record& region = ::casac::initialize_record(""), const ::casac::variant& mask = ::casac::initialize_variant(""), const bool dropdeg = false, const bool overwrite = false);

    bool fromshape(const std::string& outfile = "", const std::vector<int>& shape = std::vector<int> (1, 0), const ::casac::record& csys = ::casac::initialize_record(""), const bool linear = false, const bool overwrite = false, const bool log = true);

    bool maketestimage(const std::string& outfile = "", const bool overwrite = false);

    ::casac::image * adddegaxes(const std::string& outfile = "", const bool direction = false, const bool spectral = false, const std::string& stokes = "", const bool linear = false, const bool tabular = false, const bool overwrite = false);

    bool addnoise(const std::string& type = "normal", const std::vector<double>& pars = ::casac::initialize_vector( 2, (double) 0.0, (double) 1.0 ), const ::casac::record& region = ::casac::initialize_record(""), const bool zero = false);

    ::casac::image * convolve(const std::string& outfile = "", const ::casac::variant& kernel = ::casac::initialize_variant(""), const double scale = -1.0, const ::casac::record& region = ::casac::initialize_record(""), const ::casac::variant& mask = ::casac::initialize_variant(""), const bool overwrite = false, const bool async = false);

    ::casac::record* boundingbox(const ::casac::record& region = ::casac::initialize_record(""));

    std::string brightnessunit();

    bool calc(const std::string& pixels);

    bool calcmask(const std::string& mask = "", const std::string& name = "", const bool asdefault = true);

    bool close();

    ::casac::image * continuumsub(const std::string& outline = "", const std::string& outcont = "continuumsub.im", const ::casac::record& region = ::casac::initialize_record(""), const std::vector<int>& channels = std::vector<int> (1, -1), const std::string& pol = "", const int fitorder = 0, const bool overwrite = false);

    ::casac::record* convertflux(const ::casac::variant& value = ::casac::initialize_variant("0Jy/beam"), const ::casac::variant& major = ::casac::initialize_variant("1arcsec"), const ::casac::variant& minor = ::casac::initialize_variant("1arcsec"), const std::string& type = "Gaussian", const bool topeak = true);

    ::casac::image * convolve2d(const std::string& outfile = "", const std::vector<int>& axes = ::casac::initialize_vector( 2, (int) 0, (int) 1 ), const std::string& type = "gaussian", const ::casac::variant& major = ::casac::initialize_variant("0deg"), const ::casac::variant& minor = ::casac::initialize_variant("0deg"), const ::casac::variant& pa = ::casac::initialize_variant("0deg"), const double scale = -1, const ::casac::record& region = ::casac::initialize_record(""), const ::casac::variant& mask = ::casac::initialize_variant(""), const bool overwrite = false, const bool async = false);

    ::casac::coordsys * coordsys(const std::vector<int>& axes = std::vector<int> (1, -1));

    ::casac::record* coordmeasures(const std::vector<double>& pixel = std::vector<double> (1, -1));

    ::casac::record* decompose(const ::casac::record& region = ::casac::initialize_record(""), const ::casac::variant& mask = ::casac::initialize_variant(""), const bool simple = false, const double threshold = -1, const int ncontour = 11, const int minrange = 1, const int naxis = 2, const bool fit = true, const double maxrms = -1, const int maxretry = -1, const int maxiter = 256, const double convcriteria = 0.0001);

    ::casac::record* deconvolvecomponentlist(const ::casac::record& complist);

    ::casac::record* deconvolvefrombeam(const ::casac::variant& source = ::casac::initialize_variant(""), const ::casac::variant& beam = ::casac::initialize_variant(""));

  bool remove(const bool done = false, const bool verbose = true);

    bool removefile(const std::string& file);

    bool done(const bool remove = false, const bool verbose = true);

    bool fft(const std::string& real = "", const std::string& imag = "", const std::string& amp = "", const std::string& phase = "", const std::vector<int>& axes = std::vector<int> (1, -1), const ::casac::record& region = ::casac::initialize_record(""), const ::casac::variant& mask = ::casac::initialize_variant(""));

    ::casac::record* findsources(const int nmax = 20, const double cutoff = 0.1, const ::casac::record& region = ::casac::initialize_record(""), const ::casac::variant& mask = ::casac::initialize_variant(""), const bool point = true, const int width = 5, const bool negfind = false);

    bool fitallprofiles(const ::casac::record& region = ::casac::initialize_record(""), const int axis = -1, const ::casac::variant& mask = ::casac::initialize_variant(""), const int ngauss = 1, const int poly = -1, const std::string& sigma = "", const std::string& fit = "", const std::string& resid = "");

    ::casac::record* fitprofile(std::vector<double>& values, std::vector<double>& resid, const ::casac::record& region = ::casac::initialize_record(""), const int axis = -1, const ::casac::variant& mask = ::casac::initialize_variant(""), const ::casac::record& estimate = ::casac::initialize_record(""), const int ngauss = -1, const int poly = -1, const bool fit = true, const std::string& sigma = "");

    ::casac::image * fitpolynomial(const std::string& residfile = "", const std::string& fitfile = "", const std::string& sigmafile = "", const int axis = -1, const int order = 0, const ::casac::record& region = ::casac::initialize_record(""), const ::casac::variant& mask = ::casac::initialize_variant(""), const bool overwrite = false);

    ::casac::record* fitsky(const ::casac::record& region = ::casac::initialize_record(""), const int chan = 0, const std::string& stokes = "I", const ::casac::variant& mask = ::casac::initialize_variant(""), const std::vector<std::string>& models = std::vector<std::string> (1, "gaussian"), const ::casac::record& estimate = ::casac::initialize_record(""), const std::vector<std::string>& fixedparams = std::vector<std::string> (1, ""), const std::vector<double>& includepix = std::vector<double> (1, -1), const std::vector<double>& excludepix = std::vector<double> (1, -1), const bool fit = true, const bool deconvolve = false, const bool list = true);

    ::casac::record* fitcomponents(const std::string& box = "", const ::casac::variant& region = ::casac::initialize_variant(""), const int chan = 0, const std::string& stokes = "I", const ::casac::variant& vmask = ::casac::initialize_variant(""), const std::vector<double>& in_includepix = std::vector<double> (1, -1), const std::vector<double>& in_excludepix = std::vector<double> (1, -1), const std::string& residual = "", const std::string& model = "", const std::string& estimates = "", const std::string& logfile = "", const bool append = true, const std::string& newestimates = "");

    bool fromrecord(const ::casac::record& record, const std::string& outfile = "");

    ::casac::variant* getchunk(const std::vector<int>& blc = std::vector<int> (1, -1), const std::vector<int>& trc = std::vector<int> (1, -1), const std::vector<int>& inc = std::vector<int> (1, 1), const std::vector<int>& axes = std::vector<int> (1, -1), const bool list = false, const bool dropdeg = false, const bool getmask = false);

    ::casac::variant* getregion(const ::casac::record& region = ::casac::initialize_record(""), const std::vector<int>& axes = std::vector<int> (1, -1), const ::casac::variant& mask = ::casac::initialize_variant(""), const bool list = false, const bool dropdeg = false, const bool getmask = false);

    ::casac::record* getslice(const std::vector<double>& x, const std::vector<double>& y, const std::vector<int>& axes = ::casac::initialize_vector( 2, (int) 0, (int) 1 ), const std::vector<int>& coord = std::vector<int> (1, -1), const int npts = 0, const std::string& method = "linear");

    ::casac::image * hanning(const std::string& outfile = "", const ::casac::record& region = ::casac::initialize_record(""), const ::casac::variant& mask = ::casac::initialize_variant(""), const int axis = -10, const bool drop = true, const bool overwrite = false, const bool async = false);

    std::vector<bool> haslock();

    bool histograms(::casac::record& histout, const std::vector<int>& axes = std::vector<int> (1, -1), const ::casac::record& region = ::casac::initialize_record(""), const ::casac::variant& mask = ::casac::initialize_variant(""), const int nbins = 25, const std::vector<double>& includepix = std::vector<double> (1, -1), const bool gauss = false, const bool cumu = false, const bool log = false, const bool list = true, const std::string& plotter = "", const int nx = 1, const int ny = 1, const std::vector<int>& size = ::casac::initialize_vector( 2, (int) 600, (int) 450 ), const bool force = false, const bool disk = false, const bool async = false);

    std::vector<std::string> history(const bool list = true, const bool browse = false);

    ::casac::image * insert(const std::string& infile = "", const ::casac::record& region = ::casac::initialize_record(""), const std::vector<double>& locate = std::vector<double> (1, -1));

    bool isopen();

    bool ispersistent();

    bool lock(const bool writelock = false, const int nattempts = 0);

    bool makecomplex(const std::string& outfile, const std::string& imag, const ::casac::record& region = ::casac::initialize_record(""), const bool overwrite = false);

    std::vector<std::string> maskhandler(const std::string& op = "default", const std::vector<std::string>& name = std::vector<std::string> (1, ""));

    ::casac::record* miscinfo();

    bool modify(const ::casac::record& model, const ::casac::record& region = ::casac::initialize_record(""), const ::casac::variant& mask = ::casac::initialize_variant(""), const bool subtract = true, const bool list = true, const bool async = false);

    ::casac::record* maxfit(const ::casac::record& region = ::casac::initialize_record(""), const bool point = true, const int width = 5, const bool negfind = false, const bool list = true);

    ::casac::image * moments(const std::vector<int>& moments = std::vector<int> (1, 0), const int axis = -10, const ::casac::record& region = ::casac::initialize_record(""), const ::casac::variant& mask = ::casac::initialize_variant(""), const std::vector<std::string>& method = std::vector<std::string> (1, ""), const std::vector<int>& smoothaxes = std::vector<int> (1, -1), const ::casac::variant& smoothtypes = ::casac::initialize_variant(""), const std::vector<double>& smoothwidths = std::vector<double> (1, 0.0), const std::vector<double>& includepix = std::vector<double> (1, -1), const std::vector<double>& excludepix = std::vector<double> (1, -1), const double peaksnr = 3.0, const double stddev = 0.0, const std::string& doppler = "RADIO", const std::string& outfile = "", const std::string& smoothout = "", const std::string& plotter = "", const int nx = 1, const int ny = 1, const bool yind = false, const bool overwrite = false, const bool drop = true, const bool async = false);

    std::string name(const bool strippath = false);

    bool open(const std::string& infile);

    ::casac::record* pixelvalue(const std::vector<int>& pixel = std::vector<int> (1, -1));

    bool putchunk(const ::casac::variant& pixels, const std::vector<int>& blc = std::vector<int> (1, -1), const std::vector<int>& inc = std::vector<int> (1, 1), const bool list = false, const bool locking = true, const bool replicate = false);

    bool putregion(const ::casac::variant& pixels = ::casac::initialize_variant(""), const ::casac::variant& pixelmask = ::casac::initialize_variant(""), const ::casac::record& region = ::casac::initialize_record(""), const bool list = false, const bool usemask = true, const bool locking = true, const bool replicate = false);

    ::casac::image * rebin(const std::string& outfile, const std::vector<int>& bin, const ::casac::record& region = ::casac::initialize_record(""), const ::casac::variant& mask = ::casac::initialize_variant(""), const bool dropdeg = false, const bool overwrite = false, const bool async = false);

    ::casac::image * regrid(const std::string& outfile = "", const std::vector<int>& shape = std::vector<int> (1, -1), const ::casac::record& csys = ::casac::initialize_record(""), const std::vector<int>& axes = std::vector<int> (1, -1), const ::casac::record& region = ::casac::initialize_record(""), const ::casac::variant& mask = ::casac::initialize_variant(""), const std::string& method = "linear", const int decimate = 10, const bool replicate = false, const bool doref = true, const bool dropdeg = false, const bool overwrite = false, const bool force = false, const bool async = false);

    ::casac::image * reorder(const std::string& outfile = "", const variant& order = ::casac::initialize_variant(""));

    ::casac::image * rotate(const std::string& outfile = "", const std::vector<int>& shape = std::vector<int> (1, -1), const ::casac::variant& pa = ::casac::initialize_variant("0deg"), const ::casac::record& region = ::casac::initialize_record(""), const ::casac::variant& mask = ::casac::initialize_variant(""), const std::string& method = "cubic", const int decimate = 0, const bool replicate = false, const bool dropdeg = false, const bool overwrite = false, const bool async = false);

    bool rename(const std::string& name, const bool overwrite = false);

    bool replacemaskedpixels(const ::casac::variant& pixels, const ::casac::record& region = ::casac::initialize_record(""), const ::casac::variant& mask = ::casac::initialize_variant(""), const bool update = false, const bool list = false);

    ::casac::record* restoringbeam();

    ::casac::image * sepconvolve(const std::string& outfile = "", const std::vector<int>& axes = std::vector<int> (1, -1), const std::vector<std::string>& types = std::vector<std::string> (1, ""), const ::casac::variant& widths = ::casac::initialize_variant(""), const double scale = -1, const ::casac::record& region = ::casac::initialize_record(""), const ::casac::variant& mask = ::casac::initialize_variant(""), const bool overwrite = false, const bool async = false);

    bool set(const ::casac::variant& pixels, const int pixelmask = -1, const ::casac::record& region = ::casac::initialize_record(""), const bool list = false);

    bool setbrightnessunit(const std::string& unit);

    bool setcoordsys(const ::casac::record& csys);

    bool sethistory(const std::string& origin = "", const std::vector<std::string>& history = std::vector<std::string> (1, ""));

    bool setmiscinfo(const ::casac::record& info);

    std::vector<int> shape();

    bool setrestoringbeam(const ::casac::variant& major = ::casac::initialize_variant("1arcsec"), const ::casac::variant& minor = ::casac::initialize_variant("1arcsec"), const ::casac::variant& pa = ::casac::initialize_variant("0deg"), const ::casac::record& beam = ::casac::initialize_record(""), const bool remove = false, const bool log = true);

    ::casac::record* statistics(const std::vector<int>& axes = std::vector<int> (1, -1), const ::casac::record& region = ::casac::initialize_record(""), const ::casac::variant& mask = ::casac::initialize_variant(""), const std::vector<std::string>& plotstats = std::vector<std::string> (0), const std::vector<double>& includepix = std::vector<double> (1, -1), const std::vector<double>& excludepix = std::vector<double> (1, -1), const std::string& plotter = "", const int nx = 1, const int ny = 1, const bool list = true, const bool force = false, const bool disk = false, const bool robust = false, const bool verbose = true, const bool async = false);

    bool twopointcorrelation(const std::string& outfile = "", const ::casac::record& region = ::casac::initialize_record(""), const ::casac::variant& mask = ::casac::initialize_variant(""), const std::vector<int>& axes = std::vector<int> (1, -1), const std::string& method = "structurefunction", const bool overwrite = false);

    ::casac::image * subimage(const std::string& outfile = "", const ::casac::record& region = ::casac::initialize_record(""), const ::casac::variant& mask = ::casac::initialize_variant(""), const bool dropdeg = false, const bool overwrite = false, const bool list = true);

    std::vector<std::string> summary(::casac::record& header, const std::string& doppler = "RADIO", const bool list = true, const bool pixelorder = true);

    bool tofits(const std::string& outfile = "", const bool velocity = false, const bool optical = true, const int bitpix = -32, const double minpix = 1, const double maxpix = -1, const ::casac::record& region = ::casac::initialize_record(""), const ::casac::variant& mask = ::casac::initialize_variant(""), const bool overwrite = false, const bool dropdeg = false, const bool deglast = false, const bool dropstokes = false, const bool stokeslast = true, const bool async = false);

    bool toASCII(const std::string& outfile, const ::casac::record& region = ::casac::initialize_record(""), const ::casac::variant& mask = ::casac::initialize_variant(""), const std::string& sep = " ", const std::string& format = "%e", const double maskvalue = -999, const bool overwrite = false);

    ::casac::record* torecord();

    std::string type();

    ::casac::record* topixel(const ::casac::variant& value);

    ::casac::record* toworld(const ::casac::variant& value = ::casac::initialize_variant(""), const std::string& format = "n");

    bool unlock();

    ::casac::image * newimagefromarray(const std::string& outfile, const ::casac::variant& pixels, const ::casac::record& csys = ::casac::initialize_record(""), const bool linear = false, const bool overwrite = false, const bool log = true);

    ::casac::image * newimagefromfits(const std::string& outfile = "", const std::string& infile = "", const int whichrep = 0, const int whichhdu = 0, const bool zeroblanks = false, const bool overwrite = false);

    ::casac::image * newimagefromimage(const std::string& infile = "", const std::string& outfile = "", const ::casac::record& region = ::casac::initialize_record(""), const ::casac::variant& mask = ::casac::initialize_variant(""), const bool dropdeg = false, const bool overwrite = false);

    ::casac::image * newimagefromshape(const std::string& outfile = "", const std::vector<int>& shape = std::vector<int> (1, 0), const ::casac::record& csys = ::casac::initialize_record(""), const bool linear = false, const bool overwrite = false, const bool log = true);

    ::casac::variant* makearray(const double v = 0.0, const std::vector<int>& shape = std::vector<int> (1, 0));

    ::casac::record* setboxregion(const std::vector<double>& blc = std::vector<double> (1, -1), const std::vector<double>& trc = std::vector<double> (1, -1), const bool frac = false, const std::string& infile = "");

    ::casac::record* echo(const ::casac::record& v, const bool godeep = false);

    void outputvariant(::casac::variant& v);

    private:

		#include <xmlcasa/images/image_private.h>	

};

} // casac namespace
#endif

