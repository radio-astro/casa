/**
   Bojan Nikolic <bn204@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>
   Initial version February 2008
   Maintained by ESO since 2013.

   This file is part of LibAIR and is licensed under GNU Public
   License Version 2

   \file radiometermeasure.hpp

   Model the radiometer measurement process
*/

#ifndef _LIBAIR_RADIOMETERMEASURE_HPP__
#define _LIBAIR_RADIOMETERMEASURE_HPP__

#include <vector>
#include <stdexcept>

#include <boost/smart_ptr.hpp>

namespace LibAIR2 {

  /**
     General exception class for errors in the radiometer subsystem.
   */
  class RadiometerError : 
    public std::runtime_error
  {
  public:
    
    RadiometerError(const std::string & s):
      std::runtime_error(s)
    {}
    
  };



  /** \brief Base class for radiometers 
   */
  class Radiometer {

    /** The frequency grid over which measurements are expected.
     */
    std::vector<double> FGrid;

    /** \brief Coefficients used to compute the output of the
	radiomter from sky brigthnesses

	Multiple vectors of coefficients allow multiple channels
	
     */
    std::vector< std::vector<double> > coeffs_v;

  public:

    // ---------- Construction / Destruction --------------

    /** \brief Constructs a single channel radiometer
     */
    Radiometer( const std::vector<double> & FGrid,
		const std::vector<double> & coeffs);

    /** \brief Construct a multi channel radiometer
     */
    Radiometer( const std::vector<double> & FGrid,
		const std::vector< std::vector<double> >  & coeffs_v);

    // ---------- Public interface  --------------------------

    /**
       \brief
       Return the frequency grid required by this radiometer
     */
    const std::vector<double> & getFGrid(void)  const
    {
      return FGrid;
    }

    const std::vector<double> & getCoeffs(size_t ch)  const
    {
      return coeffs_v[ch];
    }

    /** \brief Returns the number of channels

     */
    size_t nchannels(void) const 
    {
      return coeffs_v.size();
    }

    

    /** \brief Return the meausrement from a vector of sky Tbs
	evaluated on grid as returned by getFGrid.

	\param ch is zero-indexed
     */
    double eval( const std::vector<double> & skyTb,
		 size_t ch) const
    {
      const std::vector<double> & coeffs = coeffs_v[ch];

      double res = 0;
      for (size_t i = 0 ; i < FGrid.size() ; ++i )
	res += coeffs[i] * skyTb[i];
      return res;
      
    }

    double eval( const std::vector<double> & skyTb) const
    {
      if ( nchannels()  > 1 )
      {
	throw RadiometerError("More than one channel but asking for single result");
      }

      return eval( skyTb, 0);
    }

  };

  /**
     \brief Model of a simple double-sideband radiometer.

     At the moment this is single-channel only

   */
  class DSBRadio
  {

    boost::scoped_ptr<Radiometer> r;

  public:

    // ---------- Public data  ------------ --------------

    /// The central frequency
    const double f_0;

    /// The frequency of the middle of the IF band
    const double f_if;

    // ---------- Construction / Destruction --------------

    /** The basic constructor

     */
    DSBRadio( double f_0,
	      double f_if);

    /** Use the supplied radiometer -- for derived classes
     */
    DSBRadio( double f_0,
	      double f_if,
	      Radiometer * rr);

    // ---------- Public interface  --------------------------

    /** \brief Return a const reference to the actual radiometer
     */
    const Radiometer & getRadiometer(void) const;

    double eval( const std::vector<double> & skyTb) const;

    const std::vector<double> & getFGrid(void)  const
    {
      return r->getFGrid();
    }

  };


  /** \brief Incoroporate the effect of bandiwdth of radiometer filters.

      This version calculates the average of the signal over the band
      using rectangular integration of the signal over 30 points --
      not very efficient.

   */
  class DSBBWRadio:
    public DSBRadio
  {

  public:

    /// The fitlter bandwidth
    const double f_bw;

    // ---------- Construction / Destruction --------------

    DSBBWRadio( double f_0,
		double f_if,
		double f_bw);

    // ---------- Public interface  --------------------------

    static Radiometer * MkRadio( double f_0,
				 double f_if,
				 double f_bw);
  };

  /**
     Generate finite bandwidth radiometers with Gaussian quadrature
     integration.
   */
  class DSBBW_QuadRadio:
    public DSBRadio
  {

  public:

    // ---------- Construction / Destruction --------------

    DSBBW_QuadRadio( double f_0,
		     double f_if,
		     double f_bw);

    // ---------- Public interface  --------------------------

    static Radiometer * MkRadio( double f_0,
				 double f_if,
				 double f_bw);
  };

  

  /** \brief Exception type representing for errors due to unknown
      radiometer channel.

   */
  struct InvalidWVRChannel 
  {
    int chlow, chhigh, ch;

    InvalidWVRChannel(int chlow,
		      int chhigh,
		      int ch);
  };

  /** \brief Returns WVR perfectly to spec for ALMA

      \param ch the WVR channel (1 to 4 inclusive).

   */
  DSBRadio * MkALMARadiometer(int ch,
			      double cf_off=0.0,
			      double bw_off=0.0) throw (InvalidWVRChannel);

  /** \brief Returns WVR Dicke prototype 

      \param ch the WVR channel (1 to 4 inclusive).

   */
  DSBRadio * MkALMADickeProto(int ch) throw (InvalidWVRChannel);


  /**
     \brief Create the four channel production ALMA WVR
     
   */
  Radiometer * MkFullALMAWVR(void);

  /**
     \brief Create the ALMA radiometer but offset the centre
     frequencies by foffset (in GHz)
  */
  Radiometer * MkALMAWVR_offset(double cf,
				double bw);

  /**
     \brief Measured properties of ALMA Production radiometers
   */
  struct ALMAWVRCharacter {
    /// Measured centre frequencies of each of the filters
    double cf1, cf2, cf3, cf4;
    /// Measured bandwidths of each fo the filters
    double bw1, bw2, bw3, bw4;

    /// By default set to the nominal character
    ALMAWVRCharacter(void);
  };
  
  /** \brief Create ALMA radiometer with measured characteristics
   */
  Radiometer *MkALMAWVR(const ALMAWVRCharacter &c);

  /**
     \brief Create the four channel dicke Prototype WVR
     
   */
  Radiometer * MkFullDickeProtoWVR(void);

  /**
     \brief Create a single sideband radiometer

     The filter is approximated by five-point Gaussian quadrature
     
     \param f_c Centre frequency of the filter
     
     \param f_bw Filter bandwidth 
  */
  Radiometer * MkSSBRadio(double f_c,
			  double f_bw);


  /**
     \brief Make a model of the IRAM 22 GHz WVRs
     
     The channel centres and bandwidths are typical values for units
     at the PdB
  */
  Radiometer * MkIRAM22(void);

}

#endif

