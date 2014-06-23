//# ImageProperties.cc: an object that collects data-range and other info about a casa image
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

#include <display/Utilities/ImageProperties.h>
#include <images/Images/ImageOpener.h>

#include <images/Images/PagedImage.h>
#include <imageanalysis/ImageAnalysis/ImageAnalysis.h>
#include <images/Images/FITSImage.h>
#include <images/Images/FITSQualityImage.h>
#include <images/Images/FITSImgParser.h>
#include <images/Images/MIRIADImage.h>

#include <float.h>
#include <sstream>
#include <casa/Arrays/VectorSTLIterator.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <coordinates/Coordinates/TabularCoordinate.h>
#include <coordinates/Coordinates/StokesCoordinate.h>
#include <coordinates/Coordinates/LinearCoordinate.h>
#include <coordinates/Coordinates/QualityCoordinate.h>
#include <stdexcept>
#include <algorithm>
#include <set>


namespace casa {
	namespace viewer {

		struct strip_chars {
			// should generalize to strip 'chars'...
			strip_chars( const char */*chars*/ ) { }
			strip_chars(const strip_chars &other) : str(other.str) { }
			operator std::string( ) const {
				return str;
			}
			void operator( )( const char &c ) {
				if ( c != '[' && c != ']' ) str += c;
			}
		private:
			std::string str;
		};

		ImageProperties::ImageProperties( )  : status_ok(false), has_direction_axis(false),
			has_spectral_axis(false) { }

		ImageProperties::ImageProperties( const std::string &path ) {
			reset(path);
		}
		ImageProperties::ImageProperties(shared_ptr<ImageInterface<Float> > image ) {
			reset(image);
		}
		ImageProperties::ImageProperties( shared_ptr<ImageInterface<std::complex<float> > > ) {
			throw std::runtime_error("complex images are not supported");
		}

		const ImageProperties &ImageProperties::operator=( const std::string &path ) {
			reset(path);
			return *this;
		}

		std::vector<double> ImageProperties::beam_as_vector( const GaussianBeam &beam ) const {
			std::vector<double> result;
			if (! beam.isNull()) {
				result.push_back( beam.getMajor("arcsec") );
				result.push_back( beam.getMinor("arcsec") );
				result.push_back( beam.getPA("deg", True) );
			}
			return result;
		}

		std::vector<std::string> ImageProperties::beam_as_string_vector( const GaussianBeam &beam ) const {
			std::vector<std::string> result;
			if (! beam.isNull()) {
				char buf[512];
				sprintf( buf,"%.2f\"", beam.getMajor("arcsec"));
				result.push_back(buf);
				sprintf( buf,"%.2f\"", beam.getMinor("arcsec") );
				result.push_back(buf);
				sprintf( buf,"%.2f%c", beam.getPA("deg", True), 0x00B0 );
				result.push_back(buf);
			}
			return result;
		}

		static bool beam_compare( const GaussianBeam &a, const GaussianBeam &b ) {
			return a.getArea("rad2") < b.getArea("rad2");
		}

		std::vector<double> ImageProperties::medianRestoringBeam( ) const {
			if ( restoring_beams.size( ) == 1 )
				return beam_as_vector(restoring_beams[0]);
			else if ( restoring_beams.size( ) > 1 ) {
				std::vector<GaussianBeam> beamcopy(restoring_beams);
				size_t n = beamcopy.size( ) / 2;
				std::nth_element( beamcopy.begin( ), beamcopy.begin( )+n, beamcopy.end( ), beam_compare );
				return beam_as_vector( beamcopy[n] );
			}
			return std::vector<double>( );
		}

		std::vector<std::string> ImageProperties::medianRestoringBeamAsStr( ) const {
			if ( restoring_beams.size( ) == 1 )
				return beam_as_string_vector(restoring_beams[0]);
			else if ( restoring_beams.size( ) > 1 ) {
				std::vector<GaussianBeam> beamcopy(restoring_beams);
				size_t n = beamcopy.size( ) / 2;
				std::nth_element( beamcopy.begin( ), beamcopy.begin( )+n, beamcopy.end( ), beam_compare );
				return beam_as_string_vector( beamcopy[n] );
			}
			return std::vector<std::string>( );
		}

		void ImageProperties::clear_state( ) {
			status_ok = false;
			path_.clear( );
			has_direction_axis = false;
			has_spectral_axis = false;
			direction_type = "";
			shape_.resize(0);
			frequencies_.resize(0);
			freq_units = "";
			velocities_.resize(0);
			velo_units = "";
			ra_range.resize(0);
			ra_range_str.resize(0);
			dec_range.resize(0);
			dec_range_str.resize(0);
			restoring_beams.resize(0);
		}

		void ImageProperties::initialize_state( shared_ptr<ImageInterface<Float> > image ) {

			cs_ = image->coordinates( );
			ImageAnalysis ia(image);
			shape_ = image->shape( ).asVector( );
			if ( shape_.size( ) <= 0 ) {
				return;
			}

			// initialize...
			status_ok = true;

			if ( cs_.hasDirectionCoordinate( ) ) {
				has_direction_axis = true;
				const DirectionCoordinate &direction = cs_.directionCoordinate( );
				direction_type = MDirection::showType(direction.directionType( ));
				Vector<double> refval = direction.referenceValue( );
				if ( refval.size( ) == 2 ) {
					Vector<int> direction_axes = cs_.directionAxesNumbers( );
					Vector<double> pix(direction_axes.size( ));
					for ( unsigned int x=0; x < pix.size( ); ++x ) pix[x] = 0;
					Vector<double> world (pix.size( ));
					direction.toWorld(world,pix);

					ra_range.resize(2);
					dec_range.resize(2);
					casa::String units;
					ra_range[0] = world[0];
					ra_range_str.push_back(direction.format( units, casa::Coordinate::DEFAULT, world[0], 0, true, true ));
					dec_range[0] = world[1];
					dec_range_str.push_back(direction.format( units, casa::Coordinate::DEFAULT, world[1], 1, true, true ));

					for ( unsigned int x=0; x < pix.size( ); ++x ) pix[x] = shape_[direction_axes[x]];
					direction.toWorld(world,pix);
					ra_range[1] = world[0];
					ra_range_str.push_back(direction.format( units, casa::Coordinate::DEFAULT, world[0], 0, true, true ));
					dec_range[1] = world[1];
					dec_range_str.push_back(direction.format( units, casa::Coordinate::DEFAULT, world[1], 1, true, true ));
				}
			}

			if ( cs_.hasSpectralAxis( ) && shape_[cs_.spectralAxisNumber( )] > 1 ) {
				has_spectral_axis = true;
				SpectralCoordinate spec = cs_.spectralCoordinate( );
				Vector<String> spec_unit_vec = spec.worldAxisUnits( );
				if ( spec_unit_vec(0) == "Hz" ) spec_unit_vec(0) = "GHz";
				spec.setWorldAxisUnits(spec_unit_vec);

                frequencies_.resize(shape_[cs_.spectralAxisNumber( )]);
                velocities_.resize(frequencies_.size( ));
                freq_units = spec_unit_vec(0);
                spec.setVelocity( "km/s" );

                for ( /*size_t*/int off=0; off < shape_[cs_.spectralAxisNumber( )]; ++off ) {
                    if ( spec.toWorld(frequencies_[off],off) == false ) {
                        has_spectral_axis = false;
                        frequencies_.resize(0);
                        velocities_.resize(0);
                        break;
                    }

                    //An exception that kills the viewer is thrown if we try to convert pixels to
                    //velocity and the rest frequency is zero.
                    if ( spec.restFrequency() == 0 || spec.pixelToVelocity(velocities_[off],off) == false ) {
                    	has_spectral_axis = false;
                    	frequencies_.resize(0);
                    	velocities_.resize(0);
                    	break;
                    }

                }

                if ( velocities_.size( ) > 0 ) {
                    velo_units = "km/s";
                }

			}

			ImageInfo ii = image->imageInfo();
			if ( ii.hasBeam( ) ) {
				if ( ii.hasMultipleBeams( ) ) {
					for ( size_t i=0; i < ii.getBeamSet().nchan( ); ++i )
						restoring_beams.push_back( ii.restoringBeam(i,0) );
				} else {
					restoring_beams.push_back(ii.restoringBeam());
				}
			}

		}

		std::vector<std::vector<double> > ImageProperties::restoringBeams( ) const {
			std::vector<std::vector<double> > result;
			for ( size_t i=0; i < restoring_beams.size( ); ++i )
				result.push_back(beam_as_vector(restoring_beams[i]));
			return result;
		}

		void ImageProperties::reset(shared_ptr<ImageInterface<Float> > image ) {
			// clear settings...
			clear_state( );
			path_ = image->name(false);
			initialize_state( image );
		}

		void ImageProperties::reset( const std::string &path ) {

			// clear settings...
			clear_state( );

			path_ = path;
			// check for validity...
			if ( path_ == "" ) return;

			shared_ptr<ImageInterface<Float> > image;

			// check for a FITS extension in the path name
			File fin(path_);
			String tmp_path, ext_expr;
			tmp_path = path_;
			if (!fin.exists() && !fin.isDirectory()) {
				if (!(int)path.compare(path.length()-1, 1, "]", 1) && (int)path.rfind("[", path.length()) > -1) {
					// create a string with the file path name only
					tmp_path = String(path, 0, path.rfind("[", path.length()));
					ext_expr = String(path, path.rfind("[", path.length()), path.length());
				}
			}

			switch ( ImageOpener::imageType(path_) ) {
			case ImageOpener::AIPSPP:
				if ( imagePixelType(path_) != TpFloat ) return;
				image.reset(new PagedImage<Float>(path_, TableLock::AutoNoReadLocking));
				break;
			case ImageOpener::FITS: {
				FITSImgParser fip = FITSImgParser(tmp_path);
				if (fip.has_qualityimg() && fip.is_qualityimg(ext_expr)) {
					image.reset(new FITSQualityImage(path));
				} else {
					image.reset(new FITSImage(path));
				}
			}
			break;
			case ImageOpener::MIRIAD:
				image.reset(new MIRIADImage(path));
				break;
			default:
				return;
			}

			if ( image->ok( ) == false ) {
				return;
			}

			initialize_state( image );
		}

	}
}


std::ostream &operator<<( std::ostream &os, const casa::DisplayCoordinateSystem &cs ) {
    casa::Vector<casa::String> names = cs.worldAxisNames( );
    casa::Vector<casa::String> units = cs.worldAxisUnits( );
    os << "[[" << cs.nWorldAxes( ) << "w," << cs.nPixelAxes( ) << "p]" << std::endl;
    for ( unsigned int i=0; i < cs.nWorldAxes(); ++i ) {
        int coordNum, axisInCoord;
        cs.findWorldAxis( coordNum, axisInCoord, i );
        os << " ";
        switch ( cs.type(coordNum) ) {
          case casa::Coordinate::LINEAR:
            {
            //const casa::LinearCoordinate &coordinate = cs.linearCoordinate(coordNum);
            os << "(" << i << ":" << coordNum << ":" << axisInCoord << ":" << cs.worldAxisToPixelAxis(i) << "P),linear) " << names[i] << "/" << units[i];
            }
            break;
          case casa::Coordinate::DIRECTION:
            {
            const casa::DirectionCoordinate &coordinate = cs.directionCoordinate(coordNum);
            os << "(" << i << ":" << coordNum << ":" << axisInCoord << ":" << cs.worldAxisToPixelAxis(i) << "P),direction:" << casa::MDirection::showType(coordinate.directionType()) << ") " << names[i] << "/" << units[i];
            }
            break;
          case casa::Coordinate::SPECTRAL: 
            {
            //const casa::SpectralCoordinate &coordinate = cs.spectralCoordinate(coordNum);
            os << "(" << i << ":" << coordNum << ":" << axisInCoord << ":" << cs.worldAxisToPixelAxis(i) << "P),spectral) " << names[i] << "/" << units[i];
            }
            break;
          case casa::Coordinate::STOKES: 
            {
            //const casa::StokesCoordinate  &coordinate = cs.stokesCoordinate(coordNum);
            os << "(" << i << ":" << coordNum << ":" << axisInCoord << ":" << cs.worldAxisToPixelAxis(i) << "P),stokes) " << names[i] << "/" << units[i];
            }
            break;
          case casa::Coordinate::TABULAR:
            {
            //const casa::TabularCoordinate &coordinate = cs.tabularCoordinate(coordNum);
            os << "(" << i << ":" << coordNum << ":" << axisInCoord << ":" << cs.worldAxisToPixelAxis(i) << "P),tabular) " << names[i] << "/" << units[i];
            }
            break;
          case casa::Coordinate::QUALITY:
            {
            //const casa::QualityCoordinate &coordinate = cs.qualityCoordinate(coordNum);
            os << "(" << i << ":" << coordNum << ":" << axisInCoord << ":" << cs.worldAxisToPixelAxis(i) << "P),quality) " << names[i] << "/" << units[i];
            }
            break;
          case casa::Coordinate::COORDSYS:
            os << "<<<coordinate system>>>";
        };
        os << std::endl;
    }
    os << "]";
    return os;
}
     

