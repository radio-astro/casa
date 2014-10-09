/**
   Bojan Nikolic <b.nikolic@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>

   This file is part of LibAIR and is licensed under GNU Public
   License Version 2

   \file model_iface.hpp
   
   Model interfaces
*/
#ifndef _LIBAIR_MODEL_IFACE_HPP__
#define _LIBAIR_MODEL_IFACE_HPP__

#include <vector>
#include <boost/scoped_ptr.hpp>
#include <boost/array.hpp>

#include "almawvr/minimmodel.hxx"

#include "model_enums.hpp"

namespace LibAIR2 {

  // Forward declarations
  class TrivialWaterModel;

  /** \brief Relevant atmospheric model quantities

     This class encapsulates the quantities that we wish to obtain
     from a model of the atmosphere.
   */
  class WVRAtmoQuants
  {

  public:

    // ---------- Construction / Destruction --------------


    // ---------- Public interface           --------------    

    virtual ~WVRAtmoQuants(void);
    
    /**
       \brief Brightness temperature of channel ch
     */
    virtual double eval(size_t ch) const = 0;
    
    /**
       \brief Brightness temperatures of all channels
    */
    virtual void eval(std::vector<double> & res) const = 0;

    /**
       \brief Differential of brightness of channel ch wrt the water
       column.

       \returns  in units of (K/mm).
     */
    virtual double dTdc (size_t ch) const = 0;

    /** \brief Differential of brightness of channel ch wrt the non-dispersive path.

	Assuming the non-dispersive Smith-Weintraub approximation.

	\returns in units of K/mm
	
	\note ND for non-dispersive
    */
    virtual double dTdL_ND (size_t ch) const = 0;

    /**
       \brief Differential of all of the channels.
       
       Note pure, as we can simulate it by doing it channel by channel
     */
    virtual void dTdL_ND (std::vector<double> & res) const = 0 ;

  };

  /** \brief Interface for models
     
     This class encapsulates other interfaces that models of
     atmosphere must have. So far this is just that they can enumerate
     their parameters (through inheritance of Minim::Model)
   */
  class WVRAtmoModel :
    public Minim::Model
  {

  };

  /** \brief Base class that defines the measurements that we require
      and which can be controlled by minimisers
   */
  class WVRAtmoQuantModel:
    public WVRAtmoQuants,
    public WVRAtmoModel
  {

  public:
    
    /** \brief Predicted sky brightness as a function of single
	parameter of name pname.

	In this class since we need devinitions of "eval" and of
	parameters.

	Virtual to allow more efficient specialisations which do not
	need to collect pointers to the actual parameters
	
	\note Not a const function as we are changing the parameter
     */
    virtual boost::array<double, 4>
    evalFn(double x, const std::string &pname);
  };
    
  /** \brief Model the situation where a fraction of beam terminates
      at a fixed temperature
  */
  class CouplingModel:
    public WVRAtmoQuantModel
  {
    double coupling;
    double TTerm;

    boost::scoped_ptr<WVRAtmoQuantModel> _am;

  public:
    /** Initialise from a model of the atmosphere.

	\note this class takes ownership of the model am
     */
    CouplingModel(WVRAtmoQuantModel * am);

    // ---------- Public interface   --------------    

    /** Set the spillover parameters directly
	
	\param c The coupling coefficients to the sky

	\param T Temerature at which the spillover (1-c) terminates

     */
    void setSpill(double c, double T);


    // Inherited from WVRAtmoQuants
    virtual double eval(size_t ch) const ;
    virtual void eval(std::vector<double> & res) const ;
    virtual double dTdc (size_t ch) const;
    virtual double dTdL_ND (size_t ch) const;    
    void dTdL_ND (std::vector<double> & res) const;
    // Inherited from WVRAtmoModel
    void AddParams ( std::vector< Minim::DParamCtr > &pars );

  };

  /**
  \brief Model the absolute calibration error of the radiometers
  measuring the sky

  */
  class AbsCalModel:
    public WVRAtmoQuantModel
  {
    /// The offset (in K) of the zero point of the calibration, i.e.,
    /// apparent reading when the observation should have been zero
    /// (one for each channel)
    std::vector<double> zero;

    /// The scaling of the observation (one for each channel), i.e.,
    /// if other than unity than the observations will be in error in
    /// proportion to observed temperature
    std::vector<double> scale; 
    
    /// This is the underlying model which the absolute calibration
    /// model scales to represent calibration error
    boost::scoped_ptr<WVRAtmoQuantModel> _am;

  public:
    
    // ---------- Construction / Destruction --------------
    
    /** \brief Initialise from supplied 

	\param nchan The number of channels over which the measurement
	is to be made. Default to 4 since this is the number of
	channels on ALMA WVRs
	
	\note This class will take ownership of the object "am"
	
     */
    AbsCalModel(WVRAtmoQuantModel * am,
		size_t nchan=4);

    // ---------- Public interface ------------------------

    // Inherited from WVRAtmoQuants
    virtual double eval(size_t ch) const ;
    virtual void eval(std::vector<double> & res) const ;
    virtual double dTdc (size_t ch) const;
    virtual double dTdL_ND (size_t ch) const;    
    void dTdL_ND(std::vector<double> &res) const;
    // Inherited from WVRAtmoModel
    void AddParams(std::vector<Minim::DParamCtr> &pars);    

  };
  


}
#endif

   
