/**
   Bojan Nikolic <b.nikolic@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>
   Initial version January 2010.
   Maintained by ESO since 2013.

   This file is part of LibAIR and is licensed under GNU Public
   License Version 2

   \file arraygains.hpp

   Structure to hold gains derived from WVR data
*/
#ifndef _LIBAIR_APPS_ARRAYGAINS_HPP__
#define _LIBAIR_APPS_ARRAYGAINS_HPP__

#include <vector>
#include <set>

#include <boost/multi_array.hpp>

namespace LibAIR2 {

  // Forward 
  class InterpArrayData;
  class dTdLCoeffsBase;

  /** Gains calculated for an array (taking into account flags)
   */
  class ArrayGains
  {

  public:
    
    /// At the moment anticipate only storing the non-dispersive path
    /// for each antenna. The two dimensions are time and antenna
    /// number
    typedef boost::multi_array<double, 2> path_t;

  private:

    /// The time stamps of each data row
    std::vector<double> time;
    /// Elevation of each data row
    std::vector<double> el;
    std::vector<size_t> state, field, source;
    /// The gains
    path_t path;

  public:

    // ----------------------- Public data              -------------

    /// Number of antennas in the array
    const size_t nAnt;

    // ----------------------- Construction/Destruction -------------

    /** 
	\param time Time points at which the gains have been
	calculated
	
	\param nAnt number of antennas in the array
     */
    ArrayGains(const std::vector<double> &time, 
	       const std::vector<double> &el, 
	       const std::vector<size_t> &state, 
	       const std::vector<size_t> &field, 
	       const std::vector<size_t> &source, 
	       size_t nAnt);

    // ----------------------- Public interface ---------------------

    /** Calculate the gains. 

	This assumes the same coeffients are applicable to all time
	points and only calculates the non-dispersive path for each
	antenna
     */
    void calc(const InterpArrayData &wvrdata,
	      const std::vector<double> &coeffs);

    /** \brief Calculate the gains using the second order approximation

	\param coeffs The first order differentials
	\param c2 The second order differntials 
	\param TRef The reference sky brightness temperatures
    */
    void calc(const InterpArrayData &wvrdata,
	      const std::vector<double> &coeffs,
	      const std::vector<double> &c2,
	      const std::vector<double> &TRef);

    /** Calculate the gains. 

     */
    void calc(const InterpArrayData &wvrdata,
	      const std::vector<double> &coeffs,
	      const std::vector<double> &weights
	      );

    /** Calculate the gains

	Uses the time and antenna variable coefficients from the coeff
	object.
    */
    void calc(const InterpArrayData &wvrdata,
	      const dTdLCoeffsBase &coeffs);

    /** \brief Calculate the gains by linearly interpolating another
	array
     */
    void calcLinI(const ArrayGains &o);

    /** \brief Scale the all paths by a give factor
     */
    void scale(double s);


    const std::vector<double> &g_time(void) const
    {
      return time;
    }

    const std::vector<size_t> &g_state(void) const
    {
      return state;
    }

    const std::vector<size_t> &g_field(void) const
    {
      return field;
    }

    const std::vector<size_t> &g_source(void) const
    {
      return source;
    }
    
    const path_t &g_path(void) const
    {
      return path;
    }

    /** \brief Return the difference in path between antennas i and j
	at time *index* timei
	
	\returns The path difference in m
     */
    const double deltaPath(size_t timei,
			   size_t i,
			   size_t j) const;

    /** \brief Convenience accessor for getting absolute path on
	antenna i at time index timei
    */
    const double absPath(size_t timei,
			 size_t i) const;

    /** \brief The greatest path RMS on a baseline
     */
    double greatestRMSBl(const std::vector<std::pair<double, double> > &tmask) const;

    /** \brief Path RMS for each antenna*/
    void pathRMSAnt(const std::vector<std::pair<double, double> > &tmask,
		    std::vector<double> &res) const;

    void pathRMSAnt(std::vector<double> &res) const;

    /** \brief Compute discrepancy in path for two set of paths
     */
    void pathDiscAnt(const ArrayGains &other,
		     std::vector<double> &res) const;

    /** \brief Compute discrepancy in path for two set of paths
     */
    void pathDiscAnt(const ArrayGains &other,
		     const std::vector<std::pair<double, double> > &tmask,
		     std::vector<double> &res) const;

    /** Set path for these sources to zero
     */
    void blankSources(std::set<size_t> &flagsrc);

  };

  /** Invert the coefficients and reweight so that they can applied
      directly to flluctuations
      
      Any coefficient with value zero is ignored and not used in the
      correction.

      This version reweights the four channels to equal contribution,
      without considering the thermal noise performance at all.
  */
  void reweight(const std::vector<double> &coeffs,
		std::vector<double> &res);


  /** Invert and reweight coefficeints
      
      \see reweight
      
      \param coeffs The first order coefficients, dT/dL
      
      \param c2     The second order coefficients d2T/dL2
      
      \param res The output reweighted first order coefficients

      \param res2 The output reweighted second oorder coefficients
      
      
      This version weighs down the channels based on their thermal
      noise performance. Note that the output is in units dL/dT, i.e.,
      the oposite of the input.
   */
  void reweight_thermal(const std::vector<double> &coeffs,
			std::vector<double> &res);
  void reweight_thermal(const std::vector<double> &coeffs,
			const std::vector<double> &c2,
			std::vector<double> &res,
			std::vector<double> &res2);
  
  /** \brief Expected per-antenna error due to thermal effects
   */
  double thermal_error(const std::vector<double> &coeffs);


}

#endif
