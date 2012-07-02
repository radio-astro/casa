
/***
 * Framework independent header file for asdm2msfillertask...
 *
 * Implement the asdm2msfillertask component here.
 * 
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 ***/

#ifndef _asdm2msfillertask_cmpt__H__
#define _asdm2msfillertask_cmpt__H__
#ifndef _asdm2msfillertask_cnpt__H__
#define _asdm2msfillertask_cnpt__H__

#include <vector>
#include <xmlcasa/casac.h>

#include <xmlcasa/alma/asdm2ms_forward.h>

// put includes here

namespace casac {

/**
 * asdm2msfillertask component class 
 *
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 **/
class asdm2msfillertask
{
  private:

	

  public:

    asdm2msfillertask();
    virtual ~asdm2msfillertask();

    bool fill(const std::string& msname, const std::string& inputfile, const bool complexdata = false, const bool overwrite = false);

    private:

	#include <xmlcasa/alma/asdm2ms_private.h>

};

} // casac namespace

#endif
#endif
