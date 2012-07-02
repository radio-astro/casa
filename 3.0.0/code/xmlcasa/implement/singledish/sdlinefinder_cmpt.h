
/***
 * Framework independent header file for sdlinefinder...
 *
 * Implement the sdlinefinder component here.
 * 
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 ***/

#ifndef _sdlinefinder_cmpt__H__
#define _sdlinefinder_cmpt__H__
#ifndef _sdlinefinder_cpnt__H__
#define _sdlinefinder_cpnt__H__

#include <vector>
//#include <impl/casac/casac.h>
#include <xmlcasa/record.h>
#include <xmlcasa/singledish/sdlinefinder_forward.h>

// put includes here

namespace casac {

/**
 * sdlinefinder component class 
 *
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 **/
class sdlinefinder
{
  private:

	

  public:

    sdlinefinder();
    virtual ~sdlinefinder();

    bool setoptions(const double threshold = sqrt(3.), const int minnchan = 1, const int avglimit = 8, const double boxsize = 0.2);

//    void setscan(const ::casac::record& scan, const std::vector<bool>& mask, const ::casac::record& edge);
    void setscan(const ::casac::record& scan, const std::vector<bool>& mask, const std::vector<int>& edge);

    int findlines(const int whichRow);

    std::vector<bool> getmask(const bool invert = false);

    std::vector<double> getlineranges();

    std::vector<int> getlinerangesinchannels();

    private:

#include <xmlcasa/singledish/sdlinefinder_private.h>

};

} // casac namespace
#endif
#endif

