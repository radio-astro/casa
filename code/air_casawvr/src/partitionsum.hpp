/**
   Bojan Nikolic <bn204@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>
   Initial version February 2008
   Maintained by ESO since 2013.

   This file is part of LibAIR and is licensed under GNU Public
   License Version 2

   \file partitionsum.hpp

   Support for partition sum tables
*/

#ifndef __LIBAIR_PARTITIONSUM_HPP__
#define __LIBAIR_PARTITIONSUM_HPP__

#include "boost/multi_array.hpp"

namespace LibAIR2 {


  /**      
   Following "am", the raw partition table can be represented
   as two dimensional array
   */
  typedef boost::const_multi_array_ref<double, 2>  part_table_raw;

  /**
     Partition sum table relates temperatures to a thermodynamic
     function of is isotope in the Hitran database. 

     
   */
  class PartitionTable
  {
    
    part_table_raw raw;
    
  public:

    // ---------- Construction / Destruction --------------

    /**
       Construct from a raw table. Expect first column is tempertaure
       and the others represent isotopologues sequenced in the same
       way as in HITRAN.
     */
    PartitionTable(const part_table_raw & raw);

    // ---------- Public interface ------------------------

    /** Return the row number with the highest temperature not greater
	than T */
    size_t findrow(double T) const ;

    /** Return the linearly-interpolated partition sum at temperature
	T for isotopologue number i*/
    double eval(double T, size_t i) const;
    
  };



  

}

#endif
