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


#if     !defined(_SUBSCANINTENT_H)

#include <CSubscanIntent.h>
#define _SUBSCANINTENT_H
#endif 

#if     !defined(_SUBSCANINTENT_HH)

#include "Enum.hpp"

using namespace SubscanIntentMod;

template<>
 struct enum_set_traits<SubscanIntent> : public enum_set_traiter<SubscanIntent,9,SubscanIntentMod::UNSPECIFIED> {};

template<>
class enum_map_traits<SubscanIntent,void> : public enum_map_traiter<SubscanIntent,void> {
public:
  static bool   init_;
  static string typeName_;
  static string enumerationDesc_;
  static string order_;
  static string xsdBaseType_;
  static bool   init(){
    EnumPar<void> ep;
    m_.insert(pair<SubscanIntent,EnumPar<void> >
     (SubscanIntentMod::ON_SOURCE,ep((int)SubscanIntentMod::ON_SOURCE,"ON_SOURCE","un-documented")));
    m_.insert(pair<SubscanIntent,EnumPar<void> >
     (SubscanIntentMod::OFF_SOURCE,ep((int)SubscanIntentMod::OFF_SOURCE,"OFF_SOURCE","un-documented")));
    m_.insert(pair<SubscanIntent,EnumPar<void> >
     (SubscanIntentMod::MIXED,ep((int)SubscanIntentMod::MIXED,"MIXED","un-documented")));
    m_.insert(pair<SubscanIntent,EnumPar<void> >
     (SubscanIntentMod::REFERENCE,ep((int)SubscanIntentMod::REFERENCE,"REFERENCE","un-documented")));
    m_.insert(pair<SubscanIntent,EnumPar<void> >
     (SubscanIntentMod::SCANNING,ep((int)SubscanIntentMod::SCANNING,"SCANNING","un-documented")));
    m_.insert(pair<SubscanIntent,EnumPar<void> >
     (SubscanIntentMod::HOT,ep((int)SubscanIntentMod::HOT,"HOT","un-documented")));
    m_.insert(pair<SubscanIntent,EnumPar<void> >
     (SubscanIntentMod::AMBIENT,ep((int)SubscanIntentMod::AMBIENT,"AMBIENT","un-documented")));
    m_.insert(pair<SubscanIntent,EnumPar<void> >
     (SubscanIntentMod::TEST,ep((int)SubscanIntentMod::TEST,"TEST","un-documented")));
    m_.insert(pair<SubscanIntent,EnumPar<void> >
     (SubscanIntentMod::UNSPECIFIED,ep((int)SubscanIntentMod::UNSPECIFIED,"UNSPECIFIED","un-documented")));
    return true;
  }
  static map<SubscanIntent,EnumPar<void> > m_;
};
#define _SUBSCANINTENT_HH
#endif
