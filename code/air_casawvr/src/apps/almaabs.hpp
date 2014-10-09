/**
   Bojan Nikolic <b.nikolic@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>
   Initial version November 2009
   Maintained by ESO since 2013.

   \file almaabs.hpp

   Retrievals for ALMA based on absolute WVR measurements only
   
*/
#ifndef _LIBAIR_ALMAABS_HPP__
#define _LIBAIR_ALMAABS_HPP__

#include <vector>
#include <list>
#include <set>
#include <iostream>

#include <boost/scoped_ptr.hpp>
#include <boost/array.hpp>
#include <boost/ptr_container/ptr_list.hpp>

#include "alma_datastruct.h"

namespace LibAIR2 {

  // Forward declarations
  class ALMAWVRCharacter;
  class iALMAAbsRet;  
  class ALMAResBase;
  class ALMAContRes;
  class dTdLCoeffsBase;
  class InterpArrayData;
  class dTdLCoeffsSingleInterpolated;
  class ALMARetOpts;

  /**
   */
  class ALMAAbsRet {
    boost::scoped_ptr<iALMAAbsRet> i;

  public:
    /**
       \param TObs The observed sky temperatures

       \param el Elevation of observation (radian)

       \param WVRChar Characterisation of the WVR used to make this
       measurement
     */
    ALMAAbsRet(const std::vector<double> &TObs,
	       double el,
	       const ALMAWVRCharacter &WVRChar);

    virtual ~ALMAAbsRet();

    /** \brief Get retrieved results
     */
    void g_Res(ALMAResBase &res);

  };

  std::ostream & operator<<(std::ostream &os, 
			    const  ALMAAbsInput &i);

  /** \brief Simple list of data inputs for retrieval
   */
  struct ALMAAbsInpL:
    public std::list<ALMAAbsInput> 
  {

  };

  std::ostream & operator<<(std::ostream &os, 
			    const  ALMAAbsInpL &i);

  /** Prepare input data for single retrieval from observation
      mid-point and for WVR on atenna 0 only
   */
  ALMAAbsInpL SimpleSingleI(const InterpArrayData &d);  

  /** 
      Retrieve for single antenna but multiple retrievals in time

      \param n Make n retrievals uniformly distributed in time across
      the data set
      
      \param states Consider only data with one of these state IDs
   */
  ALMAAbsInpL MultipleUniformI(const InterpArrayData &d,
			       size_t n,
			       const std::set<size_t> &states);  

  /** Prepare input for mid-point of each new field (sequantially)

      \param time The stamps of each field ID 

      \param fb The segments correspodnging to field boundaries
      
      \param states Consider only data with one of these state IDs
   */
  ALMAAbsInpL FieldMidPointI(const InterpArrayData &d,
			     const std::vector<double> &time,
			     const std::vector<std::pair<double, double> >  &fb,
			     const std::set<size_t> &states);

  

  /**  Carry out the retrieval of coefficients form a list of inputs
   */
  boost::ptr_list<ALMAResBase> doALMAAbsRet(const ALMAAbsInpL &il, std::vector<int> &problemAnts);
  

  /** \brief Calculate coefficients for phase correction from inputs
   */ 
  dTdLCoeffsBase *
  ALMAAbsProcessor(const ALMAAbsInpL &inp,
		   boost::ptr_list<ALMAResBase> &r);

  /** Single retrieval for mid point of the observation
   */
  dTdLCoeffsBase * 
  SimpleSingle(const InterpArrayData &d);  


  /** Single retrieval for mid point of the observation
   */
  dTdLCoeffsBase * 
  SimpleSingle(const InterpArrayData &d);

  /** Single retrieval at mid-point, but fitting also for the
      continuum
   */
  dTdLCoeffsBase * 
  SimpleSingleCont(const InterpArrayData &d);


  /** 
      Separate retrieval for each new field
  */
  dTdLCoeffsSingleInterpolated *
  SimpleMultiple(const InterpArrayData &d,
		 const std::vector<double> &time,
		 const std::vector<std::pair<double, double> > &fb,
		 boost::ptr_list<ALMAResBase> &r);

  
  /** Slightly experimental approach for a single-shot simple
      retrieveval of water vapour
   */
  void ALMAAbsContRetrieve(const std::vector<double> &TObs,
			   double el,
			   const ALMAWVRCharacter &WVRChar,
			   ALMAContRes &res,
			   const ALMARetOpts &opts);

  



}

#endif
