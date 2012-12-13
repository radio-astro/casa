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
//# $Id: DOmirfiller.cc,v 1.2 2009/09/03 23:28:32 pteuben Exp $

//# Includes
#if 0
#include <miriad/Filling/DOmirfiller.h>
#include <miriad/Filling/FillMetadata.h>
#include <casa/Arrays/ArrayUtil.h>
#include <tasking/Tasking.h>

// not available to users
mirfiller::mirfiller() : filler_p(String()) { }
mirfiller::mirfiller(const mirfiller& other) : filler_p(String()) { }

mirfiller::mirfiller(const String& mirfile) : filler_p(mirfile, False) { }

mirfiller::~mirfiller() { }

Vector<String> mirfiller::methods() const {
    return stringToVector(
        String("setoptions,getoptions,summary,fill,selectspectra,selectpols"));
}

String mirfiller::className() const { return String("mirfiller"); }

MethodResult mirfiller::runMethod(uInt which,     
                                  ParameterSet &parameters,
                                  Bool runMethod) 
{
    static String msfile = "msfile";
    static String options = "options";
    static String header = "header";
    static String wideChans = "widechans";
    static String windows = "windows";
    static String pols = "pols";
    enum methodnames { SETOPTIONS, GETOPTIONS, SUMMARY, FILL, SELSP, SELPOL };

    switch (which) {
    case SETOPTIONS: {
        Record opts;
//        GlishRecord *args = parameters.values();
        Parameter<GlishRecord > in(parameters, options, ParameterSet::In);
        in().toRecord(opts);

        if (runMethod) {
            setoptions(opts);
        }
        break;
    }

    case GETOPTIONS: {
//        GlishRecord *args = parameters.values();
        Parameter<GlishRecord > opts(parameters, options, ParameterSet::Out);

        if (runMethod) {
//            Record opts = getoptions();
            opts().fromRecord(getoptions());
        }
        break;
    }

    case SUMMARY: {
        Parameter<GlishRecord> hdr(parameters, header, ParameterSet::Out);
        Parameter<Bool> verb(parameters, "verbose", ParameterSet::In);
        Parameter<Bool> prevu(parameters, "preview", ParameterSet::In);

        if (runMethod) {
            hdr() = summary(verb(), prevu());
        }
        break;
    }

    case SELSP: {
        Parameter<Vector<Int> > wide(parameters, wideChans, ParameterSet::In);
        Parameter<Vector<Int> > wins(parameters, windows, ParameterSet::In);

        if (runMethod) {
            selectspectra(wide(), wins());
        }
        break;
    }

    case SELPOL: {
        Parameter<Vector<Int> > pollist(parameters, pols, ParameterSet::In);
        if (runMethod) {
            selectpols(pollist());
        }
        break;
    }

    case FILL: {
        Parameter<String> ms(parameters, msfile, ParameterSet::In);

        if (runMethod) {
            fill(ms());
        }
        break;
    }

    default:
        return error("Unknown method requested");
        break;
    }

    return ok();
}

Vector<String> mirfiller::noTraceMethods() const {
    return stringToVector(
        String("setoptions,getoptions,summary,selectspectra,selectpols"));
}

void mirfiller::selectpols(const Vector<Int> pols) {
    Stokes::StokesTypes ptype = Stokes::Undefined;
    MirPolSetup polsel;
    for(uInt i=0; i < pols.nelements(); i++) {
        ptype = Stokes::type(pols(i));
        if (ptype != Stokes::Undefined) 
            polsel.setCorr(MirPolCorr::toMiriad(ptype));
    }
    filler_p.selectPols(polsel);
}

GlishRecord mirfiller::summary(Bool verbose, Bool scan) { 
    return filler_p.summary(verbose, scan); 
}

#endif
