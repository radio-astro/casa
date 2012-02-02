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


#if     !defined(_ACCUMMODE_H)

#include <CAccumMode.h>
#define _ACCUMMODE_H
#endif 

#if     !defined(_ACCUMMODE_HH)

#include "Enum.hpp"

using namespace AccumModeMod;

template<>
 struct enum_set_traits<AccumMode> : public enum_set_traiter<AccumMode,2,AccumModeMod::ALMA_NORMAL> {};

template<>
class enum_map_traits<AccumMode,void> : public enum_map_traiter<AccumMode,void> {
public:
  static bool   init_;
  static string typeName_;
  static string enumerationDesc_;
  static string order_;
  static string xsdBaseType_;
  static bool   init(){
    EnumPar<void> ep;
    m_.insert(pair<AccumMode,EnumPar<void> >
     (AccumModeMod::ALMA_FAST,ep((int)AccumModeMod::ALMA_FAST,"ALMA_FAST","1 ms dump time, available only for autocorrelation")));
    m_.insert(pair<AccumMode,EnumPar<void> >
     (AccumModeMod::ALMA_NORMAL,ep((int)AccumModeMod::ALMA_NORMAL,"ALMA_NORMAL","16ms dump time, available for both autocorrelation and cross-orrelation")));
    return true;
  }
  static map<AccumMode,EnumPar<void> > m_;
};
#define _ACCUMMODE_HH
#endif
