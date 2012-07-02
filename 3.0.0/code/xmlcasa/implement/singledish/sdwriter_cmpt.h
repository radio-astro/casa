
/***
 * Framework independent header file for sdwriter...
 *
 * Implement the sdwriter component here.
 * 
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 ***/

#ifndef _sdwriter_cmpt__H__
#define _sdwriter_cmpt__H__
#ifndef _sdwriter_cpnt__H__
#define _sdwriter_cpnt__H__

#include <vector>
//#include <impl/casac/casac.h>

#include <xmlcasa/singledish/sdwriter_forward.h>

// put includes here

namespace casac {

/**
 * sdwriter component class 
 *
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 **/
class sdwriter
{
  private:

	

  public:

    sdwriter();
    virtual ~sdwriter();

    int setformat(const std::string& format = "SDFITS");

    int write(const std::string& intable, const std::string& filename, const bool tostokes);

    private:

#include <xmlcasa/singledish/sdwriter_private.h>

};

} // casac namespace
#endif
#endif

