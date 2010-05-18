
/***
 * Framework independent header file for quanta...
 *
 * Implement the quanta component here.
 * 
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 ***/

#ifndef _quanta_cmpt__H__
#define _quanta_cmpt__H__

#include <vector>
#include <xmlcasa/casac.h>
#include <xmlcasa/conversions.h>
#include <xmlcasa/record.h>
#include <xmlcasa/variant.h>
#include <xmlcasa/casa/quanta_forward.h>

// put includes here

namespace casac {

/**
 * quanta component class 
 *
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 **/
class quanta
{
  private:

	

  public:

    quanta();
    virtual ~quanta();

    ::casac::record* convertfreq(const ::casac::variant& v = ::casac::initialize_variant("1.0"), const std::string& outunit = "Hz");

    ::casac::record* convertdop(const ::casac::variant& v = ::casac::initialize_variant("0.0"), const std::string& outunit = "km/s");

    ::casac::record* quantity(const ::casac::variant& v, const std::string& unitname = "");

    std::vector<double> getvalue(const ::casac::variant& v);

    std::string getunit(const ::casac::variant& v);

    ::casac::record* canonical(const ::casac::variant& v = ::casac::initialize_variant("1.0"));

    ::casac::record* canon(const ::casac::variant& v);

    ::casac::record* convert(const ::casac::variant& v, const ::casac::variant& outunit);

    bool define(const std::string& name, const ::casac::variant& v = ::casac::initialize_variant("1"));

    std::string map(const std::string& v = "all");

    ::casac::record* maprec(const std::string& v = "all");

    bool fits();

    std::vector<std::string> angle(const ::casac::variant& v, const int prec = 0, const std::vector<std::string>& form = std::vector<std::string> (1, ""), const bool showform = false);

    std::vector<std::string> time(const ::casac::variant& v, const int prec = 0, const std::vector<std::string>& form = std::vector<std::string> (1, ""), const bool showform = false);

    ::casac::record* add(const ::casac::variant& v, const ::casac::variant& a = ::casac::initialize_variant("0"));

    ::casac::record* sub(const ::casac::variant& v, const ::casac::variant& a = ::casac::initialize_variant("0"));

    ::casac::record* mul(const ::casac::variant& v, const ::casac::variant& a = ::casac::initialize_variant("1"));

    ::casac::record* div(const ::casac::variant& v, const ::casac::variant& a = ::casac::initialize_variant("1"));

    ::casac::record* neg(const ::casac::variant& v = ::casac::initialize_variant("1"));

    ::casac::record* norm(const ::casac::variant& v, const double a = -0.5);

    bool le(const ::casac::variant& v, const ::casac::variant& a = ::casac::initialize_variant("0"));

    bool lt(const ::casac::variant& v, const ::casac::variant& a = ::casac::initialize_variant("0"));

    bool eq(const ::casac::variant& v, const ::casac::variant& a = ::casac::initialize_variant("0"));

    bool ne(const ::casac::variant& v, const ::casac::variant& a = ::casac::initialize_variant("0"));

    bool gt(const ::casac::variant& v, const ::casac::variant& a = ::casac::initialize_variant("0"));

    bool ge(const ::casac::variant& v, const ::casac::variant& a = ::casac::initialize_variant("0"));

    ::casac::record* sin(const ::casac::variant& v);

    ::casac::record* cos(const ::casac::variant& v);

    ::casac::record* tan(const ::casac::variant& v);

    ::casac::record* asin(const ::casac::variant& v);

    ::casac::record* acos(const ::casac::variant& v);

    ::casac::record* atan(const ::casac::variant& v);

    ::casac::record* atan2(const ::casac::variant& v, const ::casac::variant& a);

    ::casac::record* abs(const ::casac::variant& v);

    ::casac::record* ceil(const ::casac::variant& v);

    ::casac::record* floor(const ::casac::variant& v);

    ::casac::record* log(const ::casac::variant& v);

    ::casac::record* log10(const ::casac::variant& v);

    ::casac::record* exp(const ::casac::variant& v);

    ::casac::record* sqrt(const ::casac::variant& v);

    bool compare(const ::casac::variant& v, const ::casac::variant& a);

    bool check(const std::string& v);

    bool checkfreq(const ::casac::variant& cm);

    ::casac::record* pow(const ::casac::variant& v, const int a = 1);

    ::casac::record* constants(const std::string& v = "pi");

    bool isangle(const ::casac::variant& v);

    ::casac::record* totime(const ::casac::variant& v);

    ::casac::record* toangle(const ::casac::variant& v);

    ::casac::record* splitdate(const ::casac::variant& v);

    std::string tos(const ::casac::variant& v, const int prec = 9);

    std::string type();

    bool done(const bool kill = false);

    ::casac::record* unit(const ::casac::variant& v, const std::string& unitname = "");

    bool isquantity(const ::casac::variant& v);

    bool setformat(const std::string& t = "", const std::string& v = "F");

    std::string getformat(const std::string& t = "");

    std::string formxxx(const ::casac::variant& v, const std::string& format = "dms", const int prec = 2);

    private:

#include <xmlcasa/casa/quanta_private.h>

};

} // casac namespace
#endif

