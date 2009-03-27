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


#if     !defined(_SWITCHINGMODE_H)

#include <CSwitchingMode.h>
#define _SWITCHINGMODE_H
#endif 

#if     !defined(_SWITCHINGMODE_HH)

#include "Enum.hpp"

using namespace SwitchingModeMod;

template<>
 struct enum_set_traits<SwitchingMode> : public enum_set_traiter<SwitchingMode,7,SwitchingModeMod::CHOPPER_WHEEL> {};

template<>
class enum_map_traits<SwitchingMode,void> : public enum_map_traiter<SwitchingMode,void> {
public:
  static bool   init_;
  static string typeName_;
  static string enumerationDesc_;
  static string order_;
  static string xsdBaseType_;
  static bool   init(){
    EnumPar<void> ep;
    m_.insert(pair<SwitchingMode,EnumPar<void> >
     (SwitchingModeMod::NO_SWITCHING,ep((int)SwitchingModeMod::NO_SWITCHING,"NO_SWITCHING","No switching")));
    m_.insert(pair<SwitchingMode,EnumPar<void> >
     (SwitchingModeMod::LOAD_SWITCHING,ep((int)SwitchingModeMod::LOAD_SWITCHING,"LOAD_SWITCHING","Receiver beam is switched between sky and load")));
    m_.insert(pair<SwitchingMode,EnumPar<void> >
     (SwitchingModeMod::POSITION_SWITCHING,ep((int)SwitchingModeMod::POSITION_SWITCHING,"POSITION_SWITCHING","Antenna (main reflector) pointing direction  is switched ")));
    m_.insert(pair<SwitchingMode,EnumPar<void> >
     (SwitchingModeMod::PHASE_SWITCHING,ep((int)SwitchingModeMod::PHASE_SWITCHING,"PHASE_SWITCHING","90 degrees phase switching  (switching mode used for sideband separation or rejection with DSB receivers)")));
    m_.insert(pair<SwitchingMode,EnumPar<void> >
     (SwitchingModeMod::FREQUENCY_SWITCHING,ep((int)SwitchingModeMod::FREQUENCY_SWITCHING,"FREQUENCY_SWITCHING","LO frequency is switched (definition context sensitive: fast if cycle shrorter than the integration duration, slow if e.g. step one step per subscan)")));
    m_.insert(pair<SwitchingMode,EnumPar<void> >
     (SwitchingModeMod::NUTATOR_SWITCHING,ep((int)SwitchingModeMod::NUTATOR_SWITCHING,"NUTATOR_SWITCHING","Switching between different directions by nutating the sub-reflector")));
    m_.insert(pair<SwitchingMode,EnumPar<void> >
     (SwitchingModeMod::CHOPPER_WHEEL,ep((int)SwitchingModeMod::CHOPPER_WHEEL,"CHOPPER_WHEEL","Switching using a chopper wheel")));
    return true;
  }
  static map<SwitchingMode,EnumPar<void> > m_;
};
#define _SWITCHINGMODE_HH
#endif
