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


#include <casa/aips.h>
#include <casa/Arrays.h>
#include <measures/Measures.h>
#include <coordinates/Coordinates.h>
#include <casa/Exceptions/Error.h>
#include <lattices/Lattices/RegionType.h>
#include <tables/Tables/TableRecord.h>
#include <casa/BasicSL/String.h>
#include <casa/Utilities/Assert.h>

#include <display/QtPlotter/QtCanvas.qo.h>
#include <display/QtPlotter/QtProfile.qo.h>
#include <display/QtPlotter/QtProfilePrefs.qo.h>
#include <display/QtPlotter/QtMWCTools.qo.h>
#include <display/QtPlotter/ColorSummaryWidget.qo.h>
#include <display/QtPlotter/Util.h>
#include <display/QtPlotter/LegendPreferences.qo.h>
#include <display/QtPlotter/SmoothPreferences.qo.h>
#include <display/QtPlotter/conversion/Converter.h>
#include <display/QtPlotter/conversion/ConverterIntensity.h>

#include <imageanalysis/ImageAnalysis/ImageAnalysis.h>
#include <images/Images/ImageUtilities.h>
#include <images/Images/PagedImage.h>
#include <images/Images/ImageFITSConverter.h>
#include <coordinates/Coordinates/QualityCoordinate.h>
#include <casa/Logging/LogFilter.h>
#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogOrigin.h>
#include <images/Images/ImageUtilities.h>
#include <images/Images/ImageInfo.h>
#include <display/DisplayEvents/MWCPTRegion.h>
#include <display/Display/Options.h>

#include <graphics/X11/X_enter.h>
#include <QDir>
#include <QColor>
#include <QHash>
#include <QWidget>
#include <QDialog>
#include <QMainWindow>
#include <QMouseEvent>
#include <cmath>
#include <QtGui>
#include <QComboBox>
#include <iostream>
#include <assert.h>
#include <graphics/X11/X_exit.h>
#include <QMessageBox>
#include <limits>

#include <tr1/memory>

namespace casa {

	const QString QtProfile::PLOT_TYPE_FLUX = "Flux Density";
	const QString QtProfile::PLOT_TYPE_MEAN = "Mean";
	const QString QtProfile::PLOT_TYPE_MEDIAN = "Median";
	const QString QtProfile::PLOT_TYPE_SUM = "Sum";
	const QString QtProfile::FREQUENCY = "frequency";
	const QString QtProfile::RADIO_VELOCITY = "radio velocity";
	const QString QtProfile::VELOCITY = "velocity";
	const QString QtProfile::CHANNEL = "channel";
	const QString QtProfile::OPTICAL = "optical";
	const QString QtProfile::AIR = "air";
	const QString QtProfile::FRAME_REST = "REST";

	QtProfile::~QtProfile() {
	}

	QtProfile::QtProfile(std::tr1::shared_ptr<ImageInterface<Float> > img, const char *name, QWidget *parent, std::string rcstr)
		:QMainWindow(parent),
//pc(0),
//te(0),
		 analysis(0), image(img), over(0),WORLD_COORDINATES("world"),
		 coordinate( WORLD_COORDINATES ), coordinateType(""),xaxisUnit(""),ctypeUnit(""),
		 cSysRval(""), fileName(name), position(""), yUnit(""),
		 yUnitPrefix(""), xpos(""), ypos(""), cube(0),
		 npoints(0), npoints_old(0), stateMProf(2), stateRel(0),
		 z_xval(Vector<Float>()), z_yval(Vector<Float>()),
		 lastPX(Vector<Double>()), lastPY(Vector<Double>()),
		 lastWX(Vector<Double>()), lastWY(Vector<Double>()),
		 z_eval(Vector<Float>()), region(""), rc(viewer::getrc()), rcid_(rcstr),
		 itsPlotType(QtProfile::PMEAN), itsLog(new LogIO()), ordersOfM_(0),
		 NO_REGION_ID(-1),
		 colorSummaryWidget( NULL ), legendPreferencesDialog( NULL ),newOverplots( false ) {
		setupUi(this);
		initPlotterResource();
		showTopAxis = true;

		//Remove the setPosition tab because duplicate functionality exists in the viewer.
		functionTabs->removeTab(0);

		initSmoothing();
		current_region_id = NO_REGION_ID;

		setBackgroundRole(QPalette::Dark);

		plotMode->addItem( PLOT_TYPE_MEAN );
		plotMode->addItem( PLOT_TYPE_MEDIAN );
		plotMode->addItem( PLOT_TYPE_SUM );
		plotMode->addItem( PLOT_TYPE_FLUX );
		fillPlotTypes(img);
		connect(plotMode, SIGNAL(currentIndexChanged(const QString &)),
		        this, SLOT(changePlotType(const QString &)));
		connect(errorMode, SIGNAL(currentIndexChanged(const QString &)),
		        this, SLOT(changeErrorType(const QString &)));

		pixelCanvas = this->canvasHolder->getCanvas();
		QPalette pal = pixelCanvas->palette();
		pal.setColor(QPalette::Background, Qt::white);
		pixelCanvas->setPalette(pal);
		connect( functionTabs, SIGNAL(currentChanged(int)), pixelCanvas, SLOT(changeTaskMode(int)));

		initPreferences();

		//Plot colors we will use
		colorSummaryWidget = new ColorSummaryWidget( this );
		colorSummaryWidget->setColorCanvas( pixelCanvas );

		//User legend preferences
		legendPreferencesDialog = new LegendPreferences( canvasHolder, this );

		//X Axis Units
		initializeXAxisUnits();
		setXAxisUnits();

		initializeSpectralProperties();

		// get reference frame info for freq axis label
		MFrequency::Types freqtype = determineRefFrame(img);
		spcRefFrame = String(MFrequency::showType(freqtype));
		updateSpectralReferenceFrame();

		connect(bottomAxisCType, SIGNAL(currentIndexChanged(const QString &)),
		        this, SLOT(changeCoordinateType(const QString &)));
		connect(topAxisCType, SIGNAL( currentIndexChanged( const QString &)),
		        this, SLOT(changeTopAxisCoordinateType(const QString &)));
		connect(spcRef, SIGNAL(currentIndexChanged(const QString &)),
		        this, SLOT(changeFrame(const QString &)));

		QSettings settings("CASA", "Viewer");
		QString pName = settings.value("Print/printer").toString();

		connect(pixelCanvas, SIGNAL(xRangeChanged(double, double)), this, SLOT(setCollapseRange(double, double)));
		connect(pixelCanvas, SIGNAL(channelSelect(float)), this, SLOT(channelSelect(float)));
		connect(pixelCanvas, SIGNAL(channelRangeSelect(float,float)), this, SLOT( channelRangeSelect(float,float)));

		pixelCanvas->setTitle("");
		pixelCanvas->setWelcome("assign a mouse button to\n"
		                        "'crosshair' or 'rectangle' or 'polygon'\n"
		                        "click/press+drag the assigned button on\n"
		                        "the image to get a spectral profile");

		QString lbl = bottomAxisCType->currentText();
		pixelCanvas->setXLabel(lbl, 12, QtCanvas::FONT_NAME, QtPlotSettings::xBottom );
		lbl = topAxisCType->currentText();
		pixelCanvas->setXLabel( lbl, 12, QtCanvas::FONT_NAME, QtPlotSettings::xTop);

		yUnit = QString(img->units().getName().chars());
		setPixelCanvasYUnits( yUnitPrefix, yUnit );
		adjustPlotUnits();
		yAxisCombo->setCurrentIndex( 0 );
		setDisplayYUnits( yAxisCombo->currentText() );
		initializeSolidAngle();
		connect( yAxisCombo, SIGNAL( currentIndexChanged(const QString&)), this , SLOT( setDisplayYUnits(const QString&)));

		pixelCanvas->setAutoScaleX(true);
		pixelCanvas->setAutoScaleY(true);

		connect(actionZoomIn, SIGNAL(triggered()), this, SLOT(zoomIn()));
		connect(actionZoomOut, SIGNAL(triggered()), this, SLOT(zoomOut()));
		connect(actionZoomNeutral, SIGNAL(triggered()), this, SLOT(zoomNeutral()));
		connect(actionPrint, SIGNAL(triggered()), this, SLOT(print()));
		connect(actionSaveGraphic, SIGNAL(triggered()), this, SLOT(saveGraphic()));
		connect(actionExport, SIGNAL(triggered()), this, SLOT(exportProfile()));
		connect(actionMoveLeft, SIGNAL(triggered()), this, SLOT(left()));
		connect(actionMoveRight, SIGNAL(triggered()), this, SLOT(right()));
		connect(actionMoveUp, SIGNAL(triggered()), this, SLOT(up()));
		connect(actionMoveDown, SIGNAL(triggered()), this, SLOT(down()));
		connect(actionPreferences, SIGNAL(triggered()), this, SLOT(preferences()));
		connect(actionColors, SIGNAL(triggered()), this, SLOT(curveColorPreferences()));
		connect(actionLegend, SIGNAL(triggered()), this, SLOT(legendPreferences()));
		connect(actionSmooth, SIGNAL(triggered()), this, SLOT(showSmoothingPreferences()));
		connect(actionAnnotationText, SIGNAL(triggered()), pixelCanvas, SLOT(createAnnotationText()));
		connect(actionRangeXSelection, SIGNAL(triggered()), pixelCanvas, SLOT(rangeSelectionMode()));
		connect(actionChannelPositioning, SIGNAL(triggered()), pixelCanvas, SLOT(channelPositioningMode()));
		connect(pixelCanvas,SIGNAL(clearPaletteModes()),this, SLOT(clearPaletteModes()));
		connect(pixelCanvas, SIGNAL(togglePalette(int)), this, SLOT(togglePalette(int)));
		QActionGroup* paletteGroup = new QActionGroup(this );
		actionAnnotationText->setActionGroup( paletteGroup );
		actionRangeXSelection->setActionGroup( paletteGroup );
		actionChannelPositioning->setActionGroup( paletteGroup );
		actionChannelPositioning->setToolTip("<html>Specify a new channel position in the viewer or movie the viewer through a range of channels<br/> (Ctrl+click the right mouse button to set a new channel position or drag the channel indicator to movie the channel</html>");
		actionAnnotationText->setCheckable( true );
		actionRangeXSelection->setCheckable( true );
		actionChannelPositioning->setCheckable( true );

		//Spectral Line Fitting & Moments/Collapse initialization
		momentSettingsWidget->setTaskSpecLineFitting( false );
		specFitSettingsWidget->setTaskSpecLineFitting( true );
		specFitSettingsWidget -> setCanvas( pixelCanvas );
		momentSettingsWidget -> setCanvas( pixelCanvas );
		lineOverlaysHolder->setCanvas( pixelCanvas );
		lineOverlaysHolder->setInitialReferenceFrame( spcRef->currentText() );
		specFitSettingsWidget -> setTaskMonitor( this );
		momentSettingsWidget -> setTaskMonitor( this );
		positioningWidget -> setTaskMonitor( this );
		specFitSettingsWidget -> setLogger( itsLog );
		momentSettingsWidget -> setLogger( itsLog );
		positioningWidget->setLogger( itsLog );
		try {
			analysis  = new ImageAnalysis(img);
			momentSettingsWidget->reset();
			specFitSettingsWidget->reset();
			setUnitsText( xaxisUnit );
		} catch (AipsError x) {
			String message = "Error when starting the profiler:\n" + x.getMesg();
			*itsLog << LogIO::WARN << message << LogIO::POST;
		}

	}

	void QtProfile::initializeXAxisUnits(){
		xUnitsList =(QStringList()<<
		   "radio velocity [m/s]"<< "radio velocity [km/s]" <<
		   "optical velocity [m/s]" << "optical velocity [km/s]" <<
		   "frequency [Hz]" << "frequency [MHz]" << "frequency [GHz]" <<
		   "wavelength [mm]" << "wavelength [um]" << "wavelength [nm]" << "wavelength [Angstrom]" <<
		   "air wavelength [mm]" << "air wavelength [um]" << "air wavelength [nm]"<<"air wavelength [Angstrom]"<<
		   CHANNEL);
		for ( int i = 0; i < xUnitsList.size(); i++ ){
			bottomAxisCType->addItem( xUnitsList[i]);
		}
		restrictTopAxisOptions( false, bottomAxisCType->currentText() );
	}

	void QtProfile::setXAxisUnits(){
		QString pref_ctype = read( ".freqcoord.type");

		//We can't use velocity with a tabular axis because the
		//rest frequency is zero.
		if ( !isSpectralAxis() && pref_ctype.indexOf(VELOCITY) >= 0){
			pref_ctype="frequency [Hz]";
		}

		if (pref_ctype.size()>0) {
			// change to the preferred ctype
			updateAxisUnitCombo( pref_ctype, bottomAxisCType );
			updateAxisUnitCombo( pref_ctype, topAxisCType );
		}

		ctypeUnit = String(bottomAxisCType->currentText().toStdString());
		getcoordTypeUnit(ctypeUnit, coordinateType, xaxisUnit);
		pixelCanvas -> setToolTipXUnit( xaxisUnit.c_str());
	}

	void QtProfile::updateSpectralReferenceFrame( ){
		//Cannot convert from rest frame to other spectral frames without
		//reloading the image.
		bool restFrame = false;
		if ( FRAME_REST.toStdString() == spcRefFrame.c_str()){
			restFrame = true;
		}
		int restIndex = spcRef->findText( FRAME_REST );

		//Add rest frame if it is not already there.
		if ( restFrame ){
			if ( restIndex == -1 ){
				spcRef->addItem( FRAME_REST );
			}
		}
		//Remove rest frame as an option to convert to.
		else {
			if ( restIndex >= 0 ){
				spcRef->removeItem( restIndex );
			}
		}
		//Update the combo with the current rest frame.
		int restFrameIndex = spcRef->findText( spcRefFrame.c_str() );
		spcRef->setCurrentIndex( restFrameIndex );
		spcRef->setEnabled( !restFrame );
		lineOverlaysHolder->setInitialReferenceFrame( spcRef->currentText() );
	}





	void QtProfile::setUnitsText( String unitStr ) {
		QString unitLabel("<font color='black'>["+QString(unitStr.c_str())+"]</font>");
		specFitSettingsWidget->setUnits( unitLabel );
		momentSettingsWidget->setUnits( unitLabel );
	}


	MFrequency::Types QtProfile::determineRefFrame(std::tr1::shared_ptr<ImageInterface<Float> > img, bool check_native_frame ) {
		MFrequency::Types freqtype;

		DisplayCoordinateSystem cSys=img->coordinates();
		Int specAx=cSys.findCoordinate(Coordinate::SPECTRAL);
		if ( specAx < 0 ) {
			int tabIndex = Util::getTabularFrequencyAxisIndex( img );
			if ( tabIndex < 0 ){
				//See if it has a tabular axis with units frequency.
				QMessageBox::information( this, "No spectral axis...",
			                          "Sorry, could not find a spectral axis for this image...",
			                          QMessageBox::Ok);
			}
			return MFrequency::DEFAULT;
		}

		SpectralCoordinate specCoor=cSys.spectralCoordinate(specAx);
		MFrequency::Types tfreqtype;
		MEpoch tepoch;
		MPosition tposition;
		MDirection tdirection;
		specCoor.getReferenceConversion(tfreqtype, tepoch, tposition, tdirection);
		freqtype = specCoor.frequencySystem(False); // false means: get the native type

		if( check_native_frame && tfreqtype != freqtype ) { // there is an active conversion layer
			// ask user if he/she wants to change to native frame
			String title = "Change display reference frame?";
			String message = "Native reference frame is " + MFrequency::showType(freqtype)
			                 + ",\n display frame is " + MFrequency::showType(tfreqtype) + ".\n"
			                 + "Change display frame permanently to " + MFrequency::showType(freqtype) + "?\n"
			                 + "(Needs write access to image.)";
			if(QMessageBox::question(this, title.c_str(), message.c_str(),
			                         QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
				// user wants to change
				try {
					// set the reference conversion to the native type, effectively switching it off
					if(!specCoor.setReferenceConversion(freqtype, tepoch, tposition, tdirection)
					        || !cSys.replaceCoordinate(specCoor, specAx)
					        || !img->setCoordinateInfo(cSys)) {

						img->coordinates().spectralCoordinate(specAx).getReferenceConversion(tfreqtype, tepoch, tposition, tdirection);
						title = "Failure";
						message = "casaviewer: Error setting reference frame conversion to native frame ("
						          + MFrequency::showType(freqtype) + ")\nWill use " + MFrequency::showType(tfreqtype) + " instead";
						QMessageBox::warning(this, title.c_str(), message.c_str(),
						                     QMessageBox::Ok, QMessageBox::NoButton);
						freqtype = tfreqtype;
					}
				} catch (AipsError x) {
					title = "Failure";
					message = "Error when trying to change display reference frame:\n" + x.getMesg();
					QMessageBox::warning(this, title.c_str(), message.c_str(),
					                     QMessageBox::Ok, QMessageBox::NoButton);
					freqtype = tfreqtype;
				}
			} else { // user does not want to change
				freqtype = tfreqtype;
			}
		} // end if there is a conv layer

		return freqtype;

	}
	void QtProfile::zoomOut() {
		pixelCanvas->zoomOut();
	}

	void QtProfile::zoomIn() {
		pixelCanvas->zoomIn();
	}

	void QtProfile::zoomNeutral() {
		pixelCanvas->zoomNeutral();
	}

	void QtProfile::print() {
#ifndef QT_NO_PRINTER
		QPrinter printer(QPrinter::ScreenResolution);
		//printer.setFullPage(true);
		QPrintDialog *dlg = new QPrintDialog(&printer, this);
		if (dlg->exec() == QDialog::Accepted) {
			QSettings settings("CASA", "Viewer");
			settings.setValue("Print/printer", printer.printerName());
			printIt(&printer);
		}
		delete dlg;
#endif
	}
	void QtProfile::printExp() {
#ifndef QT_NO_PRINTER
		QPrinter printer(QPrinter::ScreenResolution);
		QSettings settings("CASA", "Viewer");
		printer.setPrinterName(settings.value("Print/printer").toString());
		printIt(&printer);
#endif
	}

	void QtProfile::saveGraphic() {
		QString dflt = fileName + position + ".png";

		QString fn = QFileDialog::getSaveFileName(this,
		             tr("Save as..."), QString(dflt), tr("(*.png);;(*.pdf);;(*.xpm);;(*.jpg);;(*.ppm);;(*.jpeg)"));

		if (fn.isEmpty())
			return ;

		QString ext = fn.section('.', -1);
		if (ext == "xpm" || ext == "jpg" || ext == "png" ||
		        ext == "xbm" || ext == "ppm" || ext == "jpeg")
			;
		else
			fn.append(".png");

		QFile file(fn);
		if (!file.open(QFile::WriteOnly))
			return ;

		pixelCanvas->graph()->save(fn);
		return ;
	}
	void QtProfile::saveExp() {
		//qDebug() << "fileName:" << fileName;
		QFile file(fileName.append(position).append(".png"));
		if (!file.open(QFile::WriteOnly))
			return ;
		//qDebug() << "open ok";

		pixelCanvas->graph()->save(fileName, "PNG");
		//qDebug() << "save ok";
		return ;
	}

	void QtProfile::exportProfile() {
		QString filterString = tr("FITS files (*.fits);;Text files (*.txt);;Vector Graphic Plotter files(*.plt)");
		QString currentDirectory = QDir::currentPath();
		QFileDialog fd( this, tr("Export profile"), currentDirectory );
		fd.setFileMode( QFileDialog::AnyFile );
		fd.setNameFilter( filterString );
		if ( fd.exec() ) {
			QStringList fileNames = fd.selectedFiles();
			if ( fileNames.size() > 0 ) {
				QString outputFileName = fileNames[0];
				QString ext = outputFileName.section('.', -1);
				bool ok;
				if (ext =="fits"){
					ok = exportFITSSpectrum(/*fn*/outputFileName);
				}
				else {
					if (ext != "txt" && ext != "plt"){
						//fn.append(".txt");
						outputFileName.append( ".txt");
					}
					ok = exportASCIISpectrum(/*fn*/outputFileName );
				}
			}
		}
	}

	void QtProfile::left() {
		QApplication::sendEvent(pixelCanvas,
		                        new QKeyEvent(QEvent::KeyPress, Qt::Key_Left, 0, 0));
	}

	void QtProfile::right() {
		QApplication::sendEvent(pixelCanvas,
		                        new QKeyEvent(QEvent::KeyPress, Qt::Key_Right, 0, 0));
	}

	void QtProfile::up() {
		QApplication::sendEvent(pixelCanvas,
		                        new QKeyEvent(QEvent::KeyPress, Qt::Key_Up, 0, 0));
	}

	void QtProfile::down() {
		QApplication::sendEvent(pixelCanvas,
		                        new QKeyEvent(QEvent::KeyPress, Qt::Key_Down, 0, 0));
	}

	void QtProfile::initSmoothing(){
		smoothWidget = new SmoothPreferences( this );
		connect( smoothWidget, SIGNAL(smoothingChanged()), this, SLOT(replotCurves()));
	}

	void QtProfile::showSmoothingPreferences(){
		smoothWidget->show();
	}

	void QtProfile::initPreferences() {
		profilePrefs = new QtProfilePrefs(this,pixelCanvas->getAutoScaleX(), pixelCanvas->getAutoScaleY(),
		                                  pixelCanvas->getShowGrid(),stateMProf, stateRel, pixelCanvas->getShowToolTips(), showTopAxis,
		                                  pixelCanvas->isDisplayStepFunction(), specFitSettingsWidget->isOptical(), pixelCanvas->isShowChannelLine());
		connect(profilePrefs, SIGNAL(currentPrefs(bool, bool, int, int, int, bool, bool, bool, bool, bool)),
		        this, SLOT(setPreferences(bool, bool, int, int, int, bool, bool, bool, bool, bool)));
		profilePrefs->syncUserPreferences();
	}

	void QtProfile::preferences() {
		profilePrefs->show();
	}

	void QtProfile::setPreferences(bool inAutoX, bool inAutoY, int showGrid, int inMProf, int inRel,
	                               bool showToolTips, bool showTopAxis, bool displayStepFunction,
	                               bool opticalFitter,
	                               bool showChannelLine) {
		bool update=false;
		if ((lastPX.nelements() > 0) && ((inMProf!=stateMProf) || (inRel!=stateRel)))
			update=true;
		pixelCanvas->setAutoScaleX(inAutoX);
		pixelCanvas->setAutoScaleY(inAutoY);
		pixelCanvas->setShowGrid(showGrid);
		pixelCanvas->setShowToolTips( showToolTips );
		pixelCanvas->setShowChannelLine( showChannelLine );
		pixelCanvas ->setDisplayStepFunction( displayStepFunction );

		if ( !opticalFitter ){
			this->showTopAxis = showTopAxis;
		}
		else {
			this->showTopAxis = false;
		}
		adjustTopAxisSettings();
		changeTopAxis();

		stateMProf=inMProf;
		stateRel  = inRel;

		bool oldOpticalFitter = specFitSettingsWidget->isOptical();
		if ( opticalFitter != oldOpticalFitter ) {
			SettingsWidget::setOptical( opticalFitter );
			specFitSettingsWidget->reset();
			momentSettingsWidget->reset();
			actionColors-> setVisible( !opticalFitter );
			if ( opticalFitter ){
				showTopAxis = false;
			}
		}

		if (update) {
			wcChanged(coordinate, lastPX, lastPY, lastWX, lastWY, UNKNPROF);
		}

	}

	void QtProfile::setPlotError(int st) {
		pixelCanvas->setPlotError(st);
	}

	void QtProfile::changeCoordinate(const QString &text) {
		coordinate = String(text.toStdString());
		emit coordinateChange(coordinate);
	}

	MFrequency::Types QtProfile::getReferenceFrame() const {
		QString refText = spcRef->currentText();
		MFrequency::Types mFrequency = MFrequency::DEFAULT;
		MFrequency::getType( mFrequency, refText.toStdString());
		return mFrequency;
	}

	bool QtProfile::isSpectralAxis() const {
		bool spectralAxis = true;
		if ( image ){
			const DisplayCoordinateSystem& cSys = image->coordinates();
			if ( !cSys.hasSpectralAxis()){
				spectralAxis = false;
			}
		}
		return spectralAxis;
	}

	void QtProfile::changeFrame(const QString &text) {
		spcRefFrame=String(text.toStdString());
		changeCoordinateType(QString(ctypeUnit.c_str()));
	}

	void QtProfile::updateXAxisLabel( const QString &text, QtPlotSettings::AxisIndex axisIndex ) {
		QString lbl = text;
		pixelCanvas->setXLabel(lbl, 12, QtCanvas::FONT_NAME, axisIndex );
	}

	void QtProfile::changeTopAxisCoordinateType( const QString & /*text*/ ) {
		changeTopAxis();
	}



	void QtProfile::changeCoordinateType(const QString &text ) {
		xpos = "";
		ypos = "";
		position = QString("");
		profileStatus->showMessage(position);
		pixelCanvas->clearCurve();

		ctypeUnit = String(text.toStdString());
		getcoordTypeUnit(ctypeUnit, coordinateType, xaxisUnit);

		setUnitsText( xaxisUnit );

		//In the case of multiple images loaded we want to disable/enable
		//top axis settings based on whether the images match or not.
		adjustTopAxisSettings();

		pixelCanvas->setPlotSettings(QtPlotSettings());
		pixelCanvas -> setToolTipXUnit(xaxisUnit.c_str() );
		updateXAxisLabel( text, QtPlotSettings::xBottom );

		//cout << "put to rc.viewer: " << text.toStdString() << endl;
		persist( ".freqcoord.type", text);
		if(lastPX.nelements() > 0) {
			// update display with new coord type
			wcChanged(coordinate, lastPX, lastPY, lastWX, lastWY, UNKNPROF );
			frameChanged( -1 );
		}

	}

	void QtProfile::replotCurves(){
		pixelCanvas->clearCurve();
		if ( lastPX.nelements() > 0 ){
			wcChanged(coordinate,lastPX, lastPY, lastWX, lastWY, UNKNPROF );
		}
	}

	void QtProfile::closeEvent (QCloseEvent *) {
		lastPX.resize(0);
		lastPY.resize(0);
		lastWX.resize(0);
		lastWY.resize(0);
		z_xval.resize(0);
		z_yval.resize(0);
		z_eval.resize(0);
		emit hideProfile();
	}

	void QtProfile::resetProfile(std::tr1::shared_ptr<ImageInterface<Float> > img, const char *name) {
		image = img;
		try {
			if (analysis)
				delete analysis;
			analysis = new ImageAnalysis(img);

			specFitSettingsWidget->reset( );
			momentSettingsWidget->reset();
		}
		catch (AipsError x) {
			String message = "Error when re-setting the profiler:\n" + x.getMesg();
			*itsLog << LogIO::WARN << message << LogIO::POST;
		}

		newOverplots = false;
		fileName = name;
		setWindowTitle(QString("Spectral Profile - ").append(name));

		// re-set the images
		// that are overplotted
		if (over) {
			delete over;
			over = 0;
			over = new QList<OverplotAnalysis>();
		}

		// adjust the error box
		fillPlotTypes(img);

		// read the preferred ctype from casarc
		QString pref_ctype = read( ".freqcoord.type");
		if (pref_ctype.size()>0) {
			// change to the preferred ctype
			updateAxisUnitCombo( pref_ctype, bottomAxisCType );
			updateAxisUnitCombo( pref_ctype, topAxisCType );
		}

		initializeSpectralProperties();
		initializeSolidAngle();

		ctypeUnit = String(bottomAxisCType->currentText().toStdString());
		getcoordTypeUnit(ctypeUnit, coordinateType, xaxisUnit);
		setUnitsText( xaxisUnit );

		QString lbl = bottomAxisCType->currentText();
		pixelCanvas->setXLabel(lbl, 12, QtCanvas::FONT_NAME,QtPlotSettings::xBottom);
		lbl = topAxisCType->currentText();
		pixelCanvas->setXLabel(lbl, 12, QtCanvas::FONT_NAME, QtPlotSettings::xTop);

		// get reference frame info for frequency axis label
		MFrequency::Types freqtype = determineRefFrame(img);
		spcRefFrame = String(MFrequency::showType(freqtype));
		updateSpectralReferenceFrame();


		//YUnits
		yUnit = QString(img->units().getName().chars());
		//Images created with immoments seem to have units of the form
		//Jy/beam.km/s (CAS-5216).  We have to strip the ".km/s" in order
		//to have units recognized by the profiler.
		int periodIndex = yUnit.indexOf( ".");
		if ( periodIndex > 0 ){
			yUnit = yUnit.left( periodIndex);
		}
		yUnitPrefix = "";
		adjustPlotUnits();
		setPixelCanvasYUnits( yUnitPrefix, yUnit );
		setDisplayYUnits( yAxisCombo->currentText());

		xpos = "";
		ypos = "";
		lastPX.resize(0);
		lastPY.resize(0);
		lastWX.resize(0);
		lastWY.resize(0);
		position = QString("");
		profileStatus->showMessage(position);
		pixelCanvas->clearCurve();
	}

	void QtProfile::initializeSpectralProperties(){
		DisplayCoordinateSystem cSys = image->coordinates();
		bool spectralAxis = cSys.hasSpectralAxis();

		if ( spectralAxis ) {
			SpectralCoordinate spectralCoordinate = cSys.spectralCoordinate();
			Converter::setSpectralCoordinate( spectralCoordinate );
		}
		else {
			Int tabularCoordinateIndex = Util::getTabularFrequencyAxisIndex( image );
			if ( tabularCoordinateIndex >= 0 ){
				resetTabularConversion();
			}
		}
		resetXUnits(spectralAxis);
	}

	void QtProfile::resetXUnits( bool spectralAxis ){
		//Because we do not know the rest frequency we can not make valid conversions
		//from frequency/wavelength <-> velocity.  Therefore, we remove all velocity
		//units from the combos if we do not have a spectral axis..
		QString oldUnit = bottomAxisCType->currentText();
		bottomAxisCType->clear();
		int unitCount = xUnitsList.size();
		for ( int i = 0; i < unitCount; i++ ){
			if ( xUnitsList[i].indexOf( "velocity") == -1 || spectralAxis ){
				bottomAxisCType->addItem( xUnitsList[i]);
			}
		}
		int oldUnitsIndex = bottomAxisCType->findText( oldUnit );
		if ( oldUnitsIndex != -1 ){
			bottomAxisCType->setCurrentIndex( oldUnitsIndex );
		}
	}

	void QtProfile::resetTabularConversion(){

		int tabularIndex = Util::getTabularFrequencyAxisIndex( image );
		if ( tabularIndex >= 0 ){
			DisplayCoordinateSystem cSys = image->coordinates( );
			TabularCoordinate tabCoordinate = cSys.tabularCoordinate( tabularIndex );
			Vector<Double> worlds = tabCoordinate.worldValues();
			MFrequency::Types mFrequency = getReferenceFrame();
			SpectralCoordinate spectralCoordinate( mFrequency, worlds );

			//Put the new spectral coordinate into the image in place of the tabular
			//coordinate.
			//cSys.replaceCoordinate( spectralCoordinate, tabularIndex);
			Converter::setSpectralCoordinate( spectralCoordinate );
		}

	}

	void QtProfile::adjustPlotUnits() {
		//Store the old units so we can try to reset them
		QString currentYUnits = yAxisCombo->currentText();


		yAxisCombo->clear();
		QStringList yUnitsList =(QStringList()<< ConverterIntensity::JY_BEAM <<
		                         ConverterIntensity::JY_ARCSEC << ConverterIntensity::JY_SR <<
		                         ConverterIntensity::FRACTION_OF_PEAK<<ConverterIntensity::KELVIN);

		// remove the "/beam" in case of plotting flux
		const QString PER_BEAM = "/beam";
		if( itsPlotType==QtProfile::PFLUX) {
			Int pos = yUnit.indexOf(PER_BEAM,0,Qt::CaseInsensitive);
			if(pos>-1) {
				yUnit.remove(pos,5);
			}
			yUnitsList[0] = "Jy";
		}
		//Add   in case of sum
		else if ( itsPlotType==QtProfile::PSUM ) {
			Int pos = yUnit.indexOf(PER_BEAM,0,Qt::CaseInsensitive);
			if(pos>-1) {
				if ( yUnit.indexOf( ConverterIntensity::TIMES_PIXELS) == -1 ) {
					yUnit = yUnit + ConverterIntensity::TIMES_PIXELS;
				}
				for ( int i = 0; i < yUnitsList.size(); i++ ) {
					if ( yUnitsList[i] != ConverterIntensity::FRACTION_OF_PEAK ) {
						yUnitsList[i] = yUnitsList[i] + ConverterIntensity::TIMES_PIXELS;
					}
				}
			}
		}

		//Adapt the yAxis combo to display appropriate units/transformations
		if ( itsPlotType==QtProfile::PFLUX ) {
			yAxisCombo->addItem( yUnitsList[0] );
			yAxisCombo->addItem( ConverterIntensity::FRACTION_OF_PEAK );
		} else {
			for ( int i = 0; i < yUnitsList.size(); i++ ) {
				yAxisCombo->addItem( yUnitsList[i]);
			}
		}

		resetYUnits( currentYUnits );

	}

	void QtProfile::wcChanged( const String c,
	                           const Vector<Double> px, const Vector<Double> py,
	                           const Vector<Double> wx, const Vector<Double> wy,
	                           const ProfileType ptype) {
		if (!isVisible()) return;
		if (!analysis) return;
		*itsLog << LogOrigin("QtProfile", "wcChanged");

		//Since we are going to reset the data, we need to store the
		//current units so that we can attempt to set them back later.
		QString currentYUnits = yAxisCombo->currentText();

		bool cubeZero = checkCube();
		if (cubeZero) {
			return;
		}

		copyToLastEvent( c,px,py,wx,wy );

		setPlotType( wx.size() );

		assignCoordinate( c );

		Int ns;
		px.shape(ns);

		Vector<Double> pxv(ns);
		Vector<Double> pyv(ns);
		Vector<Double> wxv(ns);
		Vector<Double> wyv(ns);

		initializeCoordinateVectors( px,py,wx,wy,pxv,pyv,wxv,wyv );


		if (ns < 1) return;
		setTitle( ptype );

		setPositionStatus( pxv,pyv,wxv,wyv );

		//Get Profile Flux density v/s coordinateType
		bool ok = assignFrequencyProfile( wxv,wyv,
				coordinateType,xaxisUnit,z_xval,z_yval, getRegionShape());
		if ( !ok ) {
			return;
		}
		ok = setErrorPlotting( wxv, wyv );
		if ( !ok ) {
			return;
		}
		// scale for better display
		Int ordersOfM = scaleAxis();

		adjustPlotUnits();
		setPixelCanvasYUnits( yUnitPrefix, yUnit);

		// plot the graph
		plotMainCurve();
		addImageAnalysisGraph( wxv,wyv, ordersOfM );
		storeCoordinates( pxv, pyv, wxv, wyv );

		momentSettingsWidget->setCollapseVals( z_xval );
		specFitSettingsWidget->setCollapseVals( z_xval );

		resetYUnits( currentYUnits );
	}


	void QtProfile::changePlotType(const QString &text) {

		// store the plot type and set the class data
		persist( ".plot.type", text );
		stringToPlotType(text, itsPlotType);

		// get the coo-sys
		DisplayCoordinateSystem cSys = image->coordinates();
		yUnit = QString(image->units().getName().chars());

		switch (itsPlotType) {
		case QtProfile::PMEAN:
			if (npoints !=1 && errorMode->findText("rmse") < 0)
				errorMode->insertItem(1, "rmse");
			if (npoints ==1 && errorMode->findText("rmse") > -1)
				errorMode->removeItem(errorMode->findText("rmse"));
			if (cSys.qualityAxisNumber() > -1 && errorMode->findText("propagated") < 0)
				errorMode->insertItem(2, "propagated");
			break;
		case QtProfile::PMEDIAN:
			if (npoints !=1 && errorMode->findText("rmse") < 0)
				errorMode->insertItem(1, "rmse");
			if (npoints ==1 && errorMode->findText("rmse") > -1)
				errorMode->removeItem(errorMode->findText("rmse"));
			if (errorMode->findText("propagated") > -1)
				errorMode->removeItem(errorMode->findText("propagated"));
			break;
		case QtProfile::PSUM:
		case QtProfile::PFLUX:
			if (errorMode->findText("rmse") > -1)
				errorMode->removeItem(errorMode->findText("rmse"));
			if (cSys.qualityAxisNumber() > -1 && errorMode->findText("propagated") < 0)
				errorMode->insertItem(1, "propagated");
			break;
			//case QtProfile::PVRMSE:
			//	if (errorMode->findText("rmse") > -1)
			//		errorMode->removeItem(errorMode->findText("rmse"));
			//	if (errorMode->findText("propagated") > -1)
			//		errorMode->removeItem(errorMode->findText("propagated"));
			//	break;
		}
		redraw();
	}

	void QtProfile::changeErrorType(const QString &text) {
		persist( ".error.type", text);

		stringToErrorType(text, itsErrorType);

		redraw();
	}

	void QtProfile::redraw( ) {
		wcChanged( last_event_cs, last_event_px, last_event_py, last_event_wx, last_event_wy, UNKNPROF);
	}


	void QtProfile::changeAxis(String xa, String ya, String za, std::vector<int> ) {

		// the logic is as follows:
		// "za" (the z-axis") MUST be named "Frequency" or "Quality"
		// "xa" and "ya" must contain EITHER "Right" and "Declination"
		// OR "Latitude" and "Longitude".
		// The order is not important, and "cb=1" marks "normal" order
		// (xa=Ra/Lo, ya=Dec/La) while "cb=1" marks "not normal" order
		// (xa=Dec/La, ya=Ra/Lo).
		int cb = 0;
		if (xa.contains("Decl") && ya.contains("Right")){
			cb = -1;
		}
		if (xa.contains("Right") && ya.contains("Decl")){
			cb = 1;
		}
		if (xa.contains("atitu") && ya.contains("ongitu")){
			cb = -1;
		}
		if (xa.contains("ongitu") && ya.contains("atitu")){
			cb = 1;
		}
		if (!za.contains("Freq") && !za.contains("Quality")){
			cb = 0;
		}

		if (cb==0) {
			// the current configuration can
			// NOT be plotted
			xpos = "";
			ypos = "";
			position = QString("");
			profileStatus->showMessage(position);
			pixelCanvas->setWelcome("No profile available "
			                        "for the given data \nor\n"
			                        "No profile available for the "
			                        "display axes orientation"
			                       );
			xpos = "";
			ypos = "";
			position = QString("");
			profileStatus->showMessage(position);
			pixelCanvas->clearCurve();
		} else {
			if (cb != cube) {
				// either the profiler was initialized
				// or the axes have changed
				xpos = "";
				ypos = "";
				position = QString("");
				profileStatus->showMessage(position);
				pixelCanvas->setWelcome("assign a mouse button to\n"
				                        "'crosshair' or 'rectangle' or 'polygon'\n"
				                        "click/press+drag the assigned button on\n"
				                        "the image to get a spectral profile");
				pixelCanvas->clearCurve();
			}
		}

		// store the value
		cube = cb;
	}

	void QtProfile::changeSpectrum(String spcTypeUnit, String spcRval, String spcSys) {
		// NOTE: A change of any of these values requires a re-plot of the
		//       extracted spectrum. A change in "spcTypeUnit" and "spcSys"
		//       causes a change in a GUI element which then invokes a re-plot.
		//       However if "spcRval" changes, a re-plot must be enforced.
		*itsLog << LogOrigin("QtProfile", "changeSpectrum");

		// transform to QString
		QString  qSpcTypeUnit = QString(spcTypeUnit.c_str());
		QString  qSpcRval(spcRval.c_str());
		QString  qSpcSys(spcSys.c_str());

		if (qSpcSys != spcRef->currentText()) {
			// if necessary, change the spectral frame
			int index = spcRef->findText(qSpcSys);
			if (index > -1){
				spcRef->setCurrentIndex(index);
			}
		}

		//If the spectral unit in the data display options changes we should
		//reset the units (CAS-5824).
		if (qSpcTypeUnit != bottomAxisCType->currentText()) {
			// if necessary, change the unit and the spectral quantity
			updateAxisUnitCombo( qSpcTypeUnit, bottomAxisCType );

		}

		if (spcRval != cSysRval) {
			// if necessary, change the rest freq./wavel.
			cSysRval = spcRval;

			//Put the new rest frequency into the converter.
			QString restFreqStr( cSysRval.c_str());
			QRegExp characterExpression("[a-df-zA-Z]" );
			int unitIndex = restFreqStr.indexOf( characterExpression );
			if ( unitIndex > 0 ) {
				restFreqStr= restFreqStr.mid( 0, unitIndex);
			}
			bool validNumber = false;
			double restFreq = restFreqStr.toDouble(&validNumber);
			if ( validNumber ) {
				Converter::setRestFrequency( restFreq );
			} else {
				qDebug() << "QtProfiler::rest frequency parse error: "<<restFreqStr;
			}

			// an immediate replot has to be triggered such that the
			// new rest frequency is taken into account this COULD be done as such:
			if(lastPX.nelements() > 0) { // update display with new rest frequency/wavelength
				wcChanged(coordinate, lastPX, lastPY, lastWX, lastWY, UNKNPROF);
				//Reset the channel marker to whatever it was before.
				frameChanged( frameIndex );
			}
		}
	}
	void QtProfile::updateAxisUnitCombo( const QString& textToMatch, QComboBox* axisUnitCombo ) {
		int index = axisUnitCombo->findText( textToMatch );
		if (index > -1) {
			axisUnitCombo->setCurrentIndex(index);
		} /*else {
			//
			*itsLog << LogIO::WARN << "Can not switch profile to spectral quantity and unit: \"" << textToMatch.toStdString() << "\"!" << LogIO::POST;
		}*/
	}

	bool QtProfile::isAxisAscending(const Vector<Float>& axisValues ) const {
		bool axisAscending = true;
		if ( axisValues.size() > 0 ) {
			float first = axisValues[0];
			float last = axisValues[axisValues.size() - 1];
			if ( last < first ) {
				axisAscending = false;
			}
		}
		return axisAscending;
	}

	int QtProfile::getChannelCount( ImageAnalysis* analysis ){
		std::tr1::shared_ptr<const ImageInterface<float> > img(analysis->getImage());
		DisplayCoordinateSystem cSys = img->coordinates();
		Int spectralIndex = cSys.spectralAxisNumber();
		IPosition imgShape = img->shape();
		int channelCount = -1;
		if ( spectralIndex >= 0 ){
			channelCount = imgShape[spectralIndex];
		}
		else {
			Int tabularIndex = Util::getTabularFrequencyAxisIndex( img );
			if ( tabularIndex >= 0 ){
				channelCount = imgShape[tabularIndex];
			}
			else {
				qDebug() << "Could not find a tabular or spectral frequency axis: "<<img->name().c_str();
			}
		}
		return channelCount;
	}

	ImageAnalysis* QtProfile::findImageWithMaximumChannels(){
		ImageAnalysis* maxChannelAnalysis = analysis;
		int maxChannelCount = getChannelCount( analysis );
		if ( over ){
			QListIterator<OverplotAnalysis> i(*over);
			while (i.hasNext() && stateMProf) {
				OverplotAnalysis overplot = i.next();
				QString ky = overplot.first;
				ImageAnalysis* ana = overplot.second;
				int channelCount = getChannelCount( ana );
				if ( channelCount > maxChannelCount ){
					maxChannelAnalysis = ana;
					maxChannelCount = channelCount;
				}
			}
		}
		return maxChannelAnalysis;
	}

	void QtProfile::changeTopAxis() {

		if ( topAxisCType->isEnabled() && lastWX.size() /*>= 2*/> 0 ) {
			Vector<Float> xValues (lastWX.size());
			Vector<Float> yValues (lastWY.size());
			QString text = topAxisCType ->currentText();
			updateXAxisLabel( text, QtPlotSettings::xTop );
			String xUnits = String(text.toStdString());
			String coordinateType;
			String cTypeUnit;
			getcoordTypeUnit(xUnits, coordinateType, cTypeUnit);

			//Check to see if the bottom axis ordering is ascending in x
			bool bottomAxisAscendingX = isAxisAscending( z_xval );

			//Get the minimum and maximum x-value for the top axis
			//Of all the images we are overplotting, use the one with the maximum
			//number of channels.
			ImageAnalysis* maxChannelAnalysis = findImageWithMaximumChannels();
			int tabularIndex = getFreqProfileTabularIndex( maxChannelAnalysis );
			//We don't have to worry about smoothing here because we are only using
			//the x-values.
			bool ok=maxChannelAnalysis->getFreqProfile( lastWX, lastWY, xValues, yValues,
						                             WORLD_COORDINATES, coordinateType, 0,
						                             tabularIndex, 0, cTypeUnit, spcRefFrame,
						                             (Int)QtProfile::MEAN, 0, cSysRval, -1, getRegionShape());
			if ( ok ){
				//Check to see if the top axis ordering is ascending in x
				bool topAxisAscendingX = isAxisAscending(xValues);

				//We will have to show the labels of the top axis descending if the
				//order does not match that of the bottom axis.
				bool topAxisDescending = false;
				if ( topAxisAscendingX != bottomAxisAscendingX ) {
					topAxisDescending = true;
				}
				pixelCanvas -> setTopAxisRange( xValues, topAxisDescending );
			}
		}
	}

	void QtProfile::plotMainCurve() {
		pixelCanvas -> clearCurve();

		pixelCanvas -> plotPolyLine(z_xval, z_yval, z_eval, fileName);
		specFitSettingsWidget->setCurveName( fileName );
		adjustTopAxisSettings();
		changeTopAxis();
	}

	int QtProfile::findNearestChannel( float xval ) const {
		//This finds the channel closest to the passed in value.
		int channel_num = 0;
		bool forward = false;
		int size = z_xval.size();
		if(  z_xval[0] < z_xval[size-1] ) {
			forward = true;
		}
		if ( forward ) {
			for ( int i=0; i < size; ++i ) {
				if ( xval > z_xval[i] ) {
					channel_num = i;
				}
			}
			if ( channel_num < (size-1) ) {
				if ( (xval-z_xval[channel_num]) > (z_xval[channel_num+1]-xval) ) {
					channel_num += 1;
				}
			}
		} else {
			for ( int i=0; i < size; ++i ) {
				if ( xval < z_xval[i] ) {
					channel_num = i;
				}
			}
			if ( channel_num > 0 ) {
				if ( (xval-z_xval[channel_num]) > (z_xval[channel_num-1]-xval) ) {
					channel_num -= 1;
				}
			}
		}
		return channel_num;
	}

	void QtProfile::channelSelect( float xval ) {
		unsigned int size = z_xval.size( );
		if ( size == 0 ) return;
		int channelIndex = findNearestChannel( xval );
		emit channelSelect( channelIndex );
	}

	void QtProfile::channelRangeSelect( float channelStart, float channelEnd ) {
		unsigned int size = z_xval.size( );
		if ( size == 0 ) return;
		int channelStartIndex = findNearestChannel( channelStart );
		int channelEndIndex = findNearestChannel( channelEnd );
		if ( channelStartIndex != channelEndIndex ) {
			emit movieChannel( channelStartIndex, channelEndIndex );
		}
	}


	void QtProfile::setCollapseRange(double xmin, double xmax) {
		momentSettingsWidget->setRange( xmin, xmax );
		specFitSettingsWidget->setRange( xmin, xmax );
		lineOverlaysHolder->setRange( xmin, xmax, xaxisUnit );
	}

	void QtProfile::overplot(QList<OverplotInterface> hash) {
		// re-set the images that are overplotted
		if (over) {
			delete over;
			over = 0;
		}

		over = new QList<OverplotAnalysis>();
		QListIterator<OverplotInterface> i(hash);
		while (i.hasNext()) {
			OverplotInterface overplot = i.next();
			QString ky = overplot.first;
			ImageAnalysis* ana = new ImageAnalysis(overplot.second);
			OverplotAnalysis overlay( ky, ana );
			(*over).append( overlay );
		}
		newOverplots = true;
	}

	void QtProfile::clearPlots(){
		pixelCanvas->clearCurve();
		delete analysis;
		analysis = NULL;
		delete over;
		over = NULL;

	}


	void QtProfile::newRegion( int id_, const QString &shape, const QString &/*name*/,
	                           const QList<double> &world_x, const QList<double> &world_y,
	                           const QList<int> &pixel_x, const QList<int> &pixel_y,
	                           const QString &/*linecolor*/, const QString & /*text*/, const QString &/*font*/, int /*fontsize*/, int /*fontstyle*/ ) {
		if (!isVisible()) return;
		if (!analysis) return;
		if (shape == "polyline") {
			return;
		}

		//Only treat it as a new region if we haven't already registered
		//it in the map.  This method executing multiple times was causing
		//images with many regions to be slow to load.  However, if new Overplots
		//have been added since the last time this method was called for the
		//region, we need to execute in order to add the overplots to the
		//graph.
		int occurances = spectra_info_map.count( id_ );
		if ( occurances >= 1 && !newOverplots ) {
			return;
		}

		//Okay, we are committed to doing something.  Try to store the
		//y-axis units since the routine is going to munge them, then
		//we will attempt to reset them at the end.
		QString currentYUnits = yAxisCombo->currentText();

		spectra_info_map[id_] = shape;
		String c(WORLD_COORDINATES);

		Vector<Double> px(pixel_x.size());
		Vector<Double> py(pixel_y.size());
		Vector<Double> wx(world_x.size());
		Vector<Double> wy(world_y.size());

		for ( uint x=0; x < px.nelements(); ++x ) px[x] = pixel_x[x];
		for ( uint x=0; x < py.nelements(); ++x ) py[x] = pixel_y[x];
		for ( uint x=0; x < wx.nelements(); ++x ) wx[x] = world_x[x];
		for ( uint x=0; x < wy.nelements(); ++x ) wy[x] = world_y[x];

		*itsLog << LogOrigin("QtProfile", "newRegion");

		bool cubeZero = checkCube();
		if ( cubeZero ) {
			return;
		}

		current_region_id = id_;

		copyToLastEvent( c, px, py, wx, wy );

		setPlotType( static_cast<int>(wx.size()) );

		assignCoordinate( c );

		Int ns;
		px.shape(ns);

		Vector<Double> pxv(ns);
		Vector<Double> pyv(ns);
		Vector<Double> wxv(ns);
		Vector<Double> wyv(ns);

		initializeCoordinateVectors( px,py,wx,wy,pxv,pyv,wxv,wyv );

		if (ns < 1) return;

		setTitle( shape );

		if ( pxv.size() > 0 ) {
			setPositionStatus( pxv, pyv, wxv, wyv );
		}

		//Get Profile Flux density v/s coordinateType
		bool ok = assignFrequencyProfile( wxv,wyv, coordinateType,
				xaxisUnit,z_xval, z_yval, shape.toStdString() );
		if ( !ok ) {
			return;
		}

		ok = setErrorPlotting( wxv, wyv);

		Int ordersOfM = scaleAxis();
		adjustPlotUnits( );


		// plot the graph
		plotMainCurve();

		addImageAnalysisGraph( wxv, wyv, ordersOfM );
		storeCoordinates( pxv, pyv, wxv, wyv );

		momentSettingsWidget->setCollapseVals( z_xval );
		specFitSettingsWidget->setCollapseVals( z_xval );
		positioningWidget->updateRegion( pxv, pyv, wxv, wyv );
		newOverplots = false;


		//Okay, now we try to reset the y-Axis units.
		resetYUnits( currentYUnits );
	}

	void QtProfile::resetYUnits( const QString& units ) {
		int unitIndex = yAxisCombo->findText( units );
		if ( unitIndex >= 0 ) {
			yAxisCombo->setCurrentIndex( unitIndex );
		}
	}

	void QtProfile::updateRegion( int id_, viewer::region::RegionChanges type, const QList<double> &world_x, const QList<double> &world_y,
	                              const QList<int> &pixel_x, const QList<int> &pixel_y ) {

		if (!isVisible()) return;
		if (!analysis) return;
		if ( type == viewer::region::RegionChangeDelete ) {
			SpectraInfoMap::iterator it = spectra_info_map.find(id_);
			if ( it != spectra_info_map.end() ) {
				spectra_info_map.erase( it );
				it = spectra_info_map.find(id_);
			}
			if ( id_ == current_region_id ) {
				pixelCanvas -> clearCurve();
				current_region_id = NO_REGION_ID;
			}
			return;
		}
		//Normally we go off the region selected event because the focus event is called
		//both when a region loses(bad) or gains focus(good).  However, in the case we
		//don't have a current_region_id, we'll take anything.

		else if ( (type == viewer::region::RegionChangeSelected && current_region_id == id_)) {
			return;
		} else if ( (type == viewer::region::RegionChangeSelected && current_region_id != id_) ||
		            (type == viewer::region::RegionChangeFocus && current_region_id == NO_REGION_ID )) {
			current_region_id = id_;			// viewer region focus has changed
		}

		else if ( type == viewer::region::RegionChangeFocus ) {
			return;
		}
		//Added because profiler is just getting bogged down in too many updates.
		else if ( type == viewer::region::RegionChangeUpdate ){
			return;
		}

		else if ( type == viewer::region::RegionChangeNewChannel ) {
			return;						// viewer moving to new channel
		} else if ( id_ != current_region_id ) {
			return;						// some other region
		}

		//Since we are now committed to changing the data, we need to store the
		//current yUnits so we can try to reset them later.
		QString currentYUnits = yAxisCombo->currentText();


		SpectraInfoMap::iterator it = spectra_info_map.find(id_);
		if ( it == spectra_info_map.end( ) ) return;
		QString shape = it->second.shape( );

		String c(WORLD_COORDINATES);

		Vector<Double> px(pixel_x.size());
		Vector<Double> py(pixel_y.size());
		Vector<Double> wx(world_x.size());
		Vector<Double> wy(world_y.size());

		for ( uint x=0; x < px.nelements(); ++x ) px[x] = pixel_x[x];
		for ( uint x=0; x < py.nelements(); ++x ) py[x] = pixel_y[x];
		for ( uint x=0; x < wx.nelements(); ++x ) {
			wx[x] = world_x[x];
		}
		for ( uint x=0; x < wy.nelements(); ++x ) {
			wy[x] = world_y[x];
		}

		*itsLog << LogOrigin("QtProfile", "updateRegion");

		bool cubeZero = checkCube();
		if ( cubeZero ) {
			return;
		}

		copyToLastEvent( c, px, py, wx, wy );

		setPlotType( wx.size() );

		assignCoordinate( c );

		Int ns;
		px.shape(ns);

		Vector<Double> pxv(ns);
		Vector<Double> pyv(ns);
		Vector<Double> wxv(ns);
		Vector<Double> wyv(ns);

		initializeCoordinateVectors( px, py, wx, wy, pxv, pyv, wxv, wyv );

		if (ns < 1) return;

		setTitle( shape );

		setPositionStatus( pxv,pyv,wxv,wyv );
		positioningWidget->updateRegion( pxv,pyv,wxv,wyv);

		//Get Profile Flux density v/s coordinateType
		bool ok = assignFrequencyProfile( wxv,wyv, coordinateType, xaxisUnit,
				z_xval,z_yval, shape.toStdString());

		if ( !ok ) {
			return;
		}

		setErrorPlotting( wxv,wyv);

		// scale for better display
		Int ordersOfM = scaleAxis();

		// plot the graph
		plotMainCurve();

		addImageAnalysisGraph( wxv, wyv, ordersOfM );
		storeCoordinates( pxv, pyv, wxv, wyv );

		momentSettingsWidget->setCollapseVals(z_xval );
		specFitSettingsWidget->setCollapseVals( z_xval );

		resetYUnits( currentYUnits );
	}


	bool QtProfile::exportFITSSpectrum(QString &fn) {
		// get the image coo and the spectral axis therein
		DisplayCoordinateSystem cSys = image->coordinates();
		Int wCoord = cSys.findCoordinate(Coordinate::SPECTRAL);
		Vector<Int> pCoord = cSys.pixelAxes(wCoord);

		if (wCoord <0) {
			// well, it REALLY should not get to here
			QString msg="No spectral coordinate in image:\n" + QString((image->name(True)).c_str());
			messageFromProfile(msg);
			return false;
		}

		// get the spectral dimension and make some checks
		uInt nPoints=(image->shape())(pCoord(0));
		if (nPoints != z_yval.size()) {
			// well, this should not happen
			QString msg="The dimension of the image and\nthe extracted profile do not match!";
			messageFromProfile(msg);
			return false;
		} else if (nPoints<1) {
			// well, this should not happen
			QString msg="The extracted profile contains no points!";
			messageFromProfile(msg);
			return false;
		}

		// create a new coo and add the spectral one
		DisplayCoordinateSystem csysProfile = DisplayCoordinateSystem();
		csysProfile.addCoordinate(cSys.spectralCoordinate(wCoord));

		// if necessary, add a quality coordinate
		IPosition profDim;
		if (z_eval.size()>0) {
			Vector<Int> quality(2);
			quality(0) = Quality::DATA;
			quality(1) = Quality::ERROR;
			QualityCoordinate qualAxis(quality);
			csysProfile.addCoordinate(qualAxis);

			profDim = IPosition(2,nPoints,2);
		} else {
			profDim = IPosition(1,nPoints);
		}

		// create the temp-image
		TempImage<Float> profile(TiledShape(profDim),csysProfile);

		// scale the data and store the values in the
		// temp-image
		Float scaleFactor=pow(10.,ordersOfM_);
		IPosition posIndex;
		if (z_eval.size()>0) {
			Int qualCooPos, qualIndex;
			posIndex = IPosition(2, 0);

			// re-find the quality coordinate
			qualCooPos = csysProfile.findCoordinate(Coordinate::QUALITY);
			if (qualCooPos<0) {
				// this really should not happen
				QString msg="Error finding the Quality coordinate!";
				messageFromProfile(msg);
				return false;
			}

			// get the pixel index of DATA
			if (!(csysProfile.qualityCoordinate(qualCooPos)).toPixel(qualIndex, Quality::DATA)) {
				QString msg="Error finding the DATA index in quality coordinate!";
				messageFromProfile(msg);
				return false;
			}
			posIndex(1)=qualIndex;
			for (uInt index=0; index<nPoints; index++) {
				posIndex(0)=index;
				profile.putAt (z_yval(index)/scaleFactor, posIndex);
				//cout << " " << z_yval(index)/scaleFactor << " : " << posIndex;
			}


			// get the pixel index of ERROR
			if (!(csysProfile.qualityCoordinate(qualCooPos)).toPixel(qualIndex, Quality::ERROR)) {
				QString msg="Error finding the ERROR index in quality coordinate!";
				messageFromProfile(msg);
				return false;
			}
			posIndex(1)=qualIndex;
			for (uInt index=0; index<nPoints; index++) {
				posIndex(0)=index;
				profile.putAt (z_eval(index)/scaleFactor, posIndex);
				//cout << " " << z_eval(index)/scaleFactor << " : " << posIndex;
			}
		} else {
			posIndex = IPosition(1);
			for (uInt index=0; index<nPoints; index++) {
				posIndex(0)=index;
				profile.putAt (z_yval(index)/scaleFactor, posIndex);
				//cout << " " << z_yval(index)/scaleFactor;
			}
		}

		// attach a mask to the temp-image
		Array<Bool> maskArray(profDim, True);
		ArrayLattice<Bool> maskLattice=ArrayLattice<Bool>(maskArray);
		profile.attachMask(maskLattice);

		// compile and set the miscInfo
		TableRecord miscInfo;
		miscInfo.define("inimage", image->name(True));
		miscInfo.setComment("inimage", "name input image");
		miscInfo.define("position", position.toStdString());
		miscInfo.setComment("position", "extraction position");
		miscInfo.define("proftype", (pixelCanvas->getTitle()).toStdString());
		miscInfo.setComment("proftype", "the profile type");
		miscInfo.define("plottype", (plotMode->currentText()).toStdString());
		miscInfo.setComment("plottype", "the plot type");
		if (z_eval.size()>0) {
			miscInfo.define("errtype", (errorMode->currentText()).toStdString());
			miscInfo.setComment("errtype", "the error type");
		}
		profile.setMiscInfo(miscInfo);

		// thats the default values for the call "ImageFITSConverter::ImageToFITS"
		String error;
		uInt memoryInMB(64);
		Bool preferVelocity(True);
		Bool opticalVelocity(True);
		Int BITPIX(-32);
		Float minPix(1.0);
		Float maxPix(-1.0);
		Bool allowOverwrite(False);
		Bool degenerateLast(False);
		Bool verbose(True);
		Bool stokesLast(False);
		Bool preferWavelength(False);
		Bool preferAirWavelength(False);
		String origin("CASA Viewer / Spectral Profiler");
		allowOverwrite=True;
		String outFile(fn.toStdString());

		// find the "natural" flags for the spectral axis
		SpectralCoordinate::SpecType spcType = (cSys.spectralCoordinate(wCoord)).nativeType();
		switch (spcType) {
		case SpectralCoordinate::FREQ:
			preferVelocity      = False;
			opticalVelocity     = False;
			preferWavelength    = False;
			preferAirWavelength = False;
			break;
		case SpectralCoordinate::VRAD:
			preferVelocity      = True;
			opticalVelocity     = False;
			preferWavelength    = False;
			preferAirWavelength = False;
			break;
		case SpectralCoordinate::VOPT:
			preferVelocity      = True;
			opticalVelocity     = True;
			preferWavelength    = False;
			preferAirWavelength = False;
			break;
		case SpectralCoordinate::BETA:
			preferVelocity      = False;
			opticalVelocity     = False;
			preferWavelength    = False;
			preferAirWavelength = False;
			break;
		case SpectralCoordinate::WAVE:
			preferVelocity      = False;
			opticalVelocity     = False;
			preferWavelength    = True;
			preferAirWavelength = False;
			break;
		case SpectralCoordinate::AWAV:
			preferVelocity      = False;
			opticalVelocity     = False;
			preferWavelength    = True;
			preferAirWavelength = True;
			break;
		default:
			preferVelocity      = False;
			opticalVelocity     = False;
			preferWavelength    = False;
			preferAirWavelength = False;
		}

		try {
			// export the image to FITS
			ImageFITSConverter::ImageToFITS(
			    error,
			    profile,
			    outFile,
			    memoryInMB,
			    preferVelocity,
			    opticalVelocity,
			    BITPIX,
			    minPix,
			    maxPix,
			    allowOverwrite,
			    degenerateLast,
			    verbose,
			    stokesLast,
			    preferWavelength,
			    preferAirWavelength,
			    origin
			);
		} catch (AipsError x) {
			// catch an exception and report
			QString msg="Error while exporting FITS:\n"+QString((x.getMesg()).c_str());
			messageFromProfile(msg);
			return false;
		}

		// check for any error indicated via the error-string
		if (error.size()>0) {
			QString msg="Error while exporting FITS:\n"+QString((error.c_str()));
			messageFromProfile(msg);
			return false;
		}

		return true;
	}

	bool QtProfile::exportASCIISpectrum(QString &fn) {
		QFile file(fn);
		if (!file.open(QFile::WriteOnly | QIODevice::Text))
			return false;
		QTextStream ts(&file);


		// get the scale factor
		Float scaleFactor=pow(10.,-ordersOfM_);
		ts.setRealNumberNotation(QTextStream::ScientificNotation);
		ts.setRealNumberPrecision(12);

		//There should be a space separating the ra from the dec.
		int coordinateCount = lastWX.size();
		QList<QString> cornerCoordinatesWorld;
		QList<QString> cornerCoordinatesPixel;
		for ( int i = 0; i < coordinateCount; i++ ) {
			QString raDecStr = getRaDec( lastWX[i], lastWY[i]);
			int decIndex = raDecStr.indexOf("+");
			if ( decIndex == -1 ) {
				decIndex = raDecStr.indexOf( "-");
			}
			raDecStr.insert(decIndex, ", ");
			cornerCoordinatesWorld.append("["+raDecStr+"]");
			cornerCoordinatesPixel.append("["+ QString::number(lastPX[i])+
					", "+QString::number(lastPY[i])+"]");
		}

		ts << "#title: Spectral profile - " << fileName << "\n";
		ts << "#region (world): "<< region << "[";
		for ( int i = 0; i < coordinateCount; i++ ) {
			ts << cornerCoordinatesWorld[i];
			if ( i != coordinateCount - 1 ) {
				ts << ", ";
			}
		}
		ts << "]\n";
		ts << "#region (pixel): "<< region << "[";
		for ( int i = 0; i < coordinateCount; i++ ) {
			ts << cornerCoordinatesPixel[i];
			if ( i != coordinateCount - 1 ) {
				ts << ", ";
			}
		}
		ts << "]\n";
		ts << "#coordinate: " << QString(coordinate.chars()) << "\n";
		ts << "#xLabel: " << QString(ctypeUnit.chars()) << "\n";
		ts << "#yLabel: " << "[" << yUnit << "] "<< plotMode->currentText() << "\n";
		if (z_eval.size() > 0){
			ts << "#eLabel: " << "[" << yUnit << "] " << errorMode->currentText() << "\n";
		}



		/*if (z_eval.size() > 0) {
				for (uInt i = 0; i < z_xval.size(); i++) {
					ts << z_xval(i) << "    " << scaleFactor*z_yval(i) << "    "  << scaleFactor*z_eval(i) << "\n";
				}
			} else {
				for (uInt i = 0; i < z_xval.size(); i++) {
					ts << z_xval(i) << "    " << scaleFactor*z_yval(i) << "\n";
				}
			}*/


		int curveCount = pixelCanvas->getLineCount();
		for (int k = 1; k < curveCount; k++) {
			outputCurve( k, ts, scaleFactor );
		}
		if ( curveCount > 0 ){
			outputCurve( 0, ts, scaleFactor );
		}
		return true;
	}

	void QtProfile::outputCurve( int k, QTextStream& ts, Float scaleFactor ){
		ts << "\n";
		ts << "# " << pixelCanvas->getCurveName(k) << "\n";
		CurveData data = pixelCanvas->getCurveData(k);
		ErrorData errorData = pixelCanvas->getCurveError(k);
		int j = data.size() / 2;
		int errorDataSize = errorData.size();
		for (int m = 0; m < j; m++) {
			ts << data[2 * m] << " " << scaleFactor*data[2 * m + 1];
			if ( errorDataSize > 0 ){
				ts << " " << scaleFactor * errorData[2 * m + 1];
			}
			ts << "\n";
		}
	}

	void QtProfile::messageFromProfile(QString &msg) {
		//QMessageBox qMsg(this);
		//qMsg.setText(msg);
		//qMsg.exec();
		// critical
		QMessageBox::critical(this, "Error", msg);
	}


	void QtProfile::fillPlotTypes(const std::tr1::shared_ptr<ImageInterface<Float> > img) {

		// check whether plot mode "flux" make sense
		bool allowFlux(false);
		const Unit& brightnessUnit = img->units();
		String bUName = brightnessUnit.getName();
		bUName.downcase();
		if(bUName.contains("/beam"))
			allowFlux=true;

		if (plotMode->count() <1 ) {
			// fill the plot types
			plotMode->addItem( PLOT_TYPE_MEAN );
			plotMode->addItem( PLOT_TYPE_MEDIAN );
			plotMode->addItem( PLOT_TYPE_SUM );
			if (allowFlux)
				plotMode->addItem( PLOT_TYPE_FLUX );

			// read the preferred plot mode from casarc
			QString pref_plotMode = read(".plot.type");
			if (pref_plotMode.size()>0) {
				// change to the preferred plotMode
				int etypeindex= plotMode->findText(pref_plotMode);
				if (etypeindex > -1)
					plotMode->setCurrentIndex(etypeindex);
				else
					plotMode->setCurrentIndex(0);
			}
			stringToPlotType(plotMode->currentText(), itsPlotType);
		} else {
			// add/remove "flux" if necessary
			if (allowFlux) {
				if (plotMode->findText( PLOT_TYPE_FLUX )<0)
					plotMode->addItem( PLOT_TYPE_FLUX );
			} else {
				if (plotMode->findText( PLOT_TYPE_FLUX ) > -1)
					plotMode->removeItem(plotMode->findText( PLOT_TYPE_FLUX ));
			}
		}

		// clean out the error box
		if (errorMode->count() > 0) {
			if (errorMode->findText("propagated") > -1) {
				errorMode->removeItem(errorMode->findText("propagated"));
			}
			if (errorMode->findText("rmse") > -1) {
				errorMode->removeItem(errorMode->findText("rmse"));
			}
		}

		// get the coo-sys
		DisplayCoordinateSystem cSys = image->coordinates();

		// add the 'no error' option
		if (errorMode->findText("no error") < 0)
			errorMode->addItem("no error");

		switch (itsPlotType) {
		case QtProfile::PMEAN:
			if (npoints!=1)
				errorMode->addItem("rmse");
			if (cSys.qualityAxisNumber() > -1)
				errorMode->addItem("propagated");
			break;
		case QtProfile::PMEDIAN:
			if (npoints!=1)
				errorMode->addItem("rmse");
			break;
		case QtProfile::PSUM:
		case QtProfile::PFLUX:
			if (cSys.qualityAxisNumber() > -1)
				errorMode->addItem("propagated");
			break;
			//case QtProfile::PVRMSE:
			//	break;
		}

		// read the preferred error mode from casarc
		QString pref_errMode = read( ".error.type");
		if (pref_errMode.size()>0) {
			// change to the preferred plotMode
			int e_index= errorMode->findText(pref_errMode);
			if (e_index > -1)
				errorMode->setCurrentIndex(e_index);
			else
				errorMode->setCurrentIndex(0);
		}

		stringToErrorType(errorMode->currentText(), itsErrorType);
		persist( ".error.type", errorMode->currentText());
	}


	void QtProfile::stringToPlotType(const QString &text, QtProfile::PlotType &pType) {
		*itsLog << LogOrigin("QtProfile", "stringToPlotType");

		if (!text.compare(QString( PLOT_TYPE_MEAN )))
			pType = QtProfile::PMEAN;
		else if (!text.compare(QString( PLOT_TYPE_MEDIAN )))
			pType = QtProfile::PMEDIAN;
		else if (!text.compare(QString( PLOT_TYPE_SUM )))
			pType = QtProfile::PSUM;
		else if (!text.compare(QString( PLOT_TYPE_FLUX )))
			pType = QtProfile::PFLUX;
		//else if (!text.compare(QString("rmse")))
		//	pType = QtProfile::PVRMSE;
		else
			*itsLog << LogIO::WARN << "The string: " << text.toStdString() << " does not correspond to a plot type!" << LogIO::POST;
	}

	void QtProfile::stringToErrorType(const QString &text, QtProfile::ErrorType &eType) {
		*itsLog << LogOrigin("QtProfile", "stringToErrorType");

		if (!text.compare(QString("no error")))
			eType = QtProfile::PNOERROR;
		else if (!text.compare(QString("rmse")))
			eType = QtProfile::PERMSE;
		else if (!text.compare(QString("propagated")))
			eType = QtProfile::PPROPAG;
		else
			*itsLog << LogIO::WARN << "The string: " << text.toStdString() << " does not correspond to an error type!" << LogIO::POST;
	}


	void QtProfile::getcoordTypeUnit(String &ctypeUnitStr, String &cTypeStr, String &unitStr) {
		// determine the coordinate type
		if (ctypeUnitStr.contains("air wavelength"))
			cTypeStr = String("air wavelength");
		else if (ctypeUnitStr.contains("wavelength"))
			cTypeStr = String("wavelength");
		else if (ctypeUnitStr.contains("radio velocity"))
			cTypeStr = String("radio velocity");
		else if (ctypeUnitStr.contains("optical velocity"))
			cTypeStr = String("optical velocity");
		else if (ctypeUnitStr.contains("frequency"))
			cTypeStr = String("frequency");
		else
			cTypeStr = String( CHANNEL.toStdString() );

		// determine the unit
		if (ctypeUnitStr.contains("[Hz]"))
			unitStr = String("Hz");
		else if (ctypeUnitStr.contains("[MHz]"))
			unitStr = String("MHz");
		else if (ctypeUnitStr.contains("[GHz]"))
			unitStr = String("GHz");
		else if (ctypeUnitStr.contains("[m/s]"))
			unitStr = String("m/s");
		else if (ctypeUnitStr.contains("[km/s]"))
			unitStr = String("km/s");
		else if (ctypeUnitStr.contains("[mm]"))
			unitStr = String("mm");
		else if (ctypeUnitStr.contains("[um]"))
			unitStr = String("um");
		else if (ctypeUnitStr.contains("[nm]"))
			unitStr = String("nm");
		else if (ctypeUnitStr.contains("[Angstrom]"))
			unitStr = String("Angstrom");
		else
			unitStr = String("");
	}


	void QtProfile::printIt(QPrinter* printer) {
		QPainter painter(printer);
		QRect rect = painter.viewport();
		rect.adjust(72, 72, -72, -72);
		QPixmap *mp = pixelCanvas->graph();
		QSize size = mp->size();
		size.scale(rect.size(), Qt::KeepAspectRatio);
		painter.setViewport(rect.x(), rect.y(), size.width(), size.height());
		painter.setWindow(mp->rect());
		painter.drawPixmap(0, 0, *mp);
		painter.end();
	}

	QString QtProfile::getRaDec(double x, double y) {

		int h, m;
		double ras;
		Util::getRa( x, h, m, ras );
		int d, c;
		double decs;
		int sign = (y > 0) ? 1 : -1;
		Util::getDec( y, d, c, decs );

		QString raDec = "";
		raDec.append((h < 10) ? "0" : "").append(QString().setNum(h)).append(":")
		.append((m < 10) ? "0" : "").append(QString().setNum(m)).append(":")
		.append((ras < 10) ? "0" : "").append(QString().setNum(ras))
		.append((sign > 0) ? "+" : "-")
		.append((d < 10) ? "0" : "").append(QString().setNum(d)).append("d")
		.append((c < 10) ? "0" : "").append(QString().setNum(c)).append("m")
		.append((decs < 10) ? "0" : "").append(QString().setNum(decs));
		return raDec;
	}


	void QtProfile::setTitle(const ProfileType ptype){
		QString smoothDescription = smoothWidget->toString();
		switch (ptype) {
			case SINGPROF:
				pixelCanvas->setTitle("Single Point Profile"+smoothDescription);
				region = "Point";
				break;
			case RECTPROF:
				pixelCanvas->setTitle("Rectangle Region Profile"+smoothDescription);
				region = "Rect";
				break;
			case ELLPROF:
				pixelCanvas->setTitle("Elliptical Region Profile"+smoothDescription);
				region = "Ellipse";
				break;
			case POLYPROF:
				pixelCanvas->setTitle("Polygon Region Profile"+smoothDescription);
				region = "Poly";
				break;
			case UNKNPROF:
			{
				//In this case, we will not change the first part of the title,
				//but, we will update any smoothing.
				QString previousTitle = pixelCanvas->getTitle();
				const QString PROFILE( "Profile");
				int profileIndex = previousTitle.indexOf( PROFILE );
				if ( profileIndex >= 0 ){
					QString baseTitle = previousTitle.left( profileIndex + PROFILE.size());
					baseTitle = baseTitle + smoothDescription;
					pixelCanvas->setTitle(baseTitle );
				}
			}
				break;
			default:
				pixelCanvas->setTitle("");
				region = "";
		}
		pixelCanvas->setWelcome("");
	}

	void QtProfile::setTitle( const QString& shape ) {
		QString smoothDescription = smoothWidget->toString();
		if ( shape == "point" ) {
			pixelCanvas->setTitle("Single Point Profile"+smoothDescription);
			region = "Point";
		} else if ( shape == "rectangle" ) {
			pixelCanvas->setTitle("Rectangle Region Profile"+smoothDescription);
			region = "Rect";
		} else if ( shape == "ellipse" ) {
			pixelCanvas->setTitle("Elliptical Region Profile"+smoothDescription);
			region = "Ellipse";
		} else if ( shape == "polygon" ) {
			pixelCanvas->setTitle("Polygon Region Profile"+smoothDescription);
			region = "Poly";
		} else {
			pixelCanvas->setTitle("");
			region = "";
		}
		pixelCanvas->setWelcome("");
	}


	void QtProfile::copyToLastEvent( const String& c, const Vector<Double> &px,
	                                 const Vector<Double> &py,
	                                 const Vector<Double> &wx,
	                                 const Vector<Double> &wy ) {
		last_event_cs.resize(c.size());
		last_event_cs = c;

		last_event_px.resize(px.size());
		last_event_px = px;

		last_event_py.resize(py.size());
		last_event_py = py;

		last_event_wx.resize(wx.size());
		last_event_wx = wx;

		last_event_wy.resize(wy.size());
		last_event_wy = wy;

		npoints = wx.size();
	}

	void QtProfile::setPlotType( int wcArraySize ) {
		if (npoints_old==0 && wcArraySize>0) {
			npoints_old = wcArraySize;
			changePlotType(plotMode->currentText());
		} else {
			if (npoints==1 & npoints_old!=1) {
				npoints_old = npoints;
				changePlotType(plotMode->currentText());
			} else if (npoints!=1 & npoints_old==1) {
				npoints_old = npoints;
				changePlotType(plotMode->currentText());
			}
		}
	}

	bool QtProfile::checkCube() {
		bool cubeZero = false;
		if (cube == 0) {
			pixelCanvas->setWelcome("No profile available "
			                        "for the given data \nor\n"
			                        "No profile available for the "
			                        "display axes orientation");
			pixelCanvas->clearCurve();
			cubeZero = true;
		}
		return cubeZero;
	}

	void QtProfile::assignCoordinate( const String& c) {
		if (c != coordinate) {
			coordinate = c.chars();
			//if (coordinate == WORLD_COORDINATES)
			//    chk->setCurrentIndex(0);
			//else
			//    chk->setCurrentIndex(1);
		}
	}

	void QtProfile::initializeCoordinateVectors(const Vector<double> &px, const Vector<double> &py,
	        const Vector<double> &wx, const Vector<double> &wy, Vector<double> &pxv,
	        Vector<double> &pyv, Vector<double> &wxv, Vector<double> &wyv) const {
		if (cube == -1) {
			pxv.assign(py);
			pyv.assign(px);
			wxv.assign(wy);
			wyv.assign(wx);
		} else {
			pxv.assign(px);
			pyv.assign(py);
			wxv.assign(wx);
			wyv.assign(wy);
		}
	}

	void QtProfile::setPositionStatus(const Vector<double> &pxv, const Vector<double> &pyv,
	                                  const Vector<double> &wxv, const Vector<double> &wyv ) {
		Double xmean, ymean;
		if (WORLD_COORDINATES.compare( coordinate )== 0 ) {
			//xpos, ypos and position only used for display
			xpos = QString::number( Util::getCenter( wxv, xmean));
			ypos = QString::number( Util::getCenter( wyv, ymean));
			position = getRaDec(xmean, ymean);
		} else {

			//xpos, ypos and position only used for display
			xpos = QString::number(Util::getCenter(pxv,xmean));
			ypos = QString::number(Util::getCenter(pyv,ymean));
			position = QString("[%1, %2]").arg(xpos).arg(ypos);
		}
		profileStatus->showMessage(position);
	}

	bool QtProfile::getFrequencyProfileWrapper( ImageAnalysis* analysis, const Vector<double> &wxv, const Vector<double> &wyv,
	        Vector<Float> &z_xval, Vector<Float> &z_yval,
	        const String& xytype, const String& specaxis,
	        const Int& whichStokes, const Int& whichTabular,
	        const Int& whichLinear, const String& xunits,
	        const String& specFrame, const Int &combineType,
	        const Int& whichQuality, const String& restValue, const String& shape ) {

		Bool ok = false;

		Int beamChannel = 0;

		// might want to introduce more fancy selection of the
		//beamChannel here but using channel 0 is a good choice already
		ok = analysis->getFreqProfile( wxv, wyv, z_xval, z_yval,
		                               xytype, specaxis, whichStokes, whichTabular, whichLinear, xunits, specFrame,
		                               combineType, whichQuality, restValue,
		                               beamChannel, shape);

		//Note:  we don't have to worry about applying smoothing here because the callers
		//of this method currently do it.

		if ( itsPlotType == QtProfile::PFLUX ) {
			//Post a warning that flux was calculated using a given channel
			//and the resulting calculation was only an approximation.
			ostringstream oss;
			oss << "Calculation was performed using the beam for channel " << beamChannel << endl << "The result should be considered an approximation.";
			postStatus( oss.str() );
		}
		return ok;
	}



	bool QtProfile::assignFrequencyProfile( const Vector<double> &wxv, const Vector<double> &wyv,
	                                        const String& coordinateType, const String& xaxisUnit,
	                                        Vector<Float> &z_xval, Vector<Float> &z_yval,
	                                        const String& shape) {

		Int whichTabular = getFreqProfileTabularIndex( analysis );

		Bool ok = false;

		switch (itsPlotType) {
		case QtProfile::PMEAN:
			ok=analysis->getFreqProfile( wxv, wyv, z_xval, z_yval,
					WORLD_COORDINATES, coordinateType, 0, whichTabular, 0,
					xaxisUnit, spcRefFrame,
					(Int)QtProfile::MEAN, 0, cSysRval, -1, shape );
			break;
		case QtProfile::PMEDIAN:
			ok=analysis->getFreqProfile( wxv, wyv, z_xval, z_yval,
					WORLD_COORDINATES, coordinateType, 0, whichTabular, 0, xaxisUnit, spcRefFrame,
					(Int)QtProfile::MEDIAN, 0, cSysRval, -1, shape );
			break;
		case QtProfile::PSUM:
			ok=analysis->getFreqProfile( wxv, wyv, z_xval, z_yval,
					WORLD_COORDINATES, coordinateType, 0, whichTabular, 0, xaxisUnit, spcRefFrame,
					(Int)QtProfile::SUM, 0, cSysRval, -1, shape );
			break;
		case QtProfile::PFLUX:
			ok=getFrequencyProfileWrapper( analysis, wxv, wyv, z_xval, z_yval,
					WORLD_COORDINATES, coordinateType, 0, whichTabular, 0, xaxisUnit, spcRefFrame,
					(Int)QtProfile::FLUX, 0, cSysRval, shape);
			break;

		default:
			ok=analysis->getFreqProfile( wxv, wyv, z_xval, z_yval,
					WORLD_COORDINATES, coordinateType, 0, whichTabular, 0, xaxisUnit, spcRefFrame,
					(Int)QtProfile::MEAN, 0, cSysRval, -1, shape );
			break;
		}

		if (!ok) {
			// change to notify user of error...
			*itsLog << LogIO::WARN << "Can not generate the frequency profile!" << LogIO::POST;
		}
		else {
			//Apply smoothing to the vectors we get back.
			if ( smoothWidget != NULL ){
				z_yval = smoothWidget->applySmoothing( z_yval );
			}
		}
		return ok;
	}

	int QtProfile::getFreqProfileTabularIndex(ImageAnalysis* analysis ) {
		Int whichTabular = -1;
		std::tr1::shared_ptr<const ImageInterface<Float> > img = analysis->getImage();
		const DisplayCoordinateSystem& cSys = img->coordinates();
		if ( !cSys.hasSpectralAxis() ){
			whichTabular = Util::getTabularFrequencyAxisIndex( img );
		}
		return whichTabular;
	}

	bool QtProfile::setErrorPlotting( const Vector<double> &wxv, const Vector<double> &wyv ) {
		bool ok = true;
		// get the coordinate system
		DisplayCoordinateSystem cSys = image->coordinates();
		String shape = getRegionShape();

		int tabularIndex = getFreqProfileTabularIndex(analysis);
		switch (itsErrorType) {
		case QtProfile::PNOERROR:
			if (z_eval.size()> 0)
				z_eval.resize(0);
			break;
		case QtProfile::PERMSE:
			if ( wxv.size() < 2 ) {
				*itsLog << LogIO::NORMAL << "Can not do the plot request, only one point!" << LogIO::POST;
				if (z_eval.size()> 0)
					z_eval.resize(0);
			} else {
				switch (itsPlotType) {
				case QtProfile::PSUM:
				case QtProfile::PFLUX:
					*itsLog << LogIO::NORMAL << "Plotting RMSE as error of SUM makes no sense!" << LogIO::POST;
					if (z_eval.size()> 0)
						z_eval.resize(0);
					break;
					//case QtProfile::PVRMSE:
					//		 cout << "Plotting RMSE over RMSE makes no sense!" << endl;
					//  	 if (z_eval.size()> 0)
					//  		 z_eval.resize(0);
					//		 break;
				default:
					ok=analysis->getFreqProfile( wxv, wyv, z_xval, z_eval,
					                             WORLD_COORDINATES, coordinateType, 0, tabularIndex, 0, xaxisUnit, spcRefFrame,
					                             (Int)QtProfile::RMSE, 0, cSysRval, -1, shape);
					break;
				}
			}
			break;
		case QtProfile::PPROPAG:
			if ( cSys.qualityAxisNumber() < 0 ) {
				*itsLog << LogIO::NORMAL << "Can not do the plot request, no quality axis!" << LogIO::POST;
				if (z_eval.size()> 0)
					z_eval.resize(0);
			} else {
				switch ( itsPlotType ) {
				case QtProfile::PMEAN:
					ok=analysis->getFreqProfile( wxv, wyv, z_xval, z_eval,
					                             WORLD_COORDINATES, coordinateType, 0, tabularIndex, 0, xaxisUnit, spcRefFrame,
					                             (Int)QtProfile::NSQRTSUM, 1, cSysRval, -1, shape);
					break;
				case QtProfile::PMEDIAN:
					*itsLog << LogIO::NORMAL << "Can not plot the error, NO propagation for median!" << LogIO::POST;
					if (z_eval.size()> 0)
						z_eval.resize(0);
					break;
				case QtProfile::PSUM:
					ok=analysis->getFreqProfile( wxv, wyv, z_xval, z_eval,
					                             WORLD_COORDINATES, coordinateType, 0, tabularIndex, 0, xaxisUnit, spcRefFrame,
					                             (Int)QtProfile::SQRTSUM, 1, cSysRval, -1, shape);
					break;
				case QtProfile::PFLUX:
					ok=getFrequencyProfileWrapper( analysis, wxv, wyv, z_xval, z_eval,
					                               WORLD_COORDINATES, coordinateType, 0, tabularIndex, 0, xaxisUnit, spcRefFrame,
					                               (Int)QtProfile::EFLUX, 1, cSysRval, shape);
					break;
				default:
					if (z_eval.size()> 0)
						z_eval.resize(0);
					break;
				}
			}
			break;
		default:
			if (z_eval.size()> 0)
				z_eval.resize(0);
			break;
		}

		if ( ! ok ) {
			// change to notify user of error...
			*itsLog << LogIO::WARN << "Can not generate the frequency error profile!" << LogIO::POST;

		}
		return ok;
	}

	Int QtProfile::scaleAxis() {
		// scale for better display
		// max absolute display numbers should be between 0.1 and 100.0
		Double dmin = 0.1;
		Double dmax = 100.;
		Double ymin = min(z_yval);
		Double ymax = max(z_yval);
		ymax = max(abs(ymin), ymax);
		Double symax;
		Int ordersOfM = 0;
		symax = ymax;
		while(symax < dmin && ymax != 0.0) {
			ordersOfM += 3;
			symax = ymax * pow(10.,ordersOfM);
		}
		while(symax > dmax && ymax != 0.0) {
			ordersOfM -= 3;
			symax = ymax * pow(10.,ordersOfM);
		}

		if(ordersOfM!=0) {
			// correct display y axis values
			for (uInt i = 0; i < z_yval.size(); i++) {
				z_yval(i) *= pow(10.,ordersOfM);
			}
			if ( z_eval.size() > 0 ) {
				for (uInt i = 0; i < z_eval.size(); i++) {
					z_eval(i) *= pow(10.,ordersOfM);
				}
			}

		}

		// store the scaling factor
		ordersOfM_=ordersOfM;

		if(ordersOfM!=0) {
			// correct unit string
			if( yUnit.startsWith("(") || yUnit.startsWith("[") || yUnit.startsWith("\"") ) {
				// express factor as number
				ostringstream oss;
				oss << -ordersOfM;
				yUnitPrefix = "10E"+QString(oss.str().c_str())+" ";
			} else {
				// express factor as character
				switch(-ordersOfM) { // note the sign!
				case -12:
					yUnitPrefix = "p";
					break;
				case -9:
					yUnitPrefix = "n";
					break;
				case -6:
					yUnitPrefix = "u";
					break;
				case -3:
					yUnitPrefix = "m";
					break;
				case 3:
					yUnitPrefix = "k";
					break;
				case 6:
					yUnitPrefix = "M";
					break;
				case 9:
					yUnitPrefix = "G";
					break;
				default:
					ostringstream oss;
					oss << -ordersOfM;
					yUnitPrefix = "10E"+QString(oss.str().c_str())+" ";
					break;
				}
			}

		} else { // no correction
			yUnitPrefix = "";
		}
		setPixelCanvasYUnits( yUnitPrefix, yUnit );
		return ordersOfM;
	}

	void QtProfile::setDisplayYUnits( const QString& unitStr ) {
		//Called with the left axis combo box changes its units.
		QString displayUnit = unitStr;
		//Right now optical units are not being supported as far as changing
		//them on the y-axis.
		bool convertableUnits = ConverterIntensity::isSupportedUnits( yUnit );
		if ( !convertableUnits ) {
			displayUnit = "";
		} else {
			postConversionWarning( displayUnit );
		}

		QString canvasUnit = yUnitPrefix + displayUnit;
		if ( displayUnit == ConverterIntensity::FRACTION_OF_PEAK ) {
			canvasUnit = displayUnit;
		}
		pixelCanvas->setDisplayYUnits( canvasUnit );
		specFitSettingsWidget->setDisplayYUnits( canvasUnit );
	}

	void QtProfile::setPixelCanvasYUnits( const QString& yUnitPrefix, const QString& yUnit ) {
		bool unitsAcceptable = ConverterIntensity::isSupportedUnits( yUnit );
		setYUnitConversionVisibility( unitsAcceptable );
		if ( unitsAcceptable ) {
			yAxisCombo->setCurrentIndex( 0 );
		}
		specFitSettingsWidget->setImageYUnits( yUnitPrefix + yUnit );
		pixelCanvas->setDisplayYUnits( "" );
		pixelCanvas->setImageYUnits( yUnitPrefix + yUnit );
	}

	void QtProfile::setYUnitConversionVisibility( bool visible ) {
		yAxisCombo->setVisible( visible );
		leftLabel->setVisible( visible );
	}


	void QtProfile::addImageAnalysisGraph( const Vector<double> &wxv,
			const Vector<double> &wyv, Int ordersOfM ) {
		bool ok = true;

		if ( over != NULL ) {
			QListIterator<OverplotAnalysis> i(*over);
			while (i.hasNext() && stateMProf) {
				OverplotAnalysis overplot = i.next();
				QString ky = overplot.first;
				ImageAnalysis* ana = overplot.second;
				int tabularIndex = getFreqProfileTabularIndex( ana );
				Vector<Float> xval(100);
				Vector<Float> yval(100);
				String shape = getRegionShape();

				switch (itsPlotType) {
				case QtProfile::PMEAN:
					ok=ana->getFreqProfile( wxv, wyv, xval, yval,
					                        WORLD_COORDINATES, coordinateType, 0, tabularIndex, 0, xaxisUnit, spcRefFrame,
					                        (Int)QtProfile::MEAN, 0, "", -1, shape);
					break;
				case QtProfile::PMEDIAN:
					ok=ana->getFreqProfile( wxv, wyv, xval, yval,
					                        WORLD_COORDINATES, coordinateType, 0, tabularIndex, 0, xaxisUnit, spcRefFrame,
					                        (Int)QtProfile::MEDIAN, 0, "", -1, shape);
					break;
				case QtProfile::PSUM:
					ok=ana->getFreqProfile( wxv, wyv, xval, yval,
					                        WORLD_COORDINATES, coordinateType, 0, tabularIndex, 0, xaxisUnit, spcRefFrame,
					                        (Int)QtProfile::PSUM, 0, "", -1, shape);
					break;
				case QtProfile::PFLUX:
					ok=getFrequencyProfileWrapper( ana, wxv, wyv, xval, yval,
					                               WORLD_COORDINATES, coordinateType, 0, tabularIndex, 0, xaxisUnit, spcRefFrame,
					                               (Int)QtProfile::PFLUX, 0, "", shape);
					break;

				default:
					ok=ana->getFreqProfile( wxv, wyv, xval, yval,
					                        WORLD_COORDINATES, coordinateType, 0, tabularIndex, 0, xaxisUnit, spcRefFrame,
					                        (Int)QtProfile::MEAN, 0, "", -1, shape);
					break;
				}

				if (ok) {
					//Apply smoothing to the vectors we get back.
					if ( smoothWidget != NULL ){
						yval = smoothWidget->applySmoothing( yval );
					}
					if(ordersOfM!=0) {
						// correct display y axis values
						for (uInt i = 0; i < yval.size(); i++) {
							yval(i) *= pow(10.,ordersOfM);
						}
					}
					Vector<Float> xRel(yval.size());
					Vector<Float> yRel(yval.size());
					Int count = 0;
					if (stateRel) {
						ky = ky+ "_rel.";
						for (uInt i = 0; i < yval.size(); i++) {
							uInt k = z_yval.size() - 1;
							//cout << xval(i) << " - " << yval(i) << endl;
							//cout << z_xval(0) << " + " << z_xval(k) << endl;
							if (coordinateType.contains("elocity")) {
								if (xval(i) < z_xval(0) && xval(i) >= z_xval(k)) {
									for (uInt j = 0; j < k; j++) {
										//cout << z_xval(j) << " + "
										//     << z_yval(j) << endl;
										if (xval(i) <= z_xval(j) &&
										        xval(i) > z_xval(j + 1)) {
											float s = z_xval(j + 1) - z_xval(j);
											if (s != 0) {
												xRel(count) = xval(i);
												yRel(count)= yval(i) -
												             (z_yval(j) + (xval(i) - z_xval(j)) / s *
												              (z_yval(j + 1) - z_yval(j)));
												count++;
												//yval(i) -= (z_yval(j) + (xval(i) - z_xval(j)) / s *
												//           (z_yval(j + 1) - z_yval(j)));

											}
											break;
										}
									}
								}
							} else {
								if (xval(i) >= z_xval(0) && xval(i) < z_xval(k)) {
									for (uInt j = 0; j < k; j++) {
										if (xval(i) >= z_xval(j) && xval(i) < z_xval(j + 1)) {
											float s = z_xval(j + 1) - z_xval(j);
											if (s != 0) {
												xRel(count) = xval(i);
												yRel(count)= yval(i) -
												             (z_yval(j) + (xval(i) - z_xval(j)) / s *
												              (z_yval(j + 1) - z_yval(j)));
												count++;
												//yval(i) -= (z_yval(j) + (xval(i) - z_xval(j)) / s *
												//           (z_yval(j + 1) - z_yval(j)));
											}
											break;
										}
									}
								}
							}
						}
						xRel.resize(count, True);
						yRel.resize(count, True);
						addCanvasMainCurve( xRel, yRel, ky );
					} else {
						addCanvasMainCurve( xval, yval, ky );
					}
				}
			}

		}
	}

	double QtProfile::getUnitsPerChannel( ImageAnalysis* analysis, bool* ok, const QString& matchUnits ){
		//We just care about the min and the max so we use a vector
		//of size 2.
		Vector<Float> xval(2);
		Vector<Float> yval(2);
		double unitPerChannel = -1;
		//Note that we don't care if it is mean or something else because
		//we are just interested in x-values here.
		*ok = false;
		if ( z_xval.size()> 0 ){
			QRegExp unitsMatch( matchUnits+"*");
			unitsMatch.setPatternSyntax( QRegExp::Wildcard);
			int unitIndex = xUnitsList.indexOf( unitsMatch );
			QString text = xUnitsList[unitIndex];
			String xUnits = String(text.toStdString());
			String coordinateType;
			String cTypeUnit;

			getcoordTypeUnit(xUnits, coordinateType, cTypeUnit);
			int tabularIndex = getFreqProfileTabularIndex( analysis );
			//We don't have to worry about smoothing here because we are only
			//using the x-values.
			*ok=analysis->getFreqProfile( lastWX, lastWY, xval, yval,
					WORLD_COORDINATES, coordinateType, 0, tabularIndex, 0, cTypeUnit, spcRefFrame,
							(Int)QtProfile::MEAN, 0, "", -1, getRegionShape());
			if ( *ok ){
				unitPerChannel = qAbs(xval[0] - xval[1]) / z_xval.size();
			}
		}
		return unitPerChannel;
	}

	String QtProfile::getRegionShape(){
		String shapeStr( " ");
		std::map<int,spectra_info>::iterator iter = spectra_info_map.find( current_region_id );
		if ( iter != spectra_info_map.end() ){
			QString shapeString = spectra_info_map[current_region_id].shape();
			shapeStr = shapeString.toStdString();
		}
		return shapeStr;
	}

	bool QtProfile::isFrequencyMatch(){
		return isXUnitsMatch( FREQUENCY );
	}

	bool QtProfile::isVelocityMatch(){
		return isXUnitsMatch( RADIO_VELOCITY );
	}

	bool QtProfile::isXUnitsMatch(const QString& matchUnits){
		//Calculate the units per channel for each image
		//and see if it matches within a given error percentage.
		bool unitsMatch = true;

		//Get the units per channel for the main image
		bool unitsAvailable = false;
		double unitsPerChannel = getUnitsPerChannel( analysis, &unitsAvailable, matchUnits );
		if ( !unitsAvailable ){
			unitsMatch = false;
		}
		else {

			//Compare that with the frequency per channel of the overplots
			if ( over != NULL){
				const float ERROR_TOL = 0.00001f;
				QListIterator<OverplotAnalysis> i(*over);
				while (i.hasNext() && stateMProf) {
					OverplotAnalysis overplot = i.next();
					QString ky = overplot.first;
					ImageAnalysis* ana = overplot.second;
					double overUnitsPerChannel = getUnitsPerChannel( ana, &unitsAvailable, matchUnits );
					if ( !unitsAvailable ){
						unitsMatch = false;
						break;
					}
					else if ( qAbs( overUnitsPerChannel - unitsPerChannel) > ERROR_TOL ){
						unitsMatch = false;
						break;
					}
				}
			}
		}
		return unitsMatch;
	}


	void QtProfile::restrictTopAxisOptions( bool restrictOptions, const QString& bottomUnits,
			bool allowFrequency, bool allowVelocity ){
		topAxisCType->clear();

		for ( int i = 0; i < xUnitsList.size(); i++ ){
			bool channelVisible = false;
			bool freqVisible = false;
			bool velocityVisible = false;
			bool waveVisible = false;
			if ( restrictOptions ){
				if ( xUnitsList[i].indexOf( OPTICAL ) >= 0 || xUnitsList[i].indexOf( AIR) >= 0 ){
					continue;
				}

				bool bottomVelocity = isVelocityUnit( bottomUnits );
				bool bottomWavelength = isWavelengthUnit( bottomUnits );
				bool bottomFrequency = isFrequencyUnit( bottomUnits );

				//Allow a top axis of frequency if the bottom axis is already displaying
				//frequency or the images are compatible in channel/frequency.
				freqVisible = isFrequencyUnit( xUnitsList[i])  && (allowFrequency || bottomFrequency);
				waveVisible = isWavelengthUnit(xUnitsList[i])  && (allowFrequency || bottomWavelength);
				velocityVisible = isVelocityUnit( xUnitsList[i]) && (allowVelocity || bottomVelocity );

				//We enable channels if the units on the bottom axis are compatible in
				//channels.
				int channelIndex = xUnitsList[i].indexOf( CHANNEL );
				if ( channelIndex >= 0 ){
					if ( bottomVelocity && allowVelocity ){
						channelVisible = true;
					}
					else if ( (bottomWavelength || bottomFrequency ) && allowFrequency ){
						channelVisible = true;
					}
				}
			}
			if ( !restrictOptions ||
				channelVisible || freqVisible || velocityVisible || waveVisible ){
				//One last check, make sure the bottom axis already contains the unit.
				//In the case of using a tabular spectral index, the rest frequency may not
				//be available.  In such a case, we can not make valid velocity conversions
				//so velocity units are removed from the bottom axis, and should not be supported
				//on the top axis either.
				int unitIndex = bottomAxisCType->findText( xUnitsList[i]);
				if ( unitIndex >= 0 ){
					topAxisCType->addItem( xUnitsList[i]);
				}
			}
		}
	}

	bool QtProfile::isFrequencyUnit( const QString& unit ) const {
		bool frequencyUnit = false;
		int frequencyIndex = unit.indexOf( FREQUENCY );
		if ( frequencyIndex >= 0 ){
			frequencyUnit = true;
		}
		return frequencyUnit;
	}

	bool QtProfile::isVelocityUnit( const QString& unit ) const {
			bool velocityUnit = false;
			int velocityIndex = unit.indexOf( RADIO_VELOCITY );
			if ( velocityIndex >= 0 ){
				velocityUnit = true;
			}
			return velocityUnit;
		}

	bool QtProfile::isWavelengthUnit( const QString& unit ) const {
			bool waveUnit = false;
			int waveIndex = unit.indexOf( "wavelength" );
			if ( waveIndex == 0 ){
				waveUnit = true;
			}
			return waveUnit;
		}


	void QtProfile::adjustTopAxisSettings() {
		int mainCurveCount = pixelCanvas->getLineCount();
		//If the user does not want us to show a top axis
		//we don't.
		QString bottomAxisUnits = bottomAxisCType->currentText();

		restrictTopAxisOptions( false, bottomAxisUnits );
		if ( !showTopAxis ){
			topAxisCType->setEnabled( false );
		}

		//If we have more than one curve, we show the top axis
		//only if there is an image whose domain encompasses the domains
		//of all the other images using the frequency units on the bottom
		//axis.  The is because the canvas plots the bottom axis using the
		//min and max of all the images whereas the top axis just uses a
		//the min/max of a single image.  We also need to check that the
		//frequency units/channel are the same on all images.
		else if ( mainCurveCount > 1 ) {
			int opticalIndex = bottomAxisUnits.indexOf( OPTICAL );
			int airIndex = bottomAxisUnits.indexOf( AIR );
			if ( opticalIndex >= 0 || airIndex >= 0 ){
				topAxisCType->setEnabled( false );
			}
			else {
				bool frequencyMatch = isFrequencyMatch();
				bool velocityMatch = isVelocityMatch();
				bool enableTopAxis = frequencyMatch || velocityMatch;
				topAxisCType->setEnabled( enableTopAxis );
				if ( enableTopAxis ){
					restrictTopAxisOptions( true, bottomAxisUnits, frequencyMatch, velocityMatch );
				}
				pixelCanvas->setTopAxisCompatible( enableTopAxis );
			}
		//If there is only one curve, we show the top axis assuming
		//nothing else applies.
		} else {
			topAxisCType-> setEnabled( true );
		}
		pixelCanvas->setShowTopAxis( topAxisCType->isEnabled());
	}

	void QtProfile::addCanvasMainCurve( const Vector<Float>& xVals,
			const Vector<Float>& yVals, const QString& label ) {
		specFitSettingsWidget->addCurveName( label );
		pixelCanvas->addPolyLine(xVals, yVals, label );
		adjustTopAxisSettings();
	}

	void QtProfile::storeCoordinates( const Vector<double> pxv, const Vector<double> pyv,
	                                  const Vector<double> wxv, const Vector<double> wyv ) {
		lastWX.assign(wxv);
		lastWY.assign(wyv);
		lastPX.assign(pxv);
		lastPY.assign(pyv);
	}


//Methods from the SpecFitMonitor interface that this class
//implements.
	void QtProfile::postStatus( String status ) {
		profileStatus->showMessage(QString(status.c_str()));
	}
	Vector<Float> QtProfile::getXValues() const {
		return z_xval;
	}
	Vector<Float> QtProfile::getYValues() const {
		return z_yval;
	}
	Vector<Float> QtProfile::getZValues() const {
		return z_eval;
	}

	QString QtProfile::getYUnit() const {
		return yUnit;
	}

	QString QtProfile::getYUnitPrefix() const {
		return yUnitPrefix;
	}
	String QtProfile::getXAxisUnit() const {
		return xaxisUnit;
	}

	QString QtProfile::getFileName() const {
		return fileName;
	}

	QString QtProfile::getImagePath() const {
		return imagePath;
	}

	std::tr1::shared_ptr<const ImageInterface<Float> > QtProfile::getImage( const QString& imageName ) const {
		//First look for a specific image with the name
		if ( imageName.length() > 0 && over ) {
			QListIterator<OverplotAnalysis> i( *over );
			while (i.hasNext()) {
				OverplotAnalysis overplot = i.next();
				QString ky = overplot.first;
				if ( ky == imageName ) {
					ImageAnalysis* analysis = overplot.second;
					std::tr1::shared_ptr<const ImageInterface<Float> > imageInterface(analysis->getImage());
					return imageInterface;
				}
			}
		}
		return image;
	}

	const void QtProfile::getPixelBounds( Vector<double>& pixelX, Vector<double>& pixelY) const {
		pixelX = lastPX;
		pixelY = lastPY;
	}

	void QtProfile::persist( const QString& key, const QString& value ) {
		rc.put( "viewer." + rcid() + key.toStdString(), value.toStdString());
	}

	QString QtProfile::read( const QString & key ) const {
		QString valueStr( rc.get("viewer." + rcid() + key.toStdString()).c_str());
		return valueStr;
	}

	void QtProfile::imageCollapsed(String path, String dataType, String displayType, Bool autoRegister,
			Bool tmpData, std::tr1::shared_ptr<ImageInterface<Float> > img) {
		emit showCollapsedImg(path, dataType, displayType, autoRegister, tmpData, img);
	}

	void QtProfile::pixelsChanged( int pixX, int pixY ) {
		specFitSettingsWidget->pixelsChanged( pixX, pixY );
	}

	void QtProfile::processTrackRecord( const String& dataName, const String& positionInfo ) {
		if ( image != NULL ){
			QString imageName( image->name(true).c_str());
			QString dataNameStr( dataName.c_str());
			if ( dataNameStr.indexOf( imageName ) >= 0 ) {
				QString posStr( positionInfo.c_str());
				int pixelIndex = posStr.indexOf("Pixel:");
				posStr = posStr.mid( pixelIndex );
				QStringList parts = posStr.split( " ");
				if ( parts.size() > 2 ) {
					QString pixelX = parts[1];
					QString pixelY = parts[2];
					bool validX = false;
					int pixX = pixelX.toInt(&validX);
					bool validY = false;
					int pixY = pixelY.toInt( &validY);
					if ( validX && validY ) {
						pixelsChanged( pixX, pixY );
					}
				}
			}
		}
	}


	void QtProfile::curveColorPreferences() {
		colorSummaryWidget->show();
	}

	void QtProfile::legendPreferences() {
		legendPreferencesDialog->show();
	}

	void QtProfile::frameChanged( int frameNumber ) {
		if ( frameNumber >= 0 ) {
			frameIndex = frameNumber;
		}
		if ( 0 <= frameIndex && frameIndex < static_cast<int>(z_xval.size()) ) {
			pixelCanvas->setFrameMarker( z_xval[frameIndex]);
		}
	}

	void QtProfile::setPosition( const QList<double>& xValues, const QList<double>& yValues ) {
		emit adjustPosition( xValues[0], yValues[0], xValues[1], yValues[1]);
	}

	void QtProfile::setPurpose( ProfileTaskMonitor::PURPOSE purpose ) {
		const QString TITLE_BASE = "Spectral Profile";
		if ( purpose == MOMENTS_COLLAPSE ) {
			//Hide the function tabs and show only the moments/collapse functionality
			this->purposeStackedLayout->setCurrentIndex( 1 );
			setWindowTitle( TITLE_BASE+": Collapse/Moments - "+fileName);
			pixelCanvas->changeTaskMode( 0 );
		} else if ( purpose == SPECTROSCOPY ) {
			//Show the function tabs and hide moments/collapse functionality
			this->purposeStackedLayout->setCurrentIndex( 0 );
			setWindowTitle( TITLE_BASE+" - "+fileName);
			int tabIndex = functionTabs->currentIndex();
			pixelCanvas->changeTaskMode( tabIndex );
		}

	}

	void QtProfile::initializeSolidAngle() const {
		//Get the major and minor axis beam widths.
		ImageInfo information = this->image->imageInfo();
		GaussianBeam beam;
		bool multipleBeams = information.hasMultipleBeams();
		if ( !multipleBeams ) {
			beam = information.restoringBeam();
		} else {
			beam = information.restoringBeam( 0, -1 );
		}

		Quantity majorQuantity = beam.getMajor();
		Quantity minorQuantity = beam.getMinor();
		double arcsecArea = beam.getArea( "arcsec2");
		ConverterIntensity::setBeamArea( arcsecArea );

		//Calculate: PI * (half power width)^2 * ARCSEC^2_SR_CONVERSIONFACTOR / 4 ln 2
		double halfPowerWidthSquared = (majorQuantity.getValue() * minorQuantity.getValue() );
		const double ARCSEC2_SR_CONVERSION = 0.0000000000235045;
		const double PI = 3.1415926535;
		double solidAngle = PI * halfPowerWidthSquared * ARCSEC2_SR_CONVERSION/ (4 * log( 2 ));
		if ( solidAngle > 0 ) {
			ConverterIntensity::setSolidAngle( solidAngle );
			//Add Kelvin conversion if it is not already there.
			int yAxisUnitCount = yAxisCombo->count();
			QString lastItem = yAxisCombo->itemText( yAxisUnitCount - 1 );
			if ( lastItem != ConverterIntensity::KELVIN ) {
				yAxisCombo->addItem( ConverterIntensity::KELVIN );
				yAxisCombo->setCurrentIndex( 0 );
			}
		} else {
			//No Kelvin conversions so remove it as an option
			int yAxisUnitCount = yAxisCombo->count();
			QString lastItem = yAxisCombo->itemText( yAxisUnitCount - 1 );
			if ( lastItem == ConverterIntensity::KELVIN ) {
				yAxisCombo->removeItem( yAxisUnitCount - 1 );
			}
		}
	}

	void QtProfile::postConversionWarning( QString unitStr ) {
		if ( unitStr == ConverterIntensity::KELVIN ) {
			if ( z_xval.size() > 0 ) {
				//Calculate h nu/ k T where h is Plank's constant, k is boltzman's constant
				//nu is observing frequency and T is the maximum temperature.
				pair<double,double> freqTempPair = getMaximumTemperature( );
				double proportion = ( QC::h.getValue() * freqTempPair.first ) / ( QC::k.getValue() * freqTempPair.second );
				const float RATIO_THRESHOLD = 0.1;
				if ( proportion > RATIO_THRESHOLD ) {
					//Calculate the Kelvin threshold 5 h nu / kB
					double kelvinThreshold = ( 5 * QC::h.getValue() * freqTempPair.first ) / QC::k.getValue();
					QString msg( "Warning: Brightness was calculated using the Raleigh Jeans approximation.  For source temperatures < " );
					msg.append( QString::number(kelvinThreshold));
					msg.append( " K errors are >~10%");
					*itsLog << LogIO::WARN << msg.toStdString() << LogIO::POST;
					profileStatus->showMessage( msg );
				}
			}
		}
	}

	pair<double,double> QtProfile::getMaximumTemperature() {
		//Convert xValues to Hz
		int zxCount = z_xval.size();
		Vector<float> xValues(zxCount);
		for ( int i = 0; i < zxCount; i++ ) {
			xValues[i] = z_xval[i];
		}
		const QString HERTZ = "Hz";
		QString xAxisUnit(xaxisUnit.c_str());
		if ( xAxisUnit != HERTZ ) {
			Converter* converter = Converter::getConverter( xAxisUnit, HERTZ );
			for ( int i = 0; i < static_cast<int>(xValues.size()); i++ ) {
				xValues[i] = converter->convert( xValues[i] );
			}
			delete converter;
		}

		//Convert the y values
		Vector<float> yValues = getYValues();
		ConverterIntensity::convert( yValues, xValues, yUnitPrefix + yUnit, ConverterIntensity::KELVIN, 0, "" );
		int maxIndex = 0;
		double max = numeric_limits<double>::min();
		for( int i = 0; i < static_cast<int>(yValues.size()); i++ ) {
			if ( yValues[i] > max ) {
				max = yValues[i];
				maxIndex = i;
			}
		}
		pair<double,double> results( xValues[maxIndex], max);
		return results;
	}

	void QtProfile::clearPaletteModes() {
		actionRangeXSelection->setChecked( false );
		actionChannelPositioning->setChecked( false );
		actionAnnotationText->setChecked( false );
	}

	void QtProfile::toggleAction( QAction* action ) {
		if ( ! action->isChecked() ) {
			clearPaletteModes();
			action->setChecked( true );
		}
	}

	void QtProfile::togglePalette( int mode ) {
		if ( mode == CanvasMode::MODE_ANNOTATION ) {
			toggleAction( actionAnnotationText );
		} else {
			qDebug() << "QtProfile unsupported toggle mode: "<< mode;
		}
	}

}
