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
#include <calibrater_cmpt.h>

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
     //close
     cb->close();
     cout << "A file missing error is expected here.!" << endl;
     rtn = cb->open( "/bckup/alma/test/ngc5921.ms.wrong");
     if (rtn==false) ; // ignore it
     rtn = (cb->open( "/bckup/alma/test/ngc5921.ms"));
     if (rtn==false) return -1;

// 1. Derive Gain Calibration Solutions.
     cout << "1. DERIVE GAIN CALIBRATION SOLUTIONS" << endl;
     cb->reset();
     //cb.setdata(msselect='FIELD_ID <= 2', mode='channel', start=3, nchan=55)
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
     mode="channel";
     nchan=55;
     start=2;
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
     //cb.setsolve(type='G',t=0,refant=14,table='ngc5921.gcal')
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
     table="/bckup/alma/test/ngc5921.gcal";
     type="G";
     t=0;
     preavg=60.0;
     phaseonly=false;
     refant=14;
     append=false;
     unset=false;
     cb->setsolve(table,type,t,preavg,phaseonly,refant,append,unset);
     cb->state();
     cb->solve();

     //# setsolvebandpoly
     //     cb->reset();
     //cb.setdata(msselect='FIELD_ID <= 2',mode='channel',start=3,nchan=55)
     //cb.setapply(type='TOPAC',t=-1,opacity=0.0001)
     //cb.setapply(type='GAINCURVE',t=-1)
     //cb.setsolvebandpoly("ngc5921.bandpoly",refant=14)

     //# setsolvegainpoly
     //     cb->reset();
     //cb.setdata(msselect='FIELD_ID <= 2',mode='channel',start=3,nchan=55)
     //cb.setapply(type='TOPAC',t=-1,opacity=0.0001)
     //cb.setapply(type='GAINCURVE',t=-1)
     //cb.setsolvegainpoly("ngc5921.gcalpoly",mode="PHASAMP",refant=14)

     //# setsolvegainspline
     //     cb->reset();
     //cb.setdata(msselect='FIELD_ID <= 2',mode='channel',start=3,nchan=55)
     //cb.setapply(type='TOPAC',t=-1,opacity=0.0001)
     //cb.setapply(type='GAINCURVE',t=-1)
     //cb.setsolvegainspline("ngc5921.gcalspline",mode="PHASAMP",refant=14)

// 2. Derive Bandpass Calibration Solutions
     cout << "2. DERIVE BANDPASS CALIBRATION SOLUTIONS" << endl;
     cb->reset();
     //cb.setdata(msselect='FIELD_ID==1')
     mode="none";
     nchan=1;
     start=1;
     step=1;
     mstart=casac::Quantity(std::vector<double>(1,0.0),"km/s");
     mstep=casac::Quantity(std::vector<double>(1,0.0),"km/s");
     uvrange=vector<double>(1,0);
     msselect="FIELD_ID==0";
     cb->setdata(mode,nchan,start,step,mstart,mstep,uvrange,msselect);
     //cb.setapply(type='TOPAC',t=-1,opacity=0.0001)
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
     //cb.setapply(type='G',t=0.0,table='ngc5921.gcal')
     type="G";
     t=0.0;
     table="/bckup/alma/test/ngc5921.gcal";
     interp="nearest";
     select=" ";
     spwmap=vector<int>(1,-1);
     unset=false;
     opacity=0.0;
     rawspw=vector<int>(1,-1);
     cb->setapply(type,t,table,interp,select,spwmap,unset,opacity,rawspw);
     //cb.setsolve(type='B',t=86400.0,refant=14,table='ngc5921.bcal')
     table="/bckup/alma/test/ngc5921.bcal";
     type="B";
     t=86400.0;
     preavg=60.0;
     phaseonly=false;
     refant=14;
     append=false;
     unset=false;
     cb->setsolve(table,type,t,preavg,phaseonly,refant,append,unset);
     cb->state();
     cb->solve();

    
// 3. Transfer the flux density scale
     cout << "3. TRANSFER THE FLUX DENSITY SCALE" << endl;
     //cb.fluxscale(tablein='ngc5921.gcal',tableout='ngc5921.fluxcal',reference=['1331+30500002'],transfer=['1445+09900002'])
     // fluxscale defaults
     /*
     std::vector<double> fluxd;
     std::string tablein;
     std::vector<std::string> reference;
     std::string tableout;
     std::vector<std::string> transfer;
     bool append;
     std::vector<int> refspwmap;
     --
     tableout = "";
     transfer = std::vector<std::string>(1,"-1");
     bool append = false;
     refspwmap = std::vector<int>(1,-1);
*/
     std::vector<double> fluxd;
     std::string tablein("/bckup/alma/test/ngc5921.gcal");
     std::vector<std::string> reference(1,"1331+30500002");
     std::string tableout("/bckup/alma/test/ngc5921.fluxcal");
     std::vector<std::string> transfer(1,"1445+09900002");
     append=false;
     std::vector<int> refspwmap(1,-1);
     cb->fluxscale(fluxd, tablein, reference, tableout, transfer,
		   append, refspwmap);
     for (uInt j=0; j< fluxd.size(); ++j) cout << "fluxd[" << j << "]=" << fluxd[j] << endl;

// 4. Correct the target source data
     cout << "4. CORRECT THE TARGET SOURCE DATA" << endl;
     cb->reset();
     //cb.setdata(msselect='FIELD_ID IN [2,3]')
     mode="none";
     nchan=1;
     start=1;
     step=1;
     mstart=casac::Quantity(std::vector<double>(1,0.0),"km/s");
     mstep=casac::Quantity(std::vector<double>(1,0.0),"km/s");
     uvrange=vector<double>(1,0);
     msselect = "FIELD_ID IN [1,2]";
     cb->setdata(mode,nchan,start,step,mstart,mstep,uvrange,msselect);
     //cb.setapply(type='TOPAC',t=-1,opacity=0.0001)
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
     t=-1.0;
     table=" ";
     interp="nearest";
     select=" ";
     spwmap=std::vector<int>(1,-1);
     unset=false;
     opacity=0.0;
     rawspw=std::vector<int>(1,-1);
     cb->setapply(type,t,table,interp,select,spwmap,unset,opacity,rawspw);
     //cb.setapply(type='G',t=0.0,table='ngc5921.fluxcal',select='FIELD_ID==2')
     type="G";
     t=0.0;
     table="/bckup/alma/test/ngc5921.fluxcal";
     interp="nearest";
     select="FIELD_ID=1";
     spwmap=std::vector<int>(1,-1);
     unset=false;
     opacity=0.0;
     rawspw=std::vector<int>(1,-1);
     cb->setapply(type,t,table,interp,select,spwmap,unset,opacity,rawspw);
     //cb.setapply(type='B',t=0.0,table='ngc5921.bcal',select='')
     type="B";
     t=0.0;
     table="/bckup/alma/test/ngc5921.bcal";
     interp="nearest";
     select="";
     spwmap=vector<int>(1,-1);
     unset=false;
     opacity=0.0;
     rawspw=vector<int>(1,-1);
     cb->setapply(type,t,table,interp,select,spwmap,unset,opacity,rawspw);
     cb->state();
     cb->correct();

     cout << "--Calling initcalset" << endl;
     cb->initcalset();

     //done
     cout << "--Calling done.  It returned " << cb->done() << endl;
     cout << "--Calling open.  It returned " << cb->open( "/bckup/alma/test/ngc5921.ms") << endl;
     cb->close();
     cb->done();

  } catch( AipsError e ){
    cout << "Exception ocurred." << endl;
    cout << e.getMesg() << endl;
  }
};
