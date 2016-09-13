//# Copyright (C) 2005
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

#include <display/QtPlotter/QtPlotSettings.h>
#include <display/QtPlotter/conversion/Converter.h>
#include <assert.h>
#include <QtGui>

namespace casa {

	const QString QtPlotSettings::RADIO_VELOCITY   = "radio velocity";
	const QString QtPlotSettings::OPTICAL_VELOCITY = "optical velocity";
	const QString QtPlotSettings::OPTICAL_WAVELENGTH = "air wavelength";
	const double QtPlotSettings::ZERO_LIMIT        = 0.0000000000000005f;

	QtPlotSettings::QtPlotSettings( ) {
		const int TICK_MIN = 0;
		const int TICK_MAX = 10;
		const int TICK_COUNT = 5;
		for ( int i = 0; i < QtPlotSettings::END_AXIS_INDEX; i++ ) {
			AxisIndex axisIndex = static_cast<AxisIndex>(i);
			minX[axisIndex] = TICK_MIN;
			maxX[axisIndex] = TICK_MAX;
		}
		numXTicks = TICK_COUNT;
		minY = TICK_MIN;
		maxY = TICK_MAX;
		numYTicks = TICK_COUNT;
		minPercentage = 0;
		maxPercentage = 0;
	}


	void QtPlotSettings::scroll(int dx, int dy) {
		for ( int i = 0; i < QtPlotSettings::END_AXIS_INDEX; i++ ) {
			AxisIndex axisIndex = static_cast<AxisIndex>(i);
			double stepX = spanX( axisIndex ) / numXTicks/*[i]*/;
			minX[i] += dx * stepX;
			maxX[i] += dx * stepX;
		}

		double stepY = spanY() / numYTicks;
		minY += dy * stepY;
		maxY += dy * stepY;
	}

	pair<double,double> QtPlotSettings::getZoomOutY( double zoomFactor ) const {
		double prevSpanY = spanY();
		double minY = this->minY - zoomFactor * prevSpanY;
		double maxY = this->maxY + zoomFactor * prevSpanY;
		pair<double,double> percentageSpan( minY, maxY );
		return percentageSpan;
	}

	void QtPlotSettings::zoomOut( double zoomFactor, const QString& topUnits, const QString& topType,
	                              const QString& bottomUnits, const QString& bottomType,
	                              bool autoScaleX, bool autoScaleY ) {
		for ( int i = 0; i < END_AXIS_INDEX; i++ ) {
			AxisIndex axisIndex = static_cast<AxisIndex>(i);
			double prevSpanX = spanX(axisIndex);
			minX[i] = minX[i] - zoomFactor * prevSpanX;
			maxX[i] = maxX[i] + zoomFactor * prevSpanX;
		}
		adjust( topUnits, topType, bottomUnits, bottomType, autoScaleX, autoScaleY, true );
	}


	pair<double,double> QtPlotSettings::getZoomInY( double zoomFactor ) const {
		double prevSpanY = spanY();
		double minY = this->minY + zoomFactor * prevSpanY;
		double maxY = this->maxY - zoomFactor * prevSpanY;
		pair<double,double> percentageSpan( minY,maxY);
		return percentageSpan;
	}


	void QtPlotSettings::zoomY( double minY, double maxY, bool autoScaleY ) {
		this->minY = minY;
		this->maxY = maxY;
		if ( autoScaleY ) {
			adjustAxis( minY, maxY, numYTicks );
		}
	}



	void QtPlotSettings::zoomIn( double zoomFactor, const QString& topUnits, const QString& topType,
	                             const QString& bottomUnits, const QString& bottomType,
	                             bool autoScaleX, bool autoScaleY ) {
		for ( int i = 0; i < END_AXIS_INDEX; i++ ) {
			AxisIndex axisIndex = static_cast<AxisIndex>(i);
			double prevSpanX = spanX( axisIndex );
			minX[i] = minX[i] + zoomFactor * prevSpanX;
			maxX[i] = maxX[i] - zoomFactor * prevSpanX;
		}
		adjust( topUnits, topType, bottomUnits, bottomType, autoScaleX, autoScaleY, true );
	}



	void QtPlotSettings::adjust( const QString& topUnits, const QString& topType,
			const QString& bottomUnits, const QString& bottomType,
			bool autoScaleX, bool autoScaleY, bool zoom) {

		m_topType = topType;
		m_bottomType = bottomType;
		m_topUnits = topUnits;
		m_bottomUnits = bottomUnits;
		if ( autoScaleX ) {
			//Adjust the bottom axis allowing it to set the number of ticks.
			pair<double,double> percentChange=adjustAxis( minX[xBottom], maxX[xBottom], numXTicks);

			if ( percentChange.first > 0 ){
				minPercentage = percentChange.first;
			}
			if ( zoom && percentChange.first == 0 ){
				minPercentage = 0;
			}
			if ( percentChange.second > 0 ){
				maxPercentage = percentChange.second;
			}
			if ( zoom && percentChange.second == 0 ){
				maxPercentage = 0;
			}

			//Adjust the top axis using the same number of ticks.  Use a
			//converter to get its min and max based on the min and max of
			//the bottom axis.
			adjustAxisTop( minX[xTop], maxX[xTop]);
		}

		if ( autoScaleY ) {
			//Now adjust the y-axis
			adjustAxis(minY, maxY, numYTicks);
		}
	}


	pair<double,double> QtPlotSettings::adjustAxis(double &min, double &max,
	                                int &numTicks ) {
		const int MinTicks = 4;
		double grossStep = fabs(max - min) / MinTicks;
		double step = std::pow(10, floor(log10(grossStep)));
		if (5 * step < grossStep) {
			step *= 5;
		} else if (2 * step < grossStep) {
			step *= 2;
		}
		numTicks = (int)fabs(ceil(max / step) - floor(min / step));
		double newMin = floor(min / step) * step;
		double newMax = ceil(max / step) * step;
		double minPercentage = 0;
		double maxPercentage = 0;
		if ( max  != min ){
			minPercentage = (min - newMin) / (max - min);
			maxPercentage = (newMax - max) / (max - min );
		}
		min = newMin;
		max = newMax;

		pair<double,double> percentageChange( minPercentage, maxPercentage );
		return percentageChange;
	}



	void QtPlotSettings::adjustAxisTop(double &min, double &max) {
		//The calculation below, based on percentages is being used rather than
		//using a converter because of CAS-5175 (CAS-5252?).  The problem was that the
		//converter could not distinguish between optical velocity in km/sec
		//and radio velocity in km/sec since the units were the same.
		/*double topRange = maxX[QtPlotSettings::xTop] - minX[QtPlotSettings::xTop];
		min = minX[QtPlotSettings::xTop] - minPercentage * topRange;
		max = maxX[QtPlotSettings::xTop] + maxPercentage * topRange;*/

		//However, it now appears we are back to conversion.  Please see CAS-8591.
		//There are errors being introduced with percentages.  Thus, the spectral coordinate has been
		//introduces and we are setting a doppler.
		Quantity botUnit( 0, m_bottomUnits.toStdString().c_str());
		//Top axis is not channels
		if ( m_topUnits != "" ) {
			//If the bottom unit is not already Hz, convert it to such since optical
			//and radio Hz will agree.
			double minHz = 0;
			double maxHz = 0;
			if ( !botUnit.isConform( "Hz") ){
				Converter* converter = Converter::getConverter( m_bottomUnits, "Hz");
				minHz = converter->convert( minX[QtPlotSettings::xBottom], m_spectralCoordinate );
				maxHz = converter->convert( maxX[QtPlotSettings::xBottom], m_spectralCoordinate );
				delete converter;
			}

			//If the top axis involves optical units, then we need to set the doppler
			//before being able to convert.
			if ( m_topType == OPTICAL_VELOCITY || m_topType == OPTICAL_WAVELENGTH ){
				std::pair<double,double> topBounds = convertBottomBounds( minHz, maxHz, m_topUnits, m_topType);
				min = topBounds.first;
				max = topBounds.second;
			}

			else if ( m_topUnits != "Hz"){
				Converter* converter = Converter::getConverter( "Hz", m_topUnits);
				min = converter->convert( minHz, m_spectralCoordinate );
				max = converter->convert( maxHz, m_spectralCoordinate  );
				delete converter;
			}
			else {
				min = minHz;
				max = maxHz;
			}
		}
		//Top axis is channels, but bottom axis is not
		else if ( m_topUnits == "" && m_bottomUnits != "" ) {
			if ( !botUnit.isConform( "m/s") ){
				Converter* channelConverter = Converter::getConverter(m_bottomUnits,m_topUnits);
				double startChannel = channelConverter->toPixel( minX[QtPlotSettings::xBottom], m_spectralCoordinate);
				double endChannel = channelConverter->toPixel( maxX[QtPlotSettings::xBottom], m_spectralCoordinate);
				delete channelConverter;
				min = startChannel;
				max = endChannel;
			}
			else {
				double topRange = maxX[QtPlotSettings::xTop] - minX[QtPlotSettings::xTop];
				min = minX[QtPlotSettings::xTop] - minPercentage * topRange;
				max = maxX[QtPlotSettings::xTop] + maxPercentage * topRange;
			}

		}
		//Both axis are using channels
		else {
			min = minX[QtPlotSettings::xBottom];
			max = maxX[QtPlotSettings::xBottom];
		}

		if ( m_bottomUnits == "") {
			minPercentage = 0;
			maxPercentage = 0;
		}
	}

	void QtPlotSettings::setMinX( AxisIndex index, double value ) {
		minX[static_cast<int>(index)] = value;
	}

	void QtPlotSettings::setMaxX( AxisIndex index, double value ) {
		maxX[static_cast<int>(index)] = value;
	}

	void QtPlotSettings::setMinY( double value ) {
		minY = value;
	}

	void QtPlotSettings::setMaxY( double value ) {
		maxY = value;
	}


	double QtPlotSettings::getTickValue(int tickIndex, int tickCount,
			QtPlotSettings::AxisIndex axisIndex) const {
		double value = getMinX(axisIndex) + (tickIndex * spanX(axisIndex) / tickCount);
		return value;
	}


	double QtPlotSettings::getTickLabelX(int tickIndex, int tickCount,
			QtPlotSettings::AxisIndex axisIndex) const {
		double label = getTickValue( tickIndex, tickCount, axisIndex);
		QtPlotSettings::AxisIndex otherIndex = QtPlotSettings::xTop;
		if( axisIndex == QtPlotSettings::xTop ){
			otherIndex = QtPlotSettings::xBottom;
		}
		double otherLabel = getTickValue(tickIndex, tickCount, otherIndex);

		//If the label is very close to zero, make it zero.
		if ( qAbs( label ) < ZERO_LIMIT  ){
			label = 0;
		}
		//Note:  The latter two cases if the if statement are inspired by CAS-8512.  The idea
		//being that if optical velocity is zero at one of the label points, then radio velocity
		//should also be zero at the label point and vice versa.  This was generalized to include the case if
		//the units are the same on both the top and bottom axes, then the labels should be the
		//same.
		//If the type and units are the same as the other axis type and units, eliminate round-off
		//by making them the same.
		else if ( m_bottomType == m_topType && m_topUnits == m_bottomUnits ){
			if ( axisIndex == QtPlotSettings::xTop ){
				label = otherLabel;
			}
		}
		//If the other one was very close to zero and the only difference between them is optical
		//versus radio, make sure they are both zero at zero.
		else if ( qAbs(otherLabel) < ZERO_LIMIT && axisIndex == QtPlotSettings::xTop ){
			//For this label to be zero, it should either be the first index, the last index,
			//or bracketed by a positive & negative on each side.
			bool bracketed = false;
			if ( tickIndex > 0 && tickIndex < tickCount - 1 ){
				double label1 = getTickValue( tickIndex-1, tickCount, axisIndex );
				double label2 = getTickValue( tickIndex+1, tickCount, axisIndex );
				if ( label1 * label2 < 0 ){
					bracketed = true;
				}
			}
			if ( tickIndex == 0 || (tickIndex == tickCount - 1) || bracketed ){
				if ( m_topType == OPTICAL_VELOCITY ){
					if ( m_bottomType == RADIO_VELOCITY){
						label = 0;
					}
				}
				else if ( m_topType == RADIO_VELOCITY ){
					if ( m_bottomType == OPTICAL_VELOCITY){
						label = 0;
					}
				}
			}
		}
		return label;
	}


	void QtPlotSettings::setSpectralCoordinate( const SpectralCoordinate& coord ){
		m_spectralCoordinate = coord;
	}


	MDoppler::Types QtPlotSettings::getDoppler(  const QString& unitType ){
		MDoppler::Types dopplerType = MDoppler::RELATIVISTIC;
		if ( unitType == RADIO_VELOCITY ){
			dopplerType = MDoppler::RADIO;
		}
		else if ( unitType == OPTICAL_VELOCITY ){
			dopplerType = MDoppler::OPTICAL;
		}
		return dopplerType;
	}


	std::pair<double,double> QtPlotSettings::convertBottomBounds(
			double min, double max, const QString& units, const QString& unitType ) {

		Vector<double> coords(2);
		coords[0] = min;
		coords[1] = max;
		Unit unit( units.toStdString().c_str());
		Quantity t(0, unit);
		//Velocity units
		if (t.isConform("m/s")) {
			MDoppler::Types doppler = getDoppler( unitType );
			m_spectralCoordinate.setVelocity(units.toStdString().c_str() ,doppler);
			m_spectralCoordinate.frequencyToVelocity(coords, coords);
		}
		else {
			// unit must be conformant with meters
			m_spectralCoordinate.setWavelengthUnit(units.toStdString().c_str());
			if ( unitType == OPTICAL_WAVELENGTH ){
				m_spectralCoordinate.frequencyToAirWavelength( coords, coords );
			}
			else {
				m_spectralCoordinate.frequencyToWavelength( coords, coords );
			}
		}
		std::pair<double, double> result( coords[0], coords[1]);
		return result;
	}
}

