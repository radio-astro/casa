#include <display/ds9/ds9context.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <float.h>
#include <math.h>
#include <vector>
#include <display/region/RegionCreator.h>
#include <measures/Measures/MeasConvert.h>
#include <measures/Measures/MCDirection.h>
#include <display/region/Region.qo.h>

namespace casa {
	namespace viewer {
		double degToRad(double d) {
			double r =  M_PI * d / 180.;

			if (r > 0)
				while (r >= 2*M_PI) r -= 2*M_PI;
			else
				while (r < 0) r += 2*M_PI;

			return r;
		}

		double radToDeg(double r) {
			double d = 180. * r / M_PI;

			if (d > 0)
				while(d >= 360) d -= 360.;
			else
				while(d < 0) d += 360.;

			return d;
		}

		double dmsToDegree(int sign, int degree, int min, double sec) {
			// sign is needed because of -00 vs +00
			return double(sign) * (abs(degree) + (min/60.) + (sec/60./60.));
		}

		double parseSEXStr(const char* d) {
			char* dms = strdup(d); // its going to get clobbered
			char* ptr = dms;

			int sign = 1;
			int degree = atoi(strtok(ptr,":"));
			int minute = atoi(strtok(NULL,":"));
			float sec = atof(strtok(NULL,":"));

			// assumes the minus sign is the first char
			if (degree != 0)
				sign = degree>0 ? 1 : -1;
			else
				sign = d[0] == '-' ? -1 : 1;

			free(dms);

			return dmsToDegree(sign,abs(degree),minute,sec);
		}

		double parseHMSStr(const char* str) {
			char* hms = strdup(str); // its going to get clobbered
			char* ptr = hms;

			int sign = 1;
			int hour = atoi(strtok(ptr,"h"));
			int minute = atoi(strtok(NULL,"m"));
			float second = atof(strtok(NULL,"s"));

			// assumes the minus sign is the first char
			if (hour != 0)
				sign = hour>0 ? 1 : -1;
			else
				sign = str[0] == '-' ? -1 : 1;

			free(hms);
			return dmsToDegree(sign,abs(hour),minute,second)/24.*360.;
		}

		double parseDMSStr(const char* str) {
			char* dms = strdup(str); // its going to get clobbered
			char* ptr = dms;

			int sign = 1;
			int degree = atoi(strtok(ptr,"d"));
			int minute = atoi(strtok(NULL,"m"));
			float sec = atof(strtok(NULL,"s"));

			// assumes the minus sign is the first char
			if (degree != 0)
				sign = degree>0 ? 1 : -1;
			else
				sign = str[0] == '-' ? -1 : 1;

			free(dms);
			return dmsToDegree(sign,abs(degree),minute,sec);
		}

		std::vector<double> coordtovec( double *c ) {
			std::vector<double> result(3);
			result[0] = c[0];
			result[1] = c[1];
			result[2] = c[2];
			return result;
		}

		std::vector<double> doubletovec( double x, double y, double z ) {
			std::vector<double> result(3);
			result[0] = x;
			result[1] = y;
			result[2] = z;
			return result;
		}

		static inline double wrap_angle( double before, double after ) {
			const double UNIT_WRAPAROUND = 2.0 * M_PI;
			if ( after < 0 && before > 0 )
				return after + UNIT_WRAPAROUND;
			else if ( after > 0 && before < 0 )
				return after - UNIT_WRAPAROUND;
			return after;
		}

		static inline Quantum<casa::Vector<double> > convert_angle( double x, const std::string &xunits, double y, const std::string &yunits,
		        MDirection::Types original_coordsys, MDirection::Types new_coordsys, const std::string &new_units="rad" ) {
			Quantum<double> xq(x,String(xunits));
			Quantum<double> yq(y,String(yunits));
			MDirection md = MDirection::Convert(MDirection(xq,yq,original_coordsys), new_coordsys)();
			casa::Quantum<casa::Vector<double> > result = md.getAngle("rad");
			xq.convert("rad");
			yq.convert("rad");
			result.getValue( )(0) = wrap_angle(xq.getValue( ), result.getValue( )(0));
			result.getValue( )(1) = wrap_angle(yq.getValue( ), result.getValue( )(1));
			result.convert(String(new_units));
			return result;
		}

		//
		// our "reference format" is the viewer's "linear" coordinate system...
		//
		Vector ds9context::mapToRef(const Vector& v, CoordSystem sys, SkyFrame frame) {
			Vector result(3);

			double wx,wy;
			if ( sys == PHYSICAL ) {
				try {
					pixel_to_linear(wc_,v[0],v[1],wx,wy);
				} catch(...) {
					return result;
				}
			} else if ( sys == WCS ) {
				casa::Vector<double> pts(2);
				pts[0] = v[0];
				pts[1] = v[1];
				casa::Vector<double> pts_rad = Quantum<casa::Vector<double> >(pts,"deg").getValue("rad");
				switch ( frame ) {
				case FK5:
					to_linear(wc_,MDirection::J2000,pts_rad[0],pts_rad[1],wx,wy);
					break;
				case FK4:
					to_linear(wc_,MDirection::B1950,v[0],v[1],wx,wy);
					break;
				case ECLIPTIC:
					to_linear(wc_,MDirection::ECLIPTIC,v[0],v[1],wx,wy);
					break;
				case GALACTIC:
					to_linear(wc_,MDirection::GALACTIC,v[0],v[1],wx,wy);
					break;
				default:
					// report/mark unhandled type
					break;
				}
			} else {
				// report/mark error
			}

			result[0] = wx;
			result[1] = wy;
			result[2] = v[2];
			return result;
		}

		//
		// our "reference format" is the viewer's "linear" coordinate system...
		//
		double ds9context::mapLenToRef(double d, CoordSystem sys, SkyFormat frame) {
			double lx, ly;

			if ( sys == PHYSICAL ) {
				try {
					pixel_offset_to_linear_offset( wc_, d, 0, lx, ly );
				} catch(...) {
					return 0;
				}
			} else if ( sys == WCS ) {
				switch ( frame ) {
				case FK5:
					to_linear_offset(wc_,MDirection::J2000,d,0,lx,ly);
					break;
				case FK4:
					to_linear_offset(wc_,MDirection::B1950,d,0,lx,ly);
					break;
				case ECLIPTIC:
					to_linear_offset(wc_,MDirection::ECLIPTIC,d,0,lx,ly);
					break;
				case GALACTIC:
					to_linear(wc_,MDirection::GALACTIC,d,0,lx,ly);
					break;
				default:
					// report/mark unhandled type
					break;
				}
			} else {
				// report/mark error
			}
			return lx;
		}

		//
		// our "reference format" is the viewer's "linear" coordinate system...
		//
		Vector ds9context::mapLenToRef(const Vector &v, CoordSystem /*sys*/, SkyFormat /*format*/) {
			Vector result(v);
			double lx, ly;

			// find out how to decide what coordinate system the ds9 file uses...

			try {
				pixel_offset_to_linear_offset( wc_, v[0], v[1], lx, ly );
			} catch(...) {
				return result;
			}
			result[0] = lx;
			result[1] = ly;
			return result;
		}


		void ds9context::createBoxCmd( const Vector& center, const Vector& size, double /*angle*/,
		                               const char* color, int* /*dash*/, int /*width*/, const char* font,
		                               const char* text, unsigned short /*prop*/, const char* /*comment*/,
		                               const std::list<Tag>& /*tag*/ ) {

			double xoffset = size[0] / 2.0;
			double yoffset = size[1] / 2.0;

			if ( xoffset > 1.0 || yoffset > 1.0 ) {
				// size is big enough to make a rectangle... perhaps we should require bigger size...
				// 'width' is the line width... need to thread that through...
				const RegionCreator::creator_list_type &rect_creators = RegionCreator::findCreator( region::RectRegion );
				if ( rect_creators.size( ) > 0 && center.size( ) >= 2  && size.size( ) >= 2 ) {
					std::vector<std::pair<double,double> > pts(2);
					pts[0].first  = center[0] - xoffset;
					pts[0].second = center[1] - yoffset;
					pts[1].first  = center[0] + xoffset;
					pts[1].second = center[1] + yoffset;
					rect_creators.front( )->create( region::RectRegion, wc_, pts, text, region::TopText, std::vector<int>(2,0),
					                                font, 11, 0, color, color, region::SolidLine, 1, false, 0 );
				}
			} else {
				const RegionCreator::creator_list_type &rect_creators = RegionCreator::findCreator( region::PointRegion );
				std::vector<std::pair<double,double> > pts(2);
				pts[0].first  = pts[1].first = center[0];
				pts[0].second = pts[1].second = center[1];
				rect_creators.front( )->create( region::PointRegion, wc_, pts, text, region::TopText, std::vector<int>(2,0),
				                                font, 11, 0, color, color, region::SolidLine, 1, false, 0 );
			}

		}

#define DEFINE_POINT_COMMAND(NAME)																					\
void ds9context::create ## NAME ## Cmd( const Vector& center, int /*size*/, const char* color, int* /*dash*/,		\
				    int /*width*/, const char* font, const char* text, unsigned short /*prop*/,						\
				    const char* /*comment*/, const std::list<Tag>& /*tag*/ ) {										\
    const RegionCreator::creator_list_type &point_creators = RegionCreator::findCreator( region::PointRegion );		\
    if ( point_creators.size( ) > 0 && center.size( ) >= 2 ) {														\
	std::vector<std::pair<double,double> > pts(2);																	\
	pts[0].first  = pts[1].first = center[0];																		\
	pts[0].second = pts[1].second = center[1];																		\
	point_creators.front( )->create( region::PointRegion, wc_, pts, text, region::TopText, std::vector<int>(2,0),	\
					 font, 11, 0, color, color, region::SolidLine, 1, false, 0 );									\
    }																												\
}

		DEFINE_POINT_COMMAND(BoxPoint)
		DEFINE_POINT_COMMAND(CirclePoint)
		DEFINE_POINT_COMMAND(DiamondPoint)
		DEFINE_POINT_COMMAND(CrossPoint)
		DEFINE_POINT_COMMAND(ExPoint)
		DEFINE_POINT_COMMAND(ArrowPoint)
		DEFINE_POINT_COMMAND(BoxCirclePoint)

		void ds9context::createEllipseCmd( const Vector& center, const Vector& radius, double /*angle*/,
		                                   const char* color, int* /*dash*/, int /*width*/, const char* font,
		                                   const char* text, unsigned short /*prop*/, const char* /*comment*/,
		                                   const std::list<Tag>& /*tag*/ ) {
			// 'width' is the line width... need to thread that through...
			const RegionCreator::creator_list_type &ellipse_creators = RegionCreator::findCreator( region::EllipseRegion );
			if ( ellipse_creators.size( ) > 0 && center.size( ) >= 2  && radius.size( ) >= 2 ) {
				std::vector<std::pair<double,double> > pts(2);
				double xoffset = radius[0];
				double yoffset = radius[1];
				pts[0].first  = center[0] - xoffset;
				pts[0].second = center[1] - yoffset;
				pts[1].first  = center[0] + xoffset;
				pts[1].second = center[1] + yoffset;
				ellipse_creators.front( )->create( region::EllipseRegion, wc_, pts, text, region::TopText, std::vector<int>(2,0),
				                                   font, 11, 0, color, color, region::SolidLine, 1, false, 0 );
			}
		}

		void ds9context::createCircleCmd( const Vector& center, double radius, const char* color, int* /*dash*/,
		                                  int /*width*/, const char* font, const char* text, unsigned short /*prop*/,
		                                  const char* /*comment*/, const std::list<Tag>& /*tag*/ ) {
			// 'width' is the line width... need to thread that through...
			const RegionCreator::creator_list_type &ellipse_creators = RegionCreator::findCreator( region::EllipseRegion );
			if ( ellipse_creators.size( ) > 0 && center.size( ) >= 2 ) {
				std::vector<std::pair<double,double> > pts(2);
				pts[0].first  = center[0] - radius;
				pts[0].second = center[1] - radius;
				pts[1].first  = center[0] + radius;
				pts[1].second = center[1] + radius;
				ellipse_creators.front( )->create( region::EllipseRegion, wc_, pts, text, region::TopText, std::vector<int>(2,0),
				                                   font, 11, 0, color, color, region::SolidLine, 1, false, 0 );
			}

		}

		void ds9context::createPolygonCmd( const Vector& /*center*/, const Vector& /*bb*/, const char* /*color*/, int* /*dash*/,
		                                   int /*width*/, const char* /*font*/, const char* /*text*/, unsigned short /*prop*/,
		                                   const char* /*comment*/, const std::list<Tag>& /*tag*/ ) { }

		void ds9context::createPolygonCmd( const std::list<Vertex>& verts, const char* color, int* /*dash*/,
		                                   int /*width*/, const char* font, const char* text, unsigned short /*prop*/,
		                                   const char* /*comment*/, const std::list<Tag>& /*tag*/ ) {
			// 'width' is the line width... need to thread that through...
			const RegionCreator::creator_list_type &poly_creators = RegionCreator::findCreator( region::PolyRegion );
			if ( poly_creators.size( ) > 0 && verts.size( ) >= 3 ) {
				int count = 0;
				std::vector<std::pair<double,double> > pts(verts.size( ));
				for ( std::list<Vertex>::const_iterator it=verts.begin( ); it != verts.end(); ++it ) {
					if ( (*it).size( ) < 2 ) return;
					pts[count].first = (*it)[0];
					pts[count++].second = (*it)[1];
				}
				poly_creators.front( )->create( region::PolyRegion, wc_, pts, text, region::TopText, std::vector<int>(2,0),
				                                font, 11, 0, color, color, region::SolidLine, 1, false, 0 );
			}
		}

	}
}
