//# tcalibrater_cmpt.cc:  This tests calibrater_cmpt.
//# Copyright (C) 1996,1997,1999,2001
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

#include <casa/iostream.h>
#include <casa/aips.h>
#include <casa/Exceptions/Error.h>
#include <casa/Containers/Block.h>
#include <casa/Logging/LogIO.h>
#include <synthesis/MeasurementComponents/Calibrater.h>
#include <xmlcasa/synthesis/calibrater_cmpt.h>

// This test program uses a MeasurementSet which can be created from
// the fits dataset in '/aips++/data/nrao/VLA/ngc5921.fits'.  For
// instance, using glish:
   /*
   include 'vlafiller.g';
   include 'ms.g';
   include 'os.g';
   include 'imager.g';
   mset:=fitstoms(msfile='ngc5921.ms',
                  fitsfile='/aips++/data/nrao/VLA/ngc5921.fits');
   ms.done();
   imgr:=imager(filename='test/ngc5921.ms');
   imgr.setjy(fieldid=1);       # calculate and set flux density for 3C286
   imgr.setdata(fieldid=1,spwid=1);
   imgr.done();            
   */

int main()
{
  using namespace std;
  using namespace casa;
  using namespace casac;

  try{
     calibrater* cb = new calibrater( );
     bool rtn(false);
     //open
     rtn = (cb->open( "/bckup/alma/test/ngc5921.ms"));
     if (rtn==false) return -1;

// 1. Test accumulate
     cout << "1. TESTING ACCUMULATE" << endl;
     cb->reset();
     //cb.setdata(msselect='FIELD_ID <= 2');
     /* setdata defaults
     std::string mode;
     int nchan;
     int start;
     int step;
     casac::Quantity mstart;
     casac::Quantity mstep;
     std::vector<double> uvrange;
     std::string msselect;
     ---
     mode="none";
     nchan=1;
     start=1;
     step=1;
     mstart= casac::Quantity(std::vector<double>(1,0.0),"km/s");
     mstep= casac::Quantity(std::vector<double>(1,0.0),"km/s");
     uvrange=vector<double>(1,0);
     msselect=" ";
     */
     std::string mode;
     int nchan;
     int start;
     int step;
     casac::Quantity mstart;
     casac::Quantity mstep;
     std::vector<double> uvrange;
     std::string msselect;
     mode="none";
     nchan=1;
     start=1;
     step=1;
     mstart=casac::Quantity(std::vector<double>(1,0.0),"km/s");
     mstep=casac::Quantity(std::vector<double>(1,0.0),"km/s");
     uvrange=vector<double>(1,0);
     msselect="FIELD_ID <= 1";
     cb->setdata(mode,nchan,start,step,mstart,mstep,uvrange,msselect);
     //cb.setapply(type='TOPAC',t=-1,opacity=0.0001)
     /* setapply defaults
     std::string type;
     double t;
     std::string table;
     std::string interp;
     std::string select;
     std::vector<int> spwmap;
     bool unset;
     double opacity;
     std::vector<int> rawspw;
     ---
     type="GAINCURVE";
     t=0.0;
     table=" ";
     interp="nearest";
     select=" ";
     spwmap=vector<int>(1,-1);
     unset=false;
     opacity=0.0;
     rawspw=vector<int>(1,-1);
     */
     std::string type;
     double t;
     std::string table;
     std::string interp;
     std::string select;
     std::vector<int> spwmap;
     bool unset;
     double opacity;
     std::vector<int> rawspw;
     type="TOPAC";
     t=-1;
     table=" ";
     interp="nearest";
     select=" ";
     spwmap=vector<int>(1,-1);
     unset=false;
     opacity=0.0001;
     rawspw=vector<int>(1,-1);
     cb->setapply(type,t,table,interp,select,spwmap,unset,opacity,rawspw);
     //cb.setapply(type='GAINCURVE',t=-1)
     type="GAINCURVE";
     t=-1;
     table=" ";
     interp="nearest";
     select=" ";
     spwmap=vector<int>(1,-1);
     unset=false;
     opacity=0.0;
     rawspw=vector<int>(1,-1);
     cb->setapply(type,t,table,interp,select,spwmap,unset,opacity,rawspw);
     //cb.setsolve(type='G',table='cal.gcal1',t=15*60)
     /* setsolve defaults
     std::string table;
     std::string type;
     double t;
     double preavg;
     bool phaseonly;
     int refant;
     bool append;
     bool unset;
     --
     type="MF";
     t=60.0;
     preavg=60.0;
     phaseonly=false;
     refant=-1;
     append=false;
     unset=false;
     */
     double preavg;
     bool phaseonly;
     int refant;
     bool append;
     table="/bckup/alma/test/cal.gcal1";
     type="G";
     t=15.0*60.0;
     preavg=60.0;
     phaseonly=false;
     refant=-1;
     append=false;
     unset=false;
     cb->setsolve(table,type,t,preavg,phaseonly,refant,append,unset);
     cb->solve();
     //cb.accumulate(tablein='',incrtable='test/cal.gcal1',tableout='test/cal.cG1',interp='linear',t=120)
     /* accumulate defaults
     std::string tablein;
     std::string incrtable;
     std::string tableout;
     std::vector<std::string> field;
     std::vector<std::string> calfield;
     std::string interp;
     double t;
     --
     tablein = "";
     incrtable = "";
     tableout = "";
     field = std::vector<std::string> (1,"-1");
     calfield = std::vector<std::string> (1,"-1");
     interp = "linear";
     t = -1.0;
     */
     std::string tablein;
     std::string incrtable;
     std::string tableout;
     std::vector<std::string> field;
     std::vector<std::string> calfield;
     tablein = "";
     incrtable = "/bckup/alma/test/cal.gcal1";
     tableout = "/bckup/alma/test/cal.cG1";
     field = std::vector<std::string> (1,"-1");
     calfield = std::vector<std::string> (1,"-1");
     interp = "linear";
     t = 120.0;
     cb->accumulate(tablein, incrtable, tableout, field, calfield, interp, t);

     //done
     cout << "--Calling done.  It returned " << cb->done() << endl;
     if(cb==NULL)
       cout << "The calibrater component is now null." << endl;
     else
       cout << "The calibrater component is not null yet." << endl;

  } catch( AipsError e ){
    cout << "Exception ocurred." << endl;
    cout << e.getMesg() << endl;
  }
};
