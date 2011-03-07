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


#if     !defined(_CORRELATIONBIT_H)

#include <CCorrelationBit.h>
#define _CORRELATIONBIT_H
#endif 

#if     !defined(_CORRELATIONBIT_HH)

#include "Enum.hpp"

using namespace CorrelationBitMod;

template<>
 struct enum_set_traits<CorrelationBit> : public enum_set_traiter<CorrelationBit,3,CorrelationBitMod::BITS_4x4> {};

template<>
class enum_map_traits<CorrelationBit,void> : public enum_map_traiter<CorrelationBit,void> {
public:
  static bool   init_;
  static string typeName_;
  static string enumerationDesc_;
  static string order_;
  static string xsdBaseType_;
  static bool   init(){
    EnumPar<void> ep;
    m_.insert(pair<CorrelationBit,EnumPar<void> >
     (CorrelationBitMod::BITS_2x2,ep((int)CorrelationBitMod::BITS_2x2,"BITS_2x2","two bit correlation")));
    m_.insert(pair<CorrelationBit,EnumPar<void> >
     (CorrelationBitMod::BITS_3x3,ep((int)CorrelationBitMod::BITS_3x3,"BITS_3x3"," three bit correlation")));
    m_.insert(pair<CorrelationBit,EnumPar<void> >
     (CorrelationBitMod::BITS_4x4,ep((int)CorrelationBitMod::BITS_4x4,"BITS_4x4","four bit correlation")));
    return true;
  }
  static map<CorrelationBit,EnumPar<void> > m_;
};
#define _CORRELATIONBIT_HH
#endif
