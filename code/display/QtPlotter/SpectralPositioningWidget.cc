#include "SpectralPositioningWidget.qo.h"
#include <display/QtPlotter/Util.h>
#include <display/QtPlotter/ProfileTaskMonitor.h>
#include <display/Display/DisplayCoordinateSystem.h>
#include <images/Images/ImageInterface.h>
#include <casa/Logging/LogIO.h>
#include <casa/Quanta/MVAngle.h>
#include <assert.h>

#include <QDebug>

namespace casa {

	SpectralPositioningWidget::SpectralPositioningWidget(QWidget *parent)
		: QWidget( parent ), profileTaskMonitor( NULL ), logger( NULL ),
		  pixelXValues(2), pixelYValues(2), worldXValues(2), worldYValues(2) {
		ui.setupUi(this);

		initSpectrumPosition();
	}

//-----------------------------------------------------------------------------
//                       Initialization
//-----------------------------------------------------------------------------

	void SpectralPositioningWidget::setTaskMonitor( ProfileTaskMonitor* monitor ) {
		profileTaskMonitor = monitor;
	}

	void SpectralPositioningWidget::setLogger( LogIO* logger ) {
		this->logger = logger;
	}

	void SpectralPositioningWidget::initSpectrumPosition() {
		std::map<PositionTypeIndex,QString> positionTypeMap;
		positionTypeMap[POINT]="Point";
		positionTypeMap[BOX] = "Rectangle";
		for ( int i = POINT; i < END_POSITION_TYPE; i++ ) {
			ui.positionTypeComboBox -> addItem( positionTypeMap[static_cast<PositionTypeIndex>(i)] );
		}

		std::map<UnitIndex, QString> unitMap;
		unitMap[RADIAN]="RA/DEC";
		unitMap[PIXEL]="Pixel";
		for ( int i = RADIAN; i < END_UNIT; i++  ) {
			ui.unitComboBox -> addItem( unitMap[static_cast<UnitIndex>(i)] );
		}

		ui.stackedWidget->setCurrentIndex(POINT_RA_DEC);
		ui.boxSpecCombo -> setVisible( false );
		ui.boxSpecLabel -> setVisible( false );

		std::map<BoxSpecificationIndex,QString> boxSpecificationMap;
		boxSpecificationMap[TL_LENGTH_HEIGHT] = "Top Left Corner, Length, Height";
		boxSpecificationMap[CENTER_LENGTH_HEIGHT] = "Center, Length, Height";
		boxSpecificationMap[TL_BR]="Top Left Corner, Bottom Right Corner";
		boxSpecificationMap[BL_TR]="Bottom Left Corner, Top Right Corner";
		for ( int i = TL_LENGTH_HEIGHT; i < END_SPEC; i++ ) {
			ui.boxSpecCombo -> addItem( boxSpecificationMap[static_cast<BoxSpecificationIndex>(i)]);
		}

		connect( ui.positionTypeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(locationSelectionTypeChanged(int)));
		connect( ui.unitComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(locationUnitsChanged(int)));
		connect( ui.applyPositionButton, SIGNAL(clicked()), this, SLOT(setPosition()));
		connect( ui.boxSpecCombo, SIGNAL( currentIndexChanged(int)), this, SLOT(boxSpecChanged(int)));

		//Initialize the box label map
		QList<QString> labelTlcWidthLengthList;
		QList<QString> labelCenterWidthLengthList;
		QList<QString> labelTlcBrcList;
		QList<QString> labelBlcTrcList;
		labelTlcWidthLengthList << Util::toHTML("TLC X:") << Util::toHTML("TLC Y:") << Util::toHTML("Length:") << Util::toHTML("Height:");
		labelCenterWidthLengthList << Util::toHTML("Center X:") << Util::toHTML("Center Y:") << Util::toHTML("Length:") << Util::toHTML("Height:");
		labelTlcBrcList << Util::toHTML("TLC X:") << Util::toHTML("TLC Y:") << Util::toHTML("BRC X:") << Util::toHTML("BRC Y:");
		labelBlcTrcList << Util::toHTML("BLC X:") << Util::toHTML("BLC Y:") << Util::toHTML("TRC X:") << Util::toHTML("TRC Y:");
		QList<QString> labelTlcWidthLengthListW;
		QList<QString> labelCenterWidthLengthListW;
		QList<QString> labelTlcBrcListW;
		QList<QString> labelBlcTrcListW;
		labelTlcWidthLengthListW << Util::toHTML("TLC RA:") << Util::toHTML("TLC DEC:") << Util::toHTML("Length:") << Util::toHTML("Height:");
		labelCenterWidthLengthListW << Util::toHTML("Center RA:") << Util::toHTML("Center DEC:") << Util::toHTML("Length:") << Util::toHTML("Height:");
		labelTlcBrcListW << Util::toHTML("TLC RA:") << Util::toHTML("TLC DEC:") << Util::toHTML("BRC RA:") << Util::toHTML("BRC DEC:");
		labelBlcTrcListW << Util::toHTML("BLC RA:") << Util::toHTML("BLC DEC:") << Util::toHTML("TRC RA:") << Util::toHTML("TRC DEC:");
		boxLabelMap[TL_LENGTH_HEIGHT] = labelTlcWidthLengthList;
		boxLabelMap[CENTER_LENGTH_HEIGHT] = labelCenterWidthLengthList;
		boxLabelMap[TL_BR] = labelTlcBrcList;
		boxLabelMap[BL_TR] = labelBlcTrcList;
		boxLabelMap[TL_LENGTH_HEIGHT_WORLD] = labelTlcWidthLengthListW;
		boxLabelMap[CENTER_LENGTH_HEIGHT_WORLD] = labelCenterWidthLengthListW;
		boxLabelMap[TL_BR_WORLD] = labelTlcBrcListW;
		boxLabelMap[BL_TR_WORLD] = labelBlcTrcListW;


		pixelValidator = new QIntValidator( this );
		pixelValidator -> setBottom( 0 );
		ui.pointXLineEdit -> setValidator( pixelValidator );
		ui.pointYLineEdit -> setValidator( pixelValidator );
		ui.boxPixBLCXLineEdit -> setValidator( pixelValidator );
		ui.boxPixBLCYLineEdit -> setValidator( pixelValidator );
		ui.boxPixTRCXLineEdit -> setValidator( pixelValidator );
		ui.boxPixTRCYLineEdit -> setValidator( pixelValidator );
	}

//-------------------------------------------------------------------------
//                                   Signal/Slot
//-------------------------------------------------------------------------

	void SpectralPositioningWidget::boxSpecChanged( int index ) {
		int pageIndex = ui.stackedWidget -> currentIndex();
		if ( pageIndex == BOX_PIXEL ) {
			switchBoxLabels( index, pageIndex, ui.pixX1Label, ui.pixY1Label, ui.pixX2Label, ui.pixY2Label );
		} else if ( pageIndex == BOX_RA_DEC ) {
			switchBoxLabels( index, pageIndex, ui.ra1Label, ui.dec1Label, ui.ra2Label, ui.dec2Label );
		} else {
			assert( false );
		}
		updateUI();
	}

	void SpectralPositioningWidget::locationSelectionTypeChanged( int index ) {
		//Show/hide the boxSpecCombo & label
		bool visible = index == BOX ? true : false;
		ui.boxSpecCombo -> setVisible( visible );
		ui.boxSpecLabel -> setVisible( visible );

		//Figure out the units
		int unitIndex = ui.unitComboBox-> currentIndex();
		pageUpdate( index, unitIndex );
		updateUI();
	}

	void SpectralPositioningWidget::switchBoxLabels( int index, int pageIndex, QLabel* const x1Label, QLabel* const y1Label,
	        QLabel* const x2Label, QLabel* const y2Label ) {
		int lookupIndex = (pageIndex-2) * (END_SPEC / 2 ) + index;
		QList<QString> labelList = boxLabelMap[static_cast<BoxSpecificationIndex>(lookupIndex)];
		x1Label -> setText( labelList[0]);
		y1Label -> setText( labelList[1]);
		x2Label -> setText( labelList[2]);
		y2Label -> setText( labelList[3]);

	}

	void SpectralPositioningWidget::locationUnitsChanged( int index ) {
		int locationIndex = ui.positionTypeComboBox->currentIndex();
		pageUpdate( locationIndex, index );
		updateUI();
	}

	void SpectralPositioningWidget::pageUpdate( int selectionIndex, int unitIndex ) {
		if ( selectionIndex == POINT && unitIndex == PIXEL ) {
			ui.stackedWidget->setCurrentIndex( POINT_PIXEL );
		} else if ( selectionIndex == POINT && unitIndex == RADIAN) {
			ui.stackedWidget->setCurrentIndex( POINT_RA_DEC );
		} else if ( selectionIndex == BOX && unitIndex == PIXEL ) {
			ui.stackedWidget->setCurrentIndex( BOX_PIXEL );
			boxSpecChanged( ui.boxSpecCombo  -> currentIndex() );
		} else if ( selectionIndex == BOX && unitIndex == RADIAN ) {
			ui.stackedWidget->setCurrentIndex( BOX_RA_DEC );
			boxSpecChanged( ui.boxSpecCombo -> currentIndex() );
		} else {
			assert( false );
		}
	}

//-----------------------------------------------------------------------------
// Getting information from the text fields and using it to set the position
//-----------------------------------------------------------------------------

	void SpectralPositioningWidget::setPosition() {
		//const ImageInterface<float>* image = profileTaskMonitor->getImage().get();

		if ( profileTaskMonitor->getImage() ) {
			QList<double> worldX;
			QList<double> worldY;
			QList<int> pixelX;
			QList<int> pixelY;
			bool success = true;
			int page = ui.stackedWidget->currentIndex();
			if ( page == POINT_PIXEL ) {
				fillPointPixel( pixelX,pixelY);
				success = populateWorlds( pixelX, pixelY, worldX, worldY );
			} else if ( page == POINT_RA_DEC ) {
				success = fillPointWorld( worldX, worldY);
			} else if ( page == BOX_PIXEL ) {
				success = fillBoxPixel( pixelX, pixelY );
				if ( success ) {
					success = populateWorlds( pixelX, pixelY, worldX, worldY );
				}
			} else if ( page == BOX_RA_DEC ) {
				success = fillBoxWorld( worldX, worldY );
			}

			if ( success ) {
				try {
					profileTaskMonitor->setPosition( worldX, worldY);
				} catch( ... ) {
					QString msg( "The position could not be set.\nPlease check the format and value of the position.");
					Util::showUserMessage( msg, this );
					*logger << LogIO::WARN << "Could not set the position." << LogIO::POST;
				}
			} else {
				*logger << LogIO::WARN << "Could not set the position." << LogIO::POST;
			}
		} else {
			//*itsLog << LogIO::WARN << "Image is not loaded" << LogIO::POST;
		}
	}


	bool SpectralPositioningWidget::fillPointWorld( QList<double> &worldX, QList<double> &worldY ) {
		Bool recognizedFormat = true;
		double centerXRad = toRadians( recognizedFormat, ui.pointRALineEdit );
		if ( recognizedFormat ) {
			double centerYRad = toRadians( recognizedFormat, ui.pointDECLineEdit );
			qDebug() << "centerYRad="<<centerYRad;
			if ( recognizedFormat ) {
				double length = qAbs( worldXValues[1] - worldXValues[0] );
				double height = qAbs( worldYValues[1] - worldYValues[0] );

				//Initialize the vectors we will pass to the algorithm
				//We need the blc and then the tlc.
				worldX.append( centerXRad + length/2 );
				worldX.append( centerXRad - length/2 );
				worldY.append( centerYRad - height/2 );
				worldY.append( centerYRad + height/2 );

			}
		}
		return recognizedFormat;
	}

	void SpectralPositioningWidget::fillPointPixel( QList<int> &pixelX, QList<int>&pixelY ) const {
		int centerXPix = ui.pointXLineEdit -> text().toInt();
		int centerYPix = ui.pointYLineEdit -> text().toInt();

		//Add the blc and then the tlc to the vector.
		double length = qAbs( pixelXValues[1] - pixelXValues[0] );
		double height = qAbs( pixelYValues[1] - pixelYValues[0] );
		pixelX.append( static_cast<int>(centerXPix - length/2+0.5) );
		pixelX.append( static_cast<int>(centerXPix + length/2+0.5) );
		pixelY.append( static_cast<int>(centerYPix - height/2+0.5) );
		pixelY.append( static_cast<int>(centerYPix + height/2+0.5) );
	}

	bool SpectralPositioningWidget::fillBoxPixel( QList<int> &pixelX, QList<int>&pixelY ) {
		double firstXPix = ui.boxPixBLCXLineEdit -> text().toInt();
		double firstYPix = ui.boxPixBLCYLineEdit -> text().toInt();
		double secondXPix = ui.boxPixTRCXLineEdit -> text().toInt();
		double secondYPix = ui.boxPixTRCYLineEdit -> text().toInt();

		double blcxPix;
		double blcyPix;
		double trcxPix;
		double trcyPix;
		bool valid = fillBasedOnBoxSpecification( &firstXPix,&firstYPix, &secondXPix, &secondYPix,
		             &blcxPix, &blcyPix, &trcxPix, &trcyPix );

		if ( valid ) {
			//Initialize the vectors we will pass to the algorithm
			pixelX.append( static_cast<int>(blcxPix) );
			pixelX.append( static_cast<int>(trcxPix) );
			pixelY.append( static_cast<int>(blcyPix) );
			pixelY.append( static_cast<int>(trcyPix) );
		}
		return valid;
	}

	bool SpectralPositioningWidget::fillBasedOnBoxSpecification(  const double*  const firstXPix, const double * const firstYPix,
	        const double* const secondXPix, const double* const secondYPix,
	        double* const blcXPix, double* const blcYPix,
	        double* const trcXPix, double* const trcYPix, bool pixels ) {
		int index = ui.boxSpecCombo -> currentIndex();
		int multiplier = 1;
		if ( !pixels ) {
			multiplier = -1;
		}

		//Assign the blc and trc values based on the box specification.
		if ( index == TL_LENGTH_HEIGHT ) {
			*blcXPix = *firstXPix;
			*blcYPix = *firstYPix - *secondYPix;
			*trcXPix = *firstXPix + multiplier * *secondXPix;
			*trcYPix = *firstYPix;
		} else if ( index == CENTER_LENGTH_HEIGHT ) {
			double halfWidth = *secondXPix / 2;
			double halfLength = *secondYPix / 2;
			*blcXPix = *firstXPix - multiplier*halfWidth;
			*blcYPix = *firstYPix - halfLength;
			*trcXPix = *firstXPix + multiplier*halfWidth;
			*trcYPix = *firstYPix + halfLength;
		} else if ( index == TL_BR ) {
			*blcXPix = *firstXPix;
			*blcYPix = *secondYPix;
			*trcXPix = *secondXPix;
			*trcYPix = *firstYPix;
		} else if ( index == BL_TR ) {
			*blcXPix = *firstXPix;
			*blcYPix = *firstYPix;
			*trcXPix = *secondXPix;
			*trcYPix = *secondYPix;
		} else {
			assert( false  );
		}

		//Check to make sure that the values make at least a little
		//sense and post a message if they don't
		bool valid = true;
		if ( *blcYPix > *trcYPix ) {
			valid = false;
		} else if ( pixels && *blcXPix > *trcXPix ) {
			valid = false;
		} else if ( !pixels && *blcXPix < *trcXPix ) {
			valid = false;
		}
		if ( !valid ) {
			QString msg("Error in setting the position:\n Please check that the box is specified correctly.");
			Util::showUserMessage( msg, this );
		}
		return valid;
	}

	bool SpectralPositioningWidget::fillBoxWorld( QList<double> &worldX, QList<double> & worldY )  {
		Bool valid = true;
		double secondXRad = toRadians( valid, ui.trcxLineEdit );
		if ( valid ) {
			double firstXRad = toRadians( valid, ui.blcxLineEdit );
			if ( valid ) {
				double secondYRad = toRadians( valid, ui.trcyLineEdit );
				if ( valid ) {
					double firstYRad = toRadians( valid, ui.blcyLineEdit );
					if ( valid ) {
						double trcxRad;
						double blcxRad;
						double trcyRad;
						double blcyRad;
						valid = fillBasedOnBoxSpecification( &firstXRad, &firstYRad, &secondXRad, &secondYRad,
						                                     &blcxRad, &blcyRad, &trcxRad, &trcyRad, false );

						if ( valid ) {
							//Initialize the vectors we will pass to the algorithm
							worldX.append( blcxRad );
							worldX.append( trcxRad );
							worldY.append( blcyRad );
							worldY.append( trcyRad );
						}
					}
				}
			}
		}
		return valid;
	}

	double SpectralPositioningWidget::toRadians( Bool& valid, QLineEdit *lineEdit ) {
		Quantity quantity;
		QString angleText = lineEdit->text();
		String angleExpression( angleText.toStdString() );
		double radians = 0;
		valid = MVAngle::read( quantity, angleExpression );
		if ( valid ) {
			MVAngle angle( quantity );
			radians = angle.radian();
		}

		if ( !valid ) {
			QString msg( "The format used to specify the position was not recognized.");
			Util::showUserMessage( msg, this );
		}
		return radians;
	}


	bool SpectralPositioningWidget::populateWorlds(const QList<int> &pixelX, const QList<int> &pixelY,
	        QList<double> &worldX, QList<double> &worldY ) {

		bool success = true;
		//ImageInterface<float>* image = const_cast<ImageInterface<float>* >(profileTaskMonitor->getImage());
		SHARED_PTR<ImageInterface<Float> > image(CONST_POINTER_CAST<ImageInterface<Float> >(profileTaskMonitor->getImage()));

		DisplayCoordinateSystem cSys = image -> coordinates();
		int nAxes = cSys.nPixelAxes();
		for ( int i = 0; i < pixelX.size(); i++ ) {
			Vector<double> sourcePt( nAxes );
			sourcePt[0] = pixelX[i];
			sourcePt[1] = pixelY[i];
			Vector<double> destinationPt;
			success = cSys.toWorld( destinationPt, sourcePt );
			if ( success ) {
				worldX.append( destinationPt[0]);
				worldY.append( destinationPt[1]);
			} else {
				String errorMessage = cSys.errorMessage();
				*logger << LogIO::WARN << errorMessage <<LogIO::POST;
				QString errorMsg = "Please check that the coordinates (";
				errorMsg.append( QString::number( pixelX[i]) + ", ");
				errorMsg.append( QString::number( pixelY[i]) + ")");
				errorMsg.append( " are correct.");
				Util::showUserMessage( errorMsg, this );
				break;
			}
		}
		return success;
	}

//-------------------------------------------------------------------------------------
//   Update the text fields/spin boxes based on changes in the viewer
//-------------------------------------------------------------------------------------
	void SpectralPositioningWidget::adjustPoint( const Vector<Double>& newX, const Vector<Double>& newY,
	        Vector<Double>& xValues, Vector<Double>& yValues ) {
		//Interpret the points coming in as the center of the box.  Move the
		//current box so that it is centered at the point coming in.
		double centerX = (xValues[0] + xValues[1])/2;
		double centerY = (yValues[0] + yValues[1])/2;
		double moveX = newX[0] - centerX;
		double moveY = newY[0] - centerY;
		for ( int i = 0; i < static_cast<int>(xValues.size()); i++ ) {
			xValues[i] = xValues[i]+moveX;
			yValues[i] = yValues[i]+moveY;
		}
	}


	void SpectralPositioningWidget::updateRegion( const Vector<Double> px, const Vector<Double> py,
	        const Vector<Double> wx, const Vector<Double> wy ) {
		if ( px.size() == 1 && py.size() == 1 && wx.size() == 1 && wy.size() == 1 ) {
			adjustPoint( px, py, pixelXValues, pixelYValues );
			adjustPoint( wx, wy, worldXValues, worldYValues );
		} else if ( px.size() == 2 && py.size() == 2 && wx.size() == 2 && wy.size() == 2 ) {
			pixelXValues = px;
			pixelYValues = py;
			worldXValues = wx;
			worldYValues = wy;
		} else {
			//qDebug() << "Update region: unrecognized size! px.size="<<px.size()
			//		<<" py.size="<<py.size()<<" wx.size="<<wx.size()<<" wy.size="<<wy.size();
		}

		updateUI();
	}

	void SpectralPositioningWidget::updateUI() {
		UnitIndex unitIndex = static_cast<UnitIndex>(ui.unitComboBox->currentIndex());
		PositionTypeIndex positionIndex = static_cast<PositionTypeIndex>(ui.positionTypeComboBox->currentIndex());
		if ( unitIndex == PIXEL ) {
			if ( positionIndex == BOX ) {
				updateUIPixelBox();
			} else {
				updateUIPixelPoint();
			}
		}
		//Radians
		else {
			if ( positionIndex == BOX ) {
				updateUIWorldBox();
			} else {
				updateUIWorldPoint();
			}
		}
	}

	void SpectralPositioningWidget::updateUIWorldBox() {
		BoxSpecificationIndex index =
		    static_cast<BoxSpecificationIndex>(ui.boxSpecCombo->currentIndex());
		if ( index == TL_LENGTH_HEIGHT ) {
			double length = qAbs(worldXValues[1] - worldXValues[0]);
			double height = qAbs(worldYValues[1] - worldYValues[0]);
			setWorldEdits( worldXValues[0], worldYValues[1], length, height );
		} else if ( index == CENTER_LENGTH_HEIGHT) {
			double centerX = (worldXValues[1] + worldXValues[0])/2;
			double centerY = (worldYValues[1] + worldYValues[0])/2;
			double length = qAbs(worldXValues[1] - worldXValues[0]);
			double height = qAbs(worldYValues[1] - worldYValues[0]);
			setWorldEdits( centerX, centerY, length, height );
		} else if ( index == TL_BR ) {
			setWorldEdits( worldXValues[0], worldYValues[1], worldXValues[1], worldYValues[0]);
		}
		//BL_TR
		else {
			setWorldEdits( worldXValues[0], worldYValues[0], worldXValues[1], worldYValues[1]);
		}

	}

	void SpectralPositioningWidget::updateUIWorldPoint() {
		Double mean;
		Util::getCenter( worldXValues, mean );
		double centerX = mean;
		Util::getCenter( worldYValues, mean );
		double centerY = mean;
		int raHours, raMins;
		double raSecs;
		Util::getRa( centerX, raHours, raMins, raSecs );
		int decDegs, decMins;
		double decSecs;
		Util::getDec( centerY, decDegs, decMins, decSecs );

		QString raStr = Util::toDegreeString( raHours, raMins, raSecs );
		ui.pointRALineEdit->setText( raStr );
		QString decStr = Util::toDecString( decDegs, decMins, decSecs );
		Util::appendSign( centerY, decStr );
		ui.pointDECLineEdit->setText( decStr );
	}



	void SpectralPositioningWidget::setWorldEdits( double topLeft, double bottomLeft,
	        double topRight, double bottomRight ) {
		int blcXhr, blcXmin;
		double blcXsec;
		Util::getRa( topLeft, blcXhr, blcXmin, blcXsec );
		QString blcRAStr = Util::toDegreeString( blcXhr, blcXmin, blcXsec );
		ui.blcxLineEdit->setText( blcRAStr );

		int blcYdeg, blcYmin;
		double blcYsec;
		Util::getDec( bottomLeft, blcYdeg, blcYmin, blcYsec );
		QString blcDECStr = Util::toDecString( blcYdeg, blcYmin, blcYsec );
		Util::appendSign( bottomLeft, blcDECStr );
		ui.blcyLineEdit->setText( blcDECStr );

		int trcXhr, trcXmin;
		double trcXsec;
		Util::getRa( topRight, trcXhr, trcXmin, trcXsec );
		QString trcRAStr = Util::toDegreeString( trcXhr, trcXmin, trcXsec );
		ui.trcxLineEdit->setText( trcRAStr );

		int trcYDec, trcYmin;
		double trcYsec;
		Util::getDec( bottomRight, trcYDec, trcYmin, trcYsec );
		QString trcDECStr = Util::toDecString( trcYDec, trcYmin, trcYsec );
		Util::appendSign( bottomRight, trcDECStr );
		ui.trcyLineEdit->setText( trcDECStr );
	}

	void SpectralPositioningWidget::setPixelLineEdits( double topLeft, double bottomLeft,
	        double topRight, double bottomRight ) {
		ui.boxPixBLCXLineEdit->setText( QString::number( topLeft ));
		ui.boxPixBLCYLineEdit->setText( QString::number( bottomLeft));
		ui.boxPixTRCXLineEdit->setText( QString::number( topRight));
		ui.boxPixTRCYLineEdit->setText( QString::number( bottomRight));
	}

	void SpectralPositioningWidget::updateUIPixelBox() {
		BoxSpecificationIndex index =
		    static_cast<BoxSpecificationIndex>(ui.boxSpecCombo->currentIndex());
		if ( index == TL_LENGTH_HEIGHT ) {
			double length = pixelXValues[1] - pixelXValues[0];
			double height = pixelYValues[1] - pixelYValues[0];
			setPixelLineEdits( pixelXValues[0], pixelYValues[1], length, height );
		} else if ( index == CENTER_LENGTH_HEIGHT) {
			double centerX = (pixelXValues[1] + pixelXValues[0])/2;
			double centerY = (pixelYValues[1] + pixelYValues[0])/2;
			double length = pixelXValues[1] - pixelXValues[0];
			double height = pixelYValues[1] - pixelYValues[0];
			setPixelLineEdits( centerX, centerY, length, height );
		} else if ( index == TL_BR ) {
			setPixelLineEdits( pixelXValues[0], pixelYValues[1], pixelXValues[1], pixelYValues[0]);
		}
		//BL_TR
		else {
			setPixelLineEdits( pixelXValues[0], pixelYValues[0], pixelXValues[1], pixelYValues[1]);
		}

	}

	void SpectralPositioningWidget::updateUIPixelPoint() {
		Double mean;
		int centerX = Util::getCenter( pixelXValues, mean );
		int centerY = Util::getCenter( pixelYValues, mean );
		ui.pointXLineEdit->setText( QString::number( centerX));
		ui.pointYLineEdit->setText( QString::number( centerY));
	}

	SpectralPositioningWidget::~SpectralPositioningWidget() {

	}
}
