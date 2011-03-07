/*
 *
 * /////////////////////////////////////////////////////////////////
 * // WARNING!  DO NOT MODIFY THIS FILE!                          //
 * //  ---------------------------------------------------------  //
 * // | This is generated code using a C++ template function!   | //
 * // ! Do not modify this file.                                | //
 * // | Any changes will be lost when the file is re-generated. | //
 * //  ---------------------------------------------------------  //
 * /////////////////////////////////////////////////////////////////
 *
 */


#if     !defined(_ATMPHASECORRECTION_H)

#include <CAtmPhaseCorrection.h>
#define _ATMPHASECORRECTION_H
#endif 

#if     !defined(_ATMPHASECORRECTION_HH)

#include "Enum.hpp"

using namespace AtmPhaseCorrectionMod;

template<>
 struct enum_set_traits<AtmPhaseCorrection> : public enum_set_traiter<AtmPhaseCorrection,2,AtmPhaseCorrectionMod::AP_CORRECTED> {};

template<>
class enum_map_traits<AtmPhaseCorrection,void> : public enum_map_traiter<AtmPhaseCorrection,void> {
public:
  static bool   init_;
  static string typeName_;
  static string enumerationDesc_;
  static string order_;
  static string xsdBaseType_;
  static bool   init(){
    EnumPar<void> ep;
    m_.insert(pair<AtmPhaseCorrection,EnumPar<void> >
     (AtmPhaseCorrectionMod::AP_UNCORRECTED,ep((int)AtmPhaseCorrectionMod::AP_UNCORRECTED,"AP_UNCORRECTED","un-documented")));
    m_.insert(pair<AtmPhaseCorrection,EnumPar<void> >
     (AtmPhaseCorrectionMod::AP_CORRECTED,ep((int)AtmPhaseCorrectionMod::AP_CORRECTED,"AP_CORRECTED","un-documented")));
    return true;
  }
  static map<AtmPhaseCorrection,EnumPar<void> > m_;
};
#define _ATMPHASECORRECTION_HH
#endif
