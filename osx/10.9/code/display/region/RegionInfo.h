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
#include <casa/cppconfig.h>

namespace casa {


	namespace viewer {

		class Polyline;


		class RegionInfo {
		public:

			enum InfoTypes { MsInfoType, ImageInfoType, SliceInfoType, PVLineInfoType, InvalidInfoType };

			typedef ImageStatistics<Float>::stat_list stats_t;
			//typedef std::pair<String,String> center_element;
			//typedef std::list<center_element> center_t;
			//typedef std::list<std::pair<String,String> > center_t;
			typedef stats_t center_t;

			RegionInfo( ) : stat_list_( ), type_(InvalidInfoType) { }
			RegionInfo( const RegionInfo &other ) : stat_list_(other.stat_list_), label_(other.label_), description_(other.description_), type_( other.type_) { }
			virtual ~RegionInfo( ) { }

			shared_ptr<stats_t> &list( ) {
				return stat_list_;
			}
			const std::string &label( ) const {
				return label_;
			}
			const std::string &description( ) const {
				return description_;
			}
			InfoTypes type( ) const {
				return type_;
			}

		protected:
			RegionInfo( const std::string &label, const std::string &desc, stats_t *si, InfoTypes t ) : stat_list_(si), label_(label), description_(desc), type_(t) { }

		private:
			shared_ptr<stats_t>  stat_list_;
			std::string label_;
			std::string description_;
			InfoTypes type_;
		};

		class MsRegionInfo : public RegionInfo {
		public:
			MsRegionInfo( const std::string &label, const std::string &desc, stats_t *si ) : RegionInfo(label,desc,si,MsInfoType) { }
			~MsRegionInfo( ) { }
		};

		class ImageRegionInfo : public RegionInfo {
		public:
			ImageRegionInfo( const std::string &label, const std::string &desc, stats_t *si ) : RegionInfo(label,desc,si,ImageInfoType) { }
			~ImageRegionInfo( ) { }
		};

		class SliceRegionInfo : public RegionInfo {
		public:
			SliceRegionInfo( const std::string &label, const std::string &desc, stats_t *si, Polyline* polyline ) :
				RegionInfo(label,desc,si,SliceInfoType), region(polyline) { }
			Polyline* getRegion(){
				return region;
			}
			~SliceRegionInfo( ) { }
		private:
			Polyline* region;
		};

		class PVLineRegionInfo : public RegionInfo {
		public:
			PVLineRegionInfo( const std::string &label, const std::string &desc, stats_t *si,
			                  const std::vector<std::string> &ps, const std::vector<std::string> &ws,
			                  const std::string &pa, const std::string &sep ) :
				RegionInfo(label,desc,si,PVLineInfoType), pixel_strings_(ps), world_strings_(ws),
				position_angle(pa), point_separation(sep) { }
			~PVLineRegionInfo( ) { }
			std::vector<std::string> pixelStrings( ) const {
				return pixel_strings_;
			}
			std::vector<std::string> worldStrings( ) const {
				return world_strings_;
			}
			const std::string &positionAngle( ) const {
				return position_angle;
			}
			const std::string &separation( ) const {
				return point_separation;
			}
		private:
			std::vector<std::string> pixel_strings_;
			std::vector<std::string> world_strings_;
			std::string position_angle;
			std::string point_separation;
		};


	}
}

#endif
