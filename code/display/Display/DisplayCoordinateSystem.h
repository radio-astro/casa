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
#include <images/Images/ImageUtilities.h>

namespace casa {

    class DisplayCoordinateSystem {
        public:

            DisplayCoordinateSystem ( ) { }
            DisplayCoordinateSystem( const DisplayCoordinateSystem &o ) :
                       cs_(o.cs_), transposition_log(o.transposition_log) { }
            DisplayCoordinateSystem( const CoordinateSystem &c ) : cs_(c) { }
            virtual ~DisplayCoordinateSystem( );

            operator const CoordinateSystem &( ) const { return cs_; }

            const DisplayCoordinateSystem &operator =(const DisplayCoordinateSystem &o) {
                transposition_log = o.transposition_log;
                cs_ = o.cs_;
                return *this;
            }
            const DisplayCoordinateSystem &operator =(const CoordinateSystem &o) {
                transposition_log.clear( );
                cs_ = o;
                return *this;
            }

			String showType(uInt whichCoordinate) const
                { return cs_.showType(whichCoordinate); }

            uInt nCoordinates() const { return cs_.nCoordinates( ); }
            unsigned int nPixelAxes() const { return cs_.nPixelAxes( ); }
            unsigned int nWorldAxes() const { return cs_.nWorldAxes( ); }

            // is the first direction axis RA or Dec...
            bool isDirectionAbscissaLongitude( ) const
                { return cs_.isDirectionAbscissaLongitude( ); }

            ObsInfo obsInfo() const
                { return cs_.obsInfo( ); }
            void setObsInfo(const ObsInfo &obsinfo)
                { cs_.setObsInfo(obsinfo); }

            Vector<String> worldAxisUnits() const;
            Vector<String> worldAxisNames() const;
            bool setWorldAxisUnits(const Vector<String> &units);
            bool setWorldAxisNames( const Vector<String> &names);
            Vector<int> worldAxes(uInt whichCoord) const;
            Vector<int> pixelAxes(uInt whichCoord) const;

            Vector<double> worldMixMin () const
                { return cs_.worldMixMin( ); }
            Vector<double> worldMixMax () const
                { return cs_.worldMixMax( ); }
            bool setWorldMixRanges (const IPosition& shape)
                { return cs_.setWorldMixRanges(shape); }

			bool near( const Coordinate& other, Double tol=1e-6 ) const
                { return cs_.near(other,tol); }
			bool near(const Coordinate& other, const Vector<Int>& excludePixelAxes, Double tol=1e-6) const
                { return cs_.near(other,excludePixelAxes,tol); }

            void makePixelRelative( Vector<double>& pixel ) const;
            void makePixelAbsolute( Vector<double>& pixel ) const;
            void makeWorldAbsolute (Vector<double>& world) const;
            void makeWorldRelative (Vector<double>& world) const;

            Vector<double> referencePixel() const
                { return cs_.referencePixel( ); }
            bool setReferencePixel( const Vector<double> &refPix)
                { return cs_.setReferencePixel( Vector<double>(refPix) ); }
            Vector<double> referenceValue() const
                { return cs_.referenceValue( ); }
            bool setReferenceValue( const Vector<double> &refval )
                { return cs_.setReferenceValue(refval); }
            bool setIncrement( const Vector<double> &inc )
                { return cs_.setIncrement(inc); }


            Coordinate::Type type(unsigned int whichCoordinate) const { return cs_.type(whichCoordinate); }

            bool hasDirectionCoordinate( ) const { return cs_.hasDirectionCoordinate( ); }
            const DirectionCoordinate &directionCoordinate( int which = -1 ) const
                { return which < 0 ? cs_.directionCoordinate( ) : cs_.directionCoordinate(which); }
            Vector<int> directionAxesNumbers() const;

            bool hasSpectralAxis() const { return cs_.hasSpectralAxis( ); }
            const SpectralCoordinate &spectralCoordinate( int which = -1 ) const
                { return which < 0 ? cs_.spectralCoordinate( ) : cs_.spectralCoordinate(which); }
            int spectralAxisNumber(bool doWorld=false) const { return cs_.spectralAxisNumber(doWorld); }

            const LinearCoordinate &linearCoordinate( unsigned int which ) const
                { return cs_.linearCoordinate(which); }

            const StokesCoordinate &stokesCoordinate( int which = -1 ) const
                { return which < 0 ? cs_.stokesCoordinate( ) : cs_.stokesCoordinate(which); }

            const TabularCoordinate &tabularCoordinate( unsigned int which ) const
                { return cs_.tabularCoordinate(which); }

			const QualityCoordinate &qualityCoordinate(unsigned int which) const
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

            const Coordinate& coordinate(unsigned int which) const
                { return cs_.coordinate(which); }
            int findCoordinate( Coordinate::Type type, int afterCoord = -1 ) const
                { return cs_.findCoordinate(type,afterCoord); }
            bool replaceCoordinate( const Coordinate &newCoordinate, unsigned int whichCoordinate )
                { return cs_.replaceCoordinate( newCoordinate, whichCoordinate ); }

            void addCoordinate(const Coordinate &coord)
                { cs_.addCoordinate(coord); }
            bool removeWorldAxis(unsigned int axis, double replacement)
                { return cs_.removeWorldAxis(axis,replacement); }
            bool removePixelAxis(unsigned int axis, double replacement)
                { return cs_.removePixelAxis(axis,replacement); }
            void transpose(const Vector<int> &newWorldOrder, const Vector<int> &newPixelOrder);

            String format( String& units, Coordinate::formatType format, double worldValue, unsigned int worldAxis,
                                bool isAbsolute=true, bool showAsAbsolute=true, int precision=-1, bool usePrecForMixed=false ) const;

            // coordinate mapping...
            const String& errorMessage() const { return cs_.errorMessage( ); }

            bool toWorld( Vector<double> &world, const Vector<double> &pixel ) const;
            bool toPixel(Vector<double> &pixel, const Vector<double> &world) const;

#if 0
            bool toWorld( Vector<double> &world, const Vector<double> &pixel ) const;
            bool toPixel(Vector<double> &pixel, const Vector<double> &world) const;
#endif
            bool toMix( Vector<double>& worldOut,       Vector<double>& pixelOut,
                        const Vector<double>& worldIn,  const Vector<double>& pixelIn,
                        const Vector<bool>& worldAxes,  const Vector<bool>& pixelAxes,
                        const Vector<double>& worldMin, const Vector<double>& worldMax) const;

            void subImageInSitu( const Vector<float> &originShift, const Vector<float> &incrFac, const Vector<int>& newShape)
                { cs_.subImageInSitu( originShift, incrFac, newShape ); }


            Vector<double> increment( ) const
                { return cs_.increment( ); }

            bool save( RecordInterface &container, const String &fieldName ) const
                { return cs_.save(container,fieldName); }
			static DisplayCoordinateSystem restore( const RecordInterface &container,const String &fieldName );

            // CoordinateUtil wrappers...
            bool setRestFrequency( String& errorMsg, const String& unit, const Double& value )
                { return CoordinateUtil::setRestFrequency( errorMsg, cs_, unit, value ); }
            bool setSpectralState( String& errorMsg,const String& unit, const String& spcquant)
                { return CoordinateUtil::setSpectralState( errorMsg, cs_, unit, spcquant ); }
            bool setSpectralConversion( String& errorMsg, const String frequencySystem )
                { return CoordinateUtil::setSpectralConversion( errorMsg, cs_, frequencySystem); }
            bool setSpectralFormatting( String& errorMsg, const String& unit, const String& spcquant )
                { return CoordinateUtil::setSpectralFormatting( errorMsg, cs_, unit, spcquant ); }
            bool setDirectionUnit( const string& unit, Int which=-1)
                { return CoordinateUtil::setDirectionUnit( cs_, unit, which ); }
            void setNiceAxisLabelUnits( )
                { CoordinateUtil::setNiceAxisLabelUnits( cs_ ); }
            bool setVelocityState( String& errorMsg, const String& unit, const String& spcquant)
                { return CoordinateUtil::setVelocityState( errorMsg, cs_, unit, spcquant ); }
            bool removePixelAxes( Vector<Double>& pixelReplacement, const Vector<Int>& pixelAxes, const Bool remove)
                { return CoordinateUtil::removePixelAxes( cs_, pixelReplacement,pixelAxes,remove ); }
            bool removeAxes( Vector<double>& worldReplacement, const Vector<int>& worldAxes, bool remove)
                { return CoordinateUtil::removeAxes( cs_, worldReplacement, worldAxes, remove); }



            // from ImageUtilities...
            void worldWidthsToPixel( LogIO& os, Vector<double>& dParameters, const Vector<Quantum<Double> >& parameters,
                                     const IPosition& pixelAxes, bool doRef=false ) const {
                ImageUtilities::worldWidthsToPixel( os, dParameters, parameters, cs_, pixelAxes, doRef );
            }

			// adjust this coordinate system to match the 'other' coordinate system
			void match( const DisplayCoordinateSystem &other );
            Vector<int> transposeShape( const Vector<int> &original_shape, bool world=true );

        private:
            CoordinateSystem cs_;
			typedef vector<pair<Vector<int>,Vector<int> > > transposition_log_t;
			transposition_log_t transposition_log;
    };

}

#endif
