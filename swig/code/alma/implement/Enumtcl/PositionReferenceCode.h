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


#if     !defined(_POSITIONREFERENCECODE_H)

#include <CPositionReferenceCode.h>
#define _POSITIONREFERENCECODE_H
#endif 

#if     !defined(_POSITIONREFERENCECODE_HH)

#include "Enum.hpp"

using namespace PositionReferenceCodeMod;

template<>
 struct enum_set_traits<PositionReferenceCode> : public enum_set_traiter<PositionReferenceCode,6,PositionReferenceCodeMod::REFLECTOR> {};

template<>
class enum_map_traits<PositionReferenceCode,void> : public enum_map_traiter<PositionReferenceCode,void> {
public:
  static bool   init_;
  static string typeName_;
  static string enumerationDesc_;
  static string order_;
  static string xsdBaseType_;
  static bool   init(){
    EnumPar<void> ep;
    m_.insert(pair<PositionReferenceCode,EnumPar<void> >
     (PositionReferenceCodeMod::ITRF,ep((int)PositionReferenceCodeMod::ITRF,"ITRF","un-documented")));
    m_.insert(pair<PositionReferenceCode,EnumPar<void> >
     (PositionReferenceCodeMod::WGS84,ep((int)PositionReferenceCodeMod::WGS84,"WGS84","un-documented")));
    m_.insert(pair<PositionReferenceCode,EnumPar<void> >
     (PositionReferenceCodeMod::SITE,ep((int)PositionReferenceCodeMod::SITE,"SITE","un-documented")));
    m_.insert(pair<PositionReferenceCode,EnumPar<void> >
     (PositionReferenceCodeMod::STATION,ep((int)PositionReferenceCodeMod::STATION,"STATION","un-documented")));
    m_.insert(pair<PositionReferenceCode,EnumPar<void> >
     (PositionReferenceCodeMod::YOKE,ep((int)PositionReferenceCodeMod::YOKE,"YOKE","un-documented")));
    m_.insert(pair<PositionReferenceCode,EnumPar<void> >
     (PositionReferenceCodeMod::REFLECTOR,ep((int)PositionReferenceCodeMod::REFLECTOR,"REFLECTOR","un-documented")));
    return true;
  }
  static map<PositionReferenceCode,EnumPar<void> > m_;
};
#define _POSITIONREFERENCECODE_HH
#endif
