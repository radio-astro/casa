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

#ifndef REGIONBOX_H_
#define REGIONBOX_H_

#include <QList>
#include <QString>
#include <casa/Arrays/Vector.h>

namespace casa {

	class RegionBox {
	public:
		RegionBox(const QList<int> &pixel_x, const QList<int> &pixel_y);
		QString toString(const Vector<int>& imageBLC, const Vector<int>& imageTRC) const;
		QString toStringLabelled(const Vector<int>& imageBLC, const Vector<int>& imageTRC) const;

		void update( const QList<int> & pixelX, const QList<int> & pixelY );
		bool isInBox( double valueX, double valueY ) const;
		virtual ~RegionBox();
	private:
		void clear();
		//Code was crashing when the region box bounds were bigger than the
		//image bounds.  The actualBLC/TRC methods choose smaller region boxes
		//in such cases.
		QVector<int> getActualBLC( const Vector<int>& imageBLC, const Vector<int>& imageTRC, bool* valid) const;
		QVector<int> getActualTRC( const Vector<int>& imageBLC, const Vector<int>& imageTRC, bool* valid ) const;
		bool isInImage( const QVector<int>& coords, const Vector<int>& imageBLC,
		                const Vector<int>& imageTRC ) const;
		void initBox( const QList<int> & pixelX, const QList<int>& pixelY );
		QList<int> regionBoxBLC;
		QList<int> regionBoxTRC;
	};

} /* namespace casa */
#endif /* REGIONBOX_H_ */
