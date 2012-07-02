
/***
 * Framework independent header file for tableindex...
 *
 * Implement the tableindex component here.
 * 
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 ***/

#ifndef _tableindex_cmpt__h__
#define _tableindex_cmpt__H__
#ifndef _tableindex_cpnt__h__
#define _tableindex_cpnt__H__

#include <vector>

#include <xmlcasa/record.h>
#include <xmlcasa/tables/tableindex_forward.h>

// put includes here

namespace casac {

/**
 * tableindex component class 
 *
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 **/
class tableindex
{
  private:

	

  public:

    tableindex();
    virtual ~tableindex();

    bool set(const ::casac::record& tab, const std::vector<std::string>& columns, const bool sort = true);

    bool isunique();

    bool setchanged(const std::vector<std::string>& columns = std::vector<std::string> (1,"all"));

    int rownr(const ::casac::record& key);

    std::vector<int> rownrs(const ::casac::record& key, const ::casac::record& upperkey, const bool lowerincl = true, const bool upperincl = true);

    bool close();

    bool done();

    private:

#include <xmlcasa/tables/tableindex_private.h>

};

} // casac namespace
#endif
#endif

