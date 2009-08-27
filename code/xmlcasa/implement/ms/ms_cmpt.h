/***
 * Framework independent header file for ms...
 *
 * Implement the ms component here.
 * 
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 ***/

#ifndef _ms_cmpt__H__
#define _ms_cmpt__H__

#include <vector>
#include <xmlcasa/casac.h>
#include <xmlcasa/conversions.h>
#include <xmlcasa/record.h>
#include <xmlcasa/variant.h>
#include <xmlcasa/ms/ms_forward.h>

// put includes here

namespace casac {

/**
 * ms component class 
 *
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 **/
class ms
{
  private:

	

  public:

    ms();
    virtual ~ms();

    bool fromfits(const std::string& msfile = "", const std::string& fitsfile = "", const bool nomodify = true, const bool lock = false, const int obstype = 0, const std::string& host = "", const bool forcenewserver = false, const std::string& antnamescheme = "old");

    int nrow(const bool selected = false);

    bool iswritable();

    bool open(const std::string& thems = "", const bool nomodify = true, const bool lock = false);

    bool close();

    std::string name();

    bool tofits(const std::string& fitsfile = "", const std::string& column = "corrected", const ::casac::variant& field = ::casac::initialize_variant(""), const ::casac::variant& spw = ::casac::initialize_variant(""), const int nchan = -1, const int start = 0, const int width = 1, const ::casac::variant& baseline = ::casac::initialize_variant(""), const std::string& time = "", const ::casac::variant& scan = ::casac::initialize_variant(""), const ::casac::variant& uvrange = ::casac::initialize_variant(""), const std::string& taql = "", const bool writesyscal = false, const bool multisource = false, const bool combinespw = false, const bool writestation = false);

    bool summary(::casac::record& header, const bool verbose = false);

    bool listhistory();

    bool writehistory(const std::string& message = "", const std::string& parms = "", const std::string& origin = "MSHistoryHandler::addMessage()", const std::string& msname = "", const std::string& app = "ms");

    ::casac::record* range(const std::vector<std::string>& items, const bool useflags = true, const int blocksize = 10);

    bool lister(const std::string& options = "", const std::string& datacolumn = "data", const std::string& field = "", const std::string& spw = "", const std::string& antenna = "", const std::string& timerange = "", const std::string& correlation = "", const std::string& scan = "", const std::string& feed = "", const std::string& array = "", const std::string& uvrange = "", const std::string& average = "", const bool showflags = false, const std::string& msselect = "", const int pagerows = 50, const std::string& listfile = "");

    bool selectinit(const int datadescid = 0, const bool reset = false);

    bool select(const ::casac::record& items);

    bool selecttaql(const std::string& msselect = "");

    bool selectchannel(const int nchan, const int start, const int width, const int inc);

    bool selectpolarization(const std::vector<std::string>& wantedpol);

    bool cvel(const std::string& outframe = "", const std::string& regrid_quantity = "chan", const double regrid_velo_restfrq = -9E99 , const std::string& regrid_interp_meth = "LINEAR", const double regrid_start = -9E99, const double regrid_center = -9E99, const double regrid_bandwidth = -1., const double regrid_chan_width = -1.);

    ::casac::record* getdata(const std::vector<std::string>& items, const bool ifraxis = false, const int ifraxisgap = 0, const int increment = 1, const bool average = false);

    bool putdata(const ::casac::record& items);

    bool concatenate(const std::string& msfile = "", const ::casac::variant& freqtol = ::casac::initialize_variant("1Hz"), const ::casac::variant& dirtol = ::casac::initialize_variant("1mas"));

    bool timesort(const std::string& newmsname = "");

    bool split(const std::string&      outputms = "",
               const ::casac::variant& field = ::casac::initialize_variant(""),
               const ::casac::variant& spw = ::casac::initialize_variant(""),
               const std::vector<int>& nchan = std::vector<int> (1, -1),
               const std::vector<int>& start = std::vector<int> (1, 0),
               const std::vector<int>& step = std::vector<int> (1, 1),
               const ::casac::variant& baseline = ::casac::initialize_variant(""),
               const ::casac::variant& timebin = ::casac::initialize_variant("-1s"),
               const std::string&      time = "",
               const ::casac::variant& scan = ::casac::initialize_variant(""),
               const ::casac::variant& uvrange = ::casac::initialize_variant(""),
               const std::string&      taql = "",
               const std::string&      whichcol = "DATA",
               const ::casac::variant& tileshape = ::casac::initialize_variant(""),
               const ::casac::variant& subarray = ::casac::initialize_variant(""));

    bool iterinit(const std::vector<std::string>& columns, const double interval, const int maxrows, const bool adddefaultsortcolumns = true);

    bool iterorigin();

    bool iternext();

    bool iterend();

    bool fillbuffer(const std::string& item, const bool ifraxis = false);

    ::casac::record* diffbuffer(const std::string& direction, const int window);

    ::casac::record* getbuffer();

    bool clipbuffer(const double pixellevel, const double timelevel, const double channellevel);

    bool setbufferflags(const ::casac::record& flags);

    bool writebufferflags();

    bool clearbuffer();

    bool continuumsub(const ::casac::variant& field = ::casac::initialize_variant(""), const ::casac::variant& fitspw = ::casac::initialize_variant(""), const ::casac::variant& spw = ::casac::initialize_variant(""), const ::casac::variant& solint = ::casac::initialize_variant("int"), const int fitorder = 0, const std::string& mode = "subtract");

    bool done();

    ::casac::record* msseltoindex(const std::string& vis = "", const ::casac::variant& spw = ::casac::initialize_variant(""), const ::casac::variant& field = ::casac::initialize_variant(""), const ::casac::variant& baseline = ::casac::initialize_variant(""), const ::casac::variant& time = ::casac::initialize_variant(""), const ::casac::variant& scan = ::casac::initialize_variant(""), const ::casac::variant& uvrange = ::casac::initialize_variant(""), const std::string& taql = "");

    bool hanningsmooth();

    bool uvsub(const bool reverse = false);

    private:

#include <xmlcasa/ms/ms_private.h>

};

} // casac namespace
#endif

