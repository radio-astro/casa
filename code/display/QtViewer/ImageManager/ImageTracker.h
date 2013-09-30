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

#ifndef IMAGETRACKER_H_
#define IMAGETRACKER_H_

namespace casa {

	class QtDisplayData;

	/**
	 * Interface class designed to reduce the coupling between the GUI class,
	 * ImageManager and the DisplayDataHolder.  Provides a mechanism for the DisplayDataHolder
	 * to update the GUI, when its QtDisplayData changes methods invoked by other classes.
	 */

	class ImageTracker {
	public:
		virtual void imageAdded( QtDisplayData* image, int position,
				bool autoRegister, bool masterCoordinate,
				bool masterSaturation, bool masterHue) = 0;
		virtual void masterImageSelected( QtDisplayData* image ) =0;
		ImageTracker() {}
		virtual ~ImageTracker() {}

	};

	/**
	 * Interface implemented by QtDisplayPanel that can add/remove
	 * registered DDs and set the controlling QtDisplayData's.
	 *
	 * Methods are called by the DisplayDataHolder to make changes through the
	 * existing infrastructure.
	 */
	class ImageDisplayer {
	public:
		ImageDisplayer() {}
		virtual ~ImageDisplayer() {}
		virtual void setControllingDD( QtDisplayData* controlDD ) = 0;
		virtual void registerDD( QtDisplayData* dd, int position = -1) = 0;
		virtual void unregisterDD( QtDisplayData* dd ) = 0;
		virtual void registrationOrderChanged() = 0;
	};



}
#endif /* IMAGETRACKER_H_ */
