#ifndef VIEWER_CSYSAXISINFO_H_
#define VIEWER_CSYSAXISINFO_H_
#include <casa/Arrays/Vector.h>
#include <casa/Quanta/Unit.h>


namespace casa {
    class Coordinate;
    class DisplayData;
    class CoordinateSystem;
    namespace viewer {

	class CsysAxisInfo {
	    public:
		enum RefType { VELO, REFVAL, ERROR };
		CsysAxisInfo( DisplayData *, unsigned int pix_axis );
		CsysAxisInfo( const CsysAxisInfo &o ) : units_(o.units_), type_(o.type_), axis_(o.axis_),
						coord_index_(o.coord_index_), world_index_(o.world_index_),
						world_size_(o.world_size_), ncoordinates_(o.ncoordinates_),
						image_axis_size_(o.image_axis_size_) { }

		CsysAxisInfo( ) : type_(ERROR) { }
		const Unit &units( ) const { return units_; }
		unsigned int axis( ) const { return axis_; }
		unsigned int coord_index( ) const { return coord_index_; }
		unsigned int world_index( ) const { return world_index_; }
		unsigned int world_size( ) const { return world_size_; }
		unsigned int csys_size( ) const { return ncoordinates_; }
		unsigned int im_axis_size( ) const { return image_axis_size_; }

		RefType type( ) const { return type_; }

		const Coordinate &coord( ) const;
		const CoordinateSystem &coordsys( ) const;

		const Vector<Double> &ref_pixel( bool csys=true ) const
			{ return csys ? csys_reference_pixel_ : c_reference_pixel_; }

	    protected:
		// it seems like there are no good solutions here...
		// copying a coordinate system seems like a lengthy process...
		// but if we hold onto these objects too long, this reference will get stale...
		const CoordinateSystem *c_sys_;

		Unit units_;
		RefType type_;
		// pixel axis
		unsigned int axis_;
		// index of the coordinate for this axis in the coordinate system
		unsigned int coord_index_;
		// index of the particular pixel axis within the world axes of the coordinate
		unsigned int world_index_;
		// number of world axes that the coordinate for this axis contains, e.g.
		// the direction coordinate of the coordinate system for the viewer typically
		// has two world axes (RA and Dec)...
		unsigned int world_size_;
		// number of coordinates in our coordinate system...
		unsigned int ncoordinates_;
		// number of values along this axis in the image
		unsigned int image_axis_size_;
		// reference pixels...
		Vector<Double> c_reference_pixel_;
		Vector<Double> csys_reference_pixel_;
	};

	// display CsysAxisInfo...
	std::ostream &operator<<( std::ostream &out, const CsysAxisInfo &ainfo );

    }
}

#endif
