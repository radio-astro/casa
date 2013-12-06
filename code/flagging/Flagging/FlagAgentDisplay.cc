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
        		FlagAgentBase(dh,config,ANTENNA_PAIRS_INTERACTIVE,writePrivateFlagCube),
        		dataplotter_p(NULL),reportplotter_p(NULL),
        		userChoice_p("Continue"), userFixA1_p(""), userFixA2_p(""),
        		skipScan_p(-1), skipSpw_p(-1), skipField_p(-1),pause_p(False),
        		fieldId_p(-1), fieldName_p(""), scanStart_p(-1), scanEnd_p(-1), spwId_p(-1),
        		nPolarizations_p(1), freqList_p(Vector<Double>()),
        		antenna1_p(""),antenna2_p(""),
        		dataDisplay_p(False), reportDisplay_p(False),reportFormat_p("screen"),
        		stopAndExit_p(False),reportReturn_p(False),showBandpass_p(False)
{
	// Parse parameters and set base variables.
	setAgentParameters(config);
	// Request loading polarization map to FlagDataHandler
	flagDataHandler_p->setMapPolarizations(true);
	// Make the list of colours (these are almost all predefined ones for Qt.
	// Can add more later, based on RGB values.
	plotColours_p.resize(13);
	plotColours_p[0]="blue";
	plotColours_p[1]="red";
	plotColours_p[2]="green";
	plotColours_p[3]="cyan";
	plotColours_p[4]="darkGray";
	plotColours_p[5]="magenta";
	plotColours_p[6]="yellow";
	plotColours_p[7]="darkBlue";
	plotColours_p[8]="darkRed";
	plotColours_p[9]="darkGreen";
	plotColours_p[10]="darkCyan";
	plotColours_p[11]="black";
	plotColours_p[12]="darkMagenta";

}


FlagAgentDisplay::~FlagAgentDisplay()
{
	// Compiler automagically calls FlagAgentBase::~FlagAgentBase()

	if(dataplotter_p != NULL) { dataplotter_p->done(); dataplotter_p=NULL; }
	if(reportplotter_p != NULL) { reportplotter_p->done(); reportplotter_p=NULL; }
}

void FlagAgentDisplay::setAgentParameters(Record config)
{
	logger_p->origin(LogOrigin(agentName_p,__FUNCTION__,WHERE));
	int exists;

	exists = config.fieldNumber ("pause");
	if (exists >= 0)
	{
		if( config.type(exists) != TpBool )
		{
			throw( AipsError ( "Parameter 'pause' must be of type 'bool'" ) );
		}

		pause_p = config.asBool("pause");
	}
	else
	{
		pause_p = True;
	}

	*logger_p << LogIO::NORMAL << " pause is " << pause_p << LogIO::POST;

	exists = config.fieldNumber ("datadisplay");
	if (exists >= 0)
	{
		if( config.type(exists) != TpBool )
		{
			throw( AipsError ( "Parameter 'datadisplay' must be of type 'bool'" ) );
		}

		dataDisplay_p = config.asBool("datadisplay");
	}
	else
	{
		dataDisplay_p = False;
	}

	*logger_p << LogIO::NORMAL << " datadisplay is " << dataDisplay_p << LogIO::POST;

	exists = config.fieldNumber ("reportdisplay");
	if (exists >= 0)
	{
		if( config.type(exists) != TpBool )
		{
			throw( AipsError ( "Parameter 'reportdisplay' must be of type 'bool'" ) );
		}

		reportDisplay_p = config.asBool("reportdisplay");
	}
	else
	{
		reportDisplay_p = False;
	}

	*logger_p << LogIO::NORMAL << " reportdisplay is " << reportDisplay_p << LogIO::POST;


	exists = config.fieldNumber ("format");
	if (exists >= 0)
	{
		if( config.type(exists) != TpString )
		{
			throw( AipsError ( "Parameter 'format' must be of type 'bool'" ) );
		}

		reportFormat_p = config.asString("format");
		if( reportFormat_p != "screen" && reportFormat_p != "file")
		{
			throw( AipsError( "Unsupported report format : " + reportFormat_p + ". Supported formats are 'screen' and 'file'") );
		}
	}
	else
	{
		reportFormat_p = String("screen");
	}

	*logger_p << LogIO::NORMAL << " format is " << reportFormat_p << LogIO::POST;

}


void
FlagAgentDisplay::preProcessBuffer(const vi::VisBuffer2 &visBuffer)
{

	getChunkInfo(visBuffer);

	return;
}


void
FlagAgentDisplay::iterateAntennaPairsInteractive(antennaPairMap *antennaPairMap_ptr)
{
	logger_p->origin(LogOrigin(agentName_p,__FUNCTION__,WHERE));
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
			if(skipSpw_p == spwId_p) {dataDisplay_p=False;} // Skip the rest of this SPW
			else {skipSpw_p = -1; dataDisplay_p=True;} // Reached next SPW. Reset state
		}
		if(skipField_p != -1)
		{
			if(skipField_p == fieldId_p) {dataDisplay_p=False;} // Skip the rest of this Field
			else {skipField_p = -1; dataDisplay_p=True;} // Reached next Field. Reset state
		}
		if(skipScan_p != -1)
		{
			if(skipScan_p == scanEnd_p) {dataDisplay_p=False;} // Skip the rest of this Scan
			else {skipScan_p = -1; dataDisplay_p=True;} // Reached next Scan. Reset state
		}


		// Display this baseline
		if(dataDisplay_p)
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
						*logger_p << "No Previous baseline in this chunk with Ant1 : "
								<< ( (userFixA1_p==String(""))?String("any"):userFixA1_p )
								<< "  and Ant2 : "
								<< ( (userFixA2_p==String(""))?String("any"):userFixA2_p )
								<< LogIO::POST;

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
					dataDisplay_p = False;
					stopAndExit_p = True;
					*logger_p << "Exiting flagger" << LogIO::POST;
					if(dataplotter_p!=NULL) { dataplotter_p->done(); dataplotter_p=NULL; }
					flagDataHandler_p->stopIteration();
					return ;
				}
				else if(userChoice_p=="StopDisplay")
				{
					dataDisplay_p = False;
					*logger_p << "Stopping display. Continuing flagging." << LogIO::POST;
					if(dataplotter_p!=NULL) { dataplotter_p->done(); dataplotter_p=NULL; }
				}
				else if(userChoice_p=="PrevBaseline")
				{
					if( myAntennaPairMapIterator==antennaPairMap_ptr->begin() )
						*logger_p << "Already on first baseline..." << LogIO::POST;
					stepback=True;
				}
				else if(userChoice_p=="NextScan")
				{
					//*logger_p << "Next Scan " << LogIO::POST;
					skipScan_p = scanEnd_p;
				}
				else if(userChoice_p=="NextSpw")
				{
					//*logger_p << "Next SPW " << LogIO::POST;
					skipSpw_p = spwId_p;
				}
				else if(userChoice_p=="NextField")
				{
					//*logger_p << "Next Field " << LogIO::POST;
					skipField_p = fieldId_p;
				}
				else if(userChoice_p=="Continue")
				{
					//*logger_p << "Next chunk " << LogIO::POST; // Right now, a chunk is one baseline !
					return;
				}

			}// end if pause=True

		}// if dataDisplay_p

	}// end antennaMapIterator

	return;
}// end iterateAntennaPairsInteractive


Bool
FlagAgentDisplay::skipBaseline(std::pair<Int,Int> antennaPair)
{
	String antenna1Name = flagDataHandler_p->antennaNames_p->operator()(antennaPair.first);
	String antenna2Name = flagDataHandler_p->antennaNames_p->operator()(antennaPair.second);
	//	    if(userFixA2_p != "") cout << "*********** userfixa2 : " << userFixA2_p << "   thisant : " << antenna1Name << " && " << antenna2Name << LogIO::POST;
	return  (  (userFixA1_p != ""  && userFixA1_p != antenna1Name)  ||   (userFixA2_p != ""  && userFixA2_p != antenna2Name) ) ;
}

bool
FlagAgentDisplay::computeAntennaPairFlags(const vi::VisBuffer2 &visBuffer,
		VisMapper &visibilities,FlagMapper &flags,Int antenna1,Int antenna2,
		vector<uInt> &/*rows*/)
{
	logger_p->origin(LogOrigin(agentName_p,__FUNCTION__,WHERE));
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
	String spwName = String::toString(visBuffer.spectralWindows()(0));

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
	/*
    Vector<uInt> polarizations = flags.getSelectedCorrelations();
    nPolarizations_p = polarizations.size();
    polarizationIndexMap *polMap = flagDataHandler_p->getPolarizationIndexMap();
    Vector<String> corrTypes(nPolarizations_p);
    for(uInt pol=0;pol<nPolarizations_p;pol++)
      corrTypes[pol] = (*polMap)[polarizations[pol]];
	 */

	// Get Polarization Maps
	// jagonzal: Migrated to new implementation of multiple expressions handling
	vector<string> corrTypes = visibilities.getSelectedCorrelationStrings();
	nPolarizations_p = corrTypes.size();

	///cout << "Selected Correlations : " << polarizations << LogIO::POST;

	// Print where we are...
	//    *logger_p << LogIO::NORMAL  << " Baseline : " << baselineName << " Field : " << fieldName_p << " Spw : " << spwName << "  nChan : " << nChannels << " nPol : " << nPolarizations_p << " nTime : " << nTimes << LogIO::POST;

	// Build the Plot Window for the first time
	if(dataDisplay_p && dataplotter_p==NULL) buildDataPlotWindow();

	// Adding this block of code as a fix for CAS-4052.
	// J.G. : If you find a better way to do this, please change this...
	if(dataDisplay_p==True && dataplotter_p!=NULL)
	{
		Int nrows;
		if(showBandpass_p==True) nrows=3;
		else nrows=2;

		if(panels_p.nelements() != nPolarizations_p*nrows)
		{
			//cout << "Wrong number of panels !" << endl;
			dataplotter_p->done();
			dataplotter_p=NULL;
			buildDataPlotWindow();
		}
	}

	// Initialize Plot Arrays and other vars
	Float runningsum=0, runningflag=0,runningpreflag=0;
	Vector<Float> vecflagdat(0), vecdispdat(0);
	Vector<Float> origspectrum(0), flagspectrum(0), precountspec(0), countspec(0);
	if(dataDisplay_p)
	{
		vecflagdat.resize(nChannels * nTimes); vecdispdat.resize(nChannels * nTimes);
		origspectrum.resize(nChannels); flagspectrum.resize(nChannels);
		precountspec.resize(nChannels); countspec.resize(nChannels);
	}

	if(dataDisplay_p)
	{
		// Make and send plots for each polarization
		for(int pl=0;pl<(int) nPolarizations_p;pl++)  // Start Correlation Loop
		{
			runningsum=0; runningflag=0;  runningpreflag=0;
			origspectrum=0.0; flagspectrum=0.0; precountspec=0.0; countspec=0.0;
			for(int ch=0;ch<(int) nChannels;ch++)  // Start Channel Loop
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
			dataplotter_p->setlabel(" ",pl?" ":"Time",ostr1.str(),panels_p[pl].getInt());
			DisplayRaster(nChannels,nTimes,vecflagdat,panels_p[pl+nPolarizations_p].getInt());
			dataplotter_p->setlabel("Frequency",pl?" ":"Time",ostr2.str(),panels_p[pl+nPolarizations_p].getInt());

			if(showBandpass_p==True)
			{

				// Make the Before/After bandpass plots
				for(uInt ch=0;ch<nChannels;ch++)
				{
					if(precountspec[ch]==0) {origspectrum[ch]=0.0; precountspec[ch]=1.0;}
					if(countspec[ch]==0) {flagspectrum[ch]=0.0; countspec[ch]=1.0;}
				}

				origspectrum = (origspectrum/precountspec);
				flagspectrum = (flagspectrum/countspec);
				AlwaysAssert( freqList_p.nelements()==nChannels , AipsError);

				DisplayLine(nChannels, freqList_p, origspectrum, String("before:")+String(corrTypes[pl]),
						String("red"), False, panels_p[pl+(2*nPolarizations_p)].getInt());
				DisplayScatter(nChannels, freqList_p, flagspectrum, String("after:")+String(corrTypes[pl]),
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
			}// end of if (showBandPass_p)

		}//End Correlation Loop

		//*logger_p << LogIO::POST;

	}// end if dataDisplay_p

	return false;
}// end computeAntennaPairFlags

//----------------------------------------------------------------------------------------------------------

void
FlagAgentDisplay::getChunkInfo(const vi::VisBuffer2 &visBuffer)
{
	logger_p->origin(LogOrigin(agentName_p,__FUNCTION__,WHERE));
	// Read current Field name, SPW id, and scan info.
	fieldId_p = visBuffer.fieldId()(0);
	fieldName_p = flagDataHandler_p->fieldNames_p->operator()(fieldId_p);
	spwId_p = visBuffer.spectralWindows()(0);
	scanStart_p = visBuffer.scan()[0];
	scanEnd_p = visBuffer.scan()[ (visBuffer.scan().nelements())-1 ];

	*logger_p << LogIO::NORMAL << "FlagAgentDisplay::" << __FUNCTION__ << " Field : " << fieldId_p << " , " << fieldName_p << " Spw : " << spwId_p << " Scan : " << scanStart_p << " : " << scanEnd_p << LogIO::POST;
}

//----------------------------------------------------------------------------------------------------------

FlagReport
FlagAgentDisplay::getReport()
{
	logger_p->origin(LogOrigin(agentName_p,__FUNCTION__,WHERE));

	//       FlagReport dispRep("plot",agentName_p);

	// Make empty list
	FlagReport dispRep("list");

	/*

       // Make sample arrays/vectors
       Int N=10;
       Array<Float> sample( IPosition(2, N, N) );
       sample = 0.0;
       sample( IPosition(2,N/2,N/2)) = 1.0;
       Vector<Float> xdata( N ), ydata( N ), error ( N );
       for(Int i=0;i<N;i++) {xdata[i]=i;}
       ydata = 1.0;

       // (1) Make a raster plot. Only one set of data is allowed here.
       FlagReport subRep0 = FlagReport("plotraster",agentName_p,"example raster", "xaxis", "yaxis");
       subRep0.addData(sample); // add 2D data

       // Add this raster FlagReport to the list.
       dispRep.addReport( subRep0 ); 

       // (2) Make a line plot. Can give multiple lines to overlay on the same panel.
       FlagReport subRep1 = FlagReport("plotpoints",agentName_p,"example line", "xaxis", "yaxis");
       subRep1.addData("line", xdata,ydata,"",Vector<Float>(),"line 1"); // add first set of line data
       ydata[N/2]=2.0;
       subRep1.addData("scatter", xdata,ydata,"",Vector<Float>(),"scatter 2"); // add second set of line data to overlay

       // Add this line FlagReport to the list
       dispRep.addReport( subRep1 ); 

       // (3) Make an overlay of a line with errorbar, scatter with errorbar, and scatter with circle
       FlagReport subRep2 = FlagReport("plotpoints",agentName_p,"example line", "xaxis", "yaxis");
       for(Int i=0;i<N;i++) {error[i]=i;}
       subRep2.addData("line", xdata,ydata,"bar",error,"line+bar"); // add first set of line data, with errorbars
       for(Int i=0;i<N;i++) {xdata[i] += 0.3; error[i]=i; ydata[i]+=2.0;}
       subRep2.addData("scatter", xdata, ydata,"bar",error,"scatter+bar"); // add second set of scatter data to overlay, with error bars
       for(Int i=0;i<N;i++) {xdata[i] += 0.3; error[i]=i*10; ydata[i]+=2.0;}
       subRep2.addData("scatter", xdata, ydata,"circle",error,"scatter+circle"); // add third set, scatter data with circles.

       // Add this line FlagReport to the list
       dispRep.addReport( subRep2 ); 

       if( ! dispRep.verifyFields() )
	 cout << "Problem ! " << endl;


	 */

	return dispRep;
}// end of getReport()

//----------------------------------------------------------------------------------------------------------
// Go through the list of reports and make plots
Bool
FlagAgentDisplay::displayReports(FlagReport &combinedReport)
{
	logger_p->origin(LogOrigin(agentName_p,__FUNCTION__,WHERE));

	if(reportDisplay_p && !stopAndExit_p)
	{
		Int nReports = combinedReport.nReport();

		if(dataplotter_p!=NULL) { dataplotter_p->done(); dataplotter_p=NULL; }
		if(nReports>0 && reportplotter_p==NULL) buildReportPlotWindow();

		Bool stepback=False;

		for (Int reportid=0; reportid<nReports; reportid++)
		{
			String agentName, title, xlabel, ylabel;
			FlagReport oneRep;

			if(stepback==True)
			{
				// Go back until a valid one is found.
				Int previd=-1;
				for( Int rep=reportid-1; rep>=0; --rep)
				{
					Bool valid = combinedReport.accessReport(rep,oneRep);
					if(valid)
					{
						String type = oneRep.reportType();
						if( type=="plotraster" || type=="plotpoints" )
						{
							previd=rep; break;
						}
					}
				}

				if(previd==-1)
				{
					*logger_p << "Already on first plot" << LogIO::POST;
					reportid=0;
				}
				else
				{
					reportid = previd;
				}

				stepback=False;
			}


			Bool valid = combinedReport.accessReport(reportid,oneRep);
			if(valid)
			{
				oneRep.get( RecordFieldId("name") , agentName );
				String type = oneRep.reportType();

				if( type=="plotraster" || type=="plotpoints" )
				{

					*logger_p << reportid << " : " << type << " with " << oneRep.nData() << " layer(s) " << " from " << agentName << LogIO::POST;

					if( type == "plotraster" )
					{
						oneRep.get( RecordFieldId("title") , title );
						oneRep.get( RecordFieldId("xlabel") , xlabel );
						oneRep.get( RecordFieldId("ylabel") , ylabel );

						Array<Float> data;
						oneRep.get( RecordFieldId("data"+String::toString(0)) , data );

						Vector<Float> flatdata(data.reform(IPosition(1,data.shape()[0]*data.shape()[1])));

						reportplotter_p->erase(report_panels_p[0].getInt() );
						reportplotter_p->raster(dbus::af(flatdata), data.shape()[0] ,data.shape()[1], "Hot Metal 1", report_panels_p[0].getInt());
						reportplotter_p->setlabel(xlabel,ylabel,title,report_panels_p[0].getInt());

					}
					else if( type == "plotpoints")
					{
						oneRep.get( RecordFieldId("title") , title );
						oneRep.get( RecordFieldId("xlabel") , xlabel );
						oneRep.get( RecordFieldId("ylabel") , ylabel );

						reportplotter_p->erase(report_panels_p[0].getInt() );

						Int ndata = oneRep.nData();
						for(Int datid=0;datid<ndata;datid++)
						{
							Vector<Float> xdata,ydata,error;
							String legendlabel,plottype="line",errortype="";
							oneRep.get( RecordFieldId("xdata"+String::toString(datid)) , xdata );
							oneRep.get( RecordFieldId("ydata"+String::toString(datid)) , ydata );
							oneRep.get( RecordFieldId("label"+String::toString(datid)) , legendlabel );
							oneRep.get( RecordFieldId("plottype"+String::toString(datid)) , plottype );

							///reportplotter_p->line(dbus::af(xdata), dbus::af(ydata),(datid%2)?String("red"):String("blue"),legendlabel, report_panels_p[0].getInt());

							if ( oneRep.isDefined( "error"+String::toString(datid)  ) )
							{
								oneRep.get( RecordFieldId("error"+String::toString(datid)) , error );
								oneRep.get( RecordFieldId("errortype"+String::toString(datid)) , errortype );
							}

							DisplayLineScatterError(reportplotter_p , plottype, xdata, ydata, errortype, error, legendlabel, plotColours_p[datid%plotColours_p.nelements()], report_panels_p[0].getInt() );
							/// (datid%2)?String("red"):String("blue")
						}// end of for datid

						reportplotter_p->setlabel(xlabel,ylabel,title,report_panels_p[0].getInt());

					}// end of plotpoints
					else
					{
						*logger_p << "NO Display for : " << reportid << " : " << agentName << LogIO::POST;
					}

					getReportUserInput();

					// React to user-input
					if(userChoice_p=="Quit")
					{
						dataDisplay_p = False;
						stopAndExit_p = True;
						//*logger_p << "Exiting flagger" << LogIO::POST;
						if(reportplotter_p!=NULL) { reportplotter_p->done(); reportplotter_p=NULL; }
						return True;
					}
					else if(userChoice_p=="Prev")
					{
						//*logger_p << "Prev Plot" << LogIO::POST;
						if( reportid==0 )
							*logger_p << "Already on first plot..." << LogIO::POST;
						else
							--reportid;
						stepback=True;
					}
					else if(userChoice_p=="Continue" || userChoice_p=="Next")
					{
						//*logger_p << "Next Plot " << LogIO::POST;
						if( reportid==nReports-1 )
						{
							*logger_p << "Already on last plot..." << LogIO::POST;
							--reportid;
						}
					}

				}// if valid plot type
				else
				{
					//*logger_p  <<  "No plot for Report : " << reportid << LogIO::POST;
					*logger_p << reportid << " : No plot for report from " << agentName << LogIO::POST;

				}
			}// if valid plot record.
			else
			{
				*logger_p << LogIO::WARN <<  "Invalid Plot Record for : " << reportid << LogIO::POST;
			}

		}// end of for-report-in-combinedReport

	}// end of reportDisplay_p==True
	else
	{
		*logger_p << "Report Displays are turned OFF " << LogIO::POST;
	}
	return True;
} // end of displayReports()

/***********************************************************************/
/******************     Plot Functions      ******************************/
/***********************************************************************/

// Note : By default, only two rows of plots are created.
// The third row, for bandpass plots (before and after) are turned off
// by the private variable 'showBandPass_p'. This can be later
// enabled when we get per-chunk 'extra info' displays/reports from
// the agents. Right now, it's redundant info, and takes up too much
// space on small laptop screens.
Bool FlagAgentDisplay::buildDataPlotWindow()
{

	setDataLayout();
	AlwaysAssert( dock_xml_p != NULL , AipsError );

	dataplotter_p = dbus::launch<FlagPlotServerProxy>();

	Int nrows;
	if(showBandpass_p==True) nrows=3;
	else nrows=2;
	panels_p.resize(nPolarizations_p*nrows);
	string zoomloc="";
	string legendloc="bottom";

	// First row : Data with no flags
	panels_p[0] = dataplotter_p->panel( "", "", "", "",
			std::vector<int>( ),legendloc,zoomloc,0,false,false);
	if(nPolarizations_p>1)
	{
		for(Int i=1;i<(int) nPolarizations_p;i++)
		{
			panels_p[i] = dataplotter_p->panel( "", "", "", "",
					std::vector<int>( ),legendloc,zoomloc,panels_p[i-1].getInt(),false,false);
		}
	}


	// Second row : Data with flags
	panels_p[nPolarizations_p] = dataplotter_p->panel( "", "", "", "",
			std::vector<int>( ),legendloc,zoomloc, panels_p[0].getInt(),true,false);
	if(nPolarizations_p>1)
	{
		for(int i=static_cast<int>(nPolarizations_p)+1;i<2*static_cast<int>(nPolarizations_p);i++)
		{
			panels_p[i] = dataplotter_p->panel( "", "", "", "",
					std::vector<int>( ),legendloc,zoomloc, panels_p[i-1].getInt(),false,false);
		}
	}

	// Third row : Average Bandpass
	if(showBandpass_p==True)
	{
		panels_p[2*nPolarizations_p] = dataplotter_p->panel( "", "", "", "",
				std::vector<int>( ),legendloc,zoomloc, panels_p[0].getInt(),true,false);

		if(nPolarizations_p>1)
		{
			for(int i=2* static_cast<int>(nPolarizations_p)+1;i<3*static_cast<int>(nPolarizations_p);i++)
			{
				panels_p[i] = dataplotter_p->panel( "", "", "", "",
						std::vector<int>( ),legendloc,zoomloc, panels_p[i-1].getInt(),false,false);
			}
		}
	}// if showbandpass

	/*
    // Misc panel
    panels_p[8] = dataplotter_p->panel( "BandPass", "Frequency", "Amp", "",
    std::vector<int>( ),legendloc,zoomloc, panels_p[3].getInt(),false,false);

    // Dummy panel
    panels_p[9] = dataplotter_p->panel( "---", "----", "---", "",
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
	dataplotter_p->loaddock(dock_xml_p,"bottom",loc,panels_p[0].getInt());

	return True;

}// end buildDataPlotWindow


Bool FlagAgentDisplay::buildReportPlotWindow()
{

	setReportLayout();
	AlwaysAssert( report_dock_xml_p != NULL , AipsError );

	reportplotter_p = dbus::launch<FlagPlotServerProxy>();

	report_panels_p.resize(1);
	string zoomloc="";
	string legendloc="bottom";
	report_panels_p[0] = reportplotter_p->panel( "", "", "", "",
			std::vector<int>( ),legendloc,zoomloc,0,false,false);

	for (uInt i=0; i<report_panels_p.nelements(); i++)
	{
		if ( report_panels_p[i].type( ) != dbus::variant::INT ) {
			throw( AipsError("Error in panel construction") );
		}
	}


	std::vector<std::string> loc;
	loc.push_back("top");
	reportplotter_p->loaddock(report_dock_xml_p,"bottom",loc,report_panels_p[0].getInt());

	return True;

}// end buildReportPlotWindow



void FlagAgentDisplay :: getUserInput()
{
	userChoice_p = "Continue";
	String returnvalue;

	Bool exitEventLoop=False;

	while( ! exitEventLoop)
	{

		returnvalue = dataplotter_p->eventloop();
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
		else *logger_p << LogIO::DEBUG2 << "Unknown GUI choice : " << returnvalue << LogIO::POST;

		//    *logger_p << "ReturnValue : " << returnvalue << "   userChoice : " << userChoice_p << "  userFixA1 : " << userFixA1_p << "  userFixA2 : " << userFixA2_p << LogIO::POST;

	}


	return;
}



void FlagAgentDisplay :: getReportUserInput()
{
	userChoice_p = "Continue";
	String returnvalue;

	Bool exitEventLoop=False;

	while( ! exitEventLoop)
	{

		returnvalue = reportplotter_p->eventloop();
		if(returnvalue == "Next" || returnvalue == "Prev" || returnvalue == "Quit")
		{
			userChoice_p = returnvalue;
			exitEventLoop=True;
		}
		else *logger_p << LogIO::DEBUG2 << "Unknown GUI choice (Not sure why eventloop is exiting without user-click... re-entering... )" << returnvalue << LogIO::POST;

		//*logger_p << "ReturnValue : " << returnvalue << "   userChoice : " << userChoice_p  << LogIO::POST;

	}


	return;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////   Plot functions
//////////////////////////////////////////////////////////////////////////////////////////////////////

void FlagAgentDisplay::DisplayRaster(Int xdim, Int ydim, Vector<Float> &data, uInt frame)
{
	if(data.nelements() != (uInt) xdim*ydim)
	{
		logger_p->origin(LogOrigin(agentName_p,__FUNCTION__,WHERE));
		*logger_p << LogIO::WARN << "Error in data XY dimensions. Not plotting" << LogIO::POST;
		return;
	}
	//    cout << "panel id : " << frame << endl;

	// dataplotter_p->release( panel_p.getInt( ) );
	dataplotter_p->erase( frame );

	//	dataplotter_p->line(x, y, "blue", "time", panel_p.getInt() );
	dataplotter_p->raster(dbus::af(data), xdim,ydim, "Hot Metal 1", frame);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
void FlagAgentDisplay::DisplayLine(Int /*xdim*/, Vector<Double> &xdata, Vector<Float> &ydata, String label, String color, Bool hold, uInt frame)
{
	if(xdata.nelements() != ydata.nelements())
	{
		logger_p->origin(LogOrigin(agentName_p,__FUNCTION__,WHERE));
		*logger_p << LogIO::WARN << "X and Y plot data have different sizes. Not plotting " << LogIO::POST;
		return;
	}
	if( hold==False ) dataplotter_p->erase( frame );
	dataplotter_p->line(dbus::af(xdata), dbus::af(ydata),color,label,frame);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////

void FlagAgentDisplay::DisplayScatter(Int /*xdim*/, Vector<Double> &xdata, Vector<Float> &ydata, String label, String color, Bool hold, uInt frame)
{
	if(xdata.nelements() != ydata.nelements())
	{
		logger_p->origin(LogOrigin(agentName_p,__FUNCTION__,WHERE));
		*logger_p << LogIO::WARN << "X and Y plot data have different sizes. Not plotting " << LogIO::POST;
		return;
	}
	if( hold==False ) dataplotter_p->erase( frame );
	dataplotter_p->scatter(dbus::af(xdata), dbus::af(ydata),color,label,"dot",1,4,frame);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////

// Display scatter dots, with optional vertical error-bars, or circles or text labels, or all together.
// Send in vectors of size 0 for radii and pointlabels and error to disable them.
void FlagAgentDisplay::DisplayLineScatterError(FlagPlotServerProxy *&plotter, String &plottype, Vector<Float> &xdata, Vector<Float> &ydata, String &errortype, Vector<Float> &error, String label, String color, uInt frame)
{
	// Check X and Y shapes
	if(xdata.nelements() != ydata.nelements())
	{
		logger_p->origin(LogOrigin(agentName_p,__FUNCTION__,WHERE));
		*logger_p << LogIO::WARN << "X, Y plot data have different sizes. Not plotting." << LogIO::POST;
		return;
	}

	///    if( hold==False ) plotter->erase( frame );

	if(plottype==String("scatter"))
	{
		// Scatter-plot of X and Y
		plotter->scatter(dbus::af(xdata), dbus::af(ydata),color,label,"dot",1,4,frame);
	}
	else if (plottype==String("line"))
	{
		// Line plot of X and Y
		plotter->line(dbus::af(xdata), dbus::af(ydata),color,label,frame);
	}
	else
	{
		return;
	}

	// Check Error shape ( if not zero, must match xdata,ydata)
	if(error.nelements()>0)
	{
		if(error.nelements()!=xdata.nelements())
		{
			logger_p->origin(LogOrigin(agentName_p,__FUNCTION__,WHERE));
			*logger_p << LogIO::WARN << "Number of Error elements is different from data. Not plotting." << LogIO::POST;
			return;
		}

		if(errortype == "bar")
		{
			Vector<Double>pointerror(2);
			Vector<Double>xval(2);
			for ( Int onepoint=0; onepoint<(Int) error.nelements(); onepoint++)
			{
				xval[0]=xdata[onepoint];
				xval[1]=xdata[onepoint];
				pointerror[0]=ydata[onepoint] + error[onepoint];
				pointerror[1]=ydata[onepoint] - error[onepoint];
				plotter->line(dbus::af(xval), dbus::af(pointerror), color,"",frame);
			}
		}
		else if(errortype == "separator")
		{
			Vector<Double>pointerror(2);
			Vector<Double>xval(2);
			for ( Int onepoint=0; onepoint<(Int) error.nelements(); onepoint++)
			{
				xval[0]=xdata[onepoint];
				xval[1]=xdata[onepoint];
				pointerror[0]=ydata[onepoint] + error[onepoint];
				pointerror[1]=0;
				plotter->line(dbus::af(xval), dbus::af(pointerror),"black","",frame);
			}
		}
		else if(errortype == "circle")
		{
			// Check Circle shape ( if not zero, must match xdata,ydata)
			Vector<Double>xval(1),yval(1);
			for ( Int onepoint=0; onepoint<(Int) error.nelements(); onepoint++)
			{
				xval[0]=xdata[onepoint];
				yval[0]=ydata[onepoint];
				// function signature should allow a float for the error bar
				plotter->scatter(dbus::af(xval), dbus::af(yval),color,"","ellipse",static_cast<Int>(error[onepoint]),4,frame);
				// Note : I think this plots the centre point too. If so, it's a repeat....
			}
		}
		else
		{
			logger_p->origin(LogOrigin(agentName_p,__FUNCTION__,WHERE));
			*logger_p << LogIO::WARN << "Unknown error-plot type. Not plotting." << LogIO::POST;
		}
	}// if error

	// Do a similar thing for labels.
	// NOTE : Cannot plot point-labels or text strings yet.

}
//////////////////////////////////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////     GUI Layout Functions
//////////////////////////////////////////////////////////////////////////////////////////////////////


Bool FlagAgentDisplay :: setDataLayout()
{

	dock_xml_p = "\
<?xml version=\"1.0\" encoding=\"UTF-8\"?>\
<ui version=\"4.0\">\
 <class>dock01</class>\
 <widget class=\"QDockWidget\" name=\"dock01\">\
  <property name=\"geometry\">\
   <rect>\
    <x>0</x>\
    <y>0</y>\
    <width>770</width>\
    <height>120</height>\
   </rect>\
  </property>\
  <property name=\"sizePolicy\">\
   <sizepolicy hsizetype=\"Preferred\" vsizetype=\"Preferred\">\
    <horstretch>0</horstretch>\
    <verstretch>0</verstretch>\
   </sizepolicy>\
  </property>\
  <property name=\"minimumSize\">\
   <size>\
    <width>770</width>\
    <height>87</height>\
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
       <layout class=\"QVBoxLayout\" name=\"verticalLayout\">\
        <property name=\"spacing\">\
         <number>0</number>\
        </property>\
        <property name=\"sizeConstraint\">\
         <enum>QLayout::SetMinimumSize</enum>\
        </property>\
        <item>\
         <widget class=\"QCheckBox\" name=\"FixAntenna1\">\
          <property name=\"text\">\
           <string>Fix Antenna1</string>\
          </property>\
         </widget>\
        </item>\
        <item>\
         <widget class=\"QCheckBox\" name=\"FixAntenna2\">\
          <property name=\"text\">\
           <string>Fix Antenna2</string>\
          </property>\
         </widget>\
        </item>\
       </layout>\
      </item>\
      <item row=\"0\" column=\"3\">\
       <widget class=\"QPushButton\" name=\"NextSpw\">\
        <property name=\"text\">\
         <string>Next SPW</string>\
        </property>\
       </widget>\
      </item>\
      <item row=\"0\" column=\"4\">\
       <widget class=\"QPushButton\" name=\"NextScan\">\
        <property name=\"text\">\
         <string>Next Scan</string>\
        </property>\
       </widget>\
      </item>\
      <item row=\"0\" column=\"5\">\
       <widget class=\"QPushButton\" name=\"NextField\">\
        <property name=\"text\">\
         <string>Next Field</string>\
        </property>\
       </widget>\
      </item>\
      <item row=\"0\" column=\"6\">\
       <widget class=\"QPushButton\" name=\"StopDisplay\">\
        <property name=\"text\">\
         <string>Stop Display</string>\
        </property>\
       </widget>\
      </item>\
      <item row=\"0\" column=\"7\">\
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

	return True;

}// end of SetLayout


Bool FlagAgentDisplay :: setReportLayout()
{

	report_dock_xml_p = "\
<?xml version=\"1.0\" encoding=\"UTF-8\"?>\
<ui version=\"4.0\">\
 <class>dock01</class>\
 <widget class=\"QDockWidget\" name=\"dock01\">\
  <property name=\"geometry\">\
   <rect>\
    <x>0</x>\
    <y>0</y>\
    <width>320</width>\
    <height>80</height>\
   </rect>\
  </property>\
  <property name=\"sizePolicy\">\
   <sizepolicy hsizetype=\"Preferred\" vsizetype=\"Preferred\">\
    <horstretch>0</horstretch>\
    <verstretch>0</verstretch>\
   </sizepolicy>\
  </property>\
  <property name=\"minimumSize\">\
   <size>\
    <width>320</width>\
    <height>80</height>\
   </size>\
  </property>\
  <property name=\"windowTitle\">\
   <string/>\
  </property>\
  <widget class=\"QWidget\" name=\"dockWidgetContents\">\
   <property name=\"enabled\">\
    <bool>true</bool>\
   </property>\
   <property name=\"sizePolicy\">\
    <sizepolicy hsizetype=\"Preferred\" vsizetype=\"Preferred\">\
     <horstretch>0</horstretch>\
     <verstretch>0</verstretch>\
    </sizepolicy>\
   </property>\
   <layout class=\"QGridLayout\" name=\"gridLayout\">\
    <item row=\"0\" column=\"0\">\
     <layout class=\"QHBoxLayout\" name=\"horizontalLayout\">\
      <item>\
       <widget class=\"QPushButton\" name=\"Prev\">\
        <property name=\"text\">\
         <string>Prev</string>\
        </property>\
       </widget>\
      </item>\
      <item>\
       <widget class=\"QPushButton\" name=\"Next\">\
        <property name=\"text\">\
         <string>Next</string>\
        </property>\
       </widget>\
      </item>\
      <item>\
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

	return True;

}


} //# NAMESPACE CASA - END


