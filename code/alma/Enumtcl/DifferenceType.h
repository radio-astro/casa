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


#if     !defined(_DIFFERENCETYPE_H)

#include <CDifferenceType.h>
#define _DIFFERENCETYPE_H
#endif 

#if     !defined(_DIFFERENCETYPE_HH)

#include "Enum.hpp"

using namespace DifferenceTypeMod;

template<>
 struct enum_set_traits<DifferenceTypeMod::DifferenceType> : public enum_set_traiter<DifferenceTypeMod::DifferenceType,4,DifferenceTypeMod::FINAL> {};

template<>
class enum_map_traits<DifferenceTypeMod::DifferenceType,void> : public enum_map_traiter<DifferenceTypeMod::DifferenceType,void> {
public:
  static bool   init_;
  static string typeName_;
  static string enumerationDesc_;
  static string order_;
  static string xsdBaseType_;
  static bool   init(){
    EnumPar<void> ep;
    m_.insert(pair<DifferenceTypeMod::DifferenceType,EnumPar<void> >
     (DifferenceTypeMod::PREDICTED,ep((int)DifferenceTypeMod::PREDICTED,"PREDICTED","un-documented")));
    m_.insert(pair<DifferenceTypeMod::DifferenceType,EnumPar<void> >
     (DifferenceTypeMod::PRELIMINARY,ep((int)DifferenceTypeMod::PRELIMINARY,"PRELIMINARY","un-documented")));
    m_.insert(pair<DifferenceTypeMod::DifferenceType,EnumPar<void> >
     (DifferenceTypeMod::RAPID,ep((int)DifferenceTypeMod::RAPID,"RAPID","un-documented")));
    m_.insert(pair<DifferenceTypeMod::DifferenceType,EnumPar<void> >
     (DifferenceTypeMod::FINAL,ep((int)DifferenceTypeMod::FINAL,"FINAL","un-documented")));
    return true;
  }
  static map<DifferenceTypeMod::DifferenceType,EnumPar<void> > m_;
};
#define _DIFFERENCETYPE_HH
#endif
