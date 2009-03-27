
/***
 * Framework independent header file for sdfitter...
 *
 * Implement the sdfitter component here.
 * 
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 ***/

#ifndef _sdfitter_cmpt__H__
#define _sdfitter_cmpt__H__
#ifndef _sdfitter_cpnt__H__
#define _sdfitter_cpnt__H__

#include <vector>
//#include <impl/casac/casac.h>
#include <xmlcasa/singledish/sdfitter_forward.h>

// put includes here

namespace casac {

/**
 * sdfitter component class 
 *
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 **/
class sdfitter
{
  private:

	

  public:

    sdfitter();
    virtual ~sdfitter();

    bool setdata(const std::vector<double>& xdat, const std::vector<double>& ydat, const std::vector<bool>& mask = std::vector<bool> (1,false));

    bool setexpression(const std::string& expr = "poly", const int ncomp = 1);

    std::vector<double> getresidual();

    std::vector<double> getfit();

    std::vector<bool> getfixedparameters();

    bool setfixedparameters(const std::vector<bool>& fxd = std::vector<bool> (1,false));

    std::vector<double> getparameters();

    bool setparameters(const std::vector<double>& params = std::vector<double> (1,0));

    std::vector<double> getestimate();

    bool estimate();

    std::vector<double> geterrors();

    double getchi2();

    bool reset();

    bool fit();

    std::vector<double> evaluate(const int whichComp = 0);

    private:

#include <xmlcasa/singledish/sdfitter_private.h>

};

} // casac namespace
#endif
#endif

