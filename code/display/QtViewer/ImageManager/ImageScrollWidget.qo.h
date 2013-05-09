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
#ifndef IMAGESCROLLWIDGET_QO_H
#define IMAGESCROLLWIDGET_QO_H

#include <QtGui/QWidget>
#include <display/QtViewer/ImageManager/ImageScrollWidget.ui.h>

namespace casa {

	class ImageScroll;
	class ImageView;
	class QtDisplayData;
	class DisplayDataHolder;

	/**
	 * Wraps a scroll list of images with the ability to select/deselect them.
	 */

	class ImageScrollWidget : public QWidget {
		Q_OBJECT

	public:
		ImageScrollWidget(QWidget *parent = 0);
		void setImageHolder( DisplayDataHolder* holder );
		void setControllingDD( QtDisplayData* dd );
		QList<ImageView*> getSelectedViews();
		bool isManaged( QtDisplayData* displayData ) const;
		void closeImages();
		void addImageViews( QList<ImageView*>& views );
		void removeImageViews( QList<ImageView*>& view );
		void setImageColorsEnabled( bool enabled );
		void applyColorChangesIndividually();
		bool findColor( const QString& lookup, QColor* foundColor );
		~ImageScrollWidget();
	signals:
		void displayDataRemoved( QtDisplayData* imageData );
		void displayDataAdded( QtDisplayData* imageData );
		void displayTypeChanged( ImageView* imageData );
		void displayColorsChanged( ImageView* imageData );
	private slots:
		void clearSelections();
		void selectAll();
	private:
		ImageScrollWidget( const ImageScrollWidget& other );
		ImageScrollWidget operator=( const ImageScrollWidget& other );
		ImageScroll* imageScroll;
		Ui::ImageScrollWidgetClass ui;
	};

}

#endif // IMAGESCROLLWIDGET_QO_H
