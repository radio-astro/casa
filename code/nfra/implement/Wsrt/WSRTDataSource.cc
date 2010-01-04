//# WSRTDataSource.cc : class for access to WSRT datasets
//# Copyright (C) 1996,1997,1998,1999,2000,2002
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This program is free software; you can redistribute it and/or modify it
//# under the terms of the GNU General Public License as published by the Free
//# Software Foundation; either version 2 of the License, or (at your option)
//# any later version.
//#
//# This program is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
//# more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with this program; if not, write to the Free Software Foundation, Inc.,
//# 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id$


#include <nfra/Wsrt/WSRTDataSource.h>
#include <casa/OS/IBMDataConversion.h>
#include <casa/OS/VAXDataConversion.h>
#include <casa/OS/CanonicalDataConversion.h>
#include <scimath/Mathematics.h>
#include <casa/Utilities/Assert.h>
#include <casa/Exceptions/Error.h>
#include <casa/iostream.h>
#include <casa/stdio.h>                  // needed for sprintf

	
WSRTDataSource::WSRTDataSource(const String& dsName, Bool noscale)
: DataSource(dsName),
  itsFile(dsName),
  itsDataConversion(0),
  itsNoScale(noscale),
  itsFDBuffer(0),itsOHBuffer(0),itsSCBuffer(0),
  itsSHBuffer(0),itsIHBuffer(0),itsDBBuffer(0),
  itsNextContinuumRow(0),
  itsFirstDataBlock(True),
  itsBECode(' '),
  itsNumAnt(14),itsNumBand(0),itsNumChan(0),itsNumCorr(0),itsNumBase(0),
  itsLRCRD(128),  //# record length in nr of bytes
  itsPHBLL(64),   //# physical block length in nr of records
  itsNBL(0),      //# total nr of physical blocks in the dataset
  itsNrOfOH(0),itsNrOfSC(0),
  itsNrOfSH(0),itsNrOfIH(0),itsNrOfDB(0),
  itsFDlen(15),itsOHlen(0),itsSClen(0),
  itsSHlen(0),itsIHlen(0),itsDBlen(0),
  itsOHoff(-1),itsSCoff(-1),
  itsSHoff(-1),itsIHoff(-1),itsDBoff(-1)
{
    cout << className() << ": instantiated" << endl;

    cout << className() << " open file " << dsName << endl;

    cout << className() << " read the FD block " << endl;
    readFD();

    cout << className() << " read the OH block " << endl;
    readOH();

    cout << className() << " read the SC block " << endl;
    readSC();

    cout << className() << " read first SH block " << endl;
    readSH();

    cout << className() << " read first IH block " << endl;
    readIH();

    cout << className() << " read first DB block " << endl;
    itsDataConversion->toLocal (itsDBlen, itsIHBuffer+50);
    itsDBlen = 4*itsDBlen;
    readDB();

    itsContinuumRow.resize(itsNumAnt,itsNumAnt);
    itsContinuumRow = -1;
}

WSRTDataSource::~WSRTDataSource() 
{
    cout << className() << ": destructor started" << endl;
    delete itsDataConversion;
    delete[] itsFDBuffer;
    delete[] itsOHBuffer;
    delete[] itsSCBuffer;
    delete[] itsSHBuffer;
    delete[] itsIHBuffer;
    delete[] itsDBBuffer;
    cout << className() << ": destructor ended" << endl;
}

void WSRTDataSource::show()
{
    cout << endl << "From FD block: " << endl;
    cout <<         "-------------- " << endl;
    cout << " Record length = " << itsLRCRD << " bytes" << endl;
    cout << " Block size    = " << itsPHBLL << " records" << endl;
    cout << " Nr of blocks  = " << itsNBL   << endl;
    
    cout << " blockType, nrRecords, firstRecord, nrGroups: " << endl;
    cout << " FD, " << itsFDlen << ", " << "0"
	 << ", " << "1" << endl;
    cout << " OH, " << itsOHlen << ", " << itsOHoff
	 << ", " << "1" << endl;
    cout << " SC, " << itsSClen << ", " << itsSCoff
	 << ", " << itsNrOfSC << endl;
    cout << " SH, " << itsSHlen << ", " << itsSHoff
	 << ", " << itsNrOfSH << endl;
    cout << " IH, " << itsIHlen << ", " << itsIHoff
	 << ", " << itsNrOfIH << endl;
    cout << " DB, " << itsDBlen << ", " << itsDBoff
	 << ", " << itsNrOfDB << endl;
    cout << endl;


    cout << "itsOHoff, itsSCoff, itsSHoff, itsIHoff, itsDBoff :"
	 << endl;
    cout << itsOHoff << ", " << itsSCoff << ", " << itsSHoff
	 << ", " << itsIHoff << ", " << itsDBoff << endl;

    cout << endl << "From OH block: " << endl;
    cout <<         "-------------- " << endl;
    short cbi;
    itsDataConversion->toLocal (cbi,itsOHBuffer);
    cout << "cbi : " << cbi << endl;
    char cbt[2];
    itsDataConversion->toLocal (cbt,itsOHBuffer+2,2);
    cout << "cbt : " << String(cbt,2) << endl;
    int noh;
    itsDataConversion->toLocal (noh,itsOHBuffer+4);
    cout << "noh : " << noh << endl;
    int loh;
    itsDataConversion->toLocal (loh,itsOHBuffer+8);
    cout << "loh : " << loh << endl;
    int nohn;
    itsDataConversion->toLocal (nohn,itsOHBuffer+12);
    cout << "nohn : " << nohn << endl;
    short sday;
    itsDataConversion->toLocal (sday,itsOHBuffer+16);
    cout << "sday : " << sday << endl;
    short stim;
    itsDataConversion->toLocal (stim,itsOHBuffer+18);
    cout << "stim : " << stim << endl;
    short etim;
    itsDataConversion->toLocal (etim,itsOHBuffer+20);
    cout << "etim : " << etim << endl;
//    short ohw__0000;
    short nsc;
    itsDataConversion->toLocal (nsc,itsOHBuffer+24);
    cout << "nsc : " << nsc << endl;
    short project;
    itsDataConversion->toLocal (project,itsOHBuffer+26);
    cout << "project : " << project << endl;
    char field[12];
    itsDataConversion->toLocal (field,itsOHBuffer+28,12);
    cout << "field : " << String(field,12) << endl;
    int volgnr;
    itsDataConversion->toLocal (volgnr,itsOHBuffer+40);
    cout << "volgnr : " << volgnr << endl;
    short stuurc;
    itsDataConversion->toLocal (stuurc,itsOHBuffer+44);
    cout << "stuurc : " << stuurc << endl;
    char type[2];
    itsDataConversion->toLocal (type,itsOHBuffer+46,2);
    cout << "type : " << String(type,2) << endl;
    short date[6];
    itsDataConversion->toLocal (date,itsOHBuffer+48,6);
    cout << "date : " << date[0] << " " << date[1] << " " << date[2]
	<< " " << date[3] << " " << date[4] << " " << date[5] << endl;
    short prflg;
    itsDataConversion->toLocal (prflg,itsOHBuffer+60);
    cout << "prflg : " << prflg << endl;
    short olsys;
    itsDataConversion->toLocal (olsys,itsOHBuffer+62);
    cout << "olsys : " << olsys << endl;
    double jday;
    itsDataConversion->toLocal (jday,itsOHBuffer+64);
    cout << "jday : " << jday << endl;
    double becen;
    itsDataConversion->toLocal (becen,itsOHBuffer+72);
    cout << "becen : " << becen << endl;
    double jucen;
    itsDataConversion->toLocal (jucen,itsOHBuffer+80);
    cout << "jucen : " << jucen << endl;
    short freqc;
    itsDataConversion->toLocal (freqc,itsOHBuffer+88);
    cout << "freqc : " << freqc << endl;
    char categ[2];
    itsDataConversion->toLocal (categ,itsOHBuffer+90,2);
    cout << "categ : " << String(categ,2) << endl;
    short alloc;
    itsDataConversion->toLocal (alloc,itsOHBuffer+92);
    cout << "alloc : " << alloc << endl;
    char request[4];
    itsDataConversion->toLocal (request,itsOHBuffer+94,4);
    cout << "request : " << String(request,4) << endl;
//    int statbg[2];
//    int statbe[2];
//    int statse[2];
    short poper;
    itsDataConversion->toLocal (poper,itsOHBuffer+122);
    cout << "poper : " << poper << endl;
    char dxbit[2];
    itsDataConversion->toLocal (dxbit,itsOHBuffer+124,2);
    cout << "dxbit : " << String(dxbit,2) << endl;
    char apcs[2];
    itsDataConversion->toLocal (apcs,itsOHBuffer+126,2);
    cout << "apcs : " << String(apcs,2) << endl;
    double ra1;
    itsDataConversion->toLocal (ra1,itsOHBuffer+128);
    cout << "ra1 : " << ra1 << endl;
    double dec1;
    itsDataConversion->toLocal (dec1,itsOHBuffer+136);
    cout << "dec1 : " << dec1 << endl;
    short mode;
    itsDataConversion->toLocal (mode,itsOHBuffer+144);
    cout << "mode : " << mode << endl;
    short polc;
    itsDataConversion->toLocal (polc,itsOHBuffer+146);
    cout << "polc : " << polc << endl;
    float band;
    itsDataConversion->toLocal (band,itsOHBuffer+148);
    cout << "band : " << band << endl;
    short ntot;
    itsDataConversion->toLocal (ntot,itsOHBuffer+152);
    cout << "ntot : " << ntot << endl;
    short nfreq;
    itsDataConversion->toLocal (nfreq,itsOHBuffer+154);
    cout << "nfreq : " << nfreq << endl;
    short sfreq;
    itsDataConversion->toLocal (sfreq,itsOHBuffer+156);
    cout << "sfreq : " << sfreq << endl;
    short nrpol;
    itsDataConversion->toLocal (nrpol,itsOHBuffer+158);
    cout << "nrpol : " << nrpol << endl;
    short nrint;
    itsDataConversion->toLocal (nrint,itsOHBuffer+160);
    cout << "nrint : " << nrint << endl;
    short telwd[2];
    itsDataConversion->toLocal (telwd,itsOHBuffer+162,2);
    cout << "telwd : " << telwd[0] << " " << telwd[1] << endl;
    short bsint;
    itsDataConversion->toLocal (bsint,itsOHBuffer+166);
    cout << "bsint : " << bsint << endl;
    int confnr;
    itsDataConversion->toLocal (confnr,itsOHBuffer+168);
    cout << "confnr : " << confnr << endl;
    char becode[4];
    itsDataConversion->toLocal (becode,itsOHBuffer+172,4);
    cout << "becode : " << String(becode,4) << endl;
    double ra0;
    itsDataConversion->toLocal (ra0,itsOHBuffer+176);
    cout << "ra0 : " << ra0 << endl;
    double dec0;
    itsDataConversion->toLocal (dec0,itsOHBuffer+184);
    cout << "dec0 : " << dec0 << endl;
    double freq;
    itsDataConversion->toLocal (freq,itsOHBuffer+192);
    cout << "freq : " << freq << endl;
    double hast;
    itsDataConversion->toLocal (hast,itsOHBuffer+200);
    cout << "hast : " << hast << endl;
    double haend;
    itsDataConversion->toLocal (haend,itsOHBuffer+208);
    cout << "haend : " << haend << endl;
    double lst;
    itsDataConversion->toLocal (lst,itsOHBuffer+216);
    cout << "lst : " << lst << endl;
    float paralax;
    itsDataConversion->toLocal (paralax,itsOHBuffer+224);
    cout << "paralax : " << paralax << endl;
    float rra1;
    itsDataConversion->toLocal (rra1,itsOHBuffer+228);
    cout << "rra1 : " << rra1 << endl;
    float rdec1;
    itsDataConversion->toLocal (rdec1,itsOHBuffer+232);
    cout << "rdec1 : " << rdec1 << endl;
    float rra2;
    itsDataConversion->toLocal (rra2,itsOHBuffer+236);
    cout << "rra2 : " << rra2 << endl;
    float rdec2;
    itsDataConversion->toLocal (rdec2,itsOHBuffer+240);
    cout << "rdec2 : " << rdec2 << endl;
    float vlcty;
    itsDataConversion->toLocal (vlcty,itsOHBuffer+244);
    cout << "vlcty : " << vlcty << endl;
    short velc;
    itsDataConversion->toLocal (velc,itsOHBuffer+248);
    cout << "velc : " << velc << endl;
    short inx;
    itsDataConversion->toLocal (inx,itsOHBuffer+250);
    cout << "inx : " << inx << endl;
    float dra;
    itsDataConversion->toLocal (dra,itsOHBuffer+252);
    cout << "dra : " << dra << endl;
    float ddec;
    itsDataConversion->toLocal (ddec,itsOHBuffer+256);
    cout << "ddec : " << ddec << endl;
    short npc;
    itsDataConversion->toLocal (npc,itsOHBuffer+260);
    cout << "npc : " << npc << endl;
    short nps1;
    itsDataConversion->toLocal (nps1,itsOHBuffer+262);
    cout << "nps1 : " << nps1 << endl;
    float fdra1;
    itsDataConversion->toLocal (fdra1,itsOHBuffer+264);
    cout << "fdra1 : " << fdra1 << endl;
    float fdec1;
    itsDataConversion->toLocal (fdec1,itsOHBuffer+268);
    cout << "fdec1 : " << fdec1 << endl;
    short nps2;
    itsDataConversion->toLocal (nps2,itsOHBuffer+272);
    cout << "nps2 : " << nps2 << endl;
//    char ohw__0001[2];
    float fdra2;
    itsDataConversion->toLocal (fdra2,itsOHBuffer+276);
    cout << "fdra2 : " << fdra2 << endl;
    float fdec2;
    itsDataConversion->toLocal (fdec2,itsOHBuffer+280);
    cout << "fdec2 : " << fdec2 << endl;
    short nps3;
    itsDataConversion->toLocal (nps3,itsOHBuffer+284);
    cout << "nps3 : " << nps3 << endl;
//    char ohw__0002[2];
    float fdra3;
    itsDataConversion->toLocal (fdra3,itsOHBuffer+288);
    cout << "fdra3 : " << fdra3 << endl;
    float fdec3;
    itsDataConversion->toLocal (fdec3,itsOHBuffer+292);
    cout << "fdec3 : " << fdec3 << endl;
//    char ohw__0003[2];
    char spefu[2];
    itsDataConversion->toLocal (spefu,itsOHBuffer+298,2);
    cout << "spefu : " << String(spefu,2) << endl;
    int post[14];
    itsDataConversion->toLocal (post,itsOHBuffer+300,14);
    cout << "post : " << post[0] << " " << post[1] << " "
	<< post[2] << " " << post[3] << " " << post[4] << " "
	    << post[5] << " " << post[6] << " " << post[7] << " " 
		<< post[8] << " " << post[8] << " " << post[10] << " "
		    << post[11] << " " << post[12] << " " << post[13] << endl;
    short taper;
    itsDataConversion->toLocal (taper,itsOHBuffer+356);
    cout << "taper : " << taper << endl;
    char devc0[2]; 
    itsDataConversion->toLocal (devc0,itsOHBuffer+358,2);
    cout << "devc0 : " << String(devc0,2) << endl;
    double freq0;
    itsDataConversion->toLocal (freq0,itsOHBuffer+360);
    cout << "freq0 : " << freq0 << endl;
    short stopar;
    itsDataConversion->toLocal (stopar,itsOHBuffer+368);
    cout << "stopar : " << stopar << endl;
    short mspat;
    itsDataConversion->toLocal (mspat,itsOHBuffer+370);
    cout << "mspat : " << mspat << endl;
    short mposn;
    itsDataConversion->toLocal (mposn,itsOHBuffer+372);
    cout << "mposn : " << mposn << endl;
    short msnp;
    itsDataConversion->toLocal (msnp,itsOHBuffer+374);
    cout << "msnp : " << msnp << endl;
//    char ohw__0004[290];
    short nrsts;
    itsDataConversion->toLocal (nrsts,itsOHBuffer+666);
    cout << "nrsts : " << nrsts << endl;
    short nrfrq;
    itsDataConversion->toLocal (nrfrq,itsOHBuffer+668);
    cout << "nrfrq : " << nrfrq << endl;
    short lent;
    itsDataConversion->toLocal (lent,itsOHBuffer+670);
    cout << "lent : " <<lent  << endl;

    int bfreq;
    char datyp[2];
    short bandnr;
    int nsh;
    uInt off = 672;
    cout << " set, offset, bandnr, recnr, datatype, frequency: " << endl;
    for (Int set=0; set<nrsts; set++) {
	itsDataConversion->toLocal (bfreq,itsOHBuffer+off);
	itsDataConversion->toLocal (datyp,itsOHBuffer+off+4,2);
	itsDataConversion->toLocal (bandnr,itsOHBuffer+off+6);
	itsDataConversion->toLocal (nsh,itsOHBuffer+off+8);
	cout << "-----set " <<set << ", " << off << ",  " << bandnr
	    << ",  " << nsh << ",  " << String(datyp,2)
		<< ", " << bfreq << endl;
	off += 12;
    }
}

// Get the project identification string
String WSRTDataSource::project() const
{
    Short projectNr;
    Int observationNr;
    // Get project nr and observation nr
    itsDataConversion->toLocal (projectNr,itsOHBuffer+26);
    itsDataConversion->toLocal (observationNr,itsOHBuffer+40);
    // Convert to string
    char str[32];
    sprintf (str, "%i_%i", projectNr, observationNr);
    String projectID(str,32);
    return projectID;
}

// Get the field name
String WSRTDataSource::fieldName() const
{
    char str[12];
    itsDataConversion->toLocal (str,itsOHBuffer+28,12);
    // Convert to string
    String name(str,12);
    return name;
}

// Get the special observation type code
String WSRTDataSource::fieldCode() const
{
    char str[2];
    itsDataConversion->toLocal (str,itsOHBuffer+298,2);
    // Convert to string
    String name(str,2);
    return name;
}

// Get the delay direction in radians
Vector<Double> WSRTDataSource::fieldDelayDir() const
{
    Double ra1, dec1;
    Vector<Double> direction(2);
    // get direction in fractions of a circle
    itsDataConversion->toLocal (ra1,itsOHBuffer+128);
    itsDataConversion->toLocal (dec1,itsOHBuffer+136);
    // convert to radians
    direction(0) = C::_2pi * ra1;
    direction(1) = C::_2pi * dec1;
    return direction;
}

// Get the delay direction rate in radians per second
Vector<Double> WSRTDataSource::fieldDelayDirRate() const
{
    Vector<Double> rate(2);
    Float rra1, rdec1;
    // get direction rate in fractions of a circle per day
    itsDataConversion->toLocal (rra1,itsOHBuffer+228);
    itsDataConversion->toLocal (rdec1,itsOHBuffer+232);
    // convert to radians per second
    rate(0) = C::_2pi * rra1 /86400.;
    rate(1) = C::_2pi * rdec1 /86400.;
    return rate;
}

// Get the phase direction in radians
Vector<Double> WSRTDataSource::fieldPhaseDir() const
{
    Vector<Double> direction(2);
    // just copy from DelayDir
    direction = fieldDelayDir();
    return direction;
}

// Get the phase direction rate in radians per second
Vector<Double> WSRTDataSource::fieldPhaseDirRate() const
{
    Vector<Double> rate(2);
    // just copy from DelayDirRate
    rate = fieldDelayDirRate();
    return rate;
}

// Get the pointing direction in radians
Vector<Double> WSRTDataSource::fieldPointDir() const
{
    Double ra1, dec1;
    Float dra, ddec;
    Vector<Double> direction(2);
    // get directions in fractions of a circle
    itsDataConversion->toLocal (ra1,itsOHBuffer+128);
    itsDataConversion->toLocal (dec1,itsOHBuffer+136);
    itsDataConversion->toLocal (dra,itsOHBuffer+252);
    itsDataConversion->toLocal (ddec,itsOHBuffer+256);
    // convert to radians
    direction(0) = C::_2pi * (ra1 + dra);
    direction(1) = C::_2pi * (dec1 + ddec);
    return direction;
}

// Get the pointing direction rate in radians per second
Vector<Double> WSRTDataSource::fieldPointDirRate() const
{
    Vector<Double> rate(2);
    Float rra1, rdec1;
    // get direction rate in fractions of a circle per day
    itsDataConversion->toLocal (rra1,itsOHBuffer+228);
    itsDataConversion->toLocal (rdec1,itsOHBuffer+232);
    // convert to radians per second
    rate(0) = C::_2pi * rra1 /86400.;
    rate(1) = C::_2pi * rdec1 /86400.;
    return rate;
}

// Get the antenna names
Vector<String> WSRTDataSource::antName() const
{
    Vector<String> name(itsNumAnt);
    name(0) = "WSRT0";
    name(1) = "WSRT1";
    name(2) = "WSRT2";
    name(3) = "WSRT3";
    name(4) = "WSRT4";
    name(5) = "WSRT5";
    name(6) = "WSRT6";
    name(7) = "WSRT7";
    name(8) = "WSRT8";
    name(9) = "WSRT9";
    name(10) = "WSRTA";
    name(11) = "WSRTB";
    name(12) = "WSRTC";
    name(13) = "WSRTD";
    return name;
}

// Get linear offsets of antennas from reference position
Matrix<Double> WSRTDataSource::antPosition() const
{
    Matrix<Double> pos(itsNumAnt,3);
    pos = 0.0;
    Double factor = pow(2,16);
    // telescope positions in 2**(-16) m
    Int ipos;
    for (Int i=0; i<itsNumAnt; i++) {
	itsDataConversion->toLocal (ipos,itsOHBuffer+300+i*4);
	pos(i,0) = ipos/factor;
    }
    // X = ipos*sin(longitude)
    // Y = ipos*cos(longitude)
    // Z = 0
    return pos;
}

// Get the centre time and the time interval in seconds
Vector<Double> WSRTDataSource::timeInterval() const
{
    //time of the middle of the observation in julian days
    Double OHjday;
    itsDataConversion->toLocal (OHjday,itsOHBuffer+64);
    OHjday = OHjday+40000-0.5;
    //start and end U.T. time in units of 10 seconds
    Short OHstim, OHetim;
    itsDataConversion->toLocal (OHstim,itsOHBuffer+18);
    itsDataConversion->toLocal (OHetim,itsOHBuffer+20);
    Vector<Double> time(2);
    time(0) = OHjday*86400.;  //in JD sec
    if (OHetim > OHstim) {
	time(1) = 10.0*(OHetim-OHstim);
    }else{
	time(1) = 10.0*(OHetim+8640-OHstim);
    }
    return time;
}

Double WSRTDataSource::refFrequency() const
{
    //DCB: primary fringe stopping frequency in MHz
    Double refFreq;
    itsDataConversion->toLocal (refFreq,itsOHBuffer+192);
    //return frequency in Hz
    return refFreq*1.0e6;
}

Vector<Double> WSRTDataSource::freqResolution() const
{
    Short OHsfreq;
    //resolution in MHz
    Vector<Double> resolution(itsNumBand*itsNumChan);
//    cout << "WSRTDataSource::freqResolution() - D"
//	 << itsBECode << "B = " << endl;

    itsDataConversion->toLocal (OHsfreq,itsOHBuffer+156);
    if (itsBECode == 'C') {
	// DCB: decipher the bitcode
	for (Int i=0; i<itsNumBand; i++) {
	    if (OHsfreq & (1<<i)) {
//		cout << " band " << i << " used: ";
		//test band used
		if (OHsfreq & (1<<(i+8))) {
		    //test bandwidth
		    resolution(i) = 5.0e6;
		}else{
		    resolution(i) = 10.0e6;
		}
	    }else{
//		cout << "band " << i << " seems not in use???: ";
		//assume
		resolution(i) = 10.0e6;
	    }
//	    cout << " resolution(" << i << ") = " << resolution(i) << endl;
	}
    }else{
	// DLB and DXB: spacing of the frequency points is given
        // OHsfreq is in 0.1 khz
        resolution = OHsfreq*100.;
    }
    return resolution;
}

Vector<Double> WSRTDataSource::chanFreq() const
{
    Vector<Double> freqSelected(itsNumBand*itsNumChan);

    // Loop through the set table in the OH block
    // and assemble the frequencies for the different frequency bands
    uShort OHnrfrq;
    itsDataConversion->toLocal (OHnrfrq,itsOHBuffer+668);
    Vector<Double> freq(OHnrfrq);
    Vector<Bool>   done(OHnrfrq);
    done = False;

    uShort OHnrsts, OHbandnr;
    Int OHbfreq;
    //to convert from 2**-16 MHz to Hz
    Double factor = 1.0e6/pow(2,16);
    itsDataConversion->toLocal (OHnrsts,itsOHBuffer+666);

    for (uInt setnr=0; setnr<OHnrsts; setnr++) {
	itsDataConversion->toLocal (OHbandnr,itsOHBuffer+678+12*setnr);
	if (!done(OHbandnr)) {
	    itsDataConversion->toLocal (OHbfreq,itsOHBuffer+672+12*setnr);
	    freq(OHbandnr) = Double(OHbfreq)*factor;
	    done(OHbandnr) = True;
	}
    }

    // Copy channel frequencies in output vector
    for (Int bandnr=0; bandnr<itsNumBand*itsNumChan; bandnr++) {
	freqSelected(bandnr) = freq(bandnr+OHnrfrq-itsNumBand*itsNumChan);
    }
    return freqSelected;
}

Double WSRTDataSource::restFrequency() const
{
    //For line observations: rest frequency in MHz
    //For DCB: observing frequency in MHz
    Double OHfreq0;
    itsDataConversion->toLocal (OHfreq0,itsOHBuffer+360);
    return OHfreq0*1.0e6;
}

Double WSRTDataSource::freqBandwidth() const
{
    //DLB: total bandwidth in MHz
    //DCB: total bandwidth in MHz (sum of all bands)
    Float OHband;
    itsDataConversion->toLocal (OHband,itsOHBuffer+148);
    return Double(OHband)*1.0e6;
}

Vector<Double> WSRTDataSource::freqBandwidth(Int nrbands) const
{
    Vector<Double> bandwidth(nrbands);
    if (itsBECode == 'C') {
	bandwidth = freqResolution();
    }else if (itsBECode == 'L' || itsBECode == 'X') {
	bandwidth = freqBandwidth();
    }
    return bandwidth;
}

Double WSRTDataSource::contChanFreq() const
{
    Double freq = 0;
    uShort OHnrsts, OHbandnr;
    Int OHbfreq;
    //to convert from 2**-16 MHz to Hz
    Double factor = 1.0e6/pow(2,16);

    itsDataConversion->toLocal (OHnrsts,itsOHBuffer+666);

    // Loop through the set table in the OH block
    // and get the frequency for WSRT band 0 (the continuum channel)
    for (uInt setnr=0; setnr<OHnrsts; setnr++) {
	itsDataConversion->toLocal (OHbandnr,itsOHBuffer+678+12*setnr);
	if (OHbandnr == 0) {
	    itsDataConversion->toLocal (OHbfreq,itsOHBuffer+672+12*setnr);
	    freq = Double(OHbfreq)*factor;
	}
    }
    return freq;
}

Double WSRTDataSource::contFreqBandwidth() const
{
    return freqBandwidth();
}


Matrix<Float> WSRTDataSource::sysTemp() const
{
    Int nRec = 2;
    Matrix<Float> tSys(itsNumAnt,nRec);
    Int offset = 568;
    for (Int i=0; i<nRec; i++) {
	for (Int j=0; j<itsNumAnt; j++) {
	    itsDataConversion->toLocal (tSys(j,i),itsSCBuffer+offset);
	    offset += 4;
	}
    }
    return tSys;
}

// get data vector and flags for the current data block
Vector<Complex> WSRTDataSource::msData(Vector<Bool>& flag)
{
    Short IHnexp, IHfscal, IHoffs;
    itsDataConversion->toLocal (IHnexp, itsIHBuffer+54);
    itsDataConversion->toLocal (IHfscal,itsIHBuffer+56);
    itsDataConversion->toLocal (IHoffs, itsIHBuffer+58);

    Char  SHdatyp[2];
    itsDataConversion->toLocal (SHdatyp,itsSHBuffer+30,2);

    //convert from WU to Jy
    Double fscal;
    fscal = IHfscal/200.;
    //for YX data: change sign of data value
    if (SHdatyp[0]=='Y' && SHdatyp[1]=='X') fscal = -fscal;
    if (itsNoScale) {
	fscal = 1;
	IHoffs = 0;
    } else {
	fscal *= pow(2,IHnexp);
    }

    Short IHndatp;
    itsDataConversion->toLocal (IHndatp,itsIHBuffer+50);
//    cout << className() << "::readDB() -- read "
//	 << 2*IHndatp << " data values into itsDBBuffer" << endl;
    Short* DBdata = new Short[2*IHndatp];
    itsDataConversion->toLocal (DBdata,itsDBBuffer,2*IHndatp);

    Vector<Complex> data(IHndatp);
    Float re, im;
    for (Int i=0; i<IHndatp; i++) {
	//check for "undefined" values and flag them
	//defensive (?) because flags are preset to True
	if (DBdata[2*i]==-32768 || DBdata[2*i+1]==-32768) {
	    flag(i) = True;
	    data(i) = Complex(0,0);
	}else{
	    flag(i) = False;
	    re = fscal*DBdata[2*i];
	    im = fscal*DBdata[2*i+1];
	    data(i) = Complex(re,im);
	}
//	cout << className() << "::msData(...) -- DBre, DBim, re, im = "
//	     << DBdata[2*i] << ", " << DBdata[2*i+1]
//	     << ", "	<< re << ", " << im << endl;
    }
    delete [] DBdata;
    return data;
}

Cube<Complex> WSRTDataSource::continuumData() const
{
    return itsContinuumData;
}

Cube<Bool> WSRTDataSource::continuumFlag() const
{
    return itsContinuumFlag;
}


// Get correlation types
Vector<Int> WSRTDataSource::corrTypes() const
{
    Vector<Int> types(itsNumCorr);
    types = Stokes::Undefined;

    Char OHdatyp[2];
    Stokes::StokesTypes type;
    uShort OHnrsts;
    itsDataConversion->toLocal (OHnrsts,itsOHBuffer+666);

    // Loop through the set table in the OH block
    // and assemble the correlation types (ignoring "IF")
    uInt nrfound = 0;
    uInt iloop;
    Bool matched;
    for (uInt setnr=0; setnr<OHnrsts; setnr++) {
	itsDataConversion->toLocal (OHdatyp,itsOHBuffer+676+12*setnr,2);
	//convert to String and then to enum StokesTypes
	type = Stokes::type(String(OHdatyp,2));
	//loop through the filled part of types until match or end
	matched = False;
	iloop = 0;
	while (!matched && iloop<nrfound) {
	    if (type == types(iloop)) {
		matched = True;
	    }else{
		iloop++;
	    }
	}

	//if no match and valid valid type: add type to the vector
	if (!matched && type!=Stokes::Undefined) {
	    types(nrfound) = type;
	    nrfound++;
	}
    }
    return types;
}

// Get correlation type for the current data block
Stokes::StokesTypes WSRTDataSource::corrType()
{
    Char SHdatyp[2];
    itsDataConversion->toLocal (SHdatyp,itsSHBuffer+30,2);
    // Convert to string
    String datyp(SHdatyp,2);
    return Stokes::type(datyp);
}

// Get frequency channel number for the current data block
uInt WSRTDataSource::chanNr()
{
    uInt channr = 0;
    if (itsBECode == 'C') {
	channr = 0;
    }else if (itsBECode == 'L' || itsBECode == 'X') {
	Short SHbandnr;
	itsDataConversion->toLocal (SHbandnr,itsSHBuffer+20);
	channr = SHbandnr - 1;
    }else{
	throw (AipsError(" Unknown Backend Type"));
    }
    return channr;
}

// Get frequency band number for the current data block (zero-based)
uInt WSRTDataSource::bandNr()
{
    uInt bandnr = 0;
    if (itsBECode == 'C') {
	Short SHbandnr;
	itsDataConversion->toLocal (SHbandnr,itsSHBuffer+20);
	bandnr = SHbandnr;
    }else if (itsBECode == 'L' || itsBECode == 'X') {
	bandnr = 0;
    }else{
	throw (AipsError(" Unknown Backend Type"));
    }
    return bandnr;
}

Int WSRTDataSource::nextDataBlock()
{
//    cout << "WSRTDataSource::nextDataBlock() started" << endl;
    Int nRow;
    Int SHshlnk, IHihlnk, SHnih;
    Short IHbandnr;
    Char SHdatyp[2];

    Bool moreData = True;
    Bool gotOKData = False;
    Bool gotNewData = False;
    while (!gotOKData && moreData) {
	gotNewData = False;
	// Get the next IH block;
	if (itsFirstDataBlock) {
//	    cout << "just use the very first IH block" << endl;
	}else{
//	    cout << "go for the next IH block" << endl;
	    itsDataConversion->toLocal (IHihlnk,itsIHBuffer+12);
	    if (IHihlnk < 0) {
//		cout << "WSRTDataSource: no more IH blocks in this SH block" << endl;
		// get next SH block
		itsDataConversion->toLocal (SHshlnk,itsSHBuffer+12);
		if (SHshlnk < 0) {
		    cout << "WSRTDataSource: no more SH blocks => no more data" << endl;
		    // invalidate offsets
		    moreData = False;
		    gotNewData = False;
		    itsSHoff = -1;
		    itsIHoff = -1;
		    itsDBoff = -1;
		}else{
//		    cout << "WSRTDataSource: got new SH block" << endl;
		    gotNewData = True;
		    itsSHoff = SHshlnk;
		    // read new SH block
		    readSH();
		    
		    // get new IH offset
		    itsDataConversion->toLocal (SHnih,itsSHBuffer+168);
		    itsIHoff = SHnih;
		}
	    }else{
//		cout << "WSRTDataSource: got new IH block" << endl;
		gotNewData = True;
		itsIHoff = IHihlnk;
	    }
	    if (gotNewData) {
		// get the next IH block;
//		cout << "go read next IH block" << endl;
		readIH();
		// get new DB offset and read the data block
		itsDBoff = itsIHoff+1;
		itsDataConversion->toLocal (itsDBlen, itsIHBuffer+50);
		itsDBlen = 4*itsDBlen;
		readDB();
	    }
	}
	// select dattyp!=IF && band>0
	// band 0 is the continuum point for line backends DLB and DXB
	// band 0 is just an average over all continuum bands for the DCB
	if (gotNewData || itsFirstDataBlock) {
	    // get the bandnr and datatype
	    itsDataConversion->toLocal (IHbandnr,itsIHBuffer+20);
	    itsDataConversion->toLocal (SHdatyp,itsSHBuffer+30,2);
//	    cout << " SHdatyp, IHbandnr = " << SHdatyp[0] << SHdatyp[1]
//		<< ", " << IHbandnr << endl;
	    if (SHdatyp[0] == 'I') {
		gotOKData = False;
	    }else if (IHbandnr==0) {
		Short IHndatp;
		itsDataConversion->toLocal (IHndatp,itsIHBuffer+50);
		if (itsContinuumData.nelements()==0) {
		    itsContinuumData.resize(itsNumCorr,1,itsNumBase*IHndatp);
		    itsContinuumFlag.resize(itsNumCorr,1,itsNumBase*IHndatp);
//		    cout << "shapes: " << itsContinuumData.shape() << ", "
//			 << itsContinuumFlag.shape() << endl;
		}
		nRow = IHndatp;
		cacheContinuum(nRow);
		gotOKData = False;
	    }else{
		gotOKData = True;
	    }
	    itsFirstDataBlock = False;
	}
    }

//    Short SHpolc;
//    itsDataConversion->toLocal (SHpolc,itsSHBuffer+32);
//    cout << " SH block's Dipole position code = " << SHpolc << endl;

    if (gotNewData) {
	Short IHndatp;
	itsDataConversion->toLocal (IHndatp,itsIHBuffer+50);
//	cout << " Nr of datapoints in this data block = " << IHndatp << endl;
	nRow = IHndatp;
    }else{
	nRow = 0;
    }
    return nRow;
}
    
Vector<Int> WSRTDataSource::msAntennaId(Int) const
{
    //ignore argument ico
    Vector<Int> id(2);
    id = 0;
    Short IHtel[2];
    itsDataConversion->toLocal (IHtel,itsIHBuffer+24,2);
    id(0) = IHtel[0];
    id(1) = IHtel[1];
    return id;
}

// Get the times for the MS main table in seconds
Vector<Double> WSRTDataSource::msTime() const
{
    //times: exposure, interval, time
    Vector<Double> time(3);
    time = 0.0;
    Float IHintt;
    Double OHjday;
    itsDataConversion->toLocal (IHintt,itsIHBuffer+116);
    itsDataConversion->toLocal (OHjday,itsOHBuffer+64);
    time(0) = IHintt;  //in U.T. sec
    time(1) = IHintt;
    time(2) = (OHjday+40000-0.5)*86400.;  //in JD sec
    return time;
}

// Get UVW values for the MS main table (in meters)
Vector<Double> WSRTDataSource::msUVW(const uInt sampleNr) const
{
    Vector<Double> UVW(3);
    Vector<Double> dir(2);
    Float IHhab, IHdha, IHdrt;
    Double ha, dec;

    // get start and incr HA in U.T. sec ****FOUT: in circle fractions
    itsDataConversion->toLocal (IHhab,itsIHBuffer+36);
    itsDataConversion->toLocal (IHdha,itsIHBuffer+44);
    // make HA in radians
    ha = (IHhab + sampleNr*IHdha)*C::_2pi;
    // get dec in radians
    dir = fieldDelayDir();
    dec = dir(1);

    // get baseline in meters
    itsDataConversion->toLocal (IHdrt,itsIHBuffer+32);

    // calculate UVW
    UVW(0) = -IHdrt * cos(ha);
    UVW(1) = -IHdrt * sin(ha) * sin(dec);
    UVW(2) = 0.0;
    return UVW;
}

// +------------------------------------------+
// | Private functions to read dataset blocks |
// +------------------------------------------+

// Read the FD block.
// The function will "feel" the proper data conversion and store all
// the information needed to further interpret the dataset (layout
// mostly).
Bool WSRTDataSource::readFD()
{
    // Allocate buffer for first FD record and read it
    itsFDBuffer = new Char[128];
    itsFile.read(128,itsFDBuffer);

    // Construct the correct data conversion object
    // Assume: input in IBM format ("real WSRT" format)
    itsDataConversion = new IBMDataConversion();
    // Check ASCII field and short-integer fields
    short FDcbi;
    char FDcbt[2];
    itsDataConversion->toLocal (FDcbt,itsFDBuffer+2,2);
    if (String(FDcbt,2) != "FD") {
	// No IBM input format: probably VAX format
	// Check byte order in unequal-data-block-flag field
	itsDataConversion = new VAXDataConversion();
	itsDataConversion->toLocal (FDcbi,itsFDBuffer);
	if (FDcbi != 32767) {
	    // Must be canonical format (e.g., Sun-written)
	    // (future option for archiving)
	    itsDataConversion = new CanonicalDataConversion();
	}
    }
    // Do final health check:
    itsDataConversion->toLocal (FDcbi,itsFDBuffer);
    itsDataConversion->toLocal (FDcbt,itsFDBuffer+2,2);
    cout << "FDcbi : " << FDcbi << " should be: 32767" << endl;
    cout << "FDcbt : " << String(FDcbt,2) << "    should be: FD" << endl;
    if (String(FDcbt,2)!="FD" || FDcbi != 32767) {
	    throw (AipsError(" There is something wrong with input format"));
    }

    // Read some control information into the data members
    itsDataConversion->toLocal (itsFDlen, itsFDBuffer+8);
    
    itsDataConversion->toLocal (itsLRCRD, itsFDBuffer+26);
    itsDataConversion->toLocal (itsPHBLL, itsFDBuffer+28);
    
    itsDataConversion->toLocal (itsOHlen, itsFDBuffer+38);
    itsDataConversion->toLocal (itsOHoff, itsFDBuffer+40);
    itsDataConversion->toLocal (itsNrOfOH,itsFDBuffer+44);

    itsDataConversion->toLocal (itsSClen, itsFDBuffer+50);
    itsDataConversion->toLocal (itsSCoff, itsFDBuffer+52);
    itsDataConversion->toLocal (itsNrOfSC,itsFDBuffer+56);
    
    itsDataConversion->toLocal (itsSHlen, itsFDBuffer+62);
    itsDataConversion->toLocal (itsSHoff, itsFDBuffer+64);
    itsDataConversion->toLocal (itsNrOfSH,itsFDBuffer+68);
    
    itsDataConversion->toLocal (itsIHlen, itsFDBuffer+74);
    itsDataConversion->toLocal (itsIHoff, itsFDBuffer+76);
    itsDataConversion->toLocal (itsNrOfIH,itsFDBuffer+80);
    
    itsDataConversion->toLocal (itsDBlen, itsFDBuffer+86);
    itsDataConversion->toLocal (itsDBoff, itsFDBuffer+88);
    itsDataConversion->toLocal (itsNrOfDB,itsFDBuffer+92);

    // Total nr of blocks within the dataset
    itsDataConversion->toLocal (itsNBL,itsFDBuffer+100);

    cout << "  blockSize = " << itsPHBLL*itsLRCRD << endl;
    cout << "  itsNBL, itsPHBLL, itsLRCRD = " << itsNBL << ", "
	 << itsPHBLL << ", " << itsLRCRD << endl;

    return True;
}

// Read the OH block
Bool WSRTDataSource::readOH()
{
    // Allocate buffer for OH block
    itsOHBuffer = new char[itsOHlen*itsLRCRD];

    // Skip to the start of the OH block (record itsOHoff)
    Long pointPos;
    pointPos = itsFile.seek(itsOHoff*itsLRCRD);
    if (pointPos < 0) {
	throw (AipsError(" Input stream not seekable"));
    }
//    cout << className() << ": new pointer position = " << pointPos << endl; 

    // Read the block
    itsFile.read(itsOHlen*itsLRCRD,itsOHBuffer);
    
    // Extract and save some control information
    Short OHnfreq, OHnrpol, OHnrint;
    itsDataConversion->toLocal (OHnfreq,itsOHBuffer+154);
    itsDataConversion->toLocal (OHnrpol,itsOHBuffer+158);
    itsDataConversion->toLocal (OHnrint,itsOHBuffer+160);
    itsDataConversion->toLocal (itsBECode,itsOHBuffer+173);
    
    if (itsBECode == 'C') {
	itsNumBand = OHnfreq;
	itsNumChan = 1;
    }else if (itsBECode == 'L' || itsBECode == 'X') {
	// ignore band 0, the continuum point for line backends
	itsNumBand = 1;
	itsNumChan = OHnfreq-1;
    }else{
	throw (AipsError(" Unknown Backend Type"));
    }
    itsNumCorr = OHnrpol;
    itsNumBase = OHnrint;

    return True;
}

// Read the SC block
Bool WSRTDataSource::readSC()
{
    // Allocate buffer for block
    itsSCBuffer = new char[itsSClen*itsLRCRD];

    // Skip to the start of the SC block (record itsSCoff)
    Long pointPos;
    pointPos = itsFile.seek(itsSCoff*itsLRCRD);
    if (pointPos < 0) {
	throw (AipsError(" Input stream not seekable"));
    }
//    cout << className() << ": new pointer position = " << pointPos << endl; 

    // Read the block
    itsFile.read(itsSClen*itsLRCRD,itsSCBuffer);

    return True;
}


// Read the first or next SH block
Bool WSRTDataSource::readSH()
{
    // Allocate buffer for block
    if (itsSHBuffer==0) {
	itsSHBuffer = new char[itsSHlen*itsLRCRD];
    }

    // Skip to the start of the SH block (record itsSHoff)
    Long pointPos;
    pointPos = itsFile.seek(itsSHoff*itsLRCRD);
    if (pointPos < 0) {
	throw (AipsError(" Input stream not seekable"));
    }
//    cout << className() << ": new pointer position = " << pointPos << endl; 

    // Read the block
    itsFile.read(itsSHlen*itsLRCRD,itsSHBuffer);

    return True;
}


// Read the first or next IH block
Bool WSRTDataSource::readIH()
{
    // Allocate buffer for block
    if (itsIHBuffer==0) {
	itsIHBuffer = new char[itsIHlen*itsLRCRD];
    }

    // Skip to the start of the IH block (record itsIHoff)
    Long pointPos;
    pointPos = itsFile.seek(itsIHoff*itsLRCRD);
    if (pointPos < 0) {
	throw (AipsError(" Input stream not seekable"));
    }
//    cout << className() << ": new pointer position = " << pointPos << endl; 

    // Read the block
    itsFile.read(itsIHlen*itsLRCRD,itsIHBuffer);

    return True;
}


// Read the first or next DB block
Bool WSRTDataSource::readDB()
{
    // Allocate buffer for block
    if (itsDBBuffer==0) {
	itsDBBuffer = new char[itsDBlen];
	cout << className() << "::readDB() -- itsDBBuffer has "
	     << itsDBlen << " bytes" << endl;
    }

    // Skip to the start of the DB block (record itsDBoff)
    Long pointPos;
    pointPos = itsFile.seek(itsDBoff*itsLRCRD);
    if (pointPos < 0) {
	throw (AipsError(" Input stream not seekable"));
    }
//    cout << className() << ": new pointer position = " << pointPos << endl; 

    // Read the block
    itsFile.read(itsDBlen,itsDBBuffer);

    return True;
}

// Cache the continuum data and flags of the current (continuum) block
void WSRTDataSource::cacheContinuum(Int nTime)
{
    // Get the data for a specific baseline and correlation (all times),
    // convert the data and set the flags appropriately,
    // and insert them in the proper place in the cache cubes

    // get the correlation types
    Vector<Int> cType(itsNumCorr);
    cType = corrTypes();
    // translate correlation type into a correlation index
    Int iCorr = 0;
    for (Int i=0; i<itsNumCorr; i++) {
	if (corrType() == cType(i)) iCorr = i;
    }

    // get the antenna (and later also feed) id's,
    Int iRow;
    Vector<Int> msAnt(2);
    msAnt = msAntennaId();
    // translate into a baseline index, cq. relative row nr
    if (itsContinuumRow(msAnt(0),msAnt(1)) < 0) {
	//assign next free row
	iRow = itsNextContinuumRow++;
	//remember assignment
	itsContinuumRow(msAnt(0),msAnt(1)) = iRow;
//	cout << "new iRow assignment: "<< iRow << " -> base("
//	     << msAnt(0) << "," << msAnt(1) << ")" << endl;
    }else{
	iRow = itsContinuumRow(msAnt(0),msAnt(1));
//	cout << "old iRow assignment: "<< iRow << " -> base("
//	     << msAnt(0) << "," << msAnt(1) << ")" << endl;
    }

    Short IHnexp, IHfscal, IHoffs;
    itsDataConversion->toLocal (IHnexp, itsIHBuffer+54);
    itsDataConversion->toLocal (IHfscal,itsIHBuffer+56);
    itsDataConversion->toLocal (IHoffs, itsIHBuffer+58);

    Char  SHdatyp[2];
    itsDataConversion->toLocal (SHdatyp,itsSHBuffer+30,2);

    //convert from WU to Jy
    Double fscal;
    fscal = IHfscal/200.;
    //for YX data: change sign of data value
    if (SHdatyp[0]=='Y' && SHdatyp[1]=='X') fscal = -fscal;
    if (itsNoScale) {
	fscal = 1;
	IHoffs = 0;
    } else {
	fscal *= pow(2,IHnexp);
    }

    Short* DBdata = new Short[2*nTime];
    itsDataConversion->toLocal (DBdata,itsDBBuffer,2*nTime);

    Float re, im;
    Int row;
    for (Int iTime=0; iTime<nTime; iTime++) {
	//check for "undefined" values and flag them
	//defensive (?) because flags are preset to True
	row = iTime*itsNumBase + iRow;
//	cout << "filling row nr " << row << endl;
	if (DBdata[2*iTime]==-32768 || DBdata[2*iTime+1]==-32768) {
	    itsContinuumFlag(iCorr,0,row) = True;
	    itsContinuumData(iCorr,0,row) = Complex(0,0);
	}else{
	    itsContinuumFlag(iCorr,0,row) = False;
	    re = fscal*DBdata[2*iTime];
	    im = fscal*DBdata[2*iTime+1];
	    itsContinuumData(iCorr,0,row) = Complex(re,im);
	}
    }
    delete [] DBdata;
    return;
}







