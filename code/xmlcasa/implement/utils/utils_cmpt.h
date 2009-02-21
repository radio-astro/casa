
/***
 * Framework independent header file for utils...
 *
 * Implement the utils component here.
 * 
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 ***/

#ifndef _utils_cmpt__H__
#define _utils_cmpt__H__

#include <vector>
#include <xmlcasa/casac.h>

#include <xmlcasa/conversions.h>
#include <xmlcasa/record.h>
#include <xmlcasa/variant.h>
#include <xmlcasa/utils/utils_forward.h>

// put includes here

namespace casac {

/**
 * utils component class 
 *
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 **/
class utils
{
  private:

	

  public:

    utils();
    virtual ~utils();

    bool verify(const ::casac::record& input = ::casac::initialize_record(""), const ::casac::variant& xmldescriptor = ::casac::initialize_variant(""));

    bool setconstraints(const ::casac::variant& xmldescriptor = ::casac::initialize_variant(""));

    bool verifyparam(const ::casac::record& param = ::casac::initialize_record(""));

    ::casac::variant* expandparam(const std::string& name = "", const ::casac::variant& value = ::casac::initialize_variant(""));

    ::casac::record* torecord(const std::string& input);

    std::string toxml(const ::casac::record& input = ::casac::initialize_record(""), const bool asfile = false, const std::string& filename = "recordas.xml");
    std::string getrc(const std::string& rcvec = "");

    private:

	#include <xmlcasa/utils/utils_private.h>

};

} // casac namespace
#endif

