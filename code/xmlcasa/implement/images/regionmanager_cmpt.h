
/***
 * Framework independent header file for regionmanager...
 *
 * Implement the regionmanager component here.
 * 
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 ***/

#ifndef _regionmanager_cmpt__H__
#define _regionmanager_cmpt__H__

#include <vector>
#include <xmlcasa/casac.h>
#include <xmlcasa/conversions.h>
#include <xmlcasa/record.h>
#include <xmlcasa/variant.h>
#include <xmlcasa/images/regionmanager_forward.h>

// put includes here

namespace casac {

/**
 * regionmanager component class 
 *
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 **/
class regionmanager
{
  private:

	

  public:

    regionmanager();
    virtual ~regionmanager();

    std::string absreltype(const int absrelvalue = 0);

    ::casac::record* box(const std::vector<double>& blc = std::vector<double> (1, 0), const std::vector<double>& trc = std::vector<double> (1, -1), const std::vector<double>& inc = std::vector<double> (1, 1), const std::string& absrel = "abs", const bool frac = false, const std::string& comment = "");

    ::casac::record* complement(const ::casac::variant& region = ::casac::initialize_variant(""), const std::string& comment = "");

    ::casac::record* concatenation(const ::casac::variant& box, const ::casac::variant& regions, const std::string& comment = "");

    bool copyregions(const std::string& tableout = "", const std::string& tablein = "", const std::string& regionname = "");

    int dflt();

    bool deletefromtable(const std::string& tablename = "", const std::string& regionname = "");

    ::casac::record* difference(const ::casac::record& region1 = ::casac::initialize_record(""), const ::casac::record& region2 = ::casac::initialize_record(""), const std::string& comment = "");

    bool done();

    bool extension(const ::casac::record& box = ::casac::initialize_record(""), const ::casac::record& region = ::casac::initialize_record(""), const std::string& comment = "");

    ::casac::record* extractsimpleregions(const ::casac::record& region = ::casac::initialize_record(""));

    ::casac::record* fromfiletorecord(const std::string& filename = "", const bool verbose = true, const std::string& regionname = "");

    bool tofile(const std::string& filename = "", const ::casac::record& region = ::casac::initialize_record(""));

    bool fromglobaltotable(const std::string& tablename = "", const bool confirm = true, const bool verbose = true, const ::casac::variant& regionname = ::casac::initialize_variant(""), const ::casac::record& regions = ::casac::initialize_record(""));

    std::string fromrecordtotable(const std::string& tablename = "", const ::casac::variant& regionname = ::casac::initialize_variant(""), const ::casac::record& regionrec = ::casac::initialize_record(""), const bool verbose = true);

    ::casac::record* fromtabletorecord(const std::string& tablename = "", const ::casac::variant& regionname = ::casac::initialize_variant(""), const bool verbose = true);

    ::casac::record* intersection(const ::casac::variant& regions = ::casac::initialize_variant(""), const std::string& comment = "");

    bool ispixelregion(const ::casac::record& region = ::casac::initialize_record(""));

    bool isworldregion(const ::casac::record& region = ::casac::initialize_record(""));

    std::vector<std::string> namesintable(const std::string& tablename = "");

    ::casac::record* pixeltoworldregion(const ::casac::record& csys = ::casac::initialize_record(""), const std::vector<int>& shape = std::vector<int> (1, 0), const ::casac::record& region = ::casac::initialize_record(""));

    ::casac::record* quarter(const std::string& comment = "");

    bool setcoordinates(const ::casac::record& csys = ::casac::initialize_record(""));

    ::casac::record* makeunion(const ::casac::variant& regions = ::casac::initialize_variant(""), const std::string& comment = "");

    ::casac::record* wbox(const ::casac::variant& blc = ::casac::initialize_variant(""), const ::casac::variant& trc = ::casac::initialize_variant(""), const std::vector<int>& pixelaxes = std::vector<int> (1, -1), const ::casac::record& csys = ::casac::initialize_record(""), const std::string& absrel = "'abs'", const std::string& comment = "");

    ::casac::record* wpolygon(const ::casac::variant& x = ::casac::initialize_variant(""), const ::casac::variant& y = ::casac::initialize_variant(""), const std::vector<int>& pixelaxes = std::vector<int> (1, -1), const ::casac::record& csys = ::casac::initialize_record(""), const std::string& absrel = "abs", const std::string& comment = "");

    ::casac::record* wmask(const std::string& expr = "");

    private:

		#include <xmlcasa/images/regionmanager_private.h>	
        

};

} // casac namespace
#endif

