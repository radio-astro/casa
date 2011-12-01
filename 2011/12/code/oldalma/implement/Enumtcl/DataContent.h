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


#if     !defined(_DATACONTENT_H)

#include <CDataContent.h>
#define _DATACONTENT_H
#endif 

#if     !defined(_DATACONTENT_HH)

#include "Enum.hpp"

using namespace DataContentMod;

template<>
 struct enum_set_traits<DataContent> : public enum_set_traiter<DataContent,7,DataContentMod::FLAGS> {};

template<>
class enum_map_traits<DataContent,void> : public enum_map_traiter<DataContent,void> {
public:
  static bool   init_;
  static string typeName_;
  static string enumerationDesc_;
  static string order_;
  static string xsdBaseType_;
  static bool   init(){
    EnumPar<void> ep;
    m_.insert(pair<DataContent,EnumPar<void> >
     (DataContentMod::CROSS_DATA,ep((int)DataContentMod::CROSS_DATA,"CROSS_DATA","Cross-correlation data")));
    m_.insert(pair<DataContent,EnumPar<void> >
     (DataContentMod::AUTO_DATA,ep((int)DataContentMod::AUTO_DATA,"AUTO_DATA","Auto-correlation data")));
    m_.insert(pair<DataContent,EnumPar<void> >
     (DataContentMod::ZERO_LAGS,ep((int)DataContentMod::ZERO_LAGS,"ZERO_LAGS","Zero-lag data")));
    m_.insert(pair<DataContent,EnumPar<void> >
     (DataContentMod::ACTUAL_TIMES,ep((int)DataContentMod::ACTUAL_TIMES,"ACTUAL_TIMES",":Actual times (mid points of integrations)")));
    m_.insert(pair<DataContent,EnumPar<void> >
     (DataContentMod::ACTUAL_DURATIONS,ep((int)DataContentMod::ACTUAL_DURATIONS,"ACTUAL_DURATIONS","Actual duration of integrations")));
    m_.insert(pair<DataContent,EnumPar<void> >
     (DataContentMod::WEIGHTS,ep((int)DataContentMod::WEIGHTS,"WEIGHTS","Weights")));
    m_.insert(pair<DataContent,EnumPar<void> >
     (DataContentMod::FLAGS,ep((int)DataContentMod::FLAGS,"FLAGS","Baseband based flags")));
    return true;
  }
  static map<DataContent,EnumPar<void> > m_;
};
#define _DATACONTENT_HH
#endif
