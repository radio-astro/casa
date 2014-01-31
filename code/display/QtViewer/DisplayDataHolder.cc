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

#include "DisplayDataHolder.h"
#include <display/QtViewer/QtDisplayData.qo.h>
#include <display/QtViewer/ImageManager/ImageTracker.h>
#include <algorithm>

namespace casa {

	DisplayDataHolder::DisplayDataHolder()
		: imageTracker( NULL ), imageDisplayer(NULL), controlling_dd(NULL) {
	}

//******************************************************************************
//                  Iteration Support
//******************************************************************************
	int DisplayDataHolder::getCount() const {
		return dataList.size();
	}
	bool DisplayDataHolder::isEmpty() const {
		Bool empty = dataList.empty();
		return empty;
	}

	DisplayDataHolder::DisplayDataIterator DisplayDataHolder::beginDD() const {
		return dataList.begin();
	}

	DisplayDataHolder::DisplayDataIterator DisplayDataHolder::endDD() const {
		return dataList.end();
	}


	bool DisplayDataHolder::exists(QtDisplayData* qdd) const {
		bool exists = false;
		if ( dataList.size() > 0 ) {
			DisplayDataIterator iter = dataList.begin();
			while( iter != dataList.end() ) {
				if(qdd == (*iter)) {
					exists = true;
					break;
				}
				iter++;
			}
		}
		return exists;
	}

	void DisplayDataHolder::setDDControlling( QtDisplayData* controlDD ) {
		//The control dd can be just an open image.  It does not even
		//need to be in the list of registered images.
		if ( controlling_dd != controlDD || ( controlDD == NULL && controlling_dd != NULL)) {
			//Important for the statement below to be first so we don't
			//get into an infinite loop.
			controlling_dd = controlDD;
			if ( imageTracker != NULL ) {
				imageTracker->masterImageSelected( controlDD );
			}

			if ( imageDisplayer != NULL ) {
				imageDisplayer->setControllingDD( controlDD );
			}

		}
	}

	void DisplayDataHolder::registrationOrderChanged(){
		if ( imageDisplayer != NULL ){
			imageDisplayer->registrationOrderChanged();
		}
	}

	QtDisplayData* DisplayDataHolder::getChannelDD( int index ) const {
		QtDisplayData* channelDD = NULL;
		if ( dataList.size() > 0 ){
			//By default, the channel dd is the last one in the list
			if ( index < 0 || index >= static_cast<int>(dataList.size())){
				index = dataList.size() - 1;
			}
			DisplayDataIterator iter = dataList.begin();
			int i = 0;
			while (i < index ){
				iter++;
				i++;
			}
			channelDD = (*iter);
		}
		return channelDD;
	}

	QtDisplayData* DisplayDataHolder::getDDControlling( ) const {
		// retrieve the "controlling" DD...
		return controlling_dd;
	}

	bool DisplayDataHolder::isCoordinateMaster(QtDisplayData* other ) const {
		bool coordinateMaster = false;
		if ( controlling_dd != NULL && other == controlling_dd ){
			coordinateMaster = true;
		}
		return coordinateMaster;
	}

	QtDisplayData* DisplayDataHolder::getDD(const std::string& name) const {
		// retrieve DD with given name (0 if none).
		QtDisplayData* qdd = NULL;
		DisplayDataIterator iter = dataList.begin();
		while( iter != dataList.end()) {
			if( (*iter)->name() == name ) {
				qdd = (*iter);
				break;
			}
			iter++;
		}
		return qdd;
	}

	QtDisplayData* DisplayDataHolder::getDD( int index ){
		QtDisplayData* dd = NULL;
		int dataCount = dataList.size();
		if ( 0 <= index && index < dataCount){
			int i = 0;
			DisplayDataIterator iter = dataList.begin();
			while ( iter != dataList.end() ){
				if ( i == index ){
					dd = (*iter);
					break;
				}
				iter++;
				i++;
			}
		}
		return dd;
	}

	QtDisplayData* DisplayDataHolder::getDD(const DisplayData *dd) const {
		// retrieve DD with given name (0 if none).
		QtDisplayData* qdd = NULL;
		DisplayDataIterator iter = dataList.begin();
		while( iter != dataList.end()) {
			if( (*iter)->dd() == dd ) {
				qdd = (*iter);
				break;
			}
			iter++;
		}
		return qdd;
	}

	void DisplayDataHolder::addDD( QtDisplayData* dd, int position,
			bool autoRegister,
			bool masterCoordinate, bool masterSaturation, bool masterHue ) {
		if ( ! exists( dd )) {
			int dataCount = dataList.size();
			if ( position < 0 || position >= dataCount ) {
				dataList.push_back( dd);
			}
			else {
				DisplayDataList::iterator iter = dataList.begin();
				int i = 0;
				while ( i < position  ) {
					iter++;
					i++;
				}
				dataList.insert( iter, dd );
			}
			if ( masterCoordinate ){
				setDDControlling( dd );
			}
			if ( imageTracker != NULL ) {
				imageTracker->imageAdded( dd, position, autoRegister, masterCoordinate, masterSaturation, masterHue );
			}
		}
	}

	void DisplayDataHolder::discardDD( QtDisplayData* dd) {
		//First remove the dd from where it current happens to be.
		//If a higher level is available, we do it through calls to
		//the higher layer.  Otherwise, we do it internally.
		if ( exists( dd )) {
			if ( imageDisplayer != NULL ) {
				imageDisplayer->unregisterDD( dd );
			} else {
				removeDD(dd);
			}
		}
	}

	void DisplayDataHolder::insertDD( QtDisplayData* dd, int position, bool registered ) {
		if ( ! exists(dd) ) {
			//Now put it back in at the proper position.
			if ( imageDisplayer != NULL ) {
				imageDisplayer->registerDD( dd, position );
			} else {
				addDD( dd, position, registered );

			}
		}
		else {
			qDebug() << "DisplayDataHolder: dd="<<dd->name().c_str()<<" was alreading inserted";
		}
	}


	void DisplayDataHolder::removeDDAll() {
		dataList.resize(0);
		controlling_dd = NULL;
	}

	bool DisplayDataHolder::removeDD(QtDisplayData* qdd) {
		bool removed = false;
		if ( controlling_dd == qdd ){
			controlling_dd = NULL;
		}
		if ( qdd != NULL && exists( qdd) ) {
			dataList.erase(std::remove(dataList.begin(), dataList.end(), qdd), dataList.end());
			/*if ( isEmpty() || qdd == controlling_dd ) {
				controlling_dd = NULL;
			}*/

			removed = true;
		}
		return removed;
	}

	void DisplayDataHolder::setImageTracker( ImageTracker* tracker ) {
		imageTracker = tracker;
	}

	void DisplayDataHolder::setImageDisplayer( ImageDisplayer* displayer ) {
		imageDisplayer = displayer;
	}

	DisplayDataHolder::~DisplayDataHolder() {
		// TODO Auto-generated destructor stub
	}

} /* namespace casa */
