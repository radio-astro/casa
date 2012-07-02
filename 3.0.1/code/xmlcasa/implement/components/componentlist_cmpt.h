
/***
 * Framework independent header file for componentlist...
 *
 * Implement the componentlist component here.
 * 
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 ***/

#ifndef _componentlist_cmpt__H__
#define _componentlist_cmpt__H__

#include <vector>
#include <xmlcasa/casac.h>
#include <xmlcasa/conversions.h>
#include <xmlcasa/record.h>
#include <xmlcasa/variant.h>
#include <xmlcasa/components/componentlist_forward.h>

// put includes here

namespace casac {

/**
 * componentlist component class 
 *
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 **/
class componentlist
{
  private:

	

  public:

    componentlist();
    virtual ~componentlist();

    bool open(const std::string& filename = "", const bool nomodify = false, const bool log = true);

    int asciitocomponentlist(const std::string& filename, const std::string& asciifile, const std::string& refer, const std::string& format, const ::casac::record& direction, const ::casac::record& spectrum, const ::casac::record& flux, const bool log = true);

    bool concatenate(const ::casac::variant& list = ::casac::initialize_variant(""), const std::vector<int>& which = std::vector<int> (1, -1), const bool log = true);

    bool fromrecord(const ::casac::record& record = ::casac::initialize_record(""));

    ::casac::record* torecord();

    bool remove(const std::vector<int>& which = std::vector<int> (1, -1), const bool log = true);

    bool purge();

    bool recover(const bool log = true);

    int length();

    std::vector<int> indices();

    bool sort(const std::string& criteria = "Flux", const bool log = true);

    bool isphysical(const std::vector<int>& which = std::vector<int> (1, -1));

    std::vector<double> sample(const ::casac::variant& direction = ::casac::initialize_variant("J2000 00h00m00.00 90d00m00.0"), const ::casac::variant& pixellatsize = ::casac::initialize_variant("0.0deg"), const ::casac::variant& pixellongsize = ::casac::initialize_variant("0.0deg"), const ::casac::variant& frequency = ::casac::initialize_variant("1.4GHz"));

    bool rename(const std::string& filename, const bool log = true);

    bool simulate(const int howmany = 1, const bool log = true);

    bool addcomponent(const ::casac::variant& flux = ::casac::initialize_variant(""), const std::string& fluxunit = "Jy", const std::string& polarization = "Stokes", const ::casac::variant& dir = ::casac::initialize_variant("J2000 00h00m00.0 90d00m00.0"), const std::string& shape = "point", const ::casac::variant& majoraxis = ::casac::initialize_variant("2.0arcmin"), const ::casac::variant& minoraxis = ::casac::initialize_variant("1.0arcmin"), const ::casac::variant& positionangle = ::casac::initialize_variant("0.0deg"), const ::casac::variant& freq = ::casac::initialize_variant("LSRK 1.415GHz"), const std::string& spectrumtype = "constant", const double index = 1.0, const std::string& label = "");

    bool close(const bool log = true);

    bool edit(const int which, const bool log = true);

    bool done();

    bool select(const int which);

    bool deselect(const int which);

    std::vector<int> selected();

    std::string getlabel(const int which);

    bool setlabel(const int which, const std::string& value, const bool log = true);

    std::vector<double> getfluxvalue(const int which);

    std::string getfluxunit(const int which);

    std::string getfluxpol(const int which);

    std::vector<double> getfluxerror(const int which);

    bool setflux(const int which, const ::casac::variant& value = ::casac::initialize_variant(""), const std::string& unit = "Jy", const std::string& polarization = "Stokes", const ::casac::variant& error = ::casac::initialize_variant(""), const bool log = true);

    bool convertfluxunit(const int which, const std::string& unit = "Jy");

    bool convertfluxpol(const int which, const std::string& polarization = "Stokes");

    ::casac::record* getrefdir(const int which);

    std::string getrefdirra(const int which, const std::string& unit = "deg", const int precision = 6);

    std::string getrefdirdec(const int which, const std::string& unit = "deg", const int precision = 6);

    std::string getrefdirframe(const int which);

    bool setrefdir(const int which = 1, const ::casac::variant& ra = ::casac::initialize_variant(""), const ::casac::variant& dec = ::casac::initialize_variant(""), const bool log = true);

    bool setrefdirframe(const int which, const std::string& frame, const bool log = true);

    bool convertrefdir(const int which, const std::string& frame);

    std::string shapetype(const int which);

    ::casac::record* getshape(const int which);

    bool setshape(const int which, const std::string& type = "Point", const ::casac::variant& majoraxis = ::casac::initialize_variant("1.0arcmin"), const ::casac::variant& minoraxis = ::casac::initialize_variant("1.0arcmin"), const ::casac::variant& positionangle = ::casac::initialize_variant("0.0deg"), const ::casac::variant& majoraxiserror = ::casac::initialize_variant("0.0arcmin"), const ::casac::variant& minoraxiserror = ::casac::initialize_variant("0.0arcmin"), const ::casac::variant& positionangleerror = ::casac::initialize_variant("0.0deg"), const bool log = true);

    bool convertshape(const int which, const std::string& majoraxis = "arcmin", const std::string& minoraxis = "arcmin", const std::string& positionangle = "deg");

    std::string spectrumtype(const int which);

    ::casac::record* getspectrum(const int which);

    bool setspectrum(const int which, const std::string& type = "Constant", const double index = 0.0);

    ::casac::record* getfreq(const int which);

    double getfreqvalue(const int which);

    std::string getfrequnit(const int which);

    std::string getfreqframe(const int which);

    bool setfreq(const int which, const double value, const std::string& unit = "'GHz'", const bool log = true);

    bool setfreqframe(const int which, const std::string& frame = "LSRK", const bool log = true);

    bool convertfrequnit(const int which, const std::string& unit = "'GHz'");

    ::casac::record* getcomponent(const int which, const bool iknow = false);

    bool add(const ::casac::record& thecomponent, const bool iknow = true);

    bool replace(const int which, const ::casac::record& list, const std::vector<int>& whichones = std::vector<int> (1, -1));

    bool print(const int which = -1);

    bool iscomponentlist(const ::casac::variant& tool);

    private:

				#include <xmlcasa/components/componentlist_private.h>
			

};

} // casac namespace
#endif

