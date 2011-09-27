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

#include <display/region/Region.h>
#include <list>
#include <string>


namespace casa {

    namespace viewer {

	class RegionInfo {
	    public:

		enum RegionInfoTypes { MsRegionInfo, ImageRegionInfo };

		typedef ImageStatistics<Float>::stat_list image_stats_t;
		typedef std::list<std::pair<String,memory::cptr<image_stats_t> > > image_stats_list_t;

		typedef std::list<std::pair<String,String> > ms_stats_t;
		typedef std::list<memory::cptr<ms_stats_t> > ms_stats_list_t;

		RegionInfo( ms_stats_list_t *msi ) : ms_info_(msi), type_(MsRegionInfo) { }
		RegionInfo( image_stats_list_t *isi ) : image_info_(isi), type_(ImageRegionInfo) { }
		RegionInfo( const RegionInfo &other ) : ms_info_(other.ms_info_), image_info_(other.image_info_), type_( other.type_) { }
		memory::cptr<ms_stats_list_t> msInfo( ) const { return ms_info_; }
		memory::cptr<image_stats_list_t> imageInfo( ) const { return image_info_; }
		RegionInfoTypes type( ) const { return type_; }
	    private:
		RegionInfoTypes type_;
		memory::cptr<ms_stats_list_t> ms_info_;
		memory::cptr<image_stats_list_t> image_info_;
	};

    }
}

#endif
