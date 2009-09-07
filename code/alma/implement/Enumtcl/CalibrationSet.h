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


#if     !defined(_CALIBRATIONSET_H)

#include <CCalibrationSet.h>
#define _CALIBRATIONSET_H
#endif 

#if     !defined(_CALIBRATIONSET_HH)

#include "Enum.hpp"

using namespace CalibrationSetMod;

template<>
 struct enum_set_traits<CalibrationSet> : public enum_set_traiter<CalibrationSet,7,CalibrationSetMod::UNSPECIFIED> {};

template<>
class enum_map_traits<CalibrationSet,void> : public enum_map_traiter<CalibrationSet,void> {
public:
  static bool   init_;
  static string typeName_;
  static string enumerationDesc_;
  static string order_;
  static string xsdBaseType_;
  static bool   init(){
    EnumPar<void> ep;
    m_.insert(pair<CalibrationSet,EnumPar<void> >
     (CalibrationSetMod::NONE,ep((int)CalibrationSetMod::NONE,"NONE","un-documented")));
    m_.insert(pair<CalibrationSet,EnumPar<void> >
     (CalibrationSetMod::AMPLI_CURVE,ep((int)CalibrationSetMod::AMPLI_CURVE,"AMPLI_CURVE","un-documented")));
    m_.insert(pair<CalibrationSet,EnumPar<void> >
     (CalibrationSetMod::ANTENNA_POSITIONS,ep((int)CalibrationSetMod::ANTENNA_POSITIONS,"ANTENNA_POSITIONS","un-documented")));
    m_.insert(pair<CalibrationSet,EnumPar<void> >
     (CalibrationSetMod::PHASE_CURVE,ep((int)CalibrationSetMod::PHASE_CURVE,"PHASE_CURVE","un-documented")));
    m_.insert(pair<CalibrationSet,EnumPar<void> >
     (CalibrationSetMod::POINTING_MODEL,ep((int)CalibrationSetMod::POINTING_MODEL,"POINTING_MODEL","un-documented")));
    m_.insert(pair<CalibrationSet,EnumPar<void> >
     (CalibrationSetMod::TEST,ep((int)CalibrationSetMod::TEST,"TEST","un-documented")));
    m_.insert(pair<CalibrationSet,EnumPar<void> >
     (CalibrationSetMod::UNSPECIFIED,ep((int)CalibrationSetMod::UNSPECIFIED,"UNSPECIFIED","un-documented")));
    return true;
  }
  static map<CalibrationSet,EnumPar<void> > m_;
};
#define _CALIBRATIONSET_HH
#endif
