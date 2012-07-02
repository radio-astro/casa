
/***
 * Framework independent header file for tableiterator...
 *
 * Implement the tableiterator component here.
 * 
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 ***/

#ifndef _tableiterator_cmpt__h__
#define _tableiterator_cmpt__H__
#ifndef _tableiterator_cpnt__h__
#define _tableiterator_cpnt__H__

#include <vector>

#include <xmlcasa/record.h>
#include <xmlcasa/tables/tableindex_forward.h>

// put includes here

namespace casac {

/**
 * tableiterator component class 
 *
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 **/
class tableiterator
{
  private:

	

  public:

    tableiterator();
    virtual ~tableiterator();

    ::casac::record* table();

    bool reset();

    bool next();

    bool terminate();

    bool done();

    private:

#include <xmlcasa/tables/tableindex_private.h>

};

} // casac namespace
#endif
#endif

