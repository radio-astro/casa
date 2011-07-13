//# DisplayEnums.h: enumerated constants for the Display Library
//# Copyright (C) 1994,1995,1996,1997,1998,1999,2000,2001,2002
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

#ifndef TRIALDISPLAY_DISPLAYENUMS_H
#define TRIALDISPLAY_DISPLAYENUMS_H

#include <casa/aips.h>

//# Forward declarations
#include <casa/iosfwd.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// Enumerate constants for the display classes.
// </summary>
//
// <use visibility=export>
//
// <reviewed>
// </reviewed>
//  
// <prerequisite>
// <li> none
// </prerequisite>
//
// <etymology>
// DisplayEnums : Display library enumerators
// </etymology>
//
// <synopsis>
// Standard keysym constants chosen to match the X system.  Event handling done
// in other systems must provide the same constants when returning
// keysyms.
//
// Definitions for all characters in ISO LATIN 1-4, and GREEK keysyms
// are included.
//
// </synopsis>
//
// <motivation>
// High standardization and flexible support of national and
// international keyboard buttons.
// </motivation>
//

class Display
{
public:
  // Different ways of handling color on the display
  enum ColorModel {
    // ColorIndex mode
    Index,
    // RGB mode
    RGB,
    // HSV mode
    HSV 
  };

  // Color components
  enum ColorComponent {
    Red,
    Green,
    Blue,
    Hue,
    Saturation,
    Value
  };

  // Strategy to use to allocate system color resources
  enum Strategy {
    // Want the most colors regardless of consequences
    Best,
    // Want the default allocation
    Default,
    // Happy with a range between some min and max
    MinMax,
    // Want some percentage of the available allocation
    Percent,
    // Want it my way or not at all
    Custom 
  };

  // Special map is used as a placeholder to indicate that ...
  enum SpecialMap {
    // the user wants to use the system (default) Colormap
    System,
    // the user wants to use a new colormap
    New,
    // the user wants to share an existing (possibly system!) Colormap
    // (this is used internally at present)
    Shared
  };

  // Style of line to use
  enum LineStyle {
    // solid, no breaks
    LSSolid,
    // dashed line with transparent breaks
    LSDashed,
    // dashed line with background-color breaks
    LSDoubleDashed
  };

  // Controls the appearance of thick lines near their endpoints.
  // See Xlib programming guide, page 126 for complete info
  enum CapStyle {
    // don't draw pixels aligned with an endpoint
    CSNotLast,
    // draw pixels up to the endpoint
    CSButt,
    // round the edges using pixels past the endpoint
    CSRound,
    // continue linewidth/2 pixels past the endpoint
    CSProjecting
  };

  // Controls the appearance of two joined lines
  enum JoinStyle {
    // corner ends in a point
    JSMiter,
    // corner is rounded off
    JSRound,
    // corner is flat surface whose normal bisects the joint angle.
    JSBevel
  };

  // Controls fill style
  enum FillStyle {
    // solid fill
    FSSolid,
    // tiled fill
    FSTiled,
    // stippled fill
    FSStippled,
    // opaque stippled
    FSOpaqueStippled
  };

  // Fill Rule
  // Xlib programming guide, pg.132
  enum FillRule {
    // interior point filled if odd number of lines crossed to exit polygon
    FREvenOdd,
    // overlapping areas always filled
    FRWinding
  };

  // Arc Mode
  enum ArcMode {
    // Chord
    AMChord,
    // slice
    AMPieSlice
  };

  // Text Alignment
  enum TextAlign {
    // reference point aligned to center of text
    AlignCenter,
    // reference point aligned to center of left edge of text
    AlignLeft,
    // reference point aligned to center ot top edge of text
    AlignTop,
    // reference point aligned to center of right edge of text
    AlignRight,
    // reference point aligned to center of bottom edge of text
    AlignBottom,
    // reference point aligned to top left corner of text
    AlignTopLeft,
    // reference point aligned to top right corner of text
    AlignTopRight,
    // reference point aligned to bottom left corner of text
    AlignBottomLeft,
    // reference point aligned to bottom right corner of text
    AlignBottomRight
  };

  // PixelCanvas Drawing Modes
  enum DrawMode {
    // Immediate draw mode
    Draw,
    // Build display list mode
    Compile
  };

  // PixelCanvas target drawing destination
  enum DrawBuffer {
    // Nowhere
    NoBuffer,
    // Use default behaviour for the particular implementation of the PixelCanvas
    DefaultBuffer,
    // Draw only to front buffer
    FrontBuffer,
    // Draw only to back buffer
    BackBuffer,
    // Draw to both
    FrontAndBackBuffer
  };

  // PixelCanvas Drawing Logical Functions.  The functions implement
  // various combinations of dst = F(src, dest), where src is the current
  // color and dest is the existing color in the framebuffer.
  enum DrawFunction {
    // Normal operation, overwrite with src
    DFCopy,
    // Invert operation
    DFCopyInverted,
    // Clear, ignore current color, use background color
    DFClear,
    // Make all bits 1
    DFSet,
    // Invert the dest (twice to restore original dest)
    DFInvert,
    // do-nothing draw
    DFNoop,

    // Exclusive or, used for draw/erase operation over a variable 
    // background
    DFXor,
    // (!src) XOR dest
    DFEquiv,

    // src AND dest
    DFAnd,
    // !(src AND dest)
    DFNand,
    // src AND !dest
    DFAndReverse,
    // !src AND dest
    DFAndInverted,

    // src OR dest
    DFOr,
    // !(src OR dest)
    DFNor,
    // src OR !dest
    DFOrReverse,
    // !src OR dest
    DFOrInverted
  };
    
  // Callback reasons for PCRefreshEvent and WCRefreshEvent
  enum RefreshReason {
    // User told PixelCanvas to refresh via PixelCanvas refresh()
    UserCommand,
    // Colortable allocation changed (from PixelCanvasColorTable)
    ColorTableChange,
    // Colormap allocation changed - less severe than ColorTableChange,
    // this will mean an internal change to a single Display Library
    // Colormap that doesn't affect other Colormaps in the same
    // ColorTable
    ColormapChange,
    // Change in shape of PixelCanvas
    PixelCoordinateChange,
    // Change in linear coordinate system
    LinearCoordinateChange,
    // Change in world coordinate system
    WorldCoordinateChange,
    // Back buffer was copied to front buffer on the PixelCanvas
    BackCopiedToFront,
    // Clear required prior to a change in color table or distribution...
    ClearPriorToColorChange
  };

  // PixelCanvas Options
  enum Option {
    // Is a clipwindow enabled?
    ClipWindow
  };

  // How to cache images when displaying them on client-server graphics
  // systems
  enum ImageCacheStrategy {
    // Always use client-side caching (XImages for X Server)
    ClientAlways,
    // Always use server-side caching (Pixmaps for X Server)
    ServerAlways,
    // Use server-side pixmaps, recording the size of each 
    // pixmap created for image caching.  When the memory used
    // exceeds some threshold, change to use client-side
    // XImages to avoid swamping the X server
    ServerMemoryThreshold
  };

  // List of markers for use as annotations. These can also be drawn
  // as primitives on a pixelcanvas.
  const static Int nMarkers = 15;
  enum Marker {

    // Basic, Unfilled shapes;
    Cross = 0,
    X,
    Diamond,
    Circle,
    Square,
    Triangle, 
    InvertedTriangle,
    //Pentagon,
    //Hexagon,

    // Basic, filled shapes
    FilledCircle,
    FilledSquare,
    FilledDiamond,
    FilledTriangle,
    FilledInvertedTriangle,
    //FilledPentagon,
    //FilledHexagon,

    // Combination markers
    CircleAndCross,
    CircleAndX,
    CircleAndDot
  };

  // WorldCanvas - what to do with complex data if it is
  // presented to WorldCanvas::drawImage()
  enum ComplexToRealMethod {
    // Extract magnitude
    Magnitude,
    // Extract phase
    Phase,
    // Extract real part
    Real,
    // Extract imaginary part
    Imaginary
  };

  // WorldCanvasHolder - what type of DisplayData is this,
  // need to know for drawing order
  enum DisplayDataType {
    // Rastered data - should go first
    Raster,
    // Vector data - can go next
    Vector,
    // Annotation 
    Annotation,
    // Canvas annotation - can draw outside draw area
    CanvasAnnotation
  };

  // All events - modifier codes.
  // key is down if modifier & code is nonzero
  enum KeyModifier {
    // Shift key bit
    KM_Shift            = 0x0001,
    // Caps lock bit
    KM_CapsLock         = 0x0002,
    // Control key bit
    KM_Ctrl             = 0x0004,
    // Alt key bit
    KM_Alt              = 0x0008,
    // Numlock (1 = engaged)
    KM_NumLock          = 0x0010,
    // ScrollLock bit (1 = engaged)
    KM_ScrollLock       = 0x0020,
    // 
    KM_Mod1             = 0x0008,
    // 
    KM_Mod2             = 0x0010,
    // 
    KM_Mod3             = 0x0020,
    // 
    KM_Mod4             = 0x0040,
    // 
    KM_Mod5             = 0x0080,
    // Mouse button 1 bit
    KM_Pointer_Button_1 = 0x0100,
    // Mouse button 2 bit
    KM_Pointer_Button_2 = 0x0200,
    // Mouse button 3 bit
    KM_Pointer_Button_3 = 0x0400,
    // Mouse button 4 bit
    KM_Pointer_Button_4 = 0x0800,
    // Mouse button 5 bit
    KM_Pointer_Button_5 = 0x1000
  };

  //
  // Keysyms for PixelCanvas keyboard events.  These
  // can be printed using the stream functions listed
  // below.
  // These are mostly self explanatory.
  //

  enum KeySym {
    // 
    K_None              = 0x0000,
    //
    K_BackSpace		= 0xFF08,
    //
    K_Tab		= 0xFF09,
    //
    K_Linefeed		= 0xFF0A,
    //
    K_Clear		= 0xFF0B,
    //
    K_Return		= 0xFF0D,
    //
    K_Pause		= 0xFF13,
    //
    K_Scroll_Lock	= 0xFF14,
    //
    K_Sys_Req		= 0xFF15,
    //
    K_Escape		= 0xFF1B,
    //
    K_Delete		= 0xFFFF,

    //
    K_Multi_key		= 0xFF20,

    //
    K_Home		= 0xFF50,
    //
    K_Left		= 0xFF51,
    //
    K_Up		= 0xFF52,
    //
    K_Right		= 0xFF53,
    //
    K_Down		= 0xFF54,
    //
    K_Prior		= 0xFF55,
    //
    K_Page_Up		= 0xFF55,
    //
    K_Next		= 0xFF56,
    //
    K_Page_Down		= 0xFF56,
    //
    K_End		= 0xFF57,
    //
    K_Begin		= 0xFF58,

    //
    K_Select		= 0xFF60,
    //
    K_Print		= 0xFF61,
    //
    K_Execute		= 0xFF62,
    //
    K_Insert		= 0xFF63,
    //
    K_Undo		= 0xFF65,
    //
    K_Redo		= 0xFF66,
    //
    K_Menu		= 0xFF67,
    //
    K_Find		= 0xFF68,
    //
    K_Cancel		= 0xFF69,
    //
    K_Help		= 0xFF6A,
    //
    K_Break		= 0xFF6B,
    //
    K_Mode_switch	= 0xFF7E,
    //
    K_script_switch     = 0xFF7E,
    //
    K_Num_Lock		= 0xFF7F,
    //
    K_KP_Space		= 0xFF80,
    //
    K_KP_Tab		= 0xFF89,
    //
    K_KP_Enter		= 0xFF8D,
    //
    K_KP_F1		= 0xFF91,
    //
    K_KP_F2		= 0xFF92,
    //
    K_KP_F3		= 0xFF93,
    //
    K_KP_F4		= 0xFF94,
    //
    K_KP_Home		= 0xFF95,
    //
    K_KP_Left		= 0xFF96,
    //
    K_KP_Up		= 0xFF97,
    //
    K_KP_Right		= 0xFF98,
    //
    K_KP_Down		= 0xFF99,
    //
    K_KP_Prior		= 0xFF9A,
    //
    K_KP_Page_Up	= 0xFF9A,
    //
    K_KP_Next		= 0xFF9B,
    //
    K_KP_Page_Down	= 0xFF9B,
    //
    K_KP_End		= 0xFF9C,
    //
    K_KP_Begin		= 0xFF9D,
    //
    K_KP_Insert		= 0xFF9E,
    //
    K_KP_Delete		= 0xFF9F,
    //
    K_KP_Equal		= 0xFFBD,
    //
    K_KP_Multiply	= 0xFFAA,
    //
    K_KP_Add		= 0xFFAB,
    //
    K_KP_Separator	= 0xFFAC,
    //
    K_KP_Subtract	= 0xFFAD,
    //
    K_KP_Decimal	= 0xFFAE,
    //
    K_KP_Divide		= 0xFFAF,
    //
    K_KP_0		= 0xFFB0,
    //
    K_KP_1		= 0xFFB1,
    //
    K_KP_2		= 0xFFB2,
    //
    K_KP_3		= 0xFFB3,
    //
    K_KP_4		= 0xFFB4,
    //
    K_KP_5		= 0xFFB5,
    //
    K_KP_6		= 0xFFB6,
    //
    K_KP_7		= 0xFFB7,
    //
    K_KP_8		= 0xFFB8,
    //
    K_KP_9		= 0xFFB9,

    //
    K_F1		= 0xFFBE,
    //
    K_F2		= 0xFFBF,
    //
    K_F3		= 0xFFC0,
    //
    K_F4		= 0xFFC1,
    //
    K_F5		= 0xFFC2,
    //
    K_F6		= 0xFFC3,
    //
    K_F7		= 0xFFC4,
    //
    K_F8		= 0xFFC5,
    //
    K_F9		= 0xFFC6,
    //
    K_F10		= 0xFFC7,
    //
    K_F11		= 0xFFC8,
    //
    K_L1		= 0xFFC8,
    //
    K_F12		= 0xFFC9,
    //
    K_L2		= 0xFFC9,
    //
    K_F13		= 0xFFCA,
    //
    K_L3		= 0xFFCA,
    //
    K_F14		= 0xFFCB,
    //
    K_L4		= 0xFFCB,
    //
    K_F15		= 0xFFCC,
    //
    K_L5		= 0xFFCC,
    //
    K_F16		= 0xFFCD,
    //
    K_L6		= 0xFFCD,
    //
    K_F17		= 0xFFCE,
    //
    K_L7		= 0xFFCE,
    //
    K_F18		= 0xFFCF,
    //
    K_L8		= 0xFFCF,
    //
    K_F19		= 0xFFD0,
    //
    K_L9		= 0xFFD0,
    //
    K_F20		= 0xFFD1,
    //
    K_L10		= 0xFFD1,
    //
    K_F21		= 0xFFD2,
    //
    K_R1		= 0xFFD2,
    //
    K_F22		= 0xFFD3,
    //
    K_R2		= 0xFFD3,
    //
    K_F23		= 0xFFD4,
    //
    K_R3		= 0xFFD4,
    //
    K_F24		= 0xFFD5,
    //
    K_R4		= 0xFFD5,
    //
    K_F25		= 0xFFD6,
    //
    K_R5		= 0xFFD6,
    //
    K_F26		= 0xFFD7,
    //
    K_R6		= 0xFFD7,
    //
    K_F27		= 0xFFD8,
    //
    K_R7		= 0xFFD8,
    //
    K_F28		= 0xFFD9,
    //
    K_R8		= 0xFFD9,
    //
    K_F29		= 0xFFDA,
    //
    K_R9		= 0xFFDA,
    //
    K_F30		= 0xFFDB,
    //
    K_R10		= 0xFFDB,
    //
    K_F31		= 0xFFDC,
    //
    K_R11		= 0xFFDC,
    //
    K_F32		= 0xFFDD,
    //
    K_R12		= 0xFFDD,
    //
    K_F33		= 0xFFDE,
    //
    K_R13		= 0xFFDE,
    //
    K_F34		= 0xFFDF,
    //
    K_R14		= 0xFFDF,
    //
    K_F35		= 0xFFE0,
    //
    K_R15		= 0xFFE0,

    //
    K_Shift_L		= 0xFFE1,
    //
    K_Shift_R		= 0xFFE2,
    //
    K_Control_L		= 0xFFE3,
    //
    K_Control_R		= 0xFFE4,
    //
    K_Caps_Lock		= 0xFFE5,
    //
    K_Shift_Lock	= 0xFFE6,

    //
    K_Meta_L		= 0xFFE7,
    //
    K_Meta_R		= 0xFFE8,
    //
    K_Alt_L		= 0xFFE9,
    //
    K_Alt_R		= 0xFFEA,
    //
    K_Super_L		= 0xFFEB,
    //
    K_Super_R		= 0xFFEC,
    //
    K_Hyper_L		= 0xFFED,
    //
    K_Hyper_R		= 0xFFEE,
    //

    K_Pointer_Button1		= 0xFEE9,
    //
    K_Pointer_Button2		= 0xFEEA,
    //
    K_Pointer_Button3		= 0xFEEB,
    //
    K_Pointer_Button4		= 0xFEEC,
    //
    K_Pointer_Button5		= 0xFEED,
    //

    K_Pointer_DblClick_Dflt	= 0xFEEE,
    //
    K_Pointer_DblClick1		= 0xFEEF,
    //
    K_Pointer_DblClick2		= 0xFEF0,
    //
    K_Pointer_DblClick3		= 0xFEF1,
    //
    K_Pointer_DblClick4		= 0xFEF2,
    //
    K_Pointer_DblClick5		= 0xFEF3,
    //
    K_Pointer_Drag_Dflt		= 0xFEF4,
    //
    K_Pointer_Drag1		= 0xFEF5,
    //
    K_Pointer_Drag2		= 0xFEF6,
    //
    K_Pointer_Drag3		= 0xFEF7,
    //
    K_Pointer_Drag4		= 0xFEF8,
    //

    //
    K_Pointer_EnableKeys	= 0xFEF9,
    //
    K_Pointer_Accelerate	= 0xFEFA,
    //
    K_Pointer_DfltBtnNext	= 0xFEFB,
    //
    K_Pointer_DfltBtnPrev	= 0xFEFC,

    //  Latin 1  Byte 3 = 0
    K_space               = 0x0020,
    //
    K_exclam              = 0x0021,
    //
    K_quotedbl            = 0x0022,
    //
    K_numbersign          = 0x0023,
    //
    K_dollar              = 0x0024,
    //
    K_percent             = 0x0025,
    //
    K_ampersand           = 0x0026,
    //
    K_apostrophe          = 0x0027,
    //
    K_parenleft           = 0x0028,
    //
    K_parenright          = 0x0029,
    //
    K_asterisk            = 0x002a,
    //
    K_plus                = 0x002b,
    //
    K_comma               = 0x002c,
    //
    K_minus               = 0x002d,
    //
    K_period              = 0x002e,
    //
    K_slash               = 0x002f,
    //
    K_0                   = 0x0030,
    //
    K_1                   = 0x0031,
    //
    K_2                   = 0x0032,
    //
    K_3                   = 0x0033,
    //
    K_4                   = 0x0034,
    //
    K_5                   = 0x0035,
    //
    K_6                   = 0x0036,
    //
    K_7                   = 0x0037,
    //
    K_8                   = 0x0038,
    //
    K_9                   = 0x0039,
    //
    K_colon               = 0x003a,
    //
    K_semicolon           = 0x003b,
    //
    K_less                = 0x003c,
    //
    K_equal               = 0x003d,
    //
    K_greater             = 0x003e,
    //
    K_question            = 0x003f,
    //
    K_at                  = 0x0040,
    //
    K_A                   = 0x0041,
    //
    K_B                   = 0x0042,
    //
    K_C                   = 0x0043,
    //
    K_D                   = 0x0044,
    //
    K_E                   = 0x0045,
    //
    K_F                   = 0x0046,
    //
    K_G                   = 0x0047,
    //
    K_H                   = 0x0048,
    //
    K_I                   = 0x0049,
    //
    K_J                   = 0x004a,
    //
    K_K                   = 0x004b,
    //
    K_L                   = 0x004c,
    //
    K_M                   = 0x004d,
    //
    K_N                   = 0x004e,
    //
    K_O                   = 0x004f,
    //
    K_P                   = 0x0050,
    //
    K_Q                   = 0x0051,
    //
    K_R                   = 0x0052,
    //
    K_S                   = 0x0053,
    //
    K_T                   = 0x0054,
    //
    K_U                   = 0x0055,
    //
    K_V                   = 0x0056,
    //
    K_W                   = 0x0057,
    //
    K_X                   = 0x0058,
    //
    K_Y                   = 0x0059,
    //
    K_Z                   = 0x005a,
    //
    K_bracketleft         = 0x005b,
    //
    K_backslash           = 0x005c,
    //
    K_bracketright        = 0x005d,
    //
    K_asciicircum         = 0x005e,
    //
    K_underscore          = 0x005f,
    //
    K_grave               = 0x0060,
    //
    K_quoteleft           = 0x0060,
    //
    K_a                   = 0x0061,
    //
    K_b                   = 0x0062,
    //
    K_c                   = 0x0063,
    //
    K_d                   = 0x0064,
    //
    K_e                   = 0x0065,
    //
    K_f                   = 0x0066,
    //
    K_g                   = 0x0067,
    //
    K_h                   = 0x0068,
    //
    K_i                   = 0x0069,
    //
    K_j                   = 0x006a,
    //
    K_k                   = 0x006b,
    //
    K_l                   = 0x006c,
    //
    K_m                   = 0x006d,
    //
    K_n                   = 0x006e,
    //
    K_o                   = 0x006f,
    //
    K_p                   = 0x0070,
    //
    K_q                   = 0x0071,
    //
    K_r                   = 0x0072,
    //
    K_s                   = 0x0073,
    //
    K_t                   = 0x0074,
    //
    K_u                   = 0x0075,
    //
    K_v                   = 0x0076,
    //
    K_w                   = 0x0077,
    //
    K_x                   = 0x0078,
    //
    K_y                   = 0x0079,
    //
    K_z                   = 0x007a,
    //
    K_braceleft           = 0x007b,
    //
    K_bar                 = 0x007c,
    //
    K_braceright          = 0x007d,
    //
    K_asciitilde          = 0x007e,
    //

    //
    K_nobreakspace        = 0x00a0,
    //
    K_exclamdown          = 0x00a1,
    //
    K_cent        	= 0x00a2,
    //
    K_sterling            = 0x00a3,
    //
    K_currency            = 0x00a4,
    //
    K_yen                 = 0x00a5,
    //
    K_brokenbar           = 0x00a6,
    //
    K_section             = 0x00a7,
    //
    K_diaeresis           = 0x00a8,
    //
    K_copyright           = 0x00a9,
    //
    K_ordfeminine         = 0x00aa,
    //
    K_guillemotleft       = 0x00ab,
    //
    K_notsign             = 0x00ac,
    //
    K_hyphen              = 0x00ad,
    //
    K_registered          = 0x00ae,
    //
    K_macron              = 0x00af,
    //
    K_degree              = 0x00b0,
    //
    K_plusminus           = 0x00b1,
    //
    K_twosuperior         = 0x00b2,
    //
    K_threesuperior       = 0x00b3,
    //
    K_acute               = 0x00b4,
    //
    K_mu                  = 0x00b5,
    //
    K_paragraph           = 0x00b6,
    //
    K_periodcentered      = 0x00b7,
    //
    K_cedilla             = 0x00b8,
    //
    K_onesuperior         = 0x00b9,
    //
    K_masculine           = 0x00ba,
    //
    K_guillemotright      = 0x00bb,
    //
    K_onequarter          = 0x00bc,
    //
    K_onehalf             = 0x00bd,
    //
    K_threequarters       = 0x00be,
    //
    K_questiondown        = 0x00bf,
    //
    K_Agrave              = 0x00c0,
    //
    K_Aacute              = 0x00c1,
    //
    K_Acircumflex         = 0x00c2,
    //
    K_Atilde              = 0x00c3,
    //
    K_Adiaeresis          = 0x00c4,
    //
    K_Aring               = 0x00c5,
    //
    K_AE                  = 0x00c6,
    //
    K_Ccedilla            = 0x00c7,
    //
    K_Egrave              = 0x00c8,
    //
    K_Eacute              = 0x00c9,
    //
    K_Ecircumflex         = 0x00ca,
    //
    K_Ediaeresis          = 0x00cb,
    //
    K_Igrave              = 0x00cc,
    //
    K_Iacute              = 0x00cd,
    //
    K_Icircumflex         = 0x00ce,
    //
    K_Idiaeresis          = 0x00cf,
    //
    K_ETH                 = 0x00d0,
    //
    K_Ntilde              = 0x00d1,
    //
    K_Ograve              = 0x00d2,
    //
    K_Oacute              = 0x00d3,
    //
    K_Ocircumflex         = 0x00d4,
    //
    K_Otilde              = 0x00d5,
    //
    K_Odiaeresis          = 0x00d6,
    //
    K_multiply            = 0x00d7,
    //
    K_Ooblique            = 0x00d8,
    //
    K_Ugrave              = 0x00d9,
    //
    K_Uacute              = 0x00da,
    //
    K_Ucircumflex         = 0x00db,
    //
    K_Udiaeresis          = 0x00dc,
    //
    K_Yacute              = 0x00dd,
    //
    K_THORN               = 0x00de,
    //
    K_ssharp              = 0x00df,
    //
    K_agrave              = 0x00e0,
    //
    K_aacute              = 0x00e1,
    //
    K_acircumflex         = 0x00e2,
    //
    K_atilde              = 0x00e3,
    //
    K_adiaeresis          = 0x00e4,
    //
    K_aring               = 0x00e5,
    //
    K_ae                  = 0x00e6,
    //
    K_ccedilla            = 0x00e7,
    //
    K_egrave              = 0x00e8,
    //
    K_eacute              = 0x00e9,
    //
    K_ecircumflex         = 0x00ea,
    //
    K_ediaeresis          = 0x00eb,
    //
    K_igrave              = 0x00ec,
    //
    K_iacute              = 0x00ed,
    //
    K_icircumflex         = 0x00ee,
    //
    K_idiaeresis          = 0x00ef,
    //
    K_eth                 = 0x00f0,
    //
    K_ntilde              = 0x00f1,
    //
    K_ograve              = 0x00f2,
    //
    K_oacute              = 0x00f3,
    //
    K_ocircumflex         = 0x00f4,
    //
    K_otilde              = 0x00f5,
    //
    K_odiaeresis          = 0x00f6,
    //
    K_division            = 0x00f7,
    //
    K_oslash              = 0x00f8,
    //
    K_ugrave              = 0x00f9,
    //
    K_uacute              = 0x00fa,
    //
    K_ucircumflex         = 0x00fb,
    //
    K_udiaeresis          = 0x00fc,
    //
    K_yacute              = 0x00fd,
    //
    K_thorn               = 0x00fe,
    //
    K_ydiaeresis          = 0x00ff,
    //

    //   Latin 2
    //   Byte 3 = 1

    K_Aogonek             = 0x01a1,
    //
    K_breve               = 0x01a2,
    //
    K_Lstroke             = 0x01a3,
    //
    K_Lcaron              = 0x01a5,
    //
    K_Sacute              = 0x01a6,
    //
    K_Scaron              = 0x01a9,
    //
    K_Scedilla            = 0x01aa,
    //
    K_Tcaron              = 0x01ab,
    //
    K_Zacute              = 0x01ac,
    //
    K_Zcaron              = 0x01ae,
    //
    K_Zabovedot           = 0x01af,
    //
    K_aogonek             = 0x01b1,
    //
    K_ogonek              = 0x01b2,
    //
    K_lstroke             = 0x01b3,
    //
    K_lcaron              = 0x01b5,
    //
    K_sacute              = 0x01b6,
    //
    K_caron               = 0x01b7,
    //
    K_scaron              = 0x01b9,
    //
    K_scedilla            = 0x01ba,
    //
    K_tcaron              = 0x01bb,
    //
    K_zacute              = 0x01bc,
    //
    K_doubleacute         = 0x01bd,
    //
    K_zcaron              = 0x01be,
    //
    K_zabovedot           = 0x01bf,
    //
    K_Racute              = 0x01c0,
    //
    K_Abreve              = 0x01c3,
    //
    K_Lacute              = 0x01c5,
    //
    K_Cacute              = 0x01c6,
    //
    K_Ccaron              = 0x01c8,
    //
    K_Eogonek             = 0x01ca,
    //
    K_Ecaron              = 0x01cc,
    //
    K_Dcaron              = 0x01cf,
    //
    K_Dstroke             = 0x01d0,
    //
    K_Nacute              = 0x01d1,
    //
    K_Ncaron              = 0x01d2,
    //
    K_Odoubleacute        = 0x01d5,
    //
    K_Rcaron              = 0x01d8,
    //
    K_Uring               = 0x01d9,
    //
    K_Udoubleacute        = 0x01db,
    //
    K_Tcedilla            = 0x01de,
    //
    K_racute              = 0x01e0,
    //
    K_abreve              = 0x01e3,
    //
    K_lacute              = 0x01e5,
    //
    K_cacute              = 0x01e6,
    //
    K_ccaron              = 0x01e8,
    //
    K_eogonek             = 0x01ea,
    //
    K_ecaron              = 0x01ec,
    //
    K_dcaron              = 0x01ef,
    //
    K_dstroke             = 0x01f0,
    //
    K_nacute              = 0x01f1,
    //
    K_ncaron              = 0x01f2,
    //
    K_odoubleacute        = 0x01f5,
    //
    K_udoubleacute        = 0x01fb,
    //
    K_rcaron              = 0x01f8,
    //
    K_uring               = 0x01f9,
    //
    K_tcedilla            = 0x01fe,
    //
    K_abovedot            = 0x01ff,
    //

    //   Latin 3  Byte 3 = 2

    K_Hstroke             = 0x02a1,
    //
    K_Hcircumflex         = 0x02a6,
    //
    K_Iabovedot           = 0x02a9,
    //
    K_Gbreve              = 0x02ab,
    //
    K_Jcircumflex         = 0x02ac,
    //
    K_hstroke             = 0x02b1,
    //
    K_hcircumflex         = 0x02b6,
    //
    K_idotless            = 0x02b9,
    //
    K_gbreve              = 0x02bb,
    //
    K_jcircumflex         = 0x02bc,
    //
    K_Cabovedot           = 0x02c5,
    //
    K_Ccircumflex         = 0x02c6,
    //
    K_Gabovedot           = 0x02d5,
    //
    K_Gcircumflex         = 0x02d8,
    //
    K_Ubreve              = 0x02dd,
    //
    K_Scircumflex         = 0x02de,
    //
    K_cabovedot           = 0x02e5,
    //
    K_ccircumflex         = 0x02e6,
    //
    K_gabovedot           = 0x02f5,
    //
    K_gcircumflex         = 0x02f8,
    //
    K_ubreve              = 0x02fd,
    //
    K_scircumflex         = 0x02fe,

    //   Latin 4  Byte 3 = 3

    K_kra                 = 0x03a2,
    //
    K_Rcedilla            = 0x03a3,
    //
    K_Itilde              = 0x03a5,
    //
    K_Lcedilla            = 0x03a6,
    //
    K_Emacron             = 0x03aa,
    //
    K_Gcedilla            = 0x03ab,
    //
    K_Tslash              = 0x03ac,
    //
    K_rcedilla            = 0x03b3,
    //
    K_itilde              = 0x03b5,
    //
    K_lcedilla            = 0x03b6,
    //
    K_emacron             = 0x03ba,
    //
    K_gcedilla            = 0x03bb,
    //
    K_tslash              = 0x03bc,
    //
    K_ENG                 = 0x03bd,
    //
    K_eng                 = 0x03bf,
    //
    K_Amacron             = 0x03c0,
    //
    K_Iogonek             = 0x03c7,
    //
    K_Eabovedot           = 0x03cc,
    //
    K_Imacron             = 0x03cf,
    //
    K_Ncedilla            = 0x03d1,
    //
    K_Omacron             = 0x03d2,
    //
    K_Kcedilla            = 0x03d3,
    //
    K_Uogonek             = 0x03d9,
    //
    K_Utilde              = 0x03dd,
    //
    K_Umacron             = 0x03de,
    //
    K_amacron             = 0x03e0,
    //
    K_iogonek             = 0x03e7,
    //
    K_eabovedot           = 0x03ec,
    //
    K_imacron             = 0x03ef,
    //
    K_ncedilla            = 0x03f1,
    //
    K_omacron             = 0x03f2,
    //
    K_kcedilla            = 0x03f3,
    //
    K_uogonek             = 0x03f9,
    //
    K_utilde              = 0x03fd,
    //
    K_umacron             = 0x03fe,

    // Greek Byte 3 = 7

    K_Greek_ALPHAaccent            = 0x07a1,
    //
    K_Greek_EPSILONaccent          = 0x07a2,
    //
    K_Greek_ETAaccent              = 0x07a3,
    //
    K_Greek_IOTAaccent             = 0x07a4,
    //
    K_Greek_IOTAdiaeresis          = 0x07a5,
    //
    K_Greek_OMICRONaccent          = 0x07a7,
    //
    K_Greek_UPSILONaccent          = 0x07a8,
    //
    K_Greek_UPSILONdieresis        = 0x07a9,
    //
    K_Greek_OMEGAaccent            = 0x07ab,
    //
    K_Greek_accentdieresis         = 0x07ae,
    //
    K_Greek_horizbar               = 0x07af,
    //
    K_Greek_alphaaccent            = 0x07b1,
    //
    K_Greek_epsilonaccent          = 0x07b2,
    //
    K_Greek_etaaccent              = 0x07b3,
    //
    K_Greek_iotaaccent             = 0x07b4,
    //
    K_Greek_iotadieresis           = 0x07b5,
    //
    K_Greek_iotaaccentdieresis     = 0x07b6,
    //
    K_Greek_omicronaccent          = 0x07b7,
    //
    K_Greek_upsilonaccent          = 0x07b8,
    //
    K_Greek_upsilondieresis        = 0x07b9,
    //
    K_Greek_upsilonaccentdieresis  = 0x07ba,
    //
    K_Greek_omegaaccent            = 0x07bb,
    //
    K_Greek_ALPHA                  = 0x07c1,
    //
    K_Greek_BETA                   = 0x07c2,
    //
    K_Greek_GAMMA                  = 0x07c3,
    //
    K_Greek_DELTA                  = 0x07c4,
    //
    K_Greek_EPSILON                = 0x07c5,
    //
    K_Greek_ZETA                   = 0x07c6,
    //
    K_Greek_ETA                    = 0x07c7,
    //
    K_Greek_THETA                  = 0x07c8,
    //
    K_Greek_IOTA                   = 0x07c9,
    //
    K_Greek_KAPPA                  = 0x07ca,
    //
    K_Greek_LAMDA                  = 0x07cb,
    //
    K_Greek_LAMBDA                 = 0x07cb,
    //
    K_Greek_MU                     = 0x07cc,
    //
    K_Greek_NU                     = 0x07cd,
    //
    K_Greek_XI                     = 0x07ce,
    //
    K_Greek_OMICRON                = 0x07cf,
    //
    K_Greek_PI                     = 0x07d0,
    //
    K_Greek_RHO                    = 0x07d1,
    //
    K_Greek_SIGMA                  = 0x07d2,
    //
    K_Greek_TAU                    = 0x07d4,
    //
    K_Greek_UPSILON                = 0x07d5,
    //
    K_Greek_PHI                    = 0x07d6,
    //
    K_Greek_CHI                    = 0x07d7,
    //
    K_Greek_PSI                    = 0x07d8,
    //
    K_Greek_OMEGA                  = 0x07d9,
    //
    K_Greek_alpha                  = 0x07e1,
    //
    K_Greek_beta                   = 0x07e2,
    //
    K_Greek_gamma                  = 0x07e3,
    //
    K_Greek_delta                  = 0x07e4,
    //
    K_Greek_epsilon                = 0x07e5,
    //
    K_Greek_zeta                   = 0x07e6,
    //
    K_Greek_eta                    = 0x07e7,
    //
    K_Greek_theta                  = 0x07e8,
    //
    K_Greek_iota                   = 0x07e9,
    //
    K_Greek_kappa                  = 0x07ea,
    //
    K_Greek_lamda                  = 0x07eb,
    //
    K_Greek_lambda                 = 0x07eb,
    //
    K_Greek_mu                     = 0x07ec,
    //
    K_Greek_nu                     = 0x07ed,
    //
    K_Greek_xi                     = 0x07ee,
    //
    K_Greek_omicron                = 0x07ef,
    //
    K_Greek_pi                     = 0x07f0,
    //
    K_Greek_rho                    = 0x07f1,
    //
    K_Greek_sigma                  = 0x07f2,
    //
    K_Greek_finalsmallsigma        = 0x07f3,
    //
    K_Greek_tau                    = 0x07f4,
    //
    K_Greek_upsilon                = 0x07f5,
    //
    K_Greek_phi                    = 0x07f6,
    //
    K_Greek_chi                    = 0x07f7,
    //
    K_Greek_psi                    = 0x07f8,
    //
    K_Greek_omega                  = 0x07f9,
    //  Alias for mode_switch 
    K_Greek_switch                 = 0xFF7E
  };

  // Determine Display::KeyModifier given Display::KeySym.
  static Display::KeyModifier keyModifierFromKeySym(const Display::KeySym 
						    &keysym);

};

// remove this when all IL's are extracted ???
typedef Display IL;

// <summary>
// Global functions (operators) to print display class enumerations.
// </summary>

// Each enumeration can be printed to the console to assist
// in debugging.  Printed name is identical to source code
// constant name.
// <group name="DisplayEnums to string functions">
ostream & operator << (ostream & os, Display::ColorModel dmt);
ostream & operator << (ostream & os, Display::Strategy dmt);
ostream & operator << (ostream & os, Display::SpecialMap dmt);
ostream & operator << (ostream & os, Display::LineStyle dmt);
ostream & operator << (ostream & os, Display::CapStyle dmt);
ostream & operator << (ostream & os, Display::JoinStyle dmt);
ostream & operator << (ostream & os, Display::FillStyle dmt);
ostream & operator << (ostream & os, Display::FillRule dmt);
ostream & operator << (ostream & os, Display::ArcMode dmt);
ostream & operator << (ostream & os, Display::TextAlign dmt);
ostream & operator << (ostream & os, Display::DrawMode dmt);
ostream & operator << (ostream & os, Display::DrawBuffer dmt);
ostream & operator << (ostream & os, Display::DrawFunction dmt);
ostream & operator << (ostream & os, Display::RefreshReason drr);
ostream & operator << (ostream & os, Display::Option dopt);
ostream & operator << (ostream & os, Display::ImageCacheStrategy ics);
ostream & operator << (ostream & os, Display::ComplexToRealMethod ctrm);
ostream & operator << (ostream & os, Display::DisplayDataType dtype);
ostream & operator << (ostream & os, Display::KeyModifier dmt);
ostream & operator << (ostream & os, Display::KeySym dmt);
ostream & operator << (ostream & os, Display::Marker mrk);
// </group>


} //# NAMESPACE CASA - END

#endif








