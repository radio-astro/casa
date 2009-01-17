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


#if     !defined(_NETSIDEBAND_H)

#include <CNetSideband.h>
#define _NETSIDEBAND_H
#endif 

#if     !defined(_NETSIDEBAND_HH)

#include "Enum.hpp"

using namespace NetSidebandMod;

template<>
 struct enum_set_traits<NetSideband> : public enum_set_traiter<NetSideband,4,NetSidebandMod::DSB> {};

template<>
class enum_map_traits<NetSideband,void> : public enum_map_traiter<NetSideband,void> {
public:
  static bool   init_;
  static string typeName_;
  static string enumerationDesc_;
  static string order_;
  static string xsdBaseType_;
  static bool   init(){
    EnumPar<void> ep;
    m_.insert(pair<NetSideband,EnumPar<void> >
     (NetSidebandMod::NOSB,ep((int)NetSidebandMod::NOSB,"NOSB","No side band (no frequency conversion)")));
    m_.insert(pair<NetSideband,EnumPar<void> >
     (NetSidebandMod::LSB,ep((int)NetSidebandMod::LSB,"LSB","Lower side band")));
    m_.insert(pair<NetSideband,EnumPar<void> >
     (NetSidebandMod::USB,ep((int)NetSidebandMod::USB,"USB","Upper side band")));
    m_.insert(pair<NetSideband,EnumPar<void> >
     (NetSidebandMod::DSB,ep((int)NetSidebandMod::DSB,"DSB","Double side band")));
    return true;
  }
  static map<NetSideband,EnumPar<void> > m_;
};
#define _NETSIDEBAND_HH
#endif
