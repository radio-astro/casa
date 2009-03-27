
/***
 * Framework independent header file for tablerow...
 *
 * Implement the tablerow component here.
 * 
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 ***/

#ifndef _tablerow_cmpt__h__
#define _tablerow_cmpt__H__
#ifndef _tablerow_cpnt__h__
#define _tablerow_cpnt__H__

#include <vector>

#include <xmlcasa/record.h>
#include <xmlcasa/tables/tablerow_forward.h>

// put includes here

namespace casac {

/**
 * tablerow component class 
 *
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 **/
class tablerow
{
  private:

	

  public:

    tablerow();
    virtual ~tablerow();

    bool set(const ::casac::record& tab, const std::vector<std::string>& columns = std::vector<std::string> (1,"all"), const bool exclude = false);

    ::casac::record* get(const int rownr);

    bool put(const int rownr, const ::casac::record& value, const bool matchingfields = true);

    bool close();

    bool done();

    private:

#include <xmlcasa/tables/tablerow_private.h>

};

} // casac namespace
#endif
#endif

