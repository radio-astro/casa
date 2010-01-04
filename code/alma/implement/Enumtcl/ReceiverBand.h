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


#if     !defined(_RECEIVERBAND_H)

#include <CReceiverBand.h>
#define _RECEIVERBAND_H
#endif 

#if     !defined(_RECEIVERBAND_HH)

#include "Enum.hpp"

using namespace ReceiverBandMod;

template<>
 struct enum_set_traits<ReceiverBand> : public enum_set_traiter<ReceiverBand,17,ReceiverBandMod::UNSPECIFIED> {};

template<>
class enum_map_traits<ReceiverBand,void> : public enum_map_traiter<ReceiverBand,void> {
public:
  static bool   init_;
  static string typeName_;
  static string enumerationDesc_;
  static string order_;
  static string xsdBaseType_;
  static bool   init(){
    EnumPar<void> ep;
    m_.insert(pair<ReceiverBand,EnumPar<void> >
     (ReceiverBandMod::ALMA_RB_01,ep((int)ReceiverBandMod::ALMA_RB_01,"ALMA_RB_01","un-documented")));
    m_.insert(pair<ReceiverBand,EnumPar<void> >
     (ReceiverBandMod::ALMA_RB_02,ep((int)ReceiverBandMod::ALMA_RB_02,"ALMA_RB_02","un-documented")));
    m_.insert(pair<ReceiverBand,EnumPar<void> >
     (ReceiverBandMod::ALMA_RB_03,ep((int)ReceiverBandMod::ALMA_RB_03,"ALMA_RB_03","un-documented")));
    m_.insert(pair<ReceiverBand,EnumPar<void> >
     (ReceiverBandMod::ALMA_RB_04,ep((int)ReceiverBandMod::ALMA_RB_04,"ALMA_RB_04","un-documented")));
    m_.insert(pair<ReceiverBand,EnumPar<void> >
     (ReceiverBandMod::ALMA_RB_05,ep((int)ReceiverBandMod::ALMA_RB_05,"ALMA_RB_05","un-documented")));
    m_.insert(pair<ReceiverBand,EnumPar<void> >
     (ReceiverBandMod::ALMA_RB_06,ep((int)ReceiverBandMod::ALMA_RB_06,"ALMA_RB_06","un-documented")));
    m_.insert(pair<ReceiverBand,EnumPar<void> >
     (ReceiverBandMod::ALMA_RB_07,ep((int)ReceiverBandMod::ALMA_RB_07,"ALMA_RB_07","un-documented")));
    m_.insert(pair<ReceiverBand,EnumPar<void> >
     (ReceiverBandMod::ALMA_RB_08,ep((int)ReceiverBandMod::ALMA_RB_08,"ALMA_RB_08","un-documented")));
    m_.insert(pair<ReceiverBand,EnumPar<void> >
     (ReceiverBandMod::ALMA_RB_09,ep((int)ReceiverBandMod::ALMA_RB_09,"ALMA_RB_09","un-documented")));
    m_.insert(pair<ReceiverBand,EnumPar<void> >
     (ReceiverBandMod::ALMA_RB_10,ep((int)ReceiverBandMod::ALMA_RB_10,"ALMA_RB_10","un-documented")));
    m_.insert(pair<ReceiverBand,EnumPar<void> >
     (ReceiverBandMod::ALMA_RB_ALL,ep((int)ReceiverBandMod::ALMA_RB_ALL,"ALMA_RB_ALL","un-documented")));
    m_.insert(pair<ReceiverBand,EnumPar<void> >
     (ReceiverBandMod::ALMA_HOLOGRAPHY_RECEIVER,ep((int)ReceiverBandMod::ALMA_HOLOGRAPHY_RECEIVER,"ALMA_HOLOGRAPHY_RECEIVER","un-documented")));
    m_.insert(pair<ReceiverBand,EnumPar<void> >
     (ReceiverBandMod::BURE_01,ep((int)ReceiverBandMod::BURE_01,"BURE_01","un-documented")));
    m_.insert(pair<ReceiverBand,EnumPar<void> >
     (ReceiverBandMod::BURE_02,ep((int)ReceiverBandMod::BURE_02,"BURE_02","un-documented")));
    m_.insert(pair<ReceiverBand,EnumPar<void> >
     (ReceiverBandMod::BURE_03,ep((int)ReceiverBandMod::BURE_03,"BURE_03","un-documented")));
    m_.insert(pair<ReceiverBand,EnumPar<void> >
     (ReceiverBandMod::BURE_04,ep((int)ReceiverBandMod::BURE_04,"BURE_04","un-documented")));
    m_.insert(pair<ReceiverBand,EnumPar<void> >
     (ReceiverBandMod::UNSPECIFIED,ep((int)ReceiverBandMod::UNSPECIFIED,"UNSPECIFIED","un-documented")));
    return true;
  }
  static map<ReceiverBand,EnumPar<void> > m_;
};
#define _RECEIVERBAND_HH
#endif
