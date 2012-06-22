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


#if     !defined(_AXISNAME_H)

#include <CAxisName.h>
#define _AXISNAME_H
#endif 

#if     !defined(_AXISNAME_HH)

#include "Enum.hpp"

using namespace AxisNameMod;

template<>
 struct enum_set_traits<AxisNameMod::AxisName> : public enum_set_traiter<AxisNameMod::AxisName,13,AxisNameMod::HOL> {};

template<>
class enum_map_traits<AxisNameMod::AxisName,void> : public enum_map_traiter<AxisNameMod::AxisName,void> {
public:
  static bool   init_;
  static string typeName_;
  static string enumerationDesc_;
  static string order_;
  static string xsdBaseType_;
  static bool   init(){
    EnumPar<void> ep;
    m_.insert(pair<AxisNameMod::AxisName,EnumPar<void> >
     (AxisNameMod::TIM,ep((int)AxisNameMod::TIM,"TIM","un-documented")));
    m_.insert(pair<AxisNameMod::AxisName,EnumPar<void> >
     (AxisNameMod::BAL,ep((int)AxisNameMod::BAL,"BAL","un-documented")));
    m_.insert(pair<AxisNameMod::AxisName,EnumPar<void> >
     (AxisNameMod::ANT,ep((int)AxisNameMod::ANT,"ANT","un-documented")));
    m_.insert(pair<AxisNameMod::AxisName,EnumPar<void> >
     (AxisNameMod::BAB,ep((int)AxisNameMod::BAB,"BAB","un-documented")));
    m_.insert(pair<AxisNameMod::AxisName,EnumPar<void> >
     (AxisNameMod::SPW,ep((int)AxisNameMod::SPW,"SPW","un-documented")));
    m_.insert(pair<AxisNameMod::AxisName,EnumPar<void> >
     (AxisNameMod::SIB,ep((int)AxisNameMod::SIB,"SIB","un-documented")));
    m_.insert(pair<AxisNameMod::AxisName,EnumPar<void> >
     (AxisNameMod::SUB,ep((int)AxisNameMod::SUB,"SUB","un-documented")));
    m_.insert(pair<AxisNameMod::AxisName,EnumPar<void> >
     (AxisNameMod::BIN,ep((int)AxisNameMod::BIN,"BIN","un-documented")));
    m_.insert(pair<AxisNameMod::AxisName,EnumPar<void> >
     (AxisNameMod::APC,ep((int)AxisNameMod::APC,"APC","un-documented")));
    m_.insert(pair<AxisNameMod::AxisName,EnumPar<void> >
     (AxisNameMod::SPP,ep((int)AxisNameMod::SPP,"SPP","un-documented")));
    m_.insert(pair<AxisNameMod::AxisName,EnumPar<void> >
     (AxisNameMod::POL,ep((int)AxisNameMod::POL,"POL","un-documented")));
    m_.insert(pair<AxisNameMod::AxisName,EnumPar<void> >
     (AxisNameMod::STO,ep((int)AxisNameMod::STO,"STO","un-documented")));
    m_.insert(pair<AxisNameMod::AxisName,EnumPar<void> >
     (AxisNameMod::HOL,ep((int)AxisNameMod::HOL,"HOL","un-documented")));
    return true;
  }
  static map<AxisNameMod::AxisName,EnumPar<void> > m_;
};
#define _AXISNAME_HH
#endif
