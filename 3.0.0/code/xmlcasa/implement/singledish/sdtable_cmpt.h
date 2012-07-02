
/***
 * Framework independent header file for sdtable...
 *
 * Implement the sdtable component here.
 * 
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 ***/

#ifndef _sdtable_cmpt__H__
#define _sdtable_cmpt__H__
#ifndef _sdtable_cpnt__H__
#define _sdtable_cpnt__H__

#include <vector>
//#include <impl/casac/casac.h>
#include <xmlcasa/singledish/sdtable_forward.h>

// put includes here

namespace casac {

/**
 * sdtable component class 
 *
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 **/
class sdtable
{
  private:

	

  public:

    sdtable();
    sdtable(const asap::SDMemTableWrapper& mt);
    virtual ~sdtable();

    ::casac::sdtable * copy(const std::string& tosdtable = "");

    bool open(const std::string& insdtable = "");

    int getif();

    int getbeam();

    int getpol();

    std::string lines();

    int nif();

    int nbeam();

    int npol();

    int nchan();

    int nrow();

    bool setif(const int whichif);

    bool setbeam(const int whichbeam);

    bool setpol(const int whichpol);

    std::string getfluxunit();

    bool setfluxunit(const std::string& unit);

    bool setinstrument(const std::string& instrument);

    ::casac::sdtable * getscan(const std::vector<int>& scan);

    ::casac::sdtable * getsource(const std::string& source);

    std::vector<double> getspectrum(const int whichRow = 0);

    int nstokes();

    std::vector<double> getstokesspectrum(const int whichRow = 0, const bool doPol = false);

    std::vector<double> stokestopolspectrum(const int whichRow = 0, const bool tolinear = false, const int thepol = -1);

    std::string getpolarizationlabel(const bool linear = false, const bool stokes = false, const bool linpol = false);

    bool setspectrum(const std::vector<double>& spectrum, const int whichRow = 0);

    std::vector<double> getabcissa(const int whichRow = 0);

    std::string getabcissalabel(const int whichRow = 0);

    std::vector<bool> getmask(const int whichRow = 0);

    std::vector<double> gettsys();

    std::string getsourcename(const int whichRow = 0);

    double getelevation(const int whichRow = 0);

    double getazimuth(const int whichRow = 0);

    double getparangle(const int whichRow = 0);

    std::string gettime(const int whichRow = 0);

    bool flag(const int whichRow = -1);

    bool save(const std::string& filename);

    bool summary(const bool verbose = true);

    std::vector<double> getrestfreqs();

    bool setrestfreqs(const std::vector<double>& restfreqs, const std::string& unit, const std::vector<std::string>& lines, const std::string& source, const int whichif);

    bool setcoordinfo(const std::vector<std::string>& theinfo);

    std::vector<std::string> getcoordinfo();

    std::vector<std::string> gethistory();

    bool addhistory(const std::string& theinfo);

    bool addfit(const int whichRow, const std::vector<double>& p, const std::vector<bool>& m, const std::vector<std::string>& f, const std::vector<int>& c);

    bool recalc_azel();

    bool close();

    private:

#include <xmlcasa/singledish/sdtable_private.h>

};

} // casac namespace
#endif
#endif

