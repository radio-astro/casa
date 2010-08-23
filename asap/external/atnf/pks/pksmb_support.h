//#---------------------------------------------------------------------------
//# pksmb_support.h: Support functions for Parkes glish clients.
//#---------------------------------------------------------------------------
//# livedata - processing pipeline for single-dish, multibeam spectral data.
//# Copyright (C) 1997-2009, Australia Telescope National Facility, CSIRO
//#
//# This file is part of livedata.
//#
//# livedata is free software: you can redistribute it and/or modify it under
//# the terms of the GNU General Public License as published by the Free
//# Software Foundation, either version 3 of the License, or (at your option)
//# any later version.
//#
//# livedata is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
//# more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with livedata.  If not, see <http://www.gnu.org/licenses/>.
//#
//# Correspondence concerning livedata may be directed to:
//#        Internet email: mcalabre@atnf.csiro.au
//#        Postal address: Dr. Mark Calabretta
//#                        Australia Telescope National Facility, CSIRO
//#                        PO Box 76
//#                        Epping NSW 1710
//#                        AUSTRALIA
//#
//# http://www.atnf.csiro.au/computing/software/livedata.html
//# $Id: pksmb_support.h,v 19.10 2009-09-29 07:45:02 cal103 Exp $
//#---------------------------------------------------------------------------
//# Original: David Barnes, February 1997.
//#---------------------------------------------------------------------------
#ifndef ATNF_PKSMB_SUPPORT_H
#define ATNF_PKSMB_SUPPORT_H

// AIPS++ includes.
#include <casa/aips.h>
#include <casa/Arrays/Array.h>
#include <casa/BasicSL/String.h>

// ATNF includes.
#include <atnf/pksGlish/GlishSysEvent.h>
#include <atnf/pksGlish/GlishRecord.h>

#include <casa/namespace.h>
#define NIGHTTIME_ELEVATION_LIMIT -5.0


// Global generic Glish setup operations.
void pksmbSetup(GlishSysEventSource &glishStream,
                String clientName = String("Unknown client"));
Bool shutdown_event(GlishSysEvent &event, void *);
Bool unknown_event(GlishSysEvent &event, void *);


// Global Glish parameter accessing routines.
template<class T> Bool getParm(const GlishRecord &parms, const String &item,
                               const T &default_val, T &value);
template<class T> Bool getParm(const GlishRecord &parms, const String &item,
                               const T &default_val, Array<T> &value);
template<class T> Bool getParm(const GlishRecord &parms, const String &item,
                               const Array<T> &default_val, Array<T> &value);

// Global message logging functions.
void logMessage(String msg);
void logMessage(String msg,   uInt val, String suffix = "");
void logMessage(String msg,    Int val, String suffix = "");
void logMessage(String msg, Double val, String suffix = "");
void logWarning(String msg);
void logError(String msg);

#ifndef AIPS_NO_TEMPLATE_SRC
#include <atnf/pks/pksmb_support.cc>
#endif //# AIPS_NO_TEMPLATE_SRC
#endif
