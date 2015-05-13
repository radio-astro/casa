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
#include "CanvasHolder.qo.h"

#include <QFormLayout>
#include <QDebug>

namespace casa {
	CanvasHolder::CanvasHolder(QWidget *parent)
		: QWidget(parent), showLegend( true ),
		  legendLocation( CANVAS_RIGHT),
		  pixelCanvas( this ), legend( NULL ) {
		ui.setupUi(this);
		pixelCanvas.setFocus();

		//White background so the legend will look like a part
		//of the graph.
		QPalette holderPalette = palette();
		holderPalette.setColor( QPalette::Base, Qt::white);
		setAutoFillBackground( true );
		setPalette( holderPalette );
		setBackgroundRole( QPalette::Base );


		changeLegendDisplay();
		connect( &pixelCanvas, SIGNAL(curvesChanged()), this, SLOT( changeLegendDisplay()));
	}

	QList<QString> CanvasHolder::getCurveLabels() {
		int curveCount = pixelCanvas.getLineCount();
		QList<QString> curveLabels;
		for ( int i = 0; i < curveCount; i++ ) {
			curveLabels.append(pixelCanvas.getCurveName(i));
		}
		return curveLabels;
	}

	void CanvasHolder::setCurveLabels( const QList<QString>& curveNames ) {
		int curveCount = curveNames.size();
		bool nameChanged = false;
		for ( int i = 0; i < curveCount; i++ ) {
			QString oldName = pixelCanvas.getCurveName( i );
			if ( oldName != curveNames[i] && !curveNames[i].isEmpty() ) {
				nameChanged = true;
				pixelCanvas.setCurveName(i, curveNames[i] );
			}
		}
		if ( nameChanged ) {
			if ( showLegend ) {
				//We are drawing the legend
				if ( legendLocation != CANVAS ) {
					changeLegendDisplay();
				}
				//Legend is internal to the canvas
				else {
					pixelCanvas.curveLabelsChanged();
				}
			}
		}
	}

	void CanvasHolder::refreshLegend() {
		//Delete the previous legend items.
		delete legend;
		legend = NULL;

		if ( showLegend ) {
			legend = new QWidget( NULL );

			//Put the new legend items in
			QFormLayout* layout = new QFormLayout( legend );
			int curveCount = pixelCanvas.getLineCount();
			for ( int i = 0; i < curveCount; i++ ) {
				QColor itemColor = pixelCanvas.getCurveColor(i);
				QString itemLabel = pixelCanvas.getCurveName( i );

				QLabel* colorLabel = NULL;
				if ( colorBarVisible ) {
					colorLabel = new QLabel( legend );
					QPixmap labelColor( 20, 5 );
					labelColor.fill( itemColor );
					colorLabel->setPixmap( labelColor );
				}

				QLabel* textLabel = new QLabel( legend );
				QString fontColor = "black";
				if ( !colorBarVisible ) {
					fontColor = itemColor.name();
				}
				QString labelTxt("<font color='"+fontColor+"'>"+itemLabel+"</font>");
				textLabel->setText( labelTxt );
				textLabel->setAlignment(Qt::AlignLeft);
				layout->addRow(colorLabel, textLabel );
			}
			legend ->setLayout( layout );
		}
	}

	void CanvasHolder::addLegend( QBoxLayout* innerLayout, QBoxLayout* outerLayout ) {
		innerLayout->addStretch( 1 );
		innerLayout->addWidget( legend );
		innerLayout->addStretch( 1 );
		outerLayout->addLayout( innerLayout );
	}

	void CanvasHolder::addHorizontalLegend( QBoxLayout* verticalLayout ) {
		QHBoxLayout* hLayout = new QHBoxLayout();
		addLegend( hLayout, verticalLayout );
	}

	void CanvasHolder::addVerticalLegend( QBoxLayout* horizontalLayout ) {
		QVBoxLayout* vLayout = new QVBoxLayout();
		addLegend( vLayout, horizontalLayout );
	}

	void CanvasHolder::changeLegendDisplay() {

		//Delete the existing layout.
		refreshLegend();
		QBoxLayout* holderLayout = dynamic_cast<QBoxLayout*>(layout());
		if ( holderLayout != NULL ) {
			holderLayout->removeWidget( &pixelCanvas );
			delete holderLayout;
		}

		//Create a new layout based on the position of the legend
		if ( legendLocation == CANVAS_RIGHT ) {
			QHBoxLayout* holderLayout = new QHBoxLayout( this );
			holderLayout->addWidget( &pixelCanvas );
			if ( showLegend ) {
				addVerticalLegend( holderLayout );
			}
			setLayout( holderLayout );
		} else if ( legendLocation == CANVAS_BELOW ) {
			QVBoxLayout* holderLayout = new QVBoxLayout( this );
			holderLayout->addWidget( &pixelCanvas );
			if ( showLegend ) {
				addHorizontalLegend( holderLayout );
			}
			setLayout( holderLayout );
		}
		//The pixel canvas will show the legend internally (or not)
		else {
			QVBoxLayout* holderLayout = new QVBoxLayout( this );
			holderLayout -> addWidget( &pixelCanvas );
			setLayout( holderLayout );
		}
	}

	QtCanvas* CanvasHolder::getCanvas() {
		return &pixelCanvas;
	}

	void CanvasHolder::setShowLegend( bool showLegend ) {
		if ( showLegend != this->showLegend ) {
			this->showLegend = showLegend;
			changeLegendDisplay();
		}
		pixelCanvas.setShowLegend( showLegend );
	}

	void CanvasHolder::setLegendPosition( int location ) {
		if ( location != legendLocation) {
			legendLocation = static_cast<LegendLocation>(location);
			changeLegendDisplay();
		}
		pixelCanvas.setLegendPosition( location );
	}

	void CanvasHolder::setColorBarVisibility( bool visible ) {
		if ( visible != colorBarVisible ) {
			colorBarVisible = visible;
			changeLegendDisplay();
		}
	}

	CanvasHolder::~CanvasHolder() {
		//delete legend;
		//legend = NULL;
	}
}
