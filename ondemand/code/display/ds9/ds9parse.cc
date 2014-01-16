/* A Bison parser, made by GNU Bison 2.6.  */

/* Skeleton implementation for Bison LALR(1) parsers in C++

      Copyright (C) 2002-2012 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

// Take the name prefix into account.
#define yylex   casa::viewerlex

/* First part of user declarations.  */

/* Line 278 of lalr1.cc  */
#line 22 "ds9/ds9parse.yy"

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


/* Line 278 of lalr1.cc  */
#line 180 "/Users/drs/develop/casa/code/build/display/ds9parse.cc"


#include "ds9parse.hh"

/* User implementation prologue.  */

/* Line 284 of lalr1.cc  */
#line 327 "ds9/ds9parse.yy"

#include <display/ds9/ds9lex.h>

/* this "connects" the bison parser in the driver to the flex scanner class
 * object. it defines the yylex() function call to pull the next token from the
 * current lexer object of the driver context. */
#undef yylex
#define yylex driver.lexer->lex


/* Line 284 of lalr1.cc  */
#line 200 "/Users/drs/develop/casa/code/build/display/ds9parse.cc"


# ifndef YY_NULL
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULL anullptr
#  else
#   define YY_NULL 0
#  endif
# endif

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* FIXME: INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

# ifndef YYLLOC_DEFAULT
#  define YYLLOC_DEFAULT(Current, Rhs, N)                               \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).begin  = YYRHSLOC (Rhs, 1).begin;                   \
          (Current).end    = YYRHSLOC (Rhs, N).end;                     \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).begin = (Current).end = YYRHSLOC (Rhs, 0).end;      \
        }                                                               \
    while (/*CONSTCOND*/ false)
# endif


/* Suppress unused-variable warnings by "using" E.  */
#define YYUSE(e) ((void) (e))

/* Enable debugging if requested.  */
#if YYDEBUG

/* A pseudo ostream that takes yydebug_ into account.  */
# define YYCDEBUG if (yydebug_) (*yycdebug_)

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)	\
do {							\
  if (yydebug_)						\
    {							\
      *yycdebug_ << Title << ' ';			\
      yy_symbol_print_ ((Type), (Value), (Location));	\
      *yycdebug_ << std::endl;				\
    }							\
} while (false)

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug_)				\
    yy_reduce_print_ (Rule);		\
} while (false)

# define YY_STACK_PRINT()		\
do {					\
  if (yydebug_)				\
    yystack_print_ ();			\
} while (false)

#else /* !YYDEBUG */

# define YYCDEBUG if (false) std::cerr
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_REDUCE_PRINT(Rule)
# define YY_STACK_PRINT()

#endif /* !YYDEBUG */

#define yyerrok		(yyerrstatus_ = 0)
#define yyclearin	(yychar = yyempty_)

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab
#define YYRECOVERING()  (!!yyerrstatus_)


namespace casa {
	namespace viewer {

		/* Line 352 of lalr1.cc  */
#line 296 "/Users/drs/develop/casa/code/build/display/ds9parse.cc"

		/* Return YYSTR after stripping away unnecessary quotes and
		   backslashes, so that it's suitable for yyerror.  The heuristic is
		   that double-quoting is unnecessary unless the string contains an
		   apostrophe, a comma, or backslash (other than backslash-backslash).
		   YYSTR is taken from yytname.  */
		std::string
		ds9parse::yytnamerr_ (const char *yystr) {
			if (*yystr == '"') {
				std::string yyr = "";
				char const *yyp = yystr;

				for (;;)
					switch (*++yyp) {
					case '\'':
					case ',':
						goto do_not_strip_quotes;

					case '\\':
						if (*++yyp != '\\')
							goto do_not_strip_quotes;
						/* Fall through.  */
					default:
						yyr += *yyp;
						break;

					case '"':
						return yyr;
					}
do_not_strip_quotes:
				;
			}

			return yystr;
		}


		/// Build a parser object.
		ds9parse::ds9parse (class ds9parser &driver_yyarg, class ds9context *fr_yyarg)
			:
#if YYDEBUG
			yydebug_ (false),
			yycdebug_ (&std::cerr),
#endif
			driver (driver_yyarg),
			fr (fr_yyarg) {
		}

		ds9parse::~ds9parse () {
		}

#if YYDEBUG
		/*--------------------------------.
		| Print this symbol on YYOUTPUT.  |
		`--------------------------------*/

		inline void
		ds9parse::yy_symbol_value_print_ (int yytype,
		                                  const semantic_type* yyvaluep, const location_type* yylocationp) {
			YYUSE (yylocationp);
			YYUSE (yyvaluep);
			std::ostream& yyo = debug_stream ();
			std::ostream& yyoutput = yyo;
			YYUSE (yyoutput);
			switch (yytype) {
			default:
				break;
			}
		}


		void
		ds9parse::yy_symbol_print_ (int yytype,
		                            const semantic_type* yyvaluep, const location_type* yylocationp) {
			*yycdebug_ << (yytype < yyntokens_ ? "token" : "nterm")
			           << ' ' << yytname_[yytype] << " ("
			           << *yylocationp << ": ";
			yy_symbol_value_print_ (yytype, yyvaluep, yylocationp);
			*yycdebug_ << ')';
		}
#endif

		void
		ds9parse::yydestruct_ (const char* yymsg,
		                       int yytype, semantic_type* yyvaluep, location_type* yylocationp) {
			YYUSE (yylocationp);
			YYUSE (yymsg);
			YYUSE (yyvaluep);

			YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

			switch (yytype) {

			default:
				break;
			}
		}

		void
		ds9parse::yypop_ (unsigned int n) {
			yystate_stack_.pop (n);
			yysemantic_stack_.pop (n);
			yylocation_stack_.pop (n);
		}

#if YYDEBUG
		std::ostream&
		ds9parse::debug_stream () const {
			return *yycdebug_;
		}

		void
		ds9parse::set_debug_stream (std::ostream& o) {
			yycdebug_ = &o;
		}


		ds9parse::debug_level_type
		ds9parse::debug_level () const {
			return yydebug_;
		}

		void
		ds9parse::set_debug_level (debug_level_type l) {
			yydebug_ = l;
		}
#endif

		inline bool
		ds9parse::yy_pact_value_is_default_ (int yyvalue) {
			return yyvalue == yypact_ninf_;
		}

		inline bool
		ds9parse::yy_table_value_is_error_ (int yyvalue) {
			return yyvalue == yytable_ninf_;
		}

		int
		ds9parse::parse () {
			/// Lookahead and lookahead in internal form.
			int yychar = yyempty_;
			int yytoken = 0;

			/* State.  */
			int yyn;
			int yylen = 0;
			int yystate = 0;

			/* Error handling.  */
			int yynerrs_ = 0;
			int yyerrstatus_ = 0;

			/// Semantic value of the lookahead.
			semantic_type yylval;
			/// Location of the lookahead.
			location_type yylloc;
			/// The locations where the error started and ended.
			location_type yyerror_range[3];

			/// $$.
			semantic_type yyval;
			/// @$.
			location_type yyloc;

			int yyresult;

			YYCDEBUG << "Starting parse" << std::endl;


			/* User initialization code.  */

			/* Line 538 of lalr1.cc  */
#line 163 "ds9/ds9parse.yy"
			{
				// initialize the initial location object
				yylloc.begin.filename = yylloc.end.filename = &driver.streamname;
			}

			/* Line 538 of lalr1.cc  */
#line 494 "/Users/drs/develop/casa/code/build/display/ds9parse.cc"

			/* Initialize the stacks.  The initial state will be pushed in
			   yynewstate, since the latter expects the semantical and the
			   location values to have been already stored, initialize these
			   stacks with a primary value.  */
			yystate_stack_ = state_stack_type (0);
			yysemantic_stack_ = semantic_stack_type (0);
			yylocation_stack_ = location_stack_type (0);
			yysemantic_stack_.push (yylval);
			yylocation_stack_.push (yylloc);

			/* New state.  */
yynewstate:
			yystate_stack_.push (yystate);
			YYCDEBUG << "Entering state " << yystate << std::endl;

			/* Accept?  */
			if (yystate == yyfinal_)
				goto yyacceptlab;

			goto yybackup;

			/* Backup.  */
yybackup:

			/* Try to take a decision without lookahead.  */
			yyn = yypact_[yystate];
			if (yy_pact_value_is_default_ (yyn))
				goto yydefault;

			/* Read a lookahead token.  */
			if (yychar == yyempty_) {
				YYCDEBUG << "Reading a token: ";
				yychar = yylex (&yylval, &yylloc);
			}


			/* Convert token to internal form.  */
			if (yychar <= yyeof_) {
				yychar = yytoken = yyeof_;
				YYCDEBUG << "Now at end of input." << std::endl;
			} else {
				yytoken = yytranslate_ (yychar);
				YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
			}

			/* If the proper action on seeing token YYTOKEN is to reduce or to
			   detect an error, take that action.  */
			yyn += yytoken;
			if (yyn < 0 || yylast_ < yyn || yycheck_[yyn] != yytoken)
				goto yydefault;

			/* Reduce or error.  */
			yyn = yytable_[yyn];
			if (yyn <= 0) {
				if (yy_table_value_is_error_ (yyn))
					goto yyerrlab;
				yyn = -yyn;
				goto yyreduce;
			}

			/* Shift the lookahead token.  */
			YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

			/* Discard the token being shifted.  */
			yychar = yyempty_;

			yysemantic_stack_.push (yylval);
			yylocation_stack_.push (yylloc);

			/* Count tokens shifted since error; after three, turn off error
			   status.  */
			if (yyerrstatus_)
				--yyerrstatus_;

			yystate = yyn;
			goto yynewstate;

			/*-----------------------------------------------------------.
			| yydefault -- do the default action for the current state.  |
			`-----------------------------------------------------------*/
yydefault:
			yyn = yydefact_[yystate];
			if (yyn == 0)
				goto yyerrlab;
			goto yyreduce;

			/*-----------------------------.
			| yyreduce -- Do a reduction.  |
			`-----------------------------*/
yyreduce:
			yylen = yyr2_[yyn];
			/* If YYLEN is nonzero, implement the default value of the action:
			   `$$ = $1'.  Otherwise, use the top of the stack.

			   Otherwise, the following line sets YYVAL to garbage.
			   This behavior is undocumented and Bison
			   users should not rely upon it.  */
			if (yylen)
				yyval = yysemantic_stack_[yylen - 1];
			else
				yyval = yysemantic_stack_[0];

			{
				slice<location_type, location_stack_type> slice (yylocation_stack_, yylen);
				YYLLOC_DEFAULT (yyloc, slice, yylen);
			}
			YY_REDUCE_PRINT (yyn);
			switch (yyn) {
			case 7:

				/* Line 663 of lalr1.cc  */
#line 348 "ds9/ds9parse.yy"
			{std::cerr << "DS9 Regions File 3.1" << std::endl;}
			break;

			case 9:

				/* Line 663 of lalr1.cc  */
#line 351 "ds9/ds9parse.yy"
			{globalTile = (yysemantic_stack_[(2) - (2)].integer);}
			break;

			case 10:

				/* Line 663 of lalr1.cc  */
#line 353 "ds9/ds9parse.yy"
			{globalSystem=(casa::viewer::CoordSystem)(yysemantic_stack_[(1) - (1)].integer);}
			break;

			case 12:

				/* Line 663 of lalr1.cc  */
#line 354 "ds9/ds9parse.yy"
			{globalSystem=globalWCS; globalSky=(casa::viewer::SkyFrame)(yysemantic_stack_[(1) - (1)].integer);}
			break;

			case 14:

				/* Line 663 of lalr1.cc  */
#line 355 "ds9/ds9parse.yy"
			{globalSystem=globalWCS; globalSky=NATIVEWCS;}
			break;

			case 21:

				/* Line 663 of lalr1.cc  */
#line 364 "ds9/ds9parse.yy"
			{globalTile = (yysemantic_stack_[(2) - (2)].integer);}
			break;

			case 22:

				/* Line 663 of lalr1.cc  */
#line 365 "ds9/ds9parse.yy"
			{DISCARD_(1);}
			break;

			case 25:

				/* Line 663 of lalr1.cc  */
#line 369 "ds9/ds9parse.yy"
			{DISCARD_(1);}
			break;

			case 28:

				/* Line 663 of lalr1.cc  */
#line 373 "ds9/ds9parse.yy"
			{DISCARD_(1);}
			break;

			case 29:

				/* Line 663 of lalr1.cc  */
#line 373 "ds9/ds9parse.yy"
			{strncpy(localComment,(yysemantic_stack_[(4) - (3)].str),80);}
			break;

			case 31:

				/* Line 663 of lalr1.cc  */
#line 375 "ds9/ds9parse.yy"
			{DISCARD_(1);}
			break;

			case 32:

				/* Line 663 of lalr1.cc  */
#line 376 "ds9/ds9parse.yy"
			{strncpy(localComment,(yysemantic_stack_[(5) - (4)].str),80);}
			break;

			case 34:

				/* Line 663 of lalr1.cc  */
#line 380 "ds9/ds9parse.yy"
			{DISCARD_(1);}
			break;

			case 35:

				/* Line 663 of lalr1.cc  */
#line 380 "ds9/ds9parse.yy"
			{strncpy(localComment,(yysemantic_stack_[(3) - (2)].str),80);}
			break;

			case 37:

				/* Line 663 of lalr1.cc  */
#line 382 "ds9/ds9parse.yy"
			{DISCARD_(1);}
			break;

			case 38:

				/* Line 663 of lalr1.cc  */
#line 382 "ds9/ds9parse.yy"
			{strncpy(localComment,(yysemantic_stack_[(4) - (3)].str),80);}
			break;

			case 40:

				/* Line 663 of lalr1.cc  */
#line 386 "ds9/ds9parse.yy"
			{YYACCEPT;}
			break;

			case 41:

				/* Line 663 of lalr1.cc  */
#line 389 "ds9/ds9parse.yy"
			{(yyval.real)=(yysemantic_stack_[(1) - (1)].real);}
			break;

			case 42:

				/* Line 663 of lalr1.cc  */
#line 390 "ds9/ds9parse.yy"
			{(yyval.real)=(yysemantic_stack_[(1) - (1)].integer);}
			break;

			case 43:

				/* Line 663 of lalr1.cc  */
#line 393 "ds9/ds9parse.yy"
			{set_debug_level(1);}
			break;

			case 44:

				/* Line 663 of lalr1.cc  */
#line 394 "ds9/ds9parse.yy"
			{set_debug_level(0);}
			break;

			case 45:

				/* Line 663 of lalr1.cc  */
#line 397 "ds9/ds9parse.yy"
			{(yyval.integer)=((yysemantic_stack_[(1) - (1)].integer) ? 1 : 0);}
			break;

			case 46:

				/* Line 663 of lalr1.cc  */
#line 399 "ds9/ds9parse.yy"
			{(yyval.integer)=1;}
			break;

			case 47:

				/* Line 663 of lalr1.cc  */
#line 400 "ds9/ds9parse.yy"
			{(yyval.integer)=1;}
			break;

			case 48:

				/* Line 663 of lalr1.cc  */
#line 401 "ds9/ds9parse.yy"
			{(yyval.integer)=1;}
			break;

			case 49:

				/* Line 663 of lalr1.cc  */
#line 402 "ds9/ds9parse.yy"
			{(yyval.integer)=1;}
			break;

			case 50:

				/* Line 663 of lalr1.cc  */
#line 404 "ds9/ds9parse.yy"
			{(yyval.integer)=0;}
			break;

			case 51:

				/* Line 663 of lalr1.cc  */
#line 405 "ds9/ds9parse.yy"
			{(yyval.integer)=0;}
			break;

			case 52:

				/* Line 663 of lalr1.cc  */
#line 406 "ds9/ds9parse.yy"
			{(yyval.integer)=0;}
			break;

			case 53:

				/* Line 663 of lalr1.cc  */
#line 407 "ds9/ds9parse.yy"
			{(yyval.integer)=0;}
			break;

			case 60:

				/* Line 663 of lalr1.cc  */
#line 422 "ds9/ds9parse.yy"
			{cStatus = 0;}
			break;

			case 61:

				/* Line 663 of lalr1.cc  */
#line 423 "ds9/ds9parse.yy"
			{cStatus = 1;}
			break;

			case 62:

				/* Line 663 of lalr1.cc  */
#line 424 "ds9/ds9parse.yy"
			{cStatus = 1;}
			break;

			case 63:

				/* Line 663 of lalr1.cc  */
#line 427 "ds9/ds9parse.yy"
			{(yyval.real) = fr->mapAngleToRef(0,localSystem,localSky);}
			break;

			case 64:

				/* Line 663 of lalr1.cc  */
#line 428 "ds9/ds9parse.yy"
			{(yyval.real) = (yysemantic_stack_[(1) - (1)].real);}
			break;

			case 65:

				/* Line 663 of lalr1.cc  */
#line 431 "ds9/ds9parse.yy"
			{(yyval.real) = fr->mapAngleToRef(degToRad((yysemantic_stack_[(1) - (1)].real)),localSystem,localSky);}
			break;

			case 66:

				/* Line 663 of lalr1.cc  */
#line 432 "ds9/ds9parse.yy"
			{(yyval.real) = fr->mapAngleToRef(degToRad((yysemantic_stack_[(1) - (1)].real)),localSystem,localSky);}
			break;

			case 67:

				/* Line 663 of lalr1.cc  */
#line 433 "ds9/ds9parse.yy"
			{(yyval.real) = fr->mapAngleToRef((yysemantic_stack_[(1) - (1)].real),localSystem,localSky);}
			break;

			case 68:

				/* Line 663 of lalr1.cc  */
#line 436 "ds9/ds9parse.yy"
			{(yyval.real) = FITSPTR->mapLenToRef((yysemantic_stack_[(1) - (1)].real), localSystem, DEGREES);}
			break;

			case 69:

				/* Line 663 of lalr1.cc  */
#line 437 "ds9/ds9parse.yy"
			{(yyval.real) = FITSPTR->mapLenToRef((yysemantic_stack_[(1) - (1)].real), PHYSICAL);}
			break;

			case 70:

				/* Line 663 of lalr1.cc  */
#line 438 "ds9/ds9parse.yy"
			{(yyval.real) = FITSPTR->mapLenToRef((yysemantic_stack_[(1) - (1)].real), IMAGE);}
			break;

			case 71:

				/* Line 663 of lalr1.cc  */
#line 439 "ds9/ds9parse.yy"
			{(yyval.real) = FITSPTR->mapLenToRef((yysemantic_stack_[(1) - (1)].real), checkWCSSystem(), DEGREES);}
			break;

			case 72:

				/* Line 663 of lalr1.cc  */
#line 440 "ds9/ds9parse.yy"
			{(yyval.real) = FITSPTR->mapLenToRef((yysemantic_stack_[(1) - (1)].real), checkWCSSystem(), ARCMIN);}
			break;

			case 73:

				/* Line 663 of lalr1.cc  */
#line 441 "ds9/ds9parse.yy"
			{(yyval.real) = FITSPTR->mapLenToRef((yysemantic_stack_[(1) - (1)].real), checkWCSSystem(), ARCSEC);}
			break;

			case 74:

				/* Line 663 of lalr1.cc  */
#line 445 "ds9/ds9parse.yy"
			{
				casa::viewer::Vector r = FITSPTR->mapLenToRef(doubletovec((yysemantic_stack_[(3) - (1)].real),(yysemantic_stack_[(3) - (3)].real)), localSystem, DEGREES);
				(yyval.vector)[0] = r[0];
				(yyval.vector)[1] = r[1];
				(yyval.vector)[2] = r[2];
			}
			break;

			case 75:

				/* Line 663 of lalr1.cc  */
#line 452 "ds9/ds9parse.yy"
			{
				casa::viewer::Vector r = FITSPTR->mapLenToRef(doubletovec((yysemantic_stack_[(3) - (1)].real),(yysemantic_stack_[(3) - (3)].real)), PHYSICAL);
				(yyval.vector)[0] = r[0];
				(yyval.vector)[1] = r[1];
				(yyval.vector)[2] = r[2];
			}
			break;

			case 76:

				/* Line 663 of lalr1.cc  */
#line 459 "ds9/ds9parse.yy"
			{
				casa::viewer::Vector r = FITSPTR->mapLenToRef(doubletovec((yysemantic_stack_[(3) - (1)].real),(yysemantic_stack_[(3) - (3)].real)), IMAGE);
				(yyval.vector)[0] = r[0];
				(yyval.vector)[1] = r[1];
				(yyval.vector)[2] = r[2];
			}
			break;

			case 77:

				/* Line 663 of lalr1.cc  */
#line 466 "ds9/ds9parse.yy"
			{
				casa::viewer::Vector r=FITSPTR->mapLenToRef(doubletovec((yysemantic_stack_[(3) - (1)].real),(yysemantic_stack_[(3) - (3)].real)),checkWCSSystem(),DEGREES);
				(yyval.vector)[0] = r[0];
				(yyval.vector)[1] = r[1];
				(yyval.vector)[2] = r[2];
			}
			break;

			case 78:

				/* Line 663 of lalr1.cc  */
#line 473 "ds9/ds9parse.yy"
			{
				casa::viewer::Vector r=FITSPTR->mapLenToRef(doubletovec((yysemantic_stack_[(3) - (1)].real),(yysemantic_stack_[(3) - (3)].real)),checkWCSSystem(),ARCMIN);
				(yyval.vector)[0] = r[0];
				(yyval.vector)[1] = r[1];
				(yyval.vector)[2] = r[2];
			}
			break;

			case 79:

				/* Line 663 of lalr1.cc  */
#line 480 "ds9/ds9parse.yy"
			{
				casa::viewer::Vector r=FITSPTR->mapLenToRef(doubletovec((yysemantic_stack_[(3) - (1)].real),(yysemantic_stack_[(3) - (3)].real)),checkWCSSystem(),ARCSEC);
				(yyval.vector)[0] = r[0];
				(yyval.vector)[1] = r[1];
				(yyval.vector)[2] = r[2];
			}
			break;

			case 80:

				/* Line 663 of lalr1.cc  */
#line 488 "ds9/ds9parse.yy"
			{(yyval.integer) = (yysemantic_stack_[(3) - (3)].integer);}
			break;

			case 81:

				/* Line 663 of lalr1.cc  */
#line 491 "ds9/ds9parse.yy"
			{(yyval.real) = parseSEXStr((yysemantic_stack_[(1) - (1)].str));}
			break;

			case 82:

				/* Line 663 of lalr1.cc  */
#line 494 "ds9/ds9parse.yy"
			{(yyval.real) = parseHMSStr((yysemantic_stack_[(1) - (1)].str));}
			break;

			case 83:

				/* Line 663 of lalr1.cc  */
#line 497 "ds9/ds9parse.yy"
			{(yyval.real) = parseDMSStr((yysemantic_stack_[(1) - (1)].str));}
			break;

			case 84:

				/* Line 663 of lalr1.cc  */
#line 501 "ds9/ds9parse.yy"
			{
				casa::viewer::Vector r;
				casa::viewer::CoordSystem sys = checkWCSSystem();
				casa::viewer::SkyFrame sky = checkWCSSky();
				if (sky == GALACTIC || sky == ECLIPTIC)
					r = FITSPTR->mapToRef(doubletovec((yysemantic_stack_[(3) - (1)].real),(yysemantic_stack_[(3) - (3)].real)), sys, sky);
				else
					r = FITSPTR->mapToRef(doubletovec((yysemantic_stack_[(3) - (1)].real)*360./24.,(yysemantic_stack_[(3) - (3)].real)), sys, sky);
				(yyval.vector)[0] = r[0];
				(yyval.vector)[1] = r[1];
				(yyval.vector)[2] = r[2];
			}
			break;

			case 85:

				/* Line 663 of lalr1.cc  */
#line 514 "ds9/ds9parse.yy"
			{
				casa::viewer::Vector r = FITSPTR->mapToRef(doubletovec((yysemantic_stack_[(3) - (1)].real),(yysemantic_stack_[(3) - (3)].real)),
				checkWCSSystem(), checkWCSSky());
				(yyval.vector)[0] = r[0];
				(yyval.vector)[1] = r[1];
				(yyval.vector)[2] = r[2];
			}
			break;

			case 86:

				/* Line 663 of lalr1.cc  */
#line 522 "ds9/ds9parse.yy"
			{
				casa::viewer::Vector r = FITSPTR->mapToRef(doubletovec((yysemantic_stack_[(3) - (1)].real),(yysemantic_stack_[(3) - (3)].real)),
				checkWCSSystem(), checkWCSSky());
				(yyval.vector)[0] = r[0];
				(yyval.vector)[1] = r[1];
				(yyval.vector)[2] = r[2];
			}
			break;

			case 87:

				/* Line 663 of lalr1.cc  */
#line 530 "ds9/ds9parse.yy"
			{
				casa::viewer::Vector r = FITSPTR->mapToRef(doubletovec((yysemantic_stack_[(3) - (1)].real),(yysemantic_stack_[(3) - (3)].real)), localSystem, localSky);
				(yyval.vector)[0] = r[0];
				(yyval.vector)[1] = r[1];
				(yyval.vector)[2] = r[2];
			}
			break;

			case 88:

				/* Line 663 of lalr1.cc  */
#line 537 "ds9/ds9parse.yy"
			{
				casa::viewer::Vector r = FITSPTR->mapToRef(doubletovec((yysemantic_stack_[(3) - (1)].real),(yysemantic_stack_[(3) - (3)].real)),
				checkWCSSystem(), checkWCSSky());
				(yyval.vector)[0] = r[0];
				(yyval.vector)[1] = r[1];
				(yyval.vector)[2] = r[2];
			}
			break;

			case 89:

				/* Line 663 of lalr1.cc  */
#line 545 "ds9/ds9parse.yy"
			{
				casa::viewer::Vector r = FITSPTR->mapToRef(doubletovec((yysemantic_stack_[(3) - (1)].real),(yysemantic_stack_[(3) - (3)].real)), IMAGE);
				(yyval.vector)[0] = r[0];
				(yyval.vector)[1] = r[1];
				(yyval.vector)[2] = r[2];
			}
			break;

			case 90:

				/* Line 663 of lalr1.cc  */
#line 552 "ds9/ds9parse.yy"
			{
				casa::viewer::Vector r = FITSPTR->mapToRef(doubletovec((yysemantic_stack_[(3) - (1)].real),(yysemantic_stack_[(3) - (3)].real)), PHYSICAL);
				(yyval.vector)[0] = r[0];
				(yyval.vector)[1] = r[1];
			}
			break;

			case 91:

				/* Line 663 of lalr1.cc  */
#line 559 "ds9/ds9parse.yy"
			{(yyval.integer) = IMAGE;}
			break;

			case 92:

				/* Line 663 of lalr1.cc  */
#line 560 "ds9/ds9parse.yy"
			{(yyval.integer) = PHYSICAL;}
			break;

			case 93:

				/* Line 663 of lalr1.cc  */
#line 561 "ds9/ds9parse.yy"
			{(yyval.integer) = DETECTOR;}
			break;

			case 94:

				/* Line 663 of lalr1.cc  */
#line 562 "ds9/ds9parse.yy"
			{(yyval.integer) = AMPLIFIER;}
			break;

			case 95:

				/* Line 663 of lalr1.cc  */
#line 563 "ds9/ds9parse.yy"
			{(yyval.integer) = (yysemantic_stack_[(1) - (1)].integer); globalWCS = (casa::viewer::CoordSystem)(yysemantic_stack_[(1) - (1)].integer);}
			break;

			case 96:

				/* Line 663 of lalr1.cc  */
#line 566 "ds9/ds9parse.yy"
			{(yyval.integer) = WCS;}
			break;

			case 97:

				/* Line 663 of lalr1.cc  */
#line 567 "ds9/ds9parse.yy"
			{(yyval.integer) = WCSA;}
			break;

			case 98:

				/* Line 663 of lalr1.cc  */
#line 568 "ds9/ds9parse.yy"
			{(yyval.integer) = WCSB;}
			break;

			case 99:

				/* Line 663 of lalr1.cc  */
#line 569 "ds9/ds9parse.yy"
			{(yyval.integer) = WCSC;}
			break;

			case 100:

				/* Line 663 of lalr1.cc  */
#line 570 "ds9/ds9parse.yy"
			{(yyval.integer) = WCSD;}
			break;

			case 101:

				/* Line 663 of lalr1.cc  */
#line 571 "ds9/ds9parse.yy"
			{(yyval.integer) = WCSE;}
			break;

			case 102:

				/* Line 663 of lalr1.cc  */
#line 572 "ds9/ds9parse.yy"
			{(yyval.integer) = WCSF;}
			break;

			case 103:

				/* Line 663 of lalr1.cc  */
#line 573 "ds9/ds9parse.yy"
			{(yyval.integer) = WCSG;}
			break;

			case 104:

				/* Line 663 of lalr1.cc  */
#line 574 "ds9/ds9parse.yy"
			{(yyval.integer) = WCSH;}
			break;

			case 105:

				/* Line 663 of lalr1.cc  */
#line 575 "ds9/ds9parse.yy"
			{(yyval.integer) = WCSI;}
			break;

			case 106:

				/* Line 663 of lalr1.cc  */
#line 576 "ds9/ds9parse.yy"
			{(yyval.integer) = WCSJ;}
			break;

			case 107:

				/* Line 663 of lalr1.cc  */
#line 577 "ds9/ds9parse.yy"
			{(yyval.integer) = WCSK;}
			break;

			case 108:

				/* Line 663 of lalr1.cc  */
#line 578 "ds9/ds9parse.yy"
			{(yyval.integer) = WCSL;}
			break;

			case 109:

				/* Line 663 of lalr1.cc  */
#line 579 "ds9/ds9parse.yy"
			{(yyval.integer) = WCSM;}
			break;

			case 110:

				/* Line 663 of lalr1.cc  */
#line 580 "ds9/ds9parse.yy"
			{(yyval.integer) = WCSN;}
			break;

			case 111:

				/* Line 663 of lalr1.cc  */
#line 581 "ds9/ds9parse.yy"
			{(yyval.integer) = WCSO;}
			break;

			case 112:

				/* Line 663 of lalr1.cc  */
#line 582 "ds9/ds9parse.yy"
			{(yyval.integer) = WCSP;}
			break;

			case 113:

				/* Line 663 of lalr1.cc  */
#line 583 "ds9/ds9parse.yy"
			{(yyval.integer) = WCSQ;}
			break;

			case 114:

				/* Line 663 of lalr1.cc  */
#line 584 "ds9/ds9parse.yy"
			{(yyval.integer) = WCSR;}
			break;

			case 115:

				/* Line 663 of lalr1.cc  */
#line 585 "ds9/ds9parse.yy"
			{(yyval.integer) = WCSS;}
			break;

			case 116:

				/* Line 663 of lalr1.cc  */
#line 586 "ds9/ds9parse.yy"
			{(yyval.integer) = WCST;}
			break;

			case 117:

				/* Line 663 of lalr1.cc  */
#line 587 "ds9/ds9parse.yy"
			{(yyval.integer) = WCSU;}
			break;

			case 118:

				/* Line 663 of lalr1.cc  */
#line 588 "ds9/ds9parse.yy"
			{(yyval.integer) = WCSV;}
			break;

			case 119:

				/* Line 663 of lalr1.cc  */
#line 589 "ds9/ds9parse.yy"
			{(yyval.integer) = WCSW;}
			break;

			case 120:

				/* Line 663 of lalr1.cc  */
#line 590 "ds9/ds9parse.yy"
			{(yyval.integer) = WCSX;}
			break;

			case 121:

				/* Line 663 of lalr1.cc  */
#line 591 "ds9/ds9parse.yy"
			{(yyval.integer) = WCSY;}
			break;

			case 122:

				/* Line 663 of lalr1.cc  */
#line 592 "ds9/ds9parse.yy"
			{(yyval.integer) = WCSZ;}
			break;

			case 123:

				/* Line 663 of lalr1.cc  */
#line 593 "ds9/ds9parse.yy"
			{(yyval.integer) = WCS0;}
			break;

			case 124:

				/* Line 663 of lalr1.cc  */
#line 596 "ds9/ds9parse.yy"
			{(yyval.integer) = FK4;}
			break;

			case 125:

				/* Line 663 of lalr1.cc  */
#line 597 "ds9/ds9parse.yy"
			{(yyval.integer) = FK4;}
			break;

			case 126:

				/* Line 663 of lalr1.cc  */
#line 598 "ds9/ds9parse.yy"
			{(yyval.integer) = FK5;}
			break;

			case 127:

				/* Line 663 of lalr1.cc  */
#line 599 "ds9/ds9parse.yy"
			{(yyval.integer) = FK5;}
			break;

			case 128:

				/* Line 663 of lalr1.cc  */
#line 600 "ds9/ds9parse.yy"
			{(yyval.integer) = ICRS;}
			break;

			case 129:

				/* Line 663 of lalr1.cc  */
#line 601 "ds9/ds9parse.yy"
			{(yyval.integer) = GALACTIC;}
			break;

			case 130:

				/* Line 663 of lalr1.cc  */
#line 602 "ds9/ds9parse.yy"
			{(yyval.integer) = ECLIPTIC;}
			break;

			case 131:

				/* Line 663 of lalr1.cc  */
#line 605 "ds9/ds9parse.yy"
			{(yyval.integer)=DEGREES;}
			break;

			case 132:

				/* Line 663 of lalr1.cc  */
#line 606 "ds9/ds9parse.yy"
			{(yyval.integer)=ARCMIN;}
			break;

			case 133:

				/* Line 663 of lalr1.cc  */
#line 607 "ds9/ds9parse.yy"
			{(yyval.integer)=ARCSEC;}
			break;

			case 134:

				/* Line 663 of lalr1.cc  */
#line 610 "ds9/ds9parse.yy"
			{(yyval.integer) = Marker::SELECT;}
			break;

			case 135:

				/* Line 663 of lalr1.cc  */
#line 611 "ds9/ds9parse.yy"
			{(yyval.integer) = Marker::HIGHLITE;}
			break;

			case 136:

				/* Line 663 of lalr1.cc  */
#line 612 "ds9/ds9parse.yy"
			{(yyval.integer) = Marker::DASH;}
			break;

			case 137:

				/* Line 663 of lalr1.cc  */
#line 613 "ds9/ds9parse.yy"
			{(yyval.integer) = Marker::FIXED;}
			break;

			case 138:

				/* Line 663 of lalr1.cc  */
#line 614 "ds9/ds9parse.yy"
			{(yyval.integer) = Marker::EDIT;}
			break;

			case 139:

				/* Line 663 of lalr1.cc  */
#line 615 "ds9/ds9parse.yy"
			{(yyval.integer) = Marker::MOVE;}
			break;

			case 140:

				/* Line 663 of lalr1.cc  */
#line 616 "ds9/ds9parse.yy"
			{(yyval.integer) = Marker::ROTATE;}
			break;

			case 141:

				/* Line 663 of lalr1.cc  */
#line 617 "ds9/ds9parse.yy"
			{(yyval.integer) = Marker::DELETE;}
			break;

			case 142:

				/* Line 663 of lalr1.cc  */
#line 618 "ds9/ds9parse.yy"
			{(yyval.integer) = Marker::INCLUDE;}
			break;

			case 143:

				/* Line 663 of lalr1.cc  */
#line 619 "ds9/ds9parse.yy"
			{(yyval.integer) = Marker::SOURCE;}
			break;

			case 144:

				/* Line 663 of lalr1.cc  */
#line 622 "ds9/ds9parse.yy"
			{(yyval.integer) = CallBack::SELECTCB;}
			break;

			case 145:

				/* Line 663 of lalr1.cc  */
#line 623 "ds9/ds9parse.yy"
			{(yyval.integer) = CallBack::UNSELECTCB;}
			break;

			case 146:

				/* Line 663 of lalr1.cc  */
#line 624 "ds9/ds9parse.yy"
			{(yyval.integer) = CallBack::HIGHLITECB;}
			break;

			case 147:

				/* Line 663 of lalr1.cc  */
#line 625 "ds9/ds9parse.yy"
			{(yyval.integer) = CallBack::UNHIGHLITECB;}
			break;

			case 148:

				/* Line 663 of lalr1.cc  */
#line 626 "ds9/ds9parse.yy"
			{(yyval.integer) = CallBack::MOVEBEGINCB;}
			break;

			case 149:

				/* Line 663 of lalr1.cc  */
#line 627 "ds9/ds9parse.yy"
			{(yyval.integer) = CallBack::MOVECB;}
			break;

			case 150:

				/* Line 663 of lalr1.cc  */
#line 628 "ds9/ds9parse.yy"
			{(yyval.integer) = CallBack::MOVEENDCB;}
			break;

			case 151:

				/* Line 663 of lalr1.cc  */
#line 629 "ds9/ds9parse.yy"
			{(yyval.integer) = CallBack::EDITBEGINCB;}
			break;

			case 152:

				/* Line 663 of lalr1.cc  */
#line 630 "ds9/ds9parse.yy"
			{(yyval.integer) = CallBack::EDITCB;}
			break;

			case 153:

				/* Line 663 of lalr1.cc  */
#line 631 "ds9/ds9parse.yy"
			{(yyval.integer) = CallBack::EDITENDCB;}
			break;

			case 154:

				/* Line 663 of lalr1.cc  */
#line 632 "ds9/ds9parse.yy"
			{(yyval.integer) = CallBack::ROTATEBEGINCB;}
			break;

			case 155:

				/* Line 663 of lalr1.cc  */
#line 633 "ds9/ds9parse.yy"
			{(yyval.integer) = CallBack::ROTATECB;}
			break;

			case 156:

				/* Line 663 of lalr1.cc  */
#line 634 "ds9/ds9parse.yy"
			{(yyval.integer) = CallBack::ROTATEENDCB;}
			break;

			case 157:

				/* Line 663 of lalr1.cc  */
#line 635 "ds9/ds9parse.yy"
			{(yyval.integer) = CallBack::DELETECB;}
			break;

			case 158:

				/* Line 663 of lalr1.cc  */
#line 636 "ds9/ds9parse.yy"
			{(yyval.integer) = CallBack::TEXTCB;}
			break;

			case 159:

				/* Line 663 of lalr1.cc  */
#line 637 "ds9/ds9parse.yy"
			{(yyval.integer) = CallBack::COLORCB;}
			break;

			case 160:

				/* Line 663 of lalr1.cc  */
#line 638 "ds9/ds9parse.yy"
			{(yyval.integer) = CallBack::LINEWIDTHCB;}
			break;

			case 161:

				/* Line 663 of lalr1.cc  */
#line 639 "ds9/ds9parse.yy"
			{(yyval.integer) = CallBack::PROPERTYCB;}
			break;

			case 162:

				/* Line 663 of lalr1.cc  */
#line 640 "ds9/ds9parse.yy"
			{(yyval.integer) = CallBack::FONTCB;}
			break;

			case 163:

				/* Line 663 of lalr1.cc  */
#line 641 "ds9/ds9parse.yy"
			{(yyval.integer) = CallBack::KEYCB;}
			break;

			case 164:

				/* Line 663 of lalr1.cc  */
#line 642 "ds9/ds9parse.yy"
			{(yyval.integer) = CallBack::UPDATECB;}
			break;

			case 167:

				/* Line 663 of lalr1.cc  */
#line 650 "ds9/ds9parse.yy"
			{
				setProps(&globalProps,(yysemantic_stack_[(3) - (1)].integer),(yysemantic_stack_[(3) - (3)].integer));
				setProps(&localProps,(yysemantic_stack_[(3) - (1)].integer),(yysemantic_stack_[(3) - (3)].integer));
			}
			break;

			case 168:

				/* Line 663 of lalr1.cc  */
#line 655 "ds9/ds9parse.yy"
			{
				strncpy(globalColor,(yysemantic_stack_[(3) - (3)].str),16);
				strncpy(localColor,(yysemantic_stack_[(3) - (3)].str),16);
			}
			break;

			case 169:

				/* Line 663 of lalr1.cc  */
#line 659 "ds9/ds9parse.yy"
			{
				strcpy(globalColor,"#");
				strncat(globalColor,(yysemantic_stack_[(4) - (4)].str),16);
				strncpy(localColor,globalColor,16);
			}
			break;

			case 170:

				/* Line 663 of lalr1.cc  */
#line 665 "ds9/ds9parse.yy"
			{
				globalDash[0] = localDash[0] =(yysemantic_stack_[(4) - (3)].integer);
				globalDash[1] = localDash[1] =(yysemantic_stack_[(4) - (4)].integer);
			}
			break;

			case 171:

				/* Line 663 of lalr1.cc  */
#line 669 "ds9/ds9parse.yy"
			{globalWidth = localWidth = (yysemantic_stack_[(3) - (3)].integer);}
			break;

			case 172:

				/* Line 663 of lalr1.cc  */
#line 671 "ds9/ds9parse.yy"
			{
				strncpy(globalFont,(yysemantic_stack_[(3) - (3)].str),32);
				strncpy(localFont,(yysemantic_stack_[(3) - (3)].str),32);
			}
			break;

			case 173:

				/* Line 663 of lalr1.cc  */
#line 676 "ds9/ds9parse.yy"
			{
				strncpy(globalText,(yysemantic_stack_[(3) - (3)].str),80);
				strncpy(localText,(yysemantic_stack_[(3) - (3)].str),80);
			}
			break;

			case 174:

				/* Line 663 of lalr1.cc  */
#line 681 "ds9/ds9parse.yy"
			{
				setProps(&globalProps,Marker::DASH,1);
				setProps(&localProps,Marker::DASH,1);
			}
			break;

			case 175:

				/* Line 663 of lalr1.cc  */
#line 686 "ds9/ds9parse.yy"
			{
				setProps(&globalProps,Marker::SOURCE,1);
				setProps(&localProps,Marker::SOURCE,1);
			}
			break;

			case 176:

				/* Line 663 of lalr1.cc  */
#line 691 "ds9/ds9parse.yy"
			{
				setProps(&globalProps,Marker::SOURCE,0);
				setProps(&localProps,Marker::SOURCE,0);
			}
			break;

			case 177:

				/* Line 663 of lalr1.cc  */
#line 695 "ds9/ds9parse.yy"
			{globalPoint = localPoint = (yysemantic_stack_[(3) - (3)].integer);}
			break;

			case 178:

				/* Line 663 of lalr1.cc  */
#line 697 "ds9/ds9parse.yy"
			{
				globalPoint = localPoint = (yysemantic_stack_[(4) - (3)].integer);
				globalPointSize = localPointSize = (yysemantic_stack_[(4) - (4)].integer);
			}
			break;

			case 179:

				/* Line 663 of lalr1.cc  */
#line 702 "ds9/ds9parse.yy"
			{
				globalLine1 = localLine1 = (yysemantic_stack_[(4) - (3)].integer);
				globalLine2 = localLine2 = (yysemantic_stack_[(4) - (4)].integer);
			}
			break;

			case 180:

				/* Line 663 of lalr1.cc  */
#line 706 "ds9/ds9parse.yy"
			{globalVector = localVector = (yysemantic_stack_[(3) - (3)].integer);}
			break;

			case 181:

				/* Line 663 of lalr1.cc  */
#line 708 "ds9/ds9parse.yy"
			{
				globalComposite = localComposite = (yysemantic_stack_[(3) - (3)].integer);
			}
			break;

			case 182:

				/* Line 663 of lalr1.cc  */
#line 711 "ds9/ds9parse.yy"
			{}
			break;

			case 183:

				/* Line 663 of lalr1.cc  */
#line 713 "ds9/ds9parse.yy"
			{
				strncpy(globalCompassNorth,(yysemantic_stack_[(7) - (4)].str),80);
				strncpy(globalCompassEast,(yysemantic_stack_[(7) - (5)].str),80);
				strncpy(localCompassNorth,(yysemantic_stack_[(7) - (4)].str),80);
				strncpy(localCompassEast,(yysemantic_stack_[(7) - (5)].str),80);
				globalCompassNArrow = localCompassNArrow = (yysemantic_stack_[(7) - (6)].integer);
				globalCompassEArrow = localCompassEArrow = (yysemantic_stack_[(7) - (7)].integer);
			}
			break;

			case 184:

				/* Line 663 of lalr1.cc  */
#line 721 "ds9/ds9parse.yy"
			{globalTextAngle = localTextAngle = (yysemantic_stack_[(3) - (3)].real);}
			break;

			case 185:

				/* Line 663 of lalr1.cc  */
#line 722 "ds9/ds9parse.yy"
			{globalTextRotate = localTextRotate = (yysemantic_stack_[(3) - (3)].integer);}
			break;

			case 186:

				/* Line 663 of lalr1.cc  */
#line 723 "ds9/ds9parse.yy"
			{globalWCS = (casa::viewer::CoordSystem)(yysemantic_stack_[(3) - (3)].integer);}
			break;

			case 187:

				/* Line 663 of lalr1.cc  */
#line 727 "ds9/ds9parse.yy"
			{
				globalRulerCoordSystem = localRulerCoordSystem = (casa::viewer::CoordSystem)(yysemantic_stack_[(4) - (1)].integer);
				globalRulerSkyFrame = localRulerSkyFrame = (casa::viewer::SkyFrame)(yysemantic_stack_[(4) - (2)].integer);
				globalRulerDistSystem = localRulerDistSystem = (casa::viewer::CoordSystem)(yysemantic_stack_[(4) - (3)].integer);
				globalRulerDistFormat = localRulerDistFormat = (SkyFormat)(yysemantic_stack_[(4) - (4)].integer);
			}
			break;

			case 188:

				/* Line 663 of lalr1.cc  */
#line 734 "ds9/ds9parse.yy"
			{
				globalRulerCoordSystem = localRulerCoordSystem = (casa::viewer::CoordSystem)(yysemantic_stack_[(2) - (1)].integer);
				globalRulerSkyFrame = localRulerSkyFrame = FK5;
				globalRulerDistSystem = localRulerDistSystem = (casa::viewer::CoordSystem)(yysemantic_stack_[(2) - (2)].integer);
				globalRulerDistFormat = localRulerDistFormat = DEGREES;
			}
			break;

			case 189:

				/* Line 663 of lalr1.cc  */
#line 741 "ds9/ds9parse.yy"
			{
				globalRulerCoordSystem = localRulerCoordSystem = (casa::viewer::CoordSystem)(yysemantic_stack_[(2) - (1)].integer);
				globalRulerSkyFrame = localRulerSkyFrame = FK5;
				globalRulerDistSystem = localRulerDistSystem = WCS;
				globalRulerDistFormat = localRulerDistFormat = (SkyFormat)(yysemantic_stack_[(2) - (2)].integer);
			}
			break;

			case 190:

				/* Line 663 of lalr1.cc  */
#line 748 "ds9/ds9parse.yy"
			{
				globalRulerCoordSystem = localRulerCoordSystem = WCS;
				globalRulerSkyFrame = localRulerSkyFrame = (casa::viewer::SkyFrame)(yysemantic_stack_[(2) - (1)].integer);
				globalRulerDistSystem = localRulerDistSystem = (casa::viewer::CoordSystem)(yysemantic_stack_[(2) - (2)].integer);
				globalRulerDistFormat = localRulerDistFormat = DEGREES;
			}
			break;

			case 191:

				/* Line 663 of lalr1.cc  */
#line 755 "ds9/ds9parse.yy"
			{
				globalRulerCoordSystem = localRulerCoordSystem = WCS;
				globalRulerSkyFrame = localRulerSkyFrame = (casa::viewer::SkyFrame)(yysemantic_stack_[(2) - (1)].integer);
				globalRulerDistSystem = localRulerDistSystem = WCS;
				globalRulerDistFormat = localRulerDistFormat = (SkyFormat)(yysemantic_stack_[(2) - (2)].integer);
			}
			break;

			case 192:

				/* Line 663 of lalr1.cc  */
#line 762 "ds9/ds9parse.yy"
			{
				globalRulerCoordSystem = localRulerCoordSystem = WCS;
				globalRulerSkyFrame = localRulerSkyFrame = FK5;
				globalRulerDistSystem = localRulerDistSystem = (casa::viewer::CoordSystem)(yysemantic_stack_[(2) - (2)].integer);
				globalRulerDistFormat = localRulerDistFormat = DEGREES;
			}
			break;

			case 193:

				/* Line 663 of lalr1.cc  */
#line 769 "ds9/ds9parse.yy"
			{
				globalRulerCoordSystem = localRulerCoordSystem = WCS;
				globalRulerSkyFrame = localRulerSkyFrame = FK5;
				globalRulerDistSystem = localRulerDistSystem = WCS;
				globalRulerDistFormat = localRulerDistFormat = (SkyFormat)(yysemantic_stack_[(2) - (2)].integer);
			}
			break;

			case 194:

				/* Line 663 of lalr1.cc  */
#line 776 "ds9/ds9parse.yy"
			{
				globalRulerCoordSystem = localRulerCoordSystem = IMAGE;
				globalRulerSkyFrame = localRulerSkyFrame = FK5;
				globalRulerDistSystem = localRulerDistSystem = WCS;
				globalRulerDistFormat = localRulerDistFormat = (SkyFormat)(yysemantic_stack_[(1) - (1)].integer);
			}
			break;

			case 195:

				/* Line 663 of lalr1.cc  */
#line 783 "ds9/ds9parse.yy"
			{
				globalRulerCoordSystem = localRulerCoordSystem = IMAGE;
				globalRulerSkyFrame = localRulerSkyFrame = FK5;
				globalRulerDistSystem = localRulerDistSystem = IMAGE;
				globalRulerDistFormat = localRulerDistFormat = DEGREES;
			}
			break;

			case 196:

				/* Line 663 of lalr1.cc  */
#line 792 "ds9/ds9parse.yy"
			{
				globalCompassCoordSystem = localCompassCoordSystem = (casa::viewer::CoordSystem)(yysemantic_stack_[(2) - (1)].integer);
				globalCompassSkyFrame = localCompassSkyFrame = (casa::viewer::SkyFrame)(yysemantic_stack_[(2) - (2)].integer);
			}
			break;

			case 197:

				/* Line 663 of lalr1.cc  */
#line 797 "ds9/ds9parse.yy"
			{
				globalCompassCoordSystem = localCompassCoordSystem = (casa::viewer::CoordSystem)(yysemantic_stack_[(1) - (1)].integer);
				globalCompassSkyFrame = localCompassSkyFrame = FK5;
			}
			break;

			case 198:

				/* Line 663 of lalr1.cc  */
#line 802 "ds9/ds9parse.yy"
			{
				globalCompassCoordSystem = localCompassCoordSystem = WCS;
				globalCompassSkyFrame = localCompassSkyFrame = (casa::viewer::SkyFrame)(yysemantic_stack_[(1) - (1)].integer);
			}
			break;

			case 199:

				/* Line 663 of lalr1.cc  */
#line 807 "ds9/ds9parse.yy"
			{
				globalCompassCoordSystem = localCompassCoordSystem = WCS;
				globalCompassSkyFrame = localCompassSkyFrame = FK5;
			}
			break;

			case 200:

				/* Line 663 of lalr1.cc  */
#line 813 "ds9/ds9parse.yy"
			{
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
			break;

			case 203:

				/* Line 663 of lalr1.cc  */
#line 859 "ds9/ds9parse.yy"
			{setProps(&localProps,(yysemantic_stack_[(3) - (1)].integer),(yysemantic_stack_[(3) - (3)].integer));}
			break;

			case 204:

				/* Line 663 of lalr1.cc  */
#line 860 "ds9/ds9parse.yy"
			{strncpy(localColor,(yysemantic_stack_[(3) - (3)].str),16);}
			break;

			case 205:

				/* Line 663 of lalr1.cc  */
#line 861 "ds9/ds9parse.yy"
			{
				strcpy(localColor,"#");
				strncat(localColor,(yysemantic_stack_[(4) - (4)].str),16);
			}
			break;

			case 206:

				/* Line 663 of lalr1.cc  */
#line 866 "ds9/ds9parse.yy"
			{
				localDash[0] =(yysemantic_stack_[(4) - (3)].integer);
				localDash[1] =(yysemantic_stack_[(4) - (4)].integer);
			}
			break;

			case 207:

				/* Line 663 of lalr1.cc  */
#line 870 "ds9/ds9parse.yy"
			{localWidth = (yysemantic_stack_[(3) - (3)].integer);}
			break;

			case 208:

				/* Line 663 of lalr1.cc  */
#line 871 "ds9/ds9parse.yy"
			{strncpy(localFont,(yysemantic_stack_[(3) - (3)].str),32);}
			break;

			case 209:

				/* Line 663 of lalr1.cc  */
#line 872 "ds9/ds9parse.yy"
			{strncpy(localText,(yysemantic_stack_[(3) - (3)].str),80);}
			break;

			case 210:

				/* Line 663 of lalr1.cc  */
#line 873 "ds9/ds9parse.yy"
			{taglist.push_back((yysemantic_stack_[(3) - (3)].str));}
			break;

			case 211:

				/* Line 663 of lalr1.cc  */
#line 874 "ds9/ds9parse.yy"
			{/*cblist.push_back(new CallBack(fr->getInterp(),(CallBack::Type)$3,$4,$5));*/}
			break;

			case 212:

				/* Line 663 of lalr1.cc  */
#line 875 "ds9/ds9parse.yy"
			{setProps(&localProps,Marker::DASH,1);}
			break;

			case 213:

				/* Line 663 of lalr1.cc  */
#line 876 "ds9/ds9parse.yy"
			{setProps(&localProps,Marker::SOURCE,1);}
			break;

			case 214:

				/* Line 663 of lalr1.cc  */
#line 877 "ds9/ds9parse.yy"
			{setProps(&localProps,Marker::SOURCE,0);}
			break;

			case 215:

				/* Line 663 of lalr1.cc  */
#line 879 "ds9/ds9parse.yy"
			{localPoint = (yysemantic_stack_[(3) - (3)].integer);}
			break;

			case 216:

				/* Line 663 of lalr1.cc  */
#line 880 "ds9/ds9parse.yy"
			{localPoint = (yysemantic_stack_[(4) - (3)].integer); localPointSize = (yysemantic_stack_[(4) - (4)].integer);}
			break;

			case 217:

				/* Line 663 of lalr1.cc  */
#line 881 "ds9/ds9parse.yy"
			{localLine1=(yysemantic_stack_[(4) - (3)].integer); localLine2=(yysemantic_stack_[(4) - (4)].integer);}
			break;

			case 218:

				/* Line 663 of lalr1.cc  */
#line 882 "ds9/ds9parse.yy"
			{localVector=(yysemantic_stack_[(3) - (3)].integer);}
			break;

			case 219:

				/* Line 663 of lalr1.cc  */
#line 883 "ds9/ds9parse.yy"
			{localComposite=(yysemantic_stack_[(3) - (3)].integer);}
			break;

			case 221:

				/* Line 663 of lalr1.cc  */
#line 886 "ds9/ds9parse.yy"
			{
				strncpy(localCompassNorth,(yysemantic_stack_[(7) - (4)].str),80);
				strncpy(localCompassEast,(yysemantic_stack_[(7) - (5)].str),80);
				localCompassNArrow = (yysemantic_stack_[(7) - (6)].integer);
				localCompassEArrow = (yysemantic_stack_[(7) - (7)].integer);
			}
			break;

			case 222:

				/* Line 663 of lalr1.cc  */
#line 892 "ds9/ds9parse.yy"
			{localTextAngle=(yysemantic_stack_[(3) - (3)].real);}
			break;

			case 223:

				/* Line 663 of lalr1.cc  */
#line 893 "ds9/ds9parse.yy"
			{localTextRotate=(yysemantic_stack_[(3) - (3)].integer);}
			break;

			case 227:

				/* Line 663 of lalr1.cc  */
#line 900 "ds9/ds9parse.yy"
			{
				localRulerCoordSystem = (casa::viewer::CoordSystem)(yysemantic_stack_[(4) - (1)].integer);
				localRulerSkyFrame = (casa::viewer::SkyFrame)(yysemantic_stack_[(4) - (2)].integer);
				localRulerDistSystem = (casa::viewer::CoordSystem)(yysemantic_stack_[(4) - (3)].integer);
				localRulerDistFormat = (SkyFormat)(yysemantic_stack_[(4) - (4)].integer);
			}
			break;

			case 228:

				/* Line 663 of lalr1.cc  */
#line 907 "ds9/ds9parse.yy"
			{
				localRulerCoordSystem = (casa::viewer::CoordSystem)(yysemantic_stack_[(2) - (1)].integer);
				localRulerSkyFrame = FK5;
				localRulerDistSystem = (casa::viewer::CoordSystem)(yysemantic_stack_[(2) - (2)].integer);
				localRulerDistFormat = DEGREES;
			}
			break;

			case 229:

				/* Line 663 of lalr1.cc  */
#line 914 "ds9/ds9parse.yy"
			{
				localRulerCoordSystem = (casa::viewer::CoordSystem)(yysemantic_stack_[(2) - (1)].integer);
				localRulerSkyFrame = FK5;
				localRulerDistSystem = WCS;
				localRulerDistFormat = (SkyFormat)(yysemantic_stack_[(2) - (2)].integer);
			}
			break;

			case 230:

				/* Line 663 of lalr1.cc  */
#line 921 "ds9/ds9parse.yy"
			{
				localRulerCoordSystem = WCS;
				localRulerSkyFrame = (casa::viewer::SkyFrame)(yysemantic_stack_[(2) - (1)].integer);
				localRulerDistSystem = (casa::viewer::CoordSystem)(yysemantic_stack_[(2) - (2)].integer);
				localRulerDistFormat = DEGREES;
			}
			break;

			case 231:

				/* Line 663 of lalr1.cc  */
#line 928 "ds9/ds9parse.yy"
			{
				localRulerCoordSystem = WCS;
				localRulerSkyFrame = (casa::viewer::SkyFrame)(yysemantic_stack_[(2) - (1)].integer);
				localRulerDistSystem = WCS;
				localRulerDistFormat = (SkyFormat)(yysemantic_stack_[(2) - (2)].integer);
			}
			break;

			case 232:

				/* Line 663 of lalr1.cc  */
#line 935 "ds9/ds9parse.yy"
			{
				localRulerCoordSystem = WCS;
				localRulerSkyFrame = FK5;
				localRulerDistSystem = (casa::viewer::CoordSystem)(yysemantic_stack_[(2) - (2)].integer);
				localRulerDistFormat = DEGREES;
			}
			break;

			case 233:

				/* Line 663 of lalr1.cc  */
#line 942 "ds9/ds9parse.yy"
			{
				localRulerCoordSystem = WCS;
				localRulerSkyFrame = FK5;
				localRulerDistSystem = WCS;
				localRulerDistFormat = (SkyFormat)(yysemantic_stack_[(2) - (2)].integer);
			}
			break;

			case 234:

				/* Line 663 of lalr1.cc  */
#line 949 "ds9/ds9parse.yy"
			{
				localRulerCoordSystem = IMAGE;
				localRulerSkyFrame = FK5;
				localRulerDistSystem = WCS;
				localRulerDistFormat = (SkyFormat)(yysemantic_stack_[(1) - (1)].integer);
			}
			break;

			case 235:

				/* Line 663 of lalr1.cc  */
#line 956 "ds9/ds9parse.yy"
			{
				localRulerCoordSystem = IMAGE;
				localRulerSkyFrame = FK5;
				localRulerDistSystem = IMAGE;
				localRulerDistFormat = DEGREES;
			}
			break;

			case 236:

				/* Line 663 of lalr1.cc  */
#line 965 "ds9/ds9parse.yy"
			{
				localCompassCoordSystem = (casa::viewer::CoordSystem)(yysemantic_stack_[(2) - (1)].integer);
				localCompassSkyFrame = (casa::viewer::SkyFrame)(yysemantic_stack_[(2) - (2)].integer);
			}
			break;

			case 237:

				/* Line 663 of lalr1.cc  */
#line 970 "ds9/ds9parse.yy"
			{
				localCompassCoordSystem = (casa::viewer::CoordSystem)(yysemantic_stack_[(1) - (1)].integer);
				localCompassSkyFrame = FK5;
			}
			break;

			case 238:

				/* Line 663 of lalr1.cc  */
#line 975 "ds9/ds9parse.yy"
			{
				localCompassCoordSystem = WCS;
				localCompassSkyFrame = (casa::viewer::SkyFrame)(yysemantic_stack_[(1) - (1)].integer);
			}
			break;

			case 239:

				/* Line 663 of lalr1.cc  */
#line 980 "ds9/ds9parse.yy"
			{
				localCompassCoordSystem = WCS;
				localCompassSkyFrame = FK5;
			}
			break;

			case 240:

				/* Line 663 of lalr1.cc  */
#line 986 "ds9/ds9parse.yy"
			{aNum=0; aAngNum=0;}
			break;

			case 241:

				/* Line 663 of lalr1.cc  */
#line 986 "ds9/ds9parse.yy"
			{localCpanda = 2;}
			break;

			case 242:

				/* Line 663 of lalr1.cc  */
#line 987 "ds9/ds9parse.yy"
			{localCpanda=0;}
			break;

			case 243:

				/* Line 663 of lalr1.cc  */
#line 990 "ds9/ds9parse.yy"
			{aNum=0; aAngNum=0, aAngle=0;}
			break;

			case 244:

				/* Line 663 of lalr1.cc  */
#line 991 "ds9/ds9parse.yy"
			{aAngle=(yysemantic_stack_[(10) - (9)].real); localEpanda=2;}
			break;

			case 245:

				/* Line 663 of lalr1.cc  */
#line 992 "ds9/ds9parse.yy"
			{localEpanda=0;}
			break;

			case 246:

				/* Line 663 of lalr1.cc  */
#line 995 "ds9/ds9parse.yy"
			{aNum=0; aAngNum=0, aAngle=0;}
			break;

			case 247:

				/* Line 663 of lalr1.cc  */
#line 996 "ds9/ds9parse.yy"
			{aAngle=(yysemantic_stack_[(10) - (9)].real); localBpanda=2;}
			break;

			case 248:

				/* Line 663 of lalr1.cc  */
#line 997 "ds9/ds9parse.yy"
			{localBpanda=0;}
			break;

			case 249:

				/* Line 663 of lalr1.cc  */
#line 1000 "ds9/ds9parse.yy"
			{
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
			break;

			case 250:

				/* Line 663 of lalr1.cc  */
#line 1045 "ds9/ds9parse.yy"
			{(yyval.integer) = CIRCLE;}
			break;

			case 251:

				/* Line 663 of lalr1.cc  */
#line 1046 "ds9/ds9parse.yy"
			{(yyval.integer) = BOX;}
			break;

			case 252:

				/* Line 663 of lalr1.cc  */
#line 1047 "ds9/ds9parse.yy"
			{(yyval.integer) = DIAMOND;}
			break;

			case 253:

				/* Line 663 of lalr1.cc  */
#line 1048 "ds9/ds9parse.yy"
			{(yyval.integer) = CROSS;}
			break;

			case 254:

				/* Line 663 of lalr1.cc  */
#line 1049 "ds9/ds9parse.yy"
			{(yyval.integer) = XPT;}
			break;

			case 255:

				/* Line 663 of lalr1.cc  */
#line 1050 "ds9/ds9parse.yy"
			{(yyval.integer) = ARROW;}
			break;

			case 256:

				/* Line 663 of lalr1.cc  */
#line 1051 "ds9/ds9parse.yy"
			{(yyval.integer) = BOXCIRCLE;}
			break;

			case 257:

				/* Line 663 of lalr1.cc  */
#line 1054 "ds9/ds9parse.yy"
			{setProps(&localProps, Marker::INCLUDE, 1);}
			break;

			case 258:

				/* Line 663 of lalr1.cc  */
#line 1055 "ds9/ds9parse.yy"
			{setProps(&localProps, Marker::INCLUDE, 0);}
			break;

			case 259:

				/* Line 663 of lalr1.cc  */
#line 1059 "ds9/ds9parse.yy"
			{
				fr->createTextCmd(coordtovec((yysemantic_stack_[(6) - (3)].vector)),
				localTextAngle,localTextRotate,
				localColor,localDash,localWidth,localFont,
				localText,localProps,localComment,taglist /*,cblist*/);
			}
			break;

			case 260:

				/* Line 663 of lalr1.cc  */
#line 1064 "ds9/ds9parse.yy"
			{
				fr->createCompositeCmd(coordtovec((yysemantic_stack_[(8) - (3)].vector)),
				(yysemantic_stack_[(8) - (5)].real), localComposite,
				localColor,localDash,localWidth,localFont,
				localText,localProps,localComment,taglist /*,cblist*/);
			}
			break;

			case 261:

				/* Line 663 of lalr1.cc  */
#line 1069 "ds9/ds9parse.yy"
			{
				fr->createVectCmd(coordtovec((yysemantic_stack_[(10) - (3)].vector)),
				(yysemantic_stack_[(10) - (5)].real),(yysemantic_stack_[(10) - (7)].real),
				localVector,
				localColor,localDash,localWidth,localFont,
				localText,localProps,localComment,taglist /*,cblist*/);
			}
			break;

			case 262:

				/* Line 663 of lalr1.cc  */
#line 1075 "ds9/ds9parse.yy"
			{
				// hard coded into projection.tcl
				fr->createProjectionCmd(coordtovec((yysemantic_stack_[(10) - (3)].vector)),
				coordtovec((yysemantic_stack_[(10) - (5)].vector)),
				(yysemantic_stack_[(10) - (7)].real),
				"ProjectionPlotCB", "ProjectionPlotDeleteCB",
				localColor,localDash,localWidth,localFont,
				localText,localProps,localComment,taglist /*,cblist*/);
			}
			break;

			case 263:

				/* Line 663 of lalr1.cc  */
#line 1085 "ds9/ds9parse.yy"
			{
				fr->createRulerCmd(coordtovec((yysemantic_stack_[(8) - (3)].vector)),
				coordtovec((yysemantic_stack_[(8) - (5)].vector)),
				localRulerCoordSystem, localRulerSkyFrame,
				localRulerDistSystem, localRulerDistFormat,
				localColor,localDash,localWidth,localFont,
				localText,localProps,localComment,taglist /*,cblist*/);
			}
			break;

			case 264:

				/* Line 663 of lalr1.cc  */
#line 1092 "ds9/ds9parse.yy"
			{
				fr->createCompassCmd(coordtovec((yysemantic_stack_[(8) - (3)].vector)),
				(yysemantic_stack_[(8) - (5)].real),
				localCompassNorth, localCompassEast,
				localCompassNArrow, localCompassEArrow,
				localCompassCoordSystem, localCompassSkyFrame,
				localColor,localDash,localWidth,localFont,
				localText,localProps,localComment,taglist /*,cblist*/);
			}
			break;

			case 265:

				/* Line 663 of lalr1.cc  */
#line 1100 "ds9/ds9parse.yy"
			{
				// hard coded into projection.tcl
				fr->createCircle3dCmd(coordtovec((yysemantic_stack_[(8) - (3)].vector)),
				(yysemantic_stack_[(8) - (5)].real),
				"Circle3dPlotCB", "Circle3dPlotDeleteCB",
				localColor,localDash,localWidth,localFont,
				localText,localProps,localComment,taglist /*,cblist*/);
			}
			break;

			case 266:

				/* Line 663 of lalr1.cc  */
#line 1111 "ds9/ds9parse.yy"
			{
				fr->createCircleCmd(coordtovec((yysemantic_stack_[(8) - (3)].vector)),
				(yysemantic_stack_[(8) - (5)].real),
				localColor,localDash,localWidth,localFont,
				localText,localProps,localComment,taglist /*,cblist*/);
			}
			break;

			case 267:

				/* Line 663 of lalr1.cc  */
#line 1116 "ds9/ds9parse.yy"
			{
				fr->createAnnulusCmd(coordtovec((yysemantic_stack_[(10) - (3)].vector)),
				(yysemantic_stack_[(10) - (5)].real),(yysemantic_stack_[(10) - (7)].real),1,
				localColor,localDash,localWidth,localFont,
				localText,localProps,localComment,taglist /*,cblist*/);
			}
			break;

			case 268:

				/* Line 663 of lalr1.cc  */
#line 1122 "ds9/ds9parse.yy"
			{
				aAnnuli[0] = (yysemantic_stack_[(12) - (5)].real);
				aAnnuli[1] = (yysemantic_stack_[(12) - (7)].real);
				fr->createAnnulusCmd(coordtovec((yysemantic_stack_[(12) - (3)].vector)),
				aNum,aAnnuli,
				localColor,localDash,localWidth,localFont,
				localText,localProps,localComment,taglist /*,cblist*/);
			}
			break;

			case 269:

				/* Line 663 of lalr1.cc  */
#line 1132 "ds9/ds9parse.yy"
			{
				fr->createAnnulusCmd(coordtovec((yysemantic_stack_[(12) - (3)].vector)),
				(yysemantic_stack_[(12) - (5)].real),(yysemantic_stack_[(12) - (7)].real),(yysemantic_stack_[(12) - (9)].integer),
				localColor,localDash,localWidth,localFont,
				localText,localProps,localComment,taglist /*,cblist*/);
			}
			break;

			case 270:

				/* Line 663 of lalr1.cc  */
#line 1138 "ds9/ds9parse.yy"
			{
				switch (localCpanda) {
				case 0: /* ignore it */
					break;
				case 1: /* normal cpanda */
					fr->createCpandaCmd(coordtovec((yysemantic_stack_[(18) - (3)].vector)),
					(yysemantic_stack_[(18) - (5)].real),(yysemantic_stack_[(18) - (7)].real),(yysemantic_stack_[(18) - (9)].integer),
					(yysemantic_stack_[(18) - (11)].real),(yysemantic_stack_[(18) - (13)].real),(yysemantic_stack_[(18) - (15)].integer),
					localColor,localDash,localWidth,localFont,
					localText,localProps,localComment,taglist /*,cblist*/);
					break;
				case 2: /* one of our special pandas */
					fr->createCpandaCmd(coordtovec((yysemantic_stack_[(18) - (3)].vector)),
					aAngNum,aAngles,
					aNum,aAnnuli,
					localColor,localDash,localWidth,localFont,
					localText,localProps,localComment,taglist /*,cblist*/);
					break;
				}
			}
			break;

			case 271:

				/* Line 663 of lalr1.cc  */
#line 1160 "ds9/ds9parse.yy"
			{
				// for ellipse annulus
				aStatus = 1;
				aCenter = coordtovec((yysemantic_stack_[(10) - (3)].vector));
				aAngles[0] = (yysemantic_stack_[(10) - (7)].real);
				aVector[0] = coordtovec((yysemantic_stack_[(10) - (5)].vector));
				aNumsao = 1;
				strncpy(aColor,localColor,16);
				aDash[0] = localDash[0];
				aDash[1] = localDash[1];
				aWidth = localWidth;
				strncpy(aFont,localFont,32);
				strncpy(aText,localText,80);
				strncpy(aComment,localComment,80);
				aProps = localProps;

				fr->createEllipseCmd(coordtovec((yysemantic_stack_[(10) - (3)].vector)),
				coordtovec((yysemantic_stack_[(10) - (5)].vector)),
				(yysemantic_stack_[(10) - (7)].real),
				localColor,localDash,localWidth,localFont,
				localText,localProps,localComment,taglist /*,cblist*/);
			}
			break;

			case 272:

				/* Line 663 of lalr1.cc  */
#line 1184 "ds9/ds9parse.yy"
			{
				// prefered syntax
				fr->createEllipseAnnulusCmd(coordtovec((yysemantic_stack_[(12) - (3)].vector)),
				coordtovec((yysemantic_stack_[(12) - (5)].vector)),coordtovec((yysemantic_stack_[(12) - (7)].vector)),1,
				(yysemantic_stack_[(12) - (9)].real),
				localColor,localDash,localWidth,localFont,
				localText,localProps,localComment,taglist /*,cblist*/);
			}
			break;

			case 273:

				/* Line 663 of lalr1.cc  */
#line 1194 "ds9/ds9parse.yy"
			{
				// prefered syntax
				fr->createEllipseAnnulusCmd(coordtovec((yysemantic_stack_[(14) - (3)].vector)),
				coordtovec((yysemantic_stack_[(14) - (5)].vector)),coordtovec((yysemantic_stack_[(14) - (7)].vector)),(yysemantic_stack_[(14) - (9)].integer),
				(yysemantic_stack_[(14) - (11)].real),
				localColor,localDash,localWidth,localFont,
				localText,localProps,localComment,taglist /*,cblist*/);
			}
			break;

			case 274:

				/* Line 663 of lalr1.cc  */
#line 1204 "ds9/ds9parse.yy"
			{
				// prefered syntax
				aVector[0] = coordtovec((yysemantic_stack_[(14) - (5)].vector));
				aVector[1] = coordtovec((yysemantic_stack_[(14) - (7)].vector));
				fr->createEllipseAnnulusCmd(coordtovec((yysemantic_stack_[(14) - (3)].vector)),
				aNum,aVector,
				(yysemantic_stack_[(14) - (11)].real),
				localColor,localDash,localWidth,localFont,
				localText,localProps,localComment,taglist /*,cblist*/);
			}
			break;

			case 275:

				/* Line 663 of lalr1.cc  */
#line 1217 "ds9/ds9parse.yy"
			{
				// old saoimage syntax
				aStatus = 2;
				aVector[aNumsao++] = coordtovec((yysemantic_stack_[(18) - (5)].vector));
			}
			break;

			case 276:

				/* Line 663 of lalr1.cc  */
#line 1224 "ds9/ds9parse.yy"
			{
				switch (localEpanda) {
				case 0: /* ignore it */
					break;
				case 1: /* normal epanda */
					fr->createEpandaCmd(coordtovec((yysemantic_stack_[(20) - (3)].vector)),
					(yysemantic_stack_[(20) - (5)].real),(yysemantic_stack_[(20) - (7)].real),(yysemantic_stack_[(20) - (9)].integer),
					coordtovec((yysemantic_stack_[(20) - (11)].vector)),coordtovec((yysemantic_stack_[(20) - (13)].vector)),(yysemantic_stack_[(20) - (15)].integer),
					(yysemantic_stack_[(20) - (17)].real),
					localColor,localDash,localWidth,localFont,
					localText,localProps,localComment,taglist /*,cblist*/);
					break;
				case 2: /* one of our special pandas */
					fr->createEpandaCmd(coordtovec((yysemantic_stack_[(20) - (3)].vector)),
					aAngNum,aAngles,
					aNum,aVector,
					aAngle,
					localColor,localDash,localWidth,localFont,
					localText,localProps,localComment,taglist /*,cblist*/);
					break;
				}
			}
			break;

			case 277:

				/* Line 663 of lalr1.cc  */
#line 1248 "ds9/ds9parse.yy"
			{
				// for box annulus
				aStatus = 3;
				aCenter = coordtovec((yysemantic_stack_[(10) - (3)].vector));
				aAngles[0] = (yysemantic_stack_[(10) - (7)].real);
				aVector[0] = coordtovec((yysemantic_stack_[(10) - (5)].vector));
				aNumsao = 1;
				strncpy(aColor,localColor,16);
				aDash[0] = localDash[0];
				aDash[1] = localDash[1];
				aWidth = localWidth;
				strncpy(aFont,localFont,32);
				strncpy(aText,localText,80);
				strncpy(aComment,localComment,80);
				aProps = localProps;

				fr->createBoxCmd(coordtovec((yysemantic_stack_[(10) - (3)].vector)),
				coordtovec((yysemantic_stack_[(10) - (5)].vector)),
				(yysemantic_stack_[(10) - (7)].real),
				localColor,localDash,localWidth,localFont,
				localText,localProps,localComment,taglist /*,cblist*/);
			}
			break;

			case 278:

				/* Line 663 of lalr1.cc  */
#line 1271 "ds9/ds9parse.yy"
			{
				fr->createBoxCmd(coordtovec((yysemantic_stack_[(10) - (3)].vector)),
				coordtovec((yysemantic_stack_[(10) - (5)].vector)),
				(yysemantic_stack_[(10) - (7)].real),
				localColor,localDash,localWidth,localFont,
				localText,localProps,localComment,taglist /*,cblist*/);
			}
			break;

			case 279:

				/* Line 663 of lalr1.cc  */
#line 1278 "ds9/ds9parse.yy"
			{
				// prefered syntax
				fr->createBoxAnnulusCmd(coordtovec((yysemantic_stack_[(12) - (3)].vector)),
				coordtovec((yysemantic_stack_[(12) - (5)].vector)),coordtovec((yysemantic_stack_[(12) - (7)].vector)),1,
				(yysemantic_stack_[(12) - (9)].real),
				localColor,localDash,localWidth,localFont,
				localText,localProps,localComment,taglist /*,cblist*/);
			}
			break;

			case 280:

				/* Line 663 of lalr1.cc  */
#line 1288 "ds9/ds9parse.yy"
			{
				// prefered syntax
				aVector[0] = coordtovec((yysemantic_stack_[(14) - (5)].vector));
				aVector[1] = coordtovec((yysemantic_stack_[(14) - (7)].vector));
				fr->createBoxAnnulusCmd(coordtovec((yysemantic_stack_[(14) - (3)].vector)),
				aNum,aVector,
				(yysemantic_stack_[(14) - (11)].real),
				localColor,localDash,localWidth,localFont,
				localText,localProps,localComment,taglist /*,cblist*/);
			}
			break;

			case 281:

				/* Line 663 of lalr1.cc  */
#line 1300 "ds9/ds9parse.yy"
			{
				// prefered syntax
				fr->createBoxAnnulusCmd(coordtovec((yysemantic_stack_[(14) - (3)].vector)),
				coordtovec((yysemantic_stack_[(14) - (5)].vector)),coordtovec((yysemantic_stack_[(14) - (7)].vector)),(yysemantic_stack_[(14) - (9)].integer),
				(yysemantic_stack_[(14) - (11)].real),
				localColor,localDash,localWidth,localFont,
				localText,localProps,localComment,taglist /*,cblist*/);
			}
			break;

			case 282:

				/* Line 663 of lalr1.cc  */
#line 1311 "ds9/ds9parse.yy"
			{
				// old saoimage syntax
				aStatus = 4;
				aVector[aNumsao++] = coordtovec((yysemantic_stack_[(18) - (5)].vector));
			}
			break;

			case 283:

				/* Line 663 of lalr1.cc  */
#line 1318 "ds9/ds9parse.yy"
			{
				switch (localBpanda) {
				case 0: /* ignore it */
					break;
				case 1: /* normal bpanda */
					fr->createBpandaCmd(coordtovec((yysemantic_stack_[(20) - (3)].vector)),
					(yysemantic_stack_[(20) - (5)].real),(yysemantic_stack_[(20) - (7)].real),(yysemantic_stack_[(20) - (9)].integer),
					coordtovec((yysemantic_stack_[(20) - (11)].vector)),coordtovec((yysemantic_stack_[(20) - (13)].vector)),(yysemantic_stack_[(20) - (15)].integer),
					(yysemantic_stack_[(20) - (17)].real),
					localColor,localDash,localWidth,localFont,
					localText,localProps,localComment,taglist /*,cblist*/);
					break;
				case 2: /* one of our special pandas */
					fr->createBpandaCmd(coordtovec((yysemantic_stack_[(20) - (3)].vector)),
					aAngNum,aAngles,
					aNum,aVector,
					aAngle,
					localColor,localDash,localWidth,localFont,
					localText,localProps,localComment,taglist /*,cblist*/);
					break;
				}
			}
			break;

			case 284:

				/* Line 663 of lalr1.cc  */
#line 1342 "ds9/ds9parse.yy"
			{
				fr->createLineCmd(coordtovec((yysemantic_stack_[(8) - (3)].vector)),
				coordtovec((yysemantic_stack_[(8) - (5)].vector)),
				localLine1,localLine2,
				localColor,localDash,localWidth,localFont,
				localText,localProps,localComment,taglist /*,cblist*/);
			}
			break;

			case 285:

				/* Line 663 of lalr1.cc  */
#line 1349 "ds9/ds9parse.yy"
			{
				switch (localPoint) {
				case CIRCLE:
					fr->createCirclePointCmd(coordtovec((yysemantic_stack_[(6) - (3)].vector)), localPointSize,
					localColor,localDash,localWidth,localFont,
					localText,localProps,localComment,taglist /*,cblist*/);
					break;
				case BOX:
					fr->createBoxPointCmd(coordtovec((yysemantic_stack_[(6) - (3)].vector)), localPointSize,
					localColor,localDash,localWidth,localFont,
					localText,localProps,localComment,taglist /*,cblist*/);
					break;
				case DIAMOND:
					fr->createDiamondPointCmd(coordtovec((yysemantic_stack_[(6) - (3)].vector)), localPointSize,
					localColor,localDash,localWidth,localFont,
					localText,localProps,localComment,taglist /*,cblist*/);
					break;
				case CROSS:
					fr->createCrossPointCmd(coordtovec((yysemantic_stack_[(6) - (3)].vector)), localPointSize,
					localColor,localDash,localWidth,localFont,
					localText,localProps,localComment,taglist /*,cblist*/);
					break;
				case XPT:
					fr->createExPointCmd(coordtovec((yysemantic_stack_[(6) - (3)].vector)), localPointSize,
					localColor,localDash,localWidth,localFont,
					localText,localProps,localComment,taglist /*,cblist*/);
					break;
				case ARROW:
					fr->createArrowPointCmd(coordtovec((yysemantic_stack_[(6) - (3)].vector)), localPointSize,
					localColor,localDash,localWidth,localFont,
					localText,localProps,localComment,taglist /*,cblist*/);
					break;
				case BOXCIRCLE:
					fr->createBoxCirclePointCmd(coordtovec((yysemantic_stack_[(6) - (3)].vector)), localPointSize,
					localColor,localDash,localWidth,localFont,
					localText,localProps,localComment,taglist /*,cblist*/);
					break;
				}
			}
			break;

			case 286:

				/* Line 663 of lalr1.cc  */
#line 1389 "ds9/ds9parse.yy"
			{
				fr->createCirclePointCmd(coordtovec((yysemantic_stack_[(7) - (4)].vector)), localPointSize,
				localColor,localDash,localWidth,localFont,
				localText,localProps,localComment,taglist /*,cblist*/);
			}
			break;

			case 287:

				/* Line 663 of lalr1.cc  */
#line 1393 "ds9/ds9parse.yy"
			{
				fr->createBoxPointCmd(coordtovec((yysemantic_stack_[(7) - (4)].vector)), localPointSize,
				localColor,localDash,localWidth,localFont,
				localText,localProps,localComment,taglist /*,cblist*/);
			}
			break;

			case 288:

				/* Line 663 of lalr1.cc  */
#line 1397 "ds9/ds9parse.yy"
			{
				fr->createDiamondPointCmd(coordtovec((yysemantic_stack_[(7) - (4)].vector)), localPointSize,
				localColor,localDash,localWidth,localFont,
				localText,localProps,localComment,taglist /*,cblist*/);
			}
			break;

			case 289:

				/* Line 663 of lalr1.cc  */
#line 1401 "ds9/ds9parse.yy"
			{
				fr->createCrossPointCmd(coordtovec((yysemantic_stack_[(7) - (4)].vector)), localPointSize,
				localColor,localDash,localWidth,localFont,
				localText,localProps,localComment,taglist /*,cblist*/);
			}
			break;

			case 290:

				/* Line 663 of lalr1.cc  */
#line 1405 "ds9/ds9parse.yy"
			{
				fr->createExPointCmd(coordtovec((yysemantic_stack_[(7) - (4)].vector)), localPointSize,
				localColor,localDash,localWidth,localFont,
				localText,localProps,localComment,taglist /*,cblist*/);
			}
			break;

			case 291:

				/* Line 663 of lalr1.cc  */
#line 1409 "ds9/ds9parse.yy"
			{
				fr->createArrowPointCmd(coordtovec((yysemantic_stack_[(7) - (4)].vector)), localPointSize,
				localColor,localDash,localWidth,localFont,
				localText,localProps,localComment,taglist /*,cblist*/);
			}
			break;

			case 292:

				/* Line 663 of lalr1.cc  */
#line 1413 "ds9/ds9parse.yy"
			{
				fr->createBoxCirclePointCmd(coordtovec((yysemantic_stack_[(7) - (4)].vector)), localPointSize,
				localColor,localDash,localWidth,localFont,
				localText,localProps,localComment,taglist /*,cblist*/);
			}
			break;

			case 293:

				/* Line 663 of lalr1.cc  */
#line 1417 "ds9/ds9parse.yy"
			{polylist.clear();}
			break;

			case 294:

				/* Line 663 of lalr1.cc  */
#line 1419 "ds9/ds9parse.yy"
			{
				fr->createPolygonCmd(polylist,
				localColor,localDash,localWidth,localFont,
				localText,localProps,localComment,taglist /*,cblist*/);
			}
			break;

			case 299:

				/* Line 663 of lalr1.cc  */
#line 1430 "ds9/ds9parse.yy"
			{
				fr->createTextCmd(coordtovec((yysemantic_stack_[(6) - (3)].vector)),
				localTextAngle,localTextRotate,
				localColor,localDash,localWidth,localFont,
				localText,localProps,localComment,taglist /*,cblist*/);
			}
			break;

			case 300:

				/* Line 663 of lalr1.cc  */
#line 1434 "ds9/ds9parse.yy"
			{strncpy(localText,(yysemantic_stack_[(6) - (5)].str),80);}
			break;

			case 301:

				/* Line 663 of lalr1.cc  */
#line 1436 "ds9/ds9parse.yy"
			{
				fr->createTextCmd(coordtovec((yysemantic_stack_[(9) - (3)].vector)),
				localTextAngle,localTextRotate,
				localColor,localDash,localWidth,localFont,
				localText,localProps,localComment,taglist /*,cblist*/);
			}
			break;

			case 302:

				/* Line 663 of lalr1.cc  */
#line 1441 "ds9/ds9parse.yy"
			{
				fr->createCompositeCmd(coordtovec((yysemantic_stack_[(8) - (3)].vector)),
				(yysemantic_stack_[(8) - (5)].real), localComposite,
				localColor,localDash,localWidth,localFont,
				localText,localProps,localComment,taglist /*,cblist*/);
			}
			break;

			case 303:

				/* Line 663 of lalr1.cc  */
#line 1446 "ds9/ds9parse.yy"
			{
				fr->createVectCmd(coordtovec((yysemantic_stack_[(10) - (3)].vector)),
				(yysemantic_stack_[(10) - (5)].real),(yysemantic_stack_[(10) - (7)].real),
				localVector,
				localColor,localDash,localWidth,localFont,
				localText,localProps,localComment,taglist /*,cblist*/);
			}
			break;

			case 304:

				/* Line 663 of lalr1.cc  */
#line 1452 "ds9/ds9parse.yy"
			{
				// hard coded into projection.tcl
				fr->createProjectionCmd(coordtovec((yysemantic_stack_[(10) - (3)].vector)),
				coordtovec((yysemantic_stack_[(10) - (5)].vector)),
				(yysemantic_stack_[(10) - (7)].real),
				"ProjectionPlotCB", "ProjectionPlotDeleteCB",
				localColor,localDash,localWidth,localFont,
				localText,localProps,localComment,taglist /*,cblist*/);
			}
			break;

			case 305:

				/* Line 663 of lalr1.cc  */
#line 1462 "ds9/ds9parse.yy"
			{
				fr->createRulerCmd(coordtovec((yysemantic_stack_[(8) - (3)].vector)),
				coordtovec((yysemantic_stack_[(8) - (5)].vector)),
				localRulerCoordSystem, localRulerSkyFrame,
				localRulerDistSystem, localRulerDistFormat,
				localColor,localDash,localWidth,localFont,
				localText,localProps,localComment,taglist /*,cblist*/);
			}
			break;

			case 306:

				/* Line 663 of lalr1.cc  */
#line 1469 "ds9/ds9parse.yy"
			{
				fr->createCompassCmd(coordtovec((yysemantic_stack_[(8) - (3)].vector)),
				(yysemantic_stack_[(8) - (5)].real),
				localCompassNorth, localCompassEast,
				localCompassNArrow, localCompassEArrow,
				localCompassCoordSystem, localCompassSkyFrame,
				localColor,localDash,localWidth,localFont,
				localText,localProps,localComment,taglist /*,cblist*/);
			}
			break;

			case 307:

				/* Line 663 of lalr1.cc  */
#line 1477 "ds9/ds9parse.yy"
			{
				// hard coded into circle3d.tcl
				fr->createCircle3dCmd(coordtovec((yysemantic_stack_[(8) - (3)].vector)),
				(yysemantic_stack_[(8) - (5)].real),
				"Circle3dPlotCB", "Circle3dPlotDeleteCB",
				localColor,localDash,localWidth,localFont,
				localText,localProps,localComment,taglist /*,cblist*/);
			}
			break;

			case 310:

				/* Line 663 of lalr1.cc  */
#line 1491 "ds9/ds9parse.yy"
			{polylist.push_back(coordtovec((yysemantic_stack_[(1) - (1)].vector)));}
			break;

			case 313:

				/* Line 663 of lalr1.cc  */
#line 1499 "ds9/ds9parse.yy"
			{
				if (aNum < MAXANNULI)
					aAnnuli[aNum++] = (yysemantic_stack_[(1) - (1)].real);
			}
			break;

			case 316:

				/* Line 663 of lalr1.cc  */
#line 1510 "ds9/ds9parse.yy"
			{
				if (aAngNum < MAXANGLES)
					aAngles[aAngNum++] = (yysemantic_stack_[(1) - (1)].real);
			}
			break;

			case 319:

				/* Line 663 of lalr1.cc  */
#line 1520 "ds9/ds9parse.yy"
			{aVector[aNum++] = doubletovec((yysemantic_stack_[(3) - (1)].real),(yysemantic_stack_[(3) - (3)].real));}
			break;

			case 320:

				/* Line 663 of lalr1.cc  */
#line 1524 "ds9/ds9parse.yy"
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
			break;



			/* Line 663 of lalr1.cc  */
#line 3173 "/Users/drs/develop/casa/code/build/display/ds9parse.cc"
			default:
				break;
			}
			/* User semantic actions sometimes alter yychar, and that requires
			   that yytoken be updated with the new translation.  We take the
			   approach of translating immediately before every use of yytoken.
			   One alternative is translating here after every semantic action,
			   but that translation would be missed if the semantic action
			   invokes YYABORT, YYACCEPT, or YYERROR immediately after altering
			   yychar.  In the case of YYABORT or YYACCEPT, an incorrect
			   destructor might then be invoked immediately.  In the case of
			   YYERROR, subsequent parser actions might lead to an incorrect
			   destructor call or verbose syntax error message before the
			   lookahead is translated.  */
			YY_SYMBOL_PRINT ("-> $$ =", yyr1_[yyn], &yyval, &yyloc);

			yypop_ (yylen);
			yylen = 0;
			YY_STACK_PRINT ();

			yysemantic_stack_.push (yyval);
			yylocation_stack_.push (yyloc);

			/* Shift the result of the reduction.  */
			yyn = yyr1_[yyn];
			yystate = yypgoto_[yyn - yyntokens_] + yystate_stack_[0];
			if (0 <= yystate && yystate <= yylast_
			        && yycheck_[yystate] == yystate_stack_[0])
				yystate = yytable_[yystate];
			else
				yystate = yydefgoto_[yyn - yyntokens_];
			goto yynewstate;

			/*------------------------------------.
			| yyerrlab -- here on detecting error |
			`------------------------------------*/
yyerrlab:
			/* Make sure we have latest lookahead translation.  See comments at
			   user semantic actions for why this is necessary.  */
			yytoken = yytranslate_ (yychar);

			/* If not already recovering from an error, report this error.  */
			if (!yyerrstatus_) {
				++yynerrs_;
				if (yychar == yyempty_)
					yytoken = yyempty_;
				error (yylloc, yysyntax_error_ (yystate, yytoken));
			}

			yyerror_range[1] = yylloc;
			if (yyerrstatus_ == 3) {
				/* If just tried and failed to reuse lookahead token after an
				 error, discard it.  */

				if (yychar <= yyeof_) {
					/* Return failure if at end of input.  */
					if (yychar == yyeof_)
						YYABORT;
				} else {
					yydestruct_ ("Error: discarding", yytoken, &yylval, &yylloc);
					yychar = yyempty_;
				}
			}

			/* Else will try to reuse lookahead token after shifting the error
			   token.  */
			goto yyerrlab1;


			/*---------------------------------------------------.
			| yyerrorlab -- error raised explicitly by YYERROR.  |
			`---------------------------------------------------*/
yyerrorlab:

			/* Pacify compilers like GCC when the user code never invokes
			   YYERROR and the label yyerrorlab therefore never appears in user
			   code.  */
			if (false)
				goto yyerrorlab;

			yyerror_range[1] = yylocation_stack_[yylen - 1];
			/* Do not reclaim the symbols of the rule which action triggered
			   this YYERROR.  */
			yypop_ (yylen);
			yylen = 0;
			yystate = yystate_stack_[0];
			goto yyerrlab1;

			/*-------------------------------------------------------------.
			| yyerrlab1 -- common code for both syntax error and YYERROR.  |
			`-------------------------------------------------------------*/
yyerrlab1:
			yyerrstatus_ = 3;	/* Each real token shifted decrements this.  */

			for (;;) {
				yyn = yypact_[yystate];
				if (!yy_pact_value_is_default_ (yyn)) {
					yyn += yyterror_;
					if (0 <= yyn && yyn <= yylast_ && yycheck_[yyn] == yyterror_) {
						yyn = yytable_[yyn];
						if (0 < yyn)
							break;
					}
				}

				/* Pop the current state because it cannot handle the error token.  */
				if (yystate_stack_.height () == 1)
					YYABORT;

				yyerror_range[1] = yylocation_stack_[0];
				yydestruct_ ("Error: popping",
				             yystos_[yystate],
				             &yysemantic_stack_[0], &yylocation_stack_[0]);
				yypop_ ();
				yystate = yystate_stack_[0];
				YY_STACK_PRINT ();
			}

			yyerror_range[2] = yylloc;
			// Using YYLLOC is tempting, but would change the location of
			// the lookahead.  YYLOC is available though.
			YYLLOC_DEFAULT (yyloc, yyerror_range, 2);
			yysemantic_stack_.push (yylval);
			yylocation_stack_.push (yyloc);

			/* Shift the error token.  */
			YY_SYMBOL_PRINT ("Shifting", yystos_[yyn],
			                 &yysemantic_stack_[0], &yylocation_stack_[0]);

			yystate = yyn;
			goto yynewstate;

			/* Accept.  */
yyacceptlab:
			yyresult = 0;
			goto yyreturn;

			/* Abort.  */
yyabortlab:
			yyresult = 1;
			goto yyreturn;

yyreturn:
			if (yychar != yyempty_) {
				/* Make sure we have latest lookahead translation.  See comments
				   at user semantic actions for why this is necessary.  */
				yytoken = yytranslate_ (yychar);
				yydestruct_ ("Cleanup: discarding lookahead", yytoken, &yylval,
				             &yylloc);
			}

			/* Do not reclaim the symbols of the rule which action triggered
			   this YYABORT or YYACCEPT.  */
			yypop_ (yylen);
			while (yystate_stack_.height () != 1) {
				yydestruct_ ("Cleanup: popping",
				             yystos_[yystate_stack_[0]],
				             &yysemantic_stack_[0],
				             &yylocation_stack_[0]);
				yypop_ ();
			}

			return yyresult;
		}

		// Generate an error message.
		std::string
		ds9parse::yysyntax_error_ (int yystate, int yytoken) {
			std::string yyres;
			// Number of reported tokens (one for the "unexpected", one per
			// "expected").
			size_t yycount = 0;
			// Its maximum.
			enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
			// Arguments of yyformat.
			char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];

			/* There are many possibilities here to consider:
			   - If this state is a consistent state with a default action, then
			     the only way this function was invoked is if the default action
			     is an error action.  In that case, don't check for expected
			     tokens because there are none.
			   - The only way there can be no lookahead present (in yytoken) is
			     if this state is a consistent state with a default action.
			     Thus, detecting the absence of a lookahead is sufficient to
			     determine that there is no unexpected or expected token to
			     report.  In that case, just report a simple "syntax error".
			   - Don't assume there isn't a lookahead just because this state is
			     a consistent state with a default action.  There might have
			     been a previous inconsistent state, consistent state with a
			     non-default action, or user semantic action that manipulated
			     yychar.
			   - Of course, the expected token list depends on states to have
			     correct lookahead information, and it depends on the parser not
			     to perform extra reductions after fetching a lookahead from the
			     scanner and before detecting a syntax error.  Thus, state
			     merging (from LALR or IELR) and default reductions corrupt the
			     expected token list.  However, the list is correct for
			     canonical LR with one exception: it will still contain any
			     token that will not be accepted due to an error action in a
			     later state.
			*/
			if (yytoken != yyempty_) {
				yyarg[yycount++] = yytname_[yytoken];
				int yyn = yypact_[yystate];
				if (!yy_pact_value_is_default_ (yyn)) {
					/* Start YYX at -YYN if negative to avoid negative indexes in
					   YYCHECK.  In other words, skip the first -YYN actions for
					   this state because they are default actions.  */
					int yyxbegin = yyn < 0 ? -yyn : 0;
					/* Stay within bounds of both yycheck and yytname.  */
					int yychecklim = yylast_ - yyn + 1;
					int yyxend = yychecklim < yyntokens_ ? yychecklim : yyntokens_;
					for (int yyx = yyxbegin; yyx < yyxend; ++yyx)
						if (yycheck_[yyx + yyn] == yyx && yyx != yyterror_
						        && !yy_table_value_is_error_ (yytable_[yyx + yyn])) {
							if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM) {
								yycount = 1;
								break;
							} else
								yyarg[yycount++] = yytname_[yyx];
						}
				}
			}

			char const* yyformat = YY_NULL;
			switch (yycount) {
#define YYCASE_(N, S)                         \
        case N:                               \
          yyformat = S;                       \
        break
				YYCASE_(0, YY_("syntax error"));
				YYCASE_(1, YY_("syntax error, unexpected %s"));
				YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
				YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
				YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
				YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
			}

			// Argument number.
			size_t yyi = 0;
			for (char const* yyp = yyformat; *yyp; ++yyp)
				if (yyp[0] == '%' && yyp[1] == 's' && yyi < yycount) {
					yyres += yytnamerr_ (yyarg[yyi++]);
					++yyp;
				} else
					yyres += *yyp;
			return yyres;
		}


		/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
		   STATE-NUM.  */
		const short int ds9parse::yypact_ninf_ = -614;
		const short int
		ds9parse::yypact_[] = {
			-614,    19,  1624,  -614,  -614,  -614,    59,  -614,  -614,  -614,
			-614,  -614,  2353,  -614,  -614,  -614,  -614,  -614,    36,  -614,
			-614,  -614,  -614,  -614,  -614,  -614,  -614,  -614,  -614,  -614,
			-614,  -614,  -614,  -614,  -614,  -614,  -614,  -614,  -614,  -614,
			-614,  -614,  -614,  -614,  -614,  -614,  -614,  -614,  1398,   170,
			-614,  -614,  -614,  1584,  -614,  -614,  -614,  -614,   -97,   -78,
			-67,   -43,   -40,  -614,  -614,  -614,   -12,  -614,  -614,     6,
			-614,    12,  -614,    40,  -614,    47,    82,    94,    98,   100,
			105,   111,   112,    85,  -614,    25,  -614,   170,  -614,  -614,
			-614,  -614,    25,    25,    72,   180,   -38,   181,    72,   -21,
			72,    72,    72,    72,   187,   189,    72,    72,    72,    72,
			72,    72,  -614,    72,    72,    72,    72,    72,   191,   233,
			-614,  -614,   753,  -614,    20,  2044,   164,   266,   271,   268,
			37,  1729,   273,   132,   276,   284,  1077,   286,    57,  -614,
			-614,  -614,  2353,  -614,  -614,  -614,  -614,  -614,   464,    72,
			72,   464,    72,   464,    72,   464,   464,   464,   464,   464,
			72,    72,   464,   464,   161,   464,   464,   464,    72,   464,
			464,   464,   464,   464,    72,    72,    72,    72,    72,    72,
			72,   288,    72,  -614,   287,   270,  -614,  -614,  -614,   289,
			-614,   320,  -614,   291,  -614,   294,  -614,   298,  -614,  -614,
			-614,  -614,  -614,  -614,  -614,   301,  -614,  -614,  -614,  1049,
			-614,  1939,  1049,  -614,  -614,  -614,  -614,  -614,  -614,  -614,
			-614,  -614,  -614,  -614,  -614,  -614,  -614,  -614,  -614,  -614,
			-614,  -614,  -614,  -614,  -614,  -614,   306,  -614,   179,   179,
			179,  -614,  -614,  -614,   179,   179,   179,   179,   179,   464,
			464,   179,   464,   179,   464,   179,   179,   179,   179,   179,
			464,   464,   179,   179,  -614,   182,   179,   179,   161,   464,
			179,   179,   179,    23,   179,   464,   464,   464,   464,   464,
			464,   464,  -614,   464,  -614,  -614,  -614,  -614,   310,  -614,
			-614,  -614,  -614,  -614,  -614,  2254,  -614,  -614,  -614,  -614,
			312,   311,   305,   224,   313,   309,   309,   531,   161,   161,
			613,   161,   132,   161,   531,   531,   531,   132,   132,   161,
			161,   613,   132,   194,   200,   464,   132,   182,  -614,    78,
			-614,   464,   613,   464,   323,   182,   531,   161,   179,   179,
			179,   179,   179,   161,   179,   326,    91,  -614,  -614,  -614,
			-614,  -614,  -614,  -614,  -614,  -614,  -614,  -614,  -614,  -614,
			179,   182,   182,   179,   179,   179,   179,   179,   179,   179,
			182,   179,   182,   161,   161,   161,   161,  -614,   179,   182,
			182,   179,   179,  -614,  2256,  -614,  -614,   161,   179,   200,
			464,   182,   179,   179,   161,   161,   200,   179,   182,   531,
			531,   132,   464,   464,   182,   531,   328,  -614,   531,   200,
			200,   329,   324,   325,   330,   327,   224,   427,   200,   132,
			200,   182,   182,   182,   182,   132,   200,   200,   427,   132,
			-614,   205,   208,   210,   213,   215,   216,   -43,   220,   221,
			222,   226,   227,   229,    47,   230,   231,   236,   237,   244,
			247,   374,   249,    31,  -614,   182,   132,  -614,  -614,   200,
			531,   132,   182,  -614,  -614,   132,   200,   161,   161,   161,
			179,   161,  1515,   179,  -614,    78,  -614,  -614,  -614,  -614,
			-614,  -614,  -614,  -614,    27,    26,   161,   179,  -614,   179,
			-614,   200,   200,   200,   200,   179,  -614,  -614,   161,   179,
			179,   333,  2405,    33,  2149,   380,   335,   381,   336,   383,
			382,    37,  1834,   390,   391,   132,   394,   395,   396,  -614,
			57,   398,  2256,  -614,   200,    78,  -614,   161,   161,   200,
			182,   161,  -614,   182,   182,   182,   531,   182,  -614,   403,
			39,  -614,   132,   264,   182,  -103,   241,   397,  -614,  -614,
			-614,  -614,   406,   -32,   241,   407,  -614,  -614,   258,   157,
			-614,  -614,  -614,   159,  -614,  -614,  -614,  -614,  -614,  -614,
			-614,  -614,  -614,  -614,  -614,  -614,   409,  -614,   410,  -614,
			320,  -614,   413,  -614,  -614,  -614,   283,   416,  -614,  -614,
			292,  -614,   418,   419,  1049,  -614,  1939,  1049,  -614,  -614,
			-614,  -614,  -614,  -614,  -614,  -614,  -614,  -614,  -614,  -614,
			-614,    67,   182,   182,   182,  -614,   200,   182,  1515,  1515,
			1515,   161,  1515,  -614,   424,  -614,   161,   315,  -614,   161,
			78,  -614,   200,   308,   200,    80,    88,   161,   179,   179,
			179,  -614,   179,   179,   314,   200,   161,   179,   179,   179,
			132,  -614,  -614,  -614,  -614,  -614,  -614,   435,  -614,  -614,
			436,   132,  -614,   132,  -614,  -614,  -614,  -614,  -614,  2254,
			-614,  -614,  -614,  -614,  -614,   161,    78,  -614,   200,   200,
			200,  -614,   200,  -614,  -614,  -614,   182,  -614,  -614,  -614,
			182,   441,   182,   531,   182,  -614,   422,  -614,   182,   531,
			132,   476,   613,   531,   414,  -614,   182,   132,   476,   613,
			-49,  -614,   445,   -14,    15,    91,   182,   132,   182,  -614,
			-614,  -614,  -614,  1515,  -614,  1515,  -614,   200,  -614,   200,
			72,   200,  -614,   161,   161,  -614,   179,   179,    72,   200,
			161,   161,   179,   332,   448,   334,   337,  -614,   200,  -614,
			200,  -614,  -614,  -614,  -614,   464,  -614,   182,   182,   613,
			531,   464,  -614,   182,   182,   613,   531,  -614,   531,   531,
			-614,  -614,   179,   200,   200,   179,   179,   179,   200,   200,
			179,    66,    69,    70,   613,  -614,  -614,   451,   455,   613,
			-614,  -614,   458,   338,   531,  -614,   343,   179,   179,   161,
			179,   179,   132,   132,   132,   132,   182,   132,   132,   359,
			361,   161,   161,   200,   161,   161,  -614,  -614,  -614,   182,
			-614,  -614,   182,   200,   200,  -614,  -614
		};

		/* YYDEFACT[S] -- default reduction number in state S.  Performed when
		   YYTABLE doesn't specify something else to do.  Zero means the
		   default is an error.  */
		const unsigned short int
		ds9parse::yydefact_[] = {
			200,     0,   249,     1,    94,   125,     0,    93,   130,   124,
			126,   129,     0,   128,    91,   127,    14,    92,     0,     7,
			96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
			106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
			116,   117,   118,   119,   120,   121,   122,   123,   249,     0,
			10,    95,    12,     0,    44,    43,     6,   176,     0,     0,
			0,   174,     0,   141,   138,   137,     0,   135,   142,     0,
			139,     0,   140,     0,   134,   175,     0,     0,     0,     0,
			0,     0,     0,    54,   166,    24,     9,     0,     2,    40,
			39,     4,    24,    24,    56,     0,    56,     0,    56,    56,
			56,    56,    56,    56,     0,     0,    56,    56,    56,    56,
			56,    56,   293,    56,    56,    56,    56,    56,     0,    22,
			257,   258,     0,    16,     0,     0,     0,     0,     0,     0,
			0,     0,     0,     0,     0,     0,     0,     0,     0,    25,
			55,     8,     0,    15,     3,    11,    13,    57,     0,    56,
			56,     0,    56,     0,    56,     0,     0,     0,     0,     0,
			56,    56,     0,     0,    58,     0,     0,     0,    56,     0,
			0,     0,     0,     0,    56,    56,    56,    56,    56,    56,
			56,     0,    56,    18,     0,     0,    19,    17,   168,     0,
			199,   197,   198,     0,   181,     0,   172,     0,   255,   251,
			256,   250,   253,   252,   254,   177,   132,   133,   131,     0,
			195,     0,     0,   194,   182,   173,    42,    41,    66,    67,
			65,   184,   185,   180,   186,   171,    45,    53,    51,    50,
			52,    48,    49,    47,    46,   167,     0,   165,    54,    54,
			54,    81,    82,    83,    54,    54,    54,    54,    54,     0,
			0,    54,     0,    54,     0,    54,    54,    54,    54,    54,
			0,     0,    54,    54,    59,    60,    54,    54,    58,     0,
			54,    54,    54,    58,    54,     0,     0,     0,     0,     0,
			0,     0,    21,     0,    23,    20,   169,   196,     0,   170,
			179,   178,   192,   193,   188,     0,   189,   190,   191,    26,
			0,     0,     0,     0,     0,     0,     0,     0,    58,    58,
			0,    58,     0,    58,     0,     0,     0,    63,     0,    58,
			58,     0,     0,    61,   320,     0,     0,    60,   310,    54,
			309,     0,     0,     0,     0,    60,     0,    58,    54,    54,
			54,    54,    54,    58,    54,     0,     0,    88,    90,    89,
			87,    84,    85,    86,    71,    72,    73,    69,    70,    68,
			54,    60,    60,    54,    54,    54,    54,    54,    54,    54,
			60,    54,    60,    58,    58,    58,    58,    64,    54,    60,
			60,    54,    54,    62,    28,   298,    27,    58,    54,   320,
			0,    60,    54,    54,    58,    58,   320,    54,    60,     0,
			0,    63,     0,     0,    60,     0,     0,   187,     0,   320,
			320,     0,     0,     0,     0,     0,     0,    63,   320,     0,
			320,    60,    60,    60,    60,     0,   320,   320,    63,     0,
			214,     0,     0,     0,     0,     0,     0,   212,     0,     0,
			0,     0,     0,     0,   213,     0,     0,     0,     0,     0,
			0,     0,     0,    54,   202,    60,     0,   285,   308,   320,
			0,    63,    60,   300,   299,     0,   320,    58,    58,    58,
			54,    58,   320,    54,   183,    54,   291,   287,    77,    78,
			79,    75,    76,    74,    66,    65,    58,    54,   292,    54,
			286,   320,   320,   320,   320,    54,   289,   288,    58,    54,
			54,   246,     0,     0,     0,     0,   240,     0,   243,     0,
			0,     0,     0,     0,     0,     0,     0,     0,     0,   320,
			0,     0,     0,    30,   320,    54,   294,    58,    58,   320,
			60,    58,   290,    60,    60,    60,     0,    60,   259,     0,
			54,    33,     0,     0,    60,    60,    63,     0,   266,   307,
			306,   302,     0,    60,    63,     0,   248,   226,     0,     0,
			159,   157,   152,     0,   162,   146,   163,   149,   161,   155,
			144,   158,   147,   145,   164,   160,     0,   204,     0,   239,
			237,   238,     0,   219,   242,   224,     0,     0,   245,   225,
			0,   208,     0,   215,     0,   235,     0,     0,   234,   220,
			210,   209,   222,   223,   218,   207,    29,   203,   320,   201,
			284,     0,    60,    60,    60,   305,   320,    60,   320,   320,
			320,    58,   320,   320,     0,    36,    58,     0,   313,    58,
			54,   312,   320,     0,   320,    71,    68,    58,    54,    54,
			54,   318,    54,    54,     0,   320,    58,    54,    54,    54,
			0,   151,   148,   154,   153,   150,   156,     0,   205,   236,
			0,     0,   206,     0,   217,   216,   232,   233,   228,     0,
			229,   230,   231,    32,   316,    58,    54,   315,   320,   320,
			320,   301,   320,   265,   264,   260,    60,   263,    35,   320,
			60,     0,    60,     0,    60,   267,     0,   277,    60,     0,
			63,    63,     0,     0,     0,   271,    60,    63,    63,     0,
			54,   211,     0,    54,    54,     0,    60,     0,    60,   295,
			304,   278,   303,   320,    38,   320,    80,   320,   311,   320,
			56,   320,   319,    58,    58,   317,    54,    54,    56,   320,
			58,    58,    54,     0,     0,     0,     0,   227,   320,   314,
			320,   262,   261,   269,   268,     0,   279,    60,    60,     0,
			0,     0,   272,    60,    60,     0,     0,   221,     0,     0,
			297,   296,    54,   320,   320,    54,    54,    54,   320,   320,
			54,    54,    54,    54,     0,   281,   280,     0,     0,     0,
			273,   274,     0,     0,     0,   241,     0,    54,    54,    58,
			54,    54,     0,     0,    63,    63,    60,    63,    63,     0,
			0,    58,    58,   320,    58,    58,   247,   244,   282,    60,
			270,   275,    60,   320,   320,   283,   276
		};

		/* YYPGOTO[NTERM-NUM].  */
		const short int
		ds9parse::yypgoto_[] = {
			-614,  -614,  -614,   415,  -614,  -614,  -614,  -614,  -614,    29,
			-614,   611,  -614,  -614,  -502,  -614,  -614,   385,   186,  -614,
			-29,   -83,   -93,   589,   588,  -352,   -92,  -167,  -320,  -477,
			188,  -614,   -66,   594,   -80,   363,   -99,  -169,    -1,  -614,
			-614,   353,  -614,  -614,  -614,   121,   -16,  -614,  -614,  -614,
			-614,  -614,  -614,  -614,  -614,  -614,    -2,   392,   339,   388,
			-614,  -614,  -614,   123,  -253,  -173,  -613,  -196,  -550,  -602,
			-46
		};

		/* YYDEFGOTO[NTERM-NUM].  */
		const short int
		ds9parse::yydefgoto_[] = {
			-1,     1,    48,    49,    92,    93,    85,   183,   184,   141,
			236,   385,   451,   521,   538,   539,   624,    91,   244,    56,
			235,   717,   148,   265,   324,   376,   377,   638,   369,   629,
			245,   246,   247,   328,    50,    51,    52,   213,   452,   576,
			83,    84,   214,   193,     2,   540,   454,   599,   582,   585,
			586,   589,   590,   557,   558,    53,   205,   122,   186,   123,
			168,   530,   329,   330,   630,   631,   676,   677,   640,   641,
			386
		};

		/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
		   positive, shift that token.  If negative, reduce the rule which
		   number is the opposite.  If YYTABLE_NINF_, syntax error.  */
		const short int ds9parse::yytable_ninf_ = -321;
		const short int
		ds9parse::yytable_[] = {
			142,   381,    88,   151,   648,   153,   155,   156,   157,   158,
			159,    82,   393,   162,   163,   164,   165,   166,   167,     3,
			169,   170,   171,   172,   173,   188,   192,   323,   -54,   -54,
			-54,   633,   212,   -54,   124,   150,   -31,   710,   577,    86,
			293,   221,   296,   298,   -37,   191,  -320,  -320,   713,   469,
			714,   211,   154,   125,  -320,  -320,   249,   250,   198,   252,
			226,   254,   199,   200,   126,   486,   201,   260,   261,   639,
			216,   217,   202,   218,   219,   269,   498,   647,   140,   203,
			743,   275,   276,   277,   278,   279,   280,   281,  -136,   283,
			147,   127,   287,   -58,   -58,   -66,   -66,   487,   323,   735,
			-24,   -24,   644,   -65,   -65,   227,   735,   147,   499,   528,
			206,   207,   295,   140,   143,   745,   683,   684,   685,   128,
			687,   145,   146,   228,   229,   230,   231,    54,    55,   292,
			208,   294,   297,   627,   675,   216,   217,   129,   218,   219,
			360,    82,   140,   130,   746,   232,   189,   373,   374,   375,
			140,   139,   264,   140,   140,   300,   301,   302,   140,   578,
			204,   303,   304,   305,   306,   307,   140,   194,   310,   397,
			312,   131,   314,   315,   316,   317,   318,   407,  -143,   321,
			322,   233,   234,   325,   326,    89,    90,   331,   332,   333,
			334,   336,   735,   140,   637,   793,   140,   140,   795,   796,
			147,   651,   646,   654,   -58,   140,   -66,   264,   -58,   -66,
			-66,   139,   140,   132,   -65,   346,   781,   -65,   -65,   783,
			371,   751,   652,   752,   655,   133,   378,   216,   217,   134,
			382,   135,   467,   468,   388,   653,   136,   656,   473,   352,
			353,   475,   137,   138,   216,   217,   390,   635,   219,   355,
			356,   357,   358,   149,   152,   399,   400,   401,   402,   403,
			160,   405,   161,   175,   174,   176,   177,   216,   217,   195,
			354,   197,   355,   356,   357,   358,   196,   408,   215,   222,
			411,   412,   413,   414,   415,   416,   417,   223,   419,   225,
			264,   282,   284,   527,   286,   425,   288,   289,   428,   429,
			175,   290,   176,   177,   291,   456,   140,   627,   178,   460,
			461,   299,   323,   179,   465,   345,   349,   180,   347,   220,
			181,   348,   182,   243,   383,   241,   384,   489,   395,   406,
			627,   474,   479,   495,   480,   478,   501,   500,   482,   502,
			481,   503,     5,   598,   504,   178,   505,   506,   733,   734,
			179,   507,   508,   509,   180,   740,   741,   510,   511,   182,
			512,   513,   514,     8,   525,   120,   121,   515,   516,   621,
			522,     9,    10,   531,    11,   517,   628,    13,   518,   519,
			520,    15,   736,   583,   587,   592,   650,   536,   591,   742,
			542,   556,   543,   584,   588,   600,   601,   603,   604,   605,
			642,   411,   416,   608,   546,   581,   547,   523,   623,   643,
			649,   661,   552,   597,   657,   658,   554,   555,   660,   662,
			663,   664,   665,   602,   580,   667,   541,   670,   672,   689,
			216,   217,   596,   484,   219,   364,   365,   366,   367,   775,
			711,   712,   611,   696,   726,   780,   691,   730,   744,   704,
			626,   767,   811,   812,   798,   814,   815,   522,   799,   738,
			766,   801,   768,    87,   797,   769,   802,   216,   217,   800,
			238,   803,   144,   606,   239,   240,   241,   242,   243,   216,
			217,   659,   635,   219,   355,   356,   357,   358,   816,   350,
			817,   607,   351,   359,   625,   237,   368,   669,   220,   224,
			359,   359,   359,   220,   220,   453,   609,   368,   220,   593,
			187,   185,   220,   458,   666,   782,   668,   671,   368,   674,
			728,   749,   359,     0,   285,     0,   628,     0,     0,     0,
			0,     0,   732,     0,   216,   217,   737,   354,     0,   355,
			356,   357,   358,     0,     0,     0,   747,   693,     0,     0,
			0,     0,     0,     0,     0,   699,   700,   701,   674,   702,
			703,     0,   673,     0,   707,   708,   709,     0,     0,   674,
			0,   674,   541,   541,   541,     0,   541,   688,     0,     0,
			0,     0,     0,     0,     0,   359,   359,   220,     0,   715,
			0,   359,     0,   776,   359,     0,     0,     0,     0,     0,
			0,   628,   483,   485,     0,   220,     0,     0,     0,     0,
			0,   220,     0,     0,   485,   220,   216,   217,     0,   363,
			0,   364,   365,   366,   367,   674,     0,     0,     0,     0,
			0,     0,     0,     0,     0,     0,     0,   755,     0,     0,
			0,     0,   220,   724,     0,   761,   359,   220,     0,     0,
			0,   220,     0,   759,   760,     0,     0,     0,     0,   765,
			0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
			0,     0,     0,     0,     0,     0,     0,   541,     0,   541,
			0,     0,     0,     0,     0,     0,     0,     0,     0,   784,
			0,     0,   787,   788,   789,     0,     0,   792,   794,   693,
			794,   220,     0,     0,     0,     0,     0,     0,     0,     0,
			809,   810,     0,     0,   804,   805,     0,   807,   808,     0,
			0,     0,   359,     0,     0,     0,     0,     0,   220,   359,
			0,     0,   636,     0,     0,     0,     0,     0,     0,     0,
			636,     0,   248,     0,     0,   251,     0,   253,     0,   255,
			256,   257,   258,   259,     0,     0,   262,   263,     0,   266,
			267,   268,     0,   270,   271,   272,   273,   274,     0,     0,
			0,    94,     0,     0,    95,     0,     0,     0,    96,    97,
			98,     0,    99,   100,     0,   101,   102,   103,   104,     0,
			0,     0,     0,     0,     0,   105,     0,   220,   106,     0,
			107,     0,   108,     0,     0,     0,     0,     0,     0,     0,
			0,     0,     0,     0,     0,     0,   109,     0,     0,     0,
			0,     0,     0,     0,   110,     0,   111,   112,   113,     0,
			0,     0,   114,   115,     0,     0,   220,   116,     0,     0,
			0,     0,   117,   308,   309,     0,   311,   220,   313,   220,
			0,     0,     0,     0,   319,   320,     0,   327,     0,     0,
			0,     0,   335,     0,     0,     0,     0,     0,     0,   337,
			338,   339,   340,   341,   342,   343,   118,   344,     0,   359,
			0,     0,     0,     0,     0,   359,   220,   636,   368,   359,
			0,     0,     0,   220,   636,   368,     0,   361,   362,     0,
			370,     0,   372,   220,     0,     0,     0,     0,   379,   380,
			0,     0,     0,     0,     0,   389,     0,     0,   391,   387,
			0,     0,     0,   396,     0,   392,   398,   394,     0,     0,
			0,     0,   404,     0,     0,     0,     0,     0,     0,     0,
			0,     0,     0,     0,     0,   368,   359,     0,     0,   409,
			410,   368,   359,     0,   359,   359,     0,     0,   418,     0,
			420,     0,   421,   422,   423,   424,     0,   426,   427,     0,
			368,     0,     0,     0,     0,   368,   455,     0,     0,   459,
			359,     0,     0,   462,   463,     0,   466,     0,   220,   220,
			220,   220,   472,   220,   220,     0,   470,   471,     0,     0,
			457,     0,     0,     0,     0,     0,     0,   464,     0,   491,
			492,   493,   494,     0,     0,     0,     0,     0,     0,     0,
			476,   477,     0,     0,     0,     0,     0,     0,     0,   488,
			0,   490,     0,     0,     0,     0,     0,   496,   497,     0,
			0,     0,     0,   524,     0,     0,     0,     0,     0,     0,
			529,     0,     0,     0,     0,     0,   533,   534,   535,     0,
			537,     0,     0,     0,   544,     0,     4,     0,   206,   207,
			526,     0,     0,     0,     0,   545,     0,   532,     0,     0,
			0,     0,     0,     0,     0,     0,     0,   553,   208,     0,
			7,     0,     0,     0,     0,     0,     0,     0,     0,     0,
			0,     0,   548,   549,   550,   551,     0,     0,    14,     0,
			0,     0,     0,     0,   612,     0,   613,   614,   616,    17,
			617,   618,   619,   620,     0,   622,     0,     0,     0,     0,
			0,     0,   632,   634,     0,   610,     0,     0,     0,     0,
			615,   645,     0,    20,    21,    22,    23,    24,    25,    26,
			27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
			37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
			47,    20,    21,    22,    23,    24,    25,    26,    27,    28,
			29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
			39,    40,    41,    42,    43,    44,    45,    46,    47,     0,
			678,   679,   680,     0,     0,   682,     0,     0,     0,     0,
			686,     0,     0,     0,     0,   690,     0,     0,   692,   694,
			0,     0,     0,     0,     0,     0,   698,   681,     0,     0,
			0,     0,     0,     0,     0,   706,     0,     0,     0,     0,
			0,     0,     0,   695,     0,   697,     0,     0,     0,     0,
			0,     0,     0,     0,     0,     0,   705,     0,     0,     0,
			0,     0,     0,     0,   716,   718,     0,     0,     0,     0,
			0,     0,     0,     0,   723,     0,     0,     0,   725,     0,
			727,     0,   729,     0,     0,     0,   731,     0,     0,   719,
			720,   721,     0,   722,   739,     0,     0,     0,     0,     0,
			0,     0,     0,     0,   748,     0,   750,     0,     0,     0,
			0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
			0,     0,   757,   758,     0,     0,     0,     0,     0,   763,
			764,     0,     0,     0,     0,     0,     0,     0,   753,     0,
			754,     0,   756,     0,     0,   773,   774,     0,     0,   772,
			762,   778,   779,     0,     0,   777,     0,     0,     0,   770,
			0,   771,     0,     0,     0,     0,     0,     0,     0,     0,
			0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
			0,     0,     0,     0,   785,   786,     0,     0,   806,   790,
			791,     0,     0,     0,   813,     0,     0,     0,  -320,     0,
			818,   819,     0,   821,   822,     0,     0,   823,     0,     0,
			824,     0,     0,    -5,    -5,     4,     0,     0,     0,     0,
			5,     0,     0,     0,   820,     0,     0,     0,     0,     0,
			0,     0,     0,     0,   825,   826,     6,     0,     0,     7,
			0,     8,     0,     0,     0,     0,     0,     0,     0,     9,
			10,     0,    11,    12,     0,    13,     0,    14,     0,    15,
			0,     0,    16,     0,     0,     0,     0,     0,    17,     0,
			0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
			0,     0,     0,     0,     0,    18,     0,     0,    19,     0,
			0,     0,    20,    21,    22,    23,    24,    25,    26,    27,
			28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
			38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
			-34,     0,     0,     0,     0,     0,     0,     0,     0,     0,
			0,     0,     0,     0,     0,     0,     0,     0,   430,     0,
			0,     0,   431,   432,     0,     0,   433,   434,   435,   436,
			0,   437,   438,     0,     0,    63,     0,     0,     0,    64,
			0,     0,   439,     0,     0,    65,     0,     0,   440,     0,
			0,    67,     0,     0,     0,    68,     0,     0,   441,     0,
			70,     0,     0,     0,     0,     0,     0,     0,   442,     0,
			0,     0,     0,    72,     0,   443,    74,   444,   445,   446,
			447,   448,    94,     0,   449,    95,     0,     0,     0,    96,
			97,    98,     0,    99,   100,     0,   101,   102,   103,   104,
			0,     0,     0,     0,     0,     0,   105,     0,     0,   106,
			0,   107,     0,   108,     0,     0,     0,   450,     0,    -5,
			-5,     4,     0,     0,     0,     0,     5,   109,     0,     0,
			0,     0,     0,     0,     0,   110,     0,   111,   112,   113,
			0,     0,     6,   114,   115,     7,     0,     8,   116,     0,
			0,     0,     0,   117,     0,     9,    10,     0,    11,    12,
			0,    13,     0,    14,     0,    15,     0,     0,    16,     0,
			0,     0,     0,     0,    17,     0,     0,     0,     0,     0,
			0,     0,     0,     0,     0,     0,     0,   118,     0,     0,
			119,    18,     0,     0,    19,     0,   120,   121,    20,    21,
			22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
			32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
			42,    43,    44,    45,    46,    47,     4,     0,   206,   207,
			0,     5,     0,     0,     0,     0,     0,     0,     0,     0,
			0,     0,     0,     0,     0,     0,     0,     0,   208,     0,
			7,     0,     8,     0,     0,     0,     0,     0,     0,     0,
			9,    10,     0,    11,     0,     0,    13,     0,    14,     0,
			15,     0,     0,   209,     0,     0,     0,     0,     0,    17,
			0,   210,     0,     0,     0,     0,     0,     0,     0,     0,
			0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
			0,     0,     0,    20,    21,    22,    23,    24,    25,    26,
			27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
			37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
			47,     4,     0,   206,   207,     0,     5,     0,     0,     0,
			0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
			0,     0,     0,   208,     0,     7,     0,     8,     0,     0,
			0,     0,     0,     0,     0,     9,    10,     0,    11,     0,
			0,    13,     0,    14,     0,    15,     0,     0,   594,     0,
			0,     0,     0,     0,    17,     0,   595,     0,     0,     0,
			0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
			0,     0,     0,     0,     0,     0,     0,     0,    20,    21,
			22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
			32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
			42,    43,    44,    45,    46,    47,     4,     0,   206,   207,
			0,     5,     0,     0,     0,     0,     0,     0,     0,     0,
			0,     0,     0,     0,     0,     0,     0,     0,   208,     0,
			7,     0,     8,     0,     0,     0,     0,     0,     0,     0,
			9,    10,     0,    11,     0,     0,    13,     0,    14,     0,
			15,     0,     0,     0,     0,     0,     0,     0,     0,    17,
			0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
			0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
			0,     0,     0,    20,    21,    22,    23,    24,    25,    26,
			27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
			37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
			47,     4,     0,     0,     0,     0,     5,     0,     0,     0,
			0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
			0,     0,     0,     0,     0,     7,     0,     8,     0,     0,
			0,     0,     0,     0,     0,     9,    10,     0,    11,     0,
			0,    13,     0,    14,     0,    15,     0,     0,   190,     0,
			0,     0,     0,     0,    17,     0,     0,     0,     0,     0,
			0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
			0,     0,     0,     0,     0,     0,     0,     0,    20,    21,
			22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
			32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
			42,    43,    44,    45,    46,    47,     4,     0,     0,     0,
			0,     5,     0,     0,     0,     0,     0,     0,     0,     0,
			0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
			7,     0,     8,     0,     0,     0,     0,     0,     0,     0,
			9,    10,     0,    11,     0,     0,    13,     0,    14,     0,
			15,     0,     0,   579,     0,     0,     0,     0,     0,    17,
			0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
			0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
			0,     0,     0,    20,    21,    22,    23,    24,    25,    26,
			27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
			37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
			47,     4,     0,     0,     0,     0,     0,     0,     0,   430,
			0,     0,     0,   431,   432,     0,     0,   433,   434,   435,
			436,     0,   437,   438,     0,     7,    63,     0,     0,     0,
			64,     0,     0,   439,     0,     0,    65,     0,     0,   440,
			0,     0,    67,    14,     0,     0,    68,     0,     0,   441,
			0,    70,     0,     0,    17,     0,     0,     0,     0,   442,
			0,     0,     0,     0,    72,     0,   443,    74,   444,   445,
			446,   447,   448,     0,     0,   449,     0,     0,    20,    21,
			22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
			32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
			42,    43,    44,    45,    46,    47,    57,     0,   450,     0,
			0,     0,     0,     0,    58,    59,    60,     0,     0,    61,
			62,     0,     0,    63,     0,     0,     0,    64,     0,     0,
			0,     0,     0,    65,     0,     0,    66,     0,     0,    67,
			0,     0,     0,    68,     0,     0,    69,     0,    70,     0,
			0,     0,     0,     0,     0,     0,    71,     0,     0,   559,
			0,    72,     0,    73,    74,    75,   560,    76,    77,    78,
			0,     0,    79,     0,     0,   561,     0,    80,     0,   562,
			0,   563,     0,     0,     0,     0,     0,     0,   564,     0,
			0,   565,     0,     0,     0,     0,     0,   566,     0,     0,
			567,     0,     0,     0,     0,    81,     0,     0,     0,     0,
			0,     0,   568,   569,     0,     0,   570,     0,     0,   571,
			0,     0,     0,     0,     0,     0,   572,   573,   574,     0,
			0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
			0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
			0,     0,     0,     0,     0,     0,     0,   575
		};

		/* YYCHECK.  */
		const short int
		ds9parse::yycheck_[] = {
			83,   321,    48,    96,   554,    98,    99,   100,   101,   102,
			103,    12,   332,   106,   107,   108,   109,   110,   111,     0,
			113,   114,   115,   116,   117,     5,   125,   130,     5,     3,
			4,   134,   131,     6,   131,    73,     5,   650,     5,     3,
			209,   133,   211,   212,     5,   125,    15,    16,   661,   401,
			663,   131,    73,   131,    15,    16,   149,   150,    21,   152,
			3,   154,    25,    26,   131,   417,    29,   160,   161,   546,
			3,     4,    35,     6,     7,   168,   428,   554,   127,    42,
			129,   174,   175,   176,   177,   178,   179,   180,   131,   182,
			128,   131,   191,    15,    16,    15,    16,   417,   130,   701,
			15,    16,   134,    15,    16,    48,   708,   128,   428,   461,
			19,    20,   211,   127,    85,   129,   618,   619,   620,   131,
			622,    92,    93,    66,    67,    68,    69,    68,    69,   209,
			39,   211,   212,    66,   611,     3,     4,   131,     6,     7,
			307,   142,   127,   131,   129,    88,   126,   314,   315,   316,
			127,   126,   129,   127,   127,   238,   239,   240,   127,   126,
			123,   244,   245,   246,   247,   248,   127,     3,   251,   336,
			253,   131,   255,   256,   257,   258,   259,   346,   131,   262,
			263,   124,   125,   266,   267,    15,    16,   270,   271,   272,
			273,   274,   794,   127,   546,   129,   127,   127,   129,   129,
			128,    44,   554,    44,   126,   127,   126,   129,   130,   129,
			130,   126,   127,   131,   126,   295,   766,   129,   130,   769,
			312,   723,    65,   725,    65,   131,   318,     3,     4,   131,
			322,   131,   399,   400,   326,    78,   131,    78,   405,   305,
			306,   408,   131,   131,     3,     4,   329,     6,     7,     8,
			9,    10,    11,    73,    73,   338,   339,   340,   341,   342,
			73,   344,    73,    30,    73,    32,    33,     3,     4,     3,
			6,     3,     8,     9,    10,    11,     5,   360,     5,     3,
			363,   364,   365,   366,   367,   368,   369,     3,   371,     3,
			129,     3,     5,   460,     5,   378,     5,     3,   381,   382,
			30,     3,    32,    33,     3,   388,   127,    66,    75,   392,
			393,     5,   130,    80,   397,     5,    11,    84,     6,   133,
			87,    10,    89,    14,   130,    12,   126,   419,     5,     3,
			66,     3,     8,   425,     9,     6,   131,   429,    11,   131,
			10,   131,    22,   512,   131,    75,   131,   131,   700,   701,
			80,   131,   131,   131,    84,   707,   708,   131,   131,    89,
			131,   131,   131,    43,   456,   132,   133,   131,   131,   536,
			453,    51,    52,   465,    54,   131,   543,    57,   131,     5,
			131,    61,   702,     3,     3,     3,   128,   470,     5,   709,
			473,    58,   475,    58,    58,     5,     5,     3,     3,     3,
			3,   484,   485,     5,   487,   504,   489,   453,     5,     3,
			3,   128,   495,   512,     5,     5,   499,   500,     5,     3,
			128,     3,     3,   515,   504,   594,   472,   596,   597,     5,
			3,     4,   512,     6,     7,     8,     9,    10,    11,   759,
			5,     5,   525,   135,     3,   765,   131,    25,     3,   135,
			542,     3,   804,   805,     3,   807,   808,   540,     3,    45,
			128,     3,   128,    48,   784,   128,   128,     3,     4,   789,
			6,   128,    87,   519,    10,    11,    12,    13,    14,     3,
			4,   580,     6,     7,     8,     9,    10,    11,   129,   303,
			129,   520,   304,   307,   540,   142,   310,   596,   312,   136,
			314,   315,   316,   317,   318,   384,   522,   321,   322,   511,
			122,   119,   326,   390,   594,   768,   596,   597,   332,   611,
			693,   717,   336,    -1,   185,    -1,   693,    -1,    -1,    -1,
			-1,    -1,   699,    -1,     3,     4,   703,     6,    -1,     8,
			9,    10,    11,    -1,    -1,    -1,   715,   630,    -1,    -1,
			-1,    -1,    -1,    -1,    -1,   638,   639,   640,   650,   642,
			643,    -1,   608,    -1,   647,   648,   649,    -1,    -1,   661,
			-1,   663,   618,   619,   620,    -1,   622,   623,    -1,    -1,
			-1,    -1,    -1,    -1,    -1,   399,   400,   401,    -1,   669,
			-1,   405,    -1,   760,   408,    -1,    -1,    -1,    -1,    -1,
			-1,   768,   416,   417,    -1,   419,    -1,    -1,    -1,    -1,
			-1,   425,    -1,    -1,   428,   429,     3,     4,    -1,     6,
			-1,     8,     9,    10,    11,   717,    -1,    -1,    -1,    -1,
			-1,    -1,    -1,    -1,    -1,    -1,    -1,   730,    -1,    -1,
			-1,    -1,   456,   689,    -1,   738,   460,   461,    -1,    -1,
			-1,   465,    -1,   736,   737,    -1,    -1,    -1,    -1,   742,
			-1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
			-1,    -1,    -1,    -1,    -1,    -1,    -1,   723,    -1,   725,
			-1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   772,
			-1,    -1,   775,   776,   777,    -1,    -1,   780,   781,   782,
			783,   515,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
			802,   803,    -1,    -1,   797,   798,    -1,   800,   801,    -1,
			-1,    -1,   536,    -1,    -1,    -1,    -1,    -1,   542,   543,
			-1,    -1,   546,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
			554,    -1,   148,    -1,    -1,   151,    -1,   153,    -1,   155,
			156,   157,   158,   159,    -1,    -1,   162,   163,    -1,   165,
			166,   167,    -1,   169,   170,   171,   172,   173,    -1,    -1,
			-1,    18,    -1,    -1,    21,    -1,    -1,    -1,    25,    26,
			27,    -1,    29,    30,    -1,    32,    33,    34,    35,    -1,
			-1,    -1,    -1,    -1,    -1,    42,    -1,   611,    45,    -1,
			47,    -1,    49,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
			-1,    -1,    -1,    -1,    -1,    -1,    63,    -1,    -1,    -1,
			-1,    -1,    -1,    -1,    71,    -1,    73,    74,    75,    -1,
			-1,    -1,    79,    80,    -1,    -1,   650,    84,    -1,    -1,
			-1,    -1,    89,   249,   250,    -1,   252,   661,   254,   663,
			-1,    -1,    -1,    -1,   260,   261,    -1,   268,    -1,    -1,
			-1,    -1,   273,    -1,    -1,    -1,    -1,    -1,    -1,   275,
			276,   277,   278,   279,   280,   281,   123,   283,    -1,   693,
			-1,    -1,    -1,    -1,    -1,   699,   700,   701,   702,   703,
			-1,    -1,    -1,   707,   708,   709,    -1,   308,   309,    -1,
			311,    -1,   313,   717,    -1,    -1,    -1,    -1,   319,   320,
			-1,    -1,    -1,    -1,    -1,   327,    -1,    -1,   329,   325,
			-1,    -1,    -1,   335,    -1,   331,   337,   333,    -1,    -1,
			-1,    -1,   343,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
			-1,    -1,    -1,    -1,    -1,   759,   760,    -1,    -1,   361,
			362,   765,   766,    -1,   768,   769,    -1,    -1,   370,    -1,
			372,    -1,   373,   374,   375,   376,    -1,   379,   380,    -1,
			784,    -1,    -1,    -1,    -1,   789,   387,    -1,    -1,   391,
			794,    -1,    -1,   394,   395,    -1,   398,    -1,   802,   803,
			804,   805,   404,   807,   808,    -1,   402,   403,    -1,    -1,
			389,    -1,    -1,    -1,    -1,    -1,    -1,   396,    -1,   421,
			422,   423,   424,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
			409,   410,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   418,
			-1,   420,    -1,    -1,    -1,    -1,    -1,   426,   427,    -1,
			-1,    -1,    -1,   455,    -1,    -1,    -1,    -1,    -1,    -1,
			462,    -1,    -1,    -1,    -1,    -1,   467,   468,   469,    -1,
			471,    -1,    -1,    -1,   475,    -1,    17,    -1,    19,    20,
			459,    -1,    -1,    -1,    -1,   486,    -1,   466,    -1,    -1,
			-1,    -1,    -1,    -1,    -1,    -1,    -1,   498,    39,    -1,
			41,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
			-1,    -1,   491,   492,   493,   494,    -1,    -1,    59,    -1,
			-1,    -1,    -1,    -1,   525,    -1,   527,   528,   530,    70,
			531,   533,   534,   535,    -1,   537,    -1,    -1,    -1,    -1,
			-1,    -1,   544,   545,    -1,   524,    -1,    -1,    -1,    -1,
			529,   553,    -1,    94,    95,    96,    97,    98,    99,   100,
			101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
			111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
			121,    94,    95,    96,    97,    98,    99,   100,   101,   102,
			103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
			113,   114,   115,   116,   117,   118,   119,   120,   121,    -1,
			612,   613,   614,    -1,    -1,   617,    -1,    -1,    -1,    -1,
			621,    -1,    -1,    -1,    -1,   626,    -1,    -1,   629,   630,
			-1,    -1,    -1,    -1,    -1,    -1,   637,   616,    -1,    -1,
			-1,    -1,    -1,    -1,    -1,   646,    -1,    -1,    -1,    -1,
			-1,    -1,    -1,   632,    -1,   634,    -1,    -1,    -1,    -1,
			-1,    -1,    -1,    -1,    -1,    -1,   645,    -1,    -1,    -1,
			-1,    -1,    -1,    -1,   675,   676,    -1,    -1,    -1,    -1,
			-1,    -1,    -1,    -1,   686,    -1,    -1,    -1,   690,    -1,
			692,    -1,   694,    -1,    -1,    -1,   698,    -1,    -1,   678,
			679,   680,    -1,   682,   706,    -1,    -1,    -1,    -1,    -1,
			-1,    -1,    -1,    -1,   716,    -1,   718,    -1,    -1,    -1,
			-1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
			-1,    -1,   733,   734,    -1,    -1,    -1,    -1,    -1,   740,
			741,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   727,    -1,
			729,    -1,   731,    -1,    -1,   757,   758,    -1,    -1,   755,
			739,   763,   764,    -1,    -1,   761,    -1,    -1,    -1,   748,
			-1,   750,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
			-1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
			-1,    -1,    -1,    -1,   773,   774,    -1,    -1,   799,   778,
			779,    -1,    -1,    -1,   806,    -1,    -1,    -1,     0,    -1,
			811,   812,    -1,   814,   815,    -1,    -1,   819,    -1,    -1,
			822,    -1,    -1,    15,    16,    17,    -1,    -1,    -1,    -1,
			22,    -1,    -1,    -1,   813,    -1,    -1,    -1,    -1,    -1,
			-1,    -1,    -1,    -1,   823,   824,    38,    -1,    -1,    41,
			-1,    43,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    51,
			52,    -1,    54,    55,    -1,    57,    -1,    59,    -1,    61,
			-1,    -1,    64,    -1,    -1,    -1,    -1,    -1,    70,    -1,
			-1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
			-1,    -1,    -1,    -1,    -1,    87,    -1,    -1,    90,    -1,
			-1,    -1,    94,    95,    96,    97,    98,    99,   100,   101,
			102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
			112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
			5,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
			-1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    23,    -1,
			-1,    -1,    27,    28,    -1,    -1,    31,    32,    33,    34,
			-1,    36,    37,    -1,    -1,    40,    -1,    -1,    -1,    44,
			-1,    -1,    47,    -1,    -1,    50,    -1,    -1,    53,    -1,
			-1,    56,    -1,    -1,    -1,    60,    -1,    -1,    63,    -1,
			65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    73,    -1,
			-1,    -1,    -1,    78,    -1,    80,    81,    82,    83,    84,
			85,    86,    18,    -1,    89,    21,    -1,    -1,    -1,    25,
			26,    27,    -1,    29,    30,    -1,    32,    33,    34,    35,
			-1,    -1,    -1,    -1,    -1,    -1,    42,    -1,    -1,    45,
			-1,    47,    -1,    49,    -1,    -1,    -1,   122,    -1,    15,
			16,    17,    -1,    -1,    -1,    -1,    22,    63,    -1,    -1,
			-1,    -1,    -1,    -1,    -1,    71,    -1,    73,    74,    75,
			-1,    -1,    38,    79,    80,    41,    -1,    43,    84,    -1,
			-1,    -1,    -1,    89,    -1,    51,    52,    -1,    54,    55,
			-1,    57,    -1,    59,    -1,    61,    -1,    -1,    64,    -1,
			-1,    -1,    -1,    -1,    70,    -1,    -1,    -1,    -1,    -1,
			-1,    -1,    -1,    -1,    -1,    -1,    -1,   123,    -1,    -1,
			126,    87,    -1,    -1,    90,    -1,   132,   133,    94,    95,
			96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
			106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
			116,   117,   118,   119,   120,   121,    17,    -1,    19,    20,
			-1,    22,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
			-1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    39,    -1,
			41,    -1,    43,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
			51,    52,    -1,    54,    -1,    -1,    57,    -1,    59,    -1,
			61,    -1,    -1,    64,    -1,    -1,    -1,    -1,    -1,    70,
			-1,    72,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
			-1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
			-1,    -1,    -1,    94,    95,    96,    97,    98,    99,   100,
			101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
			111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
			121,    17,    -1,    19,    20,    -1,    22,    -1,    -1,    -1,
			-1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
			-1,    -1,    -1,    39,    -1,    41,    -1,    43,    -1,    -1,
			-1,    -1,    -1,    -1,    -1,    51,    52,    -1,    54,    -1,
			-1,    57,    -1,    59,    -1,    61,    -1,    -1,    64,    -1,
			-1,    -1,    -1,    -1,    70,    -1,    72,    -1,    -1,    -1,
			-1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
			-1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    94,    95,
			96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
			106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
			116,   117,   118,   119,   120,   121,    17,    -1,    19,    20,
			-1,    22,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
			-1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    39,    -1,
			41,    -1,    43,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
			51,    52,    -1,    54,    -1,    -1,    57,    -1,    59,    -1,
			61,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    70,
			-1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
			-1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
			-1,    -1,    -1,    94,    95,    96,    97,    98,    99,   100,
			101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
			111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
			121,    17,    -1,    -1,    -1,    -1,    22,    -1,    -1,    -1,
			-1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
			-1,    -1,    -1,    -1,    -1,    41,    -1,    43,    -1,    -1,
			-1,    -1,    -1,    -1,    -1,    51,    52,    -1,    54,    -1,
			-1,    57,    -1,    59,    -1,    61,    -1,    -1,    64,    -1,
			-1,    -1,    -1,    -1,    70,    -1,    -1,    -1,    -1,    -1,
			-1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
			-1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    94,    95,
			96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
			106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
			116,   117,   118,   119,   120,   121,    17,    -1,    -1,    -1,
			-1,    22,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
			-1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
			41,    -1,    43,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
			51,    52,    -1,    54,    -1,    -1,    57,    -1,    59,    -1,
			61,    -1,    -1,    64,    -1,    -1,    -1,    -1,    -1,    70,
			-1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
			-1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
			-1,    -1,    -1,    94,    95,    96,    97,    98,    99,   100,
			101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
			111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
			121,    17,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    23,
			-1,    -1,    -1,    27,    28,    -1,    -1,    31,    32,    33,
			34,    -1,    36,    37,    -1,    41,    40,    -1,    -1,    -1,
			44,    -1,    -1,    47,    -1,    -1,    50,    -1,    -1,    53,
			-1,    -1,    56,    59,    -1,    -1,    60,    -1,    -1,    63,
			-1,    65,    -1,    -1,    70,    -1,    -1,    -1,    -1,    73,
			-1,    -1,    -1,    -1,    78,    -1,    80,    81,    82,    83,
			84,    85,    86,    -1,    -1,    89,    -1,    -1,    94,    95,
			96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
			106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
			116,   117,   118,   119,   120,   121,    23,    -1,   122,    -1,
			-1,    -1,    -1,    -1,    31,    32,    33,    -1,    -1,    36,
			37,    -1,    -1,    40,    -1,    -1,    -1,    44,    -1,    -1,
			-1,    -1,    -1,    50,    -1,    -1,    53,    -1,    -1,    56,
			-1,    -1,    -1,    60,    -1,    -1,    63,    -1,    65,    -1,
			-1,    -1,    -1,    -1,    -1,    -1,    73,    -1,    -1,    24,
			-1,    78,    -1,    80,    81,    82,    31,    84,    85,    86,
			-1,    -1,    89,    -1,    -1,    40,    -1,    94,    -1,    44,
			-1,    46,    -1,    -1,    -1,    -1,    -1,    -1,    53,    -1,
			-1,    56,    -1,    -1,    -1,    -1,    -1,    62,    -1,    -1,
			65,    -1,    -1,    -1,    -1,   122,    -1,    -1,    -1,    -1,
			-1,    -1,    77,    78,    -1,    -1,    81,    -1,    -1,    84,
			-1,    -1,    -1,    -1,    -1,    -1,    91,    92,    93,    -1,
			-1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
			-1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
			-1,    -1,    -1,    -1,    -1,    -1,    -1,   122
		};

		/* STOS_[STATE-NUM] -- The (internal number of the) accessing
		   symbol of state STATE-NUM.  */
		const unsigned char
		ds9parse::yystos_[] = {
			0,   137,   180,     0,    17,    22,    38,    41,    43,    51,
			52,    54,    55,    57,    59,    61,    64,    70,    87,    90,
			94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
			104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
			114,   115,   116,   117,   118,   119,   120,   121,   138,   139,
			170,   171,   172,   191,    68,    69,   155,    23,    31,    32,
			33,    36,    37,    40,    44,    50,    53,    56,    60,    63,
			65,    73,    78,    80,    81,    82,    84,    85,    86,    89,
			94,   122,   174,   176,   177,   142,     3,   139,   206,    15,
			16,   153,   140,   141,    18,    21,    25,    26,    27,    29,
			30,    32,    33,    34,    35,    42,    45,    47,    49,    63,
			71,    73,    74,    75,    79,    80,    84,    89,   123,   126,
			132,   133,   193,   195,   131,   131,   131,   131,   131,   131,
			131,   131,   131,   131,   131,   131,   131,   131,   131,   126,
			127,   145,   157,   145,   153,   145,   145,   128,   158,    73,
			73,   158,    73,   158,    73,   158,   158,   158,   158,   158,
			73,    73,   158,   158,   158,   158,   158,   158,   196,   158,
			158,   158,   158,   158,    73,    30,    32,    33,    75,    80,
			84,    87,    89,   143,   144,   193,   194,   195,     5,   126,
			64,   170,   172,   179,     3,     3,     5,     3,    21,    25,
			26,    29,    35,    42,   123,   192,    19,    20,    39,    64,
			72,   170,   172,   173,   178,     5,     3,     4,     6,     7,
			154,   162,     3,     3,   171,     3,     3,    48,    66,    67,
			68,    69,    88,   124,   125,   156,   146,   177,     6,    10,
			11,    12,    13,    14,   154,   166,   167,   168,   169,   158,
			158,   169,   158,   169,   158,   169,   169,   169,   169,   169,
			158,   158,   169,   169,   129,   159,   169,   169,   169,   158,
			169,   169,   169,   169,   169,   158,   158,   158,   158,   158,
			158,   158,     3,   158,     5,   194,     5,   172,     5,     3,
			3,     3,   170,   173,   170,   172,   173,   170,   173,     5,
			157,   157,   157,   157,   157,   157,   157,   157,   169,   169,
			157,   169,   157,   169,   157,   157,   157,   157,   157,   169,
			169,   157,   157,   130,   160,   157,   157,   159,   169,   198,
			199,   157,   157,   157,   157,   159,   157,   169,   169,   169,
			169,   169,   169,   169,   169,     5,   170,     6,    10,    11,
			154,   166,   168,   168,     6,     8,     9,    10,    11,   154,
			163,   159,   159,     6,     8,     9,    10,    11,   154,   164,
			159,   162,   159,   163,   163,   163,   161,   162,   162,   159,
			159,   164,   162,   130,   126,   147,   206,   169,   162,   160,
			157,   159,   169,   164,   169,     5,   160,   163,   159,   157,
			157,   157,   157,   157,   159,   157,     3,   173,   157,   160,
			160,   157,   157,   157,   157,   157,   157,   157,   160,   157,
			160,   159,   159,   159,   159,   157,   160,   160,   157,   157,
			23,    27,    28,    31,    32,    33,    34,    36,    37,    47,
			53,    63,    73,    80,    82,    83,    84,    85,    86,    89,
			122,   148,   174,   181,   182,   159,   157,   147,   199,   160,
			157,   157,   159,   159,   147,   157,   160,   163,   163,   161,
			169,   169,   160,   163,     3,   163,   147,   147,     6,     8,
			9,    10,    11,   154,     6,   154,   161,   164,   147,   162,
			147,   160,   160,   160,   160,   162,   147,   147,   161,   164,
			162,   131,   131,   131,   131,   131,   131,   131,   131,   131,
			131,   131,   131,   131,   131,   131,   131,   131,   131,     5,
			131,   149,   157,   206,   160,   162,   147,   163,   161,   160,
			197,   162,   147,   159,   159,   159,   157,   159,   150,   151,
			181,   206,   157,   157,   159,   159,   157,   157,   147,   147,
			147,   147,   157,   159,   157,   157,    58,   189,   190,    24,
			31,    40,    44,    46,    53,    56,    62,    65,    77,    78,
			81,    84,    91,    92,    93,   122,   175,     5,   126,    64,
			170,   172,   184,     3,    58,   185,   186,     3,    58,   187,
			188,     5,     3,   192,    64,    72,   170,   172,   173,   183,
			5,     5,   162,     3,     3,     3,   206,   156,     5,   182,
			147,   157,   159,   159,   159,   147,   160,   159,   160,   160,
			160,   163,   160,     5,   152,   206,   162,    66,   163,   165,
			200,   201,   160,   134,   160,     6,   154,   161,   163,   165,
			204,   205,     3,     3,   134,   160,   161,   165,   204,     3,
			128,    44,    65,    78,    44,    65,    78,     5,     5,   172,
			5,   128,     3,   128,     3,     3,   170,   173,   170,   172,
			173,   170,   173,   206,   162,   165,   202,   203,   160,   160,
			160,   147,   160,   150,   150,   150,   159,   150,   206,     5,
			159,   131,   159,   157,   159,   147,   135,   147,   159,   157,
			157,   157,   157,   157,   135,   147,   159,   157,   157,   157,
			202,     5,     5,   202,   202,   170,   159,   157,   159,   147,
			147,   147,   147,   160,   206,   160,     3,   160,   201,   160,
			25,   160,   163,   161,   161,   205,   164,   163,    45,   160,
			161,   161,   164,   129,     3,   129,   129,   173,   160,   203,
			160,   150,   150,   147,   147,   158,   147,   159,   159,   157,
			157,   158,   147,   159,   159,   157,   128,     3,   128,   128,
			147,   147,   169,   160,   160,   164,   163,   169,   160,   160,
			164,   204,   200,   204,   157,   147,   147,   157,   157,   157,
			147,   147,   157,   129,   157,   129,   129,   164,     3,     3,
			164,     3,   128,   128,   157,   157,   159,   157,   157,   162,
			162,   161,   161,   160,   161,   161,   129,   129,   159,   159,
			147,   159,   159,   160,   160,   147,   147
		};

#if YYDEBUG
		/* TOKEN_NUMBER_[YYLEX-NUM] -- Internal symbol number corresponding
		   to YYLEX-NUM.  */
		const unsigned short int
		ds9parse::yytoken_number_[] = {
			0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
			265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
			275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
			285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
			295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
			305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
			315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
			325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
			335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
			345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
			355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
			365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
			375,   376,   377,   378,   379,   380,    35,    44,    40,    41,
			124,    61,    43,    45,    38,    33
		};
#endif

		/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
		const unsigned char
		ds9parse::yyr1_[] = {
			0,   136,   137,   138,   138,   139,   139,   139,   139,   139,
			140,   139,   141,   139,   142,   139,   139,   139,   139,   143,
			143,   143,   144,   143,   145,   146,   145,   147,   148,   147,
			147,   149,   147,   150,   151,   150,   150,   152,   150,   153,
			153,   154,   154,   155,   155,   156,   156,   156,   156,   156,
			156,   156,   156,   156,   157,   157,   158,   158,   159,   159,
			160,   160,   160,   161,   161,   162,   162,   162,   163,   163,
			163,   163,   163,   163,   164,   164,   164,   164,   164,   164,
			165,   166,   167,   168,   169,   169,   169,   169,   169,   169,
			169,   170,   170,   170,   170,   170,   171,   171,   171,   171,
			171,   171,   171,   171,   171,   171,   171,   171,   171,   171,
			171,   171,   171,   171,   171,   171,   171,   171,   171,   171,
			171,   171,   171,   171,   172,   172,   172,   172,   172,   172,
			172,   173,   173,   173,   174,   174,   174,   174,   174,   174,
			174,   174,   174,   174,   175,   175,   175,   175,   175,   175,
			175,   175,   175,   175,   175,   175,   175,   175,   175,   175,
			175,   175,   175,   175,   175,   176,   176,   177,   177,   177,
			177,   177,   177,   177,   177,   177,   177,   177,   177,   177,
			177,   177,   177,   177,   177,   177,   177,   178,   178,   178,
			178,   178,   178,   178,   178,   178,   179,   179,   179,   179,
			180,   181,   181,   182,   182,   182,   182,   182,   182,   182,
			182,   182,   182,   182,   182,   182,   182,   182,   182,   182,
			182,   182,   182,   182,   182,   182,   182,   183,   183,   183,
			183,   183,   183,   183,   183,   183,   184,   184,   184,   184,
			186,   185,   185,   188,   187,   187,   190,   189,   189,   191,
			192,   192,   192,   192,   192,   192,   192,   193,   193,   194,
			194,   194,   194,   194,   194,   194,   195,   195,   195,   195,
			195,   195,   195,   195,   195,   195,   195,   195,   195,   195,
			195,   195,   195,   195,   195,   195,   195,   195,   195,   195,
			195,   195,   195,   196,   195,   195,   195,   195,   195,   195,
			197,   195,   195,   195,   195,   195,   195,   195,   198,   198,
			199,   200,   200,   201,   202,   202,   203,   204,   204,   205,
			206
		};

		/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
		const unsigned char
		ds9parse::yyr2_[] = {
			0,     2,     3,     3,     2,     0,     2,     1,     3,     2,
			0,     3,     0,     3,     0,     3,     2,     3,     3,     1,
			2,     2,     0,     2,     0,     0,     3,     1,     0,     4,
			3,     0,     5,     1,     0,     3,     2,     0,     4,     1,
			1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
			1,     1,     1,     1,     0,     1,     0,     1,     0,     1,
			0,     1,     2,     0,     1,     1,     1,     1,     1,     1,
			1,     1,     1,     1,     3,     3,     3,     3,     3,     3,
			3,     1,     1,     1,     3,     3,     3,     3,     3,     3,
			3,     1,     1,     1,     1,     1,     1,     1,     1,     1,
			1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
			1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
			1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
			1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
			1,     1,     1,     1,     1,     1,     1,     1,     2,     1,
			2,     2,     1,     2,     2,     1,     2,     1,     1,     1,
			1,     1,     1,     1,     1,     3,     1,     3,     3,     4,
			4,     3,     3,     3,     1,     1,     1,     3,     4,     4,
			3,     3,     3,     7,     3,     3,     3,     4,     2,     2,
			2,     2,     2,     2,     1,     1,     2,     1,     1,     1,
			0,     3,     1,     3,     3,     4,     4,     3,     3,     3,
			3,     5,     1,     1,     1,     3,     4,     4,     3,     3,
			3,     7,     3,     3,     3,     3,     3,     4,     2,     2,
			2,     2,     2,     2,     1,     1,     2,     1,     1,     1,
			0,     7,     1,     0,    10,     1,     0,    10,     1,     0,
			1,     1,     1,     1,     1,     1,     1,     1,     1,     6,
			8,    10,    10,     8,     8,     8,     8,    10,    12,    12,
			18,    10,    12,    14,    14,    18,    20,    10,    10,    12,
			14,    14,    18,    20,     8,     6,     7,     7,     7,     7,
			7,     7,     7,     0,     7,    10,    12,    12,     5,     6,
			0,     9,     8,    10,    10,     8,     8,     8,     3,     1,
			1,     3,     1,     1,     3,     1,     1,     3,     1,     3,
			0
		};


		/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
		   First, the terminals, then, starting at \a yyntokens_, nonterminals.  */
		const char*
		const ds9parse::yytname_[] = {
			"$end", "error", "$undefined", "INT", "REAL", "STRING", "ANGDEGREE",
			"ANGRADIAN", "ARCMINUTE", "ARCSECOND", "PHYCOORD", "IMGCOORD", "SEXSTR",
			"HMSSTR", "DMSSTR", "EOF_", "EOL_", "AMPLIFIER_", "ANNULUS_", "ARCMIN_",
			"ARCSEC_", "ARROW_", "B1950_", "BACKGROUND_", "BEGIN_", "BOX_",
			"BOXCIRCLE_", "BPANDA_", "CALLBACK_", "CIRCLE_", "CIRCLE3D_", "COLOR_",
			"COMPASS_", "COMPOSITE_", "CPANDA_", "CROSS_", "DASH_", "DASHLIST_",
			"DEBUG_", "DEGREES_", "DELETE_", "DETECTOR_", "DIAMOND_", "ECLIPTIC_",
			"EDIT_", "ELLIPSE_", "END_", "EPANDA_", "FALSE_", "FIELD_", "FIXED_",
			"FK4_", "FK5_", "FONT_", "GALACTIC_", "GLOBAL_", "HIGHLITE_", "ICRS_",
			"IGNORE_", "IMAGE_", "INCLUDE_", "J2000_", "KEY_", "LINE_", "LINEAR_",
			"MOVE_", "N_", "NO_", "OFF_", "ON_", "PHYSICAL_", "PIE_", "PIXELS_",
			"POINT_", "POLYGON_", "PROJECTION_", "PROJECTION3D_", "PROPERTY_",
			"ROTATE_", "ROTBOX_", "RULER_", "SELECT_", "SOURCE_", "TAG_", "TEXT_",
			"TEXTANGLE_", "TEXTROTATE_", "TILE_", "TRUE_", "VECTOR_", "VERSION_",
			"UNHIGHLITE_", "UNSELECT_", "UPDATE_", "WCS_", "WCSA_", "WCSB_", "WCSC_",
			"WCSD_", "WCSE_", "WCSF_", "WCSG_", "WCSH_", "WCSI_", "WCSJ_", "WCSK_",
			"WCSL_", "WCSM_", "WCSN_", "WCSO_", "WCSP_", "WCSQ_", "WCSR_", "WCSS_",
			"WCST_", "WCSU_", "WCSV_", "WCSW_", "WCSX_", "WCSY_", "WCSZ_", "WCS0_",
			"WIDTH_", "X_", "Y_", "YES_", "'#'", "','", "'('", "')'", "'|'", "'='",
			"'+'", "'-'", "'&'", "'!'", "$accept", "start", "commands", "command",
			"$@1", "$@2", "$@3", "hash", "$@4", "comment", "$@5", "shapeComment",
			"$@6", "$@7", "nonshapeComment", "$@8", "$@9", "terminator", "numeric",
			"debug", "yesno", "sp", "bp", "ep", "conjuction", "optangle", "angle",
			"value", "vvalue", "numberof", "sexagesimal", "hms", "dms", "coord",
			"coordSystem", "wcsSystem", "skyFrame", "skyFormat", "property",
			"callBack", "global", "globalProperty", "globalRuler", "globalCompass",
			"initGlobal", "local", "localProperty", "localRuler", "localCompass",
			"localCpanda", "$@10", "localEpanda", "$@11", "localBpanda", "$@12",
			"initLocal", "pointProp", "include", "nonshape", "shape", "$@13", "$@14",
			"polyNodes", "polyNode", "aRads", "aRad", "aAngs", "aAng", "vRads",
			"vRad", "postLocal", YY_NULL
		};

#if YYDEBUG
		/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
		const ds9parse::rhs_number_type
		ds9parse::yyrhs_[] = {
			137,     0,    -1,   180,   138,   206,    -1,   138,   139,   153,
			-1,   139,   153,    -1,    -1,    38,   155,    -1,    90,    -1,
			55,   176,   145,    -1,    87,     3,    -1,    -1,   170,   140,
			145,    -1,    -1,   172,   141,   145,    -1,    -1,    64,   142,
			145,    -1,   191,   195,    -1,   191,   193,   195,    -1,   191,
			126,   143,    -1,   194,    -1,   193,   194,    -1,    87,     3,
			-1,    -1,   144,     5,    -1,    -1,    -1,   126,   146,     5,
			-1,   206,    -1,    -1,   126,   148,     5,   206,    -1,   126,
			181,   206,    -1,    -1,   126,   181,   149,     5,   206,    -1,
			206,    -1,    -1,   151,     5,   206,    -1,   181,   206,    -1,
			-1,   181,   152,     5,   206,    -1,    16,    -1,    15,    -1,
			4,    -1,     3,    -1,    69,    -1,    68,    -1,     3,    -1,
			125,    -1,   124,    -1,    69,    -1,    88,    -1,    67,    -1,
			66,    -1,    68,    -1,    48,    -1,    -1,   127,    -1,    -1,
			128,    -1,    -1,   129,    -1,    -1,   130,    -1,   130,   130,
			-1,    -1,   162,    -1,   154,    -1,     6,    -1,     7,    -1,
			154,    -1,    10,    -1,    11,    -1,     6,    -1,     8,    -1,
			9,    -1,   154,   157,   154,    -1,    10,   157,    10,    -1,
			11,   157,    11,    -1,     6,   157,     6,    -1,     8,   157,
			8,    -1,     9,   157,     9,    -1,    66,   131,     3,    -1,
			12,    -1,    13,    -1,    14,    -1,   166,   157,   166,    -1,
			167,   157,   168,    -1,   168,   157,   168,    -1,   154,   157,
			154,    -1,     6,   157,     6,    -1,    11,   157,    11,    -1,
			10,   157,    10,    -1,    59,    -1,    70,    -1,    41,    -1,
			17,    -1,   171,    -1,    94,    -1,    95,    -1,    96,    -1,
			97,    -1,    98,    -1,    99,    -1,   100,    -1,   101,    -1,
			102,    -1,   103,    -1,   104,    -1,   105,    -1,   106,    -1,
			107,    -1,   108,    -1,   109,    -1,   110,    -1,   111,    -1,
			112,    -1,   113,    -1,   114,    -1,   115,    -1,   116,    -1,
			117,    -1,   118,    -1,   119,    -1,   120,    -1,   121,    -1,
			51,    -1,    22,    -1,    52,    -1,    61,    -1,    57,    -1,
			54,    -1,    43,    -1,    39,    -1,    19,    -1,    20,    -1,
			81,    -1,    56,    -1,    36,    -1,    50,    -1,    44,    -1,
			65,    -1,    78,    -1,    40,    -1,    60,    -1,    82,    -1,
			81,    -1,    92,    -1,    56,    -1,    91,    -1,    24,    65,
			-1,    65,    -1,    46,    65,    -1,    24,    44,    -1,    44,
			-1,    46,    44,    -1,    24,    78,    -1,    78,    -1,    46,
			78,    -1,    40,    -1,    84,    -1,    31,    -1,   122,    -1,
			77,    -1,    53,    -1,    62,    -1,    93,    -1,   176,   157,
			177,    -1,   177,    -1,   174,   131,   156,    -1,    31,   131,
			5,    -1,    31,   131,   126,     5,    -1,    37,   131,     3,
			3,    -1,   122,   131,     3,    -1,    53,   131,     5,    -1,
			84,   131,     5,    -1,    36,    -1,    82,    -1,    23,    -1,
			73,   131,   192,    -1,    73,   131,   192,     3,    -1,    63,
			131,     3,     3,    -1,    89,   131,     3,    -1,    33,   131,
			3,    -1,    80,   131,   178,    -1,    32,   131,   179,     5,
			5,     3,     3,    -1,    85,   131,   162,    -1,    86,   131,
			3,    -1,    94,   131,   171,    -1,   170,   172,   170,   173,
			-1,   170,   170,    -1,   170,   173,    -1,   172,   170,    -1,
			172,   173,    -1,    64,   170,    -1,    64,   173,    -1,   173,
			-1,    72,    -1,   170,   172,    -1,   170,    -1,   172,    -1,
			64,    -1,    -1,   181,   157,   182,    -1,   182,    -1,   174,
			131,   156,    -1,    31,   131,     5,    -1,    31,   131,   126,
			5,    -1,    37,   131,     3,     3,    -1,   122,   131,     3,
			-1,    53,   131,     5,    -1,    84,   131,     5,    -1,    83,
			131,     5,    -1,    28,   131,   175,     5,     5,    -1,    36,
			-1,    82,    -1,    23,    -1,    73,   131,   192,    -1,    73,
			131,   192,     3,    -1,    63,   131,     3,     3,    -1,    89,
			131,     3,    -1,    33,   131,     3,    -1,    80,   131,   183,
			-1,    32,   131,   184,     5,     5,     3,     3,    -1,    85,
			131,   162,    -1,    86,   131,     3,    -1,    34,   131,   185,
			-1,    47,   131,   187,    -1,    27,   131,   189,    -1,   170,
			172,   170,   173,    -1,   170,   170,    -1,   170,   173,    -1,
			172,   170,    -1,   172,   173,    -1,    64,   170,    -1,    64,
			173,    -1,   173,    -1,    72,    -1,   170,   172,    -1,   170,
			-1,   172,    -1,    64,    -1,    -1,   186,   128,   202,   129,
			128,   200,   129,    -1,    58,    -1,    -1,   188,   128,   202,
			129,   128,   204,   129,   128,   162,   129,    -1,    58,    -1,
			-1,   190,   128,   202,   129,   128,   204,   129,   128,   162,
			129,    -1,    58,    -1,    -1,    29,    -1,    25,    -1,    42,
			-1,    35,    -1,   123,    -1,    21,    -1,    26,    -1,   132,
			-1,   133,    -1,    84,   158,   169,   159,   160,   150,    -1,
			33,   158,   169,   157,   161,   159,   160,   150,    -1,    89,
			158,   169,   157,   163,   157,   162,   159,   160,   150,    -1,
			75,   158,   169,   157,   169,   157,   163,   159,   160,   150,
			-1,    80,   158,   169,   157,   169,   159,   160,   150,    -1,
			32,   158,   169,   157,   163,   159,   160,   150,    -1,    30,
			158,   169,   157,   163,   159,   160,   150,    -1,    29,   158,
			169,   157,   163,   159,   160,   147,    -1,    18,   158,   169,
			157,   163,   157,   163,   159,   160,   147,    -1,    18,   158,
			169,   157,   163,   157,   163,   157,   200,   159,   160,   147,
			-1,    18,   158,   169,   157,   163,   157,   163,   157,   165,
			159,   160,   147,    -1,    34,   158,   169,   157,   162,   157,
			162,   157,     3,   157,   163,   157,   163,   157,     3,   159,
			160,   147,    -1,    45,   158,   169,   157,   164,   157,   161,
			159,   160,   147,    -1,    45,   158,   169,   157,   164,   157,
			164,   157,   161,   159,   160,   147,    -1,    45,   158,   169,
			157,   164,   157,   164,   157,   165,   157,   161,   159,   160,
			147,    -1,    45,   158,   169,   157,   164,   157,   164,   157,
			204,   157,   161,   159,   160,   147,    -1,    45,   158,   169,
			157,   164,   157,   161,   159,   134,   135,    45,   158,   169,
			157,   164,   157,   161,   159,    -1,    47,   158,   169,   157,
			162,   157,   162,   157,     3,   157,   164,   157,   164,   157,
			3,   157,   161,   159,   160,   147,    -1,    25,   158,   169,
			157,   164,   157,   161,   159,   160,   147,    -1,    79,   158,
			169,   157,   164,   157,   161,   159,   160,   147,    -1,    25,
			158,   169,   157,   164,   157,   164,   157,   161,   159,   160,
			147,    -1,    25,   158,   169,   157,   164,   157,   164,   157,
			204,   157,   161,   159,   160,   147,    -1,    25,   158,   169,
			157,   164,   157,   164,   157,   165,   157,   161,   159,   160,
			147,    -1,    25,   158,   169,   157,   164,   157,   161,   159,
			134,   135,    25,   158,   169,   157,   164,   157,   161,   159,
			-1,    27,   158,   169,   157,   162,   157,   162,   157,     3,
			157,   164,   157,   164,   157,     3,   157,   161,   159,   160,
			147,    -1,    63,   158,   169,   157,   169,   159,   160,   147,
			-1,    73,   158,   169,   159,   160,   147,    -1,    29,    73,
			158,   169,   159,   160,   147,    -1,    25,    73,   158,   169,
			159,   160,   147,    -1,    42,    73,   158,   169,   159,   160,
			147,    -1,    35,    73,   158,   169,   159,   160,   147,    -1,
			123,    73,   158,   169,   159,   160,   147,    -1,    21,    73,
			158,   169,   159,   160,   147,    -1,    26,    73,   158,   169,
			159,   160,   147,    -1,    -1,    74,   196,   158,   198,   159,
			160,   147,    -1,    71,   158,   169,   157,   162,   157,   162,
			159,   160,   147,    -1,    71,   158,   169,   157,   162,   157,
			162,   157,   202,   159,   160,   147,    -1,    71,   158,   169,
			157,   162,   157,   162,   157,   165,   159,   160,   147,    -1,
			49,   158,   159,   160,   147,    -1,    84,   158,   169,   159,
			160,   147,    -1,    -1,    84,   158,   169,   157,     5,   159,
			197,   160,   147,    -1,    33,   158,   169,   157,   161,   159,
			160,   147,    -1,    89,   158,   169,   157,   163,   157,   162,
			159,   160,   147,    -1,    75,   158,   169,   157,   169,   157,
			163,   159,   160,   147,    -1,    80,   158,   169,   157,   169,
			159,   160,   147,    -1,    32,   158,   169,   157,   163,   159,
			160,   147,    -1,    30,   158,   169,   157,   163,   159,   160,
			147,    -1,   198,   157,   199,    -1,   199,    -1,   169,    -1,
			200,   157,   201,    -1,   201,    -1,   163,    -1,   202,   157,
			203,    -1,   203,    -1,   162,    -1,   204,   157,   205,    -1,
			205,    -1,   163,   157,   163,    -1,    -1
		};

		/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
		   YYRHS.  */
		const unsigned short int
		ds9parse::yyprhs_[] = {
			0,     0,     3,     7,    11,    14,    15,    18,    20,    24,
			27,    28,    32,    33,    37,    38,    42,    45,    49,    53,
			55,    58,    61,    62,    65,    66,    67,    71,    73,    74,
			79,    83,    84,    90,    92,    93,    97,   100,   101,   106,
			108,   110,   112,   114,   116,   118,   120,   122,   124,   126,
			128,   130,   132,   134,   136,   137,   139,   140,   142,   143,
			145,   146,   148,   151,   152,   154,   156,   158,   160,   162,
			164,   166,   168,   170,   172,   176,   180,   184,   188,   192,
			196,   200,   202,   204,   206,   210,   214,   218,   222,   226,
			230,   234,   236,   238,   240,   242,   244,   246,   248,   250,
			252,   254,   256,   258,   260,   262,   264,   266,   268,   270,
			272,   274,   276,   278,   280,   282,   284,   286,   288,   290,
			292,   294,   296,   298,   300,   302,   304,   306,   308,   310,
			312,   314,   316,   318,   320,   322,   324,   326,   328,   330,
			332,   334,   336,   338,   340,   342,   344,   346,   348,   351,
			353,   356,   359,   361,   364,   367,   369,   372,   374,   376,
			378,   380,   382,   384,   386,   388,   392,   394,   398,   402,
			407,   412,   416,   420,   424,   426,   428,   430,   434,   439,
			444,   448,   452,   456,   464,   468,   472,   476,   481,   484,
			487,   490,   493,   496,   499,   501,   503,   506,   508,   510,
			512,   513,   517,   519,   523,   527,   532,   537,   541,   545,
			549,   553,   559,   561,   563,   565,   569,   574,   579,   583,
			587,   591,   599,   603,   607,   611,   615,   619,   624,   627,
			630,   633,   636,   639,   642,   644,   646,   649,   651,   653,
			655,   656,   664,   666,   667,   678,   680,   681,   692,   694,
			695,   697,   699,   701,   703,   705,   707,   709,   711,   713,
			720,   729,   740,   751,   760,   769,   778,   787,   798,   811,
			824,   843,   854,   867,   882,   897,   916,   937,   948,   959,
			972,   987,  1002,  1021,  1042,  1051,  1058,  1066,  1074,  1082,
			1090,  1098,  1106,  1114,  1115,  1123,  1134,  1147,  1160,  1166,
			1173,  1174,  1184,  1193,  1204,  1215,  1224,  1233,  1242,  1246,
			1248,  1250,  1254,  1256,  1258,  1262,  1264,  1266,  1270,  1272,
			1276
		};

		/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
		const unsigned short int
		ds9parse::yyrline_[] = {
			0,   339,   339,   342,   343,   346,   347,   348,   350,   351,
			353,   353,   354,   354,   355,   355,   357,   358,   359,   362,
			363,   364,   365,   365,   368,   369,   369,   372,   373,   373,
			374,   375,   375,   379,   380,   380,   381,   382,   382,   385,
			386,   389,   390,   393,   394,   397,   399,   400,   401,   402,
			404,   405,   406,   407,   410,   411,   414,   415,   418,   419,
			422,   423,   424,   427,   428,   431,   432,   433,   436,   437,
			438,   439,   440,   441,   444,   451,   458,   465,   472,   479,
			488,   491,   494,   497,   500,   513,   521,   529,   536,   544,
			551,   559,   560,   561,   562,   563,   566,   567,   568,   569,
			570,   571,   572,   573,   574,   575,   576,   577,   578,   579,
			580,   581,   582,   583,   584,   585,   586,   587,   588,   589,
			590,   591,   592,   593,   596,   597,   598,   599,   600,   601,
			602,   605,   606,   607,   610,   611,   612,   613,   614,   615,
			616,   617,   618,   619,   622,   623,   624,   625,   626,   627,
			628,   629,   630,   631,   632,   633,   634,   635,   636,   637,
			638,   639,   640,   641,   642,   645,   646,   649,   654,   659,
			664,   669,   670,   675,   680,   685,   690,   695,   696,   701,
			706,   707,   711,   712,   721,   722,   723,   726,   733,   740,
			747,   754,   761,   768,   775,   782,   791,   796,   801,   806,
			813,   855,   856,   859,   860,   861,   865,   870,   871,   872,
			873,   874,   875,   876,   877,   879,   880,   881,   882,   883,
			884,   885,   892,   893,   894,   895,   896,   899,   906,   913,
			920,   927,   934,   941,   948,   955,   964,   969,   974,   979,
			986,   986,   987,   990,   990,   992,   995,   995,   997,  1000,
			1045,  1046,  1047,  1048,  1049,  1050,  1051,  1054,  1055,  1058,
			1063,  1068,  1074,  1084,  1091,  1099,  1110,  1115,  1120,  1130,
			1136,  1159,  1182,  1192,  1202,  1215,  1222,  1247,  1270,  1276,
			1286,  1298,  1309,  1316,  1341,  1348,  1388,  1392,  1396,  1400,
			1404,  1408,  1412,  1417,  1417,  1423,  1424,  1425,  1427,  1429,
			1434,  1434,  1440,  1445,  1451,  1461,  1468,  1476,  1487,  1488,
			1491,  1494,  1495,  1498,  1505,  1506,  1509,  1516,  1517,  1520,
			1524
		};

		// Print the state stack on the debug stream.
		void
		ds9parse::yystack_print_ () {
			*yycdebug_ << "Stack now";
			for (state_stack_type::const_iterator i = yystate_stack_.begin ();
			        i != yystate_stack_.end (); ++i)
				*yycdebug_ << ' ' << *i;
			*yycdebug_ << std::endl;
		}

		// Report on the debug stream that the rule \a yyrule is going to be reduced.
		void
		ds9parse::yy_reduce_print_ (int yyrule) {
			unsigned int yylno = yyrline_[yyrule];
			int yynrhs = yyr2_[yyrule];
			/* Print the symbols being reduced, and their result.  */
			*yycdebug_ << "Reducing stack by rule " << yyrule - 1
			           << " (line " << yylno << "):" << std::endl;
			/* The symbols being reduced.  */
			for (int yyi = 0; yyi < yynrhs; yyi++)
				YY_SYMBOL_PRINT ("   $" << yyi + 1 << " =",
				                 yyrhs_[yyprhs_[yyrule] + yyi],
				                 &(yysemantic_stack_[(yynrhs) - (yyi + 1)]),
				                 &(yylocation_stack_[(yynrhs) - (yyi + 1)]));
		}
#endif // YYDEBUG

		/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
		ds9parse::token_number_type
		ds9parse::yytranslate_ (int t) {
			static
			const token_number_type
			translate_table[] = {
				0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
				2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
				2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
				2,     2,     2,   135,     2,   126,     2,     2,   134,     2,
				128,   129,     2,   132,   127,   133,     2,     2,     2,     2,
				2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
				2,   131,     2,     2,     2,     2,     2,     2,     2,     2,
				2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
				2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
				2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
				2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
				2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
				2,     2,     2,     2,   130,     2,     2,     2,     2,     2,
				2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
				2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
				2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
				2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
				2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
				2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
				2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
				2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
				2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
				2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
				2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
				2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
				2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
				5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
				15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
				25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
				35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
				45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
				55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
				65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
				75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
				85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
				95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
				105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
				115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
				125
			};
			if ((unsigned int) t <= yyuser_token_number_max_)
				return translate_table[t];
			else
				return yyundef_token_;
		}

		const int ds9parse::yyeof_ = 0;
		const int ds9parse::yylast_ = 2527;
		const int ds9parse::yynnts_ = 71;
		const int ds9parse::yyempty_ = -2;
		const int ds9parse::yyfinal_ = 3;
		const int ds9parse::yyterror_ = 1;
		const int ds9parse::yyerrcode_ = 256;
		const int ds9parse::yyntokens_ = 136;

		const unsigned int ds9parse::yyuser_token_number_max_ = 380;
		const ds9parse::token_number_type ds9parse::yyundef_token_ = 2;


	}
} // casa::viewer

/* Line 1108 of lalr1.cc  */
#line 4719 "/Users/drs/develop/casa/code/build/display/ds9parse.cc"

/* Line 1109 of lalr1.cc  */
#line 1551 "ds9/ds9parse.yy"


static void setProps(unsigned short* props, unsigned short prop, int value) {
	if (value)
		*props |= prop;
	else
		*props &= ~prop;
}

static casa::viewer::CoordSystem checkWCSSystem() {
	switch (localSystem) {
	case casa::viewer::IMAGE:
	case casa::viewer::PHYSICAL:
		return casa::viewer::WCS;
	default:
		return localSystem;
	}
}

static casa::viewer::SkyFrame checkWCSSky() {
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
void casa::viewer::ds9parse::error( const ds9parse::location_type &l, const std::string &m ) {
	driver.error(l,m);
}
//------------------------------------------------------------------------------------------

