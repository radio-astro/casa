//# CasaColormapGen.h: produce the values in a CASA colormap as a stream.
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
//# $Id$

#if ! defined(_GRAPHPAD_CASACOLORMAPGEN_H_)
#define _GRAPHPAD_CASACOLORMAPGEN_H_
#include <tables/Tables/Table.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/ArrayColumn.h>
#include <stdexcept>
#include <limits>

namespace graphpad {

    template<typename Pixel>
    class CasaColormapGen {
	public:
	    CasaColormapGen( const std::string &table_path ) : initialized_(false), path_(table_path) { }
	    void begin( ) {
		try {
		    colormap_table = casa::Table(path_);
		} catch (const casa::AipsError &x) {
		    throw std::invalid_argument( "failed to open table:" + path_ );
		}
		num_cmaps_ = colormap_table.nrow( );
		synonym_col_.attach(colormap_table, "SYNONYMS");
		name_col_.attach(colormap_table, "CMAP_NAME");
		red_col_.attach(colormap_table, "RED");
		green_col_.attach(colormap_table, "GREEN");
		blue_col_.attach(colormap_table, "BLUE");

		column_offset_ = 0;
		pixel_offset_ = 0;

		name_ = name_col_(column_offset_);
		synonyms_ = generate_vector(synonym_col_(column_offset_));
		red_ = red_col_(column_offset_);
		green_ = green_col_(column_offset_);
		blue_ = blue_col_(column_offset_);
		current_len_ = red_.nelements() < green_.nelements() ? red_.nelements() : green_.nelements();
		current_len_ = current_len_ < blue_.nelements() ? current_len_ : blue_.nelements( );

		initialized_ = true;
	    }

	    CasaColormapGen( const CasaColormapGen &o ) : initialized_(o.initialized_), path_(o.path_),
						colormap_table(o.colormap_table), num_cmaps_(o.num_cmaps_),
						current_len_(o.current_len_), column_offset_(o.column_offset_),
						pixel_offset_(o.pixel_offset_), red_(o.red_),
						green_(o.green_), blue_(o.blue_) {
		synonym_col_.reference(o.synonym_col_);
		name_col_.reference(o.name_col_);
		red_col_.reference(o.red_col_);
		green_col_.reference(o.green_col_);
		blue_col_.reference(o.blue_col_);
	    }

	    virtual ~CasaColormapGen( ) { }

	    Pixel operator( )( ) {
		typedef typename Pixel::value_type value_type;
		// casa colormap entries go from 0.0 to 1.0...
		const float max_val = (float) std::numeric_limits<value_type>::max( );
		Pixel result( (value_type) (red_[pixel_offset_] * max_val),
			      (value_type) (green_[pixel_offset_] * max_val),
			      (value_type) (blue_[pixel_offset_] * max_val) );
		pixel_offset_ += 1;
		return result;
	    }

	    size_t size( ) const { return current_len_; }

	    size_t numColormaps( ) const { return num_cmaps_; }
	    void nextColormap( ) {
		// wrap to prevent access errors when looping, e.g.
		// for (size_t i=0; i < cmapgen.numColormaps(); ++i, cmapgen.nextColormap()) {...}
		column_offset_ = (column_offset_ + 1) % num_cmaps_;
		name_ = name_col_(column_offset_);
		synonyms_ = generate_vector(synonym_col_(column_offset_));
		red_.resize(0);
		red_ = red_col_(column_offset_);
		green_.resize(0);
		green_ = green_col_(column_offset_);
		blue_.resize(0);
		blue_ = blue_col_(column_offset_);
		current_len_ = red_.nelements() < green_.nelements() ? red_.nelements() : green_.nelements();
		current_len_ = current_len_ < blue_.nelements() ? current_len_ : blue_.nelements( );
		pixel_offset_ = 0;
	    }

	    std::string name( ) const { return name_; }
	    std::vector<std::string> synonyms( ) const { return synonyms_; }

	private:
	    static inline std::vector<std::string> generate_vector( const casa::Vector<casa::String> &o ) {
		std::vector<std::string> result(o.size());
		for ( size_t i = 0; i < result.size(); ++i ) result[i] = o[i];
		return result;
	    }

	    bool initialized_;
	    std::string path_;
	    casa::Table colormap_table;
	    size_t num_cmaps_;
	    size_t current_len_;
	    
	    size_t column_offset_;
	    casa::ROArrayColumn<casa::String> synonym_col_;
	    casa::ROScalarColumn<casa::String> name_col_;
	    casa::ROArrayColumn<casa::Float> red_col_;
	    casa::ROArrayColumn<casa::Float> green_col_;
	    casa::ROArrayColumn<casa::Float> blue_col_;

	    std::string name_;
	    std::vector<std::string> synonyms_;
	    size_t pixel_offset_;
	    casa::Vector<casa::Float> red_;
	    casa::Vector<casa::Float> green_;
	    casa::Vector<casa::Float> blue_;
    };
}

#endif
