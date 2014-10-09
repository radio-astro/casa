/**
   Bojan Nikolic <b.nikolic@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>
   
   This file is part of LibAIR and is licensed under GNU Public
   License Version 2

   \file model_enums.hpp

*/
#ifndef _LIBAIR_MODEL_ENUMS_HPP__
#define _LIBAIR_MODEL_ENUMS_HPP__

namespace LibAIR2 {

  /** \brief Types of radiometers that we know about.
   */
  enum RadiometerT { 
    /// ALMA production as specified
    ALMAProd,
    /// The Dicke prototype for ALMA
    ALMADickeProto,
    /// The 22GHz WVRs on PdBI
    IRAM22GHz
  };

  /**\brief The ways of dealing with the partition sum 
  */
  enum PartitionTreatment { 
    /// Ignore the partition sum calculation (not recomended)
    PartNone,
    /// Use the partition sum table
    PartTable 
  };

  /**\brief How to calculate the contiuum
   */
  enum Continuum { 
    /// Ignore contiuum
    NoCont,
    /// Calculate water-dry air continuum
    AirCont 
  };



}
#endif
