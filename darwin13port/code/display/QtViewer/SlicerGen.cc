//# SlicerGen.cc: GUI which provides a Slicer object
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

#include <display/QtViewer/SlicerGen.qo.h>
#include <display/Utilities/ImageProperties.h>
#include <algorithm>

namespace casa {
	namespace viewer {

        SlicerGen::SlicerGen( ) : drop_events(false) { }

		void SlicerGen::initialize( QFrame *frame ) {
			setupUi(frame);
			load_startgui( );
			load_sizegui( );
			disable( );
		}

		void SlicerGen::load_startgui( ) {
			startgui.push_back(position01);
			startgui.push_back(position02);
			startgui.push_back(position03);
			startgui.push_back(position04);
			startgui.push_back(position05);
			startgui.push_back(position06);

			axis_map.insert(spin_map_t::value_type(position01,0));
			axis_map.insert(spin_map_t::value_type(position02,1));
			axis_map.insert(spin_map_t::value_type(position03,2));
			axis_map.insert(spin_map_t::value_type(position04,3));
			axis_map.insert(spin_map_t::value_type(position05,4));
			axis_map.insert(spin_map_t::value_type(position06,5));

			connect( position01, SIGNAL(valueChanged(int)), SLOT(adjust_position(int)) );
			connect( position02, SIGNAL(valueChanged(int)), SLOT(adjust_position(int)) );
			connect( position03, SIGNAL(valueChanged(int)), SLOT(adjust_position(int)) );
			connect( position04, SIGNAL(valueChanged(int)), SLOT(adjust_position(int)) );
			connect( position05, SIGNAL(valueChanged(int)), SLOT(adjust_position(int)) );
			connect( position06, SIGNAL(valueChanged(int)), SLOT(adjust_position(int)) );
		}

		void SlicerGen::load_sizegui( ) {
			sizegui.push_back(size01);
			sizegui.push_back(size02);
			sizegui.push_back(size03);
			sizegui.push_back(size04);
			sizegui.push_back(size05);
			sizegui.push_back(size06);

			axis_map.insert(spin_map_t::value_type(size01,0));
			axis_map.insert(spin_map_t::value_type(size02,1));
			axis_map.insert(spin_map_t::value_type(size03,2));
			axis_map.insert(spin_map_t::value_type(size04,3));
			axis_map.insert(spin_map_t::value_type(size05,4));
			axis_map.insert(spin_map_t::value_type(size06,5));

			connect( size01, SIGNAL(valueChanged(int)), SLOT(adjust_size(int)) );
			connect( size02, SIGNAL(valueChanged(int)), SLOT(adjust_size(int)) );
			connect( size03, SIGNAL(valueChanged(int)), SLOT(adjust_size(int)) );
			connect( size04, SIGNAL(valueChanged(int)), SLOT(adjust_size(int)) );
			connect( size05, SIGNAL(valueChanged(int)), SLOT(adjust_size(int)) );
			connect( size06, SIGNAL(valueChanged(int)), SLOT(adjust_size(int)) );
		}

		struct disable_op {
			void operator( )( QSpinBox *s ) {
				s->setMinimum(0);
				s->setMaximum(0);
				s->setEnabled(false);
			}
		};

		void SlicerGen::disable( ) {
            drop_events = true;
			std::for_each( startgui.begin( ), startgui.end( ), disable_op( ) );
			std::for_each( sizegui.begin( ), sizegui.end( ), disable_op( ) );
            drop_events = false;
		}


        void SlicerGen::enable( const std::string &path ) {
            drop_events = true;
            ImageProperties info(path);
            shape.resize(info.shape( ).size( ));
            shape = info.shape( );
            for ( size_t x = 0; x < startgui.size( ); ++x ) {
                if ( x < shape.size( ) ) {
                    startgui[x]->setEnabled(true);
                    startgui[x]->setMinimum(0);
                    startgui[x]->setMaximum(shape[x]-1);
                    startgui[x]->setValue(0);
                    sizegui[x]->setEnabled(true);
                    sizegui[x]->setMinimum(1);
                    sizegui[x]->setMaximum(shape[x]);
                    sizegui[x]->setValue(shape[x]);
                    sizegui[x]->setEnabled(true);
                } else {
                    startgui[x]->hide( );
                    sizegui[x]->hide( );
                }
            }
            drop_events = false;
        }

		void SlicerGen::adjust_position( int value ) {
			QSpinBox *box = dynamic_cast<QSpinBox*>(QObject::sender( ));
			int axis = axis_map[box];
			sizegui[axis]->setMaximum(shape[axis]-value);
            if ( drop_events == false ) emit stateChange(sliceReady( ));
		}
		void SlicerGen::adjust_size(int) {
            if ( drop_events == false ) emit stateChange(sliceReady( ));
		}

		struct equal_op {
			equal_op( int v ) : accumulate(true), size(1), index(0), val(v), mode(VAL) { }
			equal_op( const Vector<int> *v ) : accumulate(true), size(v->size( )), index(0), vec(v), mode(VEC) { }
			operator bool( ) {
				return accumulate;
			}
			void operator( )( QSpinBox *s ) {
				if ( mode == VAL )
					accumulate = accumulate && (s->value( ) == val);
				else {
					if ( index < size )
						accumulate = accumulate && (s->value( ) == (*vec)[index]);
					++index;
				}
			}
		private:
			bool accumulate;
			enum mode_t { VAL, VEC };
			int size;
			int index;
			union {
				int val;
				const Vector<int> *vec;
			};
			mode_t mode;
		};

		bool SlicerGen::sliceReady( ) const {
			bool resultA = std::for_each( startgui.begin( ), startgui.end( ), equal_op(0) );
			bool resultB1 = std::for_each( sizegui.begin( ), sizegui.end( ), equal_op(&shape) );
			bool resultB2 = std::for_each( sizegui.begin( ), sizegui.end( ), equal_op(0) );
			return ! (resultA && (resultB1 || resultB2));
		}
		std::string SlicerGen::getSliceRep( ) const {
			std::string result("(");
			std::ostringstream out;
			int size = shape.size( );
			for ( int i=0; i < size; ++i ) {
				out << startgui[i]->value( ) << ":" << sizegui[i]->value( );
				result = result + out.str( );
				if ( i != size-1 ) result = result + ",";
				out.str("");
				out.clear( );
			}
			result = result + ")";
			return result;
		}
	}
}
