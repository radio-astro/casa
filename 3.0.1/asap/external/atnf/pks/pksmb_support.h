//----------------------------------------------------------------------------
//# pksmb_support.h: Support functions for Parkes glish clients.
//----------------------------------------------------------------------------
//# Copyright (C) 1994-2007
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
//# Original: David Barnes, February 1997.
//# $Id: pksmb_support.h,v 19.9 2007/02/07 06:44:05 cal103 Exp $
//----------------------------------------------------------------------------
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
