//# CacheFactory.h: Factory to return a cache of the proper type
//# Copyright (C) 2013
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
//# $Id: $
#ifndef CACHE_FACTORY_
#define CACHE_FACTORY_

#include <casa/BasicSL/String.h>
#include <tables/Tables/Table.h>
#include <plotms/PlotMS/PlotMS.h>
#include <plotms/Data/CalCache.h>
#include <plotms/Data/MSCache.h>
#include <plotms/Data/PlotMSCacheBase.h>

namespace casa {

class CacheFactory {
public:
    static PlotMSCacheBase* getCache(const String &filename,
                                     PlotMSApp *parent) {
        if(Table::isReadable(filename)) {
            Table tab(filename);
            // Construct proper empty cache if necessary
            if(tab.tableInfo().type() == "Calibration")
                return new CalCache(parent);
            else
                return new MSCache(parent);
        }
        return NULL;
    }

    static bool needNewCache(const PlotMSCacheBase *cache,
                             const String &filename) {
        Table tab(filename);
        if(!cache) return true;
        if((cache->cacheType() == PlotMSCacheBase::CAL &&
            tab.tableInfo().type() != "Calibration") ||
           (cache->cacheType() == PlotMSCacheBase::MS &&
            tab.tableInfo().type() == "Calibration")) {
            return true;
        }
        return false;
    }
};

} // namespace casa

#endif//CACHE_FACTORY_
