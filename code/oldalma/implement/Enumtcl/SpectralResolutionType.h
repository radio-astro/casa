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


#if     !defined(_SPECTRALRESOLUTIONTYPE_H)

#include <CSpectralResolutionType.h>
#define _SPECTRALRESOLUTIONTYPE_H
#endif 

#if     !defined(_SPECTRALRESOLUTIONTYPE_HH)

#include "Enum.hpp"

using namespace SpectralResolutionTypeMod;

template<>
 struct enum_set_traits<SpectralResolutionType> : public enum_set_traiter<SpectralResolutionType,3,SpectralResolutionTypeMod::FULL_RESOLUTION> {};

template<>
class enum_map_traits<SpectralResolutionType,void> : public enum_map_traiter<SpectralResolutionType,void> {
public:
  static bool   init_;
  static string typeName_;
  static string enumerationDesc_;
  static string order_;
  static string xsdBaseType_;
  static bool   init(){
    EnumPar<void> ep;
    m_.insert(pair<SpectralResolutionType,EnumPar<void> >
     (SpectralResolutionTypeMod::CHANNEL_AVERAGE,ep((int)SpectralResolutionTypeMod::CHANNEL_AVERAGE,"CHANNEL_AVERAGE","")));
    m_.insert(pair<SpectralResolutionType,EnumPar<void> >
     (SpectralResolutionTypeMod::BASEBAND_WIDE,ep((int)SpectralResolutionTypeMod::BASEBAND_WIDE,"BASEBAND_WIDE","")));
    m_.insert(pair<SpectralResolutionType,EnumPar<void> >
     (SpectralResolutionTypeMod::FULL_RESOLUTION,ep((int)SpectralResolutionTypeMod::FULL_RESOLUTION,"FULL_RESOLUTION","")));
    return true;
  }
  static map<SpectralResolutionType,EnumPar<void> > m_;
};
#define _SPECTRALRESOLUTIONTYPE_HH
#endif
