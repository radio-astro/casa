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


#if     !defined(_CALCURVETYPE_H)

#include <CCalCurveType.h>
#define _CALCURVETYPE_H
#endif 

#if     !defined(_CALCURVETYPE_HH)

#include "Enum.hpp"

using namespace CalCurveTypeMod;

template<>
 struct enum_set_traits<CalCurveType> : public enum_set_traiter<CalCurveType,3,CalCurveTypeMod::UNDEFINED> {};

template<>
class enum_map_traits<CalCurveType,void> : public enum_map_traiter<CalCurveType,void> {
public:
  static bool   init_;
  static string typeName_;
  static string enumerationDesc_;
  static string order_;
  static string xsdBaseType_;
  static bool   init(){
    EnumPar<void> ep;
    m_.insert(pair<CalCurveType,EnumPar<void> >
     (CalCurveTypeMod::AMPLITUDE,ep((int)CalCurveTypeMod::AMPLITUDE,"AMPLITUDE","un-documented")));
    m_.insert(pair<CalCurveType,EnumPar<void> >
     (CalCurveTypeMod::PHASE,ep((int)CalCurveTypeMod::PHASE,"PHASE","un-documented")));
    m_.insert(pair<CalCurveType,EnumPar<void> >
     (CalCurveTypeMod::UNDEFINED,ep((int)CalCurveTypeMod::UNDEFINED,"UNDEFINED","un-documented")));
    return true;
  }
  static map<CalCurveType,EnumPar<void> > m_;
};
#define _CALCURVETYPE_HH
#endif
