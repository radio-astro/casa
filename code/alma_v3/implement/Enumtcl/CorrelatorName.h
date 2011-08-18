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


#if     !defined(_CORRELATORNAME_H)

#include <CCorrelatorName.h>
#define _CORRELATORNAME_H
#endif 

#if     !defined(_CORRELATORNAME_HH)

#include "Enum.hpp"

using namespace CorrelatorNameMod;

template<>
 struct enum_set_traits<CorrelatorNameMod::CorrelatorName> : public enum_set_traiter<CorrelatorNameMod::CorrelatorName,10,CorrelatorNameMod::NRAO_WIDAR> {};

template<>
class enum_map_traits<CorrelatorNameMod::CorrelatorName,void> : public enum_map_traiter<CorrelatorNameMod::CorrelatorName,void> {
public:
  static bool   init_;
  static string typeName_;
  static string enumerationDesc_;
  static string order_;
  static string xsdBaseType_;
  static bool   init(){
    EnumPar<void> ep;
    m_.insert(pair<CorrelatorNameMod::CorrelatorName,EnumPar<void> >
     (CorrelatorNameMod::ALMA_ACA,ep((int)CorrelatorNameMod::ALMA_ACA,"ALMA_ACA","un-documented")));
    m_.insert(pair<CorrelatorNameMod::CorrelatorName,EnumPar<void> >
     (CorrelatorNameMod::ALMA_BASELINE,ep((int)CorrelatorNameMod::ALMA_BASELINE,"ALMA_BASELINE","un-documented")));
    m_.insert(pair<CorrelatorNameMod::CorrelatorName,EnumPar<void> >
     (CorrelatorNameMod::ALMA_BASELINE_ATF,ep((int)CorrelatorNameMod::ALMA_BASELINE_ATF,"ALMA_BASELINE_ATF","un-documented")));
    m_.insert(pair<CorrelatorNameMod::CorrelatorName,EnumPar<void> >
     (CorrelatorNameMod::ALMA_BASELINE_PROTO_OSF,ep((int)CorrelatorNameMod::ALMA_BASELINE_PROTO_OSF,"ALMA_BASELINE_PROTO_OSF","un-documented")));
    m_.insert(pair<CorrelatorNameMod::CorrelatorName,EnumPar<void> >
     (CorrelatorNameMod::HERSCHEL,ep((int)CorrelatorNameMod::HERSCHEL,"HERSCHEL","un-documented")));
    m_.insert(pair<CorrelatorNameMod::CorrelatorName,EnumPar<void> >
     (CorrelatorNameMod::IRAM_PDB,ep((int)CorrelatorNameMod::IRAM_PDB,"IRAM_PDB","un-documented")));
    m_.insert(pair<CorrelatorNameMod::CorrelatorName,EnumPar<void> >
     (CorrelatorNameMod::IRAM_30M_VESPA,ep((int)CorrelatorNameMod::IRAM_30M_VESPA,"IRAM_30M_VESPA","un-documented")));
    m_.insert(pair<CorrelatorNameMod::CorrelatorName,EnumPar<void> >
     (CorrelatorNameMod::IRAM_WILMA,ep((int)CorrelatorNameMod::IRAM_WILMA,"IRAM_WILMA","un-documented")));
    m_.insert(pair<CorrelatorNameMod::CorrelatorName,EnumPar<void> >
     (CorrelatorNameMod::NRAO_VLA,ep((int)CorrelatorNameMod::NRAO_VLA,"NRAO_VLA","un-documented")));
    m_.insert(pair<CorrelatorNameMod::CorrelatorName,EnumPar<void> >
     (CorrelatorNameMod::NRAO_WIDAR,ep((int)CorrelatorNameMod::NRAO_WIDAR,"NRAO_WIDAR","un-documented")));
    return true;
  }
  static map<CorrelatorNameMod::CorrelatorName,EnumPar<void> > m_;
};
#define _CORRELATORNAME_HH
#endif
