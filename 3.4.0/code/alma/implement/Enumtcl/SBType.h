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


#if     !defined(_SBTYPE_H)

#include <CSBType.h>
#define _SBTYPE_H
#endif 

#if     !defined(_SBTYPE_HH)

#include "Enum.hpp"

using namespace SBTypeMod;

template<>
 struct enum_set_traits<SBTypeMod::SBType> : public enum_set_traiter<SBTypeMod::SBType,3,SBTypeMod::EXPERT> {};

template<>
class enum_map_traits<SBTypeMod::SBType,void> : public enum_map_traiter<SBTypeMod::SBType,void> {
public:
  static bool   init_;
  static string typeName_;
  static string enumerationDesc_;
  static string order_;
  static string xsdBaseType_;
  static bool   init(){
    EnumPar<void> ep;
    m_.insert(pair<SBTypeMod::SBType,EnumPar<void> >
     (SBTypeMod::OBSERVATORY,ep((int)SBTypeMod::OBSERVATORY,"OBSERVATORY","un-documented")));
    m_.insert(pair<SBTypeMod::SBType,EnumPar<void> >
     (SBTypeMod::OBSERVER,ep((int)SBTypeMod::OBSERVER,"OBSERVER","un-documented")));
    m_.insert(pair<SBTypeMod::SBType,EnumPar<void> >
     (SBTypeMod::EXPERT,ep((int)SBTypeMod::EXPERT,"EXPERT","un-documented")));
    return true;
  }
  static map<SBTypeMod::SBType,EnumPar<void> > m_;
};
#define _SBTYPE_HH
#endif
