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
#ifndef IMAGESCROLL_QO_H
#define IMAGESCROLL_QO_H

#include <display/QtViewer/ImageManager/ImageTracker.h>
#include <QtGui/QWidget>

class QDragMoveEvent;
class QDragEnterEvent;
class QDropEvent;
class QMimeData;
class QSpacerItem;

namespace casa {

class ImageView;
class QtDisplayData;
class DisplayDataHolder;

/**
 * Displays a list of images, allowing the user to reorder them.
 */

class ImageScroll : public QWidget, public ImageTracker {
    Q_OBJECT

public:
    ImageScroll(QWidget *parent = 0);
    void setImageHolder( DisplayDataHolder* holder );
    void setControllingDD( QtDisplayData* dd );
    QList<ImageView*> getSelectedViews();

    bool isManaged( QtDisplayData* displayData ) const;
    void closeImages();
    void addImageViews( QList<ImageView*>& views );
    void removeImageViews( QList<ImageView*>& view );
    void setSelectAll( bool selectAll);
    void setImageColorsEnabled( bool enabled );
    void applyColorChangesIndividually();
    bool findColor( const QString& lookup, QColor* foundColor );
    virtual ~ImageScroll();

    //Image Tracker Interface
    virtual void masterImageSelected( QtDisplayData* image );
    virtual void imageAdded( QtDisplayData* image );
    virtual void imageRemoved( QtDisplayData* image );

signals:
	void displayDataRemoved( QtDisplayData* imageData );
	void displayDataAdded( QtDisplayData* imageData );
	void displayTypeChanged( ImageView* displayData );
	void displayColorsChanged( ImageView* displayData );

protected:
	void dragEnterEvent( QDragEnterEvent* enterEvent );
    void dropEvent( QDropEvent* dropEvent );
    void dragMoveEvent( QDragMoveEvent* dragMoveEvent );

private:
    ImageScroll( const ImageScroll& other );
    ImageScroll operator=( const ImageScroll& other );
    void addImage( ImageView* imageView );
    void closeImage( ImageView* imageView, bool deleteImage = true );
    ImageView* findImageView( const QString& name );
    ImageView* getMimeImageView( const QMimeData* mimeData );
    QSpacerItem* spacer;
    const int LAYOUT_SPACING;
    const int LAYOUT_MARGIN;
    QList<ImageView*> images;
    DisplayDataHolder* managedImages;
    bool imageColorsEnabled;
};

}

#endif // IMAGESCROLL_QO_H
