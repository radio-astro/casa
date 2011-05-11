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


#if     !defined(_ANTENNATYPE_H)

#include <CAntennaType.h>
#define _ANTENNATYPE_H
#endif 

#if     !defined(_ANTENNATYPE_HH)

#include "Enum.hpp"

using namespace AntennaTypeMod;

template<>
 struct enum_set_traits<AntennaType> : public enum_set_traiter<AntennaType,3,AntennaTypeMod::TRACKING_STN> {};

template<>
class enum_map_traits<AntennaType,void> : public enum_map_traiter<AntennaType,void> {
public:
  static bool   init_;
  static string typeName_;
  static string enumerationDesc_;
  static string order_;
  static string xsdBaseType_;
  static bool   init(){
    EnumPar<void> ep;
    m_.insert(pair<AntennaType,EnumPar<void> >
     (AntennaTypeMod::GROUND_BASED,ep((int)AntennaTypeMod::GROUND_BASED,"GROUND_BASED","Ground-based antenna")));
    m_.insert(pair<AntennaType,EnumPar<void> >
     (AntennaTypeMod::SPACE_BASED,ep((int)AntennaTypeMod::SPACE_BASED,"SPACE_BASED","Antenna in a spacecraft")));
    m_.insert(pair<AntennaType,EnumPar<void> >
     (AntennaTypeMod::TRACKING_STN,ep((int)AntennaTypeMod::TRACKING_STN,"TRACKING_STN","Space-tracking station antenna")));
    return true;
  }
  static map<AntennaType,EnumPar<void> > m_;
};
#define _ANTENNATYPE_HH
#endif
