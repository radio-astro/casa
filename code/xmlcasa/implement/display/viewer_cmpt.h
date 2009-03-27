
/***
 * Framework independent header file for viewer...
 *
 * Implement the viewer component here.
 * 
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 ***/

#ifndef _viewer_cmpt__H__
#define _viewer_cmpt__H__
#ifndef _viewer_cnpt__H__
#define _viewer_cnpt__H__

#include <vector>
#include <xmlcasa//casac.h>

#include <xmlcasa/display/viewer_forward.h>

// put includes here

namespace casac {

/**
 * viewer component class 
 *
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 **/
class viewer
{
  private:

	

  public:

    viewer();
    virtual ~viewer();

    bool show(const std::string& filename = "", const std::string& datatype = "image", const std::string& displaytype = "raster");

    private:

	#include <xmlcasa/display/viewer_private.h>

};

} // casac namespace
#endif
#endif

