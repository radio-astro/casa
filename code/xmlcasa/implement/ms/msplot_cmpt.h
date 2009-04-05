//# msplot_cmpt.h:  defines the MsPlot tool, which handles the plotting
//#                 of measurement sets.
//# Copyright (C) 2003,2007-2008
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
//# @author  Gary Li & Shannon Jaeger
//# @version 
//////////////////////////////////////////////////////////////////////////////
// Standard Include files

#ifndef _msplot_cmpt__H__
#define _msplot_cmpt__H__
#ifndef _msplot_cpnt__H__
#define _msplot_cpnt__H__

#include <vector>
#include <xmlcasa/casac.h>
#include <xmlcasa/ms/msplot_forward.h>
#include <flagging/MSPlot/MsPlot.h>

// put includes here

namespace casac {

/**
 * msplot component class 
 *
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author Shannon Jaeger
 * @version 
 **/
class msplot
{
  private:

	
  public:

    msplot();
    virtual ~msplot();

    // open, resetting, clearing, and done methods
    bool open(const std::string& msname = "", const bool dovel = false, 
              const std::string& restfreq = "", 
              const std::string& frame = "", 
              const std::string& doppler = "");

    bool clearplot( const int subplot = 000 );
    bool emperorsNewClose();
    bool closeMS();
    bool reset();
    bool close();
    bool done();

    
    /* Selection of options and data */
    bool plotoptions( const int subplot = 0, 
	    const std::string& plotsymbol = "",
	    const std::string& plotcolor = "", 
	    const std::string& multicolor = "none",
	    const std::string& plotrange = "",
	    const char timeplot='o',
	    const double markersize=-1.0,
	    const double linewidth=-1.0,
	    const bool overplot = false, 
	    const bool replacetopplot = false, 
	    const bool removeoldpanels = true,
	    const int skipnrows = 0,
	    const int averagenrows = 0,
            const std::string& extendflag = "",
	    const std::string& connect = "none",	    
	    const bool showflags = false,
	    const std::string& title = "", 
	    const std::string& xlabel = "",
	    const std::string& ylabel = "", 
	    const double fontsize = -1.0, 
	    const double windowsize = -1.0, 
	    const double aspectratio = -1.0 );
    
    bool summary( const bool selected=true );
   
    bool setdata( const std::string& baseline = "",
	    const std::string& field = "",
	    const std::string& scan = "",
	    const std::string& uvrange = "",
	    const std::string& array = "",
	    const std::string& feed = "",
	    const std::string& spw = "",
	    const std::string& correlation = "",
	    const std::string& time = "");

    bool avedata( const std::string& chanavemode = "none",
    	    const std::string& corravemode = "none",
            const std::string& datacolumn = "DATA",
            const std::string& averagemode = "vector", 
            const std::string& averagechan = "1", 
            const std::string& averagetime = "0",
            const bool averageflagged = false,
            const bool averagescan = false,
            const bool averagebl = false,
            const bool averagearray = false,
            const bool averagechanid = false,
            const bool averagevel = false );

     bool extendflag(const std::string& extendcorr = "", 
                     const std::string& extendchan = "", 
                     const std::string& extendspw = "", 
                     const std::string& extendant = "", 
                     const std::string& extendtime = "");

    bool checkplotxy(const std::string& x = "uvdist", 
	    const std::string& y = "data",
	    const std::string& xcolumn = "data",
	    const std::string& ycolumn = "data", 
	    const std::string& xvalue = "amp",
	    const std::string& yvalue = "amp",
	    const std::vector<std::string>& 
               iteration = std::vector<std::string> (1, "") );

    bool plotxy(const std::string& x = "uvdist", 
	    const std::string& y = "data",
	    const std::string& xcolumn = "data", 
	    const std::string& ycolumn = "data", 
	    const std::string& xvalue = "amp",
	    const std::string& yvalue = "amp",
	    const std::vector<std::string>& iteration = std::vector<std::string> (1, "") );
    

    bool checkplot( const std::string& type = "", 
	    const std::string& column = "", 
	    const std::string& value = "", 
	    const std::vector<std::string>& iteration = std::vector<std::string> (1, "") );
    
    bool plot( const std::string& type = "", 
	    const std::string& column = "", 
	    const std::string& value = "", 
	    const std::vector<std::string>& iteration = std::vector<std::string> (1, "") );

    
    // Iterative plot function definitions.
    // Note that iterplotstart() is the same as iterplotnext(), we just
    // have it so the user's are more comfortable with things.  They can
    // just use iterplotnext() without using iterplotstart() if they wish.
    bool iterplotstart();
    bool iterplotnext();
    bool iterplotstop( const bool rmplotter = false );

    // For those who wish to create papers, and the pipeline group a routine
    // for saving the plotted image to a file.  Only supported types at this
    // point are eps, ps, pdf, and png. png is takes the least amount of time
    // to create.
    bool savefig( const std::string& filename = "",
                const int dpi = -1,
		const std::string& orientation = "",
		const std::string& papertype = "",
		const std::string& facecolor = "",
	        const std::string& edgecolor = "");
    


    /* Interaction with the plot window */
    bool markregion(const int subplot = 111, 
	     const std::vector<double>& region = std::vector<double> (1,0.0));

    bool flagdata();
    bool unflagdata();
    bool clearflags();
    bool locatedata();

    /* Flag Version functions */
    bool saveflagversion(const std::string& versionname = """", const std::string& comment = """", const std::string& merge = """");
    bool restoreflagversion(const std::vector<std::string>& versionname = std::vector<std::string> (1, """"), const std::string& merge = """");
    bool deleteflagversion(const std::vector<std::string>& versionname = std::vector<std::string> (1, """"));
    bool getflagversionlist();

    private:

#include <xmlcasa/ms/msplot_private.h>

};

} // casac namespace
#endif
#endif


