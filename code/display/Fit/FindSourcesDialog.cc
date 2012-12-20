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
#include <QMessageBox>
#include <QFileSystemModel>
#include <imageanalysis/Regions/CasacRegionManager.h>
#include <imageanalysis/ImageAnalysis/ImageAnalysis.h>
#include <tables/Tables.h>
#include <casa/Quanta/MVAngle.h>
#include <display/Display/Options.h>

namespace casa {

FindSourcesDialog::FindSourcesDialog(QWidget *parent)
    : QDialog(parent){
	ui.setupUi(this);

	QDoubleValidator* validator = new QDoubleValidator( 0, std::numeric_limits<double>::max(), 7, this );
	ui.cutoffLineEdit->setValidator( validator );
	ui.cutoffLineEdit->setText( QString::number(0.01f));

	QStringList tableHeaders =(QStringList()<< "ID" << "RA" << "DEC" << "Flux");
	ui.sourceTable->setColumnCount( tableHeaders.size());
	ui.sourceTable->setHorizontalHeaderLabels( tableHeaders );
	ui.sourceTable->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.sourceTable->setSelectionMode( QAbstractItemView::SingleSelection );
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
		resetSourceView();
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
		setTableValue( i, ID_COL, String::toString(i));

		//RA and DEC
		String raStr = skyList.getRA( i );
		String decStr = skyList.getDEC( i );
		setTableValue( i, RA_COL, raStr );
		setTableValue( i, DEC_COL, decStr );

		//Flux
		double fluxVal = skyList.getFlux( i ).getValue();
		String fluxStr = String::toString( fluxVal );
		setTableValue( i, FLUX_COL, fluxStr );
	}
}

void FindSourcesDialog::setTableValue(int row, int col, const String& val ){
	QTableWidgetItem* tableItem = new QTableWidgetItem();
	QString valueStr( val.c_str() );
	tableItem -> setText( valueStr );
	ui.sourceTable->setItem( row, col, tableItem );
}

void FindSourcesDialog::setSourceResultsVisible( bool visible ){
	ui.sourceTable->setVisible( visible );
	ui.line->setVisible( visible );
	ui.deleteSourceButton->setVisible( visible );
	ui.sourceEstimatesLabel->setVisible( visible );
	ui.fileLabel->setVisible( visible );
	ui.directoryLabel->setVisible( visible );
	ui.fileLineEdit->setVisible( visible );
	ui.directoryLineEdit->setVisible( visible );
	ui.treeWidget->setVisible( visible );
	ui.saveButton->setVisible( visible );
}

//----------------------------------------------------------------
//                    Finding Sources
//----------------------------------------------------------------
void FindSourcesDialog::findSources(){
	if ( image == NULL ){
		QMessageBox::warning( this, "No Image", "Please load an image to fit.");
		return;
	}
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
		int count = skyList.getSize();
		ui.sourceTable->setRowCount( count );
		String imagePath = image->name();
		QString imagePathStr( imagePath.c_str());
		String skyPathStr;
		int lastSeparator = imagePathStr.lastIndexOf( QDir::separator());
		if ( lastSeparator > 0 ){
			QString fileName = imagePathStr.right( imagePathStr.length() - lastSeparator - 1 );

			QString directory = imagePathStr.left( lastSeparator + 1);
			int dotIndex = fileName.lastIndexOf( ".");
			if ( dotIndex > 0 ){
				fileName = fileName.left( dotIndex );
			}
			fileName = fileName + ".skycat";
			QString skyPath = directory + fileName;
			skyPathStr = skyPath.toStdString();
		}
		createTable( skyPathStr );
		resetSourceView();

		//Now tell the viewer to display this sky catalog
		emit showOverlay( skyPathStr, "sky cat", "skycatalog" );
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

void FindSourcesDialog::createTable( const String& path ){
	TableDesc::
	TableDesc td("tTableDesc", "1", TableDesc::New);
	td.addColumn( ScalarColumnDesc<String>("Type") );
	td.addColumn( ScalarColumnDesc<double>("Long") );
	td.addColumn( ScalarColumnDesc<double>("Lat") );
	td.addColumn( ScalarColumnDesc<String>("COMP_ID") );
	td.addColumn( ScalarColumnDesc<String>("RA") );
	td.addColumn( ScalarColumnDesc<String>("DEC") );
	td.addColumn( ScalarColumnDesc<double>("FluxValue") );

	//Setup a new table from the description.
	SetupNewTable newtab( path, td, Table::New);
	Table sourceTable(newtab);

	//Construct the various column objects.
	ScalarColumn<String> typeCol(sourceTable, "Type");
	ScalarColumn<double> longCol(sourceTable, "Long");
	ScalarColumn<double> latCol( sourceTable, "Lat");
	ScalarColumn<String> idCol( sourceTable, "COMP_ID");
	ScalarColumn<String> raCol( sourceTable, "RA");
	ScalarColumn<String> decCol( sourceTable, "DEC");
	ScalarColumn<double> fluxCol( sourceTable, "FluxValue");

	//Write the data into the columns.
	int rowCount = skyList.getSize();
	for ( int i=0; i<rowCount; i++ ){
		sourceTable.addRow();

		//Latitude and longitude
		Vector<Double> angleVector = skyList.getLatLong( i );
		longCol.put(i, angleVector[0]);
		latCol.put( i, angleVector[1]);
		const String UNIT_STR( "unit");
		const String DEGREE_STR( "deg");
		longCol.rwKeywordSet().define ( UNIT_STR, DEGREE_STR );
		latCol.rwKeywordSet().define( UNIT_STR, DEGREE_STR );

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
