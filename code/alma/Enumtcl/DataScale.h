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


#if     !defined(_DATASCALE_H)

#include <CDataScale.h>
#define _DATASCALE_H
#endif 

#if     !defined(_DATASCALE_HH)

#include "Enum.hpp"

using namespace DataScaleMod;

template<>
 struct enum_set_traits<DataScaleMod::DataScale> : public enum_set_traiter<DataScaleMod::DataScale,4,DataScaleMod::CORRELATION_COEFFICIENT> {};

template<>
class enum_map_traits<DataScaleMod::DataScale,void> : public enum_map_traiter<DataScaleMod::DataScale,void> {
public:
  static bool   init_;
  static string typeName_;
  static string enumerationDesc_;
  static string order_;
  static string xsdBaseType_;
  static bool   init(){
    EnumPar<void> ep;
    m_.insert(pair<DataScaleMod::DataScale,EnumPar<void> >
     (DataScaleMod::K,ep((int)DataScaleMod::K,"K","un-documented")));
    m_.insert(pair<DataScaleMod::DataScale,EnumPar<void> >
     (DataScaleMod::JY,ep((int)DataScaleMod::JY,"JY","un-documented")));
    m_.insert(pair<DataScaleMod::DataScale,EnumPar<void> >
     (DataScaleMod::CORRELATION,ep((int)DataScaleMod::CORRELATION,"CORRELATION","un-documented")));
    m_.insert(pair<DataScaleMod::DataScale,EnumPar<void> >
     (DataScaleMod::CORRELATION_COEFFICIENT,ep((int)DataScaleMod::CORRELATION_COEFFICIENT,"CORRELATION_COEFFICIENT","un-documented")));
    return true;
  }
  static map<DataScaleMod::DataScale,EnumPar<void> > m_;
};
#define _DATASCALE_HH
#endif
