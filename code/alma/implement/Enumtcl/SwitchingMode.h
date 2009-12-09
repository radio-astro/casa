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
     (SwitchingModeMod::NO_SWITCHING,ep((int)SwitchingModeMod::NO_SWITCHING,"NO_SWITCHING","un-documented")));
    m_.insert(pair<SwitchingMode,EnumPar<void> >
     (SwitchingModeMod::LOAD_SWITCHING,ep((int)SwitchingModeMod::LOAD_SWITCHING,"LOAD_SWITCHING","un-documented")));
    m_.insert(pair<SwitchingMode,EnumPar<void> >
     (SwitchingModeMod::POSITION_SWITCHING,ep((int)SwitchingModeMod::POSITION_SWITCHING,"POSITION_SWITCHING","un-documented")));
    m_.insert(pair<SwitchingMode,EnumPar<void> >
     (SwitchingModeMod::PHASE_SWITCHING,ep((int)SwitchingModeMod::PHASE_SWITCHING,"PHASE_SWITCHING","un-documented")));
    m_.insert(pair<SwitchingMode,EnumPar<void> >
     (SwitchingModeMod::FREQUENCY_SWITCHING,ep((int)SwitchingModeMod::FREQUENCY_SWITCHING,"FREQUENCY_SWITCHING","un-documented")));
    m_.insert(pair<SwitchingMode,EnumPar<void> >
     (SwitchingModeMod::NUTATOR_SWITCHING,ep((int)SwitchingModeMod::NUTATOR_SWITCHING,"NUTATOR_SWITCHING","un-documented")));
    m_.insert(pair<SwitchingMode,EnumPar<void> >
     (SwitchingModeMod::CHOPPER_WHEEL,ep((int)SwitchingModeMod::CHOPPER_WHEEL,"CHOPPER_WHEEL","un-documented")));
    return true;
  }
  static map<SwitchingMode,EnumPar<void> > m_;
};
#define _SWITCHINGMODE_HH
#endif
