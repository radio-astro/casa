#include <iostream>
#include <display/DisplayDatas/CsysAxisInfo.h>
#include <display/DisplayDatas/DisplayData.h>
#include <display/DisplayDatas/PrincipalAxesDD.h>
#include <display/region/Region.h>		// viewer exceptions should be move to their own header files

namespace casa {
    namespace viewer {

	CsysAxisInfo::CsysAxisInfo( DisplayData *dd, unsigned int pix_axis ) : type_(ERROR),
		       axis_(pix_axis), coord_index_(0), world_index_(0), world_size_(0),
		       ncoordinates_(0), c_sys_(0) {

	    if ( dd == 0 ) throw internal_error( "null display data" );
	    ImageInterface<Float> *image = dd->imageinterface( );
	    if ( image == 0 ) throw internal_error( "display data has no image interface" );
	    const CoordinateSystem &csys = image->coordinates( );
	    c_sys_ = &csys;
	    ncoordinates_ = csys.nCoordinates( );
	    csys_reference_pixel_.resize(csys.referencePixel( ).size());
	    csys_reference_pixel_ = csys.referencePixel( );
	    IPosition shape = image->shape( );
	    if ( pix_axis >= shape.size( ) ) throw internal_error( "pixel axis exceeds image shape" );
	    image_axis_size_ = shape[pix_axis];

	    bool ok = false;
	    // setup coordinate system info...
	    for ( unsigned int i=0; i < csys.nCoordinates( ); ++i ) {
		Vector<Int> windexes = csys.worldAxes(i);
		for ( unsigned int x=0; x < windexes.size( ); ++x )
		    if ( pix_axis == windexes[x] ) {
			ok = true;
			coord_index_ = i;
			world_index_ = x;
			world_size_ = windexes.nelements( );
			break;
		    }
	    }

	    // setup failed...
	    if ( ok == false ) {
		char buf[128];
		sprintf( buf, "could not match pixel index (%u) with coordinate [%u] + world axes", pix_axis, csys.nCoordinates( ) );
		throw internal_error( buf );
	    }

	    // setup units as displayed by the viewer... (I think spectral
	    // coordinates are the only ones that require special treatment)
	    PrincipalAxesDD *padd = 0;
	    const Coordinate &coord = csys.coordinate(coord_index_);
	    c_reference_pixel_.resize(coord.referencePixel( ).size());
	    c_reference_pixel_ = coord.referencePixel( );
	    if ( coord.type() == Coordinate::SPECTRAL && (padd = dynamic_cast<PrincipalAxesDD*>(dd)) ) {
		units_ = Unit(padd->spectralunitStr( ));
		type_ = VELO;
	    } else {
		units_ = Unit(coord.worldAxisUnits( )[world_index_]);
		type_ = REFVAL;
	    }
	}
	  
	const Coordinate &CsysAxisInfo::coord( ) const {
	    if ( c_sys_ == 0 ) throw internal_error( "null coordinate system" );
	    return c_sys_->coordinate(coord_index( ));
	}

	const CoordinateSystem &CsysAxisInfo::coordsys( ) const {
	    if ( c_sys_ == 0 ) throw internal_error( "null coordinate system" );
	    return *c_sys_;
	}

	std::ostream &operator<<( std::ostream &out, const CsysAxisInfo &ainfo ) {
	    out << "csys-axis(" << ainfo.axis( ) << ": ";
	    for ( unsigned int ci=0; ci < ainfo.csys_size( ); ++ci ) {
		if ( ci == ainfo.coord_index( ) ) {
		    out << "[";
		    for ( unsigned int wi=0; wi < ainfo.world_size( ); ++wi ) {
			if ( wi == ainfo.world_index( ) ) 
			    out << ainfo.units( ).getName( ) << "{" << ainfo.im_axis_size( ) << "}";
			else 
			    out << "0";
			if ( wi != (ainfo.world_size( ) - 1) ) out << " ";
		    }
		    out << "]";
		} else {
		    out << "0";
		}
		if ( ci != (ainfo.csys_size( ) - 1) ) out << " ";
	    }
	    out << ")";
	    return out;
	}

    }
}
