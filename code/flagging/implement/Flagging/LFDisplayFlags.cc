///# LFDisplayFlags.cc: this defines a light autoflagger 
//# Copyright (C) 2000,2001,2002
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
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/BasicSL/Complex.h>
#include <casa/OS/HostInfo.h>
#include <casa/stdio.h>
#include <casa/math.h>
#include <stdarg.h>

#include <flagging/Flagging/LFDisplayFlags.h>

#include <sstream>
#include <iostream>
#include <iomanip>

namespace casa {
  
#define MIN(a,b) ((a)<=(b) ? (a) : (b))
  
  // -----------------------------------------------------------------------
  // Default Constructor
  // -----------------------------------------------------------------------
  LFDisplayFlags::LFDisplayFlags () : LFBase()
  {
    plotter_p=NULL;
    allflagcounts.define("method","displayflags");
    allflags.clear();
    allcounts.clear();
    antnames_p.resize();
    corrlist_p.resize();
}


  LFDisplayFlags::~LFDisplayFlags ()
  {
    
    if(plotter_p!=NULL) { plotter_p->done(); plotter_p=NULL; }
    
  }
  
  
  Bool LFDisplayFlags::setParameters(Record &parameters)
  {
    /* Create an agent record */
    Record fullrec;
    fullrec = getParameters();
    fullrec.merge(parameters,Record::OverwriteDuplicates);
    
     /* Read values into local variables */
    ShowPlots = fullrec.asBool("showplots");
 
    
    return True;
  }
  
  
  Record LFDisplayFlags::getParameters()
  {
    Record rec;
    if( !rec.nfields() )
      {
        rec.define("algorithm","displayflags");
        rec.define("showplots",False);
      }
    return rec;
  }
  
  
  /* Extend Flags */
  /* Openmp on baselines... */
  Bool LFDisplayFlags :: runMethod(VisBuffer &inVb, 
				  Cube<Float> &inVisc, Cube<Bool> &inFlagc, Cube<Bool> &inPreFlagc,
				   uInt numT, uInt numAnt, uInt numB, uInt numC, uInt numP,
				   Vector<CountedPtr<LFBase> > &flagmethods)
  {
    LogIO os = LogIO(LogOrigin("DisplayFlags","runMethod()",WHERE));
    // Initialize all the shape information, and make a references for visc and flagc.
    LFBase::runMethod(inVisc, inFlagc, numT, numAnt, numB, numC, numP);
    preflagc.reference(inPreFlagc);
    
    // Read MS info from the vb
    if(antnames_p.nelements()==0) ReadMSInfo(inVb);
   
    // Read info about the current vb.  
    uInt thisspw = inVb.spectralWindow();
    uInt thisfield = inVb.fieldId();
    freqlist_p.assign(inVb.frequency());
    freqlist_p = freqlist_p/1e+09;
    //    cout << "field : " << thisfield << "spw : " << thisspw <<  "Freq list : " << freqlist_p << endl;
    AlwaysAssert(thisfield >=0 && thisfield < fieldnames_p.nelements() , AipsError);

    // Build and launch the GUI on first entry
    if(ShowPlots && plotter_p==NULL) BuildPlotWindow();   
 
    // Initialize various counters
    uInt a1,a2;
    IPosition shp(2),tshp(2); shp(0)=NumC; shp(1)=NumT;
    Float runningsum=0, runningflag=0,runningpreflag=0;
    chan_count.resize(NumC); chan_count=0;
    chan_flags.resize(NumC); chan_flags=0;
    baseline_count.resize(NumB); baseline_count=0;
    baseline_flags.resize(NumB); baseline_flags=0;
    corr_count.resize(NumP); corr_count=0;
    corr_flags.resize(NumP); corr_flags=0;
    chunk_count=0; chunk_flags=0;

     
    // Initialize Plot Arrays
    Vector<Float> vecflagdat(0), vecdispdat(0), origspectrum(0), flagspectrum(0), precountspec(0), countspec(0);
    if(ShowPlots) { vecflagdat.resize(NumC * NumT); vecdispdat.resize(NumC * NumT); 
                            origspectrum.resize(NumC); flagspectrum.resize(NumC);
                            precountspec.resize(NumC); countspec.resize(NumC); }
    char choice = 'a';

    AlwaysAssert( NumP == corrlist_p.nelements() , AipsError );

    // Iterate through the full chunk of data    
    for(Int bs=0;bs<NumB;bs++) // Start Baseline Loop
      {
        AlwaysAssert(bs>=0 && bs<NumB, AipsError);
	Ants(bs,&a1,&a2);
	AlwaysAssert( a1>=0 && a1<antnames_p.nelements(), AipsError );
	AlwaysAssert( a2>=0 && a2<antnames_p.nelements(), AipsError );
	//cout << "baseline : " << bs << " ants : " << a1 << "," << a2 << endl;
	if(a1 != a2) // If only cross-correlations
	  {
	    if(ShowPlots) os << LogIO::NORMAL << antnames_p[a1] << "-" << antnames_p[a2] << " : " ;
	    //cout << "Nants : " << antnames_p.nelements() << "  " << antnames_p[a1] << "-" << antnames_p[a2] << " : for ants : " << a1 << " and " << a2 << endl;
	    for(int pl=0;pl<NumP;pl++)  // Start Correlation Loop
	      {
		runningsum=0;
		runningflag=0;
                runningpreflag=0;
                if(ShowPlots){origspectrum=0.0; flagspectrum=0.0; precountspec=0.0; countspec=0.0;}
		for(int ch=0;ch<NumC;ch++)  // Start Channel Loop
		  { 
		    for(uInt tm=0;tm<NumT;tm++)  // Start Time Loop

		      {       
			  if(ShowPlots)
			    {
			      vecdispdat( ch*NumT + tm) = (visc(pl,ch,(((tm*NumB)+bs)))) * (!preflagc(pl,ch,(tm*NumB)+bs));
			      vecflagdat( ch*NumT + tm) = vecdispdat( ch*NumT + tm)*(!flagc(pl,ch,(tm*NumB)+bs));

			      origspectrum[ch] += visc(pl,ch,(((tm*NumB)+bs))) * (!preflagc(pl,ch,(tm*NumB)+bs));
			      flagspectrum[ch] += visc(pl,ch,(((tm*NumB)+bs))) * (!flagc(pl,ch,(tm*NumB)+bs));

			      precountspec[ch] += (!preflagc(pl,ch,(tm*NumB)+bs));
			      countspec[ch] += (!flagc(pl,ch,(tm*NumB)+bs));
			    }
			  runningsum += visc(pl,ch,(((tm*NumB)+bs)));
			  runningflag += (Float)(flagc(pl,ch,(tm*NumB)+bs));
			  runningpreflag += (Float)(preflagc(pl,ch,(tm*NumB)+bs));
                
                          chan_count[ch]++;   baseline_count[bs]++;  corr_count[pl]++; chunk_count++;
                          if( flagc(pl,ch,(tm*NumB)+bs) ) 
			    {
			      chan_flags[ch]++; baseline_flags[bs]++; corr_flags[pl]++; chunk_flags++;
			    }

		      }// End Time Loop
		  }//End Channel Loop

		/*		
                if(!ShowPlots) 
		  {
		cout << " Flagged : " << 100 * runningflag/(NumC*NumT) << " %  of corr " << corrlist_p[pl] << " on baseline " << a1 << "-" << a2;
		if(!runningsum)	  {    cout << " : No non-zero data !" << endl;  }
		else  {   cout << endl;  }    
		  }
		*/

		// Send the Plots			
                if(ShowPlots)
		  {
                    stringstream ostr1,ostr2;
		    ostr1 << "(" << thisfield << ") " << fieldnames_p[thisfield] << "\n[spw:" << thisspw << "] " << antnames_p[a1] << "-" << antnames_p[a2] << "  ( " << corrlist_p[pl] << " )";
                    ostr2 << fixed;
                    ostr2.precision(1);
		    ostr2 << " flag:" << 100 * runningflag/(NumC*NumT) << "% (pre-flag:" << 100 * runningpreflag/(NumC*NumT) << "%)";

		    os << "[" << corrlist_p[pl] << "]:" << 100 * runningflag/(NumC*NumT) << "%(" << 100 * runningpreflag/(NumC*NumT) << "%) "; 

		    DisplayRaster(NumC,NumT,vecdispdat,panels_p[pl].getInt());
                    plotter_p->setlabel(" ",pl?" ":"Time",ostr1.str(),panels_p[pl].getInt());
		    DisplayRaster(NumC,NumT,vecflagdat,panels_p[pl+NumP].getInt());
		    plotter_p->setlabel("Frequency",pl?" ":"Time",ostr2.str(),panels_p[pl+NumP].getInt());
		    
		    for(uInt ch=0;ch<NumC;ch++)
		      {
			if(precountspec[ch]==0) {origspectrum[ch]=0.0; precountspec[ch]=1.0;}
			if(countspec[ch]==0) {flagspectrum[ch]=0.0; countspec[ch]=1.0;}
		      }

                    origspectrum = (origspectrum/precountspec);
                    flagspectrum = (flagspectrum/countspec);

                    AlwaysAssert( freqlist_p.nelements()==NumC , AipsError); 

                    DisplayLine(NumC, freqlist_p, origspectrum, String("before:")+corrlist_p[pl], 
				String("red"), False, panels_p[pl+(2*NumP)].getInt());
                    DisplayScatter(NumC, freqlist_p, flagspectrum, String("after:")+corrlist_p[pl], 
				   String("blue"), True, panels_p[pl+(2*NumP)].getInt());
             
		    // If available, get a plot from the agents
		    
		    for (uInt fmeth=0; fmeth<flagmethods.nelements(); fmeth++)
		      {
			if(flagmethods[fmeth]->getMonitorSpectrum(flagspectrum,pl,bs))
			  {
			    //		    flagspectrum = log10(flagspectrum);
			    DisplayLine(NumC, freqlist_p, flagspectrum, flagmethods[fmeth]->methodName(), 
				String("green"), True, panels_p[pl+(2*NumP)].getInt());
			  }
		      }
		  }// if ShowPlots
	      }//End Correlation Loop

	    if(ShowPlots) os << LogIO::POST;
            
            if(ShowPlots)
	      {	    
		// Wait for User Input
		choice = GetUserInput();
		
		// React to user-input
		if(choice=='q')
		  {
		    ShowPlots = False; 
		    StopAndExit = True;
		    cout << "Exiting flagger" << endl;
		    if(plotter_p!=NULL) { plotter_p->done(); plotter_p=NULL; }
		    return False;
		  }
		else if(choice=='s')
		  {
		    ShowPlots = False;
		    cout << "Stopping display. Continuing flagging." << endl;
		    if(plotter_p!=NULL) { plotter_p->done(); plotter_p=NULL; }
		  }
		/*
		else if(choice=='n')
		  {
		    cout << bs << endl;
                    // The last baseline will always be an autocorrelation.
		    // So, the second-last is the last cross-baseline...
		    if(bs>=NumB-2){bs = NumB-3; cout << "Reached last baseline !" << endl;}
		  }
		*/
		else if(choice=='p')
		  {
                    bs -= 2;
                    if(bs < 0) {bs=-1; cout << "Reached first baseline !" << endl; }
		  }
		else if(choice=='c')
		  {
		    //cout << "Next chunk " << endl;
		    break; // break out of baseline loop
		  }

	      }// end if showplots
	    
	  }// end if cross-correlations only

      }//End Baseline Loop

    AccumulateStats(inVb);

    return True;
  }// end runMethod
  

/************************************************************************/
  // Display final Stat counts on the plotter 
  // Construct a record of counts by mapping to visbuffer indices...
  // and return the record of counts.
  /************************************************************************/
  Record LFDisplayFlags :: getStatistics()
  {
    Record res;

    for (map<string, map<string, float> >::iterator j = allcounts.begin();
           j != allcounts.end();
           j++) {
        
          Record prop;
          for (map<string, float>::const_iterator i = j->second.begin();
               i != j->second.end();
               i++) {
            
              Record tmp;

              tmp.define("flagged", (uInt) allflags[j->first][i->first]);
              tmp.define("total", (uInt) i->second);
              
              prop.defineRecord(i->first, tmp);
          }
          
          res.defineRecord(j->first, prop);
      }

    return res;
  }

  /***************************************************************/
  /***************************************************************/
  /***************************************************************/
  void LFDisplayFlags::AccumulateStats(VisBuffer &vb)
  {
   LogIO os = LogIO(LogOrigin("DisplayFlags","Stats()",WHERE));

   if(chunk_count>0)  os << LogIO::NORMAL << " --> Flagged " << 100*chunk_flags/chunk_count << "% ";
   else os << LogIO::NORMAL << " --> No data to flag" ;
    
    // Read info from the vb.
    Int spw=vb.spectralWindow();
    Int fieldid = vb.fieldId();

    // Accumulate field counts
    stringstream fieldstr;
    fieldstr << fieldid ;
    allflags["field"][fieldstr.str()] += chunk_flags;
    allcounts["field"][fieldstr.str()] += chunk_count;
    
    // Accumulate spw counts
    stringstream spwstr;
    spwstr << spw ;
    allflags["spw"][spwstr.str()] += chunk_flags;
    allcounts["spw"][spwstr.str()] += chunk_count;
    
    // Accumulate channel counts
    for(int ch=0;ch<NumC;ch++)
      { 
	stringstream chanstr;
	chanstr << fieldid << ":" << spw << ":" << ch;
	allflags["channel"][chanstr.str()] += chan_flags[ch];
	allcounts["channel"][chanstr.str()] += chan_count[ch];
      }
    
    // Accumulate baseline counts    
    for(uInt bs=0;bs<NumB;bs++) 
      {
	Ants(bs,&a1,&a2);
	if(a1 != a2) // If only cross-correlations
	  {
	    stringstream baselinestr;
	    baselinestr << fieldid << "-" << spw << "-" << a1 << "-" << a2;
	    allflags["baseline"][baselinestr.str()] += baseline_flags[bs];
	    allcounts["baseline"][baselinestr.str()] += baseline_count[bs];
	  }
      }
    
    // Accumulate correlation counts    
    for(int pl=0;pl<NumP;pl++)  
      {
	stringstream corrstr;
	corrstr << spw << ":" << pl;
	allflags["correlation"][corrstr.str()] += corr_flags[pl];
	allcounts["correlation"][corrstr.str()] += corr_count[pl];
	if(corr_count[pl]>0) 	os << " [" << corrlist_p[pl] << "]:" << 100*corr_flags[pl]/corr_count[pl] ;
	else os << " [" << corrlist_p[pl] << "]: No data" ;
      }    

    os << LogIO::POST;

  }// end of accumulateStats
  /***************************************************************/
  /***************************************************************/

  void LFDisplayFlags::ReadMSInfo(VisBuffer &vb)
  {
    // ROMSColumns msc = vb.msColumns();
    //ROMSAntennaColumns antcol = msc.antenna();
    //ROScalarColumn antnamecol = antcol.name();
    //antnames_p = antnamecol.getColumn();

    antnames_p = vb.msColumns().antenna().name().getColumn();

    Vector<Int> corrtypes;
    vb.msColumns().polarization().corrType().get(0,corrtypes);

    corrlist_p.resize(corrtypes.nelements());
    for(Int i=0;i<corrtypes.nelements();i++)
        corrlist_p[i] = Stokes::name((Stokes::StokesTypes)corrtypes[i]);

    fieldnames_p = vb.msColumns().field().name().getColumn();

  }

  /***************************************************************/
  /***************************************************************/
  
  
  /***********************************************************************/  
  /******************     Plot Functions      ******************************/  
  /***********************************************************************/  
  
  Bool LFDisplayFlags::BuildPlotWindow()
  {

    /*
    
    dock_xml_p = "\
<?xml version=\"1.0\" encoding=\"UTF-8\"?>	\
<ui version=\"4.0\">				\
 <class>dock01</class>				\
 <widget class=\"QDockWidget\" name=\"dock01\">	\
  <property name=\"geometry\">			\
   <rect>					\
    <x>0</x>					\
    <y>0</y>					\
    <width>645</width>				\
    <height>83</height>				\
   </rect>					\
  </property>					\
  <property name=\"minimumSize\">		\
   <size>					\
    <width>645</width>				\
    <height>83</height>						\
   </size>							\
  </property>							\
  <property name=\"windowTitle\">				\
   <string/>							\
  </property>							\
  <widget class=\"QWidget\" name=\"dockWidgetContents\">	\
   <layout class=\"QGridLayout\" name=\"gridLayout\">		\
    <item row=\"0\" column=\"0\">				\
     <layout class=\"QHBoxLayout\" name=\"horizontalLayout\">	\
      <item>							\
       <widget class=\"QPushButton\" name=\"Continue\">		\
        <property name=\"text\">				\
         <string>Continue</string>				\
        </property>						\
       </widget>						\
      </item>							\
      <item>							\
       <widget class=\"QPushButton\" name=\"StopDisplay\">	\
        <property name=\"text\">				\
         <string>Stop Display</string>				\
        </property>						\
       </widget>						\
      </item>							\
      <item>							\
       <widget class=\"QPushButton\" name=\"Quit\">		\
        <property name=\"text\">				\
         <string>Quit</string>					\
        </property>						\
       </widget>						\
      </item>							\
     </layout>							\
    </item>							\
   </layout>							\
  </widget>							\
 </widget>							\
 <resources/>							\
 <connections/>							\
</ui>								\
";
    
    */

    dock_xml_p = "\
<?xml version=\"1.0\" encoding=\"UTF-8\"?>\
<ui version=\"4.0\">\
 <class>dock01</class>\
 <widget class=\"QDockWidget\" name=\"dock01\">\
  <property name=\"geometry\">\
   <rect>\
    <x>0</x>\
    <y>0</y>\
    <width>485</width>\
    <height>80</height>\
   </rect>\
  </property>\
  <property name=\"minimumSize\">\
   <size>\
    <width>485</width>\
    <height>80</height>\
   </size>\
  </property>\
  <property name=\"windowTitle\">\
   <string/>\
  </property>\
  <widget class=\"QWidget\" name=\"dockWidgetContents\">\
   <layout class=\"QGridLayout\" name=\"gridLayout_2\">\
    <item row=\"0\" column=\"0\">\
     <layout class=\"QGridLayout\" name=\"gridLayout\">\
      <item row=\"0\" column=\"0\">\
       <widget class=\"QPushButton\" name=\"PrevBaseline\">\
        <property name=\"text\">\
         <string>Prev Baseline</string>\
        </property>\
       </widget>\
      </item>\
      <item row=\"0\" column=\"1\">\
       <widget class=\"QPushButton\" name=\"NextBaseline\">\
        <property name=\"text\">\
         <string>Next Baseline</string>\
        </property>\
       </widget>\
      </item>\
      <item row=\"0\" column=\"2\">\
       <widget class=\"QPushButton\" name=\"NextChunk\">\
        <property name=\"text\">\
         <string>Next Chunk</string>\
        </property>\
       </widget>\
      </item>\
      <item row=\"0\" column=\"3\">\
       <widget class=\"QPushButton\" name=\"StopDisplay\">\
        <property name=\"text\">\
         <string>Stop Display</string>\
        </property>\
       </widget>\
      </item>\
      <item row=\"0\" column=\"4\">\
       <widget class=\"QPushButton\" name=\"Quit\">\
        <property name=\"text\">\
         <string>Quit</string>\
        </property>\
       </widget>\
      </item>\
     </layout>\
    </item>\
   </layout>\
  </widget>\
 </widget>\
 <resources/>\
 <connections/>\
</ui>\
";

    plotter_p = dbus::launch<FlagPlotServerProxy>();
    
    panels_p.resize(NumP*3);
    string zoomloc="";
    string legendloc="bottom";
    panels_p[0] = plotter_p->panel( "", "", "", "",
				    std::vector<int>( ),legendloc,zoomloc,0,false,false);
    
    if(NumP>1)
      {
    for(Int i=1;i<NumP;i++)
      {
	panels_p[i] = plotter_p->panel( "", "", "", "",
					std::vector<int>( ),legendloc,zoomloc,panels_p[i-1].getInt(),false,false);
      }
      }
    
      panels_p[NumP] = plotter_p->panel( "", "", "", "",
				    std::vector<int>( ),legendloc,zoomloc, panels_p[0].getInt(),true,false);

      if(NumP>1)
	{
    for(int i=NumP+1;i<2*NumP;i++)
      {
	panels_p[i] = plotter_p->panel( "", "", "", "",
					std::vector<int>( ),legendloc,zoomloc, panels_p[i-1].getInt(),false,false);
      }
	}
    panels_p[2*NumP] = plotter_p->panel( "", "", "", "",
				    std::vector<int>( ),legendloc,zoomloc, panels_p[0].getInt(),true,false);
    if(NumP>1)
      {
    for(int i=2*NumP+1;i<3*NumP;i++)
      {
	panels_p[i] = plotter_p->panel( "", "", "", "",
					std::vector<int>( ),legendloc,zoomloc, panels_p[i-1].getInt(),false,false);
      }
      }
    /*
    // Misc panel
   panels_p[8] = plotter_p->panel( "BandPass", "Frequency", "Amp", "",
				    std::vector<int>( ),legendloc,zoomloc, panels_p[3].getInt(),false,false);

    // Dummy panel
      panels_p[9] = plotter_p->panel( "---", "----", "---", "",
   				    std::vector<int>( ),legendloc,zoomloc, panels_p[7].getInt(),false,false);
    */

    for (uInt i=0; i<panels_p.nelements(); i++)
      {
	if ( panels_p[i].type( ) != dbus::variant::INT ) {
	  throw( AipsError("Error in panel construction") );
	}
      }
    
    std::vector<std::string> loc;
    //loc.push_back("top");
    loc.push_back("top");
    plotter_p->loaddock(dock_xml_p,"bottom",loc,panels_p[0].getInt());

      

  }
  
  
  Char LFDisplayFlags :: GetUserInput()
  {
    Char choice='c';
    String returnvalue = plotter_p->eventloop();
    if(returnvalue == "NextBaseline") choice = 'n';
    else if(returnvalue == "PrevBaseline") choice = 'p';
    else if(returnvalue == "NextChunk") choice = 'c';
    else if(returnvalue == "StopDisplay") choice = 's';
    else if(returnvalue == "Quit") choice = 'q';

    //cout << "Choice : " << returnvalue << endl;

    return choice;
  }
  
  void LFDisplayFlags::DisplayRaster(Int xdim, Int ydim, Vector<Float> &data, uInt frame)
  {
    //    cout << "panel id : " << frame << endl;;
    
    // plotter_p->release( panel_p.getInt( ) );
    plotter_p->erase( frame );
    
    //	plotter_p->line(x, y, "blue", "time", panel_p.getInt() );
    plotter_p->raster(dbus::af(data), xdim,ydim, "Hot Metal 1", frame);
  }

  void LFDisplayFlags::DisplayLine(Int xdim, Vector<Double> &xdata, Vector<Float> &ydata, String label, String color, Bool hold, uInt frame)
  {
    if( hold==False ) plotter_p->erase( frame );
    plotter_p->line(dbus::af(xdata), dbus::af(ydata),color,label,frame);
  }

  void LFDisplayFlags::DisplayScatter(Int xdim, Vector<Double> &xdata, Vector<Float> &ydata, String label, String color, Bool hold, uInt frame)
  {
    if( hold==False ) plotter_p->erase( frame );
    plotter_p->scatter(dbus::af(xdata), dbus::af(ydata),color,label,"dot",1,4,frame);
  }
  
  
} //#end casa namespace
