/**
   Bojan Nikolic <b.nikolic@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>
   Initial version January 2010.
   Maintained by ESO since 2013.

   This file is part of LibAIR and is licensed under GNU Public
   License Version 2

   \file dtdlcoeffs.hpp

   Class hirerachy defining computed dT/dL coefficients for an ALMA
   observation
*/
#ifndef _LIBAIR_DTDLCOEFFS_HPP__
#define _LIBAIR_DTDLCOEFFS_HPP__

#include <vector>
#include <set>
#include <iosfwd>

#include <boost/array.hpp>
#include <boost/multi_array.hpp>

namespace LibAIR2 {

  // Forward declarations
  class ALMAResBase;

  /** \brief Base class representing dTdL coefficients that can be
      applied to observed interferometer data to correct the phase
   */
  class dTdLCoeffsBase
  {

  public:

    // ----------------------  Public data -------------------------

    /// Allows masking or scaling of channels
    boost::array<double, 4> chmask;

    // ----------------------- Construction/Destruction -------------
    
    dTdLCoeffsBase();
    
    /** Virtual destructor requried in this calls
     */
    virtual ~dTdLCoeffsBase();

    // ----------------------- Public Interface ---------------------
      

    /** Get the coefficients 
	
	\param i WVR or antenna number
	\param time Time 
	\param el elevation
	
	\param res Vector to store the result in

	\param c2 Vector to stre the second order differentials (store
	zeros here if these have not been estimated)
    */
    virtual void get(size_t i,
		     double time,
		     double el,
		     std::vector<double> &res,
		     std::vector<double> &c2) const =0;

    //virtual void print(std::ostream &os)
    //{
    //};

    /** \brief Return a representative set of coefficients
	
	These can be used for error estimates, printing to the user,
	etc.
     */
    virtual void repr(std::vector<double> &res,
		      std::vector<double> &err) const =0;

    /** \brief Are all coefficients in this object finite?
	Useful to check for errors
     */
    virtual bool isnan(void) const =0;
		     
  };

  /** \brief The trivial implementation in which alsways a single set
      of coefficients is returned
   */
  class dTdLCoeffsSingle:
    public dTdLCoeffsBase
  {

    std::vector<double> c, c2, e;

  public:

    // ----------------------- Construction/Destruction -------------
    
    /**
       \param c The phase correction coefficients 

       \param e The errors on the coefficients
     */
    dTdLCoeffsSingle(const std::vector<double> &c,
		     const std::vector<double> &e);

    /// Construct from a single retrieval, assuming that the values
    /// are valid for all WVRs
    dTdLCoeffsSingle(const  ALMAResBase &r);

    // ----------------------- Public Interface ---------------------

    // ----------------------- Inherited from dTdLCoeffsBase --------
    virtual void get(size_t i,
		     double time,
		     double el,
		     std::vector<double> &res,
		     std::vector<double> &res2) const;

    virtual void print(std::ostream &os);

    virtual void repr(std::vector<double> &res,
		      std::vector<double> &err) const;

    virtual bool isnan(void) const;

  };


  /** \brief Separate coefficents for each WVR
   */
  class dTdLCoeffsIndiv:
    public dTdLCoeffsBase
  {
  public:

    /// First dimension is channel number, second is antenna number, third
    /// is (coefficent, error, second order coefficient)
    typedef boost::multi_array<double, 3> coeff_t;

  private:

    coeff_t coeff;

  public:

    // ----------------------- Construction/Destruction -------------
    
    /**
       \param c The phase correction coefficients 

       \param e The errors on the coefficients
     */
    dTdLCoeffsIndiv(const coeff_t &c);

    dTdLCoeffsIndiv(size_t nAnts);

    // ----------------------- Public Interface ---------------------

    /// Set the coefficients for antenna i
    void set(size_t i,
	     const std::vector<double> &c,
	     const std::vector<double> &e);

    // ----------------------- Inherited from dTdLCoeffsBase --------
    virtual void get(size_t i,
		     double time,
		     double el,
		     std::vector<double> &res,
		     std::vector<double> &c2) const;

    virtual void print(std::ostream &os);

    virtual void repr(std::vector<double> &res,
		      std::vector<double> &err) const;

    virtual bool isnan(void) const;

  };

  /** \brief Implementation in which the coefficients are interpolated
     in time
     
   */
  class dTdLCoeffsSingleInterpolated:
    public dTdLCoeffsBase
  {

    struct ret_t {

      /// The time to which the retrieval applies
      double time;
      
      /// The coefficients
      boost::array<double, 4> coeffs;

      /// Second order coefficients
      boost::array<double, 4> c2;

      boost::array<double, 4> err;

      /// Defines the ordering of retrievals so that they can be
      /// sorted in a set
      bool operator<(const ret_t &other) const{
	return time<other.time;
      }
      
    };

    std::set<ret_t> retrievals;

  public:

    // ----------------------- Construction/Destruction -------------
    
    dTdLCoeffsSingleInterpolated();

    // ----------------------- Public Interface ---------------------

    /** Add a new solution to the sequence
     */
    void insert(double time,
		const boost::array<double, 4> &coeffs,
		const boost::array<double, 4> &err);


    // ----------------------- Inherited from dTdLCoeffsBase --------
    virtual void get(size_t i,
		     double time,
		     double el,
		     std::vector<double> &res,
		     std::vector<double> &c2) const;
    virtual void print(std::ostream &os);
    virtual void repr(std::vector<double> &res,
		      std::vector<double> &err) const;
    virtual bool isnan() const;
    
  };


}

#endif
