//# DisplayCoordinateSystem.h: Interconvert pixel and image coordinates.
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
//#
//# $Id: CoordinateSystem.h 20491 2009-01-16 08:33:56Z gervandiepen $

#ifndef DISPLAY_COORDINATESYSTEM_H_
#define DISPLAY_COORDINATESYSTEM_H_

#include <coordinates/Coordinates/CoordinateSystem.h>
#include <coordinates/Coordinates/CoordinateUtil.h>
#include <components/ComponentModels/SkyComponentFactory.h>

namespace casa {

    class DisplayCoordinateSystem {
        public:

            DisplayCoordinateSystem ( ) { }
            DisplayCoordinateSystem( const DisplayCoordinateSystem &o ) :
                       cs_(o.cs_), transposition_log(o.transposition_log) { }
            DisplayCoordinateSystem( const casacore::CoordinateSystem &c ) : cs_(c) { }
            virtual ~DisplayCoordinateSystem( );

            operator const casacore::CoordinateSystem &( ) const { return cs_; }

            const DisplayCoordinateSystem &operator =(const DisplayCoordinateSystem &o) {
                transposition_log = o.transposition_log;
                cs_ = o.cs_;
                return *this;
            }
            const DisplayCoordinateSystem &operator =(const casacore::CoordinateSystem &o) {
                transposition_log.clear( );
                cs_ = o;
                return *this;
            }

			casacore::String showType(casacore::uInt whichCoordinate) const
                { return cs_.showType(whichCoordinate); }

            casacore::uInt nCoordinates() const { return cs_.nCoordinates( ); }
            unsigned int nPixelAxes() const { return cs_.nPixelAxes( ); }
            unsigned int nWorldAxes() const { return cs_.nWorldAxes( ); }

            // is the first direction axis RA or Dec...
            bool isDirectionAbscissaLongitude( ) const
                { return cs_.isDirectionAbscissaLongitude( ); }

            casacore::ObsInfo obsInfo() const
                { return cs_.obsInfo( ); }
            void setObsInfo(const casacore::ObsInfo &obsinfo)
                { cs_.setObsInfo(obsinfo); }

            casacore::Vector<casacore::String> worldAxisUnits() const;
            casacore::Vector<casacore::String> worldAxisNames() const;
            bool setWorldAxisUnits(const casacore::Vector<casacore::String> &units);
            bool setWorldAxisNames( const casacore::Vector<casacore::String> &names);
            casacore::Vector<int> worldAxes(casacore::uInt whichCoord) const;
            casacore::Vector<int> pixelAxes(casacore::uInt whichCoord) const;

            casacore::Vector<double> worldMixMin () const
                { return cs_.worldMixMin( ); }
            casacore::Vector<double> worldMixMax () const
                { return cs_.worldMixMax( ); }
            bool setWorldMixRanges (const casacore::IPosition& shape)
                { return cs_.setWorldMixRanges(shape); }

			bool near( const casacore::Coordinate& other, casacore::Double tol=1e-6 ) const
                { return cs_.near(other,tol); }
			bool near(const casacore::Coordinate& other, const casacore::Vector<casacore::Int>& excludePixelAxes, casacore::Double tol=1e-6) const
                { return cs_.near(other,excludePixelAxes,tol); }

            void makePixelRelative( casacore::Vector<double>& pixel ) const;
            void makePixelAbsolute( casacore::Vector<double>& pixel ) const;
            void makeWorldAbsolute (casacore::Vector<double>& world) const;
            void makeWorldRelative (casacore::Vector<double>& world) const;

            casacore::Vector<double> referencePixel() const
                { return cs_.referencePixel( ); }
            bool setReferencePixel( const casacore::Vector<double> &refPix)
                { return cs_.setReferencePixel( casacore::Vector<double>(refPix) ); }
            casacore::Vector<double> referenceValue() const
                { return cs_.referenceValue( ); }
            bool setReferenceValue( const casacore::Vector<double> &refval )
                { return cs_.setReferenceValue(refval); }
            bool setIncrement( const casacore::Vector<double> &inc )
                { return cs_.setIncrement(inc); }


            casacore::Coordinate::Type type(unsigned int whichCoordinate) const { return cs_.type(whichCoordinate); }

            bool hasDirectionCoordinate( ) const { return cs_.hasDirectionCoordinate( ); }
            const casacore::DirectionCoordinate &directionCoordinate( int which = -1 ) const
                { return which < 0 ? cs_.directionCoordinate( ) : cs_.directionCoordinate(which); }
            casacore::Vector<int> directionAxesNumbers() const;

            bool hasSpectralAxis() const { return cs_.hasSpectralAxis( ); }
            const casacore::SpectralCoordinate &spectralCoordinate( int which = -1 ) const
                { return which < 0 ? cs_.spectralCoordinate( ) : cs_.spectralCoordinate(which); }
            int spectralAxisNumber(bool doWorld=false) const { return cs_.spectralAxisNumber(doWorld); }

            const casacore::LinearCoordinate &linearCoordinate( unsigned int which ) const
                { return cs_.linearCoordinate(which); }
            bool hasPolarizationCoordinate() const { return cs_.hasPolarizationCoordinate(); }

            casacore::Int polarizationAxisNumber(casacore::Bool doWorld=false) const {
            	return cs_.polarizationAxisNumber(doWorld);
            }
            const casacore::StokesCoordinate &stokesCoordinate( int which = -1 ) const
                { return which < 0 ? cs_.stokesCoordinate( ) : cs_.stokesCoordinate(which); }

            const casacore::TabularCoordinate &tabularCoordinate( unsigned int which ) const
                { return cs_.tabularCoordinate(which); }

			const casacore::QualityCoordinate &qualityCoordinate(unsigned int which) const
                { return cs_.qualityCoordinate(which); }
			int qualityAxisNumber() const
                { return cs_.qualityAxisNumber( ); }

            int worldAxisToPixelAxis(unsigned int worldAxis) const
                { return cs_.worldAxisToPixelAxis(worldAxis); }
            int pixelAxisToWorldAxis(unsigned int pixelAxis) const
                { return cs_.pixelAxisToWorldAxis(pixelAxis); }

            void findWorldAxis( int &coordinate, int &axisInCoordinate, unsigned int axisInCoordinateSystem) const
                { cs_.findWorldAxis( coordinate, axisInCoordinate, axisInCoordinateSystem ); }
            void findPixelAxis( int &coordinate, int &axisInCoordinate, unsigned int axisInCoordinateSystem) const
                { cs_.findPixelAxis( coordinate, axisInCoordinate, axisInCoordinateSystem ); }

            const casacore::Coordinate& coordinate(unsigned int which) const
                { return cs_.coordinate(which); }
            int findCoordinate( casacore::Coordinate::Type type, int afterCoord = -1 ) const
                { return cs_.findCoordinate(type,afterCoord); }
            bool replaceCoordinate( const casacore::Coordinate &newCoordinate, unsigned int whichCoordinate )
                { return cs_.replaceCoordinate( newCoordinate, whichCoordinate ); }

            void addCoordinate(const casacore::Coordinate &coord)
                { cs_.addCoordinate(coord); }
            bool removeWorldAxis(unsigned int axis, double replacement)
                { return cs_.removeWorldAxis(axis,replacement); }
            bool removePixelAxis(unsigned int axis, double replacement)
                { return cs_.removePixelAxis(axis,replacement); }
            void transpose(const casacore::Vector<int> &newWorldOrder, const casacore::Vector<int> &newPixelOrder);

            casacore::String format( casacore::String& units, casacore::Coordinate::formatType format, double worldValue, unsigned int worldAxis,
                                bool isAbsolute=true, bool showAsAbsolute=true, int precision=-1, bool usePrecForMixed=false ) const;

            // coordinate mapping...
            const casacore::String& errorMessage() const { return cs_.errorMessage( ); }

            bool toWorld( casacore::Vector<double> &world, const casacore::Vector<double> &pixel ) const;
            bool toPixel(casacore::Vector<double> &pixel, const casacore::Vector<double> &world) const;

#if 0
            bool toWorld( casacore::Vector<double> &world, const casacore::Vector<double> &pixel ) const;
            bool toPixel(casacore::Vector<double> &pixel, const casacore::Vector<double> &world) const;
#endif
            bool toMix( casacore::Vector<double>& worldOut,       casacore::Vector<double>& pixelOut,
                        const casacore::Vector<double>& worldIn,  const casacore::Vector<double>& pixelIn,
                        const casacore::Vector<bool>& worldAxes,  const casacore::Vector<bool>& pixelAxes,
                        const casacore::Vector<double>& worldMin, const casacore::Vector<double>& worldMax) const;

            void subImageInSitu( const casacore::Vector<float> &originShift, const casacore::Vector<float> &incrFac, const casacore::Vector<int>& newShape)
                { cs_.subImageInSitu( originShift, incrFac, newShape ); }


            casacore::Vector<double> increment( ) const
                { return cs_.increment( ); }

            bool save( casacore::RecordInterface &container, const casacore::String &fieldName ) const
                { return cs_.save(container,fieldName); }
			static DisplayCoordinateSystem restore( const casacore::RecordInterface &container,const casacore::String &fieldName );

            // casacore::CoordinateUtil wrappers...
            bool setRestFrequency( casacore::String& errorMsg, const casacore::String& unit, const casacore::Double& value )
                { return cs_.setRestFrequency( errorMsg, casacore::Quantity(value, unit) ); }
            bool setSpectralState( casacore::String& errorMsg,const casacore::String& unit, const casacore::String& spcquant)
                { return casacore::CoordinateUtil::setSpectralState( errorMsg, cs_, unit, spcquant ); }
            bool setSpectralConversion( casacore::String& errorMsg, const casacore::String frequencySystem )
                { return cs_.setSpectralConversion( errorMsg, frequencySystem); }
            bool setSpectralFormatting( casacore::String& errorMsg, const casacore::String& unit, const casacore::String& spcquant )
                { return casacore::CoordinateUtil::setSpectralFormatting( errorMsg, cs_, unit, spcquant ); }
            bool setDirectionUnit( const string& unit, casacore::Int which=-1)
                { return casacore::CoordinateUtil::setDirectionUnit( cs_, unit, which ); }
            void setNiceAxisLabelUnits( )
                { casacore::CoordinateUtil::setNiceAxisLabelUnits( cs_ ); }
            bool setVelocityState( casacore::String& errorMsg, const casacore::String& unit, const casacore::String& spcquant)
                { return casacore::CoordinateUtil::setVelocityState( errorMsg, cs_, unit, spcquant ); }
            bool removePixelAxes( casacore::Vector<casacore::Double>& pixelReplacement, const casacore::Vector<casacore::Int>& pixelAxes, const casacore::Bool remove)
                { return casacore::CoordinateUtil::removePixelAxes( cs_, pixelReplacement,pixelAxes,remove ); }
            bool removeAxes( casacore::Vector<double>& worldReplacement, const casacore::Vector<int>& worldAxes, bool remove)
                { return casacore::CoordinateUtil::removeAxes( cs_, worldReplacement, worldAxes, remove); }



            // from ImageUtilities...
            void worldWidthsToPixel( casacore::LogIO& /*os*/, casacore::Vector<double>& dParameters, const casacore::Vector<casacore::Quantum<casacore::Double> >& parameters,
                                     const casacore::IPosition& pixelAxes, bool doRef=false ) const {
                SkyComponentFactory::worldWidthsToPixel(dParameters, parameters, cs_, pixelAxes, doRef );
            }

			// adjust this coordinate system to match the 'other' coordinate system
			void match( const DisplayCoordinateSystem &other );
            casacore::Vector<int> transposeShape( const casacore::Vector<int> &original_shape, bool world=true );

        private:
            casacore::CoordinateSystem cs_;
			typedef vector<pair<casacore::Vector<int>,casacore::Vector<int> > > transposition_log_t;
			transposition_log_t transposition_log;
    };

}

#endif
