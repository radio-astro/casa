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
#include "DisplayLabel.qo.h"
#include <QDebug>

namespace casa {

DisplayLabel::DisplayLabel(int characterCount, QWidget *parent)
    : QTextEdit(parent), bgColor("#D3D3D3"){
	setTextInteractionFlags( Qt::TextBrowserInteraction );
	setFrameStyle( QFrame::Sunken );
	setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
	setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
	setContextMenuPolicy( Qt::NoContextMenu );
	setAutoFillBackground( true );
	setBackgroundColor();
	setReadOnly( true );
	setFixedHeight( 18 );
	if ( characterCount == 1 ){
		setFixedWidth( 15 );
	}
	else if ( characterCount == 2 ){
		setFixedWidth( 30 );
	}
	else {
		qDebug() << "DisplayLabel::unrecognized character count="<<characterCount;
	}
	setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
}



void DisplayLabel::setBackgroundColor( ){
	QPalette pal = palette();
	pal.setColor( QPalette::Base, bgColor );
	setPalette( pal );
}


bool DisplayLabel::isEmpty() const {
	QTextDocument* contentDoc = document();
	bool emptyDoc = true;
	if ( contentDoc != NULL ){
		emptyDoc = contentDoc->isEmpty();
	}
	return emptyDoc;
}


void DisplayLabel::setEmptyColor( QColor color ){
	bgColor = color;
	setBackgroundColor();
}


DisplayLabel::~DisplayLabel(){

}
}
