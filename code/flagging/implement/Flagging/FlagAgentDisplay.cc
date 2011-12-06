//# FlagAgentDisplay.cc: This file contains the implementation of the FlagAgentDisplay class.
//#
//#  CASA - Common Astronomy Software Applications (http://casa.nrao.edu/)
//#  Copyright (C) Associated Universities, Inc. Washington DC, USA 2011, All rights reserved.
//#  Copyright (C) European Southern Observatory, 2011, All rights reserved.
//#
//#  This library is free software; you can redistribute it and/or
//#  modify it under the terms of the GNU Lesser General Public
//#  License as published by the Free software Foundation; either
//#  version 2.1 of the License, or (at your option) any later version.
//#
//#  This library is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY, without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//#  Lesser General Public License for more details.
//#
//#  You should have received a copy of the GNU Lesser General Public
//#  License along with this library; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
//#  MA 02111-1307  USA
//# $Id: rurvashi 28 Nov 2011$

#include <flagging/Flagging/FlagAgentDisplay.h>

namespace casa { //# NAMESPACE CASA - BEGIN
  
  FlagAgentDisplay::FlagAgentDisplay(FlagDataHandler *dh, Record config, Bool writePrivateFlagCube):
    //    FlagAgentBase(dh,config,ANTENNA_PAIRS,writePrivateFlagCube), 
        FlagAgentBase(dh,config,ANTENNA_PAIRS_INTERACTIVE,writePrivateFlagCube), 
	plotter_p(NULL), ShowPlots(True), 
	userChoice_p("Continue"), userFixA1_p(""), userFixA2_p(""),
	skipScan_p(-1), skipSpw_p(-1), skipField_p(-1),pause_p(False),
	fieldId_p(-1), fieldName_p(""), scanStart_p(-1), scanEnd_p(-1), spwId_p(-1),
	nPolarizations_p(1), freqList_p(Vector<Double>()),
	antenna1_p(""),antenna2_p("")
  {


    setAgentParameters(config);
    
    // Request loading polarization map to FlagDataHandler
    flagDataHandler_p->setMapPolarizations(true);
  }
  
  FlagAgentDisplay::~FlagAgentDisplay()
  {
    // Compiler automagically calls FlagAgentBase::~FlagAgentBase()
    
    if(plotter_p != NULL) { plotter_p->done(); plotter_p=NULL; }
  }
  
  void FlagAgentDisplay::setAgentParameters(Record config)
  {
    int exists;
    
    exists = config.fieldNumber ("pause");
    if (exists >= 0)
      {
	pause_p = config.asBool("pause");
      }
    else
      {
	pause_p = True;
      }
    
    *logger_p << LogIO::NORMAL << "FlagAgentDisplay::" << __FUNCTION__ << " pause is " << pause_p << LogIO::POST;
    
    
  }


void
FlagAgentDisplay::preProcessBuffer(const VisBuffer &visBuffer)
{

  getChunkInfo(visBuffer);

    return;
}

  
  void
  FlagAgentDisplay::iterateAntennaPairsInteractive(antennaPairMap *antennaPairMap_ptr)
  {
    // Check if the visibility expression is suitable for this spw
    if (!checkVisExpression(flagDataHandler_p->getPolarizationMap())) return;
    
    // Iterate through antenna pair map
    std::pair<Int,Int> antennaPair;
    antennaPairMapIterator myAntennaPairMapIterator;
    bool stepback=False;
    for (myAntennaPairMapIterator=antennaPairMap_ptr->begin(); myAntennaPairMapIterator != antennaPairMap_ptr->end(); ++myAntennaPairMapIterator)
      {

	// Check whether to skip the rest of this chunk or not
	if(skipSpw_p != -1) 
	  {
	    if(skipSpw_p == spwId_p) {ShowPlots=False;} // Skip the rest of this SPW
	    else {skipSpw_p = -1; ShowPlots=True;} // Reached next SPW. Reset state
	  }
	if(skipField_p != -1) 
	  {
	    if(skipField_p == fieldId_p) {ShowPlots=False;} // Skip the rest of this Field
	    else {skipField_p = -1; ShowPlots=True;} // Reached next Field. Reset state
	  }
	if(skipScan_p != -1) 
	  {
	    if(skipScan_p == scanEnd_p) {ShowPlots=False;} // Skip the rest of this Scan
	    else {skipScan_p = -1; ShowPlots=True;} // Reached next Scan. Reset state
	  }


	// Display this baseline
	if(ShowPlots)
	  {
	    
	    // If choice from previous plot was to go backwards in baseline.
	    if(stepback)
	      {
		// Go to previous baseline (decrement by 2)
		if( myAntennaPairMapIterator != antennaPairMap_ptr->begin() )
		  -- myAntennaPairMapIterator; 
		if( myAntennaPairMapIterator != antennaPairMap_ptr->begin() )
		  -- myAntennaPairMapIterator; 
		
		// If antenna constraints exist, keep going back until first match is found. 
		// If not found, stay on current baseline (continue)
		if( userFixA1_p != String("") || userFixA2_p != String("") )
		  {
		    antennaPairMapIterator tempIterator;
		    bool found=False;
		    for(tempIterator = myAntennaPairMapIterator; tempIterator != antennaPairMap_ptr->begin() ; --tempIterator )
		      {
			if( ! skipBaseline(tempIterator->first) ) {found=True; break;}
		      }
		    if(found) // Jump to this antenna pair
		      { 
			myAntennaPairMapIterator = tempIterator; 
		      }
		    else 
		      {
			cout << "No Previous baseline in this chunk with Ant1 : " 
			     << ( (userFixA1_p==String(""))?String("any"):userFixA1_p )
			     << "  and Ant2 : " 
			     << ( (userFixA2_p==String(""))?String("any"):userFixA2_p )
			     << endl;
		
			// Stay on current baseline
			if( myAntennaPairMapIterator != antennaPairMap_ptr->end() )
			  ++myAntennaPairMapIterator; 
		      }
		  }

		// Reset state
		stepback=False;
	      }
	    
	    // Get antenna pair from map
	    antennaPair = myAntennaPairMapIterator->first;
	    
	    // Check whether or not to display this baseline (for going in the forward direction)
	    if( skipBaseline(antennaPair) ) continue;
	    
 	    // Process antenna pair
	    processAntennaPair(antennaPair.first,antennaPair.second);
	    
	    // If Plot window is visible, and, if asked for, get and react to user-choices.
	    if(pause_p==True)
	      {
		
		// Wait for User Input
		getUserInput(); // Fills in userChoice_p. userfix
		
		// React to user-input
		if(userChoice_p=="Quit")
		  {
		    ShowPlots = False; 
		    StopAndExit = True;
		    cout << "Exiting flagger" << endl;
		    if(plotter_p!=NULL) { plotter_p->done(); plotter_p=NULL; }
		    flagDataHandler_p->stopIteration();
		    return ;
		  }
		else if(userChoice_p=="StopDisplay")
		  {
		    ShowPlots = False;
		    cout << "Stopping display. Continuing flagging." << endl;
		    if(plotter_p!=NULL) { plotter_p->done(); plotter_p=NULL; }
		  }
		else if(userChoice_p=="PrevBaseline")
		  {
		    if( myAntennaPairMapIterator==antennaPairMap_ptr->begin() )
		      cout << "Already on first baseline..." << endl;
		    stepback=True;
		  }
		else if(userChoice_p=="NextScan")
		  {
		    cout << "Next Scan " << endl;
		    skipScan_p = scanEnd_p;
		  }
		else if(userChoice_p=="NextSpw")
		  {
		    cout << "Next SPW " << endl;
		    skipSpw_p = spwId_p;
		  }
		else if(userChoice_p=="NextField")
		  {
		    cout << "Next Field " << endl;
		    skipField_p = fieldId_p;
		  }
		else if(userChoice_p=="Continue")
		  {
		    cout << "Next chunk " << endl; // Right now, a chunk is one baseline !
		    return; 
		  }
		
	      }// end if pause=True
	    
	  }// if ShowPlots
	
      }// end antennaMapIterator
    
    return;
  }
  
  
  Bool 
  FlagAgentDisplay::skipBaseline(std::pair<Int,Int> antennaPair)
  {
    String antenna1Name = flagDataHandler_p->antennaNames_p->operator()(antennaPair.first);
    String antenna2Name = flagDataHandler_p->antennaNames_p->operator()(antennaPair.second);
    //	    if(userFixA2_p != "") cout << "*********** userfixa2 : " << userFixA2_p << "   thisant : " << antenna1Name << " && " << antenna2Name << endl;
    return  (  (userFixA1_p != ""  && userFixA1_p != antenna1Name)  ||   (userFixA2_p != ""  && userFixA2_p != antenna2Name) ) ;
  }
  
  void
  FlagAgentDisplay::computeAntennaPairFlags(const VisBuffer &visBuffer, VisMapper &visibilities,FlagMapper &flags,Int antenna1,Int antenna2,vector<uInt> &rows)
  {
    // Gather shapes
    IPosition flagCubeShape = visibilities.shape();
    uInt nChannels = flagCubeShape(0);
    uInt nTimes = flagCubeShape(1);
    
    // Read antenna names for the current baseline
    String antenna1Name = flagDataHandler_p->antennaNames_p->operator()(antenna1);
    String antenna2Name = flagDataHandler_p->antennaNames_p->operator()(antenna2);
    String baselineName = antenna1Name + "&&" + antenna2Name;
    antenna1_p = antenna1Name;
    antenna2_p = antenna2Name;
    
    String scanRange = (scanStart_p!=scanEnd_p)?String::toString(scanStart_p)+"~"+String::toString(scanEnd_p) : String::toString(scanStart_p);
    String spwName = String::toString(visBuffer.spectralWindow());
    
    // Get Frequency List
    freqList_p.resize(nChannels);
    for(uInt ch=0;ch<nChannels;ch++) freqList_p[ch]=(Double)ch;
    
    
    /*    
    // Read current Field name, SPW id, and correlation string from visBuffer Info.
    uInt fieldId_p = visBuffer.fieldId();
    String fieldName = visBuffer.msColumns().field().name().getColumn()[fieldId_p];
    String spwName = String::toString(visBuffer.spectralWindow());
    Int scanstart = visBuffer.scan()[0];
    int scanend = visBuffer.scan()[ (visBuffer.scan().nelements())-1 ];
    String scanRange = (scanstart!=scanend)?String::toString(scanstart)+"~"+String::toString(scanend) : String::toString(scanstart);
    */
    
    // Get Polarization Maps
    Vector<uInt> polarizations = flags.getSelectedCorrelations();
    nPolarizations_p = polarizations.size();
    polarizationIndexMap *polMap = flagDataHandler_p->getPolarizationIndexMap();
    Vector<String> corrTypes(nPolarizations_p);
    for(uInt pol=0;pol<nPolarizations_p;pol++)
      corrTypes[pol] = (*polMap)[polarizations[pol]];
    
    // Print where we are...
    //    *logger_p << LogIO::NORMAL << "FlagAgentDisplay::" << __FUNCTION__ << " Baseline : " << baselineName << " Field : " << fieldName_p << " Spw : " << spwName << "  nChan : " << nChannels << " nPol : " << nPolarizations_p << " nTime : " << nTimes << LogIO::POST;
    
    // Build the Plot Window for the first time
    if(ShowPlots && plotter_p==NULL) BuildPlotWindow();
    
    // Initialize Plot Arrays and other vars
    Float runningsum=0, runningflag=0,runningpreflag=0;
    Vector<Float> vecflagdat(0), vecdispdat(0);
    Vector<Float> origspectrum(0), flagspectrum(0), precountspec(0), countspec(0);
    if(ShowPlots)
      {
	vecflagdat.resize(nChannels * nTimes); vecdispdat.resize(nChannels * nTimes); 
	origspectrum.resize(nChannels); flagspectrum.resize(nChannels);
	precountspec.resize(nChannels); countspec.resize(nChannels);
      }
    
    if(ShowPlots)
      {
	// Make and send plots for each polarization
	for(int pl=0;pl<nPolarizations_p;pl++)  // Start Correlation Loop
	  {
	    runningsum=0; runningflag=0;  runningpreflag=0;
	    origspectrum=0.0; flagspectrum=0.0; precountspec=0.0; countspec=0.0;
	    for(int ch=0;ch<nChannels;ch++)  // Start Channel Loop
	      { 
		for(uInt tm=0;tm<nTimes;tm++)  // Start Time Loop
		  {       
		    // UUU FOR TEST ONLY -- Later, enable additional ManualFlagAgent in the tFlagAgentDisplay
		    /////if(ch>10 && ch<20) flags.applyFlag(ch,tm);
		    
		    vecdispdat( ch*nTimes + tm ) = visibilities(pl,ch,tm) * ( ! flags.getOriginalFlags(pl,ch,tm) );
		    vecflagdat( ch*nTimes + tm ) = visibilities(pl,ch,tm) * ( ! flags.getModifiedFlags(pl,ch,tm) );
		    
		    origspectrum[ch] += visibilities(pl,ch,tm) * ( ! flags.getOriginalFlags(pl,ch,tm) );
		    flagspectrum[ch] += visibilities(pl,ch,tm) * ( ! flags.getModifiedFlags(pl,ch,tm) );
		    
		    precountspec[ch] += ( ! flags.getOriginalFlags(pl,ch,tm) );
		    countspec[ch] += ( ! flags.getModifiedFlags(pl,ch,tm) );
		    
		    runningsum += visibilities(pl,ch,tm);
		    runningflag += (Float)(flags.getModifiedFlags(pl,ch,tm));
		    runningpreflag += (Float)(flags.getOriginalFlags(pl,ch,tm));
		    
		  }// End Time Loop
	      }//End Channel Loop
	    
	    // Make the Labels
	    stringstream ostr1,ostr2;
	    ostr1 << "(" << fieldId_p << ") " << fieldName_p << "  [scan:" << scanRange << "]\n[spw:" << spwName << "] " << baselineName << "  ( " << corrTypes[pl] << " )";
	    ostr2 << fixed;
	    ostr2.precision(1);
	    ostr2 << " flag:" << 100 * runningflag/(nChannels*nTimes) << "% (pre-flag:" << 100 * runningpreflag/(nChannels*nTimes) << "%)";
	    
	    //*logger_p << "[" << corrTypes[pl] << "]:" << 100 * runningflag/(nChannels*nTimes) << "%(" << 100 * runningpreflag/(nChannels*nTimes) << "%) "; 
	    
	    // Make the Before/After Raster Plots
	    DisplayRaster(nChannels,nTimes,vecdispdat,panels_p[pl].getInt());
	    plotter_p->setlabel(" ",pl?" ":"Time",ostr1.str(),panels_p[pl].getInt());
	    DisplayRaster(nChannels,nTimes,vecflagdat,panels_p[pl+nPolarizations_p].getInt());
	    plotter_p->setlabel("Frequency",pl?" ":"Time",ostr2.str(),panels_p[pl+nPolarizations_p].getInt());
	    
	    // Make the Before/After bandpass plots
	    for(uInt ch=0;ch<nChannels;ch++)
	      {
		if(precountspec[ch]==0) {origspectrum[ch]=0.0; precountspec[ch]=1.0;}
		if(countspec[ch]==0) {flagspectrum[ch]=0.0; countspec[ch]=1.0;}
	      }
	    
	    origspectrum = (origspectrum/precountspec);
	    flagspectrum = (flagspectrum/countspec);
	    AlwaysAssert( freqList_p.nelements()==nChannels , AipsError); 
	    
	    DisplayLine(nChannels, freqList_p, origspectrum, String("before:")+corrTypes[pl], 
			String("red"), False, panels_p[pl+(2*nPolarizations_p)].getInt());
	    DisplayScatter(nChannels, freqList_p, flagspectrum, String("after:")+corrTypes[pl], 
			   String("blue"), True, panels_p[pl+(2*nPolarizations_p)].getInt());
	    
	    //// TODO : Can I query the tfcrop agent for a "view" to overlay here. 
	    // If available, get a plot from the agents
	    /*
	      for (uInt fmeth=0; fmeth<flagmethods.nelements(); fmeth++)
	      {
	      if(flagmethods[fmeth]->getMonitorSpectrum(flagspectrum,pl,bs))
	      {
	      //		    flagspectrum = log10(flagspectrum);
	      DisplayLine(nChannels, freqlist_p, flagspectrum, flagmethods[fmeth]->methodName(), 
	      String("green"), True, panels_p[pl+(2*nPolarizations_p)].getInt());
	      }
	      }
	    */
	    
	  }//End Correlation Loop
	
	*logger_p << LogIO::POST;
	
      }// end if ShowPlots
    
    return;
  }
  
  //----------------------------------------------------------------------------------------------------------
  
  void 
  FlagAgentDisplay::getChunkInfo(const VisBuffer &visBuffer)
  {
    // Read current Field name, SPW id, and scan info.
    fieldId_p = visBuffer.fieldId();
    fieldName_p = flagDataHandler_p->fieldNames_p->operator()(fieldId_p);
    spwId_p = visBuffer.spectralWindow();
    scanStart_p = visBuffer.scan()[0];
    scanEnd_p = visBuffer.scan()[ (visBuffer.scan().nelements())-1 ];
    
    *logger_p << LogIO::NORMAL << "FlagAgentDisplay::" << __FUNCTION__ << " Field : " << fieldId_p << " , " << fieldName_p << " Spw : " << spwId_p << " Scan : " << scanStart_p << " : " << scanEnd_p << LogIO::POST;
  }
  
  
  /***********************************************************************/  
  /******************     Plot Functions      ******************************/  
  /***********************************************************************/  
  
  Bool FlagAgentDisplay::BuildPlotWindow()
  {
    
    setLayout();
    AlwaysAssert( dock_xml_p != NULL , AipsError );
    
    plotter_p = dbus::launch<FlagPlotServerProxy>();
    
    panels_p.resize(nPolarizations_p*3);
    string zoomloc="";
    string legendloc="bottom";
    panels_p[0] = plotter_p->panel( "", "", "", "",
				    std::vector<int>( ),legendloc,zoomloc,0,false,false);
    
    if(nPolarizations_p>1)
      {
	for(Int i=1;i<nPolarizations_p;i++)
	  {
	    panels_p[i] = plotter_p->panel( "", "", "", "",
					    std::vector<int>( ),legendloc,zoomloc,panels_p[i-1].getInt(),false,false);
	  }
      }
    
    panels_p[nPolarizations_p] = plotter_p->panel( "", "", "", "",
						   std::vector<int>( ),legendloc,zoomloc, panels_p[0].getInt(),true,false);
    
    if(nPolarizations_p>1)
      {
	for(int i=nPolarizations_p+1;i<2*nPolarizations_p;i++)
	  {
	    panels_p[i] = plotter_p->panel( "", "", "", "",
					    std::vector<int>( ),legendloc,zoomloc, panels_p[i-1].getInt(),false,false);
	  }
      }
    panels_p[2*nPolarizations_p] = plotter_p->panel( "", "", "", "",
						     std::vector<int>( ),legendloc,zoomloc, panels_p[0].getInt(),true,false);
    if(nPolarizations_p>1)
      {
	for(int i=2*nPolarizations_p+1;i<3*nPolarizations_p;i++)
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
    
    
    
  }// end BuildPlotWindow
  
  
  
  void FlagAgentDisplay :: getUserInput()
  {
    userChoice_p = "Continue";
    String returnvalue;
    
    Bool exitEventLoop=False;
    
    while( ! exitEventLoop)
      {
	
	returnvalue = plotter_p->eventloop();
	if(returnvalue == "NextBaseline" || returnvalue == "PrevBaseline" || returnvalue == "NextScan"
	   || returnvalue == "NextField" || returnvalue == "NextSpw" 
	   || returnvalue == "StopDisplay" || returnvalue == "Quit") 
	  {
	    userChoice_p = returnvalue; 
	    exitEventLoop=True;
	  }
	else if(returnvalue == "FixAntenna1:0" || returnvalue == "FixAntenna1:2" ) 
	  {
	    userChoice_p = "Continue";  
	    //	    userFixA1_p=(returnvalue.lastchar()=='0')?-1:antenna1_p; 
	    userFixA1_p=(returnvalue.lastchar()=='0')?String(""):antenna1_p; 
	    exitEventLoop=False;
	  }
	else if(returnvalue == "FixAntenna2:0" || returnvalue == "FixAntenna2:2" ) 
	  {
	    userChoice_p = "Continue";  
	    //	    userFixA2_p=(returnvalue.lastchar()=='0')?-1:antenna2_p; 
	    userFixA2_p=(returnvalue.lastchar()=='0')?String(""):antenna2_p; 
	    exitEventLoop=False;
	  }
	else cout << "Unknown GUI choice" << endl;
	
	//    cout << "ReturnValue : " << returnvalue << "   userChoice : " << userChoice_p << "  userFixA1 : " << userFixA1_p << "  userFixA2 : " << userFixA2_p << endl;
	
      }
    
    
    return;
  }
  
  void FlagAgentDisplay::DisplayRaster(Int xdim, Int ydim, Vector<Float> &data, uInt frame)
  {
    //    cout << "panel id : " << frame << endl;;
    
    // plotter_p->release( panel_p.getInt( ) );
    plotter_p->erase( frame );
    
    //	plotter_p->line(x, y, "blue", "time", panel_p.getInt() );
    plotter_p->raster(dbus::af(data), xdim,ydim, "Hot Metal 1", frame);
  }
  
  void FlagAgentDisplay::DisplayLine(Int xdim, Vector<Double> &xdata, Vector<Float> &ydata, String label, String color, Bool hold, uInt frame)
  {
    if( hold==False ) plotter_p->erase( frame );
    plotter_p->line(dbus::af(xdata), dbus::af(ydata),color,label,frame);
  }
  
  void FlagAgentDisplay::DisplayScatter(Int xdim, Vector<Double> &xdata, Vector<Float> &ydata, String label, String color, Bool hold, uInt frame)
  {
    if( hold==False ) plotter_p->erase( frame );
    plotter_p->scatter(dbus::af(xdata), dbus::af(ydata),color,label,"dot",1,4,frame);
  }
  
  
  
  Bool FlagAgentDisplay :: setLayout()
  {
    
    dock_xml_p = "\
<?xml version=\"1.0\" encoding=\"UTF-8\"?>	\
<ui version=\"4.0\">				\
 <class>dock01</class>				\
 <widget class=\"QDockWidget\" name=\"dock01\">	\
  <property name=\"geometry\">			\
   <rect>					\
    <x>0</x>					\
    <y>0</y>					\
    <width>770</width>				\
    <height>80</height>				\
   </rect>					\
  </property>					\
  <property name=\"sizePolicy\">				\
   <sizepolicy hsizetype=\"Preferred\" vsizetype=\"Preferred\">	\
    <horstretch>0</horstretch>					\
    <verstretch>0</verstretch>					\
   </sizepolicy>						\
  </property>							\
  <property name=\"minimumSize\">				\
   <size>							\
    <width>770</width>						\
    <height>80</height>						\
   </size>							\
  </property>							\
  <property name=\"windowTitle\">				\
   <string/>							\
  </property>							\
  <widget class=\"QWidget\" name=\"dockWidgetContents\">	\
   <widget class=\"QWidget\" name=\"\">				\
    <property name=\"geometry\">				\
     <rect>							\
      <x>13</x>							\
      <y>14</y>							\
      <width>735</width>					\
      <height>46</height>					\
     </rect>							\
    </property>							\
    <layout class=\"QGridLayout\" name=\"gridLayout\">		\
     <item row=\"0\" column=\"0\">				\
      <widget class=\"QPushButton\" name=\"PrevBaseline\">	\
       <property name=\"text\">					\
        <string>Prev Baseline</string>				\
       </property>						\
      </widget>							\
     </item>							\
     <item row=\"0\" column=\"1\">				\
      <widget class=\"QPushButton\" name=\"NextBaseline\">	\
       <property name=\"text\">					\
        <string>Next Baseline</string>				\
       </property>						\
      </widget>							\
     </item>							\
     <item row=\"0\" column=\"2\">				\
      <layout class=\"QVBoxLayout\" name=\"verticalLayout\">	\
       <property name=\"spacing\">				\
        <number>0</number>					\
       </property>						\
       <property name=\"sizeConstraint\">			\
        <enum>QLayout::SetMinimumSize</enum>			\
       </property>						\
       <item>							\
        <widget class=\"QCheckBox\" name=\"FixAntenna1\">	\
         <property name=\"text\">				\
          <string>Fix Antenna1</string>				\
         </property>						\
        </widget>						\
       </item>							\
       <item>							\
        <widget class=\"QCheckBox\" name=\"FixAntenna2\">	\
         <property name=\"text\">				\
          <string>Fix Antenna2</string>				\
         </property>						\
        </widget>						\
       </item>							\
      </layout>							\
     </item>							\
     <item row=\"0\" column=\"3\">				\
      <widget class=\"QPushButton\" name=\"NextSpw\">		\
       <property name=\"text\">					\
        <string>Next SPW</string>				\
       </property>						\
      </widget>							\
     </item>							\
     <item row=\"0\" column=\"4\">				\
      <widget class=\"QPushButton\" name=\"NextScan\">		\
       <property name=\"text\">					\
        <string>Next Scan</string>				\
       </property>						\
      </widget>							\
     </item>							\
     <item row=\"0\" column=\"5\">				\
      <widget class=\"QPushButton\" name=\"NextField\">		\
       <property name=\"text\">					\
        <string>Next Field</string>				\
       </property>						\
      </widget>							\
     </item>							\
     <item row=\"0\" column=\"6\">				\
      <widget class=\"QPushButton\" name=\"StopDisplay\">	\
       <property name=\"text\">					\
        <string>Stop Display</string>				\
       </property>						\
      </widget>							\
     </item>							\
     <item row=\"0\" column=\"7\">				\
      <widget class=\"QPushButton\" name=\"Quit\">		\
       <property name=\"text\">					\
        <string>Quit</string>					\
       </property>						\
      </widget>							\
     </item>							\
    </layout>							\
   </widget>							\
  </widget>							\
 </widget>							\
 <resources/>							\
 <connections/>							\
</ui>								\
";
    
    
    /*
      
      dock_xml_p = "\
      <?xml version=\"1.0\" encoding=\"UTF-8\"?>	\
      <ui version=\"4.0\">				\
      <class>dock01</class>				\
      <widget class=\"QDockWidget\" name=\"dock01\">		\
      <property name=\"sizePolicy\">				\
      <sizepolicy hsizetype=\"Preferred\" vsizetype=\"Preferred\">	\
      <horstretch>0</horstretch>					\
      <verstretch>0</verstretch>					\
      </sizepolicy>							\
      </property>							\
      <property name=\"minimumSize\">					\
      <size>								\
      <width>770</width>						\
      <height>80</height>						\
      </size>								\
      </property>							\
      <property name=\"windowTitle\">					\
      <string/>								\
      </property>							\
      <widget class=\"QWidget\" name=\"dockWidgetContents\">		\
      <widget class=\"QWidget\" name=\"\">				\
      <property name=\"geometry\">					\
      <rect>								\
      <x>12</x>								\
      <y>13</y>								\
      <width>735</width>						\
      <height>46</height>						\
      </rect>								\
      </property>							\
      <layout class=\"QGridLayout\" name=\"gridLayout\">		\
      <item row=\"0\" column=\"0\">					\
      <widget class=\"QPushButton\" name=\"PrevBaseline\">		\
      <property name=\"text\">						\
      <string>Prev Baseline</string>					\
      </property>							\
      </widget>								\
      </item>								\
      <item row=\"0\" column=\"1\">					\
      <widget class=\"QPushButton\" name=\"NextBaseline\">		\
      <property name=\"text\">						\
      <string>Next Baseline</string>					\
      </property>							\
      </widget>								\
      </item>								\
      <item row=\"0\" column=\"2\">					\
      <layout class=\"QVBoxLayout\" name=\"verticalLayout\">		\
      <property name=\"spacing\">					\
      <number>0</number>						\
      </property>							\
      <property name=\"sizeConstraint\">				\
      <enum>QLayout::SetMinimumSize</enum>				\
      </property>							\
      <item>								\
      <widget class=\"QCheckBox\" name=\"FixAntenna1\">			\
      <property name=\"text\">						\
      <string>Fix Antenna1</string>					\
      </property>							\
      </widget>								\
      </item>								\
      <item>								\
      <widget class=\"QCheckBox\" name=\"FixAntenna2\">			\
      <property name=\"text\">						\
      <string>Fix Antenna2</string>					\
      </property>							\
      </widget>								\
      </item>								\
      </layout>								\
      </item>								\
      <item row=\"0\" column=\"3\">					\
      <widget class=\"QPushButton\" name=\"NextScan\">			\
      <property name=\"text\">						\
      <string>Next Scan</string>					\
      </property>							\
      </widget>								\
      </item>								\
      <item row=\"0\" column=\"4\">					\
      <widget class=\"QPushButton\" name=\"NextSpw\">			\
      <property name=\"text\">						\
      <string>Next SPW</string>						\
      </property>							\
      </widget>								\
      </item>								\
      <item row=\"0\" column=\"5\">					\
      <widget class=\"QPushButton\" name=\"NextField\">			\
      <property name=\"text\">						\
      <string>Next Field</string>					\
      </property>							\
      </widget>								\
      </item>								\
      <item row=\"0\" column=\"6\">					\
      <widget class=\"QPushButton\" name=\"StopDisplay\">		\
      <property name=\"text\">						\
      <string>Stop Display</string>					\
      </property>							\
      </widget>								\
      </item>								\
      <item row=\"0\" column=\"7\">					\
      <widget class=\"QPushButton\" name=\"Quit\">			\
      <property name=\"text\">						\
      <string>Quit</string>						\
      </property>							\
      </widget>								\
      </item>								\
      </layout>								\
      </widget>								\
      </widget>								\
      </widget>								\
      <resources/>							\
      <connections/>							\
      </ui>								\
      ";
      
      
    */
    
  }// end of SetLayout
  
  
} //# NAMESPACE CASA - END


