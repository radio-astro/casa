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


#include "FindSourcesDialog.qo.h"
#include <QDir>
#include <QDebug>
#include <QTime>
#include <QMessageBox>
#include <QFileSystemModel>
#include <imageanalysis/Regions/CasacRegionManager.h>
#include <imageanalysis/ImageAnalysis/ImageAnalysis.h>
#include <tables/Tables.h>
#include <casa/Quanta/MVAngle.h>
#include <display/Display/Options.h>
#include <display/Fit/RegionBox.h>
#include <display/Fit/ColorComboDelegate.h>
#include <imageanalysis/ImageAnalysis/ImageStatsCalculator.h>
#include <limits>

namespace casa {

	const QStringList FindSourcesDialog::colorNames = QStringList()<<"yellow"<<"white"<<"red"<<
	        "green"<<"blue"<<"cyan"<<"magenta"<<"black";

	FindSourcesDialog::FindSourcesDialog(QWidget *parent,
	                                     bool displayModeFunctionality)
		: QDialog(parent),  DEFAULT_KEY(-1),
		  SKY_CATALOG("skycatalog"), pixelRangeDialog(this) {

		ui.setupUi(this);
		currentRegionId = DEFAULT_KEY;
		overlayColorName = "white";
		pixelRangeDialog.setLineMode( true );
		pixelRangeDialog.setTitle( "Intensity Minimum");

		imageMode = true;
		ui.imageRadioButton->setVisible( displayModeFunctionality );
		ui.regionRadioButton->setVisible( displayModeFunctionality );
		ui.colorLabel->setVisible( displayModeFunctionality );
		ui.colorCombo->setVisible( displayModeFunctionality );


		//Whether or not to display the sources on the viewer.
		connect( ui.displayViewerCheckBox, SIGNAL(stateChanged(int)), this, SLOT(viewerDisplayChanged()));


		if ( displayModeFunctionality ) {
			QButtonGroup* buttonGroup = new QButtonGroup( this );
			buttonGroup->addButton( ui.imageRadioButton );
			buttonGroup->addButton( ui.regionRadioButton );
			ui.imageRadioButton->setChecked( imageMode );
			connect( ui.imageRadioButton, SIGNAL(toggled(bool)),this, SLOT(setImageMode(bool)));

			colorDelegate = new ColorComboDelegate( this );
			ui.colorCombo->setItemDelegate(  colorDelegate );
			colorDelegate->setSupportedColors( FindSourcesDialog::colorNames );
			for ( int i = 0; i < colorNames.size(); i++ ) {
				ui.colorCombo->addItem(colorNames[i]);
			}
			connect( ui.colorCombo, SIGNAL(currentIndexChanged(const QString&)),
			         this, SLOT(setOverlayColor(const QString&)));
			setOverlayColor( colorNames[0] );

		}

		QTime time = QTime::currentTime();
		qsrand((uInt)time.msec());

		QDoubleValidator* validator = new QDoubleValidator( std::numeric_limits<double>::min(),
		        std::numeric_limits<double>::max(), 7, this );
		ui.cutoffLineEdit->setValidator( validator );
		ui.cutoffLineEdit->setText( QString::number(0.01f));
		QButtonGroup* cutOffGroup = new QButtonGroup( this );
		cutOffGroup->addButton( ui.fractionOfPeakRadio );
		cutOffGroup->addButton( ui.noiseRadio );
		bool fractionOfPeakMode = true;
		ui.fractionOfPeakRadio->setChecked( fractionOfPeakMode );
		cutoffModeChanged(!fractionOfPeakMode );
		pixelRangeDialog.setWindowTitle( "Find Sources Intensity Cut-Off" );
		//pixelRangeDialog.setRangeMaxEnabled( false );
		connect( &pixelRangeDialog, SIGNAL(accepted()), this, SLOT(pixelRangeChanged()));
		connect( ui.noiseRadio, SIGNAL(toggled(bool)), this, SLOT(cutoffModeChanged(bool)));

		QStringList tableHeaders =(QStringList()<< "ID"<<"RA" << "DEC" << "Flux" <<
		                           "Major Axis"<<"Minor Axis"<<"Angle"/*<<"Fixed"*/);
		ui.sourceTable->setColumnCount( tableHeaders.size());
		ui.sourceTable->setHorizontalHeaderLabels( tableHeaders );
		ui.sourceTable->setSelectionBehavior(QAbstractItemView::SelectRows);
		ui.sourceTable->setSelectionMode( QAbstractItemView::SingleSelection );
		ui.sourceTable->setSortingEnabled( true );
		ui.sourceTable->verticalHeader()->hide();
		setSourceResultsVisible( false );

		initializeFileManagement();

		connect( ui.graphicalNoiseButton, SIGNAL(clicked()), this, SLOT(showPixelRange()));
		connect( ui.findSourcesButton, SIGNAL(clicked()), this, SLOT(findSources()));
		connect( ui.deleteSourceButton, SIGNAL(clicked()), this, SLOT( deleteSelectedSource()));
		connect( ui.cancelButton, SIGNAL(clicked()), this, SLOT(canceledFindSources()));
		connect( ui.saveButton, SIGNAL(clicked()), this, SLOT(saveEstimateFile()));
	}

	void FindSourcesDialog::clearImage(){
		image.reset();
	}

	void FindSourcesDialog::viewerDisplayChanged() {
		if ( ui.displayViewerCheckBox->isChecked() ) {
			if ( skyPath.trimmed().length() > 0 ) {
				String skyPathStr( skyPath.toStdString());
				emit showOverlay( skyPathStr, overlayColorName );
			}
		} else {
			emit removeOverlay( getRemoveOverlayPath().toStdString());
		}
	}


//------------------------------------------------------------------
//              Cut-Off
//------------------------------------------------------------------

	void FindSourcesDialog::cutoffModeChanged( bool noise ) {
		ui.graphicalNoiseButton->setEnabled( noise );
	}

	void FindSourcesDialog::pixelRangeChanged() {
		pair<double,double> range = pixelRangeDialog.getInterval();
		QString startRange = QString::number( range.first );

		ui.cutoffLineEdit->setText( startRange );
	}

	void FindSourcesDialog::showPixelRange() {
		pixelRangeDialog.setImage( image );
		pixelRangeDialog.show();
	}

	double FindSourcesDialog::populateCutOff(bool* valid) const {
		QString cutoffStr = ui.cutoffLineEdit->text();
		double value = cutoffStr.toDouble( valid);
		if ( *valid ) {
			if ( ui.fractionOfPeakRadio->isChecked() ) {
				if ( value < 0 || value > 1) {
					*valid = false;
				}
			} else {
				std::vector<float> pixelValues = pixelRangeDialog.getXValues();
				if ( pixelValues.size( ) > 0 ) {
					// when array is empty dereferenceing iterator == dereference of null pointer
					std::vector<float>::iterator maxValuePos = std::max_element( pixelValues.begin(), pixelValues.end());
					std::vector<float>::iterator minValuePos = std::min_element( pixelValues.begin(), pixelValues.end());
					if ( value < *minValuePos ) {
						value = *minValuePos;
					} else if ( value > *maxValuePos ) {
						value = *maxValuePos;
					}
					double span = *maxValuePos - *minValuePos;
					double dataSpan = value - *minValuePos;
					value = dataSpan / span;
				} else { return std::numeric_limits<double>::quiet_NaN( ); }
			}
		}
		return value;
	}

//----------------------------------------------------------------------
//                      Properties
//----------------------------------------------------------------------

	void FindSourcesDialog::canceledFindSources() {
		close();
	}

	void FindSourcesDialog::setImageMode( bool enabled ) {
		if ( imageMode != enabled ) {
			imageMode = enabled;
			resetCurrentId( -1 );
			populatePixelBox();
		}
	}

	void FindSourcesDialog::setOverlayColor(const QString& colorName) {
		if ( colorName != overlayColorName ) {
			overlayColorName = colorName;
			if ( skyPath.length() > 0 ) {
				String removeName = getRemoveOverlayPath().toStdString();
				emit removeOverlay( removeName );
				if ( ui.displayViewerCheckBox->isChecked()) {
					emit showOverlay( skyPath.toStdString(), overlayColorName );
				}
			}
		}
	}

	String FindSourcesDialog::getPixelBox() const {
		return pixelBox;
	}

	void FindSourcesDialog::resetCurrentId( int suggestedId ) {
		if ( !imageMode && regions.contains( suggestedId ) && suggestedId >= 0) {
			currentRegionId  = suggestedId;
		} else if ( !imageMode && !regions.isEmpty()) {
			QList<int> keys = regions.keys();
			currentRegionId = keys[0];
		} else {
			currentRegionId = DEFAULT_KEY;
		}
	}


//-------------------------------------------------------------------
//                  Editing the Source List
//------------------------------------------------------------------

	void FindSourcesDialog::deleteSelectedSource() {
		QList<QTableWidgetItem*> selectedItems = ui.sourceTable->selectedItems();
		if ( selectedItems.isEmpty()) {
			QMessageBox::warning( this, "No Source", "Please select a source to delete.");
		} else {
			clearSkyOverlay();

			//Decide which ones we are going to remove
			int removeCount = selectedItems.size();
			QVector<int> removeIndices;
			int colCount = ui.sourceTable->columnCount();
			for ( int i = ID_COL; i < removeCount; i=i+colCount ) {
				QString idText = selectedItems[i]->text();
				bool validInt;
				int itemId = idText.toInt(&validInt);
				if ( validInt ) {
					removeIndices.append( itemId );
				} else {
					qDebug() << "FindSourcesDialog -Selected item: "<<idText<<" was not a valid ID";
				}
			}
			skyList.remove( removeIndices );
			resetSkyOverlay();
		}
	}

//--------------------------------------------------------------
//            View Table Operations
//--------------------------------------------------------------

	void FindSourcesDialog::resetSourceView() {
		//Write the data into the columns.
		int rowCount = skyList.getSize();
		ui.sourceTable->setRowCount( rowCount );

		bool sourceResultsVisible = false;
		if ( rowCount > 0 ) {
			sourceResultsVisible = true;
		}
		setSourceResultsVisible( sourceResultsVisible );
		ui.sourceTable->setSortingEnabled( false );
		for ( int i=0; i<rowCount; i++ ) {

			//Type
			setTableValue( i, ID_COL, String::toString(i));

			//RA and DEC
			String raStr = skyList.getRA( i );
			String decStr = skyList.getDEC( i );
			setTableValue( i, RA_COL, raStr );
			setTableValue( i, DEC_COL, decStr );

			//Flux
			Quantity fluxQuantity = skyList.getFlux(i);
			double fluxVal = fluxQuantity.getValue();
			String fluxStr = String::toString( fluxVal );
			Unit fluxUnit = fluxQuantity.getUnit();
			fluxStr = fluxStr + " "+fluxUnit.getName();
			setTableValue( i, FLUX_COL, fluxStr );

			//Major & minor axis and position angle
			Quantity majorAxisQuantity = skyList.getMajorAxis( i );
			String majorAxisStr = String::toString(majorAxisQuantity.getValue());
			Unit majorUnit = majorAxisQuantity.getUnit();
			majorAxisStr = majorAxisStr + " " + majorUnit.getName();
			setTableValue( i, MAJOR_AXIS_COL, majorAxisStr );
			Quantity minorAxisQuantity = skyList.getMinorAxis( i );
			String minorAxisStr = String::toString( minorAxisQuantity.getValue());
			Unit minorUnit= minorAxisQuantity.getUnit();
			minorAxisStr = minorAxisStr + " " + minorUnit.getName();
			setTableValue( i, MINOR_AXIS_COL, minorAxisStr );
			Quantity angleQuantity = skyList.getAngle( i );
			String angleStr = String::toString( angleQuantity.getValue());
			Unit angleUnit = angleQuantity.getUnit();
			angleStr = angleStr + " "+angleUnit.getName();
			setTableValue( i, ANGLE_COL, angleStr );
		}
		ui.sourceTable->setSortingEnabled( true );
		ui.sourceTable->resizeColumnsToContents();
	}

	void FindSourcesDialog::setTableValue(int row, int col, const String& val ) {
		QTableWidgetItem* tableItem = new QTableWidgetItem();
		tableItem->setFlags( tableItem->flags() ^ Qt::ItemIsEditable );
		QString valueStr( val.c_str() );
		bool validNum;
		float numVal = valueStr.toFloat(&validNum);
		if ( !validNum ) {
			tableItem -> setText( valueStr );
		} else {
			tableItem ->setData(Qt::DisplayRole, numVal );
		}
		ui.sourceTable->setItem( row, col, tableItem );
	}

	void FindSourcesDialog::setSourceResultsVisible( bool visible ) {
		QLayout* dLayout = layout();
		QVBoxLayout* dialogLayout = dynamic_cast<QVBoxLayout*> (dLayout);
		int holderIndex = dialogLayout->indexOf( ui.sourceResultHolder );
		if ( !visible ) {
			if ( holderIndex >= 0 ) {
				resultIndex = holderIndex;
				dialogLayout->removeWidget( ui.sourceResultHolder );
				ui.sourceResultHolder->setParent( NULL );
				setMaximumSize(600,300);
			}
		} else {
			if ( holderIndex < 0 ) {
				dialogLayout->insertWidget( resultIndex, ui.sourceResultHolder );
				setMaximumSize( 3000, 3000 );
			}
		}
		ui.saveButton->setVisible( visible );
	}


//----------------------------------------------------------------
//                    Finding Sources
//----------------------------------------------------------------

	void FindSourcesDialog::clearSkyOverlay() {
		//Clean up if we have old results lying around.
		if ( skyPath.length() > 0 ) {
			String removePath = getRemoveOverlayPath().toStdString();
			//Tell the viewer to remove the previous overlay
			emit removeOverlay( removePath );
			skyPath.clear();
		}
	}

	QString FindSourcesDialog::getRemoveOverlayPath() const {
		//Just get the file name
		int dirIndex = skyPath.lastIndexOf( QDir::separator() );
		QString fileName = skyPath;
		if ( dirIndex >= 0 ) {
			fileName = skyPath.right( skyPath.length() - dirIndex - 1 );
		}
		//Add in the -skycatalog suffix
		fileName = fileName +"-"+ SKY_CATALOG;
		return fileName;
	}

	void FindSourcesDialog::resetSkyOverlay() {
		createTable( );
		resetSourceView();
		if ( ui.displayViewerCheckBox->isChecked()) {
			//Now tell the viewer to display this sky catalog
			String skyPathStr( skyPath.toStdString());
			emit showOverlay( skyPathStr, overlayColorName );
		}
	}


	void FindSourcesDialog::findSources() {
		if ( image == NULL ) {
			QMessageBox::warning( this, "No Image", "Please load an image to fit.");
			return;
		}

		skyList.clear();
		ui.sourceTable->setRowCount( 0 );
		clearSkyOverlay();

		SHARED_PTR<ImageInterface<Float> > x(image->cloneII());
		ImageAnalysis* analysis = new ImageAnalysis( x );
		bool validRegion = false;
		Record region = makeRegion( &validRegion );
		if ( !validRegion ) {
			QMessageBox::warning( this, "Invalid Region", "Region was not valid for image.");
			return;
		}
		QString cutoffStr = ui.cutoffLineEdit->text();
		bool cutOffValid = false;
		double cutoff = populateCutOff( &cutOffValid );
		if ( !cutOffValid ) {
			QMessageBox::warning( this, "Invalid Cut-Off", "Please specify a valid cut-off value.");
			return;
		}
		int maxEstimates = ui.sourceEstimateCountSpinBox->value();
		try {
			Record sources = analysis->findsources(maxEstimates,cutoff,region,"",True );
			String errorMsg;
			bool ok = skyList.fromRecord(errorMsg, sources);
			if ( !ok ) {
				qDebug() << "Got error from making sky list from record: "<<errorMsg.c_str();
				return;
			}
			resetSkyOverlay();
		} catch( AipsError& error ) {
			qDebug()<< "Could not find sources: "<<error.getMesg().c_str();
		}
	}

	Record FindSourcesDialog::makeRegion( bool * valid ){
		IPosition pos = image->shape();
		String regionName;
		String channelStr = String::toString( channel ) + "~" + String::toString( channel );
		String stokesStr;
		DisplayCoordinateSystem cSys = image->coordinates();

		uInt channelCount = 1;

		CasacRegionManager crm( cSys );
		String diagnostics;
		String infile = image->name();
		Record region;
		if ( pixelBox.length() == 0 ) {
			populatePixelBox();
		}
		if ( pixelBox.length() == 0 ){
			*valid = false;
		}
		else {
			try {
				region = crm.fromBCS( diagnostics, channelCount, stokesStr,
				                      NULL, regionName, channelStr, CasacRegionManager::USE_FIRST_STOKES,
				                      pixelBox, pos, infile);
				*valid = true;
			}
			catch( AipsError& error ) {
				QString errorMsg( error.getMesg().c_str());
				qDebug() << "Error making region: "<<errorMsg;
				*valid = false;
			}
		}
		return region;
	}

	void FindSourcesDialog::createTable( ) {
		TableDesc td("tTableDesc", "1", TableDesc::New);
		const String TYPE_COLUMN("Type");
		const String LONG_COLUMN("Long");
		const String LAT_COLUMN("Lat");
		const String COMP_COLUMN("COMP_ID");
		const String RA_COLUMN("RA");
		const String DEC_COLUMN("DEC");
		const String FLUX_COLUMN("FluxValue");
		td.addColumn( ScalarColumnDesc<String>( TYPE_COLUMN ) );
		td.addColumn( ScalarColumnDesc<double>( LONG_COLUMN ) );
		td.addColumn( ScalarColumnDesc<double>( LAT_COLUMN ) );
		td.addColumn( ScalarColumnDesc<String>( COMP_COLUMN ) );
		td.addColumn( ScalarColumnDesc<String>( RA_COLUMN ) );
		td.addColumn( ScalarColumnDesc<String>( DEC_COLUMN ) );
		td.addColumn( ScalarColumnDesc<double>( FLUX_COLUMN ) );

		//Setup a new table from the description.
		String fileName( "Fit2DTable");
		String filePath = viewer::options.temporaryPath( fileName );
		SetupNewTable newtab( filePath, td, Table::New);
		Table sourceTable(newtab);
		String tableName = sourceTable.tableName();
		skyPath = QString( tableName.c_str());

		//Construct the various column objects.
		ScalarColumn<String> typeCol(sourceTable, TYPE_COLUMN );
		ScalarColumn<double> longCol(sourceTable, LONG_COLUMN );
		ScalarColumn<double> latCol( sourceTable, LAT_COLUMN );
		ScalarColumn<String> idCol( sourceTable, COMP_COLUMN );
		ScalarColumn<String> raCol( sourceTable, RA_COLUMN );
		ScalarColumn<String> decCol( sourceTable, DEC_COLUMN );
		ScalarColumn<double> fluxCol( sourceTable, FLUX_COLUMN );

		//Write the data into the columns.
		int rowCount = skyList.getSize();
		for ( int i=0; i<rowCount; i++ ) {
			sourceTable.addRow();

			//Latitude and longitude
			Quantum< Vector<double> > latLongQuantum = skyList.getLatLong( i );
			Vector<double> angleVector = latLongQuantum.getValue();
			longCol.put(i, angleVector[0]);
			latCol.put( i, angleVector[1]);
			const String UNIT_STR( "unit");
			String unitStr = latLongQuantum.getUnit();
			longCol.rwKeywordSet().define ( UNIT_STR, unitStr );
			latCol.rwKeywordSet().define( UNIT_STR, unitStr );

			//Type
			String refString = skyList.getType( i );
			typeCol.put( i, refString );
			idCol.put(i, String::toString(i));

			//RA and DEC
			String raStr = skyList.getRA( i );
			String decStr = skyList.getDEC( i );
			raCol.put( i, raStr );
			decCol.put( i, decStr );

			//Flux
			double fluxValue = skyList.getFlux(i).getValue();
			fluxCol.put(i, fluxValue );

		}

		sourceTable.rwKeywordSet().define("type", "Skycatalog");
	}

//---------------------------------------------------------------
//                Estimate File
//---------------------------------------------------------------

	bool FindSourcesDialog::writeEstimateFile( QString& filePath,
	        bool screenEstimates, RegionBox* screeningBox ) {
		QFile file( filePath );
		bool success = file.open( QIODevice::WriteOnly | QIODevice::Text );
		if ( success ) {
			QTextStream out( &file );
			QString errorMsg;
			success = skyList.toEstimateFile( out, image.get(), errorMsg, screenEstimates, screeningBox );
			if ( !success ) {
				QMessageBox::warning( this, "Problem Writing Estimates", errorMsg );
			}
			out.flush();
			file.close();
		} else {
			QMessageBox::warning( this, "File Problem", "Please check the file path.\n The file could not be opened for writing.");
		}
		return success;
	}


	String FindSourcesDialog::getScreenedEstimatesFile( const String& estimatesFileName,
	        bool* errorWritingFile ) {
		QString screenedEstimatesFileName(estimatesFileName.c_str());
		if ( screenedEstimatesFileName.length() > 0 && skyList.getSize() > 0 ) {
			if ( !regions.isEmpty() ) {
				RegionBox* screenBox = regions[currentRegionId ];
				if ( screenBox != NULL ) {
					int separatorIndex = screenedEstimatesFileName.lastIndexOf( QDir::separator());
					if ( separatorIndex > 0 ) {
						screenedEstimatesFileName = screenedEstimatesFileName.right(screenedEstimatesFileName.length() - separatorIndex-1);
					}
					String outName= viewer::options.temporaryPath( screenedEstimatesFileName.toStdString() );
					QString outNameStr( outName.c_str() );
					bool success = writeEstimateFile( outNameStr, true, screenBox );
					if ( success ) {
						screenedEstimatesFileName = outNameStr;
					} else {
						*errorWritingFile = true;
					}
				}
			}
		}
		return screenedEstimatesFileName.toStdString();
	}


	void FindSourcesDialog::validateDirectory( const QString& str ) {
		QFile file( str );
		bool valid = file.exists();
		if ( valid ) {
			QAbstractItemModel* model = ui.treeWidget->model();
			QFileSystemModel* fileModel = dynamic_cast<QFileSystemModel*>(model);
			QModelIndex pathIndex = fileModel->index( str );
			ui.treeWidget->setCurrentIndex( pathIndex );
		}
	}

	void FindSourcesDialog::directoryChanged(const QModelIndex& modelIndex ) {
		QString path = fileModel->filePath( modelIndex );
		ui.directoryLineEdit->setText( path );
	}

	void FindSourcesDialog::saveEstimateFile() {
		QString fileName = ui.fileLineEdit->text().trimmed();
		QString fileDirectory = ui.directoryLineEdit->text().trimmed();
		if ( !fileDirectory.endsWith( QDir::separator())) {
			fileDirectory.append( QDir::separator() );
		}

		QString fullPath = fileDirectory + fileName;
		bool written = writeEstimateFile( fullPath );
		if ( written ) {
			emit estimateFileSpecified( fullPath );
			close();
		}
	}

	void FindSourcesDialog::initializeFileManagement() {
		//Initialize the file browsing tree
		fileModel = new QFileSystemModel( ui.treeWidget );
		String fileName( "SourceEstimates");
		String filePath = viewer::options.temporaryPath( fileName );
		QString initialDir( filePath.c_str());
		int dirIndex = initialDir.lastIndexOf( QDir::separator());
		QString fileNameStr = initialDir.right(initialDir.length() - dirIndex - 1)+".txt";
		initialDir = initialDir.left(dirIndex + 1);
		ui.treeWidget->setModel( fileModel );
		fileModel->setRootPath( initialDir );
		QModelIndex initialIndex = fileModel->index( initialDir );
		ui.treeWidget->setCurrentIndex( initialIndex );
		ui.treeWidget->setColumnHidden( 1, true );
		ui.treeWidget->setColumnHidden( 2, true );
		ui.treeWidget->setColumnHidden( 3, true );
		ui.directoryLineEdit->setText( initialDir );
		ui.fileLineEdit->setText( fileNameStr );
		connect( ui.treeWidget, SIGNAL(clicked(const QModelIndex&)), this, SLOT(directoryChanged(const QModelIndex&)));
		connect( ui.directoryLineEdit, SIGNAL(textEdited(const QString&)), this, SLOT(validateDirectory(const QString&)));
	}

//---------------------------------------------------------------
//              Image and Region Updates
//---------------------------------------------------------------

	void FindSourcesDialog::populateImageBounds() {
		ImageStatsCalculator calc( image, NULL, "", false);
		calc.setVerbose(False);
		calc.setList(False);
		Record result = calc.calculate();
		Vector<int> blc(result.asArrayInt( "blc"));
		blcVector.resize(blc.size( ));
		blcVector = blc;
		Vector<int> trc(result.asArrayInt( "trc"));
		trcVector.resize(trc.size( ));
		trcVector = trc;
	}

	QString FindSourcesDialog::getImagePixelBox() const {
		//No regions so just use the image as bounds.
		QString pixelBoxStr;
		if ( image != NULL ){
			const QString ZERO_STR( "0");
			const QString COMMA_STR( ",");
			pixelBoxStr.append( ZERO_STR + COMMA_STR );
			pixelBoxStr.append( ZERO_STR + COMMA_STR );
			IPosition imageShape = image->shape();
			pixelBoxStr.append( QString::number( imageShape(0) - 1) + COMMA_STR);
			pixelBoxStr.append( QString::number( imageShape(1) - 1));
		}
		return pixelBoxStr;
	}

	void FindSourcesDialog::populatePixelBox() {
		QString pixelBoxStr("");
		if ( currentRegionId != DEFAULT_KEY ) {
			pixelBoxStr = regions[currentRegionId]->toString( blcVector, trcVector);
		} else {
			pixelBoxStr = getImagePixelBox();
		}
		pixelBox = pixelBoxStr.toStdString();
	}

	void FindSourcesDialog::setImage( SHARED_PTR<const ImageInterface<Float> > img ) {
		image = img;
		if ( image != NULL ) {
			const DisplayCoordinateSystem cSys = image->coordinates();
			Vector<String> axisNames = cSys.worldAxisNames();
			if ( axisNames.size() >= 2 ) {
				QTableWidgetItem* raHeaderItem = new QTableWidgetItem(axisNames[0].c_str());
				ui.sourceTable->setHorizontalHeaderItem( static_cast<int>(RA_COL), raHeaderItem);
				QTableWidgetItem* decHeaderItem = new QTableWidgetItem( axisNames[1].c_str());
				ui.sourceTable->setHorizontalHeaderItem( static_cast<int>(DEC_COL), decHeaderItem);
			}
			populateImageBounds();
		}
	}

	bool FindSourcesDialog::newRegion( int id, const QString & /*shape*/, const QString &/*name*/,
	                                   const QList<double> & /*world_x*/, const QList<double> & /*world_y*/,
	                                   const QList<int> &pixel_x, const QList<int> &pixel_y,
	                                   const QString & /*linecolor*/, const QString & /*text*/, const QString & /*font*/,
	                                   int /*fontsize*/, int /*fontstyle*/ ) {
		bool regionChanged = false;
		if ( pixel_x.size() == 2 && pixel_y.size() == 2 ) {
			RegionBox* regionBox = regions[id];
			if ( regionBox == NULL ) {
				regionBox = new RegionBox( pixel_x, pixel_y );
				regions[id] = regionBox;
			} else {
				regionBox->update( pixel_x, pixel_y );
			}
			resetCurrentId( id );

			populatePixelBox();
			regionChanged = true;
		}
		return regionChanged;
	}

	bool FindSourcesDialog::updateRegion( int id, viewer::region::RegionChanges changes,
	                                      const QList<double> &/*world_x*/, const QList<double> &/*world_y*/,
	                                      const QList<int> &pixel_x, const QList<int> &pixel_y ) {
		bool regionChanged = false;
		if ( pixel_x.size() == 2 && pixel_y.size() == 2 ) {
			if ( changes != viewer::region::RegionChangeDelete ) {
				RegionBox* box = regions[id];
				if ( box != NULL ) {
					box->update( pixel_x, pixel_y );
					resetCurrentId( id );
					populatePixelBox();
					regionChanged = true;
				} /*else {
					qDebug() << "FitSources::updateRegion unrecognized id="<<id;
				}*/
			} else {
				RegionBox* regionToRemove = regions.take(id);
				delete regionToRemove;
				resetCurrentId( -1 );
				regionChanged = true;
				populatePixelBox();
			}
		}
		return regionChanged;
	}

	QString FindSourcesDialog::getRegionString() const {
		QString str;
		if ( regions.contains( currentRegionId)) {
			if ( regions[currentRegionId] != NULL ){
				str = regions[currentRegionId]->toStringLabelled(blcVector, trcVector);
			}
		}
		return str;
	}

	void FindSourcesDialog::clearRegions() {
		QList<int> keys = regions.keys();
		for ( int i = 0; i < keys.size(); i++ ) {
			RegionBox* box = regions.take( keys[i] );
			delete box;
		}
	}

	void FindSourcesDialog::setChannel( int channel ) {
		this->channel = channel;
	}

	FindSourcesDialog::~FindSourcesDialog() {
		clearRegions();
		delete fileModel;
	}
}
