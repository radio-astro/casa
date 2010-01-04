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


#if     !defined(_DETECTORBANDTYPE_H)

#include <CDetectorBandType.h>
#define _DETECTORBANDTYPE_H
#endif 

#if     !defined(_DETECTORBANDTYPE_HH)

#include "Enum.hpp"

using namespace DetectorBandTypeMod;

template<>
 struct enum_set_traits<DetectorBandType> : public enum_set_traiter<DetectorBandType,4,DetectorBandTypeMod::SUBBAND> {};

template<>
class enum_map_traits<DetectorBandType,void> : public enum_map_traiter<DetectorBandType,void> {
public:
  static bool   init_;
  static string typeName_;
  static string enumerationDesc_;
  static string order_;
  static string xsdBaseType_;
  static bool   init(){
    EnumPar<void> ep;
    m_.insert(pair<DetectorBandType,EnumPar<void> >
     (DetectorBandTypeMod::BASEBAND,ep((int)DetectorBandTypeMod::BASEBAND,"BASEBAND","un-documented")));
    m_.insert(pair<DetectorBandType,EnumPar<void> >
     (DetectorBandTypeMod::DOWN_CONVERTER,ep((int)DetectorBandTypeMod::DOWN_CONVERTER,"DOWN_CONVERTER","un-documented")));
    m_.insert(pair<DetectorBandType,EnumPar<void> >
     (DetectorBandTypeMod::HOLOGRAPHY_RECEIVER,ep((int)DetectorBandTypeMod::HOLOGRAPHY_RECEIVER,"HOLOGRAPHY_RECEIVER","un-documented")));
    m_.insert(pair<DetectorBandType,EnumPar<void> >
     (DetectorBandTypeMod::SUBBAND,ep((int)DetectorBandTypeMod::SUBBAND,"SUBBAND","un-documented")));
    return true;
  }
  static map<DetectorBandType,EnumPar<void> > m_;
};
#define _DETECTORBANDTYPE_HH
#endif
