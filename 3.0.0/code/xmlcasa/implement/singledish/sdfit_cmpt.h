
/***
 * Framework independent header file for sdfit...
 *
 * Implement the sdfit component here.
 * 
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 ***/

#ifndef _sdfit_cmpt__H__
#define _sdfit_cmpt__H__
#ifndef _sdfit_cpnt__H__
#define _sdfit_cpnt__H__

#include <vector>
//#include <impl/casac/casac.h>

#include <xmlcasa/singledish/sdfit_forward.h>

// put includes here

namespace casac {

/**
 * sdfit component class 
 *
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 **/
class sdfit
{
  private:

	

  public:

    sdfit();
    sdfit(const asap::SDFitTable& fittab);
    virtual ~sdfit();

    ::casac::sdfit * open(const std::string& tablename = "");

    int __len__(const int nfits_ = 0);

    std::vector<bool> getfixedparameters(const int which = 0);

    std::vector<double> getparameters(const int which = 0);

    std::vector<std::string> getfunctions(const int which = 0);

    std::vector<int> getcomponents(const int which = 0);

    std::vector<std::string> getframeinfo(const int which = 0);

    private:

#include <xmlcasa/singledish/sdfit_private.h>

};

} // casac namespace
#endif
#endif

