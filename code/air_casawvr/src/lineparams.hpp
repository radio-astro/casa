/**
   Bojan Nikolic <bn204@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>
   Initial version February 2008
   Maintained by ESO since 2013.

   This file is part of LibAIR and is licensed under GNU Public
   License Version 2

   \file lineparams.hpp

   Line parameter calculations.

   The two primary variables to consider are temperature and
   pressure. The third possilbe variable is the volume mixing ratio
   (vmr) which determines air broadening versus self-broadening.

   
*/
#ifndef _LIBAIR_LINEPARAMS_HPP__
#define _LIBAIR_LINEPARAMS_HPP__

#include <memory>
#include <cstddef>

namespace LibAIR2 {

  // Forward declarations
  class PartitionTable;  

  /**
     An entry in the hitran database
   */
  struct HITRAN_entry {

    /*!\brief  Line intensity  */
    double S;
    /*!\brief  Line frequency  */    
    double freq;
    /*!\brief Air-induced pressure shift */
    double delta_air  ;
    /*!\brief HITRAN isotopologue number */
    int    iso;
    /*!\brief Lower-state energy */
    double Elo;
    /*!\brief Air-broadening coefficient */
    double gam_air;
    /*!\brief Self-broadening coefficient [GHz / mbar] */
    double gam_self;
    /*!\brief T dependence exponent of gam_air */
    double nair;
    
  };

  /**
     Line parameters needed for actual computation of its strength.
   */
  struct CLineParams
  {
    /// Line frequency
    double f0;
    /// Line strength
    double S;
    /// Line width
    double gamma;
  };

  /** \brief Compute the actual line parameters 

      Temparature dependance so far:
      \f[ 
      \propto 
      e^{\frac{E_l (T-T_{\rm ref})}{T T_{\rm ref}}}
      \frac{ 1- e^{ -h\nu/T}}{1-e^{-h\nu/T_{\rm ref}}}
      \f]

      \param P is pressure in mbar

      \bug only valid for Gross as far as I know so far.

      \bug This ignores the Q-ratio completely. Should be using
      something like a linear approximation.
   */
  void ComputeLinePars(const HITRAN_entry & he,
		       double T,
		       double P,
		       double vmr,
		       double Tref,
		       CLineParams & res);


  /**
     \brief Like ComputeLinePars but take into accout the partition
     sum table

      Temparature dependance:
      \f[ 
      \propto 
      e^{\frac{E_l (T-T_{\rm ref})}{T T_{\rm ref}}}
      \frac{ 1- e^{ -h\nu/T}}{1-e^{-h\nu/T_{\rm ref}}}
      \frac{ Q(T_{\rm ref} )}{Q(T)}
      \f]

     \note pt must be for the species that is specified in he.

   */
  void ComputeLineParsWQ(const HITRAN_entry & he,
			 const PartitionTable &pt,
			 double T,
			 double P,
			 double vmr,
			 double Tref,
			 CLineParams & res);

  /** \brief Creates the 183 GHz Water line Hi Tran entry

      Primarily for testing purposes.
  */
  HITRAN_entry * Mk183WaterEntry(void);
  
  /** \brief Create the 22GHz Water Line HiTRAN entry
      
      \bug Primarily for testing use. Long term should read external
      database.
  */
  HITRAN_entry * Mk22WaterEntry(void);


  /** Structure for the parameters of continuum
   */
  struct ContinuumParams 
  {
    
    /** Strentgth of the continuum
    */
    double C0;

    /**reference temperature [K] */
    double T0;

    /** Temperature exponent */
    double m;

  };

  /** \brief Returns the continuum params for water for gross line
      shapes*/
  ContinuumParams *  MkWaterGrossCont(void);

  const HITRAN_entry * get_h2o_lines(void);
  size_t  get_h2o_lines_n(void);

}

#endif   

