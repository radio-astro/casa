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


#if     !defined(_POINTINGMODELMODE_H)

#include <CPointingModelMode.h>
#define _POINTINGMODELMODE_H
#endif 

#if     !defined(_POINTINGMODELMODE_HH)

#include "Enum.hpp"

using namespace PointingModelModeMod;

template<>
 struct enum_set_traits<PointingModelMode> : public enum_set_traiter<PointingModelMode,2,PointingModelModeMod::OPTICAL> {};

template<>
class enum_map_traits<PointingModelMode,void> : public enum_map_traiter<PointingModelMode,void> {
public:
  static bool   init_;
  static string typeName_;
  static string enumerationDesc_;
  static string order_;
  static string xsdBaseType_;
  static bool   init(){
    EnumPar<void> ep;
    m_.insert(pair<PointingModelMode,EnumPar<void> >
     (PointingModelModeMod::RADIO,ep((int)PointingModelModeMod::RADIO,"RADIO","un-documented")));
    m_.insert(pair<PointingModelMode,EnumPar<void> >
     (PointingModelModeMod::OPTICAL,ep((int)PointingModelModeMod::OPTICAL,"OPTICAL","un-documented")));
    return true;
  }
  static map<PointingModelMode,EnumPar<void> > m_;
};
#define _POINTINGMODELMODE_HH
#endif
