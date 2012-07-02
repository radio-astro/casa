
/***
 * Framework independent header file for vlafillertask...
 *
 * Implement the vlafillertask component here.
 * 
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 ***/

#ifndef _vlafillertask_cmpt__H__
#define _vlafillertask_cmpt__H__

#include <vector>
#include <xmlcasa/casac.h>
#include <xmlcasa/variant.h>
#include <xmlcasa/conversions.h>

// put includes here

namespace casac {

/**
 * vlafillertask component class 
 *
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 **/
class vlafillertask
{
  private:

	

  public:

    vlafillertask();
    virtual ~vlafillertask();

    void fill(const std::string& msname, 
	      const std::string& inputfile, 
	      const std::string& project = "", 
	      const std::string& start = "1970/1/1/00:00:00", 
	      const std::string& stop = "2199/1/1/23:59:59", 
	      const std::string& centerfreq = "1.0e18Hz", 
	      const std::string& bandwidth = "2.0e18Hz", 
	      const std::string& bandname = "*", 
	      const std::string& source = "", 
	      const int subarray = 0, 
	      const int qualifier = -65536, 
	      const std::string& calcode = "#", 
	      const bool overwrite = false, 
	      const ::casac::variant& freqtol = ::casac::initialize_variant("0.0"),
	      const bool applytsys = true, 
	      const bool keepautocorr = false, 
	      const std::string& antnamescheme = "new", 
	      const int useday = 1,
	      const bool keepblanks = false,
	      const bool evlabands = true);

};

} // casac namespace
#endif

