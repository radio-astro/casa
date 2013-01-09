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

namespace casa {

FindSourcesDialog::FindSourcesDialog(QWidget *parent)
    : QDialog(parent), SKY_CATALOG("skycatalog"){

	ui.setupUi(this);

	QTime time = QTime::currentTime();
	qsrand((uInt)time.msec());

	QDoubleValidator* validator = new QDoubleValidator( 0, std::numeric_limits<double>::max(), 7, this );
	ui.cutoffLineEdit->setValidator( validator );
	ui.cutoffLineEdit->setText( QString::number(0.01f));

	QStringList tableHeaders =(QStringList()<< "RA" << "DEC" << "Flux" << "Major Axis"<<"Minor Axis"<<"Angle");
	ui.sourceTable->setColumnCount( tableHeaders.size());
	ui.sourceTable->setHorizontalHeaderLabels( tableHeaders );
	ui.sourceTable->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.sourceTable->setSelectionMode( QAbstractItemView::SingleSelection );
	ui.sourceTable->setSortingEnabled( true );
	setSourceResultsVisible( false );

	initializeFileManagement();

	connect( ui.findSourcesButton, SIGNAL(clicked()), this, SLOT(findSources()));
	connect( ui.deleteSourceButton, SIGNAL(clicked()), this, SLOT( deleteSelectedSource()));
	connect( ui.cancelButton, SIGNAL(clicked()), this, SLOT(canceledFindSources()));
	connect( ui.saveButton, SIGNAL(clicked()), this, SLOT(saveEstimateFile()));
}

void FindSourcesDialog::canceledFindSources(){
	close();
}


//-------------------------------------------------------------------
//                  Editing the Source List
//------------------------------------------------------------------

void FindSourcesDialog::deleteSelectedSource(){
	QList<QTableWidgetSelectionRange> selectedItems = ui.sourceTable->selectedRanges();
	if ( selectedItems.isEmpty()){
		QMessageBox::warning( this, "No Source", "Please select a source to delete.");
	}
	else {
		clearSkyOverlay();
		int removeCount = selectedItems.size();
		QVector<int> removeIndices;
		for ( int i = 0; i < removeCount; i++ ){
			int startRow = selectedItems[i].topRow();
			int endRow = selectedItems[i].bottomRow();
			for ( int j = startRow; j <= endRow; j++ ){
				removeIndices.append( j );
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
	if ( rowCount > 0 ){
		sourceResultsVisible = true;
	}
	setSourceResultsVisible( sourceResultsVisible );

	for ( int i=0; i<rowCount; i++ ){

		//Type
		QTableWidgetItem* item = new QTableWidgetItem( QString::number(i) );
		ui.sourceTable->setVerticalHeaderItem(i, item );
		//setTableValue( i, ID_COL, String::toString(i));

		//RA and DEC
		String raStr = skyList.getRA( i );
		String decStr = skyList.getDEC( i );
		setTableValue( i, RA_COL, raStr );
		setTableValue( i, DEC_COL, decStr );

		//Flux
		double fluxVal = skyList.getFlux( i ).getValue();
		String fluxStr = String::toString( fluxVal );
		setTableValue( i, FLUX_COL, fluxStr );

		//Major & minor axis and position angle
		Quantity majorAxisQuantity = skyList.getMajorAxis( i );
		String majorAxisStr = String::toString(majorAxisQuantity.getValue());
		setTableValue( i, MAJOR_AXIS_COL, majorAxisStr );
		Quantity minorAxisQuantity = skyList.getMinorAxis( i );
		String minorAxisStr = String::toString( minorAxisQuantity.getValue());
		setTableValue( i, MINOR_AXIS_COL, minorAxisStr );
		Quantity angleQuantity = skyList.getAngle( i );
		String angleStr = String::toString( angleQuantity.getValue());
		setTableValue( i, ANGLE_COL, angleStr );
	}
	ui.sourceTable->resizeColumnsToContents();
}

void FindSourcesDialog::setTableValue(int row, int col, const String& val ){
	QTableWidgetItem* tableItem = new QTableWidgetItem();
	tableItem->setFlags( tableItem->flags() ^ Qt::ItemIsEditable );
	QString valueStr( val.c_str() );
	bool validNum;
	float numVal = valueStr.toFloat(&validNum);
	if ( !validNum ){
		tableItem -> setText( valueStr );
	}
	else {
		tableItem ->setData(Qt::DisplayRole, numVal );
	}
	ui.sourceTable->setItem( row, col, tableItem );
}

void FindSourcesDialog::setSourceResultsVisible( bool visible ){
	QLayout* dLayout = layout();
	QVBoxLayout* dialogLayout = dynamic_cast<QVBoxLayout*> (dLayout);
	int holderIndex = dialogLayout->indexOf( ui.sourceResultHolder );
	if ( !visible ){
		if ( holderIndex >= 0 ){
			resultIndex = holderIndex;
			dialogLayout->removeWidget( ui.sourceResultHolder );
			ui.sourceResultHolder->setParent( NULL );
			setMinimumSize(300,150);
			setMaximumSize(400,200);
		}
	}
	else {
		if ( holderIndex < 0 ){
			dialogLayout->insertWidget( resultIndex, ui.sourceResultHolder );
			setMinimumSize( 950, 400 );
			setMaximumSize( 1200, 600);
		}
	}
	ui.saveButton->setVisible( visible );
}


//----------------------------------------------------------------
//                    Finding Sources
//----------------------------------------------------------------

void FindSourcesDialog::clearSkyOverlay(){
	//Clean up if we have old results lying around.
	if ( skyPath.length() > 0 ){
		//Just get the file name
		int dirIndex = skyPath.lastIndexOf( QDir::separator() );
		QString fileName = skyPath;
		if ( dirIndex >= 0 ){
			fileName = skyPath.right( skyPath.length() - dirIndex - 1 );
		}
		//Add in the -skycatalog suffix
		fileName = fileName +"-"+ SKY_CATALOG;
		String removePath( fileName.toStdString() );
		//Tell the viewer to remove the previous overlay
		emit removeOverlay( removePath );
		skyPath.clear();
	}
}

void FindSourcesDialog::resetSkyOverlay(){
	createTable( );
	resetSourceView();
	//Now tell the viewer to display this sky catalog
	String skyPathStr( skyPath.toStdString());
	emit showOverlay( skyPathStr );
}


void FindSourcesDialog::findSources(){
	if ( image == NULL ){
		QMessageBox::warning( this, "No Image", "Please load an image to fit.");
		return;
	}

	skyList.clear();
	ui.sourceTable->setRowCount( 0 );
	clearSkyOverlay();

	ImageAnalysis* analysis = new ImageAnalysis( image );
	Record region = makeRegion();
	QString cutoffStr = ui.cutoffLineEdit->text();
	double cutoff = cutoffStr.toDouble();
	int maxEstimates = ui.sourceEstimateCountSpinBox->value();
	try {
		Record sources = analysis->findsources(maxEstimates,cutoff,region,"",False );
		String errorMsg;
		bool ok = skyList.fromRecord(errorMsg, sources);
		if ( !ok ){
			qDebug() << "Got error from making sky list from record: "<<errorMsg.c_str();
			return;
		}
		resetSkyOverlay();
	}
	catch( AipsError& error ){
		qDebug()<< "Could not find sources: "<<error.getMesg().c_str();
	}
}

Record FindSourcesDialog::makeRegion() const {
	IPosition pos = image->shape();
	String regionName;
	String channelStr = String::toString( channel ) + "~" + String::toString( channel );
	String stokesStr;
	CoordinateSystem cSys = image->coordinates();

	uInt channelCount = 1;

	CasacRegionManager crm( cSys );
	String diagnostics;
	String infile = image->name();
	Record region = crm.fromBCS( diagnostics, channelCount, stokesStr,
			NULL, regionName, channelStr, CasacRegionManager::USE_FIRST_STOKES,
			pixelBox, pos, infile);
	return region;
}

void FindSourcesDialog::createTable( ){
	TableDesc::
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
	SetupNewTable newtab( /*path*/"", td, Table::New);
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
	for ( int i=0; i<rowCount; i++ ){
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

bool FindSourcesDialog::writeEstimateFile( QString& filePath ){
	QFile file( filePath );
	bool success = file.open( QIODevice::WriteOnly | QIODevice::Text );
	if ( success ){
		QTextStream out( &file );
		QString errorMsg;
		success = skyList.toEstimateFile( out, image, errorMsg );
		if ( !success ){
			QMessageBox::warning( this, "Problem Writing Estimates", errorMsg );
		}
		out.flush();
		file.close();
	}
	else {
		QMessageBox::warning( this, "File Problem", "Please check the file path.\n The file could not be opened for writing.");
	}
	return success;
}

void FindSourcesDialog::validateDirectory( const QString& str ){
	QFile file( str );
	bool valid = file.exists();
	if ( valid ){
		QAbstractItemModel* model = ui.treeWidget->model();
		QFileSystemModel* fileModel = dynamic_cast<QFileSystemModel*>(model);
		QModelIndex pathIndex = fileModel->index( str );
		ui.treeWidget->setCurrentIndex( pathIndex );
	}
}

void FindSourcesDialog::directoryChanged(const QModelIndex& modelIndex ){
	QString path = fileModel->filePath( modelIndex );
	ui.directoryLineEdit->setText( path );
}

void FindSourcesDialog::saveEstimateFile(){
	QString fileName = ui.fileLineEdit->text().trimmed();
	QString fileDirectory = ui.directoryLineEdit->text().trimmed();
	if ( !fileDirectory.endsWith( QDir::separator())){
		fileDirectory.append( QDir::separator() );
	}

	QString fullPath = fileDirectory + fileName;
	bool written = writeEstimateFile( fullPath );
	if ( written ){
		emit estimateFileSpecified( fullPath );
		close();
	}
}

void FindSourcesDialog::initializeFileManagement(){
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

void FindSourcesDialog::setImage( ImageInterface<Float>* img ){
	image = img;
}

void FindSourcesDialog::setPixelBox( const String& box ){
	pixelBox = box;
}

void FindSourcesDialog::setChannel( int channel ){
	this->channel = channel;
}

FindSourcesDialog::~FindSourcesDialog(){
}
}
