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


#if     !defined(_CORRELATIONMODE_H)

#include <CCorrelationMode.h>
#define _CORRELATIONMODE_H
#endif 

#if     !defined(_CORRELATIONMODE_HH)

#include "Enum.hpp"

using namespace CorrelationModeMod;

template<>
 struct enum_set_traits<CorrelationMode> : public enum_set_traiter<CorrelationMode,3,CorrelationModeMod::CROSS_AND_AUTO> {};

template<>
class enum_map_traits<CorrelationMode,void> : public enum_map_traiter<CorrelationMode,void> {
public:
  static bool   init_;
  static string typeName_;
  static string enumerationDesc_;
  static string order_;
  static string xsdBaseType_;
  static bool   init(){
    EnumPar<void> ep;
    m_.insert(pair<CorrelationMode,EnumPar<void> >
     (CorrelationModeMod::CROSS_ONLY,ep((int)CorrelationModeMod::CROSS_ONLY,"CROSS_ONLY","Cross-correlations only [not for ALMA]")));
    m_.insert(pair<CorrelationMode,EnumPar<void> >
     (CorrelationModeMod::AUTO_ONLY,ep((int)CorrelationModeMod::AUTO_ONLY,"AUTO_ONLY","Auto-correlations only")));
    m_.insert(pair<CorrelationMode,EnumPar<void> >
     (CorrelationModeMod::CROSS_AND_AUTO,ep((int)CorrelationModeMod::CROSS_AND_AUTO,"CROSS_AND_AUTO","Auto-correlations and Cross-correlations")));
    return true;
  }
  static map<CorrelationMode,EnumPar<void> > m_;
};
#define _CORRELATIONMODE_HH
#endif
