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


#if     !defined(_RECEIVERSIDEBAND_H)

#include <CReceiverSideband.h>
#define _RECEIVERSIDEBAND_H
#endif 

#if     !defined(_RECEIVERSIDEBAND_HH)

#include "Enum.hpp"

using namespace ReceiverSidebandMod;

template<>
 struct enum_set_traits<ReceiverSideband> : public enum_set_traiter<ReceiverSideband,4,ReceiverSidebandMod::TSB> {};

template<>
class enum_map_traits<ReceiverSideband,void> : public enum_map_traiter<ReceiverSideband,void> {
public:
  static bool   init_;
  static string typeName_;
  static string enumerationDesc_;
  static string order_;
  static string xsdBaseType_;
  static bool   init(){
    EnumPar<void> ep;
    m_.insert(pair<ReceiverSideband,EnumPar<void> >
     (ReceiverSidebandMod::NOSB,ep((int)ReceiverSidebandMod::NOSB,"NOSB","Direct output signal (no frequency conversion)")));
    m_.insert(pair<ReceiverSideband,EnumPar<void> >
     (ReceiverSidebandMod::DSB,ep((int)ReceiverSidebandMod::DSB,"DSB","DOuble side-band ouput")));
    m_.insert(pair<ReceiverSideband,EnumPar<void> >
     (ReceiverSidebandMod::SSB,ep((int)ReceiverSidebandMod::SSB,"SSB","Single-side band receiver.")));
    m_.insert(pair<ReceiverSideband,EnumPar<void> >
     (ReceiverSidebandMod::TSB,ep((int)ReceiverSidebandMod::TSB,"TSB","Eeceiver with dual-output")));
    return true;
  }
  static map<ReceiverSideband,EnumPar<void> > m_;
};
#define _RECEIVERSIDEBAND_HH
#endif
