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

#ifndef MOLECULARLINE_H_
#define MOLECULARLINE_H_

#include <QString>
#include <qwt_plot_marker.h>
#include <casa/aips.h>

class QTextStream;

namespace casa {



	class MolecularLine : public QwtPlotMarker {
	public:
		static void setMolecularLineColor( QColor lineColor );

		MolecularLine( float center, float peak, const QString& name,
		               const QString& chemicalName, const QString& resolvedQNs,
		               const QString& frequencyUnits, float originalFrequency );
		virtual int rtti() const;
		void setCenter( float center );
		float getCenter( ) const;
		void setPeak( float peak );
		float getPeak() const;
		void setLabel( const QString& label );
		QString getLabel() const;
		virtual void draw ( QPainter* painter, const QwtScaleMap & xMap,
		                    const QwtScaleMap & yMap, const QRect & canvasRect) const;
		void draw (QPainter * painter, int centerPixel,
		           int peakPixel, int zeroPixel, int width, int height ) const;
		void getMinMax( Double& xmin, Double& xmax, Double& ymin, Double& ymax ) const;
		bool equalTo( const MolecularLine* const other ) const;
		void toStream( QTextStream* stream ) const;
		virtual ~MolecularLine();

	private:
		MolecularLine();
		static QColor lineColor;
		float center;
		float originalFrequency;
		float peak;
		QString label;
		QString resolvedQNs;
		QString chemicalName;
		QString frequencyUnits;

		void init();
	};

} /* namespace casa */
#endif /* MOLECULARLINE_H_ */
