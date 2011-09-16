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


#if     !defined(_POLARIZATIONTYPE_H)

#include <CPolarizationType.h>
#define _POLARIZATIONTYPE_H
#endif 

#if     !defined(_POLARIZATIONTYPE_HH)

#include "Enum.hpp"

using namespace PolarizationTypeMod;

template<>
 struct enum_set_traits<PolarizationType> : public enum_set_traiter<PolarizationType,4,PolarizationTypeMod::Y> {};

template<>
class enum_map_traits<PolarizationType,void> : public enum_map_traiter<PolarizationType,void> {
public:
  static bool   init_;
  static string typeName_;
  static string enumerationDesc_;
  static string order_;
  static string xsdBaseType_;
  static bool   init(){
    EnumPar<void> ep;
    m_.insert(pair<PolarizationType,EnumPar<void> >
     (PolarizationTypeMod::R,ep((int)PolarizationTypeMod::R,"R","Right-handed Circular")));
    m_.insert(pair<PolarizationType,EnumPar<void> >
     (PolarizationTypeMod::L,ep((int)PolarizationTypeMod::L,"L","Left-handed Circular")));
    m_.insert(pair<PolarizationType,EnumPar<void> >
     (PolarizationTypeMod::X,ep((int)PolarizationTypeMod::X,"X","X linear")));
    m_.insert(pair<PolarizationType,EnumPar<void> >
     (PolarizationTypeMod::Y,ep((int)PolarizationTypeMod::Y,"Y","Y linear")));
    return true;
  }
  static map<PolarizationType,EnumPar<void> > m_;
};
#define _POLARIZATIONTYPE_HH
#endif
