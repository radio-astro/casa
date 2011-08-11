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


#if     !defined(_DOPPLERTRACKINGMODE_H)

#include <CDopplerTrackingMode.h>
#define _DOPPLERTRACKINGMODE_H
#endif 

#if     !defined(_DOPPLERTRACKINGMODE_HH)

#include "Enum.hpp"

using namespace DopplerTrackingModeMod;

template<>
 struct enum_set_traits<DopplerTrackingModeMod::DopplerTrackingMode> : public enum_set_traiter<DopplerTrackingModeMod::DopplerTrackingMode,4,DopplerTrackingModeMod::SB_BASED> {};

template<>
class enum_map_traits<DopplerTrackingModeMod::DopplerTrackingMode,void> : public enum_map_traiter<DopplerTrackingModeMod::DopplerTrackingMode,void> {
public:
  static bool   init_;
  static string typeName_;
  static string enumerationDesc_;
  static string order_;
  static string xsdBaseType_;
  static bool   init(){
    EnumPar<void> ep;
    m_.insert(pair<DopplerTrackingModeMod::DopplerTrackingMode,EnumPar<void> >
     (DopplerTrackingModeMod::NONE,ep((int)DopplerTrackingModeMod::NONE,"NONE","un-documented")));
    m_.insert(pair<DopplerTrackingModeMod::DopplerTrackingMode,EnumPar<void> >
     (DopplerTrackingModeMod::CONTINUOUS,ep((int)DopplerTrackingModeMod::CONTINUOUS,"CONTINUOUS","un-documented")));
    m_.insert(pair<DopplerTrackingModeMod::DopplerTrackingMode,EnumPar<void> >
     (DopplerTrackingModeMod::SCAN_BASED,ep((int)DopplerTrackingModeMod::SCAN_BASED,"SCAN_BASED","un-documented")));
    m_.insert(pair<DopplerTrackingModeMod::DopplerTrackingMode,EnumPar<void> >
     (DopplerTrackingModeMod::SB_BASED,ep((int)DopplerTrackingModeMod::SB_BASED,"SB_BASED","un-documented")));
    return true;
  }
  static map<DopplerTrackingModeMod::DopplerTrackingMode,EnumPar<void> > m_;
};
#define _DOPPLERTRACKINGMODE_HH
#endif
