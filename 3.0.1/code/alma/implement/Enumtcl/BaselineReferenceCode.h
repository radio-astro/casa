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


#if     !defined(_BASELINEREFERENCECODE_H)

#include <CBaselineReferenceCode.h>
#define _BASELINEREFERENCECODE_H
#endif 

#if     !defined(_BASELINEREFERENCECODE_HH)

#include "Enum.hpp"

using namespace BaselineReferenceCodeMod;

template<>
 struct enum_set_traits<BaselineReferenceCode> : public enum_set_traiter<BaselineReferenceCode,28,BaselineReferenceCodeMod::ITRF> {};

template<>
class enum_map_traits<BaselineReferenceCode,void> : public enum_map_traiter<BaselineReferenceCode,void> {
public:
  static bool   init_;
  static string typeName_;
  static string enumerationDesc_;
  static string order_;
  static string xsdBaseType_;
  static bool   init(){
    EnumPar<void> ep;
    m_.insert(pair<BaselineReferenceCode,EnumPar<void> >
     (BaselineReferenceCodeMod::J2000,ep((int)BaselineReferenceCodeMod::J2000,"J2000","un-documented")));
    m_.insert(pair<BaselineReferenceCode,EnumPar<void> >
     (BaselineReferenceCodeMod::B1950,ep((int)BaselineReferenceCodeMod::B1950,"B1950","un-documented")));
    m_.insert(pair<BaselineReferenceCode,EnumPar<void> >
     (BaselineReferenceCodeMod::GALACTIC,ep((int)BaselineReferenceCodeMod::GALACTIC,"GALACTIC","un-documented")));
    m_.insert(pair<BaselineReferenceCode,EnumPar<void> >
     (BaselineReferenceCodeMod::SUPERGAL,ep((int)BaselineReferenceCodeMod::SUPERGAL,"SUPERGAL","un-documented")));
    m_.insert(pair<BaselineReferenceCode,EnumPar<void> >
     (BaselineReferenceCodeMod::ECLIPTIC,ep((int)BaselineReferenceCodeMod::ECLIPTIC,"ECLIPTIC","un-documented")));
    m_.insert(pair<BaselineReferenceCode,EnumPar<void> >
     (BaselineReferenceCodeMod::JMEAN,ep((int)BaselineReferenceCodeMod::JMEAN,"JMEAN","un-documented")));
    m_.insert(pair<BaselineReferenceCode,EnumPar<void> >
     (BaselineReferenceCodeMod::JTRUE,ep((int)BaselineReferenceCodeMod::JTRUE,"JTRUE","un-documented")));
    m_.insert(pair<BaselineReferenceCode,EnumPar<void> >
     (BaselineReferenceCodeMod::APP,ep((int)BaselineReferenceCodeMod::APP,"APP","un-documented")));
    m_.insert(pair<BaselineReferenceCode,EnumPar<void> >
     (BaselineReferenceCodeMod::BMEAN,ep((int)BaselineReferenceCodeMod::BMEAN,"BMEAN","un-documented")));
    m_.insert(pair<BaselineReferenceCode,EnumPar<void> >
     (BaselineReferenceCodeMod::BTRUE,ep((int)BaselineReferenceCodeMod::BTRUE,"BTRUE","un-documented")));
    m_.insert(pair<BaselineReferenceCode,EnumPar<void> >
     (BaselineReferenceCodeMod::JNAT,ep((int)BaselineReferenceCodeMod::JNAT,"JNAT","un-documented")));
    m_.insert(pair<BaselineReferenceCode,EnumPar<void> >
     (BaselineReferenceCodeMod::MECLIPTIC,ep((int)BaselineReferenceCodeMod::MECLIPTIC,"MECLIPTIC","un-documented")));
    m_.insert(pair<BaselineReferenceCode,EnumPar<void> >
     (BaselineReferenceCodeMod::TECLIPTIC,ep((int)BaselineReferenceCodeMod::TECLIPTIC,"TECLIPTIC","un-documented")));
    m_.insert(pair<BaselineReferenceCode,EnumPar<void> >
     (BaselineReferenceCodeMod::TOPO,ep((int)BaselineReferenceCodeMod::TOPO,"TOPO","un-documented")));
    m_.insert(pair<BaselineReferenceCode,EnumPar<void> >
     (BaselineReferenceCodeMod::MERCURY,ep((int)BaselineReferenceCodeMod::MERCURY,"MERCURY","un-documented")));
    m_.insert(pair<BaselineReferenceCode,EnumPar<void> >
     (BaselineReferenceCodeMod::VENUS,ep((int)BaselineReferenceCodeMod::VENUS,"VENUS","un-documented")));
    m_.insert(pair<BaselineReferenceCode,EnumPar<void> >
     (BaselineReferenceCodeMod::MARS,ep((int)BaselineReferenceCodeMod::MARS,"MARS","un-documented")));
    m_.insert(pair<BaselineReferenceCode,EnumPar<void> >
     (BaselineReferenceCodeMod::JUPITER,ep((int)BaselineReferenceCodeMod::JUPITER,"JUPITER","un-documented")));
    m_.insert(pair<BaselineReferenceCode,EnumPar<void> >
     (BaselineReferenceCodeMod::SATURN,ep((int)BaselineReferenceCodeMod::SATURN,"SATURN","un-documented")));
    m_.insert(pair<BaselineReferenceCode,EnumPar<void> >
     (BaselineReferenceCodeMod::NEPTUN,ep((int)BaselineReferenceCodeMod::NEPTUN,"NEPTUN","un-documented")));
    m_.insert(pair<BaselineReferenceCode,EnumPar<void> >
     (BaselineReferenceCodeMod::SUN,ep((int)BaselineReferenceCodeMod::SUN,"SUN","un-documented")));
    m_.insert(pair<BaselineReferenceCode,EnumPar<void> >
     (BaselineReferenceCodeMod::MOON,ep((int)BaselineReferenceCodeMod::MOON,"MOON","un-documented")));
    m_.insert(pair<BaselineReferenceCode,EnumPar<void> >
     (BaselineReferenceCodeMod::HADEC,ep((int)BaselineReferenceCodeMod::HADEC,"HADEC","un-documented")));
    m_.insert(pair<BaselineReferenceCode,EnumPar<void> >
     (BaselineReferenceCodeMod::AZEL,ep((int)BaselineReferenceCodeMod::AZEL,"AZEL","un-documented")));
    m_.insert(pair<BaselineReferenceCode,EnumPar<void> >
     (BaselineReferenceCodeMod::AZELGEO,ep((int)BaselineReferenceCodeMod::AZELGEO,"AZELGEO","un-documented")));
    m_.insert(pair<BaselineReferenceCode,EnumPar<void> >
     (BaselineReferenceCodeMod::AZELSW,ep((int)BaselineReferenceCodeMod::AZELSW,"AZELSW","un-documented")));
    m_.insert(pair<BaselineReferenceCode,EnumPar<void> >
     (BaselineReferenceCodeMod::AZELNE,ep((int)BaselineReferenceCodeMod::AZELNE,"AZELNE","un-documented")));
    m_.insert(pair<BaselineReferenceCode,EnumPar<void> >
     (BaselineReferenceCodeMod::ITRF,ep((int)BaselineReferenceCodeMod::ITRF,"ITRF","un-documented")));
    return true;
  }
  static map<BaselineReferenceCode,EnumPar<void> > m_;
};
#define _BASELINEREFERENCECODE_HH
#endif
