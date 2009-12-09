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


#if     !defined(_PROCESSORSUBTYPE_H)

#include <CProcessorSubType.h>
#define _PROCESSORSUBTYPE_H
#endif 

#if     !defined(_PROCESSORSUBTYPE_HH)

#include "Enum.hpp"

using namespace ProcessorSubTypeMod;

template<>
 struct enum_set_traits<ProcessorSubType> : public enum_set_traiter<ProcessorSubType,3,ProcessorSubTypeMod::HOLOGRAPHY> {};

template<>
class enum_map_traits<ProcessorSubType,void> : public enum_map_traiter<ProcessorSubType,void> {
public:
  static bool   init_;
  static string typeName_;
  static string enumerationDesc_;
  static string order_;
  static string xsdBaseType_;
  static bool   init(){
    EnumPar<void> ep;
    m_.insert(pair<ProcessorSubType,EnumPar<void> >
     (ProcessorSubTypeMod::ALMA_CORRELATOR_MODE,ep((int)ProcessorSubTypeMod::ALMA_CORRELATOR_MODE,"ALMA_CORRELATOR_MODE","ALMA_CORRELATOR_MODE")));
    m_.insert(pair<ProcessorSubType,EnumPar<void> >
     (ProcessorSubTypeMod::SQUARE_LAW_DETECTOR,ep((int)ProcessorSubTypeMod::SQUARE_LAW_DETECTOR,"SQUARE_LAW_DETECTOR","SQUARE_LAW_DETECTOR")));
    m_.insert(pair<ProcessorSubType,EnumPar<void> >
     (ProcessorSubTypeMod::HOLOGRAPHY,ep((int)ProcessorSubTypeMod::HOLOGRAPHY,"HOLOGRAPHY","HOLOGRAPHY")));
    return true;
  }
  static map<ProcessorSubType,EnumPar<void> > m_;
};
#define _PROCESSORSUBTYPE_HH
#endif
