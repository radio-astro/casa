/**
   Bojan Nikolic <b.nikolic@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>
   Initial version January 2010.
   Maintained by ESO since 2013.

   This file is part of LibAIR and is licensed under GNU Public
   License Version 2

   \file arraydata.hpp

   Structure to hold data from all WVRs  in an array
*/
#ifndef _LIBAIR_APPS_ARRAYDATA_HPP__
#define _LIBAIR_APPS_ARRAYDATA_HPP__

#include <vector>
#include <boost/multi_array.hpp>

#include "antennautils.hpp"

namespace LibAIR2 {


  /** \brief Observation from an array of WVRs which has been
      interpolated or sampled onto a common time base

      \note This number of time points and WVRs must be specified at
      construction of this object, it can not grow.

   */
  class InterpArrayData {

  public:
    
    /// This is the type which is used to store the actual sky
    /// brightness data from all the WVRs
    typedef boost::multi_array<double, 3> wvrdata_t;
    
  private:

    /// The time stamps of each data row
    std::vector<double> time;
    /// Elevation of each observing point
    std::vector<double> el;
    /// Azimuth of each observation
    std::vector<double> az;
    /// State ID ([sub-]scan intent) of each observation
    std::vector<size_t> state;
    /// Field ID of each observation
    std::vector<size_t> field;
    /// Source ID of each observation
    std::vector<size_t> source;
    
    /// The data itself
    wvrdata_t wvrdata;

  public:

    // ----------------------  Public data ------------------

    /// Number of WVRs
    const size_t nWVRs;


    // ----------------------- Construction/Destruction -------------

    /**

       \param nWVRs number of WVRs for which the data are recorded

     */
    InterpArrayData(const std::vector<double> &time, 
		    const std::vector<double> &el, 
		    const std::vector<double> &az, 
		    const std::vector<size_t> &state, 
		    const std::vector<size_t> &field, 
		    const std::vector<size_t> &source, 
		    size_t nWVRs);

    // ----------------------- Public Interface -------------

    const std::vector<double> &g_time(void) const
    {
      return time;
    }

    const std::vector<double> &g_el(void) const
    {
      return el;
    }

    const std::vector<double> &g_az(void) const
    {
      return az;
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

    const wvrdata_t &g_wvrdata(void) const
    {
      return wvrdata;
    }

    /** Set one element of the WVR data array
     */
    void set(size_t time,
	     size_t wvr,
	     size_t ch,
	     double Tsky)
    {
      wvrdata[time][wvr][ch]=Tsky;
    }

    /** Number of time points in the data
     */
    size_t nTimes(void) const
    {
      return time.size();
    }

    /**  Adjust time stamps for a timing offset relative to the
	 interfermetric data .
     */
    void offsetTime(double dt);
    

  };

  /** Interpolate data for a bad antenna a by the mean of antennas aset
   */
  void interpBadAnt(InterpArrayData &d,
		    size_t a,
		    const AntSet &aset);

  /** Interpolate data for a bad antenna a weighted average of nearby
      antennas
   */
  void interpBadAntW(InterpArrayData &d,
		     size_t a,
		     const AntSetWeight &aset);

  /** \brief Create new array data with only states in it
   */
  InterpArrayData *filterState(InterpArrayData &d,
			       const std::set<size_t>& states);

  /** \brief Smooth the WVR data in time. The smoothing is symmetric
      in time. The initial and last nsample/2 are not smoothed 

      The smoothing is broken at every change of source and state id
      to avoid smoothing across large change of airmass or when
      observing calibration loads.
   */
  void smoothWVR(InterpArrayData &d,
		 size_t nsample);
			       

}

#endif
