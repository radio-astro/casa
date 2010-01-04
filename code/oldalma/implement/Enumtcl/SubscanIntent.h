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


#if     !defined(_SUBSCANINTENT_H)

#include <CSubscanIntent.h>
#define _SUBSCANINTENT_H
#endif 

#if     !defined(_SUBSCANINTENT_HH)

#include "Enum.hpp"

using namespace SubscanIntentMod;

template<>
 struct enum_set_traits<SubscanIntent> : public enum_set_traiter<SubscanIntent,11,SubscanIntentMod::RASTER> {};

template<>
class enum_map_traits<SubscanIntent,void> : public enum_map_traiter<SubscanIntent,void> {
public:
  static bool   init_;
  static string typeName_;
  static string enumerationDesc_;
  static string order_;
  static string xsdBaseType_;
  static bool   init(){
    EnumPar<void> ep;
    m_.insert(pair<SubscanIntent,EnumPar<void> >
     (SubscanIntentMod::HOLOGRAPHY_RASTER,ep((int)SubscanIntentMod::HOLOGRAPHY_RASTER,"HOLOGRAPHY_RASTER","The scanning subscans in a holography raster map.")));
    m_.insert(pair<SubscanIntent,EnumPar<void> >
     (SubscanIntentMod::HOLOGRAPHY_PHASECAL,ep((int)SubscanIntentMod::HOLOGRAPHY_PHASECAL,"HOLOGRAPHY_PHASECAL","The boresight reference measurement for a holography raster map")));
    m_.insert(pair<SubscanIntent,EnumPar<void> >
     (SubscanIntentMod::UNSPECIFIED,ep((int)SubscanIntentMod::UNSPECIFIED,"UNSPECIFIED","Unspecified")));
    m_.insert(pair<SubscanIntent,EnumPar<void> >
     (SubscanIntentMod::IN_FOCUS,ep((int)SubscanIntentMod::IN_FOCUS,"IN_FOCUS","In Focus measurement")));
    m_.insert(pair<SubscanIntent,EnumPar<void> >
     (SubscanIntentMod::OUT_OF_FOCUS,ep((int)SubscanIntentMod::OUT_OF_FOCUS,"OUT_OF_FOCUS","out of focus measurement")));
    m_.insert(pair<SubscanIntent,EnumPar<void> >
     (SubscanIntentMod::ON_SOURCE,ep((int)SubscanIntentMod::ON_SOURCE,"ON_SOURCE","on-source measurement")));
    m_.insert(pair<SubscanIntent,EnumPar<void> >
     (SubscanIntentMod::OFF_SOURCE,ep((int)SubscanIntentMod::OFF_SOURCE,"OFF_SOURCE","off-source measurement")));
    m_.insert(pair<SubscanIntent,EnumPar<void> >
     (SubscanIntentMod::MIXED_FOCUS,ep((int)SubscanIntentMod::MIXED_FOCUS,"MIXED_FOCUS","Focus measurement, some antennas in focus, some not")));
    m_.insert(pair<SubscanIntent,EnumPar<void> >
     (SubscanIntentMod::MIXED_POINTING,ep((int)SubscanIntentMod::MIXED_POINTING,"MIXED_POINTING","Pointing measurement, some antennas are on -ource, some off-source")));
    m_.insert(pair<SubscanIntent,EnumPar<void> >
     (SubscanIntentMod::REFERENCE,ep((int)SubscanIntentMod::REFERENCE,"REFERENCE","reference measurement")));
    m_.insert(pair<SubscanIntent,EnumPar<void> >
     (SubscanIntentMod::RASTER,ep((int)SubscanIntentMod::RASTER,"RASTER","The scanning subscans in an astronomy raster map.")));
    return true;
  }
  static map<SubscanIntent,EnumPar<void> > m_;
};
#define _SUBSCANINTENT_HH
#endif
