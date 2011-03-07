/*============================================================================

    WCSLIB 4.3 - an implementation of the FITS WCS standard.
    Copyright (C) 1995-2007, Mark Calabretta

    This file is part of WCSLIB.

    WCSLIB is free software: you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License as published by the
    Free Software Foundation, either version 3 of the License, or (at your
    option) any later version.

    WCSLIB is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
    more details.

    You should have received a copy of the GNU Lesser General Public License
    along with WCSLIB.  If not, see <http://www.gnu.org/licenses/>.

    Correspondence concerning WCSLIB may be directed to:
       Internet email: mcalabre@atnf.csiro.au
       Postal address: Dr. Mark Calabretta
                       Australia Telescope National Facility, CSIRO
                       PO Box 76
                       Epping NSW 1710
                       AUSTRALIA

    Author: Mark Calabretta, Australia Telescope National Facility
    http://www.atnf.csiro.au/~mcalabre/index.html
    $Id: wcsutrn.l,v 4.3 2007/12/27 05:41:36 cal103 Exp $
*=============================================================================
*
*   wcsutrn.l is a Flex description file containing the definition of a
*   lexical scanner that translates non-standard FITS units specifications.
*
*   It requires Flex v2.5.4 or later.
*
*   Refer to wcsunits.h for a description of the user interface and operating
*   notes.
*
*===========================================================================*/

/* Options. */
%option full
%option never-interactive
%option noyywrap
%option outfile="wcsutrn.c"
%option prefix="wcsutrn"

/* Exclusive start states. */
%x NEXT FLUSH

%{
/* To get the prototype for fileno() from stdio.h when gcc is invoked with
 * -std=c89 (same as -ansi) or -std=c99 since we do not define YY_INPUT. */
#define _POSIX_SOURCE 1

#include <stdio.h>
#include <string.h>

#include "wcsunits.h"

#define YY_DECL int wcsutrn(int ctrl, char unitstr[])

#ifdef DEBUG
  /* Print text as it is matched. */
  #define YY_USER_ACTION printf("Rule %03d: \"%s\"\n", yy_act, yytext);
#endif

%}

%%
	char orig[80], subs[80];
	int bracket = 0;
	int unsafe  = 0;
	int status  = -1;
	YY_BUFFER_STATE inbuff;
        int yylex_destroy(void);
	
	*orig = '\0';
	*subs = '\0';
	
	inbuff = yy_scan_string(unitstr);
	*unitstr = '\0';
	BEGIN(INITIAL);
	
	#ifdef DEBUG
	printf("\n%s ->\n", unitstr);
	#endif

^" "*"[" {
	  /* Rule 001: Looks like a keycomment. */
	  strcat(unitstr, "[");
	  bracket = 1;
	}

" "+	  /* Rule 002: Discard leading whitespace. */

[^A-Za-z] {
	  /* Rule 003: Non-alphabetic character. */
	  strcat(unitstr, yytext);
	  if (bracket && *yytext == ']') {
	    BEGIN(FLUSH);
	  }
	}

angstrom {
	  /* Rule 004. */
	  strcpy(orig, yytext);
	  strcpy(subs, "Angstrom");
	  BEGIN(NEXT);
	}

arcmins|ARCMINS? {
	  /* Rule 005. */
	  strcpy(orig, yytext);
	  strcpy(subs, "arcmin");
	  BEGIN(NEXT);
	}

arcsecs|ARCSECS? {
	  /* Rule 006. */
	  strcpy(orig, yytext);
	  strcpy(subs, "arcsec");
	  BEGIN(NEXT);
	}

BEAM {
	  /* Rule 007. */
	  strcpy(orig, yytext);
	  strcpy(subs, "beam");
	  BEGIN(NEXT);
	}

Byte {
	  /* Rule 008. */
	  strcpy(orig, yytext);
	  strcpy(subs, "byte");
	  BEGIN(NEXT);
	}

days?|DAYS? {
	  /* Rule 009. */
	  strcpy(orig, yytext);
	  strcpy(subs, "d");
	  BEGIN(NEXT);
	}

D {
	  /* Rule 010. */
	  unsafe = 1;
	  strcpy(orig, yytext);
	  strcpy(subs, (ctrl & 4) ? "d" : "D");
	  BEGIN(NEXT);
	}

degrees?|DEG|DEGREES? {
	  /* Rule 011. */
	  strcpy(orig, yytext);
	  strcpy(subs, "deg");
	  BEGIN(NEXT);
	}

GHZ {
	  /* Rule 012. */
	  strcpy(orig, yytext);
	  strcpy(subs, "GHz");
	  BEGIN(NEXT);
	}

hr|HR {
	  /* Rule 013. */
	  strcpy(orig, yytext);
	  strcpy(subs, "h");
	  BEGIN(NEXT);
	}

H {
	  /* Rule 014. */
	  unsafe = 1;
	  strcpy(orig, yytext);
	  strcpy(subs, (ctrl & 2) ? "h" : "H");
	  BEGIN(NEXT);
	}

hz|HZ {
	  /* Rule 015. */
	  strcpy(orig, yytext);
	  strcpy(subs, "Hz");
	  BEGIN(NEXT);
	}

KHZ {
	  /* Rule 016. */
	  strcpy(orig, yytext);
	  strcpy(subs, "kHz");
	  BEGIN(NEXT);
	}

JY {
	  /* Rule 017. */
	  strcpy(orig, yytext);
	  strcpy(subs, "Jy");
	  BEGIN(NEXT);
	}

[kK]elvins?|KELVINS? {
	  /* Rule 018. */
	  strcpy(orig, yytext);
	  strcpy(subs, "K");
	  BEGIN(NEXT);
	}

KM {
	  /* Rule 019. */
	  strcpy(orig, yytext);
	  strcpy(subs, "km");
	  BEGIN(NEXT);
	}

metres?|meters?|M|METRES?|METERS? {
	  /* Rule 020. */
	  strcpy(orig, yytext);
	  strcpy(subs, "m");
	  BEGIN(NEXT);
	}

MIN {
	  /* Rule 021. */
	  strcpy(orig, yytext);
	  strcpy(subs, "min");
	  BEGIN(NEXT);
	}

MHZ {
	  /* Rule 022. */
	  strcpy(orig, yytext);
	  strcpy(subs, "MHz");
	  BEGIN(NEXT);
	}

Ohm {
	  /* Rule 023. */
	  strcpy(orig, yytext);
	  strcpy(subs, "ohm");
	  BEGIN(NEXT);
	}

[pP]ascals?|PASCALS? {
	  /* Rule 024. */
	  strcpy(orig, yytext);
	  strcpy(subs, "Pa");
	  BEGIN(NEXT);
	}

pixels|PIXELS? {
	  /* Rule 025. */
	  strcpy(orig, yytext);
	  strcpy(subs, "pixel");
	  BEGIN(NEXT);
	}

radians?|RAD|RADIANS? {
	  /* Rule 026. */
	  strcpy(orig, yytext);
	  strcpy(subs, "rad");
	  BEGIN(NEXT);
	}

sec|seconds?|SEC|SECONDS? {
	  /* Rule 027. */
	  strcpy(orig, yytext);
	  strcpy(subs, "s");
	  BEGIN(NEXT);
	}

S {
	  /* Rule 028. */
	  unsafe = 1;
	  strcpy(orig, yytext);
	  strcpy(subs, (ctrl & 1) ? "s" : "S");
	  BEGIN(NEXT);
	}

[vV]olts?|VOLTS? {
	  /* Rule 029. */
	  strcpy(orig, yytext);
	  strcpy(subs, "V");
	  BEGIN(NEXT);
	}

years?|YR|YEARS? {
	  /* Rule 030. */
	  strcpy(orig, yytext);
	  strcpy(subs, "yr");
	  BEGIN(NEXT);
	}

[A-Za-z]+ {
	  /* Rule 031: Not a recognized alias. */
	  strcpy(orig, yytext);
	  strcpy(subs, orig);
	  BEGIN(NEXT);
	}

<NEXT>[A-Za-z]+ {
	  /* Rule 032: Reject the alias match. */
	  strcat(orig, yytext);
	  strcpy(subs, orig);
	}

<NEXT>" "+[^A-Za-z] {
	  /* Rule 033: Discard separating whitespace. */
	  unput(yytext[yyleng-1]);
	}

<NEXT>" "+[A-Za-z] {
	  /* Rule 034: Compress separating whitespace. */
	  strcat(unitstr, subs);
	  strcat(unitstr, " ");
	  if (strcmp(orig, subs)) status = 0;
	  unput(yytext[yyleng-1]);
	  *subs = '\0';
	  BEGIN(INITIAL);
	}

<NEXT>. {
	  /* Rule 035: Copy anything else unchanged. */
	  strcat(unitstr, subs);
	  if (strcmp(orig, subs)) status = 0;
	  unput(*yytext);
	  *subs = '\0';
	  BEGIN(INITIAL);
	}

<FLUSH>.* {
	  /* Rule 036: Copy out remaining input. */
	  strcat(unitstr, yytext);
	}

<<EOF>> {
	  /* Rule unnumbered: End-of-string. */
	  if (*subs) {
	    strcat(unitstr, subs);
	    if (strcmp(orig, subs)) status = 0;
	  }
	
	  yylex_destroy();
	  return unsafe ? 12 : status;
	}

%%
