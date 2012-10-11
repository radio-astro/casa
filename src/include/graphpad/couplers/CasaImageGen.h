//# CasaImageGen.h: produce the values in a CASA image as a stream.
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

#if ! defined(_GRAPHPAD_CASAIMAGEGEN_H_)
#define _GRAPHPAD_CASAIMAGEGEN_H_
#include <stdexcept>
#include <graphpad/Image.h>
#include <images/Images/ImageOpener.h>
#include <images/Images/PagedImage.h>
#include <images/Images/FITSImage.h>
#include <images/Images/MIRIADImage.h>
#include <images/Images/ImageStatistics.h>

namespace graphpad {

    inline casa::Coordinate::Type axis_to_coordinate( PlotAxes x ) {
	return (x == RA || x == DEC) ? casa::Coordinate::DIRECTION :
	       (x == FREQ || x == VELO) ? casa::Coordinate::SPECTRAL :
	       casa::Coordinate::STOKES;
    }


    template<typename Value>
    class CasaImageGen {
	public:
	    typedef Value return_type;
	    CasaImageGen( const std::string &p ) : initialized_(false), path_(p), image_(0), minmax_initialized_(false) { }

	    // xaxis => width
	    // yaxis => height
	    // zaxis => iterator
	    // hxais => hidden axis
	    void begin( PlotAxes xaxis, PlotAxes yaxis, PlotAxes zaxis=NOAXIS ) {
		if ( image_ ) delete image_;
		switch ( casa::ImageOpener::imageType(path_) ) {
		    case casa::ImageOpener::AIPSPP:
			if ( casa::imagePixelType(path_) != casa::ValType::getType((Value*)0) )
			    throw std::invalid_argument( path_ + " does not have the right type of pixel" );
			image_ = new casa::PagedImage<Value>(path_, casa::TableLock::AutoNoReadLocking);
			break;
		    case casa::ImageOpener::FITS:
			if ( casa::ValType::getType((Value*)0) != casa::TpFloat )
			    // probably this will fail to compile if the types don't line up...
			    throw std::invalid_argument( path_ + " does not have the right type of pixel" );
			image_ = new casa::FITSImage(path_);
			break;
		    case casa::ImageOpener::MIRIAD:
			if ( casa::ValType::getType((Value*)0) != casa::TpFloat )
			    // probably this will fail to compile if the types don't line up...
			    throw std::invalid_argument( path_ + " does not have the right type of pixel" );
			image_ = new casa::MIRIADImage(path_);
			break;
		    default:
			throw std::invalid_argument( path_ + " is of unknown type" );
		}

		if ( image_->ok( ) == false )
		    throw std::invalid_argument( "opening " + path_ + " failed" );

		imgShape_ = image_->shape( );
		axisPath_.resize(imgShape_.size( ));
		const ssize_t empty = axisPath_.size( ) + 1;
		axisPath_ = empty;
		size_t axes_set = 0;
		PlotAxes order[3] = { xaxis, yaxis, zaxis };
		const casa::CoordinateSystem& cs = image_->coordinates();
		for ( size_t axis=0; axis < 3; ++axis ) {
		    switch ( order[axis] ) {
			case RA:
			case DEC:
			  { int index = cs.findCoordinate(axis_to_coordinate(order[axis]));
			    if ( index == -1 ) continue;
			    casa::Vector<int> pix = cs.pixelAxes(index);
			    if ( pix.size( ) != 2 ) continue;
			    axisPath_[(order[axis] == RA ? pix[0] : pix[1])] = axes_set++;
			  } break;
			case FREQ:
			case VELO:
			case STOKES:
			  { int index = cs.findCoordinate(axis_to_coordinate(order[axis]));
			    if ( index == -1 ) continue;
			    casa::Vector<int> pix = cs.pixelAxes(index);
			    if ( pix.size( ) != 1 ) continue;
			    axisPath_[pix[0]] = axes_set++;
			  } break;
			default:
			  break;
		    }
		}

		for( ssize_t t=0; t < axisPath_.size( ); ++t ) if ( axisPath_[t] == empty ) axisPath_[t] = axes_set++;
	    }

	    // select one offset along the zaxis
	    void select( size_t channel ) {
		casa::IPosition start(imgShape_.size(),0);
		start[axisPath_[2]] = channel;
		casa::IPosition cursor(imgShape_.size(),1);
		cursor[axisPath_[0]] = imgShape_[axisPath_[0]];
		cursor[axisPath_[1]] = imgShape_[axisPath_[1]];

		image_->getSlice( buffer_, start, cursor, true );
		iter_ = buffer_.begin( );
		iter_end_ = buffer_.end( );

		initialized_ = true;
	    }


	    // serve values from bottom-left-corner to top-right-corner...
	    Value operator( )( ) {
		if ( initialized_ == false ) throw std::runtime_error( "incomplete initialization..." );
		Value result = *iter_;
		if ( ++iter_ == iter_end_ )
		    iter_ = buffer_.begin( );
		return result;
	    }

	    const std::vector<int> shape( ) const {
		std::vector<int> result(2);
		result[0] = imgShape_[axisPath_[0]];
		result[1] = imgShape_[axisPath_[1]];
		return result;
	    }

	    const int planes( ) const {
		return imgShape_[axisPath_[2]];
	    }

	    std::pair<Value,Value> getMinMax( ) {
		if ( minmax_initialized_ == false ) {
		    casa::LatticeStatistics<Value> stats(*image_);
		    if ( stats.getFullMinMax( minmax_.first, minmax_.second ) == false ) {
			// no unmasked data... perhaps should throw an exception instead of stubbing out the values...
			minmax_.first = -1;
			minmax_.second = 1;
		    }
		    minmax_initialized_ = true;
		}
		return minmax_;
	    }

	    std::pair<std::vector<double>,std::vector<double> > getWorldRange( ) {
		const casa::CoordinateSystem &csys = image_->coordinates( );
		std::pair<std::vector<double>,std::vector<double> > result;
		casa::IPosition minp(imgShape_.size(),0);
		casa::IPosition maxp(imgShape_.size(),0);
		maxp[axisPath_[0]] = imgShape_[axisPath_[0]];
		maxp[axisPath_[1]] = imgShape_[axisPath_[1]];
		casa::Vector<double> minw;
		if ( csys.toWorld( minw, minp ) == false ) return result;
		casa::Vector<double> maxw;
		if ( csys.toWorld( maxw, maxp ) == false ) return result;
		result.first.push_back(minw[axisPath_[0]]);
		result.first.push_back(minw[axisPath_[1]]);
		result.second.push_back(maxw[axisPath_[0]]);
		result.second.push_back(maxw[axisPath_[1]]);
		return result;
	    }

	private:
	    bool initialized_;
	    std::string path_;
	    casa::ImageInterface<Value> *image_;
	    // iteration path through the lattice...
	    // also index offsets... i.e. our primary axis is of shape imgShape_[axisPath_[0]]
	    casa::IPosition axisPath_;
	    casa::IPosition imgShape_;
	    casa::Array<Value> buffer_;
	    typename casa::Array<Value>::iterator iter_;
	    typename casa::Array<Value>::iterator iter_end_;
	    ssize_t xIndex_;
	    ssize_t yIndex_;
	    bool minmax_initialized_;
	    std::pair<Value,Value> minmax_;
    };
}

#endif
