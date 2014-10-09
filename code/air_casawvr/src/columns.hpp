/**
   \file columns.hpp
   Bojan Nikolic <bn204@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>
   
   Initial version February 2008
   Maintained by ESO since 2013.


*/

#ifndef __LIBAIR__COLUMNS__HPP__
#define __LIBAIR__COLUMNS__HPP__

#include <vector>
#include <boost/scoped_ptr.hpp>

namespace LibAIR2 {

  // Forwards 
  struct HITRAN_entry ;
  struct ContinuumParams ;  

  class  PartitionTable;
  class  Slice;

  /**
     \brief A base class for types of columns

   */
  class Column 
  {

    /// The number of atoms of this species in this column
    double n;

  public:

    // ---------- Construction / Destruction --------------

    Column(double n):
      n(n)
    {};


    virtual ~Column() {};

    // ---------- Public interface ------------------------


    /** \brief Set the column densitiy to a new value
     */
    void setN(double nnew);

    /** \brief Return the column density 
     */
    double getN(void) const
    {
      return n;
    }

    /**
       Compute opacity of this line on a set of frequencies.
     */
    virtual void ComputeTau( const std::vector<double> & f,
			     const Slice & s,
			     std::vector<double> & res) const = 0;

  };

  /**
     An experiemntal class for a column of a species that only
     contains a single line.

     \bug No proper reference temperature handling
     \bug No volume mixing
     \bug Single line only

   */
  class TrivialGrossColumn :
    public Column
  {
    boost::scoped_ptr<HITRAN_entry>  he;

    /**
       If provided, the partition table goes here. The correction is
       always applied from the table if it is present.
     */
    const PartitionTable * pt;

  public:
    
    // ---------- Construction / Destruction --------------

    /** 
	\param n the column density of the species
     */
    TrivialGrossColumn(const HITRAN_entry  & he,
		       double n);

    /** \brief Costructor taking a PartitionTable

	The partition table is used in the calculation.

	\note Not makig a copy of the partition table
     */
    TrivialGrossColumn(const HITRAN_entry   & he,
		       const PartitionTable * pt,
		       double n);
    
    
    // ---------- Public interface ------------------------


    // ------------------ Inherited from Column -------------
    void ComputeTau( const std::vector<double> & f,
		     const Slice & s,
		     std::vector<double> & res) const;

  };

  /**

   */
  class H2OCol:
    public Column    
  {
    
    const HITRAN_entry * ltable;
    size_t nlines;

    const PartitionTable * pt;

  public:

    H2OCol(const PartitionTable * pt,
	   size_t nlines=0);

    // ---------- Public interface ------------------------
    // ------------------ Inherited from Column -------------
    void ComputeTau( const std::vector<double> & f,
		     const Slice & s,
		     std::vector<double> & res) const;
    
  };

  /** \brief An experimental column to represent the continuum due to
      a species.

   */
  class ContinuumColumn :     
    public Column
  {

    boost::scoped_ptr<ContinuumParams>  cp;

  public:
    
    // ---------- Construction / Destruction --------------

    /**
       \param cp take ownership of this parameter
     */
    ContinuumColumn( double n,
		     ContinuumParams * cp) ;


    // ------------------ Inherited from Column -------------
    void ComputeTau( const std::vector<double> & f,
		     const Slice & s,
		     std::vector<double> & res) const;    
    
    
  };

  /** \brief Column representing an empirical continuum

      The parameter N is intepreted as the opacity at a reference
      frequency. Frequency squared dependence of opacity is assuemd
      
  */
  class EmpContColumn:
    public Column
  {
    const double ff;

  public:

    /**  
	 \param n The initial species column (in this case opacity at
	 f0)
	 
	 \param f0 The reference frequency
     */
    EmpContColumn(double n,
		  double f0);

    // ------------------ Inherited from Column -------------
    void ComputeTau(const std::vector<double> &f,
		    const Slice &s,
		    std::vector<double> &res) const;        

  };

}

#endif
