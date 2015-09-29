/***
 * Framework independent implementation file for atcafiller...
 *
 * Implement the atcafiller component here.
 * 
 * // The filler for ATCA data in the RPFITS format
 * // Resurrected from the pre-CABB, pre-svn version.
 * // Updated for CABB, Jan 2015.
 *
 * @author
 * @version 
 ***/

//#include <iostream>

#include <atnf/atca/ATCAFiller.h>
#include <atcafiller_cmpt.h>
#include <stdcasa/StdCasa/CasacSupport.h>

using namespace std;

namespace casac {

atcafiller::atcafiller()
{
  itsATCAFiller = new casa::ATCAFiller();
}


atcafiller::~atcafiller() 
{
  if (itsATCAFiller) {
    delete itsATCAFiller;
    itsATCAFiller=0;
  }
}

bool atcafiller::open(const std::string& msName,
		 const std::vector<std::string>& rpfitsFiles,
		 const std::vector<std::string>& options)
{
    itsATCAFiller->open(msName,rpfitsFiles,options,0);
    return True;
}

bool atcafiller::fill()
{
    return itsATCAFiller->fill();
}

bool atcafiller::select(int firstScan, int lastScan, 
       const std::vector<int>& spw, 
		   const Quantity& lowFreq, const Quantity& highFreq,
		   const std::vector<std::string>& fieldList,float edge)
{
    itsATCAFiller->scanRange(firstScan,lastScan);
    itsATCAFiller->freqSel(spw);
    itsATCAFiller->freqRange(casaQuantity(lowFreq).getValue("Hz"),
			    casaQuantity(highFreq).getValue("Hz"));
    itsATCAFiller->fields(fieldList);
    itsATCAFiller->edge(edge);
    return True;
}

} // casac namespace

