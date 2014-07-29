// Copyright (C) 1999-2010
// Smithsonian Astrophysical Observatory, Cambridge, MA, USA
// For conditions of distribution and use, see copyright notice in "copyright"

%pure-parser
/* The driver is passed by reference to the parser and to the scanner. This
 * provides a simple but effective pure interface, not relying on global
 * variables. */
%parse-param { class ds9parser &driver }
%parse-param { class ds9context *fr }
//%lex-param {ds9lex* ll}
//%parse-param {ds9lex* ll}

%require "2.3"
%debug
%defines
%skeleton "lalr1.cc"

%name-prefix="casa::viewer"
%define "parser_class_name" "ds9parse"

%{
#ifdef yylex
#undef yylex
#endif

#define YYDEBUG 1
// from marker.h...
#define POINTSIZE 11

#define FITSPTR (fr)
#define DISCARD_(x) {yyclearin; driver.lexer->discard(x);}

#include <math.h>
#include <string.h>
#include <iostream>

//#include "base.h"
//#include "fitsimage.h"
//#include "util.h"
//#include "vector.h"
//#include "list.h"
//#include "basemarker.h"
//#include "coord.h"

#include <display/ds9/ds9lex.h>
#include <display/ds9/ds9parser.h>

#include <list>

const int MAXANNULI = 512;
const int MAXANGLES = 720;

//extern int mklex(void*, ds9lex*);
//extern void mkDiscard(int);

static casa::viewer::CoordSystem globalSystem;
static casa::viewer::CoordSystem globalWCS;
static casa::viewer::SkyFrame globalSky;
static casa::viewer::CoordSystem localSystem;
static casa::viewer::SkyFrame localSky;

static int globalTile;

static unsigned short globalProps;
static unsigned short localProps;

static int globalDash[2];
static int localDash[2];

static int globalWidth;
static int localWidth;

static char globalColor[16];
static char localColor[16];

static char globalFont[32];
static char localFont[32];

static char globalText[80];
static char localText[80];

static char localComment[80];

static int globalLine1;
static int localLine1;
static int globalLine2;
static int localLine2;

static int globalVector;
static int localVector;

static int globalComposite;
static int localComposite;

static int globalPoint;
static int localPoint;
static int globalPointSize;
static int localPointSize;

static double globalTextAngle;
static double localTextAngle;
static int globalTextRotate;
static int localTextRotate;

static casa::viewer::CoordSystem globalRulerCoordSystem;
static casa::viewer::CoordSystem localRulerCoordSystem;
static casa::viewer::SkyFrame globalRulerSkyFrame;
static casa::viewer::SkyFrame localRulerSkyFrame;
static casa::viewer::CoordSystem globalRulerDistSystem;
static casa::viewer::CoordSystem localRulerDistSystem;
static casa::viewer::SkyFormat globalRulerDistFormat;
static casa::viewer::SkyFormat localRulerDistFormat;

static casa::viewer::CoordSystem globalCompassCoordSystem;
static casa::viewer::SkyFrame globalCompassSkyFrame;
static char globalCompassNorth[80];
static char globalCompassEast[80];
static int globalCompassNArrow;
static int globalCompassEArrow;
static casa::viewer::CoordSystem localCompassCoordSystem;
static casa::viewer::SkyFrame localCompassSkyFrame;
static char localCompassNorth[80];
static char localCompassEast[80];
static int localCompassNArrow;
static int localCompassEArrow;

static int localCpanda;
static int localEpanda;
static int localBpanda;

static std::list<casa::viewer::Vertex> polylist;
static std::list<casa::viewer::Tag> taglist;

static double aAnnuli[MAXANNULI];
static casa::viewer::Vector aVector[MAXANNULI];
static int aNum;
static int aNumsao;
static int aStatus;
static int cStatus;
static casa::viewer::Vector aCenter;
static double aAngles[MAXANGLES];
static int aAngNum;
static double aAngle;
static unsigned short aProps;
static char aColor[16];
static int aWidth;
static int aDash[2];
static char aFont[32];
static char aText[80];
static char aComment[80];

static void setProps(unsigned short* props, unsigned short prop, int value);
static casa::viewer::CoordSystem checkWCSSystem();
static casa::viewer::SkyFrame checkWCSSky();

enum {CIRCLE,BOX,DIAMOND,CROSS,XPT,ARROW,BOXCIRCLE};
%}

/* keep track of the current position within the input */
%locations
%initial-action
{
    // initialize the initial location object
    @$.begin.filename = @$.end.filename = &driver.streamname;
};

/* verbose error messages */
%error-verbose

%union {
#define MKBUFSIZE 2048
  double real;
  int integer;
  char str[MKBUFSIZE];
  double vector[3];
}

%type <real> numeric
%type <integer> yesno

%type <real> angle
%type <real> optangle
%type <real> value
%type <vector> vvalue
%type <real> sexagesimal
%type <real> hms
%type <real> dms
%type <vector> coord
%type <integer> coordSystem
%type <integer> wcsSystem
%type <integer> skyFrame
%type <integer> skyFormat
%type <integer> property
%type <integer> callBack
%type <integer> pointProp
%type <integer> numberof

%token <integer> INT
%token <real> REAL
%token <str> STRING

%token <real> ANGDEGREE
%token <real> ANGRADIAN
%token <real> ARCMINUTE
%token <real> ARCSECOND
%token <real> PHYCOORD
%token <real> IMGCOORD

%token <str> SEXSTR
%token <str> HMSSTR
%token <str> DMSSTR

%token EOF_
%token EOL_

%token AMPLIFIER_
%token ANNULUS_
%token ARCMIN_
%token ARCSEC_
%token ARROW_
%token B1950_
%token BACKGROUND_
%token BEGIN_
%token BOX_
%token BOXCIRCLE_
%token BPANDA_
%token CALLBACK_
%token CIRCLE_
%token CIRCLE3D_
%token COLOR_
%token COMPASS_
%token COMPOSITE_
%token CPANDA_
%token CROSS_
%token DASH_
%token DASHLIST_
%token DEBUG_
%token DEGREES_
%token DELETE_
%token DETECTOR_
%token DIAMOND_
%token ECLIPTIC_
%token EDIT_
%token ELLIPSE_
%token END_
%token EPANDA_
%token FALSE_
%token FIELD_
%token FIXED_
%token FK4_
%token FK5_
%token FONT_
%token GALACTIC_
%token GLOBAL_
%token HIGHLITE_
%token ICRS_
%token IGNORE_
%token IMAGE_
%token INCLUDE_
%token J2000_
%token KEY_
%token LINE_
%token LINEAR_
%token MOVE_
%token N_
%token NO_
%token OFF_
%token ON_
%token PHYSICAL_
%token PIE_
%token PIXELS_
%token POINT_
%token POLYGON_
%token PROJECTION_
%token PROJECTION3D_
%token PROPERTY_
%token ROTATE_
%token ROTBOX_
%token RULER_
%token SELECT_
%token SOURCE_
%token TAG_
%token TEXT_
%token TEXTANGLE_
%token TEXTROTATE_
%token TILE_
%token TRUE_
%token VECTOR_
%token VERSION_
%token UNHIGHLITE_
%token UNSELECT_
%token UPDATE_
%token WCS_
%token WCSA_
%token WCSB_
%token WCSC_
%token WCSD_
%token WCSE_
%token WCSF_
%token WCSG_
%token WCSH_
%token WCSI_
%token WCSJ_
%token WCSK_
%token WCSL_
%token WCSM_
%token WCSN_
%token WCSO_
%token WCSP_
%token WCSQ_
%token WCSR_
%token WCSS_
%token WCST_
%token WCSU_
%token WCSV_
%token WCSW_
%token WCSX_
%token WCSY_
%token WCSZ_
%token WCS0_
%token WIDTH_
%token X_
%token Y_
%token YES_

%{
#include <display/ds9/ds9lex.h>

/* this "connects" the bison parser in the driver to the flex scanner class
 * object. it defines the yylex() function call to pull the next token from the
 * current lexer object of the driver context. */
#undef yylex
#define yylex driver.lexer->lex
%}

%%

start	: initGlobal commands postLocal
	;

commands: commands command terminator
	| command terminator
	;

command : /* empty */
	| DEBUG_ debug
	| VERSION_ {std::cerr << "DS9 Regions File 3.1" << std::endl;}

	| GLOBAL_ global comment
	| TILE_ INT {globalTile = $2;}

	| coordSystem {globalSystem=(casa::viewer::CoordSystem)$1;} comment
	| skyFrame {globalSystem=globalWCS; globalSky=(casa::viewer::SkyFrame)$1;} comment
	| LINEAR_ {globalSystem=globalWCS; globalSky=NATIVEWCS;} comment

	| initLocal shape
	| initLocal include shape
	| initLocal '#' hash
	;

hash	: nonshape
	| include nonshape
	| TILE_ INT {globalTile = $2;}
	| {DISCARD_(1);} STRING
	;

comment : /* empty */
	| '#' {DISCARD_(1);} STRING
	;

shapeComment : /* empty */ postLocal
	| '#' {DISCARD_(1);} STRING postLocal {strncpy(localComment,$3,80);}
	| '#' local postLocal
	| '#' local {DISCARD_(1);} STRING postLocal
	  {strncpy(localComment,$4,80);}
	;

nonshapeComment : /* empty */ postLocal
	| {DISCARD_(1);} STRING postLocal {strncpy(localComment,$2,80);}
	| local postLocal
	| local {DISCARD_(1);} STRING postLocal {strncpy(localComment,$3,80);}
	;

terminator: EOL_
	| EOF_ {YYACCEPT;}
	;

numeric	: REAL {$$=$1;}
	| INT {$$=$1;}
	;

debug	: ON_ {set_debug_level(1);}
	| OFF_ {set_debug_level(0);}
	;

yesno	: INT {$$=($1 ? 1 : 0);}

	| YES_ {$$=1;}
	| Y_ {$$=1;}
	| ON_ {$$=1;}
	| TRUE_ {$$=1;}

	| NO_ {$$=0;}
	| N_ {$$=0;}
	| OFF_ {$$=0;}
	| FALSE_ {$$=0;}
	;

sp	: /* empty */
	| ','
	;

bp	: /* empty */
	| '('
	;

ep	: /* emtpy */
	| ')'
	;

conjuction : /* empty */ {cStatus = 0;}
	| '|' {cStatus = 1;}
	| '|' '|' {cStatus = 1;}
	;

optangle: /* empty */ {$$ = fr->mapAngleToRef(0,localSystem,localSky);}
	| angle {$$ = $1;}
	;

angle	: numeric {$$ = fr->mapAngleToRef(degToRad($1),localSystem,localSky);}
	| ANGDEGREE {$$ = fr->mapAngleToRef(degToRad($1),localSystem,localSky);}
	| ANGRADIAN {$$ = fr->mapAngleToRef($1,localSystem,localSky);}
	;

value	: numeric {$$ = FITSPTR->mapLenToRef($1, localSystem, DEGREES);}
	| PHYCOORD {$$ = FITSPTR->mapLenToRef($1, PHYSICAL);}
	| IMGCOORD {$$ = FITSPTR->mapLenToRef($1, IMAGE);}
	| ANGDEGREE {$$ = FITSPTR->mapLenToRef($1, checkWCSSystem(), DEGREES);}
	| ARCMINUTE {$$ = FITSPTR->mapLenToRef($1, checkWCSSystem(), ARCMIN);}
	| ARCSECOND {$$ = FITSPTR->mapLenToRef($1, checkWCSSystem(), ARCSEC);}
	;

vvalue	: numeric sp numeric 
	{
	  casa::viewer::Vector r = FITSPTR->mapLenToRef(doubletovec($1,$3), localSystem, DEGREES);
	  $$[0] = r[0];
	  $$[1] = r[1];
	  $$[2] = r[2];
	}
	| PHYCOORD sp PHYCOORD 
	{
	  casa::viewer::Vector r = FITSPTR->mapLenToRef(doubletovec($1,$3), PHYSICAL);
	  $$[0] = r[0];
	  $$[1] = r[1];
	  $$[2] = r[2];
	}
	| IMGCOORD sp IMGCOORD 
	{
	  casa::viewer::Vector r = FITSPTR->mapLenToRef(doubletovec($1,$3), IMAGE);
	  $$[0] = r[0];
	  $$[1] = r[1];
	  $$[2] = r[2];
	}
	| ANGDEGREE sp ANGDEGREE
	{
	  casa::viewer::Vector r=FITSPTR->mapLenToRef(doubletovec($1,$3),checkWCSSystem(),DEGREES);
	  $$[0] = r[0];
	  $$[1] = r[1];
	  $$[2] = r[2];
	}
	| ARCMINUTE sp ARCMINUTE 
	{
	  casa::viewer::Vector r=FITSPTR->mapLenToRef(doubletovec($1,$3),checkWCSSystem(),ARCMIN);
	  $$[0] = r[0];
	  $$[1] = r[1];
	  $$[2] = r[2];
	}
	| ARCSECOND sp ARCSECOND 
	{
	  casa::viewer::Vector r=FITSPTR->mapLenToRef(doubletovec($1,$3),checkWCSSystem(),ARCSEC);
	  $$[0] = r[0];
	  $$[1] = r[1];
	  $$[2] = r[2];
	}
	;

numberof: N_ '=' INT {$$ = $3;}
	;

sexagesimal: SEXSTR {$$ = parseSEXStr($1);}
	;

hms	: HMSSTR {$$ = parseHMSStr($1);}
	;

dms	: DMSSTR {$$ = parseDMSStr($1);}
	;

coord	: sexagesimal sp sexagesimal
	{
	  casa::viewer::Vector r;
	  casa::viewer::CoordSystem sys = checkWCSSystem();
	  casa::viewer::SkyFrame sky = checkWCSSky();
	  if (sky == GALACTIC || sky == ECLIPTIC) 
	    r = FITSPTR->mapToRef(doubletovec($1,$3), sys, sky);
	  else
	    r = FITSPTR->mapToRef(doubletovec($1*360./24.,$3), sys, sky);
	  $$[0] = r[0];
	  $$[1] = r[1];
	  $$[2] = r[2];
	}
	| hms sp dms
        {
	  casa::viewer::Vector r = FITSPTR->mapToRef(doubletovec($1,$3),
	  checkWCSSystem(), checkWCSSky());
	  $$[0] = r[0];
	  $$[1] = r[1];
	  $$[2] = r[2];
	}
	| dms sp dms
        {
	  casa::viewer::Vector r = FITSPTR->mapToRef(doubletovec($1,$3),
	  checkWCSSystem(), checkWCSSky());
	  $$[0] = r[0];
	  $$[1] = r[1];
	  $$[2] = r[2];
	}
	| numeric sp numeric 
	{
	  casa::viewer::Vector r = FITSPTR->mapToRef(doubletovec($1,$3), localSystem, localSky);
	  $$[0] = r[0];
	  $$[1] = r[1];
	  $$[2] = r[2];
	}
	| ANGDEGREE sp ANGDEGREE
	{
	  casa::viewer::Vector r = FITSPTR->mapToRef(doubletovec($1,$3), 
	    checkWCSSystem(), checkWCSSky());
	  $$[0] = r[0];
	  $$[1] = r[1];
	  $$[2] = r[2];
	}
	| IMGCOORD sp IMGCOORD
	{
	  casa::viewer::Vector r = FITSPTR->mapToRef(doubletovec($1,$3), IMAGE);
	  $$[0] = r[0];
	  $$[1] = r[1];
	  $$[2] = r[2];
	}
	| PHYCOORD sp PHYCOORD
	{
	  casa::viewer::Vector r = FITSPTR->mapToRef(doubletovec($1,$3), PHYSICAL);
	  $$[0] = r[0];
	  $$[1] = r[1];
	}
	;

coordSystem :IMAGE_ {$$ = IMAGE;}
	| PHYSICAL_ {$$ = PHYSICAL;}
	| DETECTOR_ {$$ = DETECTOR;}
	| AMPLIFIER_ {$$ = AMPLIFIER;}
	| wcsSystem {$$ = $1; globalWCS = (casa::viewer::CoordSystem)$1;}
	;

wcsSystem : WCS_ {$$ = WCS;}
	| WCSA_ {$$ = WCSA;}
	| WCSB_ {$$ = WCSB;}
	| WCSC_ {$$ = WCSC;}
	| WCSD_ {$$ = WCSD;}
	| WCSE_ {$$ = WCSE;}
	| WCSF_ {$$ = WCSF;}
	| WCSG_ {$$ = WCSG;}
	| WCSH_ {$$ = WCSH;}
	| WCSI_ {$$ = WCSI;}
	| WCSJ_ {$$ = WCSJ;}
	| WCSK_ {$$ = WCSK;}
	| WCSL_ {$$ = WCSL;}
	| WCSM_ {$$ = WCSM;}
	| WCSN_ {$$ = WCSN;}
	| WCSO_ {$$ = WCSO;}
	| WCSP_ {$$ = WCSP;}
	| WCSQ_ {$$ = WCSQ;}
	| WCSR_ {$$ = WCSR;}
	| WCSS_ {$$ = WCSS;}
	| WCST_ {$$ = WCST;}
	| WCSU_ {$$ = WCSU;}
	| WCSV_ {$$ = WCSV;}
	| WCSW_ {$$ = WCSW;}
	| WCSX_ {$$ = WCSX;}
	| WCSY_ {$$ = WCSY;}
	| WCSZ_ {$$ = WCSZ;}
	| WCS0_ {$$ = WCS0;}
	;

skyFrame : FK4_ {$$ = FK4;}
	| B1950_ {$$ = FK4;}
	| FK5_ {$$ = FK5;}
	| J2000_ {$$ = FK5;}
	| ICRS_ {$$ = ICRS;}
	| GALACTIC_ {$$ = GALACTIC;}
	| ECLIPTIC_ {$$ = ECLIPTIC;}
	;

skyFormat : DEGREES_ {$$=DEGREES;}
	| ARCMIN_ {$$=ARCMIN;}
	| ARCSEC_ {$$=ARCSEC;}
	;

property : SELECT_ {$$ = Marker::SELECT;}
	| HIGHLITE_ {$$ = Marker::HIGHLITE;}
	| DASH_ {$$ = Marker::DASH;}
	| FIXED_ {$$ = Marker::FIXED;}
	| EDIT_ {$$ = Marker::EDIT;}
	| MOVE_  {$$ = Marker::MOVE;}
	| ROTATE_ {$$ = Marker::ROTATE;}
	| DELETE_ {$$ = Marker::DELETE;}
	| INCLUDE_ {$$ = Marker::INCLUDE;}
	| SOURCE_ {$$ = Marker::SOURCE;}
	;

callBack : SELECT_ {$$ = CallBack::SELECTCB;}
	| UNSELECT_ {$$ = CallBack::UNSELECTCB;}
	| HIGHLITE_ {$$ = CallBack::HIGHLITECB;}
	| UNHIGHLITE_ {$$ = CallBack::UNHIGHLITECB;}
	| BEGIN_ MOVE_  {$$ = CallBack::MOVEBEGINCB;}
	| MOVE_  {$$ = CallBack::MOVECB;}
	| END_ MOVE_  {$$ = CallBack::MOVEENDCB;}
	| BEGIN_ EDIT_ {$$ = CallBack::EDITBEGINCB;}
	| EDIT_ {$$ = CallBack::EDITCB;}
	| END_ EDIT_ {$$ = CallBack::EDITENDCB;}
	| BEGIN_ ROTATE_ {$$ = CallBack::ROTATEBEGINCB;}
	| ROTATE_ {$$ = CallBack::ROTATECB;}
	| END_ ROTATE_ {$$ = CallBack::ROTATEENDCB;}
	| DELETE_ {$$ = CallBack::DELETECB;}
	| TEXT_ {$$ = CallBack::TEXTCB;}
	| COLOR_ {$$ = CallBack::COLORCB;}
	| WIDTH_ {$$ = CallBack::LINEWIDTHCB;}
	| PROPERTY_ {$$ = CallBack::PROPERTYCB;}
	| FONT_ {$$ = CallBack::FONTCB;}
	| KEY_ {$$ = CallBack::KEYCB;}
	| UPDATE_ {$$ = CallBack::UPDATECB;}
	;

global	: global sp globalProperty
	| globalProperty
	;

globalProperty : property '=' yesno 
	{
	  setProps(&globalProps,$1,$3);
	  setProps(&localProps,$1,$3);
	}
	| COLOR_ '=' STRING 
	{
	  strncpy(globalColor,$3,16);
	  strncpy(localColor,$3,16);
	}
	| COLOR_ '=' '#' STRING {
	  strcpy(globalColor,"#");
	  strncat(globalColor,$4,16);
	  strncpy(localColor,globalColor,16);
	}
	| DASHLIST_ '=' INT INT 
	{
	  globalDash[0] = localDash[0] =$3;
	  globalDash[1] = localDash[1] =$4;
        }
	| WIDTH_ '=' INT {globalWidth = localWidth = $3;}
	| FONT_ '=' STRING 
	{
	  strncpy(globalFont,$3,32);
	  strncpy(localFont,$3,32);
	}
	| TEXT_ '=' STRING 
	{
	  strncpy(globalText,$3,80);
	  strncpy(localText,$3,80);
	}
	| DASH_
	{
	  setProps(&globalProps,Marker::DASH,1);
	  setProps(&localProps,Marker::DASH,1);
	}
	| SOURCE_
	{
	  setProps(&globalProps,Marker::SOURCE,1);
	  setProps(&localProps,Marker::SOURCE,1);
	}
	| BACKGROUND_
	{
	  setProps(&globalProps,Marker::SOURCE,0);
	  setProps(&localProps,Marker::SOURCE,0);
	}
	| POINT_ '=' pointProp {globalPoint = localPoint = $3;}
	| POINT_ '=' pointProp INT 
	{
	  globalPoint = localPoint = $3;
	  globalPointSize = localPointSize = $4;
	}
	| LINE_ '=' INT INT 
	{
	  globalLine1 = localLine1 = $3;
	  globalLine2 = localLine2 = $4;
	}
	| VECTOR_ '=' INT {globalVector = localVector = $3;}
	| COMPOSITE_ '=' INT
	{
	  globalComposite = localComposite = $3;
	}
	| RULER_ '=' globalRuler {}
	| COMPASS_ '=' globalCompass STRING STRING INT INT
	{
	  strncpy(globalCompassNorth,$4,80);
	  strncpy(globalCompassEast,$5,80);
	  strncpy(localCompassNorth,$4,80);
	  strncpy(localCompassEast,$5,80);
	  globalCompassNArrow = localCompassNArrow = $6;
	  globalCompassEArrow = localCompassEArrow = $7;
	}
	| TEXTANGLE_ '=' angle {globalTextAngle = localTextAngle = $3;}
	| TEXTROTATE_ '=' INT {globalTextRotate = localTextRotate = $3;}
	| WCS_ '=' wcsSystem {globalWCS = (casa::viewer::CoordSystem)$3;}
	;

globalRuler : coordSystem skyFrame coordSystem skyFormat
	{
	  globalRulerCoordSystem = localRulerCoordSystem = (casa::viewer::CoordSystem)$1;
	  globalRulerSkyFrame = localRulerSkyFrame = (casa::viewer::SkyFrame)$2;
	  globalRulerDistSystem = localRulerDistSystem = (casa::viewer::CoordSystem)$3;
	  globalRulerDistFormat = localRulerDistFormat = (SkyFormat)$4;
	}
	| coordSystem coordSystem
	{
	  globalRulerCoordSystem = localRulerCoordSystem = (casa::viewer::CoordSystem)$1;
	  globalRulerSkyFrame = localRulerSkyFrame = FK5;
	  globalRulerDistSystem = localRulerDistSystem = (casa::viewer::CoordSystem)$2;
	  globalRulerDistFormat = localRulerDistFormat = DEGREES;
	}
	| coordSystem skyFormat
	{
	  globalRulerCoordSystem = localRulerCoordSystem = (casa::viewer::CoordSystem)$1;
	  globalRulerSkyFrame = localRulerSkyFrame = FK5;
	  globalRulerDistSystem = localRulerDistSystem = WCS;
	  globalRulerDistFormat = localRulerDistFormat = (SkyFormat)$2;
	}
	| skyFrame coordSystem
	{
	  globalRulerCoordSystem = localRulerCoordSystem = WCS;
	  globalRulerSkyFrame = localRulerSkyFrame = (casa::viewer::SkyFrame)$1;
	  globalRulerDistSystem = localRulerDistSystem = (casa::viewer::CoordSystem)$2;
	  globalRulerDistFormat = localRulerDistFormat = DEGREES;
	}
	| skyFrame skyFormat
	{
	  globalRulerCoordSystem = localRulerCoordSystem = WCS;
	  globalRulerSkyFrame = localRulerSkyFrame = (casa::viewer::SkyFrame)$1;
	  globalRulerDistSystem = localRulerDistSystem = WCS;
	  globalRulerDistFormat = localRulerDistFormat = (SkyFormat)$2;
	}
	| LINEAR_ coordSystem
	{
	  globalRulerCoordSystem = localRulerCoordSystem = WCS;
	  globalRulerSkyFrame = localRulerSkyFrame = FK5;
	  globalRulerDistSystem = localRulerDistSystem = (casa::viewer::CoordSystem)$2;
	  globalRulerDistFormat = localRulerDistFormat = DEGREES;
	}
	| LINEAR_ skyFormat
	{
	  globalRulerCoordSystem = localRulerCoordSystem = WCS;
	  globalRulerSkyFrame = localRulerSkyFrame = FK5;
	  globalRulerDistSystem = localRulerDistSystem = WCS;
	  globalRulerDistFormat = localRulerDistFormat = (SkyFormat)$2;
	}
	| skyFormat
	{
	  globalRulerCoordSystem = localRulerCoordSystem = IMAGE;
	  globalRulerSkyFrame = localRulerSkyFrame = FK5;
	  globalRulerDistSystem = localRulerDistSystem = WCS;
	  globalRulerDistFormat = localRulerDistFormat = (SkyFormat)$1;
	}
	| PIXELS_
	{
	  globalRulerCoordSystem = localRulerCoordSystem = IMAGE;
	  globalRulerSkyFrame = localRulerSkyFrame = FK5;
	  globalRulerDistSystem = localRulerDistSystem = IMAGE;
	  globalRulerDistFormat = localRulerDistFormat = DEGREES;
	}
	;

globalCompass : coordSystem skyFrame
	{
	  globalCompassCoordSystem = localCompassCoordSystem = (casa::viewer::CoordSystem)$1;
	  globalCompassSkyFrame = localCompassSkyFrame = (casa::viewer::SkyFrame)$2;
	}
	| coordSystem
	{
	  globalCompassCoordSystem = localCompassCoordSystem = (casa::viewer::CoordSystem)$1;
	  globalCompassSkyFrame = localCompassSkyFrame = FK5;
	}
	| skyFrame
	{
	  globalCompassCoordSystem = localCompassCoordSystem = WCS;
	  globalCompassSkyFrame = localCompassSkyFrame = (casa::viewer::SkyFrame)$1;
	}
	| LINEAR_
	{
	  globalCompassCoordSystem = localCompassCoordSystem = WCS;
	  globalCompassSkyFrame = localCompassSkyFrame = FK5;
	}
	;

initGlobal:{
	  // global properties
	  globalSystem = PHYSICAL;
	  globalWCS = WCS;
	  globalSky = NATIVEWCS;
	  globalTile = 1;
	  globalProps =
	    Marker::SELECT | Marker::EDIT | Marker::MOVE |
	    Marker::ROTATE | Marker::DELETE | Marker::HIGHLITE |
	    Marker::INCLUDE | Marker::SOURCE;
	  strcpy(globalColor,"green");
	  globalDash[0] = 8;
	  globalDash[1] = 3;
          globalWidth = 1;
	  strcpy(globalFont,"helvetica 10 normal roman");
	  strcpy(globalText,"");

	  // unique properties
	  globalLine1 = 0;
	  globalLine2 = 0;
	  globalVector = 1;
	  globalComposite = 1;
	  globalRulerCoordSystem = PHYSICAL;
	  globalRulerSkyFrame = FK5;
	  globalRulerDistSystem = PHYSICAL;
	  globalRulerDistFormat = DEGREES;
	  globalCompassCoordSystem = PHYSICAL;
	  globalCompassSkyFrame = FK5;
	  strcpy(globalCompassNorth,"N");
	  strcpy(globalCompassEast,"E");
	  globalCompassNArrow = 1;
	  globalCompassEArrow = 1;
	  globalPoint = BOXCIRCLE;
	  globalPointSize = POINTSIZE;
	  globalTextAngle=0;
	  globalTextRotate=1;

	  aStatus = 0;
	  cStatus = 0;
	} 
	;

local	: local sp localProperty
	| localProperty
	;

localProperty : property '=' yesno {setProps(&localProps,$1,$3);}
	| COLOR_ '=' STRING {strncpy(localColor,$3,16);}
	| COLOR_ '=' '#' STRING {
	  strcpy(localColor,"#");
	  strncat(localColor,$4,16);
	}
	| DASHLIST_ '=' INT INT 
	{
	  localDash[0] =$3;
	  localDash[1] =$4;
        }
	| WIDTH_ '=' INT {localWidth = $3;}
	| FONT_ '=' STRING {strncpy(localFont,$3,32);}
	| TEXT_ '=' STRING {strncpy(localText,$3,80);}
	| TAG_ '=' STRING {taglist.push_back($3);}
	| CALLBACK_ '=' callBack STRING STRING {/*cblist.push_back(new CallBack(fr->getInterp(),(CallBack::Type)$3,$4,$5));*/}
	| DASH_ {setProps(&localProps,Marker::DASH,1);}
	| SOURCE_ {setProps(&localProps,Marker::SOURCE,1);}
	| BACKGROUND_ {setProps(&localProps,Marker::SOURCE,0);}

	| POINT_ '=' pointProp {localPoint = $3;}
	| POINT_ '=' pointProp INT {localPoint = $3; localPointSize = $4;}
	| LINE_ '=' INT INT {localLine1=$3; localLine2=$4;}
	| VECTOR_ '=' INT {localVector=$3;}
	| COMPOSITE_ '=' INT {localComposite=$3;}
	| RULER_ '=' localRuler
	| COMPASS_ '=' localCompass STRING STRING INT INT
	{
	  strncpy(localCompassNorth,$4,80);
	  strncpy(localCompassEast,$5,80);
	  localCompassNArrow = $6;
	  localCompassEArrow = $7;
	}
	| TEXTANGLE_ '=' angle {localTextAngle=$3;}
	| TEXTROTATE_ '=' INT {localTextRotate=$3;}
	| CPANDA_ '=' localCpanda
	| EPANDA_ '=' localEpanda
	| BPANDA_ '=' localBpanda
	;

localRuler : coordSystem skyFrame coordSystem skyFormat
	{
	  localRulerCoordSystem = (casa::viewer::CoordSystem)$1;
	  localRulerSkyFrame = (casa::viewer::SkyFrame)$2;
	  localRulerDistSystem = (casa::viewer::CoordSystem)$3;
	  localRulerDistFormat = (SkyFormat)$4;
	}
	| coordSystem coordSystem
	{
	  localRulerCoordSystem = (casa::viewer::CoordSystem)$1;
	  localRulerSkyFrame = FK5;
	  localRulerDistSystem = (casa::viewer::CoordSystem)$2;
	  localRulerDistFormat = DEGREES;
	}
	| coordSystem skyFormat
	{
	  localRulerCoordSystem = (casa::viewer::CoordSystem)$1;
	  localRulerSkyFrame = FK5;
	  localRulerDistSystem = WCS;
	  localRulerDistFormat = (SkyFormat)$2;
	}
	| skyFrame coordSystem
	{
	  localRulerCoordSystem = WCS;
	  localRulerSkyFrame = (casa::viewer::SkyFrame)$1;
	  localRulerDistSystem = (casa::viewer::CoordSystem)$2;
	  localRulerDistFormat = DEGREES;
	}
	| skyFrame skyFormat
	{
	  localRulerCoordSystem = WCS;
	  localRulerSkyFrame = (casa::viewer::SkyFrame)$1;
	  localRulerDistSystem = WCS;
	  localRulerDistFormat = (SkyFormat)$2;
	}
	| LINEAR_ coordSystem
	{
	  localRulerCoordSystem = WCS;
	  localRulerSkyFrame = FK5;
	  localRulerDistSystem = (casa::viewer::CoordSystem)$2;
	  localRulerDistFormat = DEGREES;
	}
	| LINEAR_ skyFormat
	{
	  localRulerCoordSystem = WCS;
	  localRulerSkyFrame = FK5;
	  localRulerDistSystem = WCS;
	  localRulerDistFormat = (SkyFormat)$2;
	}
	| skyFormat
	{
	  localRulerCoordSystem = IMAGE;
	  localRulerSkyFrame = FK5;
	  localRulerDistSystem = WCS;
	  localRulerDistFormat = (SkyFormat)$1;
	}
	| PIXELS_
	{
	  localRulerCoordSystem = IMAGE;
	  localRulerSkyFrame = FK5;
	  localRulerDistSystem = IMAGE;
	  localRulerDistFormat = DEGREES;
	}
	;

localCompass : coordSystem skyFrame
	{
	  localCompassCoordSystem = (casa::viewer::CoordSystem)$1;
	  localCompassSkyFrame = (casa::viewer::SkyFrame)$2;
	}
	| coordSystem
	{
	  localCompassCoordSystem = (casa::viewer::CoordSystem)$1;
	  localCompassSkyFrame = FK5;
	}
	| skyFrame
	{
	  localCompassCoordSystem = WCS;
	  localCompassSkyFrame = (casa::viewer::SkyFrame)$1;
	}
	| LINEAR_
	{
	  localCompassCoordSystem = WCS;
	  localCompassSkyFrame = FK5;
	}
	;

localCpanda: {aNum=0; aAngNum=0;} '(' aAngs ')' '(' aRads ')' {localCpanda = 2;}
	| IGNORE_ {localCpanda=0;}
	;

localEpanda: {aNum=0; aAngNum=0, aAngle=0;} 
	  '(' aAngs ')' '(' vRads ')' '(' angle ')' {aAngle=$9;localEpanda=2;}
	| IGNORE_ {localEpanda=0;}
	;

localBpanda: {aNum=0; aAngNum=0, aAngle=0;} 
	  '(' aAngs ')' '(' vRads ')' '(' angle ')' {aAngle=$9;localBpanda=2;}
	| IGNORE_ {localBpanda=0;}
	;

initLocal : {
	  // needed for annulus, ellipse annulus, and box annulus
	  aNum = 2;

	  // composite (previous conjuction found?)
	  if (!cStatus) fr->resetCompositeMarker();

	  // global properties
	  localSystem = globalSystem;
	  localSky = globalSky;
	  localProps = globalProps;
	  strcpy(localColor,globalColor);
	  localDash[0] = globalDash[0];
	  localDash[1] = globalDash[1];
	  localWidth = globalWidth;
	  strcpy(localFont,globalFont);
	  strcpy(localText,globalText);
	  strcpy(localComment,"");
	  taglist.clear();

	  // unique properties
	  localLine1 = globalLine1;
	  localLine2 = globalLine2;
	  localVector = globalVector;
	  localComposite = globalComposite;
	  localPoint = globalPoint;
	  localPointSize = globalPointSize;
	  localRulerCoordSystem = globalRulerCoordSystem;
	  localRulerSkyFrame = globalRulerSkyFrame;
	  localRulerDistSystem = globalRulerDistSystem;
	  localRulerDistFormat = globalRulerDistFormat;
	  localCompassCoordSystem = globalCompassCoordSystem;
	  localCompassSkyFrame = globalCompassSkyFrame;
	  strcpy(localCompassNorth,globalCompassNorth);
	  strcpy(localCompassEast,globalCompassEast);
	  localCompassNArrow = globalCompassNArrow;
	  localCompassEArrow = globalCompassEArrow;
	  localTextAngle = globalTextAngle;
	  localTextRotate = globalTextRotate;
	  localCpanda = 1;
	  localEpanda = 1;
	  localBpanda = 1;
	}
	;

pointProp : CIRCLE_ {$$ = CIRCLE;}
	| BOX_ {$$ = BOX;}
	| DIAMOND_ {$$ = DIAMOND;}
	| CROSS_ {$$ = CROSS;}
	| X_ {$$ = XPT;}
	| ARROW_ {$$ = ARROW;}
	| BOXCIRCLE_ {$$ = BOXCIRCLE;}
	;

include	: '+' {setProps(&localProps, Marker::INCLUDE, 1);}
	| '-' {setProps(&localProps, Marker::INCLUDE, 0);}
	;

nonshape : TEXT_ bp coord ep conjuction nonshapeComment
	  {fr->createTextCmd(coordtovec($3),
	    localTextAngle,localTextRotate,
	    localColor,localDash,localWidth,localFont,
	    localText,localProps,localComment,taglist /*,cblist*/);}
	| COMPOSITE_ bp coord sp optangle ep conjuction nonshapeComment
	  {fr->createCompositeCmd(coordtovec($3),
	    $5, localComposite,
	    localColor,localDash,localWidth,localFont,
	    localText,localProps,localComment,taglist /*,cblist*/);}
	| VECTOR_ bp coord sp value sp angle ep conjuction nonshapeComment
	  {fr->createVectCmd(coordtovec($3),
	    $5,$7,
	    localVector,
	    localColor,localDash,localWidth,localFont,
	    localText,localProps,localComment,taglist /*,cblist*/);}
	| PROJECTION_ bp coord sp coord sp value ep conjuction nonshapeComment
	{
	  // hard coded into projection.tcl
	  fr->createProjectionCmd(coordtovec($3),
	    coordtovec($5),
	    $7,
	    "ProjectionPlotCB", "ProjectionPlotDeleteCB",
	    localColor,localDash,localWidth,localFont,
	    localText,localProps,localComment,taglist /*,cblist*/);
	}
	| RULER_ bp coord sp coord ep conjuction nonshapeComment
	  {fr->createRulerCmd(coordtovec($3),
	    coordtovec($5),
   	    localRulerCoordSystem, localRulerSkyFrame,
	    localRulerDistSystem, localRulerDistFormat,
	    localColor,localDash,localWidth,localFont,
	    localText,localProps,localComment,taglist /*,cblist*/);}
	| COMPASS_ bp coord sp value ep conjuction nonshapeComment
	  {fr->createCompassCmd(coordtovec($3), 
	   $5,
	   localCompassNorth, localCompassEast, 
	   localCompassNArrow, localCompassEArrow,
   	   localCompassCoordSystem, localCompassSkyFrame,
	   localColor,localDash,localWidth,localFont,
	   localText,localProps,localComment,taglist /*,cblist*/);}
	| CIRCLE3D_ bp coord sp value ep conjuction nonshapeComment
	{
	  // hard coded into projection.tcl
	  fr->createCircle3dCmd(coordtovec($3),
	    $5,
	    "Circle3dPlotCB", "Circle3dPlotDeleteCB",
	    localColor,localDash,localWidth,localFont,
	    localText,localProps,localComment,taglist /*,cblist*/);
	}
	;

shape	: CIRCLE_ bp coord sp value ep conjuction shapeComment
	  {fr->createCircleCmd(coordtovec($3),
	    $5,
	    localColor,localDash,localWidth,localFont,
	    localText,localProps,localComment,taglist /*,cblist*/);}
	| ANNULUS_ bp coord sp value sp value ep conjuction shapeComment
	  {fr->createAnnulusCmd(coordtovec($3),
	    $5,$7,1,
	    localColor,localDash,localWidth,localFont,
	    localText,localProps,localComment,taglist /*,cblist*/);}
	| ANNULUS_ bp coord sp value sp value sp aRads ep conjuction 
	    shapeComment
	{
	  aAnnuli[0] = $5;
	  aAnnuli[1] = $7;
	  fr->createAnnulusCmd(coordtovec($3),
	    aNum,aAnnuli,
	    localColor,localDash,localWidth,localFont,
	    localText,localProps,localComment,taglist /*,cblist*/);
	}
	| ANNULUS_ bp coord sp value sp value sp numberof ep conjuction 
	    shapeComment
	  {fr->createAnnulusCmd(coordtovec($3),
	    $5,$7,$9,
	    localColor,localDash,localWidth,localFont,
	    localText,localProps,localComment,taglist /*,cblist*/);}
	| CPANDA_ bp coord sp angle sp angle sp INT sp 
	    value sp value sp INT ep conjuction shapeComment
	{
	  switch (localCpanda) {
	  case 0: /* ignore it */
	    break;
	  case 1: /* normal cpanda */
	    fr->createCpandaCmd(coordtovec($3),
	      $5,$7,$9,
	      $11,$13,$15,
	      localColor,localDash,localWidth,localFont,
	      localText,localProps,localComment,taglist /*,cblist*/);
	    break;
	  case 2: /* one of our special pandas */
	    fr->createCpandaCmd(coordtovec($3),
	      aAngNum,aAngles,
	      aNum,aAnnuli,
	      localColor,localDash,localWidth,localFont,
	      localText,localProps,localComment,taglist /*,cblist*/);
	    break;
	  }
	}

	| ELLIPSE_ bp coord sp vvalue sp optangle ep conjuction shapeComment
	{
	  // for ellipse annulus
	  aStatus = 1;
	  aCenter = coordtovec($3);
	  aAngles[0] = $7;
	  aVector[0] = coordtovec($5);
	  aNumsao = 1;
	  strncpy(aColor,localColor,16);
	  aDash[0] = localDash[0];
	  aDash[1] = localDash[1];
	  aWidth = localWidth;
	  strncpy(aFont,localFont,32);
	  strncpy(aText,localText,80);
	  strncpy(aComment,localComment,80);
	  aProps = localProps;

	  fr->createEllipseCmd(coordtovec($3),
	    coordtovec($5),
	    $7,
	    localColor,localDash,localWidth,localFont,
	    localText,localProps,localComment,taglist /*,cblist*/);
	}
	| ELLIPSE_ bp coord sp vvalue sp vvalue sp optangle ep conjuction 
	    shapeComment
	{
	  // prefered syntax
	  fr->createEllipseAnnulusCmd(coordtovec($3),
	    coordtovec($5),coordtovec($7),1,
	    $9,
	    localColor,localDash,localWidth,localFont,
	    localText,localProps,localComment,taglist /*,cblist*/);
	}
	| ELLIPSE_ bp coord sp vvalue sp vvalue sp 
	    numberof sp optangle ep conjuction shapeComment
	{
	  // prefered syntax
	  fr->createEllipseAnnulusCmd(coordtovec($3),
	    coordtovec($5),coordtovec($7),$9,
	    $11,
	    localColor,localDash,localWidth,localFont,
	    localText,localProps,localComment,taglist /*,cblist*/);
	}
	| ELLIPSE_ bp coord sp vvalue sp vvalue sp
	    vRads sp optangle ep conjuction shapeComment
	{
	  // prefered syntax
	  aVector[0] = coordtovec($5);
	  aVector[1] = coordtovec($7);
	  fr->createEllipseAnnulusCmd(coordtovec($3),
	    aNum,aVector,
	    $11,
	    localColor,localDash,localWidth,localFont,
	    localText,localProps,localComment,taglist /*,cblist*/);
	}

	| ELLIPSE_ bp coord sp vvalue sp optangle ep '&' '!' 
	  ELLIPSE_ bp coord sp vvalue sp optangle ep
	{	
	  // old saoimage syntax
	  aStatus = 2;
	  aVector[aNumsao++] = coordtovec($5);
	}
	| EPANDA_ bp coord sp angle sp angle sp INT sp 
	    vvalue sp vvalue sp INT sp optangle ep conjuction shapeComment
	{
	  switch (localEpanda) {
	  case 0: /* ignore it */
	    break;
	  case 1: /* normal epanda */
	    fr->createEpandaCmd(coordtovec($3),
	      $5,$7,$9,
	      coordtovec($11),coordtovec($13),$15,
	      $17,
	      localColor,localDash,localWidth,localFont,
	      localText,localProps,localComment,taglist /*,cblist*/);
	    break;
	  case 2: /* one of our special pandas */
	    fr->createEpandaCmd(coordtovec($3),
	      aAngNum,aAngles,
	      aNum,aVector,
	      aAngle,
	      localColor,localDash,localWidth,localFont,
	      localText,localProps,localComment,taglist /*,cblist*/);
	    break;
	  }
	}

	| BOX_ bp coord sp vvalue sp optangle ep conjuction shapeComment
	{
	  // for box annulus
	  aStatus = 3;
	  aCenter = coordtovec($3);
	  aAngles[0] = $7;
	  aVector[0] = coordtovec($5);
	  aNumsao = 1;
	  strncpy(aColor,localColor,16);
	  aDash[0] = localDash[0];
	  aDash[1] = localDash[1];
	  aWidth = localWidth;
	  strncpy(aFont,localFont,32);
	  strncpy(aText,localText,80);
	  strncpy(aComment,localComment,80);
	  aProps = localProps;

	  fr->createBoxCmd(coordtovec($3),
	    coordtovec($5),
	    $7,
	    localColor,localDash,localWidth,localFont,
	    localText,localProps,localComment,taglist /*,cblist*/);
	}
	| ROTBOX_ bp coord sp vvalue sp optangle ep conjuction shapeComment
	  {fr->createBoxCmd(coordtovec($3),
	    coordtovec($5),
	    $7,
	    localColor,localDash,localWidth,localFont,
	    localText,localProps,localComment,taglist /*,cblist*/);}
	| BOX_ bp coord sp vvalue sp vvalue sp optangle ep conjuction 
	    shapeComment
	{
	  // prefered syntax
	  fr->createBoxAnnulusCmd(coordtovec($3),
	    coordtovec($5),coordtovec($7),1,
	    $9,
	    localColor,localDash,localWidth,localFont,
	    localText,localProps,localComment,taglist /*,cblist*/);
	}
	| BOX_ bp coord sp vvalue sp vvalue sp 
	    vRads sp optangle ep conjuction shapeComment
	{
	  // prefered syntax
	  aVector[0] = coordtovec($5);
	  aVector[1] = coordtovec($7);
	  fr->createBoxAnnulusCmd(coordtovec($3),
	    aNum,aVector,
	    $11,
	    localColor,localDash,localWidth,localFont,
	    localText,localProps,localComment,taglist /*,cblist*/);
	}
	| BOX_ bp coord sp vvalue sp vvalue sp 
	    numberof sp optangle ep conjuction shapeComment
	{
	  // prefered syntax
	  fr->createBoxAnnulusCmd(coordtovec($3),
	    coordtovec($5),coordtovec($7),$9,
	    $11,
	    localColor,localDash,localWidth,localFont,
	    localText,localProps,localComment,taglist /*,cblist*/);
	}

	| BOX_ bp coord sp vvalue sp optangle ep '&' '!' 
	  BOX_ bp coord sp vvalue sp optangle ep
	{	
	  // old saoimage syntax
	  aStatus = 4;
	  aVector[aNumsao++] = coordtovec($5);
	}
	| BPANDA_ bp coord sp angle sp angle sp INT sp 
	    vvalue sp vvalue sp INT sp optangle ep conjuction shapeComment
	{
	  switch (localBpanda) {
	  case 0: /* ignore it */
	    break;
	  case 1: /* normal bpanda */
	    fr->createBpandaCmd(coordtovec($3),
	      $5,$7,$9,
	      coordtovec($11),coordtovec($13),$15,
	      $17,
	      localColor,localDash,localWidth,localFont,
	      localText,localProps,localComment,taglist /*,cblist*/);
	    break;
	  case 2: /* one of our special pandas */
	    fr->createBpandaCmd(coordtovec($3),
	      aAngNum,aAngles,
	      aNum,aVector,
	      aAngle,
	      localColor,localDash,localWidth,localFont,
	      localText,localProps,localComment,taglist /*,cblist*/);
	    break;
	  }
	}

	| LINE_ bp coord sp coord ep conjuction shapeComment
	  {fr->createLineCmd(coordtovec($3),
	    coordtovec($5),
	    localLine1,localLine2,
	    localColor,localDash,localWidth,localFont,
	    localText,localProps,localComment,taglist /*,cblist*/);}

	| POINT_ bp coord ep conjuction shapeComment 
	{
	  switch (localPoint) {
	  case CIRCLE:
	    fr->createCirclePointCmd(coordtovec($3), localPointSize, 
	      localColor,localDash,localWidth,localFont,
	      localText,localProps,localComment,taglist /*,cblist*/);
	    break;
	  case BOX:
	    fr->createBoxPointCmd(coordtovec($3), localPointSize, 
	      localColor,localDash,localWidth,localFont,
	      localText,localProps,localComment,taglist /*,cblist*/);
	    break;
	  case DIAMOND:
	    fr->createDiamondPointCmd(coordtovec($3), localPointSize, 
	      localColor,localDash,localWidth,localFont,
	      localText,localProps,localComment,taglist /*,cblist*/);
	    break;
	  case CROSS:
	    fr->createCrossPointCmd(coordtovec($3), localPointSize, 
	      localColor,localDash,localWidth,localFont,
	      localText,localProps,localComment,taglist /*,cblist*/);
	    break;
	  case XPT:
	    fr->createExPointCmd(coordtovec($3), localPointSize, 
	      localColor,localDash,localWidth,localFont,
	      localText,localProps,localComment,taglist /*,cblist*/);
	    break;
	  case ARROW:
	    fr->createArrowPointCmd(coordtovec($3), localPointSize, 
	      localColor,localDash,localWidth,localFont,
	      localText,localProps,localComment,taglist /*,cblist*/);
	    break;
	  case BOXCIRCLE:
	    fr->createBoxCirclePointCmd(coordtovec($3), localPointSize, 
	      localColor,localDash,localWidth,localFont,
	      localText,localProps,localComment,taglist /*,cblist*/);
	    break;
	  }
	}
	| CIRCLE_ POINT_ bp coord ep conjuction shapeComment
	  {fr->createCirclePointCmd(coordtovec($4), localPointSize, 
	    localColor,localDash,localWidth,localFont,
	    localText,localProps,localComment,taglist /*,cblist*/);}
	| BOX_ POINT_ bp coord ep conjuction shapeComment
	  {fr->createBoxPointCmd(coordtovec($4), localPointSize, 
	    localColor,localDash,localWidth,localFont,
	    localText,localProps,localComment,taglist /*,cblist*/);}
	| DIAMOND_ POINT_ bp coord ep conjuction shapeComment
	  {fr->createDiamondPointCmd(coordtovec($4), localPointSize, 
	    localColor,localDash,localWidth,localFont,
	    localText,localProps,localComment,taglist /*,cblist*/);}
	| CROSS_ POINT_ bp coord ep conjuction shapeComment
	  {fr->createCrossPointCmd(coordtovec($4), localPointSize, 
	    localColor,localDash,localWidth,localFont,
	    localText,localProps,localComment,taglist /*,cblist*/);}
	| X_ POINT_ bp coord ep conjuction shapeComment
	  {fr->createExPointCmd(coordtovec($4), localPointSize, 
	    localColor,localDash,localWidth,localFont,
	    localText,localProps,localComment,taglist /*,cblist*/);}
	| ARROW_ POINT_ bp coord ep conjuction shapeComment
	  {fr->createArrowPointCmd(coordtovec($4), localPointSize, 
	    localColor,localDash,localWidth,localFont,
	    localText,localProps,localComment,taglist /*,cblist*/);}
	| BOXCIRCLE_ POINT_ bp coord ep conjuction shapeComment
	  {fr->createBoxCirclePointCmd(coordtovec($4), localPointSize, 
	    localColor,localDash,localWidth,localFont,
	    localText,localProps,localComment,taglist /*,cblist*/);}

	| POLYGON_ {polylist.clear();} bp polyNodes ep conjuction 
	    shapeComment
	  {fr->createPolygonCmd(polylist, 
	      localColor,localDash,localWidth,localFont,
	      localText,localProps,localComment,taglist /*,cblist*/);} 

	| PIE_ bp coord sp angle sp angle ep conjuction shapeComment
	| PIE_ bp coord sp angle sp angle sp aAngs ep conjuction shapeComment
	| PIE_ bp coord sp angle sp angle sp numberof ep conjuction 
	    shapeComment
	| FIELD_ bp ep conjuction shapeComment

 	| TEXT_ bp coord ep conjuction shapeComment
	  {fr->createTextCmd(coordtovec($3),
	    localTextAngle,localTextRotate,
	    localColor,localDash,localWidth,localFont,
	    localText,localProps,localComment,taglist /*,cblist*/);}
	| TEXT_ bp coord sp STRING ep {strncpy(localText,$5,80);} conjuction 
	    shapeComment
	  {fr->createTextCmd(coordtovec($3),
	    localTextAngle,localTextRotate,
	    localColor,localDash,localWidth,localFont,
	    localText,localProps,localComment,taglist /*,cblist*/);}
	| COMPOSITE_ bp coord sp optangle ep conjuction shapeComment
	  {fr->createCompositeCmd(coordtovec($3),
	    $5, localComposite,
	    localColor,localDash,localWidth,localFont,
	    localText,localProps,localComment,taglist /*,cblist*/);}
	| VECTOR_ bp coord sp value sp angle ep conjuction shapeComment
	  {fr->createVectCmd(coordtovec($3),
	    $5,$7,
	    localVector,
	    localColor,localDash,localWidth,localFont,
	    localText,localProps,localComment,taglist /*,cblist*/);}
	| PROJECTION_ bp coord sp coord sp value ep conjuction shapeComment
	{
	  // hard coded into projection.tcl
	  fr->createProjectionCmd(coordtovec($3),
	    coordtovec($5),
	    $7,
	    "ProjectionPlotCB", "ProjectionPlotDeleteCB",
	    localColor,localDash,localWidth,localFont,
	    localText,localProps,localComment,taglist /*,cblist*/);
	}
	| RULER_ bp coord sp coord ep conjuction shapeComment
	  {fr->createRulerCmd(coordtovec($3),
	    coordtovec($5),
   	    localRulerCoordSystem, localRulerSkyFrame,
	    localRulerDistSystem, localRulerDistFormat,
	    localColor,localDash,localWidth,localFont,
	    localText,localProps,localComment,taglist /*,cblist*/);}
	| COMPASS_ bp coord sp value ep conjuction shapeComment
	  {fr->createCompassCmd(coordtovec($3), 
	   $5,
	   localCompassNorth, localCompassEast, 
	   localCompassNArrow, localCompassEArrow,
   	   localCompassCoordSystem, localCompassSkyFrame,
	   localColor,localDash,localWidth,localFont,
	   localText,localProps,localComment,taglist /*,cblist*/);}
	| CIRCLE3D_ bp coord sp value ep conjuction shapeComment
	{
	  // hard coded into circle3d.tcl
	  fr->createCircle3dCmd(coordtovec($3),
	    $5,
	    "Circle3dPlotCB", "Circle3dPlotDeleteCB",
	    localColor,localDash,localWidth,localFont,
	    localText,localProps,localComment,taglist /*,cblist*/);
	}
	;

polyNodes : polyNodes sp polyNode
	| polyNode
	;

polyNode : coord {polylist.push_back(coordtovec($1));}
	;

aRads	: aRads sp aRad
	| aRad
	;

aRad	: value 
	{
	  if (aNum < MAXANNULI)
	    aAnnuli[aNum++] = $1;
	}
	;

aAngs	: aAngs sp aAng
	| aAng
	;

aAng	: angle 
	{
	  if (aAngNum < MAXANGLES)
	    aAngles[aAngNum++] = $1;
	}
	;

vRads	: vRads sp vRad
	| vRad
	;

vRad	: value sp value {aVector[aNum++] = doubletovec($1,$3);}
	;

postLocal : /* empty */
	{
	  // old style annulus
	  switch (aStatus) {
	  case 0: // do nothing
	    break;
	  case 1: // we found just an ellipse, do nothing
	    break;
	  case 2: // ok we have an ellipse annulus
	    fr->markerDeleteLastCmd(); // delete the previous ellipse
	    fr->createEllipseAnnulusCmd(aCenter,
	      aNumsao,aVector,
	      aAngles[0],
	      aColor,aDash,aWidth,aFont,aText,aProps,aComment,taglist /*,cblist*/);
	    break;
	  case 3: // we found just a box, do nothing
	    break;
	  case 4: // ok, we have a box annulus
	    fr->markerDeleteLastCmd(); // delete the previous box
	    fr->createBoxAnnulusCmd(aCenter,
	      aNumsao,aVector,
	      aAngles[0],
	      aColor,aDash,aWidth,aFont,aText,aProps,aComment,taglist /*,cblist*/);
	    break;
	  }
	  aStatus = 0;
	}
	;
%%

static void setProps(unsigned short* props, unsigned short prop, int value)
{
  if (value)
    *props |= prop;
  else
    *props &= ~prop;
}

static casa::viewer::CoordSystem checkWCSSystem()
{
  switch (localSystem) {
  case casa::viewer::IMAGE:
  case casa::viewer::PHYSICAL:
    return casa::viewer::WCS;
  default:
   return localSystem;
  }
}

static casa::viewer::SkyFrame checkWCSSky()
{
  switch (localSystem) {
  case casa::viewer::IMAGE:
  case casa::viewer::PHYSICAL:
    return casa::viewer::NATIVEWCS;
  default:
   return localSky;
  }
}

//------------------------------------------------------------------------------------------
// pass error along to the driver (ds9parser)
void casa::viewer::ds9parse::error( const ds9parse::location_type &l, const std::string &m )
     { driver.error(l,m); }
//------------------------------------------------------------------------------------------
