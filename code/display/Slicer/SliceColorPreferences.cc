//# Copyright (C) 1994,1995,1996,1997,1998,1999,2000
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
#include "SliceColorPreferences.qo.h"
#include <QColorDialog>
#include <QMessageBox>

namespace casa {

const QString SliceColorPreferences::APPLICATION = "1D Slice Tool";
const QString SliceColorPreferences::ORGANIZATION = "NRAO/CASA";
const QString SliceColorPreferences::SLICE_COLOR = "Slice Color";
const QString SliceColorPreferences::ACCUMULATED_COLOR = "Accumulated Color";
const QString SliceColorPreferences::ACCUMULATED_COLOR_COUNT = "Accumulated Slice Color Count";
const QString SliceColorPreferences::VIEWER_COLORS = "Viewer Colors";

SliceColorPreferences::SliceColorPreferences(QWidget *parent)
    : QDialog(parent)
{
	ui.setupUi(this);
	setWindowTitle( "1D Slice Color Preferences");
	sliceColor=Qt::black;
	viewerColors = true;
	accumulateColorList << "#40E0D0" << "#FF69B4" << "#00FF00" << "#FF8C00" << "#BA55D3" <<
			"#A52A2A" << "#808080" << "#FF6347" << "#FFFF00";
	ui.accumulatedColorList->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.accumulatedColorList->setSelectionMode( QAbstractItemView::SingleSelection );

	initializeUserColors();
	resetColors();
	ui.viewerColorsCheckBox->setChecked( viewerColors );
	useViewerColorsChanged( viewerColors );

	connect( ui.viewerColorsCheckBox, SIGNAL(toggled(bool)), this, SLOT(useViewerColorsChanged(bool)));
	connect( ui.sliceColorButton, SIGNAL(clicked()), this, SLOT(selectSliceColor()));
	connect( ui.addButton, SIGNAL(clicked()), this, SLOT(addColorAccumulated()));
	connect( ui.deleteButton, SIGNAL(clicked()), this, SLOT(removeColorAccumulated()));
	connect( ui.okButton, SIGNAL(clicked()), this, SLOT(colorsAccepted()));
	connect( ui.cancelButton, SIGNAL(clicked()), this, SLOT(colorsRejected()));
}

void SliceColorPreferences::useViewerColorsChanged( bool viewerColors ){
	ui.sliceColorButton->setEnabled( !viewerColors );
	ui.accumulatedColorList->setEnabled( !viewerColors );
	ui.addButton->setEnabled( !viewerColors );
	ui.deleteButton->setEnabled( !viewerColors );
}

bool SliceColorPreferences::isViewerColors() const {
	return viewerColors;
}

QColor SliceColorPreferences::getSliceColor() const {
	return sliceColor;
}

QColor SliceColorPreferences::getButtonColor( QPushButton* button ) const {
	QPalette p = button->palette();
	QBrush brush = p.brush(QPalette::Button );
	QColor backgroundColor = brush.color();
	return backgroundColor;
}

QList<QColor> SliceColorPreferences::getAccumulatedSliceColors() const {
	QList<QColor> copy;
	for ( int i = 0; i < accumulateColorList.size(); i++  ){
		QColor color = QColor( accumulateColorList[i]);
		copy.append( color );
	}
	return copy;
}

void SliceColorPreferences::showColorDialog( QPushButton* source ){
	QColor initialColor = getButtonColor( source );
	QColor selectedColor = QColorDialog::getColor( initialColor, this );
	if ( selectedColor.isValid() ){
		setButtonColor( source, selectedColor );
	}
}

void SliceColorPreferences::selectSliceColor(){
	showColorDialog( ui.sliceColorButton );
}

void SliceColorPreferences::initializeUserColors(){
	//Only use the default values passed in if the user has not indicated
	//any preferences.
	QSettings settings( ORGANIZATION, APPLICATION );

	viewerColors = settings.value( VIEWER_COLORS, viewerColors ).toBool();

	QString sliceColorName = readCustomColor( settings, SLICE_COLOR, sliceColor.name() );
	if ( sliceColorName.length() > 0 ){
		sliceColor = QColor( sliceColorName );
	}

	//If the user has specified an accumulated color scheme, read it in.
	if ( settings.contains( ACCUMULATED_COLOR_COUNT) ) {
		readCustomColor( settings, ACCUMULATED_COLOR_COUNT, ACCUMULATED_COLOR, accumulateColorList );
	}
}

void SliceColorPreferences::readCustomColor( QSettings& settings,
		const QString& countKey, const QString& baseLookup,
		QList<QString>& colorList ){
	colorList.clear();
	int colorCount = settings.value( countKey, 0 ).toInt();
	for ( int i = 0; i < colorCount; i++ ){
		QString lookupStr = baseLookup + QString::number(i);
		QString colorName = settings.value( lookupStr ).toString();
		colorList.append( colorName );
	}
}

QString SliceColorPreferences::readCustomColor( QSettings& settings,
		const QString& identifier, const QString& defaultColor){
	QString colorName = settings.value( identifier, defaultColor ).toString();
	return colorName;
}

void SliceColorPreferences::resetColors(){
	setButtonColor( ui.sliceColorButton, sliceColor );
	populateAccumulateColors();
}

void SliceColorPreferences::setButtonColor( QPushButton* button, QColor color ){
	QPalette p = button->palette();
	p.setBrush(QPalette::Button, color);
	button->setPalette( p );
}

void SliceColorPreferences::populateAccumulateColors( ){
	ui.accumulatedColorList->clear();
	for( int i = 0; i < accumulateColorList.size(); i++ ){
		QColor itemColor;
		itemColor.setNamedColor( accumulateColorList[i] );
		addColorListItem( ui.accumulatedColorList, itemColor );
	}
}

void SliceColorPreferences::addColorListItem( QListWidget* list, const QColor& listColor ){
	QListWidgetItem* listItem = new QListWidgetItem( list  );
	listItem->setBackground( listColor );
	list->addItem( listItem );
}

void SliceColorPreferences::addColorAccumulated(){
	QColor selectedColor = QColorDialog::getColor( Qt::white, this );
	if ( selectedColor.isValid() ){
		addColorListItem( ui.accumulatedColorList, selectedColor );
			//registerColorChange();
	}
}

void SliceColorPreferences::removeColorAccumulated(){
	int row = ui.accumulatedColorList->currentRow();
	if ( row >= 0 ){
		QListWidgetItem* listItem = ui.accumulatedColorList->takeItem( row );
		if ( listItem != NULL ){
			delete listItem;
		}
	}
	else {
		QString msg( "Please select a color in the list to delete.");
		QMessageBox::warning( this, "No Color Selected", msg );
	}
}


void SliceColorPreferences::colorsAccepted(){
	persistColors();
	QDialog::close();
	emit colorsChanged();
}

void SliceColorPreferences::colorsRejected(){
	resetColors();
	QDialog::close();
}

void SliceColorPreferences::persistColors(){
	//Copy the colors from the buttons into color variables

	//Save the colors in the persistent settings.
	QSettings settings( ORGANIZATION, APPLICATION );
	settings.clear();

	//Use viewer colors
	viewerColors = ui.viewerColorsCheckBox->isChecked();
	settings.setValue( VIEWER_COLORS, viewerColors );

	//Main slice color
	sliceColor = getButtonColor( ui.sliceColorButton );
	settings.setValue( SLICE_COLOR, sliceColor.name() );

	//Accumulated slice colors
	accumulateColorList.clear();
	int colorCount = ui.accumulatedColorList->count();
	settings.setValue( ACCUMULATED_COLOR_COUNT, colorCount );
	for ( int i = 0; i < colorCount; i++ ){
		QString persistStr = ACCUMULATED_COLOR + QString::number(i);
		QListWidgetItem* listItem = ui.accumulatedColorList->item(i);
		if ( listItem != NULL ){
			QColor listColor = listItem->backgroundColor();
			QString colorStr = listColor.name();
			settings.setValue( persistStr, colorStr );
			accumulateColorList.append( colorStr );
		}
	}
}

SliceColorPreferences::~SliceColorPreferences()
{

}
}
