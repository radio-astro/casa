/***
 * Framework independent implementation file for tableplot...
 *
 * Implement the tableplot component here.
 * 
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 ***/


#include <iostream>
#include <string.h>
#include <cmath>
#include <tableplot_cmpt.h>
#include <casa/aips.h>
#include <tools/tables/TablePlot/TablePlot.h>

#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/MatrixMath.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Matrix.h>

#include <casa/namespace.h>

#include <casa/Exceptions/Error.h>
#include <casa/Logging/LogOrigin.h>
#include <casa/Logging/LogIO.h>
#include <casa/OS/DOos.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/TableLock.h>
#include <tables/TaQL/TableParse.h>
#include <casa/System/ObjectID.h>
#include <casa/Utilities/Assert.h>
#include <casa/BasicSL/String.h>

using namespace std;
using namespace casa;

namespace casac {

/* Derived Class for conversion functions for addition */
class TPConvertAdd : public TPConvertBase
{
	public :
		TPConvertAdd(){};
		~TPConvertAdd(){};
		inline Double Xconvert(Double x,Int row, Int tblNum){x += 100;return x;};
		inline Double Yconvert(Double y,Int row, Int tblNum){y += 10;return y;};
};


/* Derived Class for GUI binding */
/*
class TPGuiBinder : public casa::GUIBinderBase
{
	public :
		TPGuiBinder( casac::tableplot* intbp ){tbp_p = intbp;}
		Bool markregion(Int nrows,Int ncols, Int panel, casa::Vector<casa::Double> region)
		{ 
			std::vector<double> l_region(4);
			for(Int i=0;i<4;i++) l_region[i] = region[i];
			return tbp_p->markregions(nrows, ncols, panel, l_region); 
		}
		Bool flagdata(){ return tbp_p->flagdata(); }
		Bool unflagdata(){ return tbp_p->unflagdata(); }
		Bool locatedata()
		{ 
			std::vector<std::string> l_list; l_list.resize(0);
			return tbp_p->locatedata(); 
		}
		Bool iterplotnext(){ return tbp_p->iterplotnext(); }
		Bool iterplotstop(){ return tbp_p->iterplotstop(False); }
		Bool clearplot(){return tbp_p->clearplot(0,0,0); }
                Bool quit(){return tbp_p->done(); }
	private :
		casac::tableplot* tbp_p;
};
*/

/* Default Constructor */
tableplot::tableplot()
{
	try 
	{
		adbg=0;
		if(adbg)cout << "tableplot constructor" << endl;
		nTabs=0;
		TABS.resize(0);
		TabNames.resize(0);
                resetter_p = NULL;
		
		itsLog = new LogIO();
		
		TP = NULL;
		//PyBind = NULL;
		//TP = new TablePlot();
                TP = casa::TablePlot::TablePlotInstance();
		//PyBind = new TPGuiBinder( this );
		//TP->attachGuiBinder(PyBind);

	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		RETHROW(x);
	}
}

/* Destructor */
tableplot::~tableplot()
{
	try 
	{
		if(adbg)cout << "tableplot destructor" << endl;
		done();
		
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		RETHROW(x);
	}
}

/* Open Tables */
bool
tableplot::open(const std::vector<std::string>& tabnames)
{
	if(adbg)cout << "setTables : Set tables from table names " << endl;

	try {
		//if(TP==NULL) TP = new TablePlot();
                if(TP==NULL) TP = TablePlot::TablePlotInstance();
		//if(PyBind==NULL) 
                //{
                 //       PyBind = new TPGuiBinder( this );
                 //       TP->attachGuiBinder(PyBind);
                //}
		/* Convert from vector<string> to Vector<String> */
		temptabnames.resize(0); 
		temptabnames = toVectorString(tabnames);
		nTabs = temptabnames.nelements(); 
		TabNames.resize(nTabs); 
		TabNames = temptabnames;
		Vector<String> SelStr(nTabs);

		if(adbg)cout << "nTabs : " << nTabs << endl;
	
		/* Open tables and attach to internal list */
		if(TP->setTableS(TabNames,TabNames,SelStr)==-1) return False;
		
		/* Record the tables - for use in data selection later on */
		TABS.resize(0);
		TP->getTabVec(TABS);

		SelTABS.resize(0);
		SelTABS = TABS;
		
		alive=1;

                if(resetter_p == NULL)
                        resetter_p = new TPResetCallBack();
                TP->setResetCallBack(String("tableplot"),resetter_p);
                
		
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		RETHROW(x);
	}
	return True;
}
/* Choose a gui / backend */
/* Keep this has a place-holder for choosing plotter backends */
bool
tableplot::setgui(const bool gui)
{
	try {

	    if(!gui)
	 	*itsLog << LogOrigin("tableplot", "setgui") 
               <<"Switching to NO-GUI mode. All current plots will be reset." 
               << LogIO::POST;
	    else
	 	*itsLog << LogOrigin("tableplot", "setgui") 
                << "Switching to GUI mode. All current plots  will be reset." 
                << LogIO::POST;

	    TP->setGui((Bool)gui);	
	    /* Force a new "open" call to be made */
	    alive = 0;
		
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		RETHROW(x);
	}
   return True;
}


// Saving of the plotted to an image file.
bool
tableplot::savefig( const std::string& filename,
                const int dpi,
		const std::string& orientation,
		const std::string& papertype,
		const std::string& facecolor,
		const std::string& edgecolor )
{
        casa::Bool rstat( casa::False );
	try {
	    rstat = TP->saveFigure( String( filename ), Int( dpi ),
                String( orientation ), String( papertype ), 
		String( facecolor ), String( edgecolor ) );
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		RETHROW(x);
	}
   return True;
}

/* Select subtables from opened tables */
bool
tableplot::selectdata(const std::string& taqlstring)
{
	if(alive==0 or nTabs==0){cerr <<"Open a Table first." << endl; return False;}
	String selectstring(taqlstring);
		
	try {
	
	if(adbg)cout << "setTables : Set tables from table objects " << endl;

	TableExprNode TEN;
	Table temptable,ttt;
		
	//Vector<Table> SelTABS;
	SelTABS.resize(nTabs);
	Vector<String> SelStr(nTabs);

	for(Int i=0;i<nTabs;i++) 
	{
		temptable = TABS[i];
		// catch TableError here...
		if(adbg)cout << "name : " << temptable.tableName() << endl;
		if(adbg)cout << "nrows : " << temptable.nrow() << endl;
		
		TEN = RecordGram::parse(temptable,selectstring);
		if(adbg)cout << "TEN data type : " << TEN.dataType() << endl;
		
		ttt = temptable(TEN);
		if(adbg)cout << "subtable name : " << ttt.tableName() << endl;
		if(adbg)cout << "nrows : " << ttt.nrow() << endl;

		if(!ttt.nrow()) 
		{
			cerr << "No Rows in Selected Table !" << endl; return False;
		}
		cout << "Number of selected rows : " << ttt.nrow() << endl;
		SelTABS[i] = ttt;
		SelStr[i] = selectstring;
	}
	/* Attach selected tables to internal list */
	if(TP->setTableT(SelTABS,TabNames,SelStr)==-1) return False;
	
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		RETHROW(x);
	}
   return True;
}

/* Plot Data */
bool
tableplot::plotdata(const ::casac::record& poption, const std::vector<std::string>& labels, const std::vector<std::string>& datastr)
{
	if(alive==0){ *itsLog << LogIO::WARN << "Please re-open the table" << LogIO::POST; return False;}
	if(adbg)cout << "Extract and Plot Data" << endl;

	try {

	/* Initialize */
	tempdatastr.resize(0);	tempdatastr = toVectorString(datastr);
	templabels.resize(0);   templabels = toVectorString(labels);
	tempplotoption = *toRecord(poption);

        Vector<Vector<String> > temptaqlvector(nTabs);
        for(Int i=0;i<nTabs;i++) temptaqlvector[i]=tempdatastr;

	PlotOptions pop;
	Vector<String> Errors(2);
	
	Errors[0] = pop.fillFromRecord(tempplotoption);
	if(Errors[0].length()>0){cout << "ERRORS: "<< Errors[0] << endl; return False;}

	if((Int)templabels.nelements()==3)
	{
		pop.Title = templabels[0];
		pop.XLabel = templabels[1];
		pop.YLabel = templabels[2];
	}

	/* Add a convert function */
	//pop.Convert = new TPConvertAdd();

	/* Validate Pop */
	Errors = pop.validateParams();
	if(Errors[1].length()>0)cout<<"WARNINGS :"<< Errors[1] <<endl;
	if(Errors[0].length()>0){cout<<"ERRORS :"<<Errors[0]<<endl;return False;}
	
	/* Validate all inputs */
	Vector<String> tempiteraxes(0);
	//Errors = TP->checkInputs(pop,temptaqlvector, tempiteraxes);
	Errors = TP->checkInputs(pop,tempdatastr, tempiteraxes);
	if(Errors[1].length()>0)cout<<Errors[1] <<endl;
	if(Errors[0].length()>0){cout<<"ERRORS :"<<Errors[0]<<endl;return False;}

	//TP->plotData(pop,temptaqlvector);
	TP->plotData(pop,tempdatastr);
        TP->changeGuiButtonState("iternext","disabled");
	
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		RETHROW(x);
	}
  return True;
}

/* RE-Plot Data */
bool
tableplot::replot()
{
	if(alive==0){ *itsLog << LogIO::WARN << "Please re-open the table" << LogIO::POST; return False;}
	if(adbg)cout << "Extract and Plot Data" << endl;

	try {
	
	TP->rePlot();
	
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		RETHROW(x);
	}
  return True;
}
#if 1
/* Start Iteration Plotting */
bool
tableplot::iterplotstart(const ::casac::record& poption, const std::vector<std::string>& labels, const std::vector<std::string>& datastr, const std::vector<std::string>& iteraxes)
{
	if(alive==0) return False;

	try {
	templabels.resize(0);   templabels = toVectorString(labels);
	tempdatastr.resize(0);  tempdatastr = toVectorString(datastr);
	tempiteraxes.resize(0); tempiteraxes = toVectorString(iteraxes);
	tempplotoption = *toRecord(poption);

        Vector<Vector<String> > temptaqlvector(nTabs);
        for(Int i=0;i<nTabs;i++) temptaqlvector[i]=tempdatastr;
	
	if(adbg)cout << "Plot while iterating over an axis..." << endl;
	
	if(tempdatastr.nelements() % 2 != 0)
	{cout << "Error : Need even number of TaQL strings" << endl; return False;}

	PlotOptions pop;
	Vector<String> Errors(2);
	
	Errors[0] = pop.fillFromRecord(tempplotoption);
	if(Errors[0].length()>0){cout << "ERRORS: "<< Errors[0] << endl; return False;}
	
	if((Int)templabels.nelements()==3)
	{
	  pop.Title = templabels[0];
	  pop.XLabel = templabels[1];
	  pop.YLabel = templabels[2];
        }

	/* Validate Pop */
	Errors = pop.validateParams();
	if(Errors[1].length()>0)cout<<"WARNINGS :"<< Errors[1] <<endl;
	if(Errors[0].length()>0){cout<<"ERRORS :"<<Errors[0]<<endl;return False;}
	
	/* Validate all inputs */
	Errors = TP->checkInputs(pop,temptaqlvector,tempiteraxes);
	if(Errors[1].length()>0)cout<< Errors[1] <<endl;
	if(Errors[0].length()>0){cout<<"ERRORS :"<<Errors[0]<<endl;return False;}
	
	if(TP->iterMultiPlotStart(pop,temptaqlvector,tempiteraxes)==-1)
	 return False;

        TP->changeGuiButtonState("iternext","enabled");

	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		RETHROW(x);
	}

  	return True;
}

/* IterPlotNext */
int
tableplot::iterplotnext()
{
	if(alive==0) return False;

	try {
	Vector<String> labcol;
	Vector<Vector<Double> > labval;
	
	if(TP->iterMultiPlotNext(labcol,labval)==-1)
	 return False;


	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		RETHROW(x);
	}
	return True;
}

/* Terminate Iteration plotting */
bool
tableplot::iterplotstop( const bool rmplotter )
{
	if(alive==0) return False;
	if(adbg)cout << " Stop iterplot..." << endl;

	try {
		TP->iterMultiPlotStop();
                TP->changeGuiButtonState("iternext","disabled");
		
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		RETHROW(x);
	}
	return True;
}
#endif

/* Mark Flags */
bool
tableplot::markregions(const int nrows, const int ncols, const int panel,const std::vector<double>& region)
{
	if(alive==0) return False;
	if(adbg)cout << "Mark Flag Regions" << endl;

	try {
		Vector<Double> regionvec(region.size());
		for(Int i=0;i<(Int)region.size();i++) 
			regionvec[i] = (Double)region[i];
		
		if(TP->markRegions(nrows,ncols,panel,regionvec) == -1) return False;
		
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		RETHROW(x);
	}
	return True;
}


/* Flag Data */
bool
tableplot::flagdata()
{
	if(alive==0) return False;
	if(adbg)cout << "Flag Data" << endl;

	try {
		/* Flag data */
		TP->flagData(FLAG);
		
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		RETHROW(x);
	}
	return True;
}

/* Un-Flag Data */
bool
tableplot::unflagdata()
{
	if(alive==0) return False;
	if(adbg)cout << "Unflag Data" << endl;

	try {
		/* Flag data */
		TP->flagData(UNFLAG);
		
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		RETHROW(x);
	}
	return True;
}

/* Locate Data */
bool
tableplot::locatedata(const std::vector<std::string>& columnlist)
{
	if(alive==0) return False;
	if(adbg)cout << "Locate Data" << endl;

	try {
		/* Convert from vector<string> to Vector<String> */
		Vector<String> clist;
		clist.resize(0);
		clist = toVectorString(columnlist);

		PtrBlock<Record*> INFO;
		TP->locateData(clist, INFO);
		
		/* Clean up the PtrBlock */
		for(Int i=0;i<(Int)INFO.nelements();i++)delete INFO[i];
		INFO.resize(0);
		
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		RETHROW(x);
	}
	return True;
}

bool 
tableplot::clearplot(const int nrows, const int ncols, const int panel)
{
	if(adbg)cout << " Clear Plot " << endl;
	try {
		if(TP->clearPlot(nrows,ncols,panel) == -1) return False;
	
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		RETHROW(x);
	}
	return True;
}

/* Clear Flags */
bool
tableplot::clearflags(const bool roottable)
{
	if(adbg) cout << "Clear All Flags" << endl;
	try {
		TP->clearAllFlags( (Bool)roottable );
	
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		RETHROW(x);
	}
	return True;
}

/* Save flag version */
bool
tableplot::saveflagversion(const std::string& versionname, const std::string& comment, const std::string& merge)
{
	if(adbg) cout << "saveflagversion" << endl;
	try {

		TP->saveFlagVersion( String(versionname), String(comment), String(merge) );
		
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		RETHROW(x);
	}
	return True;
}

/* Restore flag version */
bool
tableplot::restoreflagversion(const std::vector<std::string>& versionname, const std::string& merge)
{
	if(adbg) cout << "restoreflagversion" << endl;
	try {
		Vector<String> verlist;
		verlist.resize(0);
		verlist = toVectorString(versionname);
		
		TP->restoreFlagVersion( verlist, String(merge) );
		
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		RETHROW(x);
	}
	return True;
}

/* Get Flag version list */
bool
tableplot::getflagversionlist()
{
	if(adbg) cout << "getflagversionlist" << endl;
	try {

		Vector<String> versionlist(0);
      TP->getFlagVersionList(versionlist);

		for(Int i=0; i<(Int)versionlist.nelements(); i++)
			cout << versionlist[i] << endl;
		
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		RETHROW(x);
	}
	return True;
}

/* Delete flag version */
bool
tableplot::deleteflagversion(const std::vector<std::string>& versionname)
{
	if(adbg) cout << "deleteflagversion" << endl;
	try {
		Vector<String> verlist;
		verlist.resize(0);
		verlist = toVectorString(versionname);

		TP->deleteFlagVersion( verlist );
		
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		RETHROW(x);
	}
	return True;
}

/* Clean up */
bool
tableplot::done()
{
	try {
		if(adbg)cout << "tableplot::done()" << endl;
		if(alive==1)
		{
			TabNames.resize(0); nTabs=0;
			TABS.resize(0,True);
			SelTABS.resize(0,True);
			
			temptabnames.resize(0); tempdatastr.resize(0);
			tempiteraxes.resize(0); templabels.resize(0);
			
			uInt nf = tempplotoption.nfields();
			RecordFieldId rid(0);
			for(Int i=0;i<(Int)nf;i++) tempplotoption.removeField(rid);
			//if(adbg)cout << "Nfields in tempplotoptions : " << tempplotoption.nfields() << endl;

                        if(adbg)cout << "calling resettp from tableplot_cmpt" << endl;
			TP->resetTP();
                        if(adbg)cout << "finished  resettp from tableplot_cmpt" << endl;
			
			//if(TP) delete TP;
			TP = NULL;

                        if(resetter_p) delete resetter_p;
                        resetter_p = NULL;

			alive=0;
		}
		
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		RETHROW(x);
	}
	return True;
}


/*********************************************************************************/

/*********************************************************************************/

} // casac namespace




