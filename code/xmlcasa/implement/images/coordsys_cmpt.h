
/***
 * Framework independent header file for coordsys...
 *
 * Implement the coordsys component here.
 * 
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 ***/

#ifndef _coordsys_cmpt__H__
#define _coordsys_cmpt__H__

#include <vector>
#include <xmlcasa/casac.h>
#include <xmlcasa/conversions.h>
#include <xmlcasa/record.h>
#include <xmlcasa/variant.h>
#include <xmlcasa/images/coordsys_forward.h>
#include <xmlcasa/images/coordsys_cmpt.h>

// put includes here

namespace casac {

/**
 * coordsys component class 
 *
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 **/
class coordsys
{
  private:

	

  public:

    coordsys();
    virtual ~coordsys();

    ::casac::coordsys * newcoordsys(const bool direction = false, const bool spectral = false, const std::vector<std::string>& stokes = std::vector<std::string> (1, ""), const int linear = 0, const bool tabular = false);

    bool addcoordinate(const bool direction = false, const bool spectral = false, const std::vector<std::string>& stokes = std::vector<std::string> (1, ""), const int linear = 0, const bool tabular = false);

    std::vector<int> axesmap(const bool toworld = true);

    std::vector<std::string> axiscoordinatetypes(const bool world = true);

    std::string conversiontype(const std::string& type = "direction");

    std::vector<double> convert(const std::vector<double>& coordin, const std::vector<bool>& absin = std::vector<bool> (1, true), const std::string& dopplerin = "radio", const std::vector<std::string>& unitsin = std::vector<std::string> (1, "Native"), const std::vector<bool>& absout = std::vector<bool> (1, true), const std::string& dopplerout = "radio", const std::vector<std::string>& unitsout = std::vector<std::string> (1, "Native"), const std::vector<int>& shape = std::vector<int> (1, -1));

    ::casac::variant* convertmany(const ::casac::variant& coordin, const std::vector<bool>& absin = std::vector<bool> (1, true), const std::string& dopplerin = "radio", const std::vector<std::string>& unitsin = std::vector<std::string> (1, "Native"), const std::vector<bool>& absout = std::vector<bool> (1, true), const std::string& dopplerout = "radio", const std::vector<std::string>& unitsout = std::vector<std::string> (1, "Native"), const std::vector<int>& shape = std::vector<int> (1, -1));

    std::vector<std::string> coordinatetype(const int which = -1);

    ::casac::coordsys * copy();

    bool done();

    ::casac::record* epoch();

    bool findaxis(int& coordinate, int& axisincoordinate, const bool world = true, const int axis = 0);

    bool findcoordinate(std::vector<int>& pixel, std::vector<int>& world, const std::string& type = "direction", const int which = 0);

    std::vector<double> frequencytofrequency(const std::vector<double>& value, const std::string& frequnit = "", const ::casac::variant& velocity = ::casac::initialize_variant(""), const std::string& doppler = "radio");

    std::vector<double> frequencytovelocity(const std::vector<double>& value, const std::string& frequnit = "", const std::string& doppler = "radio", const std::string& velunit = "km/s");

    bool fromrecord(const ::casac::record& record);

    ::casac::record* increment(const std::string& format = "n", const std::string& type = "");

    ::casac::variant* lineartransform(const std::string& type);

    std::vector<std::string> names(const std::string& type = "");

    int naxes(const bool world = true);

    int ncoordinates();

    std::string observer();

    ::casac::record* projection(const std::string& type = "");

    std::vector<std::string> referencecode(const std::string& type = "", const bool list = false);

    ::casac::record* referencepixel(const std::string& type = "");

    ::casac::record* referencevalue(const std::string& format = "n", const std::string& type = "");

    bool reorder(const std::vector<int>& order);

    bool replace(const ::casac::record& csys, const int whichin, const int whichout);

    ::casac::record* restfrequency();

    bool setconversiontype(const std::string& direction = "", const std::string& spectral = "");

    std::string getconversiontype(const std::string& type = "", const bool showconversion = true);

    bool setdirection(const std::string& refcode = "", const std::string& proj = "", const std::vector<double>& projpar = std::vector<double> (1, -1), const std::vector<double>& refpix = std::vector<double> (1, -1), const ::casac::variant& refval = ::casac::initialize_variant(""), const ::casac::variant& incr = ::casac::initialize_variant(""), const ::casac::variant& xform = ::casac::initialize_variant(""), const ::casac::variant& poles = ::casac::initialize_variant(""));

    bool setepoch(const ::casac::record& value);

    bool setincrement(const ::casac::variant& value = ::casac::initialize_variant(""), const std::string& type = "");

    bool setlineartransform(const std::string& type, const ::casac::variant& value);

    bool setnames(const std::vector<std::string>& value, const std::string& type = "");

    bool setobserver(const std::string& value);

    bool setprojection(const std::string& type, const std::vector<double>& parameters = std::vector<double> (1, -1));

    bool setreferencecode(const std::string& value, const std::string& type = "direction", const bool adjust = true);

    bool setreferencelocation(const std::vector<int>& pixel = std::vector<int> (1, -1), const ::casac::variant& world = ::casac::initialize_variant("-1"), const std::vector<bool>& mask = std::vector<bool> (1, false));

    bool setreferencepixel(const std::vector<double>& value, const std::string& type = "");

    bool setreferencevalue(const ::casac::variant& value, const std::string& type = "");

    bool setrestfrequency(const ::casac::variant& value, const int which = 0, const bool append = false);

    bool setspectral(const std::string& refcode = "", const ::casac::variant& restfreq = ::casac::initialize_variant(""), const ::casac::variant& frequencies = ::casac::initialize_variant("1GHz"), const std::string& doppler = "", const ::casac::variant& velocities = ::casac::initialize_variant("1km/s"));

    bool setstokes(const std::vector<std::string>& stokes);

    bool settabular(const std::vector<double>& pixel = std::vector<double> (1, -1), const std::vector<double>& world = std::vector<double> (1, -1), const int which = 0);

    bool settelescope(const std::string& value);

    bool setunits(const std::vector<std::string>& value, const std::string& type = "", const bool overwrite = false, const int which = -10);

    std::vector<std::string> stokes();

    std::vector<std::string> summary(const std::string& doppler = "RADIO", const bool list = true);

    std::string telescope();

    ::casac::record* toabs(const ::casac::variant& value, const int isworld = -1);

    ::casac::record* toabsmany(const ::casac::variant& value, const int isworld = -1);

    ::casac::record* topixel(const ::casac::variant& value);

    ::casac::record* topixelmany(const ::casac::variant& value);

    ::casac::record* torecord();

    ::casac::record* torel(const ::casac::variant& value, const int isworld = -1);

    ::casac::record* torelmany(const ::casac::variant& value, const int isworld = -1);

    ::casac::record* toworld(const ::casac::variant& value = ::casac::initialize_variant(""), const std::string& format = "n");

    ::casac::record* toworldmany(const ::casac::variant& value);

    std::string type();

    std::vector<std::string> units(const std::string& type = "");

    std::vector<double> velocitytofrequency(const std::vector<double>& value, const std::string& frequnit = "", const std::string& doppler = "radio", const std::string& velunit = "km/s");

    std::string parentname();

    bool setparentname(const std::string& imagename);

    private:

		#include <xmlcasa/images/coordsys_private.h>
	

};

} // casac namespace
#endif

