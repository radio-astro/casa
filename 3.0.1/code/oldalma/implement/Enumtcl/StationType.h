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


#if     !defined(_STATIONTYPE_H)

#include <CStationType.h>
#define _STATIONTYPE_H
#endif 

#if     !defined(_STATIONTYPE_HH)

#include "Enum.hpp"

using namespace StationTypeMod;

template<>
 struct enum_set_traits<StationType> : public enum_set_traiter<StationType,3,StationTypeMod::WEATHER_STATION> {};

template<>
class enum_map_traits<StationType,void> : public enum_map_traiter<StationType,void> {
public:
  static bool   init_;
  static string typeName_;
  static string enumerationDesc_;
  static string order_;
  static string xsdBaseType_;
  static bool   init(){
    EnumPar<void> ep;
    m_.insert(pair<StationType,EnumPar<void> >
     (StationTypeMod::ANTENNA_PAD,ep((int)StationTypeMod::ANTENNA_PAD,"ANTENNA_PAD","Astronomical Antenna station")));
    m_.insert(pair<StationType,EnumPar<void> >
     (StationTypeMod::MAINTENANCE_PAD,ep((int)StationTypeMod::MAINTENANCE_PAD,"MAINTENANCE_PAD","Maintenance antenna station")));
    m_.insert(pair<StationType,EnumPar<void> >
     (StationTypeMod::WEATHER_STATION,ep((int)StationTypeMod::WEATHER_STATION,"WEATHER_STATION","Weather station")));
    return true;
  }
  static map<StationType,EnumPar<void> > m_;
};
#define _STATIONTYPE_HH
#endif
