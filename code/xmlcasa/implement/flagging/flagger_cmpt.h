
/***
 * Framework independent header file for flagger...
 *
 * Implement the flagger component here.
 * 
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 ***/

#ifndef _flagger_cmpt__H__
#define _flagger_cmpt__H__

#include <vector>
#include <xmlcasa/casac.h>
#include <xmlcasa/conversions.h>
#include <xmlcasa/record.h>
#include <xmlcasa/flagging/flagger_forward.h>

// put includes here

namespace casac {

/**
 * flagger component class 
 *
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 **/
class flagger
{
  private:

	

  public:

    flagger();
    virtual ~flagger();

    bool open(const std::string& msname = """");

    bool setdata(const std::string& field, const std::string& spw, const std::string& array, const std::string& feed, const std::string& scan, const std::string& baseline, const std::string& uvrange, const std::string& time = """", const std::string& correlation = """");

    bool setmanualflags(const std::string& field, const std::string& spw, const std::string& array, const std::string& feed, const std::string& scan, const std::string& baseline, const std::string& uvrange, const std::string& time = """", const std::string& correlation = """", const bool autocorrelation = false, const bool unflag = false, const std::string& clipexpr = """", const std::vector<double>& cliprange = std::vector<double> (0), const std::string& clipcolumn = "DATA", const bool outside = true, const bool channelavg = false, const double quackinterval = 0.0, const std::string& quackmode = "beg", const bool quackincrement = false);

    bool printflagselection();

    bool clearflagselection(const int index = 0);

    ::casac::record* getautoflagparams(const std::string& algorithm = "none");

    bool setautoflag(const std::string& algorithm = "none", const ::casac::record& parameters = ::casac::initialize_record(""));

    bool setshadowflags(const std::string& field, const std::string& spw, const std::string& array, const std::string& feed, const std::string& scan, const std::string& baseline, const std::string& uvrange, const std::string& time = """", const std::string& correlation = """", const double diameter = -1.0);

    bool setflagsummary(const std::string& field, const std::string& spw, const std::string& array, const std::string& feed, const std::string& scan, const std::string& baseline, const std::string& uvrange, const std::string& time = """", const std::string& correlation = """");

    bool setqueryflag(const std::string& field, const std::string& spw, const std::string& array, const std::string& feed, const std::string& scan, const std::string& baseline, const std::string& uvrange, const std::string& time = """", const std::string& correlation = """", const std::string& what = "fieldid", const double fractionthreshold = 0.0, const int nflagsthreshold = 0, const bool morethan = true);

    bool setextendflag(const std::string& field, const std::string& spw, const std::string& array, const std::string& feed, const std::string& scan, const std::string& baseline, const std::string& uvrange, const std::string& time = """", const std::string& correlation = """", const std::string& along = "allbaselines", const int width = 0);

    ::casac::record* run(const bool trial = false, const bool reset = false);

    bool writeflagstodisk();

    bool help(const std::string& names = "");

    bool done();

    bool saveflagversion(const std::string& versionname = """", const std::string& comment = """", const std::string& merge = """");

    bool restoreflagversion(const std::vector<std::string>& versionname = std::vector<std::string> (1, """"), const std::string& merge = """");

    bool deleteflagversion(const std::vector<std::string>& versionname = std::vector<std::string> (1, """"));

    std::vector<std::string> getflagversionlist(const bool printflags = true);

    private:

	#include <xmlcasa/flagging/flagger_private.h>

};

} // casac namespace
#endif

