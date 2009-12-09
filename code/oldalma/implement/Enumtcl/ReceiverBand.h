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
 struct enum_set_traits<ReceiverBand> : public enum_set_traiter<ReceiverBand,16,ReceiverBandMod::UNSPECIFIED> {};

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
     (ReceiverBandMod::ALMA_RB_01,ep((int)ReceiverBandMod::ALMA_RB_01,"ALMA_RB_01","ALMA Receiver band 01")));
    m_.insert(pair<ReceiverBand,EnumPar<void> >
     (ReceiverBandMod::ALMA_RB_02,ep((int)ReceiverBandMod::ALMA_RB_02,"ALMA_RB_02","ALMA Receiver band 02")));
    m_.insert(pair<ReceiverBand,EnumPar<void> >
     (ReceiverBandMod::ALMA_RB_03,ep((int)ReceiverBandMod::ALMA_RB_03,"ALMA_RB_03","ALMA Receiver band 03")));
    m_.insert(pair<ReceiverBand,EnumPar<void> >
     (ReceiverBandMod::ALMA_RB_04,ep((int)ReceiverBandMod::ALMA_RB_04,"ALMA_RB_04","ALMA Receiver band 04")));
    m_.insert(pair<ReceiverBand,EnumPar<void> >
     (ReceiverBandMod::ALMA_RB_05,ep((int)ReceiverBandMod::ALMA_RB_05,"ALMA_RB_05","ALMA Receiver band 05")));
    m_.insert(pair<ReceiverBand,EnumPar<void> >
     (ReceiverBandMod::ALMA_RB_06,ep((int)ReceiverBandMod::ALMA_RB_06,"ALMA_RB_06","ALMA Receiver band 06")));
    m_.insert(pair<ReceiverBand,EnumPar<void> >
     (ReceiverBandMod::ALMA_RB_07,ep((int)ReceiverBandMod::ALMA_RB_07,"ALMA_RB_07","ALMA Receiver band 07")));
    m_.insert(pair<ReceiverBand,EnumPar<void> >
     (ReceiverBandMod::ALMA_RB_08,ep((int)ReceiverBandMod::ALMA_RB_08,"ALMA_RB_08","ALMA Receiver band 08")));
    m_.insert(pair<ReceiverBand,EnumPar<void> >
     (ReceiverBandMod::ALMA_RB_09,ep((int)ReceiverBandMod::ALMA_RB_09,"ALMA_RB_09","ALMA Receiver band 09")));
    m_.insert(pair<ReceiverBand,EnumPar<void> >
     (ReceiverBandMod::ALMA_RB_10,ep((int)ReceiverBandMod::ALMA_RB_10,"ALMA_RB_10","ALMA Receiver band 10")));
    m_.insert(pair<ReceiverBand,EnumPar<void> >
     (ReceiverBandMod::ALMA_HOLOGRAPHY_RECEIVER,ep((int)ReceiverBandMod::ALMA_HOLOGRAPHY_RECEIVER,"ALMA_HOLOGRAPHY_RECEIVER","Alma transmitter Holography receiver.")));
    m_.insert(pair<ReceiverBand,EnumPar<void> >
     (ReceiverBandMod::BURE_01,ep((int)ReceiverBandMod::BURE_01,"BURE_01","Plateau de Bure receiver band #1.")));
    m_.insert(pair<ReceiverBand,EnumPar<void> >
     (ReceiverBandMod::BURE_02,ep((int)ReceiverBandMod::BURE_02,"BURE_02","Plateau de Bure receiver band #2.")));
    m_.insert(pair<ReceiverBand,EnumPar<void> >
     (ReceiverBandMod::BURE_03,ep((int)ReceiverBandMod::BURE_03,"BURE_03","Plateau de Bure receiver band #3.")));
    m_.insert(pair<ReceiverBand,EnumPar<void> >
     (ReceiverBandMod::BURE_04,ep((int)ReceiverBandMod::BURE_04,"BURE_04","Plateau de Bure receiver band #4")));
    m_.insert(pair<ReceiverBand,EnumPar<void> >
     (ReceiverBandMod::UNSPECIFIED,ep((int)ReceiverBandMod::UNSPECIFIED,"UNSPECIFIED","receiver band of unspecified origin.")));
    return true;
  }
  static map<ReceiverBand,EnumPar<void> > m_;
};
#define _RECEIVERBAND_HH
#endif
