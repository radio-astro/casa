
/***
 * Framework independent header file for autoflag...
 *
 * Implement the autoflag component here.
 * 
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 ***/

#ifndef _autoflag_cmpt__H__
#define _autoflag_cmpt__H__
#ifndef _autoflag_cnpt__H__
#define _autoflag_cnpt__H__

#include <vector>
#include <xmlcasa/casac.h>
#include <xmlcasa/Quantity.h>
#include <xmlcasa/variant.h>
#include <xmlcasa/record.h>
#include <xmlcasa/flagging/autoflag_forward.h>

// put includes here

namespace casac {

/**
 * autoflag component class 
 *
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 **/
class autoflag
{
  private:

	

  public:

    autoflag();
    virtual ~autoflag();

    bool settimemed(const double thr = 5.0, const int hw = 10, const double rowthr = 10.0, const int rowhw = 10, const bool norow = false, const std::string& column = "DATA", const std::string& expr = "ABS I", const bool fignore = false);

    bool setnewtimemed(const double thr = 3.0, const std::string& column = "DATA", const std::string& expr = "ABS I", const bool fignore = false);

    bool setdata(const std::string& mode = "none", const std::vector<int>& nchan = std::vector<int> (1,1), const std::vector<int>& start = std::vector<int> (1,0), const std::vector<int>& step = std::vector<int> (1,1), const Quantity& mstart = Quantity(std::vector<double> (1,0.0),"m/s"), const Quantity& mstep = Quantity(std::vector<double> (1,0.0),"m/s"), const std::vector<int>& spwid = std::vector<int> (1,-1), const std::vector<int>& fieldid = std::vector<int> (1,-1), const std::string& msselect = "", const bool async = false);

    bool setfreqmed(const double thr = 5.0, const int hw = 10, const double rowthr = 10.0, const int rowhw = 10, const std::string& column = "DATA", const std::string& expr = "ABS I`", const bool fignore = false);

    bool setsprej(const int ndeg, const double rowthr, const int rowhw, const bool norow, const int spwid, const std::vector<int>& chan, const ::casac::record& region, const std::string& column = "DATA", const std::string& expr = "ABS I", const bool fignore = false);

    bool setuvbin(const double thr = 0.0, const int minpop = 0, const std::vector<int>& nbins = std::vector<int> (1,50), const int plotchan = -1, const bool econoplot = false, const std::string& column = "DATA", const std::string& expr = "ABS I", const bool fignore = false);

    bool setselect(const std::vector<int>& spwid, const std::vector<std::string>& field, const Quantity& fq, const std::vector<int>& chan, const std::vector<std::string>& corr, const std::vector<std::string>& ant, const std::vector<int>& baseline, const std::vector<std::string>& timerng, const bool autocorr, const std::vector<std::string>& timeslot, const double dtime, const ::casac::record& clip, const ::casac::record& flagrange, const std::vector<int>& quack = std::vector<int> (1,0), const bool unflag = false);

    bool open(const std::string& msname = "");

    bool run(const ::casac::record& globparm, const int plotscr = 0, const int plotdev = 0, const std::string& devfile = "flagreport.ps/ps", const bool reset = false, const bool trial = false);

    bool summary();

    bool help(const std::string& names = "");

    bool reset(const std::vector<std::string>& methods = std::vector<std::string> (1,""));

    bool resetall();

    bool setspwid(const std::vector<int>& spwid = std::vector<int> (1,-1));

    bool setfield(const ::casac::variant& field);

    bool setfreqs(const Quantity& freqs = Quantity(std::vector<double> (1,-1.0),"MHz"));

    bool setchans(const std::vector<int>& chan = std::vector<int> (1,-1));

    bool setcorr(const std::vector<std::string>& corr = std::vector<std::string> (1,"/"));

    bool setantenna(const std::vector<std::string>& ant = std::vector<std::string> (1,"/"));

    bool setantennaID(const std::vector<int>& antID = std::vector<int> (1,-1));

    bool setbaseline(const std::vector<int>& baseline = std::vector<int> (1,-1));

    bool settimerange(const std::vector<std::string>& timerange = std::vector<std::string> (1,"/"));

    bool setautocorrelation(const bool autocorr = false);

    bool settimeslot(const std::vector<std::string>& timeslot = std::vector<std::string> (1,""));

    bool setdtime(const double dtime = 10);

    bool setclip(const ::casac::record& clip);

    bool setflagrange(const ::casac::record& flagrange);

    bool setquack(const std::vector<double>& quack = ::casac::initialize_vector( 2, (double) 0.0, (double) 0.0 ));

    bool setunflag(const bool unflag = false);

    bool setnewselector();

    bool close();

    bool done();

    private:

	#include <xmlcasa/flagging/autoflag_private.h>

};

} // casac namespace
#endif
#endif

