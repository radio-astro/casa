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
 struct enum_set_traits<NetSidebandMod::NetSideband> : public enum_set_traiter<NetSidebandMod::NetSideband,4,NetSidebandMod::DSB> {};

template<>
class enum_map_traits<NetSidebandMod::NetSideband,void> : public enum_map_traiter<NetSidebandMod::NetSideband,void> {
public:
  static bool   init_;
  static string typeName_;
  static string enumerationDesc_;
  static string order_;
  static string xsdBaseType_;
  static bool   init(){
    EnumPar<void> ep;
    m_.insert(pair<NetSidebandMod::NetSideband,EnumPar<void> >
     (NetSidebandMod::NOSB,ep((int)NetSidebandMod::NOSB,"NOSB","un-documented")));
    m_.insert(pair<NetSidebandMod::NetSideband,EnumPar<void> >
     (NetSidebandMod::LSB,ep((int)NetSidebandMod::LSB,"LSB","un-documented")));
    m_.insert(pair<NetSidebandMod::NetSideband,EnumPar<void> >
     (NetSidebandMod::USB,ep((int)NetSidebandMod::USB,"USB","un-documented")));
    m_.insert(pair<NetSidebandMod::NetSideband,EnumPar<void> >
     (NetSidebandMod::DSB,ep((int)NetSidebandMod::DSB,"DSB","un-documented")));
    return true;
  }
  static map<NetSidebandMod::NetSideband,EnumPar<void> > m_;
};
#define _NETSIDEBAND_HH
#endif
