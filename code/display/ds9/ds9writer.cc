//# ds9writer.cc: class used for writing ds9 region files
//# Copyright (C) 2012
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

#include <string.h>
#include <stdlib.h>
#include <display/region/Region.qo.h>
#include <display/ds9/ds9writer.h>
#include <display/DisplayErrors.h>
#include <QFileInfo>

namespace casa {
	namespace viewer {

		ds9writer::ds9writer( const char *output_path, const char *coord_sys ) : path(strdup(output_path)), csys(strdup(coord_sys)),
			csys_file_path(0), opened(false), fptr(0) {
			// setup global defaults:
			//    "global color=green dashlist=8 3 width=1 font=\"helvetica 10 normal roman\" select=1 highlite=1 dash=0 fixed=0 edit=1 move=1 delete=1 include=1 source=1"
			defaults.insert(str_map_type::value_type("color","green"));
			defaults.insert(str_map_type::value_type("dashlist","8 3"));
			defaults.insert(str_map_type::value_type("width","1"));
			defaults.insert(str_map_type::value_type("font","\"helvetica 10 normal roman\""));
			defaults.insert(str_map_type::value_type("select","1"));
			defaults.insert(str_map_type::value_type("highlite","1"));
			defaults.insert(str_map_type::value_type("dash","0"));
			defaults.insert(str_map_type::value_type("fixed","0"));
			defaults.insert(str_map_type::value_type("edit","1"));
			defaults.insert(str_map_type::value_type("move","1"));
			defaults.insert(str_map_type::value_type("delete","1"));
			defaults.insert(str_map_type::value_type("include","1"));
			defaults.insert(str_map_type::value_type("source","1"));

			coord_systems.insert(coord_map_type::value_type("pixel",cs("physical",MDirection::EXTRA,linear_to_pixel,linear_to_pixel)));
			coord_systems.insert(coord_map_type::value_type("j2000",cs("fk5",MDirection::J2000,linear_to_j2000,linear_to_j2000)));
			coord_systems.insert(coord_map_type::value_type("b1950",cs("fk4",MDirection::B1950,linear_to_b1950,linear_to_b1950)));
			coord_systems.insert(coord_map_type::value_type("galactic",cs("galactic",MDirection::GALACTIC,linear_to_pixel,linear_to_pixel)));
			coord_systems.insert(coord_map_type::value_type("ecliptic",cs("ecliptic",MDirection::ECLIPTIC,linear_to_pixel,linear_to_pixel)));
		}

		ds9writer::~ds9writer( ) {
			if ( fptr && opened )
				fclose(fptr);
			free(path);
			free(csys);
			if ( csys_file_path )
				free(csys_file_path);
		}

		void ds9writer::setCsysSource(const char *path) {
			if ( path && strlen(path) > 0 && csys_file_path == 0 )
				csys_file_path = strdup(path);
		}

		bool ds9writer::open( ) {
			// separating open( ) from construction allows for
			// (future) global default changes...

			fptr = fopen( path, "w" );
			if ( fptr == 0 ) {
				return false;
			}
			fprintf( fptr, "# Region file format: DS9 version 4.1\n" );

			// include image/fits path... if available...
			if ( csys_file_path ) {
				QFileInfo fi(csys_file_path);
				QString full_path = fi.canonicalFilePath( );
				if ( full_path != "" )
					fprintf( fptr, "# Filename: %s\n", full_path.toAscii( ).constData( ) );
			}

			fprintf( fptr, "global " );
			str_map_type::iterator prev, cur = defaults.begin( );
			for ( prev = cur++; cur != defaults.end( ); ++cur ) {
				fprintf( fptr, "%s=%s ", prev->first.c_str(), prev->second.c_str() );
				prev = cur;
			}
			fprintf( fptr, "%s=%s\n", prev->first.c_str(), prev->second.c_str() );

			coord_map_type::iterator it = coord_systems.find(csys);
			if ( it == coord_systems.end( ) ) {
				char buf[4096];
				sprintf( buf, "unknown coordinate system: %s", csys );
				throw internal_error(buf);
			}
			fprintf( fptr, "%s\n", it->second.ds9.c_str( ) );

			// flag as opened...
			opened = true;

			return true;

		}

		bool ds9writer::rectangle( WorldCanvas *wc, const std::vector<std::pair<double,double> > &pts ) {
			if ( pts.size( ) != 2 ) return false;
			coord_map_type::iterator it = coord_systems.find(csys);

			if ( it == coord_systems.end( ) ) return false;
			if ( opened == false ) {
				open( );
			}

			Vector<double> blc_rad(2);
			Vector<double> trc_rad(2);

			it->second.cvt4( wc, pts[0].first, pts[0].second, pts[1].first, pts[1].second, blc_rad[0], blc_rad[1], trc_rad[0], trc_rad[1] );

			if ( it->second.type == MDirection::EXTRA ) {
				// this (EXTRA) means "pixel" to us...
				fprintf( fptr, "box(%0.2f,%0.2f,%0.2f,%0.2f,0)\n",
				         (blc_rad[0]+trc_rad[0])/2.0, (blc_rad[1]+trc_rad[1])/2.0,
				         fabs(trc_rad[0]-blc_rad[0]),fabs(trc_rad[1]-blc_rad[1]) );
			} else {
				Quantum<Vector<double> > blcq(blc_rad,"rad");
				Quantum<Vector<double> > trcq(trc_rad,"rad");
				Vector<double> blc_deg = blcq.getValue("deg");
				Vector<double> trc_deg = trcq.getValue("deg");

				Quantum<Vector<double> > diffq(trc_rad-blc_rad,"rad");
				Vector<double> lengths_arcsec = diffq.getValue("arcsec");
				fprintf( fptr, "box(%f,%f,%0.2f\",%0.2f\",0)\n",
				         (blc_deg[0]+trc_deg[0])/2.0, (blc_deg[1]+trc_deg[1])/2.0,
				         fabs(lengths_arcsec[0]), fabs(lengths_arcsec[1]) );
			}

			return true;
		}

		bool ds9writer::ellipse( WorldCanvas *wc, const std::vector<std::pair<double,double> > &pts ) {
			if ( pts.size( ) != 2 ) return false;
			coord_map_type::iterator it = coord_systems.find(csys);

			if ( it == coord_systems.end( ) ) return false;
			if ( opened == false ) {
				open( );
			}

			Vector<double> blc_rad(2);
			Vector<double> trc_rad(2);

			it->second.cvt4( wc, pts[0].first, pts[0].second, pts[1].first, pts[1].second, blc_rad[0], blc_rad[1], trc_rad[0], trc_rad[1] );

			if ( it->second.type == MDirection::EXTRA ) {
				// this (EXTRA) means "pixel" to us...
				fprintf( fptr, "ellipse(%0.2f,%0.2f,%0.2f,%0.2f,0)\n",
				         (blc_rad[0]+trc_rad[0])/2.0, (blc_rad[1]+trc_rad[1])/2.0,
				         fabs(trc_rad[0]-blc_rad[0])/2.0,fabs(trc_rad[1]-blc_rad[1])/2.0 );
			} else {
				Quantum<Vector<double> > blcq(blc_rad,"rad");
				Quantum<Vector<double> > trcq(trc_rad,"rad");
				Vector<double> blc_deg = blcq.getValue("deg");
				Vector<double> trc_deg = trcq.getValue("deg");

				Quantum<Vector<double> > diffq(trc_rad-blc_rad,"rad");
				Vector<double> lengths_arcsec = diffq.getValue("arcsec");
				fprintf( fptr, "ellipse(%f,%f,%0.2f\",%0.2f\",0)\n",
				         (blc_deg[0]+trc_deg[0])/2.0, (blc_deg[1]+trc_deg[1])/2.0,
				         fabs(lengths_arcsec[0]/2.0), fabs(lengths_arcsec[1]/2.0) );
			}

			return true;
		}

		bool ds9writer::polygon( WorldCanvas *wc, const std::vector<std::pair<double,double> > &pts ) {
			if ( pts.size( ) < 3 ) return false;
			coord_map_type::iterator it = coord_systems.find(csys);

			if ( it == coord_systems.end( ) ) return false;
			if ( opened == false ) {
				open( );
			}

			Vector<double> xpt(2);

			fprintf( fptr, "polygon(" );
			if ( it->second.type == MDirection::EXTRA ) {
				// this (EXTRA) means "pixel" to us...
				for ( int i=0; i < static_cast<int>(pts.size()); ++i ) {
					it->second.cvt2( wc, pts[i].first, pts[i].second, xpt[0], xpt[1] );
					fprintf( fptr, "%0.2f,%0.2f%s", xpt[0], xpt[1], (i == static_cast<int>(pts.size()-1) ? "" : ",") );
				}

			} else {
				for ( int i=0; i < static_cast<int>(pts.size()); ++i ) {
					it->second.cvt2( wc, pts[i].first, pts[i].second, xpt[0], xpt[1] );
					Vector<double> degpt = Quantum<Vector<double> >(xpt,"rad").getValue("deg");
					fprintf( fptr, "%f,%f%s", degpt[0], degpt[1], (i == static_cast<int>(pts.size()-1) ? "" : ",") );
				}
			}
			fprintf( fptr, ")\n");

			return true;
		}


		bool ds9writer::polyline( WorldCanvas *wc, const std::vector<std::pair<double,double> > &pts ) {
			if ( pts.size( ) < 2 ) return false;
			coord_map_type::iterator it = coord_systems.find(csys);

			if ( it == coord_systems.end( ) ) return false;
			if ( opened == false ) {
				open( );
			}

			Vector<double> xpt(2);

			fprintf( fptr, "polyline(" );
			if ( it->second.type == MDirection::EXTRA ) {
				// this (EXTRA) means "pixel" to us...
				for ( int i=0; i < static_cast<int>(pts.size()); ++i ) {
					it->second.cvt2( wc, pts[i].first, pts[i].second, xpt[0], xpt[1] );
					fprintf( fptr, "%0.2f,%0.2f%s", xpt[0], xpt[1], (i == static_cast<int>(pts.size()-1) ? "" : ",") );
				}

			} else {
				for ( int i=0; i < static_cast<int>(pts.size()); ++i ) {
					it->second.cvt2( wc, pts[i].first, pts[i].second, xpt[0], xpt[1] );
					Vector<double> degpt = Quantum<Vector<double> >(xpt,"rad").getValue("deg");
					fprintf( fptr, "%f,%f%s", degpt[0], degpt[1], (i == static_cast<int>(pts.size()-1) ? "" : ",") );
				}
			}
			fprintf( fptr, ")\n");

			return true;
		}


		bool ds9writer::point( WorldCanvas *wc, const std::vector<std::pair<double,double> > &pts ) {
			if ( pts.size( ) != 2 ) return false;
			coord_map_type::iterator it = coord_systems.find(csys);

			if ( it == coord_systems.end( ) ) return false;
			if ( opened == false ) {
				open( );
			}

			Vector<double> pt(2);

			it->second.cvt2( wc, pts[0].first, pts[0].second, pt[0], pt[1] );

			if ( it->second.type == MDirection::EXTRA ) {
				// this (EXTRA) means "pixel" to us...
				fprintf( fptr, "point(%0.2f,%0.2f)\n", pt[0], pt[1] );
			} else {
				Vector<double> degpt = Quantum<Vector<double> >(pt,"rad").getValue("deg");
				fprintf( fptr, "point(%f,%f)\n", degpt[0], degpt[1] );
			}

			return true;
		}
	}
}


