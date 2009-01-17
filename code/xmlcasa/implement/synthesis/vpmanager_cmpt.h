
/***
 * Framework independent header file for vpmanager...
 *
 * Implement the vpmanager component here.
 * 
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 ***/

#ifndef _vpmanager_cmpt__H__
#define _vpmanager_cmpt__H__

#include <vector>
#include <xmlcasa/casac.h>
#include <xmlcasa/conversions.h>
#include <xmlcasa/record.h>
#include <xmlcasa/variant.h>
#include <xmlcasa/synthesis/vpmanager_forward.h>

// put includes here

namespace casac {

/**
 * vpmanager component class 
 *
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 **/
class vpmanager
{
  private:

	

  public:

    vpmanager();
    virtual ~vpmanager();

    bool saveastable(const std::string& tablename = "");

    bool summarizevps(const bool verbose = false);

    bool done();

    ::casac::record* setcannedpb(const std::string& telescope = "VLA", const std::string& othertelescope = "", const bool dopb = true, const std::string& commonpb = "DEFAULT", const bool dosquint = false, const ::casac::variant& paincrement = ::casac::initialize_variant("720deg"), const bool usesymmetricbeam = false);

    ::casac::record* setpbairy(const std::string& telescope = "VLA", const std::string& othertelescope = "", const bool dopb = true, const ::casac::variant& dishdiam = ::casac::initialize_variant("25.0m"), const ::casac::variant& blockagediam = ::casac::initialize_variant("2.5m"), const ::casac::variant& maxrad = ::casac::initialize_variant("0.8deg"), const ::casac::variant& reffreq = ::casac::initialize_variant("1.0GHz"), const ::casac::variant& squintdir = ::casac::initialize_variant(""), const ::casac::variant& squintreffreq = ::casac::initialize_variant("1.0GHz"), const bool dosquint = false, const ::casac::variant& paincrement = ::casac::initialize_variant("720deg"), const bool usesymmetricbeam = false);

    ::casac::record* setpbcospoly(const std::string& telescope = "VLA", const std::string& othertelescope = "", const bool dopb = true, const std::vector<double>& coeff = std::vector<double> (1, -1), const std::vector<double>& scale = std::vector<double> (1, -1), const ::casac::variant& maxrad = ::casac::initialize_variant("0.8deg"), const ::casac::variant& reffreq = ::casac::initialize_variant("1.0GHz"), const std::string& isthispb = "PB", const ::casac::variant& squintdir = ::casac::initialize_variant(""), const ::casac::variant& squintreffreq = ::casac::initialize_variant("1.0GHz"), const bool dosquint = false, const ::casac::variant& paincrement = ::casac::initialize_variant("720deg"), const bool usesymmetricbeam = false);

    ::casac::record* setpbgauss(const std::string& telescope = "VLA", const std::string& othertelescope = "", const bool dopb = true, const ::casac::variant& halfwidth = ::casac::initialize_variant("0.5deg"), const ::casac::variant& maxrad = ::casac::initialize_variant("1.0deg"), const ::casac::variant& reffreq = ::casac::initialize_variant("1.0GHz"), const std::string& isthispb = "PB", const ::casac::variant& squintdir = ::casac::initialize_variant(""), const ::casac::variant& squintreffreq = ::casac::initialize_variant("1.0GHz"), const bool dosquint = false, const ::casac::variant& paincrement = ::casac::initialize_variant("720deg"), const bool usesymmetricbeam = false);

    ::casac::record* setpbinvpoly(const std::string& telescope = "VLA", const std::string& othertelescope = "", const bool dopb = true, const std::vector<double>& coeff = std::vector<double> (1, -1), const ::casac::variant& maxrad = ::casac::initialize_variant("0.8deg"), const ::casac::variant& reffreq = ::casac::initialize_variant("1.0GHz"), const std::string& isthispb = "PB", const ::casac::variant& squintdir = ::casac::initialize_variant(""), const ::casac::variant& squintreffreq = ::casac::initialize_variant("1.0"), const bool dosquint = false, const ::casac::variant& paincrement = ::casac::initialize_variant("720deg"), const bool usesymmetricbeam = false);

    ::casac::record* setpbnumeric(const std::string& telescope = "VLA", const std::string& othertelescope = "", const bool dopb = true, const std::vector<double>& vect = std::vector<double> (1, -1), const ::casac::variant& maxrad = ::casac::initialize_variant("0.8deg"), const ::casac::variant& reffreq = ::casac::initialize_variant("1.0GHz"), const std::string& isthispb = "PB", const ::casac::variant& squintdir = ::casac::initialize_variant(""), const ::casac::variant& squintreffreq = ::casac::initialize_variant("1.0GHz"), const bool dosquint = false, const ::casac::variant& paincrement = ::casac::initialize_variant("720deg"), const bool usesymmetricbeam = false);

    ::casac::record* setpbimage(const std::string& telescope = "VLA", const std::string& othertelescope = "", const bool dopb = true, const std::string& realimage = "", const std::string& imagimage = "");

    ::casac::record* setpbpoly(const std::string& telescope = "VLA", const std::string& othertelescope = "", const bool dopb = true, const std::vector<double>& coeff = std::vector<double> (1, -1), const ::casac::variant& maxrad = ::casac::initialize_variant("0.8deg"), const ::casac::variant& reffreq = ::casac::initialize_variant("1.0GHz"), const std::string& isthispb = "PB", const ::casac::variant& squintdir = ::casac::initialize_variant(""), const ::casac::variant& squintreffreq = ::casac::initialize_variant("1.0GHz"), const bool dosquint = false, const ::casac::variant& paincrement = ::casac::initialize_variant("720"), const bool usesymmetricbeam = false);

    private:

        #include <xmlcasa/synthesis/vpmanager_private.h>

};

} // casac namespace
#endif

