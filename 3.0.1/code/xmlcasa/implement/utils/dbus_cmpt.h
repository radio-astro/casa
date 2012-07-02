
/***
 * Framework independent header file for dbus...
 *
 * Implement the dbus component here.
 * 
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 ***/

#ifndef _dbus_cmpt__H__
#define _dbus_cmpt__H__

#include <vector>
#include <xmlcasa/casac.h>

#include <xmlcasa/conversions.h>
#include <xmlcasa/record.h>
#include <xmlcasa/utils/dbus_forward.h>

// put includes here

namespace casac {

/**
 * dbus component class 
 *
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 **/
class dbus
{
  private:

	

  public:

    dbus();
    virtual ~dbus();

    bool asyncCallMethod(const std::string& objectName, const std::string& methodName, const ::casac::record& parameters);

    private:

    #include <xmlcasa/utils/dbus_private.h>
  

};

} // casac namespace
#endif

