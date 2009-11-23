
/***
 * Framework independent header file for simulator...
 *
 * Implement the simulator component here.
 * 
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 ***/

#ifndef _simulator_cmpt__H__
#define _simulator_cmpt__H__

#include <vector>
#include <xmlcasa/casac.h>
#include <xmlcasa/conversions.h>
#include <xmlcasa/variant.h>
#include <xmlcasa/synthesis/simulator_forward.h>

// put includes here

namespace casac {

/**
 * simulator component class 
 *
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 **/
class simulator
{
  private:

	

  public:

    simulator();
    virtual ~simulator();

    bool open(const std::string& ms = "");

    bool openfromms(const std::string& ms = "");

    bool close();

    bool done();

    std::string name();

    bool summary();

    std::string type();

    bool settimes(const ::casac::variant& integrationtime = ::casac::initialize_variant("10s"), const bool usehourangle = true, const ::casac::variant& referencetime = ::casac::initialize_variant("50000.0d"));

    bool observe(const std::string& sourcename = "", const std::string& spwname = "", const ::casac::variant& starttime = ::casac::initialize_variant("0s"), const ::casac::variant& stoptime = ::casac::initialize_variant("3600s"));

    bool setlimits(const double shadowlimit = 1e-6, const ::casac::variant& elevationlimit = ::casac::initialize_variant("10deg"));

    bool setauto(const double autocorrwt = 0.0);

    bool setconfig(const std::string& telescopename = "ALMA", 
		   const std::vector<double>& x = std::vector<double> (1, 0), 
		   const std::vector<double>& y = std::vector<double> (1, 0), 
		   const std::vector<double>& z = std::vector<double> (1, 0), 
		   const std::vector<double>& dishdiameter = std::vector<double> (1, 0), 
		   const std::vector<double>& offset = std::vector<double> (1, 0), 
		   const std::vector<std::string>& mount = std::vector<std::string> (1, "ALT-AZ"), 
		   const std::vector<std::string>& antname = std::vector<std::string> (1, "A"), 
		   const std::vector<std::string>& padname = std::vector<std::string> (1, "P"), 
		   const std::string& coordsystem = "global", 
		   const ::casac::variant& referencelocation = ::casac::initialize_variant("ALMA"));

    bool setknownconfig(const std::string& arrayname = "VLA");

    bool setfeed(const std::string& mode, const std::vector<double>& x, const std::vector<double>& y, const std::vector<std::string>& pol);

    bool setfield(const std::string& sourcename = "SOURCE", const ::casac::variant& sourcedirection = ::casac::initialize_variant(""), const std::string& calcode = "", const ::casac::variant& distance = ::casac::initialize_variant("0m"));

    bool setmosaicfield(const std::string& sourcename = "SOURCE", const std::string& calcode = "", const ::casac::variant& fieldcenter = ::casac::initialize_variant(""), const int xmosp = 1, const int ymosp = 1, const ::casac::variant& mosspacing = ::casac::initialize_variant("1arcsec"), const ::casac::variant& distance = ::casac::initialize_variant("0m"));

    bool setspwindow(const std::string& spwname = "XBAND", const ::casac::variant& freq = ::casac::initialize_variant("8.0e9Hz"), const ::casac::variant& deltafreq = ::casac::initialize_variant("50e6Hz"), const ::casac::variant& freqresolution = ::casac::initialize_variant("50.e6Hz"), const int nchannels = 1, const std::string& stokes = "RR LL");

    bool setdata(const std::vector<int>& spwid = std::vector<int> (1, 0), const std::vector<int>& fieldid = std::vector<int> (1, 0), const std::string& msselect = "");

    bool predict(const std::vector<std::string>& imagename = std::vector<std::string> (1, ""), const std::string& complist = "", const bool incremental = false);

    bool setoptions(const std::string& ftmachine = "ft", const int cache = 0, const int tile = 16, const std::string& gridfunction = "SF", const ::casac::variant& location = ::casac::initialize_variant("ALMA"), const double padding = 1.3, const int facets = 1, const double maxdata = 2000.0, const int wprojplanes = 1);

    bool setvp(const bool dovp = true, const bool usedefaultvp = true, const std::string& vptable = "", const bool dosquint = true, const ::casac::variant& parangleinc = ::casac::initialize_variant("360deg"), const ::casac::variant& skyposthreshold = ::casac::initialize_variant("180deg"), const double pblimit = 1.0e-2);

    bool corrupt();

    bool reset();

    bool setbandpass(const std::string& mode = "calculate", const std::string& table = "", const ::casac::variant& interval = ::casac::initialize_variant("3600s"), const std::vector<double>& amplitude = std::vector<double> (1, 0.0));

    bool setapply(const std::string& table = "", const std::string& type = "", const double t = 0.0, const ::casac::variant& field = ::casac::initialize_variant(""), const std::string& interp = "linear", const std::string& select = "", const bool calwt = false, const std::vector<int>& spwmap = std::vector<int> (1, -1), const double opacity = 0.0);

    bool setgain(const std::string& mode = "calculate", const std::string& table = "", 
		 const ::casac::variant& interval = ::casac::initialize_variant("10s"), 
		 const double amplitude = 0.01);

    bool settrop(const std::string& mode = "calculate", const std::string& table = "", const double pwv = 3.0, const double deltapwv = 0.5, const double beta = 2.6, const double windspeed = 7);

    bool setpointingerror(const std::string& epjtablename = "", const bool applypointingoffsets = false, const bool dopbcorrection = false);

    bool setleakage(const std::string& mode = "calculate", const std::string& table = "", const ::casac::variant& interval = ::casac::initialize_variant("18000s"), const double amplitude = 0.0);

    bool oldsetnoise(const std::string& mode = "calculate", const std::string& table = "", const ::casac::variant& simplenoise = ::casac::initialize_variant("0.0Jy"), const double antefficiency = 0.8, const double correfficiency = 0.85, const double spillefficiency = 0.85, const double tau = 0.0, const double trx = 50, const double tatmos = 250.0, const double tcmb = 2.7);

    bool setnoise(const std::string& mode = "calculate", const std::string& table = "", const ::casac::variant& simplenoise = ::casac::initialize_variant("0.0Jy"), const double antefficiency = 0.8, const double correfficiency = 0.85, const double spillefficiency = 0.85, const double tau = 0.0, const double trx = 50, const double tatmos = 250.0, const double tground = 250.0, const double tcmb = 2.7);

    bool setpa(const std::string& mode = "calculate", const std::string& table = "", const ::casac::variant& interval = ::casac::initialize_variant("10s"));

    bool setseed(const int seed = 185349251);

    private:

	#include <xmlcasa/synthesis/simulator_private.h>

};

} // casac namespace
#endif

