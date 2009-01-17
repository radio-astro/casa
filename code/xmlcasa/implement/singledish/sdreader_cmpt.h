
/***
 * Framework independent header file for sdreader...
 *
 * Implement the sdreader component here.
 * 
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 ***/

#ifndef _sdreader_cmpt__H__
#define _sdreader_cmpt__H__
#ifndef _sdreader_cpnt__H__
#define _sdreader_cpnt__H__

#include <vector>
//#include <impl/casac/casac.h>
//#include <casac/variant.h>
#include <xmlcasa/record.h>
//#include <singledish/sdtable_cmpt.h>
#include <xmlcasa/singledish/sdreader_forward.h>

// put includes here

namespace casac {

/**
 * sdreader component class 
 *
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 **/
class sdreader
{
  private:
    bool fopenstat;
    bool rdatastat;	
  

  public:

    sdreader();
    sdreader(const std::string& filename, const int theif, const int thebeam);
    virtual ~sdreader();

    bool open(const std::string& filename, const int theif = -1, const int thebeam = -1);

    int read(const std::vector<int>& integrations = std::vector<int> (1, -1));

    bool close();

    bool reset();

    ::casac::sdtable * getdata();

    std::vector<int> header();

    private:

#include <xmlcasa/singledish/sdreader_private.h>

};

} // casac namespace
#endif
#endif

