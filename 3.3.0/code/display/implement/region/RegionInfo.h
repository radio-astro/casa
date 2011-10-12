//# RegionInfo.h: union class for the various types of region information
//# Copyright (C) 2011
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


#ifndef REGION_REGIONINFO_H_
#define REGION_REGIONINFO_H_

#include <list>
#include <string>
#include <images/Images/ImageStatistics.h>
#include <casadbus/types/ptr.h>

namespace casa {

    namespace viewer {

	class RegionInfo {
	    public:

		enum InfoTypes { MsInfoType, ImageInfoType, InvalidInfoType };

		typedef ImageStatistics<Float>::stat_list stats_t;

		RegionInfo( ) : type_(InvalidInfoType) { }
		RegionInfo( const RegionInfo &other ) : stat_list_(other.stat_list_), label_(other.label_), type_( other.type_) { }
		virtual ~RegionInfo( ) { }

		memory::cptr<stats_t> &list( ) { return stat_list_; }
		const std::string &label( ) const { return label_; }
		InfoTypes type( ) const { return type_; }

	    protected:
		RegionInfo( const std::string &label, stats_t *si, InfoTypes t ) : stat_list_(si), label_(label), type_(t) { }

	    private:
		memory::cptr<stats_t> stat_list_;
		std::string label_;
		InfoTypes type_;
	};

	class MsRegionInfo : public RegionInfo {
	    public:
		MsRegionInfo( const std::string &label, stats_t *si ) : RegionInfo(label,si,MsInfoType) { }
		~MsRegionInfo( ) { }
	};

	class ImageRegionInfo : public RegionInfo {
	    public:
		ImageRegionInfo( const std::string &label, stats_t *si ) : RegionInfo(label,si,ImageInfoType) { }
		~ImageRegionInfo( ) { }
	};
	

    }
}

#endif
