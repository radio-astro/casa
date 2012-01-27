//# DisplayEnums.cc: enumerated constants for the Display Library
//# Copyright (C) 1993,1994,1995,1996,1998,1999,2000,2001,2002
//# Associated Universities, Inc. Washington DC, USA.
//# 
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//# 
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//# 
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//# 
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//# 
//# $Id$

#include <display/Display/DisplayEnums.h>
#include <casa/Exceptions/Error.h>
#include <casa/iostream.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// write to ostream support
ostream & operator << (ostream & os, Display::ColorModel dmt)
{
  switch(dmt)
    {
      case Display::Index: os << "Index"; break;
      case Display::RGB: os << "RGB"; break;
      case Display::HSV: os << "HSV"; break;
    }

  return os;
}
// write to ostream support
ostream & operator << (ostream & os, Display::Strategy dmt)
{
  switch(dmt)
    {
      case Display::Best: os << "Best"; break;
      case Display::Default: os << "Default"; break;
      case Display::MinMax: os << "MinMax"; break;
      case Display::Percent: os << "Percent"; break;
      case Display::Custom: os << "Custom"; break;
    }

  return os;
}
// write to ostream support
ostream & operator << (ostream & os, Display::SpecialMap dmt)
{
  switch(dmt)
    {
      case Display::System: os << "System"; break;
      case Display::New: os << "New"; break;
      case Display::Shared: os << "Shared"; break;
    }

  return os;
}
// write to ostream support
ostream & operator << (ostream & os, Display::LineStyle dmt)
{
  switch(dmt)
    {
      case Display::LSSolid: os << "LSSolid"; break;
      case Display::LSDashed: os << "LSDashed"; break;
      case Display::LSDoubleDashed: os << "LSDoubleDashed"; break;
    }

  return os;
}
// write to ostream support
ostream & operator << (ostream & os, Display::CapStyle dmt)
{
  switch(dmt)
    {
      case Display::CSNotLast: os << "CSNotLast"; break;
      case Display::CSButt: os << "CSButt"; break;
      case Display::CSRound: os << "CSRound"; break;
      case Display::CSProjecting: os << "CSProjecting"; break;
    }

  return os;
}
// write to ostream support
ostream & operator << (ostream & os, Display::JoinStyle dmt)
{
  switch(dmt)
    {
      case Display::JSMiter: os << "JSMiter"; break;
      case Display::JSRound: os << "JSRound"; break;
      case Display::JSBevel: os << "JSBevel"; break;
    }

  return os;
}
// write to ostream support
ostream & operator << (ostream & os, Display::FillStyle dfs)
{
  switch(dfs)
    {
      case Display::FSSolid: os << "FSSolid"; break;
      case Display::FSTiled: os << "FSTiled"; break;
      case Display::FSStippled: os << "FSStippled"; break;
      case Display::FSOpaqueStippled: os << "FSOpaqueStippled"; break;
    }

  return os;
}
// write to ostream support
ostream & operator << (ostream & os, Display::FillRule dfr)
{
  switch(dfr)
    {
      case Display::FREvenOdd: os << "FREvenOdd"; break;
      case Display::FRWinding: os << "FRWinding"; break;
    }

  return os;
}
// write to ostream support
ostream & operator << (ostream & os, Display::ArcMode dam)
{
  switch(dam)
    {
      case Display::AMChord: os << "AMChord"; break;
      case Display::AMPieSlice: os << "AMPieSlice"; break;
    }

  return os;
}
// write to ostream support
ostream & operator << (ostream & os, Display::TextAlign dta)
{
  switch(dta)
    {
      case Display::AlignCenter: os << "AlignCenter"; break;
      case Display::AlignLeft: os << "AlignLeft"; break;
      case Display::AlignTop: os << "AlignTop"; break;
      case Display::AlignRight: os << "AlignRight"; break;
      case Display::AlignBottom: os << "AlignBottom"; break;
      case Display::AlignTopLeft: os << "AlignTopLeft"; break;
      case Display::AlignTopRight: os << "AlignTopRight"; break;
      case Display::AlignBottomLeft: os << "AlignBottomLeft"; break;
      case Display::AlignBottomRight: os << "AlignBottomRight"; break;
    }

  return os;
}

ostream & operator << (ostream & os, Display::Marker mrk) 
{
  switch(mrk) 
    {
    case Display::Cross : os << "Cross"; break;
    case Display::X: os << "X"; break;
    case Display::Diamond: os << "Diamond"; break;
    case Display::Circle: os << "Circle"; break;
    case Display::Square: os << "Square"; break;
    case Display::Triangle: os << "Triangle"; break;
    case Display::InvertedTriangle: os << "InvertedTriangle"; break;
      //case Display::Pentagon: os << "Pentagon"; break;
      //case Display::Hexagon: os << "Hexagon"; break;
    case Display::FilledCircle: os << "FilledCircle"; break;
    case Display::FilledSquare: os << "FilledSquare"; break;
    case Display::FilledDiamond: os << "FilledDiamond" ; break;
    case Display::FilledTriangle: os << "FilledTriangle"; break;
    case Display::FilledInvertedTriangle: 
      os << "FilledInvertedTriangle" ; break;
      //case Display::FilledPentagon: os << "FilledPentagon"; break;
      //case Display::FilledHexagon: os << "FilledHexagon"; break;
    case Display::CircleAndCross: os << "CircleAndCross" ; break;
    case Display::CircleAndX: os << "CircleAndX" ; break;
    case Display::CircleAndDot: os << "CircleAndDot"; break;
    default: throw(AipsError("Shouldn't get here"));
    }
  return os;
}

// write to ostream support
ostream & operator << (ostream & os, Display::DrawMode ddm)
{
  switch(ddm)
    {
    case Display::Draw: os << "Draw"; break;
    case Display::Compile: os << "Compile"; break;
    default: throw(AipsError("Shouldn't get here"));
    }
  
  return os;
}

ostream & operator << (ostream & os, Display::DrawBuffer ddb)
{
  switch(ddb)
    {
    case Display::NoBuffer: os << "NoBuffer"; break;
    case Display::DefaultBuffer: os << "DefaultBuffer"; break;
    case Display::FrontBuffer: os << "FrontBuffer"; break;
    case Display::BackBuffer: os << "BackBuffer"; break;
    case Display::FrontAndBackBuffer: os << "FrontAndBackBuffer"; break;
    default: throw(AipsError("Shouldn't get here"));
    }

  return os;
}

ostream & operator << (ostream & os, Display::DrawFunction ddf)
{
  switch(ddf)
    {
    case Display::DFCopy: os << "DFCopy"; break;
    case Display::DFCopyInverted: os << "DFCopyInverted"; break;
    case Display::DFClear: os << "DFClear"; break;
    case Display::DFSet: os << "DFSet"; break;
    case Display::DFInvert: os << "DFInvert"; break;
    case Display::DFNoop: os << "DFNoop"; break;
    case Display::DFXor: os << "DFXor"; break;
    case Display::DFEquiv: os << "DFEquiv"; break;
    case Display::DFAnd: os << "DFAnd"; break;
    case Display::DFNand: os << "DFNand"; break;
    case Display::DFAndReverse: os << "DFAndReverse"; break;
    case Display::DFAndInverted: os << "DFAndInverted"; break;
    case Display::DFOr: os << "DFOr"; break;
    case Display::DFNor: os << "DFNor"; break;
    case Display::DFOrReverse: os << "DFOrReverse"; break;
    case Display::DFOrInverted: os << "DFOrInverted"; break;
    default: throw(AipsError("Shouldn't get here"));
    }
  return os;
}

ostream & operator << (ostream & os, Display::RefreshReason drr)
{
  switch(drr)
    {
    case Display::UserCommand: os << "UserCommand"; break;
    case Display::ColorTableChange: os << "ColorTableChange"; break;
    case Display::PixelCoordinateChange: os << "PixelCoordinateChange"; break;
    case Display::LinearCoordinateChange: os << "LinearCoordinateChange"; break;
    case Display::WorldCoordinateChange: os << "WorldCoordinateChange"; break;
    case Display::ColormapChange: os << "ColormapChange"; break;
    case Display::BackCopiedToFront: os << "BackCopiedToFront"; break;
    case Display::ClearPriorToColorChange: os << "ClearPriorToColorChange";
      break;
    }
  return os;
}

ostream & operator << (ostream & os, Display::ImageCacheStrategy ics)
{
  switch(ics)
    {
    case Display::ClientAlways: os << "ClientAlways"; break;
    case Display::ServerAlways: os << "ServerAlways"; break;
    case Display::ServerMemoryThreshold: os << "ServerMemoryThreshold"; break;
    }
  return os;
}

ostream & operator << (ostream & os, Display::ComplexToRealMethod ctrm)
{
  switch(ctrm)
    {
    case Display::Magnitude: os << "Magnitude"; break;
    case Display::Phase: os << "Phase"; break;
    case Display::Real: os << "Real"; break;
    case Display::Imaginary: os << "Imaginary"; break;
    }
  return os;
}

ostream & operator << (ostream & os, Display::DisplayDataType dtype) {
  switch(dtype) {
  case Display::Raster: os << "Raster"; break;
  case Display::Vector: os << "Vector"; break;
  case Display::Annotation: os << "Annotation"; break;
  case Display::CanvasAnnotation: os << "CanvasAnnotation"; break;
  }
  return os;
}

ostream & operator << (ostream & os, Display::KeyModifier dkm)
{
  Bool sep = False;
  os << "{";
  if (dkm & Display::KM_Shift) { if (sep) os << "|"; sep = True; os << "Shift"; }
  if (dkm & Display::KM_Shift) { if (sep) os << "|"; sep = True; os << "CapsLock"; }
  if (dkm & Display::KM_Shift) { if (sep) os << "|"; sep = True; os << "Ctrl"; }
  if (dkm & Display::KM_Shift) { if (sep) os << "|"; sep = True; os << "Alt"; }
  if (dkm & Display::KM_Shift) { if (sep) os << "|"; sep = True; os << "NumLock"; }
  if (dkm & Display::KM_Shift) { if (sep) os << "|"; sep = True; os << "ScrollLock"; }
  if (dkm & Display::KM_Shift) { if (sep) os << "|"; sep = True; os << "Mod4"; }
  if (dkm & Display::KM_Shift) { if (sep) os << "|"; sep = True; os << "Mod5"; }
  if (dkm & Display::KM_Shift) { if (sep) os << "|"; sep = True; os << "PointerButton1"; }
  if (dkm & Display::KM_Shift) { if (sep) os << "|"; sep = True; os << "PointerButton2"; }
  if (dkm & Display::KM_Shift) { if (sep) os << "|"; sep = True; os << "PointerButton3"; }
  os << "}";
  return os;
}

// For Keysyms 0xFF00 to 0xFFFF
static char * ttyFuncStr[256] =
{
  "","","","","","","","",
  "K_BackSpace","K_Tab","K_Linefeed","K_Clear","","K_Return","","",

  "","","","K_Pause","K_Scroll_Lock","K_Sys_Req","","",
  "","","","K_Escape","","","","",

  "K_Multi_key","","","","","","","",
  "","","","","","","","",

  "","","","","","","","",
  "","","","","","","","",

  "","","","","","","","",
  "","","","","","","","",

  "K_Home","K_Left","K_Up","K_Right","K_Down","K_Page_Up","K_Page_Down","K_End",
  "K_Begin","","","","","","","",

  "K_Select","K_Print","K_Execute","K_Insert","","K_Undo","K_Redo","K_Menu",
  "K_Find","K_Cancel","K_Help","K_Break","","","","",

  "","","","","","","","",
  "","","","","","","K_Mode_switch","K_Num_Lock",

  "K_KP_Space","","","","","","","",
  "","K_KP_Tab","","","","K_KP_Enter","","",

  "","K_KP_F1","K_KP_F2","K_KP_F3","K_KP_F4","K_KP_Home","K_KP_Left","K_KP_Up",
  "K_KP_Right","K_KP_Down","K_KP_Page_Up","K_KP_Page_Down","K_KP_End","K_KP_Begin","K_KP_Insert","K_KP_Delete",

  "","","","","","","","",
  "","","K_KP_Multiply","K_KP_Add","K_KP_Separator","K_KP_Subtract","K_KP_Decimal","K_KP_Divide",

  "K_KP_0","K_KP_1","K_KP_2","K_KP_3","K_KP_4","K_KP_5","K_KP_6","K_KP_7",
  "K_KP_8","K_KP_9","","","","","K_F1","K_F2",

  "K_F3","K_F4","K_F5","K_F6","K_F7","K_F8","K_F9","K_F10",
  "K_F11","K_F12","K_F13","K_F14","K_F15","K_F16","K_F17","K_F18",

  "K_F19","K_F20","K_F21","K_F22","K_F23","K_F24","K_F25","K_F26",
  "K_F27","K_F28","K_F29","K_F30","K_F31","K_F32","K_F33","K_F34",

  "K_F35","K_Shift_L","K_Shift_R","K_Control_L","K_Control_R","K_Caps_Lock","K_Shift_Lock","K_Meta_L",
  "K_Meta_R","K_Alt_L","K_Alt_R","K_Super_L","K_Super_R","K_Hyper_L","K_Hyper_R","",

  "","","","","","","","",
  "","","","","","","","K_Delete"
};

// begins at 0xFEE9
static const char * mouseStr[5] =
{
  "K_Pointer_Button1",  "K_Pointer_Button2",  "K_Pointer_Button3",
  "K_Pointer_Button4",  "K_Pointer_Button5"
};

// begin at 0x0000
static const char * latin1Str[256] =
{
  "K_None","","","","","","","",
  "","","","","","","","",

  "","","","","","","","",
  "","","","","","","","",

  "K_space","K_exclam","K_quotedbl","K_numbersign","K_dollar","K_percent","K_ampersand","K_apostrophe",
  "K_parenleft","K_parenright","K_asterisk","K_plus","K_comma","K_minus","K_period","K_slash",
  
  "K_0","K_1","K_2","K_3","K_4","K_5","K_6","K_7",
  "K_8","K_9","K_colon","K_semicolon","K_less","K_equal","K_greater","K_question",
  
  "K_at","K_A","K_B","K_C","K_D","K_E","K_F","K_G",
  "K_H","K_I","K_J","K_K","K_L","K_M","K_N","K_O",
  
  "K_P","K_Q","K_R","K_S","K_T","K_U","K_V","K_W",
  "K_X","K_Y","K_Z","K_bracketleft","K_backslash","K_bracketright","K_asciicircum","K_underscore",
  
  "K_grave","K_a","K_b","K_c","K_d","K_e","K_f","K_g",
  "K_h","K_i","K_j","K_k","K_l","K_m","K_n","K_o",
  
  "K_p","K_q","K_r","K_s","K_t","K_u","K_v","K_w",
  "K_x","K_y","K_z","K_braceleft","K_bar","K_braceright","K_asciitilde","",

  "","","","","","","","",
  "","","","","","","","",

  "","","","","","","","",
  "","","","","","","","",

  "K_nobreakspace","K_exclamdown","K_cent","K_sterling","K_currency","K_yen","K_brokenbar","K_section",
  "K_diaeresis","K_copyright","K_ordfeminine","K_guillemotleft","K_notsign","K_hyphen","K_registered","K_macron",

  "K_degree","K_plusminus","K_twosuperior","K_threesuperior","K_acute","K_mu","K_paragraph","K_periodcentered",
  "K_cedilla","K_onesuperior","K_masculine","K_guillemotright","K_onequarter","K_onehalf","K_threequarters","K_questiondown",

  "K_Agrave","K_Aacute","K_Acircumflex","K_Atilde","K_Adiaeresis","K_Aring","K_AE","K_Ccedilla",
  "K_Egrave","K_Eacute","K_Ecircumflex","K_Ediaeresis","K_Igrave","K_Iacute","K_Icircumflex","K_Idiaeresis",

  "K_ETH","K_Ntilde","K_Ograve","K_Oacute","K_Ocircumflex","K_Otilde","K_Odiaeresis","K_multiply",
  "K_Ooblique","K_Ugrave","K_Uacute","K_Ucircumflex","K_Udiaeresis","K_Yacute","K_THORN","K_ssharp",

  "K_agrave","K_aacute","K_acircumflex","K_atilde","K_adiaeresis","K_aring","K_ae","K_ccedilla",
  "K_egrave","K_eacute","K_ecircumflex","K_ediaeresis","K_igrave","K_iacute","K_icircumflex","K_idiaeresis",

  "K_eth","K_ntilde","K_ograve","K_oacute","K_ocircumflex","K_otilde","K_odiaeresis","K_division",
  "K_oslash","K_ugrave","K_uacute","K_ucircumflex","K_udiaeresis","K_yacute","K_thorn","K_ydiaeresis",
};

// begins at 01A0
static const char * latin2Str[96] =
{
  "","K_Aogonek","K_breve","K_Lstroke","","K_Lcaron","K_Sacute","",
  "","K_Scaron","K_Scedilla","K_Tcaron","K_Zacute","","K_Zcaron","K_Zabovedot",

  "","K_aogonek","K_ogonek","K_lstroke","","K_lcaron","K_sacute","K_caron",
  "","K_scaron","K_scedilla","K_tcaron","K_zacute","K_doubleacute","K_zcaron","K_zabovedot",

  "K_Racute","","","K_Abreve","","K_Lacute","K_Cacute","",
  "K_Ccaron","","K_Eogonek","","K_Ecaron","","","K_Dcaron",

  "K_Dstroke","K_Nacute","K_Ncaron","","","K_Odoubleacute","","",
  "K_Rcaron","K_Uring","","K_Udoubleacute","","","K_Tcedilla","",

  "K_racute","","","K_abreve","","K_lacute","K_cacute","",
  "K_ccaron","","K_eogonek","","K_ecaron","","","K_dcaron",

  "K_dstroke","K_nacute","K_ncaron","","","K_odoubleacute","","",
  "K_rcaron","K_uring","","K_udoubleacute","","","K_tcedilla","K_abovedot"
};

// begins at 02a0
static const char * latin3Str[96] =
{
  "","K_Hstroke","","","","","K_Hcircumflex","",
  "","K_Iabovedot","","K_Gbreve","K_Jcircumflex","","","",

  "","K_hstroke","","","","","K_hcircumflex","",
  "","K_idotless","","K_gbreve","K_jcircumflex","","","",

  "","","","","","K_Cabovedot","K_Ccircumflex","",
  "","","","","","","","",

  "","","","","","","K_Gabovedot","",
  "K_Gcircumflex","","","","","K_Ubreve","K_Scircumflex","",

  "","","","","","K_cabovedot","K_ccircumflex","",
  "","","","","","","","",

  "","","","","","K_gabovedot","","",
  "K_gcircumflex","","","","","K_ubreve","K_scircumflex",""
};

// begins at 03a0
static const char * latin4Str[96] =
{
  "","","K_kra","K_Rcedilla","","K_Itilde","K_Lcedilla","",
  "","","K_Emacron","K_Gcedilla","K_Tslash","","","",

  "","","","K_rcedilla","","K_itilde","K_lcedilla","",
  "","","K_emacron","K_gcedilla","K_tslash","K_ENG","","K_eng",

  "K_Amacron","","","","","","","K_Iogonek",
  "","","","","K_Eabovedot","","","K_Imacron",

  "","K_Ncedilla","K_Omacron","K_Kcedilla","","","","",
  "","K_Uogonek","","","","K_Utilde","K_Umacron","",

  "K_amacron","","","","","","","K_iogonek",
  "","","","","K_eabovedot","","","K_imacron",

  "K_ncedilla","K_omacron","K_kcedilla","","","","","",
  "","K_uogonek","","","","K_utilde","K_umacron",""
};

// begins at 07a0
static const char * greekStr[96] =
{
  "","K_Greek_ALPHAaccent","K_Greek_EPSILONaccent","K_Greek_ETAaccent",
  "K_Greek_IOTAaccent","K_Greek_IOTAdiaeresis","","K_Greek_OMICRNaccent",
  "K_Greek_UPSILONaccent","K_Greek_UPSILONdieresis","","K_Greek_OMEGAaccent",
  "","","K_Greek_accentdieresis","K_Greek_horizbar",

  "","K_Greek_alphaaccent","K_Greek_epsilonaccent","K_Greek_etaaccent",
  "K_Greek_iotaaccent","K_Greek_iotadieresis","K_Greek_iotaacentdieresis","K_Greek_omicronaccent",
  "K_Greek_upsilonaccent","K_Greek_upsilondieresis","K_Greek_upsilonaccentdieresis","K_Greek_omegaaccent",
  "","","","",

  "","K_Greek_ALPHA","K_Greek_BETA","K_Greek_GAMMA",
  "K_Greek_DELTA","K_Greek_EPSILON","K_Greek_ZETA","K_Greek_ETA",
  "K_Greek_THETA","K_Greek_IOTA","K_Greek_KAPPA","K_Greek_LAMBDA",
  "K_Greek_MU","K_Greek_NU","K_Greek_XI","K_Greek_OMICRON",

  "K_Greek_PI","K_Greek_RHO","K_Greek_SIGMA","",
  "K_Greek_TAU","K_Greek_UPSILON","K_Greek_PHI","K_Greek_CHI",
  "K_Greek_PSI","K_Greek_OMEGA","","",
  "","","","",

  "","K_Greek_alpha","K_Greek_beta","K_Greek_gamma",
  "K_Greek_delta","K_Greek_epsilon","K_Greek_zeta","K_Greek_eta",
  "K_Greek_theta","K_Greek_iota","K_Greek_kappa","K_Greek_lambda",
  "K_Greek_mu","K_Greek_nu","K_Greek_xi","K_Greek_omicron",
  
  "K_Greek_pi","K_Greek_rho","K_Greek_sigma","K_Greek_finalsmallsigma",
  "K_Greek_tau","K_Greek_upsilon","K_Greek_phi","K_Greek_chi",
  "K_Greek_psi","K_Greek_omega","","",
  "","","",""
};

// write to ostream support
ostream & operator << (ostream & os, Display::KeySym dks)
{
  uInt i = (uInt) dks;

  if (i <= 0xFF) os << latin1Str[i];
  else if (i >= 0x01A0 && i <= 0x01FF) os << latin2Str[i-0x01A0];
  else if (i >= 0x02A0 && i <= 0x02FF) os << latin3Str[i-0x02A0];
  else if (i >= 0x03A0 && i <= 0x03FF) os << latin4Str[i-0x03A0];
  else if (i >= 0x07A0 && i <= 0x07FF) os << greekStr[i-0x07A0];
  else if (i >= 0xFEE9 && i <= 0xFEED) os << mouseStr[i-0xFEE9];
  else if (i >= 0xFF00 && i <= 0xFFFF) os << ttyFuncStr[i-0xFF00];

  return os;
}

// Convert to KeyModifier from KeySym
Display::KeyModifier Display::keyModifierFromKeySym(const Display::KeySym 
						    &keysym) {
  switch (keysym) {
  case Display::K_Shift_L:
  case Display::K_Shift_R:
    {
      return Display::KM_Shift;
      break;
    }
  case Display::K_Control_L:
  case Display::K_Control_R:
    { 
      return Display::KM_Ctrl;
      break;
    }
  case Display::K_Alt_L:
  case Display::K_Alt_R:
    {
      return Display::KM_Alt;
    }
  case Display::K_Pointer_Button1: 
    {
      return Display::KM_Pointer_Button_1;
      break;
    }
  case Display::K_Pointer_Button2:
    {
      return Display::KM_Pointer_Button_2;
      break;
    }
  case Display::K_Pointer_Button3:
    {
      return Display::KM_Pointer_Button_3;
      break;
    }
  case Display::K_Pointer_Button4:
    {
      return Display::KM_Pointer_Button_4;
      break;
    }
  case Display::K_Pointer_Button5:
    {
      return Display::KM_Pointer_Button_5;
      break;
    }
  case Display::K_None:
    {
      return ((Display::KeyModifier)0);
      break;
    }
  default:
    throw(AipsError("Cannot map Display::KeySym given to "
		    "Display::keyModifierFromKeySym"));    
  }
}

} //# NAMESPACE CASA - END

