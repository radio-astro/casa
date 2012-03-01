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


#if     !defined(_SIDEBANDPROCESSINGMODE_H)

#include <CSidebandProcessingMode.h>
#define _SIDEBANDPROCESSINGMODE_H
#endif 

#if     !defined(_SIDEBANDPROCESSINGMODE_HH)

#include "Enum.hpp"

using namespace SidebandProcessingModeMod;

template<>
 struct enum_set_traits<SidebandProcessingModeMod::SidebandProcessingMode> : public enum_set_traiter<SidebandProcessingModeMod::SidebandProcessingMode,5,SidebandProcessingModeMod::FREQUENCY_OFFSET_REJECTION> {};

template<>
class enum_map_traits<SidebandProcessingModeMod::SidebandProcessingMode,void> : public enum_map_traiter<SidebandProcessingModeMod::SidebandProcessingMode,void> {
public:
  static bool   init_;
  static string typeName_;
  static string enumerationDesc_;
  static string order_;
  static string xsdBaseType_;
  static bool   init(){
    EnumPar<void> ep;
    m_.insert(pair<SidebandProcessingModeMod::SidebandProcessingMode,EnumPar<void> >
     (SidebandProcessingModeMod::NONE,ep((int)SidebandProcessingModeMod::NONE,"NONE","un-documented")));
    m_.insert(pair<SidebandProcessingModeMod::SidebandProcessingMode,EnumPar<void> >
     (SidebandProcessingModeMod::PHASE_SWITCH_SEPARATION,ep((int)SidebandProcessingModeMod::PHASE_SWITCH_SEPARATION,"PHASE_SWITCH_SEPARATION","un-documented")));
    m_.insert(pair<SidebandProcessingModeMod::SidebandProcessingMode,EnumPar<void> >
     (SidebandProcessingModeMod::FREQUENCY_OFFSET_SEPARATION,ep((int)SidebandProcessingModeMod::FREQUENCY_OFFSET_SEPARATION,"FREQUENCY_OFFSET_SEPARATION","un-documented")));
    m_.insert(pair<SidebandProcessingModeMod::SidebandProcessingMode,EnumPar<void> >
     (SidebandProcessingModeMod::PHASE_SWITCH_REJECTION,ep((int)SidebandProcessingModeMod::PHASE_SWITCH_REJECTION,"PHASE_SWITCH_REJECTION","un-documented")));
    m_.insert(pair<SidebandProcessingModeMod::SidebandProcessingMode,EnumPar<void> >
     (SidebandProcessingModeMod::FREQUENCY_OFFSET_REJECTION,ep((int)SidebandProcessingModeMod::FREQUENCY_OFFSET_REJECTION,"FREQUENCY_OFFSET_REJECTION","un-documented")));
    return true;
  }
  static map<SidebandProcessingModeMod::SidebandProcessingMode,EnumPar<void> > m_;
};
#define _SIDEBANDPROCESSINGMODE_HH
#endif
