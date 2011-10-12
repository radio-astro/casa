//# ClassFileName.cc:  this defines ClassName, which ...
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
//# $Id: FillMetadata.cc,v 1.3 2011/08/11 18:03:53 pteuben Exp $

//# Includes
#include <miriad/Filling/FillMetadata.h>
#include <ms/MeasurementSets.h>              // Measurementset and MSColumns
#include <casa/BasicMath/Math.h>
#include <casa/sstream.h>
#include <measures/Measures/MeasTable.h>
#include <measures/Measures/MeasConvert.h>
#include <casa/Logging/LogIO.h>

#include <mirlib/maxdimc.h>
#include <mirlib/miriad.h>

MirInfo::MirInfo() { }
MirInfo::~MirInfo() { }

MirField::MirField(float delra, float deldec, Int fid) 
    : dra(delra), ddec(deldec), id(fid) { }
MirField::~MirField() { }

MirPolCorr::MirPolCorr(Int poltype) : corr(poltype) { }
MirPolCorr::~MirPolCorr() { }

const Stokes::StokesTypes MirPolCorr::corratopol[] = { 
    Stokes::YY, Stokes::XX, Stokes::XX, Stokes::YY, 
    Stokes::LL, Stokes::RR, Stokes::RR, Stokes::LL, 
    Stokes::I,  Stokes::Q,  Stokes::U,  Stokes::V
};

const Stokes::StokesTypes MirPolCorr::corrbtopol[] = { 
    Stokes::XX, Stokes::YY, Stokes::XX, Stokes::YY, 
    Stokes::RR, Stokes::LL, Stokes::RR, Stokes::LL, 
    Stokes::I,  Stokes::Q,  Stokes::U,  Stokes::V
};

Stokes::StokesTypes MirPolCorr::toAips(Int pol) {
    // convert AIPS-convention Stokes description to aips++ enum
    // CHECK if these are really the right conversions for AIPS++
    if (pol < 0) {
	switch (pol) {
	case -8: return Stokes::YX; break;
	case -7: return Stokes::XY; break;
	case -6: return Stokes::YY; break;
	case -5: return Stokes::XX; break;
	case -4: return Stokes::LR; break;
	case -3: return Stokes::RL; break;
	case -2: return Stokes::LL; break;
	case -1: return Stokes::RR; break;
	default: return Stokes::Undefined;  // shouldn't happen
	}
    }

    return ((Stokes::StokesTypes) pol);
}

Int MirPolCorr::toMiriad(Stokes::StokesTypes pol) {
    if (pol > 4) {
	switch (pol) {
	case Stokes::YX: return -8; break;
	case Stokes::XY: return -7; break;
	case Stokes::YY: return -6; break;
	case Stokes::XX: return -5; break;
	case Stokes::LR: return -4; break;
	case Stokes::RL: return -3; break;
	case Stokes::LL: return -2; break;
	case Stokes::RR: return -1; break;
	default: return 0; break;  // shouldn't happen
	}
    }
    return (Int(pol));
}

ConstMirPolSetup::ConstMirPolSetup() : corrs(), iter(corrs) { }
ConstMirPolSetup::ConstMirPolSetup(ConstMirPolSetup& other) 
    : corrs(other.corrs), iter(corrs) { }
ConstMirPolSetup::~ConstMirPolSetup() { }

Bool ConstMirPolSetup::find(Int poltype, ConstListIter<MirPolCorr>& li) {
    uInt cur = li.pos();
    while (1) {
	if (! li.atEnd()) {
	    if (li.getRight() == poltype) return True;
	    ++li;
	}
	if (li.atEnd()) li.toStart();
	if (li.pos() == cur) break;
    }
    return False;
}

MirPolSetup::MirPolSetup() : ConstMirPolSetup() { } 
MirPolSetup::MirPolSetup(ConstMirPolSetup& other) : ConstMirPolSetup(other) { }
MirPolSetup::~MirPolSetup() { }

void MirPolSetup::addCorr(Int poltype) {
    iter.toEnd();
    iter.addRight(MirPolCorr(poltype));
}

MirPolRecps::~MirPolRecps() { }

Int MirSource::nxtid_p = 0;

MirSource::MirSource(String src, double sra, double sdec, float sepoch, 
                     Double stime, Int sid) 
    : name(src), ra(sra), dec(sdec), epoch(sepoch), time(stime), 
      id(sid), row(-1), solsys(N_Objects), motion_p(0)
{ }

MirSource::MirSource(int mirh, Double stime) : time(stime), id(-1), row(-1), 
    solsys(N_Objects), motion_p(0)
{
    int n;
    float twok = 2000.0;
    double zero = 0;
    char src[32];
    src[0] = '\0';
    uvrdvr_c(mirh, H_REAL, "epoch", (char *)&epoch, (char *)&twok, n);
    uvrdvr_c(mirh, H_DBLE, "ra", (char *)&ra, (char *)&zero, n);
    uvrdvr_c(mirh, H_DBLE, "dec", (char *)&dec, (char *)&zero, n);
    uvrdvr_c(mirh, H_BYTE, "source", src, "", 32);
    name = String(src); // warning, conversion to char* is old
    solsys = matchSolSysObj(name);
}

MirSource::~MirSource() { 
    for(ListIter<MirField *> li(flds); ! li.atEnd(); ++li) {
        MirField *f = li.getRight();
        if (f != NULL) delete f;
    }
    if (motion_p) delete motion_p;
}

const String MirSource::solSysNames[] = {
    "MERCURY", "VENUS", "MARS", "JUPITER", "SATURN", "URANUS", "NEPTUNE",
    "PLUTO", "SUN", "MOON"
};

MirSource::SolSysObject MirSource::matchSolSysObj(const String &name) {
    String nm(name);
    nm.upcase();
    uInt i, n=N_Objects;
    for(i=0; i < n; ++i) {
	if (nm == solSysNames[i]) break;
    }
    return static_cast<SolSysObject>(i);
}

MirFreqSetup::MirFreqSetup(int mirh) 
    : mode(0), corbw(NULL), corf(NULL), nschan(NULL), ischan(NULL), 
      sfreq(NULL), sdf(NULL), rfreq(NULL), wfreq(NULL), wwidth(NULL), freq(0), 
      id(-1), nfidx(0), wfidx(0)
{
    char vtype[10];
    int vlen, vupd, zero=0;

    // load scalar data
    uvrdvr_c(mirh, H_INT, "cormode", (char *)&mode, (char *)&zero, vlen);
    uvrdvr_c(mirh, H_INT, "nspect", (char *)&nspect, (char *)&zero, vlen);
    uvrdvr_c(mirh, H_INT, "nwide", (char *)&nwide, (char *)&zero, vlen);
    uvrdvr_c(mirh, H_INT, "nchan", (char *)&nchan, (char *)&zero, vlen);

    // load correlator setup parameters
    uvprobvr_c(mirh, "corfin", vtype, &vlen, &vupd);
    if (vtype[0]=='r') {
	corf  = new float[4];
	corbw = new float[4];

	uvgetvr_c(mirh, H_REAL, "corfin", (char *)corf, 4);
	uvgetvr_c(mirh, H_REAL, "corbw", (char *)corbw, 4);
    }

    // load window layout data
    if (nspect > 0) {
        nschan = new int[nspect];
        ischan = new int[nspect];
        sfreq  = new double[nspect];
        sdf    = new double[nspect];

        uvgetvr_c(mirh, H_INT, "nschan", (char *)nschan, nspect);
        uvgetvr_c(mirh, H_INT, "ischan", (char *)ischan, nspect);
        uvgetvr_c(mirh, H_DBLE, "sfreq", (char *)sfreq, nspect);
        uvgetvr_c(mirh, H_DBLE, "sdf", (char *)sdf, nspect);

        uvprobvr_c(mirh, "restfreq", vtype, &vlen, &vupd);
        if (vupd) {
            rfreq  = new double[nspect];
            uvgetvr_c(mirh, H_DBLE, "restfreq", (char *)rfreq, nspect);
        }
    } 
    if (nwide > 0) {
        wfreq  = new float[nwide];
        wwidth = new float[nwide];

        uvgetvr_c(mirh, H_REAL, "wfreq", (char *)wfreq, nwide);
        uvgetvr_c(mirh, H_REAL, "wwidth", (char *)wwidth, nwide);
    }

    double deffreq=0;
    if (nspect > 0) deffreq = (rfreq != NULL) ? rfreq[0] : sfreq[0];
    uvrdvr_c(mirh, H_DBLE, "freq", (char *)&freq, (char *)&deffreq, 1);
}

MirFreqSetup::~MirFreqSetup() {
    if (wfreq != NULL) delete wfreq;
    if (wwidth != NULL) delete wwidth;
    if (rfreq != NULL) delete rfreq;
    if (sfreq != NULL) delete sfreq;
    if (sdf != NULL) delete sdf;
    if (nschan != NULL) delete nschan;
    if (ischan != NULL) delete ischan;
    if (corf != NULL) delete corf;
    if (corbw != NULL) delete corbw;
}

Bool MirFreqSetup::operator==(MirFreqSetup &that) {
//Bool MirFreqSetup::operator(MirFreqSetup &that) {
    // this algorithm is optimized for the BIMA correllator, detecting
    // != as soon as possible
    Int i;
    if (mode != that.mode || nspect != that.nspect || nwide != that.nwide)
        return False;

    if (corf != NULL && that.corf != NULL) {
	for(i=0; i<4; i++) {
	    if (! near(corf[i], that.corf[i])) return False;
	}
    } 
    else if (corf != that.corf) {
	return False;
    }

    if (corbw != NULL && that.corbw != NULL) {
	for(i=0; i<4; i++) {
	    if (! near(corbw[i], that.corbw[i])) return False;
	}
    } 
    else if (corbw != that.corbw) {
	return False;
    }

//      if (nspect > 0) {
//          for(i=0; i < nspect; i++) {
//              if (sfreq[i]  != that.sfreq[i] ||
//                  sdf[i]    != that.sdf[i])     return False;
//          }
//          if (rfreq != that.rfreq) {   // both are non-null
//              for(i=0; i < nspect; i++) {
//                  if (rfreq[i]  != that.rfreq[i]) return False;
//              }
//          }
//          for(i=0; i < nspect; i++) {
//              if (ischan[i]  != that.ischan[i]) return False;
//          }
//      }
//      if (nwide > 0) {
//          for(i=0; i < nwide; i++) {
//              if (wfreq[i]  != that.wfreq[i] ||
//                  wwidth[i] != that.wwidth[i])  return False;
//          }
//      }
    return True;
}

uInt hashFunc(const DataDescComponents& key) { 
    return ((key.getPolarizationID() << 8) || key.getSpectralWindowID());
}

FillMetadata::FillMetadata(const String &msname)
    : ms(NULL), msc(NULL), outname(msname), obsid(0),
      inttime(-1), jyperk(1), telescope(), project(), obsupd(True), 
      nants(0), narrays(0), arrayAnt(0),
      antpos(), arrayXYZ(3,0.0), mount(NULL), diam(NULL), 
      starttime(-1), obstime(-1), feedtime(-1), modeltime(-1), lasttime(-1),
      fsetup(NULL), source(NULL), movingsrc(False), field(NULL), 
      pol(NULL), polrecp(NULL), polotf(False),
#if 0
      ddids(-1), 
#endif
      nsystemp(), wsystemp(), tsyscorrect(0), buf()
{ }


FillMetadata::~FillMetadata() {
    if (msc != NULL) { delete msc; msc = NULL; }
    if (ms != NULL) { delete ms; ms = NULL; }
//    if (pol != NULL) { delete pol; pol = NULL; }
}

void FillMetadata::setTelescope(String tel) {
    telescope = tel;
    MPosition obs;
    if (MeasTable::Observatory(obs,telescope)) {
	if (obs.type() != MPosition::ITRF) {
	    MPosition::Convert toitrf(obs, MPosition::ITRF);
	    obs = toitrf(obs);
	}
	arrayXYZ = obs.get("m").getValue();
    }
    else {
	LogIO os(LogOrigin("FillMetadata", "setTelescope()", WHERE));

	os << LogIO::WARN << "Can't find array position for the "
	   << telescope << " telescope";

	if (telescope == BIMA) 
	    os << "\n(Failed to get BIMA's position from Observatory table";
	       
	os << LogIO::POST;

//         BIMA's position:    
//           arrayXYZ(0) = -2523862.04;
//           arrayXYZ(1) = -4123592.80;
//           arrayXYZ(2) =  4147750.37;
//         ATCA's position:
//          arrayXYZ(0) = -4750915.84;
//          arrayXYZ(1) =  2792906.18;
//          arrayXYZ(2) = -3200483.75;

    }
}

const String FillMetadata::HATCREEK("HATCREEK");
const String FillMetadata::BIMA("BIMA");
const String FillMetadata::CARMA("CARMA");
const String FillMetadata::ATCA("ATCA");
const String FillMetadata::VLA("VLA");

