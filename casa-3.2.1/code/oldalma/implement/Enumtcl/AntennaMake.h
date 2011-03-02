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


#if     !defined(_ANTENNAMAKE_H)

#include <CAntennaMake.h>
#define _ANTENNAMAKE_H
#endif 

#if     !defined(_ANTENNAMAKE_HH)

#include "Enum.hpp"

using namespace AntennaMakeMod;

template<>
 struct enum_set_traits<AntennaMake> : public enum_set_traiter<AntennaMake,8,AntennaMakeMod::IRAM_15> {};

template<>
class enum_map_traits<AntennaMake,void> : public enum_map_traiter<AntennaMake,void> {
public:
  static bool   init_;
  static string typeName_;
  static string enumerationDesc_;
  static string order_;
  static string xsdBaseType_;
  static bool   init(){
    EnumPar<void> ep;
    m_.insert(pair<AntennaMake,EnumPar<void> >
     (AntennaMakeMod::AEM_12,ep((int)AntennaMakeMod::AEM_12,"AEM_12","12m AEM antenna")));
    m_.insert(pair<AntennaMake,EnumPar<void> >
     (AntennaMakeMod::MITSUBISHI_7,ep((int)AntennaMakeMod::MITSUBISHI_7,"MITSUBISHI_7","7-m Mitsubishi antenna (ACA)")));
    m_.insert(pair<AntennaMake,EnumPar<void> >
     (AntennaMakeMod::MITSUBISHI_12_A,ep((int)AntennaMakeMod::MITSUBISHI_12_A,"MITSUBISHI_12_A","12-m Mitsubishi antenna (ACA) (refurbished prototype)")));
    m_.insert(pair<AntennaMake,EnumPar<void> >
     (AntennaMakeMod::MITSUBISHI_12_B,ep((int)AntennaMakeMod::MITSUBISHI_12_B,"MITSUBISHI_12_B","12-m Mitsubishi antenna (ACA) (production)")));
    m_.insert(pair<AntennaMake,EnumPar<void> >
     (AntennaMakeMod::VERTEX_12_ATF,ep((int)AntennaMakeMod::VERTEX_12_ATF,"VERTEX_12_ATF","12-m Vertex antenna prototype")));
    m_.insert(pair<AntennaMake,EnumPar<void> >
     (AntennaMakeMod::AEM_12_ATF,ep((int)AntennaMakeMod::AEM_12_ATF,"AEM_12_ATF","12-m AEM  antenna prototype")));
    m_.insert(pair<AntennaMake,EnumPar<void> >
     (AntennaMakeMod::VERTEX_12,ep((int)AntennaMakeMod::VERTEX_12,"VERTEX_12","12-m Vertex antenna")));
    m_.insert(pair<AntennaMake,EnumPar<void> >
     (AntennaMakeMod::IRAM_15,ep((int)AntennaMakeMod::IRAM_15,"IRAM_15","15-m IRAM antenna")));
    return true;
  }
  static map<AntennaMake,EnumPar<void> > m_;
};
#define _ANTENNAMAKE_HH
#endif
