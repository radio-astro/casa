
/***
 * Framework independent header file for calibrater...
 *
 * Implement the calibrater component here.
 * 
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 ***/

#ifndef _calibrater_cmpt__H__
#define _calibrater_cmpt__H__

#include <vector>
#include <xmlcasa/casac.h>
#include <xmlcasa/Quantity.h>
#include <xmlcasa/conversions.h>
#include <xmlcasa/variant.h>
#include <xmlcasa/synthesis/calibrater_forward.h>

// put includes here

namespace casac {

/**
 * calibrater component class 
 *
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 **/
class calibrater
{
  private:

	

  public:

    calibrater();
    virtual ~calibrater();

    bool open(const std::string& filename, const bool compress = false, const bool addscratch = true);

    bool selectvis(const ::casac::variant& time = ::casac::initialize_variant(""), const ::casac::variant& spw = ::casac::initialize_variant(""), const ::casac::variant& scan = ::casac::initialize_variant(""), const ::casac::variant& field = ::casac::initialize_variant(""), const ::casac::variant& baseline = ::casac::initialize_variant(""), const ::casac::variant& uvrange = ::casac::initialize_variant(""), const std::string& chanmode = "none", const int nchan = 1, const int start = 0, const int step = 1, const Quantity& mstart = Quantity(std::vector<double> (1, 0.0),"km/s"), const Quantity& mstep = Quantity(std::vector<double> (1, 0.0),"km/s"), const std::string& msselect = "");

    bool setmodel(const std::string& modelimage);

    bool setapply(const std::string& type = "", const double t = 0.0, const std::string& table = "", const ::casac::variant& field = ::casac::initialize_variant(""), const std::string& interp = "linear", const std::string& select = "", const bool calwt = false, const std::vector<int>& spwmap = std::vector<int> (1, -1), const double opacity = 0.0);

    bool setsolve(const std::string& type, const ::casac::variant& t, const std::string& table, const bool append = false, const double preavg = -1.0, const bool phaseonly = false, const std::string& apmode = "AP", const ::casac::variant& refant = ::casac::initialize_variant(""), const int minblperant = 4, const bool solnorm = false, const float minsnr = 0.0, const std::string& combine = "", const int fillgaps = 0, const std::string& cfcache = "", const float painc = 360.0);

    bool setsolvegainspline(const std::string& table = "", const bool append = false, const std::string& mode = "PHAS", const double splinetime = 10800, const double preavg = 0.0, const int npointaver = 10, const double phasewrap = 250, const ::casac::variant& refant = ::casac::initialize_variant(""));

    bool setsolvebandpoly(const std::string& table = "", const bool append = false, const ::casac::variant& t = ::casac::initialize_variant(""), const std::string& combine = "", const int degamp = 3, const int degphase = 3, const bool visnorm = false, const bool solnorm = true, const int maskcenter = 0, const double maskedge = 5.0, const ::casac::variant& refant = ::casac::initialize_variant(""));

    bool state();

    bool reset(const bool apply = true, const bool solve = true);

    bool initcalset(const int calset = 1);

    bool solve();

    bool correct();

    bool corrupt();

    bool fluxscale(std::vector<double>& fluxd, const std::string& tablein, const ::casac::variant& reference = ::casac::initialize_variant(""), const std::string& tableout = "", const ::casac::variant& transfer = ::casac::initialize_variant(""), const bool append = false, const std::vector<int>& refspwmap = std::vector<int> (1, -1));

    bool accumulate(const std::string& tablein = "", const std::string& incrtable = "", const std::string& tableout = "", const ::casac::variant& field = ::casac::initialize_variant(""), const ::casac::variant& calfield = ::casac::initialize_variant(""), const std::string& interp = "linear", const double t = -1.0, const std::vector<int>& spwmap = std::vector<int> (1, -1));

    bool specifycal(const std::string& caltable = "", const std::string& time = "", const std::string& spw = "", const std::string& antenna = "", const std::string& pol = "", const std::string& caltype = "", const std::vector<double>& parameter = std::vector<double> (1, 1.0));

    bool smooth(const std::string& tablein, const std::string& tableout, const ::casac::variant& field = ::casac::initialize_variant(""), const std::string& smoothtype = "median", const double smoothtime = 60.0);

    bool listcal(const std::string& caltable, const ::casac::variant& field, const ::casac::variant& antenna, const ::casac::variant& spw, const std::string& listfile, const int pagerows = 50);

    bool posangcal(const std::vector<double>& posangcor, const std::string& tablein, const std::string& tableout = "");

    bool linpolcor(const std::string& tablein = "", const std::string& tableout = "", const std::vector<std::string>& fields = std::vector<std::string> (1, """"));

    bool plotcal(const std::vector<int>& antennas, const std::vector<int>& fields, const std::vector<int>& spwids, const std::string& plottype = "AMP", const std::string& tablename = "", const int polarization = 1, const bool multiplot = false, const int nx = 1, const int ny = 1, const std::string& psfile = "");

    std::vector<double> modelfit(const std::vector<bool>& vary, const int niter = 0, const std::string& compshape = "P", const std::vector<double>& par = ::casac::initialize_vector( 3, (double) 1.0, (double) 0.0, (double) 0.0 ), const std::string& file = "");

    bool close();

    bool done();

    private:

	#include <xmlcasa/synthesis/calibrater_private.h>

};

} // casac namespace
#endif

