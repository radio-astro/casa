/**
   Bojan Nikolic <b.nikolic@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>

   This file is part of LibAIR and is licensed under GNU Public
   License Version 2

   \file columns_data.hpp
*/
#ifndef _LIBAIR_COLUMNS_DATA_HPP__
#define _LIBAIR_COLUMNS_DATA_HPP__

#include <boost/scoped_ptr.hpp>

#include "model_enums.hpp"

namespace LibAIR2 {

  class Column;
  class TrivialGrossColumn;
  class ContinuumColumn;
  class PartitionTable;

  /** Utility class for any water columns to manage the lifetime of
      associated data.
   */
  class WaterData
  {

    /// The partition table (if used)
    boost::scoped_ptr<PartitionTable> pt;

    /// The water column
    boost::scoped_ptr<Column> _wcol;

    /// The water continuum column
    boost::scoped_ptr<ContinuumColumn> _wcont;
    
  public:

    // ---------- Public types ----------------------
    
    /// The lines that we know about. 
    enum Lines { 
      ///Set up for simulating the 183 GHz line
      L183, 
      ///Set up for simulating the 22 GHz line
      L22,
      ///Set up all of the lines
      LALL};


    // ---------- Public data --------------

    Column * wcol(void) {return _wcol.get();};

    ContinuumColumn * wcont(void) {return _wcont.get();} ;
    
    // ---------- Construction / Destruction --------------

    WaterData(Lines line,
	      PartitionTreatment t,
	      Continuum c,
	      double n);

    virtual ~WaterData();

  };

  /** Specialisation for 183 GHz. 

      \bug Implemented only for compatibility purposes while
      refactoring WaterData
   */
  class Water183Data :
    public WaterData
  {

  public:

    // ---------- Public types ----------------------

    // ---------- Public data --------------
    
    // ---------- Construction / Destruction --------------

    Water183Data(PartitionTreatment t,
		 Continuum c,
		 double n);
    
  };

}

#endif
