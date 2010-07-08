
/***
 * Framework independent header file for table...
 *
 * Implement the table component here.
 * 
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 ***/

#ifndef _table_cmpt__H__
#define _table_cmpt__H__

#include <vector>
#include <xmlcasa/casac.h>
#include <xmlcasa/conversions.h>
#include <xmlcasa/record.h>
#include <xmlcasa/variant.h>
#include <xmlcasa/tables/table_forward.h>
// put includes here

namespace casac {

/**
 * table component class 
 *
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 **/
class table
{
  private:

	

  public:

    table();
    virtual ~table();

    ::casac::table * fromfits(const std::string& tablename, const std::string& fitsfile, const int whichhdu = 1, const std::string& storage = "standard", const std::string& convention = "none", const bool nomodify = true, const bool ack = true);

    bool fromascii(const std::string& tablename, const std::string& asciifile, const std::string& headerfile = "", const bool autoheader = false, const std::vector<int>& autoshape = std::vector<int> (1, -1), const std::string& sep = " ", const std::string& commentmarker = "", const int firstline = 0, const int lastline = -1, const bool nomodify = true, const std::vector<std::string>& columnnames = std::vector<std::string> (1, ""), const std::vector<std::string>& datatypes = std::vector<std::string> (1, ""));

    bool fromASDM(const std::string& tablename, const std::string& xmlfile);

    bool create(const std::string& tablename,
                const ::casac::record& tabledesc,
                const ::casac::record& lockoptions = ::casac::initialize_record("default"),
                const std::string& endianformat = "", // "" == "aipsrc"
                const std::string& memtype = "",      // "" -> Table::Plain
                int nrow = 0,
                const ::casac::record& dminfo = ::casac::initialize_record(""));

    bool open(const std::string& tablename = "", const ::casac::record& lockoptions = ::casac::initialize_record("default"), const bool nomodify = true);

    bool flush();

    bool resync();

    bool close();

    ::casac::table * copy(const std::string& newtablename, const bool deep = false, const bool valuecopy = false, const ::casac::record& dminfo = ::casac::initialize_record(""), const std::string& endian = "aipsrc", const bool memorytable = false, const bool returnobject = false);

    bool copyrows(const std::string& outtable, const int startrowin = 0, const int startrowout = -1, const int nrow = -1);

    bool done();

    bool iswritable();

    std::string endianformat();

    bool lock(const bool write = true, const int nattempts = 0);

    bool unlock();

    bool datachanged();

    bool haslock(const bool write = true);

    ::casac::record* lockoptions();

    bool ismultiused(const bool checksubtables = false);

    bool browse();

    std::string name();

    bool toasciifmt(const std::string& asciifile, const std::string& headerfile = "", const std::vector<std::string>& columns = std::vector<std::string> (1, ""), const std::string& sep = "");

    ::casac::table * query(const std::string& query = "String", const std::string& name = "", const std::string& sortlist = "", const std::string& columns = "");

    ::casac::table * queryC(const std::string& query = "", const std::string& resultTable = "", const std::string& sortlist = "", const std::string& columns = "");

    ::casac::variant* calc(const std::string& expr);

    ::casac::table * selectrows(const std::vector<int>& rownrs, const std::string& name = "");

    bool putinfo(const ::casac::record& value);

    bool addreadmeline(const std::string& value);

    bool summary(const bool recurse = false);

    std::vector<std::string> colnames();

    std::vector<int> rownumbers(const ::casac::record& tab, const int nbytes);

    bool setmaxcachesize(const std::string& columnname, const int nbytes);

    bool isscalarcol(const std::string& columnname);

    bool isvarcol(const std::string& columnname);

    std::string coldatatype(const std::string& columnname);

    std::string colarraytype(const std::string& columnname);

    int ncols();

    int nrows();

    bool addrows(const int nrow = 1);

    bool removerows(const std::vector<int>& rownrs);

    bool addcols(const ::casac::record& desc, const ::casac::record& dminfo = ::casac::initialize_record(""));

    bool renamecol(const std::string& oldname, const std::string& newname);

    bool removecols(const std::vector<std::string>& columnames);

    bool iscelldefined(const std::string& columnname, const int rownr = 0);

    ::casac::variant* getcell(const std::string& columnname, const int rownr = 0);

    ::casac::variant* getcellslice(const std::string& columnname, const int rownr, const std::vector<int>& blc, const std::vector<int>& trc, const std::vector<int>& incr = std::vector<int> (1, 1));

    ::casac::variant* getcol(const std::string& columnname, const int startrow = 0, const int nrow = -1, const int rowincr = 1);

    ::casac::record* getvarcol(const std::string& columnname, const int startrow = 0, const int nrow = -1, const int rowincr = 1);

    ::casac::variant* getcolslice(const std::string& columnname, const std::vector<int>& blc, const std::vector<int>& trc, const std::vector<int>& incr, const int startrow = 0, const int nrow = -1, const int rowincr = 1);

    bool putcell(const std::string& columnname, const std::vector<int>& rownr, const ::casac::variant& thevalue);

    bool putcellslice(const std::string& columnname, const int rownr, const ::casac::variant& value, const std::vector<int>& blc, const std::vector<int>& trc, const std::vector<int>& incr = std::vector<int> (1, 1));

    bool putcol(const std::string& columnname, const ::casac::variant& value, const int startrow = 0, const int nrow = -1, const int rowincr = 1);

    bool putvarcol(const std::string& columnname, const ::casac::record& value, const int startrow = 0, const int nrow = -1, const int rowincr = 1);

    bool putcolslice(const std::string& columnname, const ::casac::variant& value, const std::vector<int>& blc, const std::vector<int>& trc, const std::vector<int>& incr = std::vector<int> (1, 1), const int startrow = 0, const int nrow = -1, const int rowincr = 1);

    std::vector<std::string> getcolshapestring(const std::string& columnname, const int startrow = 0, const int nrow = -1, const int rowincr = 1);

    ::casac::variant* getkeyword(const ::casac::variant& keyword);

    ::casac::record* getkeywords();

    ::casac::variant* getcolkeyword(const std::string& columnname, const ::casac::variant& keyword);

    ::casac::variant* getcolkeywords(const std::string& columnname);

    bool putkeyword(const ::casac::variant& keyword, const ::casac::variant& value, const bool makesubrecord = false);

    bool putkeywords(const ::casac::record& value);

    bool putcolkeyword(const std::string& columnname, const ::casac::variant& keyword, const ::casac::variant& value);

    bool putcolkeywords(const std::string& columnname, const ::casac::record& value);

    bool removekeyword(const ::casac::variant& keyword);

    bool removecolkeyword(const std::string& columnname, const ::casac::variant& keyword);

    ::casac::record* getdminfo();

    std::vector<std::string> keywordnames();

    std::vector<std::string> fieldnames(const std::string& keyword = "");

    std::vector<std::string> colkeywordnames(const std::string& columnname);

    std::vector<std::string> colfieldnames(const std::string& columnname, const std::string& keyword = "");

    ::casac::record* getdesc(const bool actual = true);

    ::casac::record* getcoldesc(const std::string& columnname);

    std::vector<std::string> showcache(const bool verbose=true);

    bool ok();

    bool clearlocks();

    bool listlocks();

    ::casac::record* statistics(const std::string& column, const std::string& complex_value, const bool useflags = true);

    private:

	#include <xmlcasa/tables/table_private.h>

};

} // casac namespace
#endif

