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
#include "ImageScrollWidget.qo.h"
#include <display/QtViewer/ImageManager/ImageScroll.qo.h>
#include <display/QtViewer/ImageManager/ImageView.qo.h>
#include <display/QtViewer/DisplayDataHolder.h>
#include <display/QtViewer/QtDisplayData.qo.h>

namespace casa {

	ImageScrollWidget::ImageScrollWidget(QWidget *parent)
		: QWidget(parent) {
		ui.setupUi(this);
		imageScroll = new ImageScroll( this );
		ui.scrollHolder->setWidget( imageScroll );
		connect( imageScroll, SIGNAL(displayDataAdded(QtDisplayData*)),
		         this, SIGNAL(displayDataAdded(QtDisplayData*)));
		connect( imageScroll, SIGNAL(displayDataRemoved(QtDisplayData*)),
		         this, SIGNAL(displayDataRemoved(QtDisplayData*)));
		connect( imageScroll, SIGNAL(displayTypeChanged(ImageView*)),
		         this, SIGNAL(displayTypeChanged(ImageView*)));
		connect( imageScroll, SIGNAL(displayColorsChanged(ImageView*)),
		         this, SIGNAL(displayColorsChanged(ImageView*)));
		connect( imageScroll, SIGNAL(imageOrderingChanged()), this,
				SIGNAL(imageOrderingChanged()));

		connect( ui.clearAllButton, SIGNAL(clicked()), this, SLOT(clearSelections()));
		connect( ui.selectAllButton, SIGNAL(clicked()), this, SLOT(selectAll()));
	}

	void ImageScrollWidget::setImageHolder( DisplayDataHolder* holder ) {
		imageScroll->setImageHolder( holder );
	}

	void ImageScrollWidget::setControllingDD( QtDisplayData* dd ) {
		imageScroll->setControllingDD( dd );
	}

	void ImageScrollWidget::setImageColorsEnabled( bool enabled ) {
		imageScroll->setImageColorsEnabled( enabled );
	}

	void ImageScrollWidget::applyColorChangesIndividually() {
		imageScroll->applyColorChangesIndividually();
	}

	bool ImageScrollWidget::findColor( const QString& lookup, QColor* foundColor ) {
		return imageScroll->findColor( lookup, foundColor );
	}

	void ImageScrollWidget::clearSelections() {
		imageScroll->setSelectAll( false );
	}

	void ImageScrollWidget::selectAll() {
		imageScroll->setSelectAll( true );
	}


	bool ImageScrollWidget::isManaged( QtDisplayData* displayData ) const {
		return imageScroll->isManaged( displayData );
	}

	void ImageScrollWidget::addImageViews( QList<ImageView*>& views ) {

		imageScroll->addImageViews( views );
	}

	void ImageScrollWidget::removeImageViews( QList<ImageView*>& views ) {

		imageScroll->removeImageViews( views );
		//resetScroll();
	}



	QList<ImageView*> ImageScrollWidget::getSelectedViews() {

		return imageScroll->getSelectedViews();
	}




	QList<QtDisplayData*> ImageScrollWidget::closeImages() {

		return imageScroll->closeImages();

	}

	ImageScrollWidget::~ImageScrollWidget() {

	}
}
