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


#if     !defined(_FREQUENCYREFERENCECODE_H)

#include <CFrequencyReferenceCode.h>
#define _FREQUENCYREFERENCECODE_H
#endif 

#if     !defined(_FREQUENCYREFERENCECODE_HH)

#include "Enum.hpp"

using namespace FrequencyReferenceCodeMod;

template<>
 struct enum_set_traits<FrequencyReferenceCode> : public enum_set_traiter<FrequencyReferenceCode,8,FrequencyReferenceCodeMod::TOPO> {};

template<>
class enum_map_traits<FrequencyReferenceCode,void> : public enum_map_traiter<FrequencyReferenceCode,void> {
public:
  static bool   init_;
  static string typeName_;
  static string enumerationDesc_;
  static string order_;
  static string xsdBaseType_;
  static bool   init(){
    EnumPar<void> ep;
    m_.insert(pair<FrequencyReferenceCode,EnumPar<void> >
     (FrequencyReferenceCodeMod::LABREST,ep((int)FrequencyReferenceCodeMod::LABREST,"LABREST","un-documented")));
    m_.insert(pair<FrequencyReferenceCode,EnumPar<void> >
     (FrequencyReferenceCodeMod::LSRD,ep((int)FrequencyReferenceCodeMod::LSRD,"LSRD","un-documented")));
    m_.insert(pair<FrequencyReferenceCode,EnumPar<void> >
     (FrequencyReferenceCodeMod::LSRK,ep((int)FrequencyReferenceCodeMod::LSRK,"LSRK","un-documented")));
    m_.insert(pair<FrequencyReferenceCode,EnumPar<void> >
     (FrequencyReferenceCodeMod::BARY,ep((int)FrequencyReferenceCodeMod::BARY,"BARY","un-documented")));
    m_.insert(pair<FrequencyReferenceCode,EnumPar<void> >
     (FrequencyReferenceCodeMod::REST,ep((int)FrequencyReferenceCodeMod::REST,"REST","un-documented")));
    m_.insert(pair<FrequencyReferenceCode,EnumPar<void> >
     (FrequencyReferenceCodeMod::GEO,ep((int)FrequencyReferenceCodeMod::GEO,"GEO","un-documented")));
    m_.insert(pair<FrequencyReferenceCode,EnumPar<void> >
     (FrequencyReferenceCodeMod::GALACTO,ep((int)FrequencyReferenceCodeMod::GALACTO,"GALACTO","un-documented")));
    m_.insert(pair<FrequencyReferenceCode,EnumPar<void> >
     (FrequencyReferenceCodeMod::TOPO,ep((int)FrequencyReferenceCodeMod::TOPO,"TOPO","un-documented")));
    return true;
  }
  static map<FrequencyReferenceCode,EnumPar<void> > m_;
};
#define _FREQUENCYREFERENCECODE_HH
#endif
