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

#ifndef DISPLAYDATAHOLDER_H_
#define DISPLAYDATAHOLDER_H_

#include <list>
#include <string>
namespace casa {

	class DisplayData;
	class QtDisplayData;
	class ImageTracker;
	class ImageDisplayer;

	/**
	 * Manages and controls access to a lit of QtDisplayDatas.
	 */

	class DisplayDataHolder {
	public:
		DisplayDataHolder();

		//Adds the DisplayData.
		void addDD( QtDisplayData* dd, int position, bool autoRegister,
				bool masterCoordinate = false, bool masterSaturation= false,
				bool masterHue=false);

		// Removes the QDD from the list and deletes it (if it existed --
		// Return value: whether qdd was in the list in the first place).
		bool removeDD(QtDisplayData* qdd);
		void removeDDAll();

		// Insert and discard to basically the same thing as add/remove
		// except that they perform the operation through a GUI level
		// if one is available.
		void insertDD( QtDisplayData* dd, int position, bool registered );
		void discardDD( QtDisplayData* dd );

		//Iteration support
		typedef std::list<QtDisplayData *> DisplayDataList;
		typedef DisplayDataList::const_iterator DisplayDataIterator;
		int getCount() const;
		bool isEmpty() const;
		bool isCoordinateMaster( QtDisplayData* displayData) const;
		DisplayDataIterator beginDD () const;
		DisplayDataIterator endDD () const;

		//Controlling DD
		QtDisplayData* getDDControlling( ) const;
		void setDDControlling( QtDisplayData* controllingDD );

		//Returns the DD that will be animating the channels in
		//normal mode.
		QtDisplayData* getChannelDD( int index ) const;

		// retrieve a DD with given name (0 if none).
		QtDisplayData* getDD(const std::string& name) const;
		QtDisplayData *getDD( const DisplayData *dd ) const;

		// Check that a given DD is on the list.
		bool exists(QtDisplayData* qdd) const;

		void setImageTracker( ImageTracker* tracker );
		void setImageDisplayer( ImageDisplayer* displayer );

		void registrationOrderChanged();
		virtual ~DisplayDataHolder();
	private:
		DisplayDataHolder( const DisplayDataHolder& displayDataHolder );
		DisplayDataHolder operator=( const DisplayDataHolder& displayDataHolder );
		ImageTracker* imageTracker;
		ImageDisplayer* imageDisplayer;
		QtDisplayData *controlling_dd;
		std::list<QtDisplayData*> dataList;
	};

} /* namespace casa */
#endif /* DISPLAYDATAHOLDER_H_ */
