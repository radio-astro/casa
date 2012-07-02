
/***
 * Framework independent header file for measures...
 *
 * Implement the measures component here.
 * 
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 ***/

#ifndef _measures_cmpt__H__
#define _measures_cmpt__H__

#include <vector>
#include <xmlcasa/casac.h>

#include <xmlcasa/conversions.h>
#include <xmlcasa/record.h>
#include <xmlcasa/variant.h>
#include <xmlcasa/measures/measures_forward.h>

// put includes here

namespace casac {

/**
 * measures component class 
 *
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 **/
class measures
{
  private:

	

  public:

    measures();
    virtual ~measures();

    std::string dirshow(const ::casac::record& v);

    std::string show(const ::casac::record& v, const bool refcode = true);

    ::casac::record* epoch(const std::string& rf = "UTC", const ::casac::variant& v0 = ::casac::initialize_variant(""), const ::casac::record& off = ::casac::initialize_record(""));

    ::casac::record* direction(const std::string& rf = "J2000", const ::casac::variant& v0 = ::casac::initialize_variant(""), const ::casac::variant& v1 = ::casac::initialize_variant(""), const ::casac::record& off = ::casac::initialize_record(""));

    ::casac::record* getvalue(const ::casac::record& v);

    std::string gettype(const ::casac::record& v);

    std::string getref(const ::casac::record& v);

    ::casac::record* getoffset(const ::casac::record& v);

    std::string cometname();

    std::string comettype();

    ::casac::record* comettopo();

    bool framecomet(const std::string& v = "");

    ::casac::record* position(const std::string& rf = "WGS84", const ::casac::variant& v0 = ::casac::initialize_variant(""), const ::casac::variant& v1 = ::casac::initialize_variant(""), const ::casac::variant& v2 = ::casac::initialize_variant(""), const ::casac::record& off = ::casac::initialize_record(""));

    ::casac::record* observatory(const std::string& name = "ATCA");

    std::string obslist();

    std::string linelist();

    ::casac::record* spectralline(const std::string& name = "HI");

    std::string sourcelist();

    ::casac::record* source(const ::casac::variant& name = ::casac::initialize_variant("1934-638"));

    ::casac::record* frequency(const std::string& rf = "LSRK", const ::casac::variant& v0 = ::casac::initialize_variant(""), const ::casac::record& off = ::casac::initialize_record(""));

    ::casac::record* doppler(const std::string& rf = "RADIO", const ::casac::variant& v0 = ::casac::initialize_variant(""), const ::casac::record& off = ::casac::initialize_record(""));

    ::casac::record* radialvelocity(const std::string& rf = "LSRK", const ::casac::variant& v0 = ::casac::initialize_variant(""), const ::casac::record& off = ::casac::initialize_record(""));

    ::casac::record* uvw(const std::string& rf = "ITRF", const ::casac::variant& v0 = ::casac::initialize_variant(""), const ::casac::variant& v1 = ::casac::initialize_variant(""), const ::casac::variant& v2 = ::casac::initialize_variant(""), const ::casac::record& off = ::casac::initialize_record(""));

    ::casac::record* touvw(::casac::record& dot, ::casac::record& xyz, const ::casac::record& v);

    ::casac::record* expand(::casac::record& xyz, const ::casac::record& v);

    ::casac::record* earthmagnetic(const std::string& rf = "IGRF", const ::casac::variant& v0 = ::casac::initialize_variant(""), const ::casac::variant& v1 = ::casac::initialize_variant(""), const ::casac::variant& v2 = ::casac::initialize_variant(""), const ::casac::record& off = ::casac::initialize_record(""));

    ::casac::record* baseline(const std::string& rf = "ITRF", const ::casac::variant& v0 = ::casac::initialize_variant(""), const ::casac::variant& v1 = ::casac::initialize_variant(""), const ::casac::variant& v2 = ::casac::initialize_variant(""), const ::casac::record& off = ::casac::initialize_record(""));

    ::casac::record* asbaseline(const ::casac::record& pos);

    ::casac::record* listcodes(const ::casac::record& ms);

    ::casac::record* measure(const ::casac::record& v, const std::string& rf, const ::casac::record& off = ::casac::initialize_record(""));

    bool doframe(const ::casac::record& v);

    bool framenow();

    std::string showframe();

    ::casac::record* toradialvelocity(const std::string& rf, const ::casac::record& v0);

    ::casac::record* tofrequency(const std::string& rf, const ::casac::record& v0, const ::casac::record& rfq);

    ::casac::record* todoppler(const std::string& rf, const ::casac::record& v0, const ::casac::variant& rfq = ::casac::initialize_variant(""));

    ::casac::record* torestfrequency(const ::casac::record& v0, const ::casac::record& d0);

    ::casac::record* rise(const ::casac::variant& crd = ::casac::initialize_variant(""), const ::casac::variant& ev = ::casac::initialize_variant("0.0deg"));

    ::casac::record* riseset(const ::casac::variant& crd, const ::casac::variant& ev = ::casac::initialize_variant("0.0deg"));

    ::casac::record* posangle(const ::casac::record& m1, const ::casac::record& m2);

    ::casac::record* separation(const ::casac::record& m1, const ::casac::record& m2);

    ::casac::record* addxvalue(const ::casac::record& a);

    std::string type();

    bool done();

    bool ismeasure(const ::casac::record& v);

    private:

	#include <xmlcasa/measures/measures_private.h>

};

} // casac namespace
#endif

