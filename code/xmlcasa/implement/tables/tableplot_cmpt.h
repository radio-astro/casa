/***
 * Framework independent header file for tableplot...
 *
 * Implement the tableplot component here.
 * 
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 ***/

#ifndef _tableplot_cmpt__H__
#define _tableplot_cmpt__H__

#include <vector>
#include <xmlcasa/casac.h>
#include <xmlcasa/conversions.h>
#include <xmlcasa/record.h>
#include <xmlcasa/tables/tableplot_forward.h>

// put includes here
namespace casac {

/**
 * tableplot component class 
 *
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 **/
class tableplot
{
  private:

	

  public:

    tableplot();
    virtual ~tableplot();

    bool open(const std::vector<std::string>& tabnames = std::vector<std::string> (1, ""));

    bool setgui(const bool gui = false);

    bool savefig(const std::string& filename = "", const int dpi = -1, const std::string& orientation = "", const std::string& papertype = "", const std::string& facecolor = "", const std::string& edgecolor = "");


    bool selectdata(const std::string& taqlstring = "");

    bool plotdata(const ::casac::record& poption = ::casac::initialize_record("1.0"), const std::vector<std::string>& labels = std::vector<std::string> (1, ""), const std::vector<std::string>& datastr = std::vector<std::string> (1, ""));

    bool iterplotstart(const ::casac::record& poption = ::casac::initialize_record("1.0"), const std::vector<std::string>& labels = std::vector<std::string> (1, ""), const std::vector<std::string>& datastr = std::vector<std::string> (1, ""), const std::vector<std::string>& iteraxes = std::vector<std::string> (1, ""));

    bool replot();

    int iterplotnext();

    bool iterplotstop(const bool rmplotter = false);

    bool markregions(const int nrows = 0, const int ncols = 0, const int panel = 1, const std::vector<double>& region = std::vector<double> (1, 0.0));

    bool flagdata();

    bool unflagdata();

    bool locatedata(const std::vector<std::string>& columnlist = std::vector<std::string> (1, ""));

    bool clearflags(const bool roottable = false);

    bool saveflagversion(const std::string& versionname = "", const std::string& comment = "", const std::string& merge = "");

    bool restoreflagversion(const std::vector<std::string>& versionname = std::vector<std::string> (1, ""), const std::string& merge = "");

    bool deleteflagversion(const std::vector<std::string>& versionname = std::vector<std::string> (1, ""));

    bool getflagversionlist();

    bool clearplot(const int nrows = 0, const int ncols = 0, const int panel = 0);

    bool done();

    private:

	#include <xmlcasa/tables/tableplot_private.h>

};

} // casac namespace
#endif

