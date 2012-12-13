/* A Bison parser, made by GNU Bison 2.6.  */

/* Skeleton interface for Bison LALR(1) parsers in C++
   
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

/**
 ** \file /Users/drs/develop/casa/code/build/display/ds9parse.hh
 ** Define the casa::viewer::parser class.
 */

/* C++ LALR(1) parser skeleton written by Akim Demaille.  */

#ifndef CASA_VIEWER_USERS_DRS_DEVELOP_CASA_CODE_BUILD_DISPLAY_DS9PARSE_HH
# define CASA_VIEWER_USERS_DRS_DEVELOP_CASA_CODE_BUILD_DISPLAY_DS9PARSE_HH



#include <string>
#include <iostream>
#include "stack.hh"
#include "location.hh"

/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif


namespace casa { namespace viewer {

/* Line 36 of lalr1.cc  */
#line 60 "/Users/drs/develop/casa/code/build/display/ds9parse.hh"

  /// A Bison parser.
  class ds9parse
  {
  public:
    /// Symbol semantic values.
#ifndef YYSTYPE
    union semantic_type
    {

/* Line 36 of lalr1.cc  */
#line 171 "ds9/ds9parse.yy"

#define MKBUFSIZE 2048
  double real;
  int integer;
  char str[MKBUFSIZE];
  double vector[3];



/* Line 36 of lalr1.cc  */
#line 83 "/Users/drs/develop/casa/code/build/display/ds9parse.hh"
    };
#else
    typedef YYSTYPE semantic_type;
#endif
    /// Symbol locations.
    typedef location location_type;
    /// Tokens.
    struct token
    {
      /* Tokens.  */
   enum yytokentype {
     INT = 258,
     REAL = 259,
     STRING = 260,
     ANGDEGREE = 261,
     ANGRADIAN = 262,
     ARCMINUTE = 263,
     ARCSECOND = 264,
     PHYCOORD = 265,
     IMGCOORD = 266,
     SEXSTR = 267,
     HMSSTR = 268,
     DMSSTR = 269,
     EOF_ = 270,
     EOL_ = 271,
     AMPLIFIER_ = 272,
     ANNULUS_ = 273,
     ARCMIN_ = 274,
     ARCSEC_ = 275,
     ARROW_ = 276,
     B1950_ = 277,
     BACKGROUND_ = 278,
     BEGIN_ = 279,
     BOX_ = 280,
     BOXCIRCLE_ = 281,
     BPANDA_ = 282,
     CALLBACK_ = 283,
     CIRCLE_ = 284,
     CIRCLE3D_ = 285,
     COLOR_ = 286,
     COMPASS_ = 287,
     COMPOSITE_ = 288,
     CPANDA_ = 289,
     CROSS_ = 290,
     DASH_ = 291,
     DASHLIST_ = 292,
     DEBUG_ = 293,
     DEGREES_ = 294,
     DELETE_ = 295,
     DETECTOR_ = 296,
     DIAMOND_ = 297,
     ECLIPTIC_ = 298,
     EDIT_ = 299,
     ELLIPSE_ = 300,
     END_ = 301,
     EPANDA_ = 302,
     FALSE_ = 303,
     FIELD_ = 304,
     FIXED_ = 305,
     FK4_ = 306,
     FK5_ = 307,
     FONT_ = 308,
     GALACTIC_ = 309,
     GLOBAL_ = 310,
     HIGHLITE_ = 311,
     ICRS_ = 312,
     IGNORE_ = 313,
     IMAGE_ = 314,
     INCLUDE_ = 315,
     J2000_ = 316,
     KEY_ = 317,
     LINE_ = 318,
     LINEAR_ = 319,
     MOVE_ = 320,
     N_ = 321,
     NO_ = 322,
     OFF_ = 323,
     ON_ = 324,
     PHYSICAL_ = 325,
     PIE_ = 326,
     PIXELS_ = 327,
     POINT_ = 328,
     POLYGON_ = 329,
     PROJECTION_ = 330,
     PROJECTION3D_ = 331,
     PROPERTY_ = 332,
     ROTATE_ = 333,
     ROTBOX_ = 334,
     RULER_ = 335,
     SELECT_ = 336,
     SOURCE_ = 337,
     TAG_ = 338,
     TEXT_ = 339,
     TEXTANGLE_ = 340,
     TEXTROTATE_ = 341,
     TILE_ = 342,
     TRUE_ = 343,
     VECTOR_ = 344,
     VERSION_ = 345,
     UNHIGHLITE_ = 346,
     UNSELECT_ = 347,
     UPDATE_ = 348,
     WCS_ = 349,
     WCSA_ = 350,
     WCSB_ = 351,
     WCSC_ = 352,
     WCSD_ = 353,
     WCSE_ = 354,
     WCSF_ = 355,
     WCSG_ = 356,
     WCSH_ = 357,
     WCSI_ = 358,
     WCSJ_ = 359,
     WCSK_ = 360,
     WCSL_ = 361,
     WCSM_ = 362,
     WCSN_ = 363,
     WCSO_ = 364,
     WCSP_ = 365,
     WCSQ_ = 366,
     WCSR_ = 367,
     WCSS_ = 368,
     WCST_ = 369,
     WCSU_ = 370,
     WCSV_ = 371,
     WCSW_ = 372,
     WCSX_ = 373,
     WCSY_ = 374,
     WCSZ_ = 375,
     WCS0_ = 376,
     WIDTH_ = 377,
     X_ = 378,
     Y_ = 379,
     YES_ = 380
   };

    };
    /// Token type.
    typedef token::yytokentype token_type;

    /// Build a parser object.
    ds9parse (class ds9parser &driver_yyarg, class ds9context *fr_yyarg);
    virtual ~ds9parse ();

    /// Parse.
    /// \returns  0 iff parsing succeeded.
    virtual int parse ();

#if YYDEBUG
    /// The current debugging stream.
    std::ostream& debug_stream () const;
    /// Set the current debugging stream.
    void set_debug_stream (std::ostream &);

    /// Type for debugging levels.
    typedef int debug_level_type;
    /// The current debugging level.
    debug_level_type debug_level () const;
    /// Set the current debugging level.
    void set_debug_level (debug_level_type l);
#endif

  private:
    /// Report a syntax error.
    /// \param loc    where the syntax error is found.
    /// \param msg    a description of the syntax error.
    virtual void error (const location_type& loc, const std::string& msg);

    /// Generate an error message.
    /// \param state   the state where the error occurred.
    /// \param tok     the lookahead token.
    virtual std::string yysyntax_error_ (int yystate, int tok);

#if YYDEBUG
    /// \brief Report a symbol value on the debug stream.
    /// \param yytype       The token type.
    /// \param yyvaluep     Its semantic value.
    /// \param yylocationp  Its location.
    virtual void yy_symbol_value_print_ (int yytype,
					 const semantic_type* yyvaluep,
					 const location_type* yylocationp);
    /// \brief Report a symbol on the debug stream.
    /// \param yytype       The token type.
    /// \param yyvaluep     Its semantic value.
    /// \param yylocationp  Its location.
    virtual void yy_symbol_print_ (int yytype,
				   const semantic_type* yyvaluep,
				   const location_type* yylocationp);
#endif


    /// State numbers.
    typedef int state_type;
    /// State stack type.
    typedef stack<state_type>    state_stack_type;
    /// Semantic value stack type.
    typedef stack<semantic_type> semantic_stack_type;
    /// location stack type.
    typedef stack<location_type> location_stack_type;

    /// The state stack.
    state_stack_type yystate_stack_;
    /// The semantic value stack.
    semantic_stack_type yysemantic_stack_;
    /// The location stack.
    location_stack_type yylocation_stack_;

    /// Whether the given \c yypact_ value indicates a defaulted state.
    /// \param yyvalue   the value to check
    static bool yy_pact_value_is_default_ (int yyvalue);

    /// Whether the given \c yytable_ value indicates a syntax error.
    /// \param yyvalue   the value to check
    static bool yy_table_value_is_error_ (int yyvalue);

    /// Internal symbol numbers.
    typedef unsigned char token_number_type;
    /* Tables.  */
    /// For a state, the index in \a yytable_ of its portion.
    static const short int yypact_[];
    static const short int yypact_ninf_;

    /// For a state, default reduction number.
    /// Unless\a  yytable_ specifies something else to do.
    /// Zero means the default is an error.
    static const unsigned short int yydefact_[];

    static const short int yypgoto_[];
    static const short int yydefgoto_[];

    /// What to do in a state.
    /// \a yytable_[yypact_[s]]: what to do in state \a s.
    /// - if positive, shift that token.
    /// - if negative, reduce the rule which number is the opposite.
    /// - if zero, do what YYDEFACT says.
    static const short int yytable_[];
    static const short int yytable_ninf_;

    static const short int yycheck_[];

    /// For a state, its accessing symbol.
    static const unsigned char yystos_[];

    /// For a rule, its LHS.
    static const unsigned char yyr1_[];
    /// For a rule, its RHS length.
    static const unsigned char yyr2_[]; 

    /// Convert the symbol name \a n to a form suitable for a diagnostic.
    static std::string yytnamerr_ (const char *n);


    /// For a symbol, its name in clear.
    static const char* const yytname_[];
#if YYDEBUG
    /// A type to store symbol numbers and -1.
    typedef short int rhs_number_type;
    /// A `-1'-separated list of the rules' RHS.
    static const rhs_number_type yyrhs_[];
    /// For each rule, the index of the first RHS symbol in \a yyrhs_.
    static const unsigned short int yyprhs_[];
    /// For each rule, its source line number.
    static const unsigned short int yyrline_[];
    /// For each scanner token number, its symbol number.
    static const unsigned short int yytoken_number_[];
    /// Report on the debug stream that the rule \a r is going to be reduced.
    virtual void yy_reduce_print_ (int r);
    /// Print the state stack on the debug stream.
    virtual void yystack_print_ ();

    /* Debugging.  */
    int yydebug_;
    std::ostream* yycdebug_;
#endif

    /// Convert a scanner token number \a t to a symbol number.
    token_number_type yytranslate_ (int t);

    /// \brief Reclaim the memory associated to a symbol.
    /// \param yymsg        Why this token is reclaimed.
    /// \param yytype       The symbol type.
    /// \param yyvaluep     Its semantic value.
    /// \param yylocationp  Its location.
    inline void yydestruct_ (const char* yymsg,
			     int yytype,
			     semantic_type* yyvaluep,
			     location_type* yylocationp);

    /// Pop \a n symbols the three stacks.
    inline void yypop_ (unsigned int n = 1);

    /* Constants.  */
    static const int yyeof_;
    /* LAST_ -- Last index in TABLE_.  */
    static const int yylast_;
    static const int yynnts_;
    static const int yyempty_;
    static const int yyfinal_;
    static const int yyterror_;
    static const int yyerrcode_;
    static const int yyntokens_;
    static const unsigned int yyuser_token_number_max_;
    static const token_number_type yyundef_token_;

    /* User arguments.  */
    class ds9parser &driver;
    class ds9context *fr;
  };

} } // casa::viewer

/* Line 36 of lalr1.cc  */
#line 396 "/Users/drs/develop/casa/code/build/display/ds9parse.hh"



#endif /* !CASA_VIEWER_USERS_DRS_DEVELOP_CASA_CODE_BUILD_DISPLAY_DS9PARSE_HH  */
