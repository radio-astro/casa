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
#include "ColorSummaryWidget.qo.h"
#include <display/QtPlotter/QtCanvas.qo.h>
#include <display/QtPlotter/ColorSummaryDelegate.h>
#include <display/QtPlotter/MolecularLine.h>
#include <display/QtPlotter/GaussianEstimateWidget.qo.h>
#include <display/QtPlotter/Util.h>

#include <QColorDialog>
#include <QDebug>

namespace casa {

	const QString ColorSummaryWidget::CUSTOM_PROFILE_COLOR = "Custom Profile Color";
	const QString ColorSummaryWidget::CUSTOM_FIT_COLOR = "Custom Fit Color";
	const QString ColorSummaryWidget::CUSTOM_SUMMARY_COLOR = "Custom Summary Color";
	const QString ColorSummaryWidget::CUSTOM_PROFILE_COLOR_COUNT = "Custom Profile Color Count";
	const QString ColorSummaryWidget::CUSTOM_FIT_COLOR_COUNT = "Custom Fit Color Count";
	const QString ColorSummaryWidget::CUSTOM_SUMMARY_COLOR_COUNT = "Custom Summary Color Count";
	const QString ColorSummaryWidget::COLOR_SCHEME_PREFERENCE = "Color Scheme Preference";
	const QString ColorSummaryWidget::CHANNEL_LINE_COLOR = "Channel Line Color";
	const QString ColorSummaryWidget::MOLECULAR_LINE_COLOR = "Molecular Line Color";
	const QString ColorSummaryWidget::INITIAL_GAUSSIAN_ESTIMATE_COLOR = "Initial Gaussian Estimate Color";
	const QString ColorSummaryWidget::ZOOM_RECT_COLOR = "Zoom Rectangle Color";

	ColorSummaryWidget::ColorSummaryWidget(QWidget *parent)
		: QDialog(parent), traditionalChange( false ), alternativeChange( false ) {
		ui.setupUi(this);
		setWindowTitle( "Curve Color Preferences");

		connect( ui.addProfileCurveButton, SIGNAL(clicked()), this, SLOT(addColorProfile()));
		connect( ui.addFitCurveButton, SIGNAL(clicked()), this, SLOT(addColorFit()));
		connect( ui.addSummaryFitCurveButton, SIGNAL(clicked()), this, SLOT(addColorFitSummary()));

		connect( ui.okButton, SIGNAL(clicked()), this, SLOT(accept()));
		connect( ui.cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

		connect( ui.deleteProfileCurveButton, SIGNAL(clicked()), this, SLOT(removeColorProfile()));
		connect( ui.deleteFitCurveButton, SIGNAL(clicked()), this, SLOT(removeColorFit()));
		connect( ui.deleteSummaryFitCurveButton, SIGNAL(clicked()), this, SLOT(removeColorFitSummary()));

		connect( ui.traditionalRadioButton, SIGNAL(clicked()), this, SLOT(colorSchemeChanged()));
		connect( ui.alternativeRadioButton, SIGNAL(clicked()), this, SLOT(colorSchemeChanged()));
		connect( ui.customRadioButton, SIGNAL(clicked()), this, SLOT(colorSchemeChanged()));

		connect( ui.channelLineColorButton, SIGNAL(clicked()), this, SLOT(channelLineColorChanged()));
		connect (ui.molecularLineColorButton, SIGNAL(clicked()), this, SLOT(molecularLineColorChanged()));
		connect( ui.initialGaussianEstimateColorButton, SIGNAL(clicked()), this, SLOT(initialGaussianEstimateColorChanged()));
		connect( ui.zoomRectColorButton, SIGNAL(clicked()), this, SLOT(zoomRectColorChanged()));

		//Initialize properties of the list
		ui.profileCurveList->setSelectionBehavior(QAbstractItemView::SelectRows);
		ui.profileCurveList->setSelectionMode( QAbstractItemView::SingleSelection );
		ui.fitCurveList->setSelectionBehavior(QAbstractItemView::SelectRows);
		ui.fitCurveList->setSelectionMode( QAbstractItemView::SingleSelection );
		ui.fitCurveSummaryList->setSelectionBehavior(QAbstractItemView::SelectRows);
		ui.fitCurveSummaryList->setSelectionMode( QAbstractItemView::SingleSelection );
		ui.profileCurveList->setItemDelegate( new ColorSummaryDelegate( this ));
		ui.fitCurveList->setItemDelegate(  new ColorSummaryDelegate( this ));
		ui.fitCurveSummaryList->setItemDelegate( new ColorSummaryDelegate( this ));

		//Read in the color schemes
		initializePresetColors();

		//Read in any persistant colors the user might have set.
		initializeUserColors();

		//Display the appropriate colors in the three
		//GUI lists.
		populateColorLists();


	}

	void ColorSummaryWidget::initializePresetColors() {
		mainCurveColorList << "#0000FF" <<  "#9102D4" << "#BA81D4"<< "#000099"<< "#0000D4";
		fitCurveColorList <<  "#CA5F00"<<"#8A6C00" << "#B08A00" << "#D6A800" <<
		                  "#A34C00"  << "#7F3A00";
		fitSummaryCurveColorList << "#00957B" <<"#00E0BB" << "#006400" << "#00B000" << "#5C8A00" << "#75B000" <<
		                         "#426300" << "#00E025" << "#00BA1F" << "#96E000";

		traditionalCurveColorList << "red" << "blue" << "green" << "cyan" << "lightGray" <<
		                          "magenta" << "yellow" << "darkRed" << "darkBlue" << "darkGreen" <<
		                          "darkCyan" << "darkGray" << "darkMagenta" << "gold" << "gray";
		channelLineColor = Qt::cyan;
		molecularLineColor = "#00957B";
		initialGaussianEstimateColor = "#CA5F00";
		zoomRectColor = Qt::yellow;
	}

	void ColorSummaryWidget::initializeUserColors() {
		//Read information from the persistent color settings
		QSettings settings( Util::ORGANIZATION, Util::APPLICATION );

		//If the user has a custom color scheme, read it in.
		if ( settings.contains( CUSTOM_PROFILE_COLOR_COUNT) ||
		        settings.contains( CUSTOM_FIT_COLOR_COUNT) ||
		        settings.contains( CUSTOM_SUMMARY_COLOR_COUNT )) {
			//Read in the custom colors set by the user.
			readCustomColor( settings, CUSTOM_PROFILE_COLOR_COUNT, CUSTOM_PROFILE_COLOR, customMainList );
			readCustomColor( settings, CUSTOM_FIT_COLOR_COUNT, CUSTOM_FIT_COLOR, customFitList );
			readCustomColor( settings, CUSTOM_SUMMARY_COLOR_COUNT, CUSTOM_SUMMARY_COLOR, customFitSummaryList );
		}

		//Set the color scheme radio buttons according to the scheme
		//preference indicated by the user.
		if ( settings.contains( COLOR_SCHEME_PREFERENCE ) ) {
			int schemePreference = settings.value( COLOR_SCHEME_PREFERENCE ).toInt();
			if ( schemePreference == TRADITIONAL ) {
				ui.traditionalRadioButton->setChecked( true );
			} else if ( schemePreference == ALTERNATIVE ) {
				ui.alternativeRadioButton->setChecked( true );
			} else {
				ui.customRadioButton->setChecked( true );
			}
		} else {
			ui.traditionalRadioButton->setChecked( true );
		}
		QString channelLineColorStr = settings.value( CHANNEL_LINE_COLOR, channelLineColor.name()).toString();
		channelLineColor.setNamedColor( channelLineColorStr );
		setLabelColor( ui.channelLineColorLabel, channelLineColorStr );

		QString molecularLineColorStr = settings.value( MOLECULAR_LINE_COLOR, molecularLineColor.name()).toString();
		molecularLineColor.setNamedColor( molecularLineColorStr );
		setLabelColor( ui.molecularLineColorLabel, molecularLineColorStr );

		QString initialGaussianColorStr = settings.value( INITIAL_GAUSSIAN_ESTIMATE_COLOR, initialGaussianEstimateColor.name()).toString();
		initialGaussianEstimateColor.setNamedColor( initialGaussianColorStr );
		setLabelColor( ui.initialGaussianEstimateColorLabel, initialGaussianColorStr );

		QString zoomRectColorStr = settings.value( ZOOM_RECT_COLOR, zoomRectColor.name()).toString();
		zoomRectColor.setNamedColor( zoomRectColorStr );
		setLabelColor( ui.zoomRectColorLabel, zoomRectColorStr );
	}

	void ColorSummaryWidget::readCustomColor( QSettings& settings,
	        const QString& countKey, const QString& baseLookup, QList<QString>& colorList ) {
		int colorCount = settings.value( countKey, 0 ).toInt();
		for ( int i = 0; i < colorCount; i++ ) {
			QString lookupStr = baseLookup + QString::number(i);
			QString colorName = settings.value( lookupStr ).toString();
			colorList.append( colorName );
		}
	}

	void ColorSummaryWidget::setColorCanvas( QtCanvas* colorCanvas ) {
		pixelCanvas = colorCanvas;

		//Tell the pixel canvas about the initial colors we are using.
		pixelCanvasColorChange();
	}

//--------------------------------------------------------------------------
//                      Adding Colors
//-------------------------------------------------------------------------

	void ColorSummaryWidget::addColorListItem( QListWidget* list, const QColor& listColor ) {
		QListWidgetItem* listItem = new QListWidgetItem( list  );
		listItem->setBackground( listColor );
		list->addItem( listItem );
	}

	void ColorSummaryWidget::addColorFitSummary() {
		addColor( ui.fitCurveSummaryList );

	}

	void ColorSummaryWidget::addColorFit() {
		addColor( ui.fitCurveList );

	}

	void ColorSummaryWidget::addColorProfile() {
		addColor( ui.profileCurveList );

	}

	void ColorSummaryWidget::addColor( QListWidget* list ) {
		QColor selectedColor = QColorDialog::getColor( Qt::white, this );
		if ( selectedColor.isValid() ) {
			addColorListItem( list, selectedColor );
			registerColorChange();
		}
	}

	void ColorSummaryWidget::setLabelColor( QLabel* colorWidget, QString colorName  ) {
		QString styleColor =  "QLabel { background-color: " + colorName + "; }";
		colorWidget->setStyleSheet( styleColor );
	}

//---------------------------------------------------------------------
//          Removing colors
//---------------------------------------------------------------------

	void ColorSummaryWidget::removeColorProfile() {
		removeColor( ui.profileCurveList );
	}

	void ColorSummaryWidget::removeColorFit() {
		removeColor( ui.fitCurveList );
	}

	void ColorSummaryWidget::removeColorFitSummary() {
		removeColor( ui.fitCurveSummaryList );
	}

	void ColorSummaryWidget::removeColor( QListWidget* list ) {
		int row = list->currentRow();
		if ( row >= 0 ) {
			QListWidgetItem* listItem = list->takeItem( row );
			if ( listItem != NULL ) {
				delete listItem;
			}
			registerColorChange();
		} else {
			QString msg( "Please select a color in the list to delete.");
			Util::showUserMessage( msg, this );
		}
	}

//----------------------------------------------------------------------
//                        Slots
//----------------------------------------------------------------------

	void ColorSummaryWidget::colorSchemeChanged() {
		populateColorLists();
		bool editable = false;
		if ( ui.customRadioButton->isChecked() ) {
			editable = true;
		}
		ui.addProfileCurveButton->setVisible( editable );
		ui.deleteProfileCurveButton->setVisible( editable );
		ui.addFitCurveButton->setVisible( editable );
		ui.deleteFitCurveButton->setVisible( editable );
		ui.addSummaryFitCurveButton->setVisible( editable );
		ui.deleteSummaryFitCurveButton->setVisible( editable );
	}


	void ColorSummaryWidget::channelLineColorChanged() {
		QColor selectedColor = QColorDialog::getColor( channelLineColor, this );
		if ( selectedColor.isValid() ) {
			channelLineColor = selectedColor;
			setLabelColor( ui.channelLineColorLabel, selectedColor.name() );
		}
	}

	void ColorSummaryWidget::zoomRectColorChanged() {
		QColor selectedColor = QColorDialog::getColor( zoomRectColor, this );
		if ( selectedColor.isValid() ) {
			zoomRectColor = selectedColor;
			setLabelColor( ui.zoomRectColorLabel, selectedColor.name() );
		}
	}

	void ColorSummaryWidget::molecularLineColorChanged() {
		QColor selectedColor = QColorDialog::getColor( molecularLineColor, this );
		if ( selectedColor.isValid() ) {
			molecularLineColor = selectedColor;
			setLabelColor( ui.molecularLineColorLabel, selectedColor.name() );
		}
	}

	void ColorSummaryWidget::initialGaussianEstimateColorChanged() {
		QColor selectedColor = QColorDialog::getColor( initialGaussianEstimateColor, this );
		if ( selectedColor.isValid() ) {
			initialGaussianEstimateColor = selectedColor;
			setLabelColor( ui.initialGaussianEstimateColorLabel, selectedColor.name() );
		}
	}

//-------------------------------------------------------------------------
//                    Private Utility
//------------------------------------------------------------------------

	void ColorSummaryWidget::clearColorLists() {
		ui.profileCurveList->clear();
		ui.fitCurveList->clear();
		ui.fitCurveSummaryList->clear();
	}

	void ColorSummaryWidget::populateColorList( const QList<QString>& colors,
	        QListWidget* list ) {
		for( int i = 0; i < colors.size(); i++ ) {
			QColor itemColor;
			itemColor.setNamedColor( colors[i] );
			addColorListItem( list, itemColor );
		}
	}

	void ColorSummaryWidget::populateColorLists() {
		clearColorLists();
		if ( ui.traditionalRadioButton->isChecked() ) {
			populateColorList( traditionalCurveColorList, ui.profileCurveList );
		} else if ( ui.alternativeRadioButton->isChecked() ) {
			populateColorList( mainCurveColorList, ui.profileCurveList );
			populateColorList( fitCurveColorList, ui.fitCurveList );
			populateColorList( fitSummaryCurveColorList, ui.fitCurveSummaryList );
		} else {
			populateColorList( customMainList, ui.profileCurveList );
			populateColorList( customFitList, ui.fitCurveList );
			populateColorList( customFitSummaryList, ui.fitCurveSummaryList );
		}
	}


	void ColorSummaryWidget::registerColorChange() {
		if ( ui.traditionalRadioButton->isChecked() ) {
			traditionalChange = true;
		} else if ( ui.alternativeRadioButton->isChecked() ) {
			alternativeChange = true;
		}
	}

	void ColorSummaryWidget::clearColorChange() {
		traditionalChange = false;
		alternativeChange = false;
	}

//----------------------------------------------------------------------------
//                    Closing the Dialog
//----------------------------------------------------------------------------

	void ColorSummaryWidget::persistColorList( QSettings& settings, QListWidget* list,
	        const QString& baseStr, const QString& countStr ) {
		int colorCount = list->count();
		settings.setValue( countStr, colorCount );
		for ( int i = 0; i < colorCount; i++ ) {
			QString persistStr = baseStr + QString::number(i);
			QListWidgetItem* listItem = list->item(i);
			if ( listItem != NULL ) {
				QColor listColor = listItem->backgroundColor();
				QString colorStr = listColor.name();
				settings.setValue( persistStr, colorStr );
			}
		}
	}

	void ColorSummaryWidget::pixelCanvasColorChange() {

		if ( ui.customRadioButton->isChecked() ) {
			pixelCanvas->setMainCurveColors( customMainList );
			pixelCanvas->setFitCurveColors( customFitList );
			pixelCanvas->setSummaryCurveColors( customFitSummaryList );
		} else if ( ui.alternativeRadioButton->isChecked() ) {
			pixelCanvas->setMainCurveColors( mainCurveColorList);
			pixelCanvas->setFitCurveColors( fitCurveColorList );
			pixelCanvas->setSummaryCurveColors( fitSummaryCurveColorList );
		}
		pixelCanvas->setTraditionalCurveColors( traditionalCurveColorList);
		bool traditionalColors = ui.traditionalRadioButton->isChecked();
		pixelCanvas->setTraditionalColors( traditionalColors );

		pixelCanvas->setChannelLineColor( channelLineColor );
		pixelCanvas->setZoomRectColor( zoomRectColor );
		GaussianEstimateWidget::setEstimateColor( initialGaussianEstimateColor );
		MolecularLine::setMolecularLineColor( molecularLineColor );
		pixelCanvas->curveColorsChanged();
	}

	void ColorSummaryWidget::copyViewList(QListWidget* listWidget, QList<QString>& canvasList) {
		canvasList.clear();
		for ( int i = 0; i < listWidget->count(); i++ ) {
			QListWidgetItem* listItem = listWidget->item(i);
			if ( listItem != NULL ) {
				QColor listColor = listItem->backgroundColor();
				QString colorStr = listColor.name();
				canvasList.append( colorStr );
			}
		}
	}

	void ColorSummaryWidget::copyViewLists() {
		if ( ui.customRadioButton->isChecked() ) {
			copyViewList( ui.profileCurveList, customMainList );
			copyViewList( ui.fitCurveList, customFitList );
			copyViewList( ui.fitCurveSummaryList, customFitSummaryList );
		} else if ( ui.alternativeRadioButton->isChecked() ) {
			copyViewList( ui.profileCurveList, mainCurveColorList );
			copyViewList( ui.fitCurveList, fitCurveColorList );
			copyViewList( ui.fitCurveSummaryList, fitSummaryCurveColorList );
		}
	}

	void ColorSummaryWidget::accept() {
		//Copy the display lists into the lists of string color names we will
		//pass to the canvas.
		copyViewLists();
		//Tell the canvas about the color changes
		pixelCanvasColorChange();
		//Persist the color changes
		persist();
		//Reset the variable marking color changes
		clearColorChange();
		//Close the dialog
		this->hide();
	}

	void ColorSummaryWidget::persist() {
		//Set the color scheme radio buttons according to the scheme
		//preference indicated by the user.
		QSettings settings( Util::ORGANIZATION, Util::APPLICATION );
		settings.clear();

		//No changes to the traditional scheme
		if ( ui.traditionalRadioButton->isChecked() && !traditionalChange ) {
			settings.setValue( COLOR_SCHEME_PREFERENCE, TRADITIONAL );
		}
		//No change to the alternative scheme
		else if ( ui.alternativeRadioButton->isChecked() && !alternativeChange ) {
			settings.setValue( COLOR_SCHEME_PREFERENCE, ALTERNATIVE );
		}
		//Either they have chosen a custom scheme explicitly, or they have
		//made changes to one of the existing schemes.  In any case, we will
		//save the changes under a custom scheme and make that their preference.
		else {
			settings.setValue( COLOR_SCHEME_PREFERENCE, CUSTOM );
			persistColorList( settings, ui.profileCurveList, CUSTOM_PROFILE_COLOR, CUSTOM_PROFILE_COLOR_COUNT );
			persistColorList( settings, ui.fitCurveList, CUSTOM_FIT_COLOR, CUSTOM_FIT_COLOR_COUNT );
			persistColorList( settings, ui.fitCurveSummaryList, CUSTOM_SUMMARY_COLOR, CUSTOM_SUMMARY_COLOR_COUNT );
		}
		settings.setValue( CHANNEL_LINE_COLOR, channelLineColor.name());
		settings.setValue( MOLECULAR_LINE_COLOR, molecularLineColor.name());
		settings.setValue( ZOOM_RECT_COLOR, zoomRectColor.name());
		settings.setValue( INITIAL_GAUSSIAN_ESTIMATE_COLOR, initialGaussianEstimateColor.name());
	}

	void ColorSummaryWidget::reject() {
		//Read in the old user custom colors
		customMainList.clear();
		customFitList.clear();
		customFitSummaryList.clear();

		initializeUserColors();
		//Put the old colors back into the list
		populateColorLists();
		//Reset variables indicating that color changes
		//have taken place
		clearColorChange();
		//Close the dialog
		this->hide();
	}




	ColorSummaryWidget::~ColorSummaryWidget() {
	}
}
