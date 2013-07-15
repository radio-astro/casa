#ifndef REGION_REGIONENUM_H_
#define REGION_REGIONENUM_H_

#include <string>
#include <set>
#include <stdio.h>

namespace casa {
	namespace viewer {

		class Region;

		namespace region {
			typedef std::set<Region*> region_list_type;

			template<class T> void dump( const std::string &prefix, const std::set<T*> &s ) {
				fprintf( stderr, "%s ", prefix.c_str( ) );
				for ( typename std::set<T*>::const_iterator i=s.begin( ); i != s.end( ); ++i ) {
					fprintf( stderr, "0x%x ", *i );
				}
				fprintf( stderr, "\n" );
			}


			/* enum states { undisplayed, inactive, highlighted, selected }; */
			// LSDoubleDashed is only used to preserve state (it is a Display::LineStyle option)
			enum LineStyle { SolidLine, DashLine, DotLine, LSDoubleDashed };
			enum TextPosition { TopText, RightText, BottomText, LeftText };
			enum TextFontStyle { ItalicText = 1 << 0, BoldText = 1 << 1 };
			enum Coord { J2000, B1950, Galactic, SuperGalactic, Ecliptic, DefaultCoord };
			enum Units { Degrees, Radians, Sexagesimal, Pixel, DefaultUnits };

			// state returned from mouse functions for regions...
			enum MouseState { MouseRefresh = 1 << 0, MouseSelected = 1 << 1, MouseHandle = 1 << 2 };

			enum RegionTypes { RectRegion, PointRegion, EllipseRegion, PolyRegion, PolylineRegion, PVLineRegion, NonRegion };
			enum RegionSelect { SelectAny, SelectRect, SelectPoint, SelectEllipse, SelectPoly, SelectPolyline, SelectPVLine };
			inline RegionTypes select_to_region( RegionSelect select ) {
				return select == SelectRect ? RectRegion :
				       select == SelectPoint ? PointRegion :
				       select == SelectEllipse ? EllipseRegion :
				       select == SelectPoly ? PolyRegion :
				       select == SelectPolyline ? PolylineRegion :
				       select == SelectPVLine ? PVLineRegion : NonRegion;
			}


			enum PointLocation { PointInside = 1 << 0, PointHandle = 1 << 1, PointOutside = 1 << 2 };

			enum RegionChanges { RegionChangeCreate, RegionChangeUpdate, RegionChangeReset,
			                     RegionChangeFocus, RegionChangeModified, RegionChangeLabel, RegionChangeDelete,
			                     RegionChangeStatsUpdate, RegionChangeNewChannel, RegionChangeSelected
			                   };

			class PointInfo {
			public:
				PointInfo( double x, double y, unsigned int location, unsigned int handle=0 ) :
					x_(x), y_(y), location_(location), handle_(handle) { }
				PointInfo( const PointInfo &other) : x_(other.x_), y_(other.y_), location_(other.location_), handle_(other.handle_) { }
				unsigned int handle( ) const {
					return handle_;
				}
				unsigned int &handle( ) {
					return handle_;
				}
				unsigned int location( ) const {
					return location_;
				}
				unsigned int operator&( region::PointLocation mask ) const {
					return location_ & mask;
				}
				const PointInfo &operator=( const PointInfo &other ) {
					x_ = other.x_;
					y_ = other.y_;
					location_ = other.location_;
					handle_ = other.handle_;
					return *this;
				}
				double x( ) const {
					return x_;
				}
				double y( ) const {
					return y_;
				}
				double &x( ) {
					return x_;
				}
				double &y( ) {
					return y_;
				}
			private:
				double x_, y_;
				unsigned int location_;
				unsigned int handle_;
			};

		}
	}
}

#endif
