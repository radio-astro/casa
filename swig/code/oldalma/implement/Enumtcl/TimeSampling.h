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


#if     !defined(_TIMESAMPLING_H)

#include <CTimeSampling.h>
#define _TIMESAMPLING_H
#endif 

#if     !defined(_TIMESAMPLING_HH)

#include "Enum.hpp"

using namespace TimeSamplingMod;

template<>
 struct enum_set_traits<TimeSampling> : public enum_set_traiter<TimeSampling,2,TimeSamplingMod::INTEGRATION> {};

template<>
class enum_map_traits<TimeSampling,void> : public enum_map_traiter<TimeSampling,void> {
public:
  static bool   init_;
  static string typeName_;
  static string enumerationDesc_;
  static string order_;
  static string xsdBaseType_;
  static bool   init(){
    EnumPar<void> ep;
    m_.insert(pair<TimeSampling,EnumPar<void> >
     (TimeSamplingMod::SUBINTEGRATION,ep((int)TimeSamplingMod::SUBINTEGRATION,"SUBINTEGRATION","Part of an integration")));
    m_.insert(pair<TimeSampling,EnumPar<void> >
     (TimeSamplingMod::INTEGRATION,ep((int)TimeSamplingMod::INTEGRATION,"INTEGRATION","Part of a subscan. An integration may be composed of several sub-integrations.")));
    return true;
  }
  static map<TimeSampling,EnumPar<void> > m_;
};
#define _TIMESAMPLING_HH
#endif
