//# IterationClient.qo.h: manage iteration of QtDisplayData lattices
//# with surrounding Gui functionality
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
//# $Id: QtDisplayPanelGui.qo.h,v 1.7 2006/10/10 21:42:05 dking Exp $

#ifndef QTITERATIONCLIENT_H
#define QTITERATIONCLIENT_H
#include <QObject>
#include <display/region/Region.h> // viewer exceptions should be moved to a separate file
#include <casa/Quanta/Unit.h>
#include <display/DisplayDatas/CsysAxisInfo.h>

namespace casa {
    class ImageAnalysis;
    class QtDisplayData;
    class QtDisplayPanelGui;
    namespace viewer {
	class IterationManager;
	class IterationClient : public QObject {
	  Q_OBJECT;	//# Allows slot/signal definition.  Must only occur in
			//# implement/.../*.h files; also, makefile must include
			//# name of this file in 'mocs' section.

	    public:
		// invalid client...
		IterationClient( ) : mgr(0), data(0), ia(0) { }
		IterationClient( IterationManager *m, QtDisplayData *dd );
		~IterationClient( );

		std::string name( ) const;
		std::string dataType( ) const;
		std::string displayType( ) const;
		void locateFrame( unsigned int movie_axis, IPosition &start );

		ImageAnalysis *imageAnalysis( );
		ImageInterface<float> *imageInterface( );
		const ImageInterface<float> *imageInterface( ) const
			{ return ((IterationClient*)this)->imageInterface( ); }
		// used to retrieve the state of the movie axis of one cube and
		// use it to match the reference vector of another cube...
		CsysAxisInfo movieAxisInfo( unsigned int pix_axis ) const;

		// returns a stdc++ container whose elements can be
		// constructed from a double...
		template <class CONTAINER>
		  CONTAINER refvector( unsigned int pix_axis );

		template <class _OutputIterator>
		  _OutputIterator refvector( unsigned int pix_axis, _OutputIterator it );

	    private:
		IterationManager *mgr;
		ImageAnalysis *ia;
		QtDisplayData *data;
	};



	template <class CONTAINER> CONTAINER IterationClient::refvector( unsigned int pix_axis ) {
	    CsysAxisInfo info(movieAxisInfo(pix_axis));
	    CONTAINER vec;

	    const Coordinate &coord = info.coord( );
	    if ( info.type( ) == CsysAxisInfo::VELO ) {
		// do spectral velocity conversion...
		try {
		    const SpectralCoordinate &sc = dynamic_cast<const SpectralCoordinate&>(coord);
		    Quantum<Double> velo;
		    for ( unsigned int i=0; i < info.im_axis_size(); ++i ) {
			sc.pixelToVelocity(velo,i);
			vec.push_back(velo.getValue( ));
		    }
		} catch(...) {
		    throw internal_error("velocity conversion failed");
		}
	    } else {
		// get reference values along the axis...
		Vector<Double> pixel_value(info.ref_pixel(false));
		Vector<Double> world_value(info.world_size( ));
		for ( unsigned int i=0; i < info.im_axis_size(); ++i ) {
		    pixel_value[info.world_index( )] = i;
		    coord.toWorld(world_value,pixel_value);
		    vec.push_back(world_value[info.world_index( )]);
		}

	    }
	    return vec;
	}

	//==========================================================================================
	template <class _OutputIterator> _OutputIterator IterationClient::refvector( unsigned int pix_axis, _OutputIterator it ) {
	    CsysAxisInfo info(movieAxisInfo(pix_axis));

	    const Coordinate &coord = info.coord( );
	    if ( info.type( ) == CsysAxisInfo::VELO ) {
		// do spectral velocity conversion...
		try {
		    const SpectralCoordinate &sc = dynamic_cast<const SpectralCoordinate&>(coord);
		    Quantum<Double> velo;
		    for ( unsigned int i=0; i < info.im_axis_size(); ++i ) {
			sc.pixelToVelocity(velo,i);
			*it++ = velo.getValue( );
		    }
		} catch(...) {
		    throw internal_error("velocity conversion failed");
		}
	    } else {
		// get reference values along the axis...
		Vector<Double> pixel_value(info.ref_pixel(false));
		Vector<Double> world_value(info.world_size( ));
		for ( unsigned int i=0; i < info.im_axis_size(); ++i ) {
		    pixel_value[info.world_index( )] = i;
		    coord.toWorld(world_value,pixel_value);
		    *it++ = world_value[info.world_index( )];
		}

	    }
	    return it;
	}

    }
}

#endif
