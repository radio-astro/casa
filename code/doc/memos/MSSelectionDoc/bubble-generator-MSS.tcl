# This file contains the data used by the three syntax diagram rendering
# programs:
#
#   bubble-generator.tcl
#   bubble-generator-bnf.tcl
#   bubble-generator-bnf.tcl
#

# Graphs:
#
set all_graphs {
    sql-stmt-list {
	toploop {optx sql-stmt} ;
    }
    sql-stmt {
	line ANALYZE {or nil /database-name /table-or-index-name
	    {line /database-name . /table-or-index-name}}
    }
    compound-select-stmt {
	stack
	{opt {line WITH {opt RECURSIVE} {loop common-table-expression ,}}}
	{line select-core {loop
	    {line {or UNION {line UNION ALL} INTERSECT EXCEPT}
		select-core} nil}}
	{optx ORDER BY {loop ordering-term ,}}
	{optx LIMIT expr {optx {or OFFSET ,} expr}}
    }
    
    SPW-EXPR
    {
	loop 
	{
	    line {loop SPWID ;} 
	    {line {or nil  {line : {loop FREQ ;}}}}
	} ,
    }

    SPWID 
    {
	line 
	{
	    or 
	    {line {or nil > <} {or INTEGER FREQ}}
	    REGEX RANGE
	}
    }

    FREQ 
    {
	line {or NUMBER RANGE} {or K M G T} Hz
    }

    RANGE
    {
	line NUMBER ~ NUMBER
    }

    ANT-STATION-SPEC
    {
	line {or INTEGER {line INTEGER ~ INTEGER} STRING REGEX}
    }

    ANT-EXPR
    {
	loop
	{
	    line 
	    {or ANTSPEC {line ( {loop ANTSPEC ,} )}}
	    {or nil {line @ {or STATIONSPEC {line ( {loop STATIONSPEC ,} )}}}}
	} ,
    }

    BASELINE-EXPR
    {
	loop
	{
	    line {or nil !} ANT-EXPR {or nil {line {or & &&} {or nil ANT-EXPR}} &&&}
	} ;
    }
}
