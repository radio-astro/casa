
/***
 * Framework independent implementation file for tablerow...
 *
 * Implement the tablerow component here.
 * 
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 ***/

#include <iostream>
#include <tablerow_cmpt.h>
#include <casa/aips.h>

using namespace std;
using namespace casa;

namespace casac {

tablerow::tablerow()
{

}

tablerow::~tablerow()
{

}

bool
tablerow::set(const ::casac::record& tab, const std::vector<std::string>& columns, const bool exclude)
{

    // TODO : IMPLEMENT ME HERE !
  return False;
}

::casac::record*
tablerow::get(const int rownr)
{

    // TODO : IMPLEMENT ME HERE !
   return 0;
}

bool
tablerow::put(const int rownr, const ::casac::record& value, const bool matchingfields)
{

    // TODO : IMPLEMENT ME HERE !
  return False;
}

bool
tablerow::close()
{

    // TODO : IMPLEMENT ME HERE !
  return False;
}

bool
tablerow::done()
{

    // TODO : IMPLEMENT ME HERE !
  return False;
}

} // casac namespace

