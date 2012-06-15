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


#if     !defined(_TIMESCALE_H)

#include <CTimeScale.h>
#define _TIMESCALE_H
#endif 

#if     !defined(_TIMESCALE_HH)

#include "Enum.hpp"

using namespace TimeScaleMod;

template<>
 struct enum_set_traits<TimeScaleMod::TimeScale> : public enum_set_traiter<TimeScaleMod::TimeScale,2,TimeScaleMod::TAI> {};

template<>
class enum_map_traits<TimeScaleMod::TimeScale,void> : public enum_map_traiter<TimeScaleMod::TimeScale,void> {
public:
  static bool   init_;
  static string typeName_;
  static string enumerationDesc_;
  static string order_;
  static string xsdBaseType_;
  static bool   init(){
    EnumPar<void> ep;
    m_.insert(pair<TimeScaleMod::TimeScale,EnumPar<void> >
     (TimeScaleMod::UTC,ep((int)TimeScaleMod::UTC,"UTC","un-documented")));
    m_.insert(pair<TimeScaleMod::TimeScale,EnumPar<void> >
     (TimeScaleMod::TAI,ep((int)TimeScaleMod::TAI,"TAI","un-documented")));
    return true;
  }
  static map<TimeScaleMod::TimeScale,EnumPar<void> > m_;
};
#define _TIMESCALE_HH
#endif
