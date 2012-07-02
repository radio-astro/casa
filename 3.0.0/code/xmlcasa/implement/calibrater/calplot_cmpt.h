
/***
 * Framework independent header file for calplot...
 *
 * Implement the calplot component here.
 * 
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 ***/

#ifndef _calplot_cmpt__H__
#define _calplot_cmpt__H__

#include <vector>
#include <xmlcasa/casac.h>
#include <xmlcasa/conversions.h>
#include <xmlcasa/variant.h>
#include <xmlcasa/calibrater/calplot_forward.h>

// put includes here

namespace casac {

/**
 * calplot component class 
 *
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 **/
class calplot
{
  private:

	

  public:

    calplot();
    virtual ~calplot();

    bool close();

    bool done();

    bool next();

    bool open(const std::string& caltable = "");

    bool plot(const std::string& xaxis = "time", const std::string& yaxis = "time");

    bool savefig( const std::string& filename = "", const int dpi = -1, const std::string& orientation = "", const std::string& papertype = "", const std::string& facecolor = "", const std::string& edgecolor = "");
	
    bool plotoptions(const int subplot = 111, const bool overplot = false, const std::string& iteration = "", const std::vector<double>& plotrange = std::vector<double> (1, 0.0), const bool showflags = false, const std::string& plotsymbol = ".", const std::string& plotcolor = "green", const double markersize = 5.0, const double fontsize = 10.0);

    bool markflags(const int panel = 0, const std::vector<double>& region = std::vector<double> (1, 0.0));

    bool flagdata();

    bool locatedata();

    bool selectcal(const ::casac::variant& antenna = ::casac::initialize_variant(""), const ::casac::variant& field = ::casac::initialize_variant(""), const ::casac::variant& spw = ::casac::initialize_variant(""), const ::casac::variant& time = ::casac::initialize_variant(""), const std::string& poln = """");

    bool stopiter(const bool rmplotter = false);

    bool clearplot(const int subplot = 000);

    private:

	#include <xmlcasa/calibrater/calplot_private.h>

};

} // casac namespace
#endif

