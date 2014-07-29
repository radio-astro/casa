//# ds9writer.h: class used for writing ds9 region files
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

#ifndef DISPLAY_DS9_DS9WRITER_H_
#define DISPLAY_DS9_DS9WRITER_H_
#include <stdio.h>

namespace casa {

	class WorldCanvas;

	namespace viewer {

		class ds9writer {
		public:
			ds9writer( const char *output_path, const char *coord_sys );
			virtual ~ds9writer( );
			void setCsysSource(const char *);
			bool open( );
			bool rectangle( WorldCanvas *, const std::vector<std::pair<double,double> > &pts );
			bool ellipse( WorldCanvas *, const std::vector<std::pair<double,double> > &pts );
			bool polygon( WorldCanvas *, const std::vector<std::pair<double,double> > &pts );
			bool polyline( WorldCanvas *, const std::vector<std::pair<double,double> > &pts );
			bool point( WorldCanvas *, const std::vector<std::pair<double,double> > &pts );
		private:
			typedef std::map<std::string,std::string> str_map_type;
			str_map_type defaults;

			struct cs {
				typedef void (*from_linear_2_type)(WorldCanvas*wc,double,double,double&,double&);
				typedef void (*from_linear_4_type)(WorldCanvas*wc,double,double,double,double,double&,double&,double&,double&);
				cs( const char *ds9_type, MDirection::Types t,
				    from_linear_2_type func2, from_linear_4_type func4 ) : ds9(ds9_type), type(t), cvt2(func2), cvt4(func4) { }
				~cs( ) { }
				const std::string ds9;
				const MDirection::Types type;
				from_linear_2_type cvt2;
				from_linear_4_type cvt4;
			};
			typedef std::map<std::string,cs> coord_map_type;
			coord_map_type coord_systems;
			char *path;
			char *csys;
			char *csys_file_path;
			bool opened;
			FILE *fptr;
		};
	}
}

#endif
