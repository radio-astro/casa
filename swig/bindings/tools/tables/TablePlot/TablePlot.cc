//# TablePlot.cc: Implement class for the tableplot DO.
//# Copyright (C) 1994,1995,1996,1997,1998,1999,2000,2001,2002,2003-2008
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

//# Includes

#include <stdio.h>
#include <string.h>
#include <casa/iostream.h>
#include <casa/fstream.h>
#include <casa/math.h>

#include <casa/Exceptions.h>
#include <casa/OS/Timer.h>
#include <casa/IO/AipsIO.h>

#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/MatrixMath.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Slicer.h>

#include <tables/Tables/TableParse.h>
#include <tables/Tables/TableGram.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/TableColumn.h>
#include <tables/Tables/TableLock.h>
#include <tables/Tables/RecordGram.h>

#include <tools/tables/TablePlot/TablePlot.h>

#include <casa/Quanta/MVTime.h>
#include <casa/Quanta/QC.h>
#include <casa/Logging/LogIO.h>

/* Code Documentation : Main data structures in TablePlot. */

/*
   TablePlot holds a TPPlotter object, as well as a list of BasePlot objects, 
   and controls the interaction between them.  It also maintains panel-specific 
   information and controls the overplot mode.
   
   TPPlotter : the plotter class that queries the BasePlot classes for data, and
               plots it.
   BasePlot/CrossPlot : the data classes, that read data from the tables, and
                        manage TaQLs and flags.
   
   [ATBPS]:vector of BPS. Each element corresponds to the plots for ONE plot panel.
           All panel-specific information (panel location, axis labels, colours,etc...)
      are maintained for each ATBPS element. Whenever the user selects a new panel
      configuration (ncols,nrows,panel), a new entry is made to ATBPS. 
      (When a particular panel is cleared, the corresponding entry is cleaned up,
      deleted and set to NULL. The next new panel entry is filled into this slot
      if it exists, otherwise appended to the end.)
      
   [BPS]:vector of BasePlot/CrossPlot objects. Each element corresponds to a
         separate input table, and locally maintains its own table object.
    Therefore, data from multiple tables can be plotted on the same panel,
    but all data/flags are maintained separately for each table.
    The use of the overplot mode, also adds to this vector.
    This allows completely different plots to be overlaid.

      Sample ATBPS for a plot window with 3 panels.
   
      ATBPS[0] -> [ BPS[0] ] [ BPS[1] ] => Panel 1 : Overlay : Same TaQL, two diff tables.
      ATBPS[1] -> NULL        => Panel 2 : Has just been cleared !
      ATBPS[2] -> [ BPS[0] ]             => Panel 3 : One plot.
   
      => Same as above, but an "overplot" has just happened on Panel 1. That is why 
         the "layer" number has been incremented. The BPS[0],BPS[1] are from a single 
         plot call with the same TaQL, but on 2 tables opened together.  The BPS[2] is
         the result of an overlay with a completely different plot (TaQL and table).
   
   => The "layer" number is also important in BasePlot::getData, as it is used
      by BasePlot to decide whether to re-read data from the disk or not.
      This is done
      
            
   [PanelMap_p]:Maintains a map between ATBPS element index, and panel
                configuration (ncols,nrows,panel).
   
      PanelMap_p[0] -> [1,2,1]
      PanelMap_p[1] -> NULL
      PanelMap_p[2] -> [1,2,2]

   [TABS_p]: Vector of Table Objects. This is updated whenever the user calls "open"
             (or the application code calls setTableS or setTableT). 
        Every time this happens, "TableTouch_p" is set to "True", indicating 
        that the user has sent in a new table, and this information has to
        propagate to the BasePlot that will read the data.  
        TablePlot::creatBP() creates the BasePlot/CrossPlot object.
        TablePlot::updateBP() hands over the Table object to the
                              BasePlot/CrossPlot. The Table object has
               it's own reference counters, and stays alive
               as long as the BasePlot/CrossPlot is alive.
      
*/

#define LOG2 1
#define LOG0 0

namespace casa { //# NAMESPACE CASA - BEGIN

casa::TPGuiBinder* GBB=NULL;
casa::TablePlot *tableplot = 0;
        
String TablePlot::clname = "TablePlot";

/* Default Constructor */
TablePlot::TablePlot()
{
   log = SLog::slog(); 

   log->FnEnter("TablePlot", clname);

   TPLP = new TPPlotter();
   
   setupTP();
   if(!GBB) 
      GBB = new TPGuiBinder( this );

   resetters_p.resize(0);
   appnames_p.resize(0);

   isGuiVisible_p = True;
}

/*********************************************************************************/
/* set GUI off... */
/* Use this as a place-holder to add in calls to multiple TPLP constructors
   that use different plotting backends */
Bool TablePlot::setGui(Bool guiVisible)
{
   log->FnEnter("setGui(guiVisible)", clname);

   // Nothing has changed so don't do anything.
   if ( guiVisible == isGuiVisible_p )
       return True;
   isGuiVisible_p = guiVisible;
   
   
   if(!GBB) GBB = new TPGuiBinder( this );

   //if(TPLP) TPLP->closeWindow(); 
   // This call is essential before deleting TPLP .
   
   resetTP();

   if(TPLP) delete TPLP;
   TPLP = new TPPlotter(guiVisible);   
   

   return True;
}


/*********************************************************************************/
/* Initialize TP */
Bool TablePlot::setupTP()
{
   /* Initialize to zero panels */
   ATBPS.resize(0);
   PAN.resize(0);

   /* Init for zero tables */
   // first release autolocks if any tables are being held
   //for(uInt i=0;i<TABS_p.nelements();i++) 
   //   TABS_p[i].relinquishAutoLocks(True);
   TABS_p.resize(0);
   nTabs_p=0;

   /* Initial state */
   IterPlotOn_p=False;
   IterPlotStarted_p=False;
   TableTouch_p=False;
   CurrentPanel_p=1;
   isPlotDisplayed_p = False;
   
   /* Initialize to zero iteration axes and no overplots */
   IterAxes_p.resize(0);
   Pops_p.resize(0);
   TaqlStr_p.resize(0);
   OvpRootTabNames_p.resize(0);
   OvpTabSelections_p.resize(0);
   OvpIters_p.resize(0);
   Separate_p=String("none");
   TitleStrings_p.resize(0);

   /* Initialize the column names to be used for flags */
   dataFlagColName_p = String("FLAG");
   rowFlagColName_p = String("FLAG_ROW");

   /* Start a new log-instance */

   return True;
}

/*********************************************************************************/
/*********************************************************************************/

/* Destructor */
TablePlot::~TablePlot()
{
   log->FnEnter( "~TabplePlot()", clname);

   //// This is a singleton class - and once created, it will get deleted when
   //// the process is exited from ( i.e. ctrl-D to exit from casapy )
   //// If "resetTP" is called here, the internal python-interpreter calls 
   ////            will seg-fault - becuase at the time of exit ( ctrl-D ), 
   ////            it appears that the interpreter gets killed before this 
   ////            Singleton destructor gets called. 
   //// TODO : Perhaps find a cleaner way to handle this ?
        
}

/*********************************************************************************/
/*********************************************************************************/
/* Function to enable or disable buttons. */
/* button : 'iternext','markregion','flag','unflag','locate' */
/* state  : 'active' , 'inactive' */
/* By default, iternext=inactive and all others are active */
Bool TablePlot::changeGuiButtonState( String button, String state )
{
   if(TPLP) 
      return TPLP->changeGuiButtonState( button, state );
   else 
      return False;
}

/*********************************************************************************/
Bool TablePlot::saveData(const String& filename) {
   //cout << "saveData " << filename << endl; 

   for(Int p=0; p<(Int)ATBPS.nelements(); p++)
   {
      if(ATBPS[p]!=NULL)
         /* Iterate over BasePlots */
         for(Int i=0;i<(Int)(*ATBPS[p]).nelements();i++) 
         {
            (*ATBPS[p])[i]->saveData(filename);
         }
   }
   return True;
} 
/*********************************************************************************/

/*
 A routine for saving the plot to an image file.
 This uses the matplotlib "pl.savefig" routine to do all the work.

 Note that the supported file formats depends on the backend used with
 matplotlib.  Currently with the Tk backend only eps, ps, pdf, and png
 formats are supported.
*/ 
Bool TablePlot::saveFigure(const String& filename, const Int dpi, 
          const String& orientation, const String& papertype, 
          const String& facecolor, const String& edgecolor ) {

    String fnname= "saveFigure";
    log->FnEnter(fnname + "(filename, dpi, orientation, ...)", clname);
    casa::Bool rstat = True;

    /* check that the figure manager has been started. */
    if(!isPlotDisplayed_p) {
       log->out("A plot needs to be made before figures can be saved", 
             fnname, clname, LogMessage::WARN );
       return False;
    }

    /* Check if we've been given a filename. */
    // If we don't have a filename there's nothing to save.
    // TODO perhaps develop a default naming scheme based on the MS name
    // and pick a default format.
    if (filename.length() < 1) {
       log->out("No file name given, unable to save the plotted figure.",
          fnname, clname, LogMessage::WARN);
       rstat = False;
    }
  
    if (upcase(filename).contains(".TXT")) {
      return saveData(filename);
    }
    
    if ( rstat )
    {
       /* Construct the pl.savefig command string to be issued. */
       try {
          //runPlotCommand(String("figwidth=4"));
          //runPlotCommand(String("figheight=3"));
          //runPlotCommand(
          //    String("pl.figure(1, figsize=(figwidth, figheight))"));
          // Add the filename to the matplotlib command.
          String cmd = String( "pl.savefig( \"" ) + filename + String( "\"" );
          /* Add the various optional values to the cmd string if the user */
          /* specified them. Note that papertype and orientation are support */
          /* for postscript files only.  Also note that we don't support the */
          /* 'format' option since it doesn't work with all of the backends. */
          if ( dpi > 0 ) {
             cmd += String( ", dpi=" ) + String::toString( dpi );
          }
          
          if ( orientation.length() > 0 ) {
             cmd += String( ", orientation=\"" ) + orientation + String( "\"" );
          }
   
          if ( papertype.length() > 0 ) {
             cmd += String( ", papertype=\"" ) + papertype + String( "\"" );
          }
   
          if ( facecolor.length() > 0 ) {
             cmd += String( ", facecolor=\"" ) + facecolor + String( "\"" );
          }
   
          if ( edgecolor.length() > 0 ) {
             cmd += String( ", edgecolor=\"" ) + edgecolor + String( "\"" );
          }
          
          /* Add the finall parenthesis at the end of the command */
          cmd += String( " ); " );
          log->out(cmd, fnname, clname, LogMessage::DEBUG1);
          
          log->out(String("Saving plot to file ") + filename, fnname, clname,
             LogMessage::NORMAL);
          runPlotCommand( cmd );
      } catch ( AipsError ae ) {
          rstat = False;
          throw;
      }
   }

   return rstat;
}

/*********************************************************************************/
/*********************************************************************************/

/* Accept Table names as input.
   - Create Table objects from the string of table names in inTabName 
   - Check if each table has a non-zero number of rows.
   - Fill in TABS_p, and set TableTouch_p = True so that other functions
     know that a new table has been attached.
 */
Bool TablePlot::setTableS(Vector<String> &inTabName, 
        Vector<String> &rootTabNames, Vector<String> &selection)
{
   log->FnEnter("setTableS(inTabName, rootTabName, selection)", clname);
   if(inTabName.nelements()==0) 
      TablePlotError(String("Empty list of table names"));

   if(rootTabNames.nelements() != inTabName.nelements()) 
      TablePlotError(String("Number of strings in rootTabNames must ") +
                     "match the number of Tables in inTabName");
   rootTabNames_p.resize(0);
   rootTabNames_p = rootTabNames;
   
   if(selection.nelements() != inTabName.nelements()) 
      TablePlotError(String("Number of strings in selection must match ") +
                    " the number of Tables in inTabName");
   tableSelections_p.resize(0);
   tableSelections_p = selection;
   
   if(IterPlotOn_p){
      iterMultiPlotStop();
   }
   
   nTabs_p = inTabName.nelements();
   
   if((Int)TABS_p.nelements() != nTabs_p) 
      TABS_p.resize(nTabs_p);
   try
   {
      for(Int i=0;i<nTabs_p;i++)
      {
         TABS_p[i] = Table(inTabName[i],Table::Update);
         if(!TABS_p[i].nrow()) TablePlotError(String("No Rows in Table."));
      }
      TableTouch_p=True;
   }
   catch(TableError &x)
   {
      TablePlotError(x.getMesg());
   }

   log->FnExit("setTableS(inTabName, rootTabName, selection)", clname);
   return True;
}

/*********************************************************************************/
Bool TablePlot::isTableSet()
{
   log->FnPass("isTableSet()", clname);
   
   if( TABS_p.nelements() ) 
       return True;
   else 
       return False;
}
/*********************************************************************************/

/* Accept Table Objects directly.
   - Check if each input table object has a non-zero number of rows.
   - Fill in TABS_p, and set TableTouch_p = True so that other functions
     know that a new table has been attached.
 */
Bool TablePlot::setTableT(Vector<Table> &inTabObj, Vector<String> &rootTabNames, Vector<String> &selection)
{
   String fnname= "setTableT";
   log->FnEnter(fnname + "(inTabObj,rootTabNames,selection)", clname );
    
   if(inTabObj.nelements()==0) 
       TablePlotError(String("Empty list of table objects"));
   
   if(rootTabNames.nelements() != inTabObj.nelements()) 
      TablePlotError(String("Number of strings must in rootTabNames ") +
                     "must match the number of Tables in inTabObj");
   rootTabNames_p.resize(0);
   rootTabNames_p = rootTabNames;
   
   if(selection.nelements() != inTabObj.nelements()) 
      TablePlotError(String("Number of strings must in selection must ") +
                    "match the number of Tables in inTabObj");
   tableSelections_p.resize(0);
   tableSelections_p = selection;

   /* CHECK VALIDITY OF INPUT TABLES */
   //Vector<String> Errors(checkTables(inTabObj));
   //if(Errors[0].length()>0) 
   //TablePlotError(String("Error in input Tables : ")+Errors[0]);

   nTabs_p = inTabObj.nelements();

   if((Int)TABS_p.nelements() != nTabs_p) 
      TABS_p.resize(nTabs_p);
   try
   {
      for(Int i=0;i<nTabs_p;i++)
      {
         TABS_p[i] = inTabObj[i];
         if(!TABS_p[i].nrow()) TablePlotError(String("No Rows in Table."));
      }
      TableTouch_p=True;
   }
   catch(TableError &x)
   {
      TablePlotError(x.getMesg());
   }
   log->FnExit(fnname, clname);
   return True;
}

/*********************************************************************************/
/* Change the default column names to be used for data and row flags */
/* data flags must be the same shape of the data column (arraycolumn) */
/* and rowflags must be a scalarcolumn. */
Bool TablePlot::useFlagColumns(String dataflags, String rowflags)
{
   dataFlagColName_p = dataflags;
   rowFlagColName_p = rowflags;
   return True;
}
/*********************************************************************************/
/* Allow the application layer to get access to the vector of Table objects. */
Bool TablePlot::getTabVec(Vector<Table> &tabs)
{
   log->FnPass("getTabVec(tabs)", clname);
   tabs.resize(TABS_p.nelements());
   for(Int i=0;i<(Int)TABS_p.nelements();i++) tabs[i] = TABS_p[i];
   return True;
}

/*********************************************************************************/

/* Create the BasePlot objects */
/*   Check if the plot Type has changed. 
     Current options are 
     (1) regular X-Y plots, where a pair of TaQL strings is sent in, one 
         for the x-axis, and one for the y-axis.
     (2) Cross plots where the x-axis is channel number, and only one TaQL 
         string is needed as input. The first of the pair of TaQL strings 
         should be "CROSS".
     (3) Histogram plots, where again, only one TaQL is needed. This is 
         NOT implemented yet.

     If any BasePlots need changing, delete them, and recreate them with 
     the new type.
*/
Bool TablePlot::createBP(PtrBlock<BasePlot* > &BPS, 
                         Vector<Vector<String> > &taqls)
{
   String fnname= "createBP";
   log->FnEnter(fnname + "(BasePlot, taqls)", clname);
   Int key = XYPLOT;

   if ( taqls.nelements() < 1 || taqls[0].nelements() < 1 )
       TablePlotError(String( 
          "Internal Error, empty TaQL found while creating BasePlot."));
   

   String pType = (taqls[0])[0];
   
   if(pType.contains("cross") || pType.contains("CROSS")) 
      key = CROSSPLOT;
   if(pType.contains("hist") || pType.contains("HIST")) 
      key = HISTPLOT;
   log->out(String("pType=") + pType + " key=" + key,
            fnname, clname, LogMessage::DEBUG1);

   Bool change = False;
   /* Check plot types, only if BasePlots/CrossPlots already exist */
   if((Int)BPS.nelements() > 0)
   {
      log->out(String("pType=") + pType + " key=" + key,
            fnname, clname, LogMessage::DEBUG1);

      /* If plot type is different, clean it up.
         NOTE :  This check is done on ONLY the first element BPS[0].
         This is okay because it doesn't make sense to make overlay
         plots with X-Y and Channel plots.
         (it'll work if you try to, although it may not make much sense...)
      */
      if(BPS[0]->getPlotType() != key || (Int)BPS.nelements()!=nTabs_p) {
         for(Int i=0;i<(Int)BPS.nelements();i++) 
            delete BPS[i];
         BPS.resize(0,Bool(1));
         log->out(String("Number of BPSs - should be zero !! : ")+
                  String::toString(BPS.nelements()),
                  fnname, clname, LogMessage::DEBUG1);
      }
      log->out(String("Number of BPSs : ") + String::toString(BPS.nelements()),
               fnname, clname, LogMessage::DEBUG1);      
   }
   /* if BPS is of zero size, then make according to chosen pType. 
      - Either it's a new panel, or all the BP's need to be of a new type 
   */
   if((Int)BPS.nelements()==0) 
   {
      BPS.resize(nTabs_p);
      switch (key)
      {
         case XYPLOT:
            for(Int i=0;i<nTabs_p;i++) 
               BPS[i] = new BasePlot();
            break;
         case CROSSPLOT:
            for(Int i=0;i<nTabs_p;i++) 
               BPS[i] = new CrossPlot();
            break;
         //case HISTPLOT:
         //   for(Int i=0;i<nTabs_p;i++) BPS[i] = new HistPlot();
         //   break;
               
      }
      change = True;
   }
   
   log->out(String("change : ")+String::toString(change), 
            fnname, clname, LogMessage::DEBUG1);
   log->FnExit(fnname, clname);
   return change;
   /* if this is True ==> need to call upDateBP
      if this is False ==> no need to call upDateBP (but no harm if called)
   */
}

/*********************************************************************************/

/* Attach one table to each BasePlot 
   - The table obj is handed over to the BasePlot, and TableTouch_p is set
     to False, to indicate that the BasePlot now has the latest table.
*/
Bool TablePlot::upDateBP(PtrBlock<BasePlot* > &BPS)
{
   String fnname= "upDateBP";
   log->FnEnter(fnname + "(BasePlot)", clname);
   
   for(Int i=0;i<nTabs_p;i++) 
   {
      //cout << TABS_p[i].tableName() << " " 
      //     << rootTabNames_p[i] << " " 
      //     << tableSelections_p[i] << endl;

      // initialise each BP - TableObj
      BPS[i]->init(TABS_p[i], i, rootTabNames_p[i], 
         tableSelections_p[i], dataFlagColName_p, rowFlagColName_p); 
   }
   
   TableTouch_p=False;
   
   log->FnExit(fnname, clname);
   return True;
}

/*********************************************************************************/
/* CleanUp TablePlot */
Bool TablePlot::deleteBasePlot(Int panel, Int layer)
{
   if((*ATBPS[panel])[layer]->callbackhooks_p)
   (*ATBPS[panel])[layer]->callbackhooks_p->releasetable(
        PAN[panel]->Plop.PanelMap[0],
        PAN[panel]->Plop.PanelMap[1],
        PAN[panel]->Plop.PanelMap[2], 
        (*ATBPS[panel])[layer]->getTableName());
   delete (*ATBPS[panel])[layer];
   return True;
}
/*********************************************************************************/
/* Reset TablePlot - callback. */
Bool TablePlot::setResetCallBack( String appname, TPResetCallBack * resetter )
{
   String fnname= "setResetCallBack";
   log->FnEnter(fnname + "(appname, resetter)", clname);
   Int n = resetters_p.nelements();
   log->out(String("appname: ") + appname, fnname, clname,
            LogMessage::DEBUG1);

   //log->out(String("start : " )+String::toString(n), fnname, clname);

        
   Bool found = False;
   for( Int i=0;i<n;i++ ) {
      if( appnames_p[i].matches(appname) ) {
         resetters_p[i] = resetter;
         found = True;
      }
   }
        
   if(!found) {
      resetters_p.resize(n+1,True);
      resetters_p[n] = resetter;
      appnames_p.resize(n+1,True);
      appnames_p[n] = appname;
   }
   
   log->FnExit(fnname, clname);
   return True;
}

/* Remove a reset callback */
Bool TablePlot::clearResetCallBack( String appname )
{
   String fnname= "clearResetCallBack";
   log->FnEnter(fnname +  "(appname)", clname);
   Int n = resetters_p.nelements();
   log->out( String("appname : " )+appname);
   log->out( String("start : " )+String::toString(n));

   Int found = -1;
   for( Int i=0;i<n;i++ ) {
      if(appnames_p[i].matches(appname)) {
         resetters_p[i] = NULL;
         appnames_p[i] = "";
         found = i;
      }
      if (found>-1) 
         break;
   }
   if (found > -1) {
      for ( Int i=found;i<(n-1);++i) {
        resetters_p[i] = resetters_p[i+1];
        appnames_p[i] = appnames_p[i+1];
      }
      resetters_p.resize(n-1,True);
      appnames_p.resize(n-1,True);
   }
        
   log->FnExit(fnname, clname);
   return True;
}
/*********************************************************************************/
/* CleanUp TablePlot */
Bool TablePlot::resetTP(Int closewindow)
{
   String fnname= "resetTP";
   log->FnEnter(fnname + "(closeWindow)", clname);

   /* If iteration data structures are active, turn them off */
   iterMultiPlotStop();
   
   if( isPlotDisplayed_p ) 
       clearPlot(0,0,0);
   
        
   for( uInt i=0;i<resetters_p.nelements();i++ )
      if(resetters_p[i]) 
         (resetters_p[i])->reset();
  
   resetters_p.resize(0,True);
   appnames_p.resize(0,True);
 
   if(TPLP && closewindow) TPLP->closeWindow(); 
    // This call is essential - before deleting TPLP
   if(TPLP) 
       delete TPLP;
   TPLP = new TPPlotter();
        
   TPLP->setupPlotter();

   setupTP();

   log->FnExit(fnname, clname);
   return True;
}

/*********************************************************************************/

/*********************************************************************************/

/* Run matplotlib commands */
Bool TablePlot::runPlotCommand(String command)
{
   TPLP->runPlotCommand(command);

   return True;
}
/*********************************************************************************/
/*********************************************************************************/

/* Manage ATBPS, and read data from the tables. */
/*
   (1) Check user input for parameter validity.
   (2) Extract panel info, and check if a new panel is being asked for
   (3) If new panel, add a new entry to ATBPS. 
       If existing panel, modify the existing ATBPS entry for that panel.
                          If(overplot) append to the ATBPS[panel]'s BPS
                        and assign an incremented "layer" number.
           If(!overplot) replace the last layer of BPS elements
                         and retain the same layer number
               (as the last layer). If this is the
               first layer, then assign layer=1.


*/
Vector<String> TablePlot::checkInputs(PlotOptions &pop, Vector<String> &datastr, Vector<String>& iteraxes)
{
        Vector<Vector<String> > xytaqlvec(TABS_p.nelements());
        for(uInt i=0;i<xytaqlvec.nelements();i++) xytaqlvec[i] = datastr;
        return checkInputs(pop,xytaqlvec,iteraxes);
}

Vector<String> TablePlot::checkInputs(PlotOptions &pop, Vector<Vector<String> > &datastrvector, Vector<String>& iteraxes)
{
   String fnname= "checkInputs";
   log->FnEnter(fnname + "(pop, datastrvector, iteraxes)", clname);
   //for(Int i=0;i<(Int)datastrvector.nelements();i++)
   //        cout << datastrvector[i] << endl;

   /* First one is Errors, and second one is Warnings */
   Vector<String> ErrorString(2);
   ErrorString[0] = String("");
   ErrorString[1] = String("");

   /* Check the Tables.... nrows>0 and the existence of a FLAG column */
   // Also try to check that all rows are the same shape.

   if( ! TABS_p.nelements() )
   {
      ErrorString[0] += String("\n No Tables are currently set !");
      log->FnExit(fnname, clname);
      return ErrorString;
   }

   /* Do checks on the input tables... do these only once per call to "setTable[T,S]" */
   if( TableTouch_p )
   {
      for(Int i=0;i<nTabs_p;i++)
      {
                        /* Check if the iteraxes columns exist */
         for(Int it=0;it<(Int)iteraxes.nelements();it++)
         {
            // Convert to UPPER CASE
            iteraxes[it] = upcase(iteraxes[it]);

            if( !TABS_p[i].tableDesc().isColumn(iteraxes[it]) )
            ErrorString[0] += String("\nTable ") + TABS_p[i].tableName() + 
                              String(" does not have a column called ") + 
                    ( iteraxes[it].length()?iteraxes[it]:String("'empty'") )
                    + String(" to iterate on.");
         }

                        /* Check if the Plop.LocateColumns table columns exist
                           and if they result in "int" or "double". */
                        //TODO

                        /* Check if flagversions will work here */
         try
         {
            if(rootTabNames_p[i].length()>0)
               FlagVersion fv(rootTabNames_p[i],dataFlagColName_p,rowFlagColName_p);
         }
         catch (AipsError x)
         {
            /* disable flag versions for this table */
            rootTabNames_p[i] = String("");
            ErrorString[1] += String("\nDisabling flag version support for ") 
                  + rootTabNames_p[i] + x.getMesg();
         }
      }
   }
   
   /* Parse the user input parameters
      Go on, only if this passes through.*/
      
   Vector<String> PopErrors = pop.validateParams();

   ErrorString[0] += PopErrors[0];   
   ErrorString[1] += PopErrors[1];

   /* Check for validity of datastrvector -> on all of TABS_p */
   /* One Vector<String> per Table */
   //cout << "# TABS=" << TABS_p.nelements() << endl;
   //cout << "# TaQL=" << datastrvector.nelements() << endl;
   if( datastrvector.nelements() != TABS_p.nelements() ) {
       ErrorString[0] += String("\n The number of XY TaQL vectors must ") +
           "be equal to the number of Tables";
       log->FnExit(fnname, clname);
       return ErrorString;
    }
    for( uInt i=0;i<datastrvector.nelements();i++ )
       if( (datastrvector[i]).nelements()%2 != 0 ) 
          ErrorString[0] += String("\n The X-Y TaQL string must have ") +
              "an even number of elements.";

   Int TotalNumPoints=0;
   try
   {
      for(Int i=0;i<(Int)TABS_p.nelements();i++)
      {
         Int nrows = TABS_p[i].nrow();
         TableExprNode ten;
         Vector<String> datastr(datastrvector[i]);
         for(Int j=0;j<(Int)datastr.nelements();j++)
         {
            Double xytemp;
            Array<Double> xtemp;
            if(!datastr[j].contains("CROSS"))
            {
#if LOG0 
               { 
                  ostringstream os;
                  os << "check " << TABS_p[i] 
                     << "\ntaql=" << datastr[j] << endl;
                  log->out(os, fnname, clname, LogMessage::DEBUG1);
               }
#endif
               ten = RecordGram::parse(TABS_p[i],datastr[j]);
         
               /* If X-TaQL is no a scalar, then its shape must match the
                  corresponding Y-TaQL */
               if(j%2==0 && !ten.isScalar()) // the X-TaQls.
               {
                  TableExprNode tempten = 
                      RecordGram::parse(TABS_p[i],datastr[j+1]);
                  if(tempten.isScalar())
                      ErrorString[0] += String("\n Y-axis cannot be scalar ") +
                                   " while X is an array";
                  else {
                     ten.get(0,xtemp);
                     Array<Double> xxtemp;
                     tempten.get(0,xxtemp);
                     if(xtemp.shape() != xxtemp.shape())
                         ErrorString[0] += String("\n X-TaQL must be scalar,") +
                            " or X TaQL and Y TaQL must be of the same shape.";
                  }
               }
                  
               if(ten.isScalar()) {
                  ten.get(0,xytemp);
                  TotalNumPoints += nrows;

                  if(datastr[j].contains("TIME")) {
                     /* Check that it's Not an absurd value
                        that matplotlib will choke on for timeplot... */
                     if(xytemp < 1.0 || xytemp > 3.652e+06) {
                        /* check for TIME in X-taql */
                        if(j%2==0 && ( pop.TimePlotChar.matches("x") || 
                              pop.TimePlotChar.matches("b")) ) 
                           ErrorString[0] += 
                              String("\n The value for X-data needs to be ") +
                                 "between 1 and 3.652e+06.\n  This is the " +
                                 "range that matplotlib can handle for " +
                                 "time-formatting.\n  Suggestion : " +
                                 "'(TIME/86400.0)+678576.0'.\n "+
                                 " Please check the input TaQL : " + 
                                 datastr[j] ;
                        if(j%2==1 && ( pop.TimePlotChar.matches("y") || 
                            pop.TimePlotChar.matches("b")) ) 
                           ErrorString[0] += String("\n The value for Y-data ")+
                              " needs to be between 1 and 3.652e+06.\n  "+
                              "This is the range that matplotlib can handle "+
                              " for time-formatting.\n  Suggestion : " + 
                              "'(TIME/86400.0)+678576.0'.\n  Please check " +
                              "the input TaQL : " + 
                              datastr[j] ;
                     }
                  }
               }
               else {
                  xtemp.resize();
                  ten.get(0,xtemp);
                  TotalNumPoints += nrows*(xtemp.shape()).product();
#if LOG0
                  {
                     ostringstream os;
                     os << "nrows=" << nrows << " shape=" 
                        << xtemp.shape() << " xtemp=" << xtemp << endl;
                     log->out(os, fnname, clname, LogMessage::DEBUG1);
                  }
#endif
               }
            } 
            else {
               // We have a Cross plot and we need to make sure that the y-TaQL
               // contains a non-scalar. We cannot do a cross-plot with a scalar
               // value.
               if( j%2==0 ) {
                  TableExprNode tempten = 
                     RecordGram::parse(TABS_p[i],datastr[j+1]);
                  if (tempten.isScalar())
                      ErrorString[0] += String( "\n Y-Axis can not be scalar") +                         " for CROSS plots. This is a non-sensical plot." ;
               }
            }
         }      
      }
   }
   catch (AipsError &x)
   {
      ErrorString[0] += String("\n TaQL string error : ") + x.getMesg();
   }

   /* Warn the user he he/she is plotting a "large" number of points that may
      take "a long time" to plot because of the laws of physics. */
   if( TotalNumPoints > 100000 )
      ErrorString[1] += String("\nMay need to read ") + String::toString(TotalNumPoints)
                 + String(" values from disk.\n");

   /* If an overplot is being done, check that the plot-type is compatible with previous
      layers for the chosen panel */
   if( pop.OverPlot )
   {
      /* Get the panel index */
      Int panindex = getPanelIndex(pop.PanelMap[0],pop.PanelMap[1],pop.PanelMap[2]);

      /* If it already exists, then check types. */
      if( panindex != -1 )
      {
         if( !pop.TimePlotChar.matches( PAN[panindex]->Plop.TimePlotChar ) )
            ErrorString[0] += String("\n Cannot overlay plots with and without Time-Formatting. Please check/change your overplot setting."); 
         
      }
   }
   
   log->FnExit(fnname, clname);
   return ErrorString;
}

/* Check the input Table Vector... */
Vector<String> TablePlot::checkTables(Vector<Table> &tables)
{
   log->FnEnter( "checkTables", "checkTables( tables )" );
   
   /* First one is Errors, and second one is Warnings */
   Vector<String> ErrorString(2);
   ErrorString[0] = String("");
   ErrorString[1] = String("");

   Int ntabs = tables.nelements();

   /* Do checks on the input tables... do these only once per call to "setTable[T,S]" */
   for(Int i=0;i<ntabs;i++)
   {
      if(!tables[i].nrow()) 
      {
         ErrorString[0] += String("\n No Rows in Table : ") + tables[i].tableName() ;
         return ErrorString;
      }
      
      /* Check if FLAG and FLAG_ROW columns exist */
      if(!tables[i].tableDesc().isColumn(dataFlagColName_p))
         ErrorString[1] += String("\nTable ") + tables[i].tableName() + 
            String(" does not have a ") + dataFlagColName_p + String(" column.\n  Will not be able to read or set Array flags.\n  If ") + rowFlagColName_p + String(" exists, it will be used. ");
      if(!tables[i].tableDesc().isColumn(rowFlagColName_p))
         ErrorString[1] += String("\nTable ") + tables[i].tableName() + 
            String(" does not have a ") + rowFlagColName_p + String(" column.\n  Will not be able to read or set row flags. ");
         
      /* For all ArrayColumns in the table, check that all rows have
         the same shape. If not same, complain, and ask the user to do a
         selection that will ensure that they are the same */
      
      Bool niceshapes = checkShapes(tables[i]);
         if(! niceshapes) ErrorString[0] += String("\n Columns of table ") + tables[i].tableName() + String(" contain different shapes for different rows. Please do a sub-selection, to pass in multiple tables each with consistent shapes.");
      
   }
   return ErrorString;
}

/********************************************************************************************/
/* Plot data */
Bool TablePlot::plotData(PlotOptions &pop, Vector<String> &datastr, const String iterstate)
{
        Vector<Vector<String> > xytaqlvec(TABS_p.nelements());
        for(uInt i=0;i<xytaqlvec.nelements();i++) xytaqlvec[i] = datastr;
        return plotData(pop,xytaqlvec,iterstate);
}

Bool TablePlot::plotData(PlotOptions &pop, 
        Vector<Vector<String> > &datastrvector, 
        const String iterstate)
{
   
   String fnname= "plotData";
   log->FnEnter(fnname + "(pop, datastrvector, iterstate)", clname);

   /* If it's not a call from iterMultiPlotNext, then assume that 
     iterations are to be stopped. */
   if( iterstate.matches(String("iter_off")) )iterMultiPlotStop();

        /* Assign the current GuiBinder to the global pointer */
        /* Needed when multiple tools or multiple instances of the 
           tool are being used simultaneously */
   //GBB = GuiBase_p;

   /* CHECK VALIDITY OF INPUT PARAMS */
   Vector<String> iter(0);
   Vector<String> Errors(checkInputs(pop,datastrvector,iter));
   if(Errors[0].length()>0) TablePlotError(String("Error in Inputs."));
   
   /* manage ATBPS data structures */
   log->out(String("ATBPS (before) : ")+
                  String::toString(ATBPS.nelements()),
                  fnname, clname, LogMessage::DEBUG1);
   for(Int i=0;i<(Int)ATBPS.nelements();i++)    
       if(ATBPS[i]!=NULL) {
          ostringstream mg;
             mg << " ATBPS " << (*ATBPS[i]).nelements() 
                << " Layers: " << PAN[i]->LayerNumbers 
                << " PanelMap " << PAN[i]->Plop.PanelMap;
           for(Int j=0;j<(Int)PAN[i]->LayerXYTaqls.nelements();j++)
             mg << " " << PAN[i]->LayerXYTaqls[j] << "\n";
           log->out(mg, fnname, clname, LogMessage::DEBUG1);
       }
   
   
   /* Extract nxpanel,nypanel,currpanel from pop.
      Hunt in list for this combination.
      If found, set 'panel' to this index + 1. (!!)
      If not found, create - at end of list and set 'panel' to that */
   if(pop.RemoveOldPanels)
      clearOverLaps(pop.PanelMap[0],pop.PanelMap[1],pop.PanelMap[2]);

   Int panindex = PAN.nelements();
   
   panindex = getPanelIndex(pop.PanelMap[0],pop.PanelMap[1],pop.PanelMap[2]);
   
   /* OverPlot conventions per panel :
      OverPlot=False -> clean up all layers and make a fresh plot.
      OverPlot=True, ReplaceTopPlot=False -> add a new layer.
      OverPlot=True, RePlaceTopPlot=True -> Change only the top-most layer.
      --> If there aren't any current layers,
       then set RePlaceTopPlot = False;
   */
   if( ! isPlotDisplayed_p && pop.ReplaceTopPlot ) pop.ReplaceTopPlot = False;

   /* If overplot = False, then clean things up - if the panel exists ! */
   /* Clean all layers if TableTouch = True. 
     Otherwise leave the first layer. */
   //if(!pop.OverPlot && panindex != -1) 
   if(panindex != -1) 
   {
      String msg=String("Preparing panel " )+
           String::toString(panindex)+String( " ..." );
      log->out(msg, fnname, clname, LogMessage::NORMAL5);

      if( !pop.OverPlot ) /*  OverPlot = False */
      {
          
         if( PAN[panindex]->MaxLayer == 1 )
            TPLP->clearPlot(panindex+1,True);
         else
            clearPlot(pop.PanelMap[0],pop.PanelMap[1],pop.PanelMap[2]);
      }
      else /* OverPlot = True */
      {
         if( pop.ReplaceTopPlot ) /* Replace the Top-Most layer */
         {
            // delete the ATBPS and PAN entries for the MaxLayer, 
            //   and change the MaxLayer !!
            // Then allow "overplot=True" to take over.
            Int nmax = 0;
            Int ntotal = ATBPS[panindex]->nelements();
            log->out(String("Plotting ")+ String::toString(ntotal)+
                 String(" base plots"), fnname, clname, LogMessage::NORMAL4);
            for(Int i=0; i<ntotal; i++) 
            {
               if( PAN[panindex]->MaxLayer == PAN[panindex]->LayerNumbers[i] )
               {
                  nmax++;
                  deleteBasePlot(panindex,i);
                  //delete (*ATBPS[panindex])[i];
               }
                  
            }
            (*ATBPS[panindex]).resize(ntotal-nmax,True);
            PAN[panindex]->changeNlayers(ntotal-nmax);
            if( PAN[panindex]->nBP==0 ) 
                PAN[panindex]->MaxLayer = 1;
            else 
                PAN[panindex]->MaxLayer = 
                      PAN[panindex]->LayerNumbers[ntotal-nmax-1];
         }
      }
   }
   
   if(panindex==-1) 
        panindex = PAN.nelements();

   Int thislayer=0;
   Int panel = panindex+1;

   /* resize ATBPS and PAN if there's a new panel */
   Int nelm = (Int)ATBPS.nelements(); 
   // check if this is always only increased by one. If so, combine with ###
   if(nelm < panel) 
   {
      ATBPS.resize(panel,True);
      for(Int i=nelm;i<(Int)ATBPS.nelements();i++) 
          ATBPS[i]=NULL;
      PAN.resize(panel,True);
      for(Int i=nelm;i<(Int)PAN.nelements();i++) 
          PAN[i]=NULL;
   }
   
   
   /* If NULL, create a BPS first */ // ###
   if(ATBPS[panel-1]==NULL) 
   {
      PAN[panel-1] = new PanelParams();
      ATBPS[panel-1] = new PtrBlock<BasePlot* >();
      (*ATBPS[panel-1]).resize(0,True);
   }
   
   /* Count the number of layers in the current panel */
   Int noldbps = (*ATBPS[panel-1]).nelements();
      
   log->out(String("TableTouch_p : ")+ String::toString(TableTouch_p),
            fnname, clname, LogMessage::DEBUG1);
   
   /* If OverPlot=True and ReplaceTopPlot=False, add a new BPS */
   //if(pop.OverPlot && !pop.ReplaceTopPlot)
   if(pop.OverPlot)
   {
      log->out(String("OverPlot : ")+
          String::toString(pop.OverPlot)+
          String(" : ")+String::toString(pop.ReplaceTopPlot),
          fnname, clname, LogMessage::DEBUG1);
      
      /* Create and fill a new add-on BPS */
      /* If there are multiple tables specified, a list of BPs 
         will be appended */
      PtrBlock<BasePlot* > newbps(0);
      if(createBP(newbps,datastrvector) || TableTouch_p) 
          upDateBP(newbps); 
      //if(createBP(newbps,(datastrvector[0])[0]) || TableTouch_p) 
      //      upDateBP(newbps); 

      /* Update ATBPS, and compute and update layer info */
      (*ATBPS[panel-1]).resize(noldbps+newbps.nelements(),True);
      PAN[panel-1]->changeNlayers(noldbps+newbps.nelements());
      
      (*ATBPS[panel-1]).resize(noldbps+newbps.nelements(),True);
      if(noldbps==0) 
         thislayer=1;
      else 
         thislayer = (PAN[panel-1]->LayerNumbers)[noldbps-1] + 1;

      if( (Int)newbps.nelements() != nTabs_p ) 
         TablePlotError(String("Internal Error : BP/Table vector mismatch"));
      for(Int i=0;i<(Int)newbps.nelements();i++)
      {
         (*ATBPS[panel-1])[i+noldbps] = newbps[i];
         (PAN[panel-1]->LayerNumbers)[i+noldbps] = thislayer;
         ((PAN[panel-1]->LayerXYTaqls)[i+noldbps])
               .resize((datastrvector[i]).nelements());
         (PAN[panel-1]->LayerXYTaqls)[i+noldbps] = datastrvector[i];
      }

   }
   
   /* If OverPlot=False, work with top plot */   
   //if(!pop.OverPlot || pop.ReplaceTopPlot)
   if(!pop.OverPlot)
   {
      log->out(String("Make first layer : ")+ String::toString(pop.OverPlot),
           fnname, clname, LogMessage::DEBUG1);
      
      /* Extract-out a BPS corresponding to the MaxLayer ONLY */
      /*
      PtrBlock<BasePlot* > tempbps(nTabs_p);
      Int Nat = ATBPS[panel-1]->nelements();
      for(Int i=0;i<nTabs_p;i++)
         tempbps[i] = (*ATBPS[panel-1])[Nat-nTabs_p+i];
      */
      
      /* Fill in the BPS (existing in ATBPS) */
      /* Assume that all BPs are the same layer */

      // create BasePlot here !
      //if(createBP(*ATBPS[panel-1],(datastrvector[0])[0]) || TableTouch_p) 
      if(createBP(*ATBPS[panel-1],datastrvector) || TableTouch_p) { 
         upDateBP(*ATBPS[panel-1]);
      
      /* Compute and update layer info */
      /* resize - since createBP could've changed nBP */
      PAN[panel-1]->changeNlayers((*ATBPS[panel-1]).nelements());
      
      /* Fresh plot for the top layer only */
      //if(noldbps==0)// check this...
      //{
         thislayer=1;
         if( PAN[panel-1]->nBP != nTabs_p ) 
               TablePlotError(String(
                  "Internal Error : BP/Table vector mismatch"));
         for(Int i=0;i<PAN[panel-1]->nBP;i++) {
            (PAN[panel-1]->LayerNumbers)[i] = thislayer;
            ((PAN[panel-1]->LayerXYTaqls)[i])
                    .resize((datastrvector[i]).nelements());
            (PAN[panel-1]->LayerXYTaqls)[i] = datastrvector[i];
         }
      //}
      //else 
      //{
          /*
          thislayer = (PAN[panel-1]->LayerNumbers)[noldbps-1]; 
          Int maxlayerindex=0;
          for(Int i=0;i<PAN[panel-1]->nBP;i++)
          if(PAN[panel-1]->LayerNumbers[i]==thislayer)
          {
              maxlayerindex=i;
              break;
          }
          if(maxlayerindex+(Int)datastrvector.nelements() != PAN[panel-1]->nBP) 
               TablePlotError(String("Internal Error in index bookkeeping."));
          for(Int i=maxlayerindex;i<PAN[panel-1]->nBP;i++)
          {
              ((PAN[panel-1]->LayerXYTaqls)[i])
                 .resize((datastrvector[i-maxlayerindex]).nelements());
              (PAN[panel-1]->LayerXYTaqls)[i] = datastrvector[i-maxlayerindex];
          }
          */
      //}
     }
   }

   /* Set plot options */
   PAN[panel-1]->Plop = pop;
   
   /* Update other layer-dependant params from the LayerNumber info */
   PAN[panel-1]->updateLayerParams();

   ostringstream mg;
   mg << " PAN : " << PAN[panel-1]->Plop.PanelMap 
      << "\n pop : " << pop.PanelMap ;
   log->out(mg, fnname, clname, LogMessage::DEBUG1);

   log->out("ATBPS (after): " + String::toString(ATBPS.nelements()),
            fnname, clname, LogMessage::DEBUG1);

   ostringstream os;
   for(Int i=0;i<(Int)ATBPS.nelements();i++) 
      if(ATBPS[i]!=NULL) {
          os << i << " ATBPS " << (*ATBPS[i]).nelements() 
             << " Layers=" << PAN[i]->LayerNumbers 
             << " PanelMap=" << PAN[i]->Plop.PanelMap << "\n";
          os << " firt of " << (Int)PAN[i]->LayerXYTaqls.nelements()
             << " TaQL="; 
          //for(Int j=0;j<(Int)PAN[i]->LayerXYTaqls.nelements();j++)
          for(Int j=0;j<1;j++)
             os << PAN[i]->LayerXYTaqls[j] << "\n";
      }
   log->out(os, fnname, clname, LogMessage::DEBUG1);

   /* Send in updated plot-options to TPLP. */
   TPLP->setPlotOptions(PAN);   

   /* Start the real stuff
    
      Call BP.getData(), which looks at the TaQL string, and the "layer" info, 
      to decide whether or not to read the data from disk.   This is done, to
      optimize on disk-reads (if only the colour of a plot is changed, there
      is no reason to re-read the data from disk.)
    */   
   
   log->out( "Now get the data", fnname, clname, LogMessage::NORMAL);

   tmr.mark();
   
   try
   {
      for(Int i=0;i<(Int)(*ATBPS[panel-1]).nelements();i++) 
      {
#if LOG0
         //can be very long strings 
         {
          ostringstream os;
          os << "i=" << i 
             << " DATASTR: " << PAN[panel-1]->LayerXYTaqls[i] 
             << " for layer " << thislayer;
          log->out(os, fnname, clname, LogMessage::DEBUG1);
         }
#endif

          ((*ATBPS[panel-1])[i])->getData(
                    PAN[panel-1]->LayerXYTaqls[i],
                    thislayer,
                    PAN[panel-1]->Plop.Convert,
                    PAN[panel-1]->Plop.CallBackHooks);
      }
      log->out(String("Time spent reading X and Y data from disk: ") +
               String::toString(tmr.all()) + " sec.", 
               fnname, clname, LogMessage::DEBUG1);
      
      log->out("Done Processing data ... ", 
          fnname, clname, LogMessage::NORMAL);
      TPLP->plotData(*ATBPS[panel-1],panel,1);

      isPlotDisplayed_p = True;
   }
   catch(AipsError &x)
   {
      TablePlotError(x.getMesg());
      /* Do something here to clean up TP's data structures, 
         if the getData fails.. */
   }
   log->FnExit(fnname, clname);
   return True;
}
/*********************************************************************************/
/* replot */
/* Allow pop - and use pop to set the current panel, which to replot.
   If the panel is invalid, then replot all panels, but don't use any pop. */
/* Allow/honour ONLY layer independant paramaters */

// Allow "useflagversion", "showflags", "plotrange". 
Bool TablePlot::rePlot()
{
   for(Int p=0;p<(Int)ATBPS.nelements();p++)
   {
      if(ATBPS[p]!=NULL)
         TPLP->plotData(*ATBPS[p],p+1,0);
   }

   return True;
}

/*********************************************************************************/


/*********************************************************************************/
/*********************************************************************************/
/* get panel index */
Int TablePlot::getPanelIndex(Int nrows,Int ncols,Int panel)
{
   Int panindex = -1;
   for(Int i=0;i<(Int)PAN.nelements();i++)
      if(PAN[i] != NULL && 
         PAN[i]->Plop.PanelMap[0]==nrows && 
         PAN[i]->Plop.PanelMap[1]==ncols && 
         PAN[i]->Plop.PanelMap[2]==panel) 
         {panindex = i; break;}

   return panindex;
}

/*********************************************************************************/
/*********************************************************************************/
/* get list of overlapping panels */
/* This function is to be used to detect when panels overlap, and
   to signal to call clearplot for underlying panels */
//Bool TablePlot::clearOverLaps(Int nrows,Int ncols,Int panel)
//{
//   String fnname= "clearOverLaps";
//   Int nr,nc,pn,row,col;
//   Double newxmin,newxmax,newymin,newymax;
//   Double xmin,xmax,ymin,ymax;
//   
//   col = panel % ncols; if(col==0) col = ncols;
//   row = ( panel - col )/ncols + 1;
//   
//   newxmin = (col-1)*(1.0/ncols);
//   newxmax = (col)*(1.0/ncols);
//
//   newymin = (row-1)*(1.0/nrows);
//   newymax = (row)*(1.0/nrows);
//   
//   //log->out( String("New Panel : ")+String::toString(nrows)+
//   //         String(",")+String::toString(ncols)+
//   //         String(",")+String::toString(panel),
//   //         fnname, clname, LogMessage::DEBUG1);
//   //log->out(String("    panel limits : ")
//   //   +String::toString(newxmin)+String(" - ")
//   //   +String::toString(newxmax)+String("  and  ")
//   //   +String::toString(newymin)+String(" - ")
//   //   +String::toString(newymax),
//   //    fnname, clname, LogMessage::DEBUG1);
//   
//   for(Int i=0;i<(Int)PAN.nelements();i++)
//   {
//      if(PAN[i] != NULL)
//      {
//          Bool samePanel = False;
//          /* If it's the same panel, let it be */
//          if( PAN[i]->Plop.PanelMap[0]==nrows && 
//          PAN[i]->Plop.PanelMap[1]==ncols && 
//          PAN[i]->Plop.PanelMap[2]==panel) 
//          {samePanel=True;}
//          
//          //log->out(String("same panel : ")+String::toString(samePanel),
//          //         fnname, clname, LogMessage::DEBUG1);
//          
//          if( ! samePanel )
//          {
//         nr = PAN[i]->Plop.PanelMap[0];
//         nc = PAN[i]->Plop.PanelMap[1];
//         pn = PAN[i]->Plop.PanelMap[2];
//
//         col = pn % nc; if(col==0) col = nc;
//         row = ( pn - col )/nc + 1;
//         
//         xmin = (col-1)*(1.0/nc);
//         xmax = (col)*(1.0/nc);
//         ymin = (row-1)*(1.0/nr);
//         ymax = (row)*(1.0/nr);
//   
//         //log->out(String("Existing : ")+
//         //         String::toString(nr)+String(",")+
//         //         String::toString(nc)+String(",")+
//         //         String::toString(pn),
//         //         fnname, clname, LogMessage::DEBUG1);
//         //log->out(String("    limits : ")
//         //     +String::toString(xmin)+String(" - ")
//         //     +String::toString(xmax)+String("  and  ")
//         //     +String::toString(ymin)+String(" - ")
//         //          +String::toString(ymax),
//         //         fnname, clname, LogMessage::DEBUG1);
//      
//         if( ( (newxmin <= xmin && newxmax > xmin) || 
//               (xmin <= newxmin && xmax > newxmin) ) &&
//             ( (newymin <= ymin && newymax > ymin) || 
//               (ymin <= newymin && ymax > newymin) )  ){
//            //log->out(String("Overlaps with f: ")+
//            //         String::toString(nr)+String(",")+
//            //         String::toString(nc)+String(",")+
//            //         String::toString(pn),
//            //         fnname, clname, LogMessage::DEBUG1);
//            clearPlot(nr,nc,pn);
//          }
//      }// end of it not same panel   
//       }// end of if not null
//   }// end of for PAN
//
//   return True;
//}

Bool TablePlot::clearOverLaps(Int nrows,Int ncols,Int panel)
{
   String fnname= "clearOverLaps";
   Int nr,nc,pn,row,col;
   Double newxmin,newxmax,newymin,newymax;
   Double xmin,xmax,ymin,ymax;
   Int sc = 100000;
   
   col = panel % ncols; 
   if (col == 0) 
      col = ncols;
   row = (panel - col) / ncols + 1;
   
   newxmin = (col - 1) * (sc / ncols);
   newxmax = (col) * (sc / ncols);

   newymin = (row - 1) * (sc / nrows);
   newymax = (row) * (sc / nrows);
   
   //log->out( String("New Panel : ")+String::toString(nrows)+
   //         String(",")+String::toString(ncols)+
   //         String(",")+String::toString(panel),
   //         fnname, clname, LogMessage::DEBUG1);
   //log->out(String("    panel limits : ")
   //   +String::toString(newxmin)+String(" - ")
   //   +String::toString(newxmax)+String("  and  ")
   //   +String::toString(newymin)+String(" - ")
   //   +String::toString(newymax),
   //    fnname, clname, LogMessage::DEBUG1);
   
   for(Int i = 0; i < (Int)PAN.nelements(); i++) {

      if(PAN[i] != NULL) {
          Bool samePanel = False;
          /* If it's the same panel, let it be */
          if (PAN[i]->Plop.PanelMap[0] == nrows && 
              PAN[i]->Plop.PanelMap[1]==ncols && 
              PAN[i]->Plop.PanelMap[2]==panel) {
             samePanel = True;
          }
          
          //log->out(String("same panel : ")+String::toString(samePanel),
          //         fnname, clname, LogMessage::DEBUG1);
          
          if (!samePanel) {
             nr = PAN[i]->Plop.PanelMap[0];
             nc = PAN[i]->Plop.PanelMap[1];
             pn = PAN[i]->Plop.PanelMap[2];

             col = pn % nc; 
             if (col == 0) 
                col = nc;
             row = (pn - col) / nc + 1;
         
             xmin = (col - 1) * (sc / nc);
             xmax = (col) * (sc / nc);
             ymin = (row - 1) * (sc / nr);
             ymax = (row) *(sc / nr);
   
             //log->out(String("Existing : ")+
             //         String::toString(nr)+String(",")+
             //         String::toString(nc)+String(",")+
             //         String::toString(pn),
             //         fnname, clname, LogMessage::DEBUG1);
             //log->out(String("    limits : ")
             //     +String::toString(xmin)+String(" - ")
             //     +String::toString(xmax)+String("  and  ")
             //     +String::toString(ymin)+String(" - ")
             //          +String::toString(ymax),
             //         fnname, clname, LogMessage::DEBUG1);
      
             Double x1 = newxmax + newxmin - xmax - xmin;
             Double y1 = newymax + newymin - ymax - ymin;
             Double x2 = newxmax - newxmin + xmax - xmin;
             Double y2 = newymax - newymin + ymax - ymin;
             if (fabs(x1) < x2 && fabs(y1) < y2)
             {
                //log->out(String("Overlaps with f: ")+
                //         String::toString(nr)+String(",")+
                //         String::toString(nc)+String(",")+
                //         String::toString(pn),
                //         fnname, clname, LogMessage::DEBUG1);
                clearPlot(nr, nc, pn);
             }
          }
       }
   }

   return True;
}

/*********************************************************************************/

/*********************************************************************************/
/*********************************************************************************/

/* Mark regions on a plot */
Bool TablePlot::markRegions(Int nrows, Int ncols, Int panel, Vector<Double> &region)
{
   String fnname= "markRegions";
   log->FnEnter(fnname + "(nrows, ncols, panel, region)", clname);
        
   Bool stat=True;
        
   if(isPlotDisplayed_p) 
       TPLP->markRegions(nrows,ncols,panel,region);
   else
       log->out("A plot needs to be made before regions can be marked", 
             fnname, clname, LogMessage::WARN);

   return stat;
}

/*****************************************************************************/
/*********************************************************************************/
/*********************************************************************************/
/* Flag data for each table in the list of BasePlots */
Bool TablePlot::flagData( Int direction){
   String fnname= "flagData";   
   log->FnEnter(fnname + "(direction)", clname);

   if(direction != 0 && direction != 1)       
        TablePlotError(String("direction must be FLAG(1) or UNFLAG(0)"));

   /* Check if there is an active plot, before proceding */
   if( ! isPlotDisplayed_p ) {      
       log->out("A plot needs to be made before regions can be (un)flagged",
                 fnname, clname, LogMessage::WARN);      
       return False;
   }

   /* Send in updated plot-options to TPLP. */   
   TPLP->setPlotOptions(PAN);

   /* Send in the marked regions to the BasePlots */
   /* Get back a list of number of marked regions per panel */   
   Vector<Int> numregions;
   numregions = TPLP->setMarkedRegions(ATBPS);         

   /* Loop over all panels.      
     Operate only on panels that actually have flag regions on them */
   Int totalnumregions=0;   
   for(Int p=0;p<(Int)ATBPS.nelements();p++)
   {      
      //for each panel
      if(ATBPS[p]!=NULL && numregions(p)>0)
      {
         totalnumregions += numregions(p);
         for (Int i=0;i<(Int)(*ATBPS[p]).nelements();i++) {
            //for each layer (BaseTable)
            (*ATBPS[p])[i]->flagData(1, 0, direction);
         }

         // maybe remove this too.
         TPLP->clearPlot(p+1,True);
      }
   }

   /* Replot so that all panels reflect these new flags */
   if(totalnumregions>0) rePlot();

   /* Dump out the flag history. Return it too. */
   PtrBlock<Record*> FLAGHIST;
   Bool ret = updateFlagHistory(FLAGHIST);
   /* Print out the results */
   if(ret)
      ret = dumpFlagHistory(FLAGHIST);
   /* Clean up the PtrBlock */
   for(Int i=0;i<(Int)FLAGHIST.nelements();i++)
        delete FLAGHIST[i];
   FLAGHIST.resize(0);
   log->out(fnname, clname);
   return True;
}

/*****************************************************************************/
/*********************************************************************************/
/*********************************************************************************/
/* Flag data for each table in the list of BasePlots */
Bool TablePlot::flagData(Int direction, Vector<Int> numregions)
{
   String fnname= "flagData";
   log->FnEnter(fnname + "(direction)", clname);

   if(direction != 0 && direction != 1) 
       TablePlotError(String("direction must be FLAG(1) or UNFLAG(0)"));

   /* Send in updated plot-options to TPLP. */
   TPLP->setPlotOptions(PAN);   
   
   //cout << "flagData numergions=" << numregions << endl; 

   /* Loop over all panels.
      Operate only on panels that actually have flag regions on them */
   Int totalnumregions=0;
   for(Int p=0;p<(Int)ATBPS.nelements();p++)
   {
      //for each panel
      if(ATBPS[p]!=NULL && numregions(p)>0)
      {
         totalnumregions += numregions(p);
         for (Int i=0;i<(Int)(*ATBPS[p]).nelements();i++) {
            //for each layer (BaseTable)

            if (PAN[p]->Plop.doAverage) {
               //cout << "flag averaged" << endl;
               (*ATBPS[p])[i]->flagData(direction, 
                    PAN[p]->Plop.MSName, PAN[p]->Plop.spwExpr, 
                    PAN[p]->Plop.RowMap, PAN[p]->Plop.ChanMap,
                    PAN[p]->Plop.FlagExt);
               //Table tb;
               //(*ATBPS[p])[i]->flagData(direction, tb, 
               //     PAN[p]->Plop.RowMap, PAN[p]->Plop.ChanMap,
               //     PAN[p]->Plop.FlagExt);
            }
            else {
                //cout << "flag non-averaged" << endl;
                if (PAN[p]->Plop.FlagExt.length() > 0) {
                   //cout << PAN[p]->Plop.print() << endl;                  
                   (*ATBPS[p])[i]->flagData(direction, 
                          PAN[p]->Plop.MSName, PAN[p]->Plop.FlagExt);
                }
                else {
                   (*ATBPS[p])[i]->flagData(1, 0, direction);
                }
            }

            //The long term, all flagging will be done from the 
            //locateData->dumpLocateInfo->flagdata
         }
              
         // maybe remove this too.
         TPLP->clearPlot(p+1,True);
      }
   }

   /* Replot so that all panels reflect these new flags */
   if(totalnumregions>0) rePlot();

   /* Dump out the flag history. Return it too. */
   PtrBlock<Record*> FLAGHIST;
   Bool ret = updateFlagHistory(FLAGHIST);
   /* Print out the results */
   if(ret) 
      ret = dumpFlagHistory(FLAGHIST);
   /* Clean up the PtrBlock */
   for(Int i=0;i<(Int)FLAGHIST.nelements();i++)
        delete FLAGHIST[i];
   FLAGHIST.resize(0);
   log->out(fnname, clname);
   return True;
}

/*********************************************************************************/
/* Update flag history for each table in the list of BasePlots */
/* This applies to the flags applied in the last call to TP.flagData */
Bool TablePlot::updateFlagHistory(PtrBlock<Record*> &flaghist)
{
   String fnname= "updateFlagHistory";
   log->FnEnter(fnname + "(flaghist)", clname);
   
   /* Variables to hold the history info from each BasePlot */
   Vector<Vector<Double> > flagmarks;
   Int dirn, numflags;
   
   /* Initialize the flag history record */
   flaghist.resize(0);
   Int histcount=0;
   
   /* Iterate over panels */
   for(Int p=0;p<(Int)ATBPS.nelements();p++)
   {
      /* If this panel is valid... */
      if(ATBPS[p]!=NULL)
      {
        /* Iterate over each layer (BasePlot) for this panel */
        for(Int i=0;i<(Int)(*ATBPS[p]).nelements();i++) 
        {
            /* Get flag history info from this BasePlot */
         (*ATBPS[p])[i]->updateFlagHistory(flagmarks, dirn, numflags);

         /* If flags have been applied, append to the history record */
         if(flagmarks.nelements()>0)
         {
            /* Create new flag record entry */
            flaghist.resize(histcount+1);
            flaghist[histcount] = new Record();
         
            /* panel number */
            flaghist[histcount]->define("panel",PAN[p]->Plop.PanelMap);
   
            /* Table name, selection string, taql strings */
            flaghist[histcount]->define("baseplotinfo",
               (*ATBPS[p])[i]->getBasePlotInfo());
            
            /* Flag or Unflag */
            flaghist[histcount]->define("direction",
                  (dirn?String("flag"):String("unflag")));
            
            /* Total number of selected points */
            flaghist[histcount]->define("numpoints",numflags);

            /* Number of marked regions */
            flaghist[histcount]->define("numregions",(Int)flagmarks.nelements());
            
            /* Marked regions */
            for(Int j=0;j<(Int)flagmarks.nelements();j++)
               flaghist[histcount]->define(String("box.")+
                 String::toString(j+1),flagmarks[j]);
            
            histcount++;
         }
        }
      }
   }
   log->FnExit(fnname, clname);
   return True;
}
/*********************************************************************************/
/* Dump the flag history obtained via TP.updateFlagHistory */
/* Currently this prints out to the logger */
Bool TablePlot::dumpFlagHistory(PtrBlock<Record*> &flaghist)
{
   String fnname= "dumpFlagHistory";
   log->FnEnter(fnname + "(flaghist)", clname);
   
   /* Variables to read out from the records */
   Vector<Int> panelnum;
   Vector<Double> flagregion;
   Vector<String> infostr;
   String str("unknown");
   Int nreg=0;
   
   /* Iterate over the list of history records */
   /* Extract its fields, and print them */
   for(Int i=0;i<(Int)flaghist.nelements();i++)
   {

      if((*flaghist[i]).isDefined("panel"))
      {
         RecordFieldId ridpanel("panel");
         (*flaghist[i]).get(ridpanel,panelnum);
         ostringstream os;
         os << "Panel: " << panelnum; 
         log->out(os, fnname, clname, LogMessage::NORMAL); 
      }
      
      if((*flaghist[i]).isDefined("baseplotinfo"))
      {
         RecordFieldId ridtab("baseplotinfo");
         (*flaghist[i]).get(ridtab,infostr);
         log->out(String("Table: ") + infostr[0],
                  fnname, clname, LogMessage::NORMAL);
      
         if(infostr[1].length()==0) 
            log->out(String("Sub-Selection: None "),
                  fnname, clname, LogMessage::NORMAL);
         else          
            log->out(String("Sub-Selection: ") + infostr[1],
                  fnname, clname, LogMessage::NORMAL);
         
         ostringstream os;
         for(Int j=2;j<(Int)infostr.nelements();j++)
            os << " : " << infostr[j];
         log->out(os, fnname, clname, LogMessage::NORMAL);
      }
      
      if((*flaghist[i]).isDefined("direction"))
      {
         RecordFieldId riddir("direction");
         (*flaghist[i]).get(riddir,str);
      }
      
      if((*flaghist[i]).isDefined("numpoints"))
      {
         RecordFieldId ridn("numpoints");
         (*flaghist[i]).get(ridn,nreg);
         
         log->out(String("TaQL XY Number of selected points: ") +
                  String::toString(nreg),
                  fnname, clname, LogMessage::NORMAL);
      }
      
      if((*flaghist[i]).isDefined("numregions"))
      {
         RecordFieldId ridn("numregions");
         (*flaghist[i]).get(ridn,nreg);
         log->out(String("Number of marked regions: ") +
                  String::toString(nreg),
                  fnname, clname, LogMessage::NORMAL);
      }
      
      for(Int j=0;j<nreg;j++)
      {
         if((*flaghist[i]).isDefined(String("box.")+String::toString(j+1)))
         {
            RecordFieldId rid(String("box.")+String::toString(j+1));
            (*flaghist[i]).get(rid,flagregion);
            ostringstream os;
            os << str << " : " << flagregion;
            log->out(os, fnname, clname, LogMessage::NORMAL);
         }
      }
   }         
   return True;
}

Bool TablePlot::locateData(Int doFlag) {
    Vector<String> clist;
    clist.resize(0);
    PtrBlock<Record*> INFO;
    Bool ret = locateData(clist, INFO, doFlag);
    /* Clean up the PtrBlock */
    for(Int i=0;i<(Int)INFO.nelements();i++)
        delete INFO[i];
    INFO.resize(0);

    return ret;
}

/*********************************************************************************/
/// TODO Hold different columnlists for different BasePlots - in ATBPS[i].
/* Print info about selected data for each table in the list of BasePlots */
Bool TablePlot::locateData(Vector<String> columnlist, PtrBlock<Record*> &INFO,
        Int doFlag)
{
   String fnname= "locateData";
   ostringstream os;
   os << fnname << "(columnlist=" << columnlist << "INFO)";
   log->FnEnter(os, clname);

   /* Check if there is an active plot, before proceding */
   if(! isPlotDisplayed_p ) {
      log->out("A plot needs to be made before 'locate' queries can be made", 
               fnname, clname, LogMessage::WARN);
      return False;
   }

   // CHECK FOR EMPTY STRING !!!
   if((Int)columnlist.nelements()==1 && (columnlist[0]).length()==0)
      columnlist.resize(0);
   //Int ncoll = columnlist.nelements()+2;

   /* Add two extra columns to the ones supplied - 
      to hold the number of points per row
      and the row number */
   /* This info maaaaaaaaay be useful when there are multiple 
      chans/pols plotted together */
   //Vector<String> loccollist(ncoll);
   //loccollist[0] = "ROW";
   //loccollist[1] = "NPTS";
   //for(Int i=2;i<ncoll;i++) loccollist[i] = columnlist[i-2];
   
   /* Initialize the data structure to return */
   INFO.resize(0);
   Int reccount = 0;

   /* Send in the marked regions to the BasePlots */
   /* Get back a list of number of marked regions per panel */
   Vector<Int> numregions;
   numregions = TPLP->setMarkedRegions(ATBPS);
   {
      ostringstream os;
      os << "nummarkedregions " << numregions,
      log->out(os, fnname, clname, LogMessage::DEBUG2);
   }

   //loop over all panels, check whether flag extension is required
   Bool extend = False;
   Bool average = False;
   for(Int p = 0; p < (Int)ATBPS.nelements(); p++) {
      if(ATBPS[p]!=NULL && numregions(p) > 0) {
         for (Int i = 0; i < (Int)(*ATBPS[p]).nelements(); i++) {
            //cout << "BP=" << p 
            //     << " extendflag=" << PAN[p]->Plop.FlagExt
            //     << endl;
            if (PAN[p]->Plop.FlagExt.length() > 0) {
               extend = True;
            }
            if (PAN[p]->Plop.doAverage) {
               average = True;
            }
         }
      }
   }
   //cout << "locateData extend or average=" << (extend || average) << endl;

   //handle non-extend-flag using existing 
   //long term, handle all flagging in a unified way regardless extendflag
   if (!(extend /*|| average*/) && doFlag != -1) {
      return flagData(doFlag, numregions);
   }

   /* Loop over all panels, and append to the list of Info Records.
      Operate only on panels that actually have flag regions on them */
   log->out("Start accumulating..", fnname, clname, LogMessage::DEBUG2);

   log->out("Located the following points:", 
            fnname, clname, LogMessage::DEBUG2);
   for(Int p=0; p<(Int)ATBPS.nelements(); p++)
   {
      //cout << "locateData ATBPS[" << p << "]=" << ATBPS[p] 
      //     << " numregions(p)=" << numregions(p) << endl;
      if(ATBPS[p]!=NULL)
      if(numregions(p)>0)
      {
         /* Iterate over BasePlots */
         for(Int i=0;i<(Int)(*ATBPS[p]).nelements();i++) 
         {
         
            /* Append to list of records */
            INFO.resize(reccount+1,True);
            INFO[reccount] = new Record();

            /* panel number */
            (*INFO[reccount]).define("panel",PAN[p]->Plop.PanelMap);
   
            /* table name */
            (*INFO[reccount]).define("baseplotinfo",
                                     (*ATBPS[p])[i]->getBasePlotInfo());
            Vector<Int> bpindex(2);
            bpindex[0] = p;
            bpindex[1] = i;
            (*INFO[reccount]).define("bpindex",bpindex);

            /* Ask BasePlot for numbers */
            /* This matrix datatype is Double... 
               ...not great... but okay to start off */
            // If a col list is specified in this function,
            // this overrides what's in the plotoption.
            Int ncolumns=2;
            if (columnlist.nelements())
               ncolumns = columnlist.nelements() + 2;
            else 
               ncolumns = PAN[p]->LayerLocateColumns[i].nelements() + 2;
                                
            Vector<String> incolumns(ncolumns);
            incolumns[0] = "ROW";
            incolumns[1] = "NPTS";
            for (Int j=2;j<ncolumns;j++) {
               if (columnlist.nelements())
                  incolumns[j] = columnlist[j-2];
               else 
                  incolumns[j] = (PAN[p]->LayerLocateColumns[i])[j-2];
            }
           
            Matrix<Double> matinfo(0,0);
            Vector<String> cpol;
            if (PAN[p]->Plop.doAverage) {
               //cout << "doAverage-----" << endl;
               //cout << "nP=" << (Int)ATBPS.nelements() 
               //     << " p=" << p 
               //     << " l=" << i << endl;
               (*ATBPS[p])[i]->locateData(incolumns, matinfo, cpol, 
                      PAN[p]->Plop.RowMap,  PAN[p]->Plop.ChanMap);
            }
            else {
               //cout << "-----" << endl;
               (*ATBPS[p])[i]->locateData(incolumns,matinfo,cpol);
            }
            
            /* Get the list of marked regions */
            Vector<Vector<Double> > regmarks;
            Int dirn, numpoints;
            (*ATBPS[p])[i]->updateFlagHistory(regmarks, dirn, numpoints);
            
            /* Total number of selected points */
            (*INFO[reccount]).define("numpoints",numpoints);

            /* If regions have been marked, record these regions. */
            if(regmarks.nelements()>0)
            {
	      (*INFO[reccount]).define("numregions",(Int)regmarks.nelements());
               for(Int j=0;j<(Int)regmarks.nelements();j++)
                  (*INFO[reccount]).define(
                     String("box.")+String::toString(j+1),regmarks[j]);
            }

            /* List of column names */
            (*INFO[reccount]).define("columnlist",incolumns);
            
            /* the numbers */
            (*INFO[reccount]).define("infomatrix",matinfo);
            // Note : This is probably making a copy of "matinfo"

            /* chan,pol strings */
            (*INFO[reccount]).define("chanpol",cpol);
            
            reccount++;
         }
      }
   }

   //cout << "dumpLocateInfo nRec=" << INFO.nelements() 
   //     << " pCount=" << ATBPS.nelements()
   //     << " doFlag=" << doFlag
   //     << " average=" << average << endl;
   /* Print out the locate info as well */

   //if (average) {
   //   (*ATBPS[0])[0]->showFlags();
   //}

   dumpLocateInfo(INFO, doFlag);
   
   INFO.resize(0);
   reccount = 0;

   if (doFlag != -1 && average) {
   if (False) {
   for(Int p=0; p<(Int)ATBPS.nelements(); p++)
   {
      //cout << "locateData ATBPS[" << p << "]=" << ATBPS[p] 
      //     << " numregions(p)=" << numregions(p) << endl;
      if(ATBPS[p]!=NULL)
      if(numregions(p)>0)
      {
         /* Iterate over BasePlots */
         for(Int i=0;i<(Int)(*ATBPS[p]).nelements();i++) 
         {
         
            /* Append to list of records */
            INFO.resize(reccount+1,True);
            INFO[reccount] = new Record();

            /* panel number */
            (*INFO[reccount]).define("panel",PAN[p]->Plop.PanelMap);
   
            /* table name */
            (*INFO[reccount]).define("baseplotinfo",
                                     (*ATBPS[p])[i]->getBasePlotInfo());
            Vector<Int> bpindex(2);
            bpindex[0] = p;
            bpindex[1] = i;
            (*INFO[reccount]).define("bpindex",bpindex);

            /* Ask BasePlot for numbers */
            /* This matrix datatype is Double... 
               ...not great... but okay to start off */
            // If a col list is specified in this function,
            // this overrides what's in the plotoption.
            Int ncolumns=2;
            if (columnlist.nelements())
               ncolumns = columnlist.nelements() + 2;
            else 
               ncolumns = PAN[p]->LayerLocateColumns[i].nelements() + 2;
                                
            Vector<String> incolumns(ncolumns);
            incolumns[0] = "ROW";
            incolumns[1] = "NPTS";
            for (Int j=2;j<ncolumns;j++) {
               if (columnlist.nelements())
                  incolumns[j] = columnlist[j-2];
               else 
                  incolumns[j] = (PAN[p]->LayerLocateColumns[i])[j-2];
            }
           
            Matrix<Double> matinfo(0,0);
            Vector<String> cpol;
            (*ATBPS[p])[i]->locateData(incolumns,matinfo,cpol);
            
            /* Get the list of marked regions */
            Vector<Vector<Double> > regmarks;
            Int dirn, numpoints;
            (*ATBPS[p])[i]->updateFlagHistory(regmarks, dirn, numpoints);
            
            /* Total number of selected points */
            (*INFO[reccount]).define("numpoints",numpoints);

            /* If regions have been marked, record these regions. */
            if(regmarks.nelements()>0)
            {
	      (*INFO[reccount]).define("numregions",(Int)regmarks.nelements());
               for(Int j=0;j<(Int)regmarks.nelements();j++)
                  (*INFO[reccount]).define(
                     String("box.")+String::toString(j+1),regmarks[j]);
            }

            /* List of column names */
            (*INFO[reccount]).define("columnlist",incolumns);
            
            /* the numbers */
            (*INFO[reccount]).define("infomatrix",matinfo);
            // Note : This is probably making a copy of "matinfo"

            /* chan,pol strings */
            (*INFO[reccount]).define("chanpol",cpol);
            
            reccount++;
         }
      }
   }

   //cout << "dumpLocateInfo nRec=" << INFO.nelements() 
   //     << " pCount=" << ATBPS.nelements()
   //     << " doFlag=" << doFlag
   //     << " average=" << average << endl;
   /* Print out the locate info as well */

   //if (average) {
   //   (*ATBPS[0])[0]->showFlags();
   //}

   dumpLocateInfo(INFO, doFlag, True);
      
   }
   else {
     log->out(String("Done flagging. Please run plotxy to reload the MS."),
               fnname, clname, LogMessage::NORMAL2, True);
   }
   }

   if (doFlag != -1)
       rePlot();

   log->FnExit(fnname, clname);

   return True;
}

/*********************************************************************************/
/* Print out the results of "locateData" */
Bool TablePlot::dumpLocateInfo(PtrBlock<Record*> &INFO, Int doFlag, Bool doAve)
{
   String fnname= "dumpLocateInfo";
   log->FnEnter(fnname + "(INFO)", clname);

   Int nRec = INFO.nelements();
   Vector<Int> panelnum;
   Vector<String> bpinfo;
   Vector<String> collist;
   Matrix<Double> infomat;
   Vector<String> cpol;
   Int nreg=0;
   Vector<Double> region;

   TPGuiCallBackHooks * callbackhooks=NULL;
   TPGuiCallBackHooks * genericcallback = new TPGuiCallBackHooks();
   
   for(Int i=0;i<nRec;i++)
   {
      if((*INFO[i]).isDefined("panel"))
      {
         RecordFieldId ridpanel("panel");
         (*INFO[i]).get(ridpanel,panelnum);
         //oss << "Panel : " << panelnum << LogIO::POST;
      }
                
      callbackhooks = NULL;
      if((*INFO[i]).isDefined("bpindex"))
      {
         Vector<Int> bpindex;
         RecordFieldId ridpanel("bpindex");
         (*INFO[i]).get(ridpanel,bpindex);
         callbackhooks = (*ATBPS[bpindex[0]])[bpindex[1]]->callbackhooks_p;
      }
      if (callbackhooks == NULL )
         callbackhooks = genericcallback;
      
      if((*INFO[i]).isDefined("baseplotinfo"))
      {
         RecordFieldId ridtab("baseplotinfo");
         (*INFO[i]).get(ridtab,bpinfo);
      
         //oss << "Table : " << bpinfo[0] << LogIO::POST;
         //if (bpinfo[1].length()==0) 
         //   oss << "Sub-Selection : None" << LogIO::POST;
         //else 
         //   oss << "Sub-Selection : " << bpinfo[1] << LogIO::POST;
         
         //oss << "TaQL X,Y " ;
         //for (Int j=2;j<(Int)bpinfo.nelements();j++) 
         //   oss << " : " <<  bpinfo[j];
         //oss << LogIO::POST;
      }
      
      if((*INFO[i]).isDefined("columnlist"))
      {
         RecordFieldId ridcol("columnlist");
         (*INFO[i]).get(ridcol,collist);
      }
      
      if((*INFO[i]).isDefined("infomatrix"))
      {
         RecordFieldId ridmat("infomatrix");
         (*INFO[i]).get(ridmat,infomat);
      }
      
      if((*INFO[i]).isDefined("chanpol"))
      {
         RecordFieldId ridcp("chanpol");
         (*INFO[i]).get(ridcp,cpol);
      }
      
      if((*INFO[i]).isDefined("numpoints"))
      {
         RecordFieldId ridn("numpoints");
         (*INFO[i]).get(ridn,nreg);
         //oss << "Number of selected points : " << nreg << LogIO::POST;
      }
      
      if((*INFO[i]).isDefined("numregions"))
      {
         RecordFieldId ridn("numregions");
         (*INFO[i]).get(ridn,nreg);
         //oss << "Number of marked regions : " << nreg << LogIO::POST;
      }
      
      for(Int j=0;j<nreg;j++)
      {
         if((*INFO[i]).isDefined(String("box.")+String::toString(j+1)))
         {
            RecordFieldId rid(String("box.")+String::toString(j+1));
            (*INFO[i]).get(rid,region);
            //oss << " locate : " << region << LogIO::POST;
         }
      }

   /* Print out the columns and their values. This is separate for each BP*/
   /* This fn can also come via a plotoption from an application class */

     //cout << "doFlag=" << doFlag 
     //       << " callbackhooks=" << callbackhooks << endl;

     //IPosition mshape = infomat.shape();
     //cout << "infomat.shape=[" << mshape[0] << ", "
     //     << mshape[1] << "] cpol.size=" << cpol.size() << endl;

     if (callbackhooks) {
        if (doFlag != -1)
           callbackhooks->flagdata(doFlag, collist, infomat, cpol, doAve);
        else
           callbackhooks->printlocateinfo(collist,infomat,cpol);
     }
   }

   delete genericcallback;
   log->FnExit(fnname, clname);   
   return True;
}
/*********************************************************************************/
/* Clear Plot */
/* If TableTouch_p = False, then don't clear out the BPs !! Only reset the PanelParams */
Bool TablePlot::clearPlot(Int nrows, Int ncols, Int panel)
{
   String fnname= "clearPlot";
   String fnCall = String("(") + String::toString(nrows)
       + String(", ") + String::toString(ncols) + String(", ")
       + String::toString(panel) + String(")");
   log->FnEnter(fnname + fnCall, clname);

   /* If no arguments are sent in, this is the default - clear the whole window */
   if(panel==0 && nrows==0 && ncols==0)
   {
      
      for(Int i=0;i<(Int)ATBPS.nelements();i++)
      {
         if(ATBPS[i]!=NULL) 
         {
            for(Int j=0;j<(Int)(*ATBPS[i]).nelements();j++) 
               deleteBasePlot(i,j);
               //delete (*ATBPS[i])[j];
            delete ATBPS[i];
            ATBPS[i]=NULL;
         }
         
      }
      ATBPS.resize(0,True);
      
      for(Int i=0;i<(Int)PAN.nelements();i++)   delete PAN[i];
      PAN.resize(0,True);
      TPLP->clearPlot(0,True);
      
                isPlotDisplayed_p = False;
      //resetTP();
   }
   else /* A specific panel is to be cleared */
   {
      /* Check if this panel exists, and get its index */
      Int panelindex;
      panelindex = getPanelIndex(nrows,ncols,panel);
      
      /* If the panel does not exist in the list - say so. */
      if(panelindex == -1) 
      {
          ostringstream os;
          os << "List of existing panels :\n";
          for(Int i=0;i<(Int)ATBPS.nelements();i++) {
             if(ATBPS[i]!=NULL) 
                os << PAN[i]->Plop.PanelMap << "\n";
          }
          log->out(os, fnname, clname, LogMessage::DEBUG1);
          log->out(String("[") + String::toString(nrows) + 
               String(",") + String::toString(ncols) + String(",") + 
               String::toString(panel) + String("]") + 
               String(" is a non-existant panel "),
               fnname, clname, LogMessage::SEVERE);

         return False;
         
      }

      /* If the panel is in the list, then check if it has a live 
         ATBPS entry and clean up */
      if( (Int)ATBPS.nelements() >= panelindex  && 
          ATBPS[panelindex] != NULL && PAN[panelindex] != NULL) 
      {
              log->out( "CLEARING " );
         TPLP->clearPlot(panelindex+1,True);

         Int start=0;
         
         start = 0;
         for(Int j=start;j<(Int)(*ATBPS[panelindex]).nelements();j++) 
                                deleteBasePlot(panelindex,j);
                                //delete (*ATBPS[panelindex])[j];
         if( start == 0 )
         {
            delete ATBPS[panelindex];
            ATBPS[panelindex]=NULL;
            
            delete PAN[panelindex];
            PAN[panelindex]=NULL;
         }
         else
         {
            ATBPS[panelindex]->resize(1,True);
            //PAN[panelindex]->reset();
            PAN[panelindex]->changeNlayers((*ATBPS[panelindex]).nelements());
         }
      }
                /* If all plots have been cleared, set the "no plot displayed" flag */
                Bool atleastoneplot=False;
                for(Int i=0;i<(Int)ATBPS.nelements();i++) 
                {
                        if(ATBPS[i]!=NULL) atleastoneplot=True;
                }
                if( ! atleastoneplot ) isPlotDisplayed_p = False;
   }

   log->FnExit(fnname, clname);   
   return True;
}

/*********************************************************************************/
/* Plot by iterating over a specified iteration axis. */
/*********************************************************************************/

Bool TablePlot::iterMultiPlotStart(PlotOptions &pop,Vector<String> &datastr,Vector<String> &iteraxes)
{
        Vector<Vector<String> > xytaqlvec(TABS_p.nelements());
        for(uInt i=0;i<xytaqlvec.nelements();i++) xytaqlvec[i] = datastr;
        return iterMultiPlotStart(pop,xytaqlvec,iteraxes);
}

Bool TablePlot::iterMultiPlotStart(PlotOptions &pop,Vector<Vector<String> > &datastrvector,Vector<String> &iteraxes)
{
   //cout << "field iter go here========" << endl;
   String fnname= "iterMultiPlotStart";
   log->FnEnter(fnname + "(pop, datastrvector, iteraxes)", clname);

   // May need to not do this if overplotting with multiplots?
   if(IterPlotOn_p ){iterMultiPlotStop();}


   if(TABS_p.nelements()<=0)
      TablePlotError(String("No Table is Set...."));

   log->out(String("TableNames : ")+TABS_p[0].tableName());
   
   /* CHECK VALIDITY OF INPUT PARAMS */
   Vector<String> Errors(checkInputs(pop,datastrvector,iteraxes));
   if(Errors[0].length()>0) TablePlotError(String("Error in Inputs:"));

   /* stays same for all overplots */
   IterAxes_p.resize(iteraxes.nelements());
   IterAxes_p = iteraxes;

   //cout << "IterAxes_p=" << IterAxes_p << endl;
   //for (int i = 0; i < datastrvector.nelements(); i++) {
   //   for (int j = 0; j < datastrvector(0).nelements(); j++) {
   //      cout << "i=" << i << " j=" << j 
   //           << " " << datastrvector(i)(j) << endl;
   //   }
   //}
   //pop.print();


   Block<String> itx;
   itx.resize(IterAxes_p.nelements()); // remove !
   for(Int i=0;i<(Int)IterAxes_p.nelements();i++)    
        itx[i] = IterAxes_p[i]; 

   /* Read in the nrows and ncols for the iterplots */
   /* If overplot, maintain a list of pops. */
   /* Apply these pops and loop over getData with each pop 
      - inner-most loop */

   Int npop = Pops_p.nelements();

   try
   {
      if(!pop.OverPlot)
      {
         log->out(String("Fresh Plot : npop : ")+String::toString(npop));
         for(Int i=0;i<npop;i++)
         {
            delete Pops_p[i];
            delete TaqlStr_p[i];
            delete OvpRootTabNames_p[i];
            delete OvpTabSelections_p[i];
            delete OvpIters_p[i];
         }
      
         Pops_p.resize(1);
         Pops_p[0] = new PlotOptions(pop);
      

         TaqlStr_p.resize(1);

         // This is called geesh, because this is a geesh 
         // variable.  We shouldn't need to create it.  It
         // seems that when you pass a pointer into the
         // Vector<Vector<String>> constructor the data isn't
         // copied correctly, I suspect a pointer is being 
         // copied instead of the data.  The one-liner that
         // would be nice to have is commented out at the end.
         Vector<Vector<String> > geesh( datastrvector.nelements() );
         for ( uInt ii=0; ii < geesh.nelements(); ii++ )
             geesh[ii] = datastrvector[ii];
         TaqlStr_p[0] = new Vector<Vector<String> >(geesh);
         
         //TaqlStr_p[0] = new Vector<Vector<String> >(datastrvector);
         OvpRootTabNames_p.resize(1);
         OvpRootTabNames_p[0] = new Vector<String>(nTabs_p);
         
         OvpTabSelections_p.resize(1);
         OvpTabSelections_p[0] = new Vector<String>(nTabs_p);
         
         OvpIters_p.resize(1);
         OvpIters_p[0] = new Vector<TableIterator>(nTabs_p);

                        npop = 1;
      }
      else
      {
         log->out(String("Over Plot : npop : ")+String::toString(npop));
         Pops_p.resize(npop+1,True);
         Pops_p[npop] = new PlotOptions(pop);


         TaqlStr_p.resize(npop+1,True);

         // This is called geesh, because this is a geesh 
         // variable.  We shouldn't need to create it.  It
         // seems that when you pass a pointer into the
         // Vector<Vector<String>> constructor the data isn't
         // copied correctly, I suspect a pointer is being 
         // copied instead of the data.  The one-liner that
         // would be nice to have is commented out at the end.
         Vector<Vector<String> > geesh( datastrvector.nelements() );
         for ( uInt ii=0; ii < geesh.nelements(); ii++ )
             geesh[ii] = datastrvector[ii];
         TaqlStr_p[npop] = new Vector<Vector<String> >(geesh);
         //TaqlStr_p[npop] = new Vector<Vector<String> >(datastrvector);

         OvpRootTabNames_p.resize(npop+1,True);
         OvpRootTabNames_p[npop] = new Vector<String>(nTabs_p);
         
         OvpTabSelections_p.resize(npop+1,True);
         OvpTabSelections_p[npop] = new Vector<String>(nTabs_p);
         
         OvpIters_p.resize(npop+1,True);
         OvpIters_p[npop] = new Vector<TableIterator>(nTabs_p);

         npop++;
      }

      /* Fill in the values for iterators for each table, 
         and root names and select strings */
      for(Int i=0;i<nTabs_p;i++) {
         (*OvpIters_p[npop-1])[i] = TableIterator(TABS_p[i],itx);
         (*OvpRootTabNames_p[npop-1])[i] = rootTabNames_p[i];
         (*OvpTabSelections_p[npop-1])[i] = tableSelections_p[i] 
                       + String("\nIterating on ");
         for(Int it=0;it<(Int)itx.nelements();it++)
             (*OvpTabSelections_p[npop-1])[i] += String(" : ") + itx[it];
      }
                
      log->out(String("nTabs_p : ")+String::toString(nTabs_p));
      log->out(String("npop : ")+String::toString(Pops_p.nelements()));
      /* Check that the number of iterations are the same
         for multiple overplots. It is assumed that the same IterAxes exists
         for all the tables involved */
      
   }
   catch(TableError &x)
   {
      TablePlotError(x.getMesg());
   }

   NRows_p = pop.PanelMap[0];
   NCols_p = pop.PanelMap[1];
        Separate_p = pop.SeparateIter;

   TitleStrings_p.resize(Pops_p.nelements(),True);
   for(uInt i=0;i<Pops_p.nelements();i++) 
      TitleStrings_p[i] = Pops_p[i]->Title;

   IterPlotStarted_p = True;

   log->FnExit(fnname, clname);
   return True;
}
/* Next iteration */
Int TablePlot::iterMultiPlotNext(Vector<String> &labelcols, 
       Vector<Vector<Double> > &labelvals)
{ 
   String fnname= "iterMultiPlotNex";
   log->FnEnter(fnname + "(labelcols, labelvals)", clname);

   //cout << "labelcols=" << labelcols << endl;
   //for (Int i = 0; i < labelvals.nelements(); i++)
   //  cout << "labelvals=" << labelvals[i] << endl;

   if( ! IterPlotStarted_p ) {
       log->out("No more iterations", fnname, clname, LogMessage::NORMAL);
       return -1;
   }
   
   Int finished = 0;

	   //cout << "IterAxes_p=" << IterAxes_p << endl;

   labelcols.resize(0);
   labelcols.resize(IterAxes_p.nelements());
   labelvals.resize(0);
   //labelvals.resize(NRows_p*NCols_p);
   if(Separate_p.matches("col") || Separate_p.matches("row") ) 
      labelvals.resize(NRows_p*NCols_p*Pops_p.nelements());
   else 
      labelvals.resize(NRows_p*NCols_p);

   //cout << "labelvals.nelements()=" << labelvals.nelements() << endl;
   for(Int i=0;i<(Int)labelvals.nelements();i++) 
      labelvals[i].resize(IterAxes_p.nelements());
   TableExprNode ten; 
   TableExprId tid(0);
   Double datat;

   log->out(String("Num of overplots : ")+String::toString(Pops_p.nelements()));
   log->out(String("nrows : ")+String::toString(NRows_p) +
            String("  ncols : ")+String::toString(NCols_p));
      
   if((*OvpIters_p[0])[0].pastEnd()) 
      finished = -1; 
   
   IterPlotOn_p=True;
   
   if( finished == 0 )
   {
      TPGuiCallBackHooks * genericcallback = new TPGuiCallBackHooks();
      //clearPlot(0,0,0);
      
      //cout << "NRows_p=" << NRows_p
      //     << " NCols_p=" << NCols_p << endl;

      CurrentPanel_p=1;
      for (Int row=0;row<NRows_p;row++)
         for(Int col=0;col<NCols_p;col++) {

            if((*OvpIters_p[0])[0].pastEnd()) { 
                finished = -1; 
		break; 
             }

           //cout << "Pops_p.nelements()=" << Pops_p.nelements() << endl;
           //cout << "Separate_p=" << Separate_p << endl;
      
           for(Int pl=0;pl<(Int)Pops_p.nelements();pl++) {
              // if parallel iteration, compute currentpanel correctly here.
              // multiply ncols_p by npops -> to separate along cols 
              // multiply nrows_p by npops -> to separate along rows.

              if(Separate_p.matches("col")) {
                 Pops_p[pl]->PanelMap[0] = NRows_p;
                 Pops_p[pl]->PanelMap[1] = NCols_p * Pops_p.nelements();
                 CurrentPanel_p = Pops_p[pl]->PanelMap[1]*row + col 
                                 + NCols_p*pl + 1;
                 Pops_p[pl]->PanelMap[2] = CurrentPanel_p;
                 Pops_p[pl]->OverPlot = False;
              }
              else if(Separate_p.matches("row")) {
                 Pops_p[pl]->PanelMap[0] = NRows_p * Pops_p.nelements();
                 Pops_p[pl]->PanelMap[1] = NCols_p;
                 CurrentPanel_p = NCols_p*NRows_p*pl + NCols_p*row + col + 1;
                 Pops_p[pl]->PanelMap[2] = CurrentPanel_p;
                 Pops_p[pl]->OverPlot = False;
              }
              else {
                 Pops_p[pl]->PanelMap[0] = NRows_p;
                 Pops_p[pl]->PanelMap[1] = NCols_p;
                 CurrentPanel_p = NCols_p * row + col + 1;
                 Pops_p[pl]->PanelMap[2] = CurrentPanel_p;
              }
         
              /* Get the current table selection */
              Vector<Table> tvec(OvpIters_p[pl]->nelements());
              for(Int i=0;i<(Int)OvpIters_p[pl]->nelements();i++) {
                 tvec[i] = (*OvpIters_p[pl])[i].table();
                 //cout << "tvec[i]=" << tvec[i] << endl;
                 if(tvec[i].nrow() < 1) 
                    TablePlotError(String("No rows !")); 
              }
         
              /* Get labels for the current selection */
              // extract the vector<col names>
              // extract the "datat" for each one => vector<double>
              // send this into the callback - to generate the title
              // set the title i.e. append to TitleStrings_p.

              Vector<Double> itervals(IterAxes_p.nelements());
              itervals.set(0.0);
              for(Int j=0;j<(Int)IterAxes_p.nelements();j++) {
                 ten = RecordGram::parse(tvec[0],IterAxes_p[j]);
                 if(ten.isScalar()) {
                    ten.get(tid,datat);
                    itervals[j] = datat;
                    labelcols[j]=IterAxes_p[j];
                    (labelvals[CurrentPanel_p-1])[j]=(Double)datat;
                 }
                 else 
                    TablePlotError(String("Cannot iterate on an ArrayColumn."));
              }
         
              String labstr("");
              if(Pops_p[pl]->CallBackHooks == NULL )
                  genericcallback->createiterplotlabels(
                            IterAxes_p,itervals,labstr);
              else
                  Pops_p[pl]->CallBackHooks->createiterplotlabels(
                            IterAxes_p,itervals,labstr);
                        
              log->out(labstr);
              Pops_p[pl]->Title = TitleStrings_p[pl] + labstr;

              //if iterate over baseline,
              //may be possible to distinquish selfcorr/crosscorr
              //then skip some of them
                        
              log->out(String("OverPlot parameter : ")+
                       String::toString(Pops_p[pl]->OverPlot) );
                        
               setTableT(tvec, *OvpRootTabNames_p[pl],*OvpTabSelections_p[pl]);
               plotData(*(Pops_p[pl]),*(TaqlStr_p[pl]),String("iter_on"));
                        
               for(Int i=0;i<(Int)OvpIters_p[pl]->nelements();i++)
                  (*OvpIters_p[pl])[i].next();
                        
        }// for pl
        //CurrentPanel_p++;
      }
      delete genericcallback;
   }// if not finished.
   //TPLP->show();
   
   if( finished == -1 ) 
      log->out("End of iterations", fnname, clname,
               LogMessage::NORMAL);

   log->FnExit(fnname, clname);
   return finished;
}
   
   

/*********************************************************************************/
/*********************************************************************************/
   
/* Stop iterations */
Bool TablePlot::iterMultiPlotStop()
{
   String fnname= "iterMultiPlotStop";
   log->FnEnter(fnname + "()", clname);
   if(IterPlotStarted_p || IterPlotOn_p)
   {

      clearPlot(0,0,0);
      
      for(Int i=0;i<(Int)Pops_p.nelements();i++)
      {
         if(Pops_p[i] != NULL) 
            delete Pops_p[i];
         delete TaqlStr_p[i];
         delete OvpRootTabNames_p[i];
         delete OvpTabSelections_p[i];
         delete OvpIters_p[i];
      }
      Pops_p.resize(0,True);
      OvpRootTabNames_p.resize(0,True);
      OvpTabSelections_p.resize(0,True);
      OvpIters_p.resize(0,True);

      // Once in iteration mode, the TABS_p forgets about the original 
      // tables that were sent in. They get replaced by the mini-tables 
      // obtained from TableIterator. Therefore, it's safe to clean up 
      // the TABS_p and force the user to re-enter 
      // the Table.   
      // However - there is a problem if you want to interupt one 
      // iterplot squence by another... even if you set a new table in 
      // between.  This check takes care of this...
      if( ! ( IterPlotStarted_p && IterPlotOn_p ) ) {
         // This check is not great - but okay.
         TABS_p.resize(0,True);
         nTabs_p=0;
      }
      
      IterPlotOn_p=False;
      IterPlotStarted_p=False;
   }
   log->FnExit(fnname, clname); 
   return True;
}

/*********************************************************************************/
Bool TablePlot::clearAllFlags(Bool forRootTable)
{
   String fnname= "clearAllFlags";
   log->FnEnter(fnname + "(forRootTable)", clname);
   if(nTabs_p)
   {
       ostringstream os;
       os << "Root Table: " << rootTabNames_p;
       log->out(os, fnname, clname, LogMessage::DEBUG1);

      /* Call clearFlags for all tables in the latest setTableT */
      for(Int i=0;i<(Int)TABS_p.nelements();i++) {
         if(forRootTable && rootTabNames_p[i].length()>0) {
            try {
               FlagVersion 
                  fv(rootTabNames_p[i],dataFlagColName_p,rowFlagColName_p);
               fv.clearAllFlags();
            }
            catch (AipsError x) {
               log->out(String("Disabling flag version support for ") +
                  rootTabNames_p[i], fnname, clname, LogMessage::WARN);
               rootTabNames_p[i] = String("");
            }
         }
         else {
            // make this also use FV... to clear flags for a subtable..
            BasePlot bp;
            bp.init(TABS_p[i],i,rootTabNames_p[i], tableSelections_p[i], 
                    dataFlagColName_p, rowFlagColName_p);
            bp.clearFlags();
         }
      }
      rePlot();
   }
   else 
      log->out("Please open a Table first", fnname, clname,
               LogMessage::SEVERE, True);

   log->FnExit(fnname, clname);
   return True;
}

/*********************************************************************************/
/* If multiple sub-selections from the same root table are sent in, 
   then be careful.
   saving repeatedly will overwrite the version. So provide a "merge" 
   option for
   saveFlagVersion as well */

// if merge = replace... then for the first instance of a root Table, 
// replace, otherwise "or".
// If merge = 'and' or 'or' == then just do that.
Bool TablePlot::saveFlagVersion(String versionname, 
       String comment, String merge )
{
   String fnname= "saveFlagVersion";
   log->FnEnter(fnname + "(versionname, comment, merge)", clname);
   
   Bool ret = True;
   if(nTabs_p) {
      ostringstream os;
      os << "Saving flag version " + versionname   
         << " for: " << rootTabNames_p;
      log->out(os, fnname, clname, LogMessage::NORMAL2);
      /* Call saveFlagVersion for all tables in the latest setTableT */
      for(Int i=0;i<(Int)TABS_p.nelements();i++) {
         if(rootTabNames_p[i].length()>0) {
            try {
               FlagVersion 
                 fv(rootTabNames_p[i],dataFlagColName_p,rowFlagColName_p);
               fv.saveFlagVersion( versionname, comment,merge );
            }
            catch (AipsError x) {
               log->out(String("Disabling flag version support for ")+
                    rootTabNames_p[i], fnname, clname, LogMessage::WARN);
               rootTabNames_p[i] = String("");
               ret = False;
            }
         }
      }
   }
   else {
      log->out("Please open a Table first", fnname, clname,
               LogMessage::SEVERE,  True);
      ret = False;
   }
   log->FnExit(fnname, clname);
   return ret;
}
/*********************************************************************************/
Bool TablePlot::restoreFlagVersion(Vector<String> versionname, String merge )
{
   String fnname= "restoreFlagVersion";
   log->FnEnter(fnname + "(versioname, merge)", clname);
   
   Bool replot=False;
   Bool ret = True;
   
   if(nTabs_p) {
      ostringstream os;
      os << "Restoring flag version " << versionname 
         << " for: " << rootTabNames_p;
      log->out(os, fnname, clname, LogMessage::NORMAL2);

      /* Call restoreFlagVersions for all Tables in the most recent setTableT*/
      for(Int i=0;i<(Int)TABS_p.nelements();i++) { 
         if(rootTabNames_p[i].length()>0) {
            try {
               FlagVersion 
                  fv(rootTabNames_p[i],dataFlagColName_p,rowFlagColName_p);
               for(Int j=0;j<(Int)versionname.nelements();j++)
                  replot |= fv.restoreFlagVersion( versionname[j], merge );
            }
            catch (AipsError x) {
               log->out(String("Disabling flag version support for ")+
                   rootTabNames_p[i], fnname, clname, LogMessage::WARN);
               rootTabNames_p[i] = String("");
               ret = False;
            }
         }
      }
   }
   else {
      log->out("Please open a Table first", fnname, clname,
              LogMessage::SEVERE,True );
      ret = False;
   }

   if(replot) rePlot();
   
   log->FnExit(fnname, clname);
   return ret;
}
/*********************************************************************************/
Bool TablePlot::deleteFlagVersion(Vector<String> versionname )
{
   String fnname= "deleteFlagVersion";
   log->FnEnter(fnname + "(versionname)", clname);
   
   Bool ret = True;
   if(nTabs_p) {
       ostringstream os;
       os << "Deleting flag version " <<  versionname 
          << " for: " << rootTabNames_p;
       log->out(os, fnname, clname, LogMessage::NORMAL2);

      /* Call deleteFlagVersions for all Tables in the most recent setTableT*/
      for(Int i=0;i<(Int)TABS_p.nelements();i++) {
         if(rootTabNames_p[i].length()>0) {
            try {
               FlagVersion 
                  fv(rootTabNames_p[i],dataFlagColName_p,rowFlagColName_p);
               for(Int j=0;j<(Int)versionname.nelements();j++)
                  fv.deleteFlagVersion( versionname[j] );
            }
            catch (AipsError x) {
               log->out(String("Disabling flag version support for ")+
                  rootTabNames_p[i], fnname, clname, LogMessage::WARN);
               rootTabNames_p[i] = String("");
               ret = False;
            }
         }
      }
   }
   else {
      log->out(String("Please open a Table first"), 
               fnname, clname, LogMessage::SEVERE);                
      ret = False;
   }

   log->FnExit(fnname, clname);
   return ret;
}
/*********************************************************************************/
Bool TablePlot::getFlagVersionList( Vector<String>& verlist )
{
   String fnname= "getFlagVersionList";
   log->FnEnter(fnname + "(verlist)", clname);

   verlist.resize(0);
   Int num;
   
   Bool ret = True;
   if(nTabs_p) {
      /* Call getVersionList for all tables in the latest setTableT */
      for(Int i=0;i<(Int)TABS_p.nelements();i++) {
         if(rootTabNames_p[i].length()>0) {
            try {
               FlagVersion 
                 fv(rootTabNames_p[i],dataFlagColName_p,rowFlagColName_p);
               Vector<String> vlist = fv.getVersionList();
               
               num = verlist.nelements();
               verlist.resize( num + vlist.nelements() + 1, True );
               verlist[num] = String("\nTable : ") + rootTabNames_p[i] + 
                              String("\n");
               for(Int j=0;j<(Int)vlist.nelements();j++)
                  verlist[num+j+1] = vlist[j];
            }
            catch (AipsError x) {
               log->out(String("Disabling flag version support for ")+
                  rootTabNames_p[i], fnname, clname, LogMessage::WARN);
               rootTabNames_p[i] = String("");
            }
            
         }
      }
   }
   else {
      log->out("Please open a Table first", fnname, clname, LogMessage::SEVERE);
      ret = False;
   }
   
   log->FnExit(fnname, clname);
   return ret;
}

/*********************************************************************************/
Bool TablePlot::checkShapes(Table &intab)
{
   String fnname= "checkShapes";
   log->FnEnter(fnname + "(intab)", clname);
   log->out(String("check shapes for ")+intab.tableName());

   TableDesc tdesc = intab.tableDesc();
   Bool sameshape = True;
   
   if( tdesc.isColumn(dataFlagColName_p) )
   {
      ROArrayColumn<Complex> col;
      col.attach(intab,dataFlagColName_p);

      // Iterate over all rows and check the shape.
      // This maaay take a while.
      IPosition shp = col.shape(0);
      for(Int i=1;i<(Int)intab.nrow();i++)
      {
         if( shp != col.shape(i) )
         {
            sameshape = False;
            break;
         }
      }
   }
   log->FnExit(fnname, clname);
   return sameshape;
}
/*********************************************************************************/
void TablePlot::TablePlotError(String msg)
{
   throw AipsError("TablePlot: " + msg);
}

/*********************************************************************************/
/*********************************************************************************/
/*********************************************************************************/

/*********************************************************************************/
#if 0
/* This is a dummy function to be used to try stuff out without worrying about making
   a new function in the xml file..... Currently has code for disk-read timing tests. */
Int TablePlot::clearAllFlags(Bool rr)
{

/*
   Int nrows = 5000000;
   Matrix<Float> cc; cc.resize(14,nrows);
   tmr.mark();
   for(Int tt=0;tt<5;tt++)
   for(Int r=0;r<nrows;r++)
   for(Int i=0;i<14;i++)
      cc(i,r) = (Float)i*r;
   cout << "Read y TENs in " << tmr.all() << " sec." << endl;
*/   
   String tabName("/home/ballista3/rurvashi/data/IC1262cal.ms");
   
   Table tab(tabName,Table::Update);
   Int nrows = tab.nrow();
   log->out(String("Nrows : ")+String::toString(nrows));

      
   TableExprNode xten = RecordGram::parse(tab,"SQRT(SUMSQUARE(UVW[1:2]))");
   TableExprNode yten = RecordGram::parse(tab,"AMPLITUDE(DATA)");
   
   Array<Double> val;
   Double vval;
   
   Matrix<Double> xx; xx.resize(1,nrows);
   tmr.mark();
   for(Int r=0;r<nrows;r++)
   {
      if(xten.isScalar())
      {
         xten.get(r,vval);
         xx(0,r) = vval;
      }
   }
   log->out(String("Read x TENs in " + tmr.all() + " sec."),
            fnname, clname, LogMessage::NORMAL3);
   
   Matrix<Float> cc; 
   cc.resize(14,nrows);
   tmr.mark();
   for(Int r=0;r<nrows;r++)
   {
      if(!yten.isScalar())
      {
      yten.get(r,val);
      for(Int i=0;i<val.shape().product();i++)
         cc(i,r) = (Float)(val.data()[i]);
      }
   }
   log->out(String("Read y TENs in " + tmr.all() + " sec."),
            fnname, clname, LogMessage::NORMAL3);

   return 0;
}
#endif


/*********************************************************************************/

/*********************************************************************************/
} //# NAMESPACE CASA - END 


