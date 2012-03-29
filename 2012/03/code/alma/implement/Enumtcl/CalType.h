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


#if     !defined(_CALTYPE_H)

#include <CCalType.h>
#define _CALTYPE_H
#endif 

#if     !defined(_CALTYPE_HH)

#include "Enum.hpp"

using namespace CalTypeMod;

template<>
 struct enum_set_traits<CalTypeMod::CalType> : public enum_set_traiter<CalTypeMod::CalType,17,CalTypeMod::CAL_WVR> {};

template<>
class enum_map_traits<CalTypeMod::CalType,void> : public enum_map_traiter<CalTypeMod::CalType,void> {
public:
  static bool   init_;
  static string typeName_;
  static string enumerationDesc_;
  static string order_;
  static string xsdBaseType_;
  static bool   init(){
    EnumPar<void> ep;
    m_.insert(pair<CalTypeMod::CalType,EnumPar<void> >
     (CalTypeMod::CAL_AMPLI,ep((int)CalTypeMod::CAL_AMPLI,"CAL_AMPLI","un-documented")));
    m_.insert(pair<CalTypeMod::CalType,EnumPar<void> >
     (CalTypeMod::CAL_ATMOSPHERE,ep((int)CalTypeMod::CAL_ATMOSPHERE,"CAL_ATMOSPHERE","un-documented")));
    m_.insert(pair<CalTypeMod::CalType,EnumPar<void> >
     (CalTypeMod::CAL_BANDPASS,ep((int)CalTypeMod::CAL_BANDPASS,"CAL_BANDPASS","un-documented")));
    m_.insert(pair<CalTypeMod::CalType,EnumPar<void> >
     (CalTypeMod::CAL_CURVE,ep((int)CalTypeMod::CAL_CURVE,"CAL_CURVE","un-documented")));
    m_.insert(pair<CalTypeMod::CalType,EnumPar<void> >
     (CalTypeMod::CAL_DELAY,ep((int)CalTypeMod::CAL_DELAY,"CAL_DELAY","un-documented")));
    m_.insert(pair<CalTypeMod::CalType,EnumPar<void> >
     (CalTypeMod::CAL_FLUX,ep((int)CalTypeMod::CAL_FLUX,"CAL_FLUX","un-documented")));
    m_.insert(pair<CalTypeMod::CalType,EnumPar<void> >
     (CalTypeMod::CAL_FOCUS,ep((int)CalTypeMod::CAL_FOCUS,"CAL_FOCUS","un-documented")));
    m_.insert(pair<CalTypeMod::CalType,EnumPar<void> >
     (CalTypeMod::CAL_FOCUS_MODEL,ep((int)CalTypeMod::CAL_FOCUS_MODEL,"CAL_FOCUS_MODEL","un-documented")));
    m_.insert(pair<CalTypeMod::CalType,EnumPar<void> >
     (CalTypeMod::CAL_GAIN,ep((int)CalTypeMod::CAL_GAIN,"CAL_GAIN","un-documented")));
    m_.insert(pair<CalTypeMod::CalType,EnumPar<void> >
     (CalTypeMod::CAL_HOLOGRAPHY,ep((int)CalTypeMod::CAL_HOLOGRAPHY,"CAL_HOLOGRAPHY","un-documented")));
    m_.insert(pair<CalTypeMod::CalType,EnumPar<void> >
     (CalTypeMod::CAL_PHASE,ep((int)CalTypeMod::CAL_PHASE,"CAL_PHASE","un-documented")));
    m_.insert(pair<CalTypeMod::CalType,EnumPar<void> >
     (CalTypeMod::CAL_POINTING,ep((int)CalTypeMod::CAL_POINTING,"CAL_POINTING","un-documented")));
    m_.insert(pair<CalTypeMod::CalType,EnumPar<void> >
     (CalTypeMod::CAL_POINTING_MODEL,ep((int)CalTypeMod::CAL_POINTING_MODEL,"CAL_POINTING_MODEL","un-documented")));
    m_.insert(pair<CalTypeMod::CalType,EnumPar<void> >
     (CalTypeMod::CAL_POSITION,ep((int)CalTypeMod::CAL_POSITION,"CAL_POSITION","un-documented")));
    m_.insert(pair<CalTypeMod::CalType,EnumPar<void> >
     (CalTypeMod::CAL_PRIMARY_BEAM,ep((int)CalTypeMod::CAL_PRIMARY_BEAM,"CAL_PRIMARY_BEAM","un-documented")));
    m_.insert(pair<CalTypeMod::CalType,EnumPar<void> >
     (CalTypeMod::CAL_SEEING,ep((int)CalTypeMod::CAL_SEEING,"CAL_SEEING","un-documented")));
    m_.insert(pair<CalTypeMod::CalType,EnumPar<void> >
     (CalTypeMod::CAL_WVR,ep((int)CalTypeMod::CAL_WVR,"CAL_WVR","un-documented")));
    return true;
  }
  static map<CalTypeMod::CalType,EnumPar<void> > m_;
};
#define _CALTYPE_HH
#endif
