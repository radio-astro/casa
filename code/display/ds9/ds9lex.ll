 // Copyright (C) 1999-2010
 // Smithsonian Astrophysical Observatory, Cambridge, MA, USA
 // Copyright (C) 2011
 // Associated Universities, Inc. Washington DC, USA.
 //
 // This library is free software; you can redistribute it and/or modify it
 // under the terms of the GNU Library General Public License as published by
 // the Free Software Foundation; either version 2 of the License, or (at your
 // option) any later version.
 //
 // This library is distributed in the hope that it will be useful, but WITHOUT
 // ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 // FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
 // License for more details.
 //
 // You should have received a copy of the GNU Library General Public License
 // along with this library; if not, write to the Free Software Foundation,
 // Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
 //
 // Correspondence concerning AIPS++ should be addressed as follows:
 //        Internet email: aips2-request@nrao.edu.
 //        Postal address: AIPS++ Project Office
 //                        National Radio Astronomy Observatory
 //                        520 Edgemont Road
 //                        Charlottesville, VA 22903-2475 USA

%option caseless
%option never-interactive
%option c++
%option prefix="ds9lex_"
%option batch

%{
  #include <display/ds9/ds9lex.h>

  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>

  //#include "util.h"
  #include <display/ds9/ds9parse.hh>

  // import the parser's token type into a local typedef
  typedef casa::viewer::ds9parse::token token;
  typedef casa::viewer::ds9parse::token_type token_type;

  casa::viewer::ds9lex *mklexx;
%}

%x DISCARD

D   [0-9]
E   [Ee][+-]?{D}+

/* rules */

%%

<DISCARD>[\n]	{ // special case-- #\n
		  BEGIN INITIAL;
		  yyless(0);             // put back the terminator
		  strcpy(yylval->str,""); // feed a blank string
		  return token::STRING;
		}

<DISCARD>[^\n]* {  // Discard reset of line
		  BEGIN INITIAL;
		  int ll = yyleng <(MKBUFSIZE-1) ? yyleng:(MKBUFSIZE-1);
		  strncpy(yylval->str,yytext,ll);
	          yylval->str[ll] = '\0';
		  return token::STRING;
		}

amplifier	{return token::AMPLIFIER_;}
ann[u][l][u][s]	{return token::ANNULUS_;}
arcmin		{return token::ARCMIN_;}
arcsec		{return token::ARCSEC_;}
arrow		{return token::ARROW_;}
b1950		{return token::B1950_;}
background	{return token::BACKGROUND_;}
begin		{return token::BEGIN_;}
box		{return token::BOX_;}
boxcircle	{return token::BOXCIRCLE_;}
bpanda		{return token::BPANDA_;}
callback	{return token::CALLBACK_;}
cir[c][l][e]	{return token::CIRCLE_;}
circle3d	{return token::CIRCLE3D_;}
color		{return token::COLOR_;}
compass		{return token::COMPASS_;}
composite	{return token::COMPOSITE_;}
cpanda		{return token::CPANDA_;}
cross		{return token::CROSS_;}
dash		{return token::DASH_;}
dashlist	{return token::DASHLIST_;}
debug		{return token::DEBUG_;}
degrees		{return token::DEGREES_;}
delete		{return token::DELETE_;}
detector	{return token::DETECTOR_;}
diamond		{return token::DIAMOND_;}
edit		{return token::EDIT_;}
ell[i][p][s][e]	{return token::ELLIPSE_;}
ecliptic	{return token::ECLIPTIC_;}
epanda		{return token::EPANDA_;}
end		{return token::END_;}
false		{return token::FALSE_;}
fie[l][d]	{return token::FIELD_;}
fixed		{return token::FIXED_;}
fk4		{return token::FK4_;}
fk5		{return token::FK5_;}
font		{return token::FONT_;}
galactic	{return token::GALACTIC_;}
global		{return token::GLOBAL_;}
highlite	{return token::HIGHLITE_;}
icrs		{return token::ICRS_;}
ignore		{return token::IGNORE_;}
include		{return token::INCLUDE_;}
image		{return token::IMAGE_;}
key		{return token::KEY_;}
j2000		{return token::J2000_;}
lin[e]		{return token::LINE_;}
linear		{return token::LINEAR_;}
move		{return token::MOVE_;}
n		{return token::N_;}
no		{return token::NO_;}
off		{return token::OFF_;}
on		{return token::ON_;}
panda		{return token::CPANDA_;}
physical	{return token::PHYSICAL_;}
pie		{return token::PIE_;}
pixels		{return token::PIXELS_;}
poi[n][t]	{return token::POINT_;}
pol[y][g][o][n]	{return token::POLYGON_;}
projection	{return token::PROJECTION_;}
property	{return token::PROPERTY_;}
rotate		{return token::ROTATE_;}
rotbox		{return token::ROTBOX_;}
ruler		{return token::RULER_;}
select		{return token::SELECT_;}
source		{return token::SOURCE_;}
tag		{return token::TAG_;}
text		{return token::TEXT_;}
textangle	{return token::TEXTANGLE_;}
textrotate	{return token::TEXTROTATE_;}
tile		{return token::TILE_;}
true		{return token::TRUE_;}
vector		{return token::VECTOR_;}
version		{return token::VERSION_;}
update		{return token::UPDATE_;}
unhighlite	{return token::UNHIGHLITE_;}
unselect	{return token::UNSELECT_;}
wcs		{return token::WCS_;}
wcsa		{return token::WCSA_;}
wcsb		{return token::WCSB_;}
wcsc		{return token::WCSC_;}
wcsd		{return token::WCSD_;}
wcse		{return token::WCSE_;}
wcsf		{return token::WCSF_;}
wcsg		{return token::WCSG_;}
wcsh		{return token::WCSH_;}
wcsi		{return token::WCSI_;}
wcsj		{return token::WCSJ_;}
wcsk		{return token::WCSK_;}
wcsl		{return token::WCSL_;}
wcsm		{return token::WCSM_;}
wcsn		{return token::WCSN_;}
wcso		{return token::WCSO_;}
wcsp		{return token::WCSP_;}
wcsq		{return token::WCSQ_;}
wcsr		{return token::WCSR_;}
wcss		{return token::WCSS_;}
wcst		{return token::WCST_;}
wcsu		{return token::WCSU_;}
wcsv		{return token::WCSV_;}
wcsw		{return token::WCSW_;}
wcsx		{return token::WCSX_;}
wcsy		{return token::WCSY_;}
wcsz		{return token::WCSZ_;}
wcs0		{return token::WCS0_;}
width		{return token::WIDTH_;}
x		{return token::X_;}
y		{return token::Y_;}
yes		{return token::YES_;}


[+-]?{D}+	{ // Integer
		  yylval->integer = atoi(yytext);
		  return token::INT;
		}

[+-]?{D}+"."?({E})? |
[+-]?{D}*"."{D}+({E})? { // Real Number
		  yylval->real = atof(yytext);
		  return token::REAL;
		}

[+-]?{D}+"."?d |
[+-]?{D}*"."{D}+d { // degrees
		  yytext[yyleng-1] = '\0';
		  yylval->real = atof(yytext);
		  return token::ANGDEGREE;
		}	

[+-]?{D}+"."?r |
[+-]?{D}*"."{D}+r { // radians
		  yytext[yyleng-1] = '\0';
		  yylval->real = atof(yytext);
		  return token::ANGRADIAN;
		}

[+-]?{D}+"."?p |
[+-]?{D}*"."{D}+p { // physical coords
		  yytext[yyleng-1] = '\0';
		  yylval->real = atof(yytext);
		  return token::PHYCOORD;
		}

[+-]?{D}+"."?i |
[+-]?{D}*"."{D}+i { // image coords
		  yytext[yyleng-1] = '\0';
		  yylval->real = atof(yytext);
		  return token::IMGCOORD;
		}

{D}+"."?' |
{D}*"."{D}+'	{ // minutes of arc
		  yytext[yyleng-1] = '\0';
		  yylval->real = atof(yytext);
		  return token::ARCMINUTE;
		}	

{D}+"."?\" |
{D}*"."{D}+\"	{ // seconds of arc
		  yytext[yyleng-1] = '\0';
		  yylval->real = atof(yytext);
		  return token::ARCSECOND;
		}

[+-]?{D}+:{D}+:{D}+"."? |
[+-]?{D}+:{D}+:{D}*"."{D}+ { // Sexagesimal
		  int ll = yyleng <(MKBUFSIZE-1) ? yyleng:(MKBUFSIZE-1);
		  strncpy(yylval->str,yytext,ll);
	          yylval->str[ll] = '\0';
		  return token::SEXSTR;
		}	

[+-]?{D}+h{D}+m{D}+"."?s |
[+-]?{D}+h{D}+m{D}*"."{D}+s { // HMS
		  int ll = yyleng <(MKBUFSIZE-1) ? yyleng:(MKBUFSIZE-1);
		  strncpy(yylval->str,yytext,ll);
	          yylval->str[ll] = '\0';
		  return token::HMSSTR;
		}	

[+-]?{D}+d{D}+m{D}+"."?s |
[+-]?{D}+d{D}+m{D}*"."{D}+s { // DMS
		  int ll = yyleng <(MKBUFSIZE-1) ? yyleng:(MKBUFSIZE-1);
		  strncpy(yylval->str,yytext,ll);
	          yylval->str[ll] = '\0';
		  return token::DMSSTR;
		}	

\"[^\"\n]*\" | 
\'[^\'\n]*\'	{ // Quoted String
		  int ll = (yyleng-2)<(MKBUFSIZE-1) ? (yyleng-2):(MKBUFSIZE-1);
		  strncpy(yylval->str,yytext+1,ll); // skip the " " 
	          yylval->str[ll] = '\0'; // Remove the '"' 
		  return token::STRING;
		}

\{[^\}\n]*\} 	{ // Quoted String
		  int ll = (yyleng-2)<(MKBUFSIZE-1) ? (yyleng-2):(MKBUFSIZE-1);
		  strncpy(yylval->str,yytext+1,ll); // skip the '{'
	          yylval->str[ll] = '\0'; // Remove the '}'
		  return token::STRING;
		}

[0-9A-Za-z]+	{ // General String
		  int ll = yyleng <(MKBUFSIZE-1) ? yyleng:(MKBUFSIZE-1);
		  strncpy(yylval->str,yytext,ll);
	          yylval->str[ll] = '\0';
		  return token::STRING;
		}

[ \t]+		{ // White Spaces
		} 

\r\n		{ // windows line feed
		  return token::EOL_;
		}

\\n		{ // fake line feed
		  return token::EOL_;
		}

\n		{ // linefeed
		  return token::EOL_;
		}

<<EOF>>		{ // eof
		  return token::EOF_;
		}

.		{ // Else, return the char
		  return static_cast<token_type>(*yytext);
		}

%%

void casa::viewer::ds9lex::discard( int doit ) {
   begin(DISCARD, doit);
}


void casa::viewer::ds9lex::begin(int which, int doit)
{
  BEGIN which;
  if (doit)
    yyless(0);
}

void casa::viewer::ds9lex::set_debug(bool b) { yy_flex_debug = b; }

//------------------------------------------------------------------------------------------
#ifdef yylex
#undef yylex
#endif
// This implementation of ExampleFlexLexer::yylex() is required to fill the
// vtable of the class ExampleFlexLexer. We define the scanner's main yylex
// function via YY_DECL to reside in the Scanner class instead.
int ds9lex_FlexLexer::yylex()
{
    std::cerr << "in ExampleFlexLexer::yylex() !" << std::endl;
    return 0;
}
//------------------------------------------------------------------------------------------
// When the scanner receives an end-of-file indication from YY_INPUT, it then
// checks the yywrap() function. If yywrap() returns false (zero), then it is
// assumed that the function has gone ahead and set up `yyin' to point to
// another input file, and scanning continues. If it returns true (non-zero),
// then the scanner terminates, returning 0 to its caller.
int ds9lex_FlexLexer::yywrap() { return 1; }
//------------------------------------------------------------------------------------------
