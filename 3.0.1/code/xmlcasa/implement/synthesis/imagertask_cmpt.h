
/***
 * Framework independent header file for imagertask...
 *
 * Implement the imagertask component here.
 * 
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 ***/

#ifndef _imagertask_cmpt__h__
#define _imagertask_cmpt__H__
#ifndef _imagertask_cpnt__h__
#define _imagertask_cpnt__H__

#include <vector>

// put includes here

namespace casac {

/**
 * imagertask component class 
 *
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 **/
class imagertask
{
  private:

	

  public:

    imagertask();
    virtual ~imagertask();

    void clean(const std::string& vis, const std::string& alg, const int niter, const double gain, const double threshold, const std::vector<std::string>& residual, const std::vector<std::string>& image, const std::vector<std::string>& model, const std::vector<std::string>& mask, const std::string& mode, const std::vector<int>& nchan = std::vector<int> (1,1), const std::vector<int>& start = std::vector<int> (1,1), const std::vector<int>& width = std::vector<int> (1,1), const std::vector<int>& step = std::vector<int> (1,1), const std::vector<int>& imsize = std::vector<int> (1,128), const std::vector<int>& cell = std::vector<int> (1,1), const std::string& stokes = "I", const std::vector<int>& fieldid = std::vector<int> (1,1), const std::vector<int>& spwid = std::vector<int> (1,1), const std::string& weighting = "uniform", const std::string& rmode = "none", const double robust = 0.0);

    void feather(const std::string& vis = "", const std::string& featherimage = "", const std::string& highres = "", const std::string& lowres = "", const std::string& lowpsf = "");

    void invert(const std::string& vis = "", const std::string& map = "", const std::string& beam = "", const std::string& mode = "", const std::vector<int>& nchan = std::vector<int> (1,1), const std::vector<int>& start = std::vector<int> (1,1), const std::vector<int>& width = std::vector<int> (1,1), const std::vector<int>& step = std::vector<int> (1,1), const std::vector<int>& imsize = std::vector<int> (1,128), const std::vector<int>& cell = std::vector<int> (1,1), const std::string& stokes = "I", const std::vector<int>& fieldid = std::vector<int> (1,1), const std::vector<int>& spwid = std::vector<int> (1,1), const std::string& weighting = "uniform", const std::string& rmode = "none", const double robust = 0.0);

    void mosaic(const std::string& vis, const std::string& alg, const int niter, const double gain, const double threshold, const std::vector<std::string>& residual, const std::vector<std::string>& image, const std::vector<std::string>& model, const std::vector<std::string>& mask, const std::string& mode = "", const std::string& gridfn = "SF", const std::string& grid = "mosaic", const std::vector<int>& nchan = std::vector<int> (1,1), const std::vector<int>& start = std::vector<int> (1,1), const std::vector<int>& width = std::vector<int> (1,1), const std::vector<int>& step = std::vector<int> (1,1), const std::vector<int>& imsize = std::vector<int> (1,128), const std::vector<int>& cell = std::vector<int> (1,1), const std::string& stokes = "I", const std::vector<int>& fieldid = std::vector<int> (1,1), const int reffieldid = 1, const std::vector<int>& spwid = std::vector<int> (1,1), const std::string& weighting = "uniform", const std::string& mosweight = "T", const std::string& rmode = "none", const double robust = 0.0, const double minpb = 0.1, const std::string& scaletype = "NONE", const int stoplargenegatives = -1, const double cyclefactor = 1.5, const double cyclespeedup = 2.0);

};

} // casac namespace
#endif
#endif

