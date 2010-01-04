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
 struct enum_set_traits<AxisName> : public enum_set_traiter<AxisName,12,AxisNameMod::HOL> {};

template<>
class enum_map_traits<AxisName,void> : public enum_map_traiter<AxisName,void> {
public:
  static bool   init_;
  static string typeName_;
  static string enumerationDesc_;
  static string order_;
  static string xsdBaseType_;
  static bool   init(){
    EnumPar<void> ep;
    m_.insert(pair<AxisName,EnumPar<void> >
     (AxisNameMod::TIM,ep((int)AxisNameMod::TIM,"TIM","Time axis")));
    m_.insert(pair<AxisName,EnumPar<void> >
     (AxisNameMod::BAL,ep((int)AxisNameMod::BAL,"BAL","Baseline number axis")));
    m_.insert(pair<AxisName,EnumPar<void> >
     (AxisNameMod::ANT,ep((int)AxisNameMod::ANT,"ANT","ANtenna number axis")));
    m_.insert(pair<AxisName,EnumPar<void> >
     (AxisNameMod::BAB,ep((int)AxisNameMod::BAB,"BAB","Baseband axis")));
    m_.insert(pair<AxisName,EnumPar<void> >
     (AxisNameMod::SPW,ep((int)AxisNameMod::SPW,"SPW","SPectral window number axis")));
    m_.insert(pair<AxisName,EnumPar<void> >
     (AxisNameMod::SIB,ep((int)AxisNameMod::SIB,"SIB","Sideband axis.")));
    m_.insert(pair<AxisName,EnumPar<void> >
     (AxisNameMod::SUB,ep((int)AxisNameMod::SUB,"SUB","Subband axis")));
    m_.insert(pair<AxisName,EnumPar<void> >
     (AxisNameMod::BIN,ep((int)AxisNameMod::BIN,"BIN","Bin axis")));
    m_.insert(pair<AxisName,EnumPar<void> >
     (AxisNameMod::APC,ep((int)AxisNameMod::APC,"APC","Atmosphere phase correction axis")));
    m_.insert(pair<AxisName,EnumPar<void> >
     (AxisNameMod::SPP,ep((int)AxisNameMod::SPP,"SPP","Spectral data axis")));
    m_.insert(pair<AxisName,EnumPar<void> >
     (AxisNameMod::POL,ep((int)AxisNameMod::POL,"POL","Polarization axes (Stokes parameters)")));
    m_.insert(pair<AxisName,EnumPar<void> >
     (AxisNameMod::HOL,ep((int)AxisNameMod::HOL,"HOL","Holography axis.")));
    return true;
  }
  static map<AxisName,EnumPar<void> > m_;
};
#define _AXISNAME_HH
#endif
