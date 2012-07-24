#include "SearchMoleculesDialog.qo.h"
#include <spectrallines/Splatalogue/SearchEngine.h>
#include <spectrallines/Splatalogue/SplatalogueTable.h>
#include <display/QtPlotter/Util.h>
#include <casa/System/Aipsrc.h>
#include <casa/Exceptions/Error.h>
#include <QFileDialog>
#include <QTemporaryFile>
#include <QDebug>

namespace casa {

const QString SearchMoleculesDialog::SPLATALOGUE_UNITS="GHz";
const double SearchMoleculesDialog::SPLATALOGUE_DEFAULT_MIN = 84;
const double SearchMoleculesDialog::SPLATALOGUE_DEFAULT_MAX = 90;

SearchMoleculesDialog::SearchMoleculesDialog(QWidget *parent)
    : QDialog(parent), unitStr( SPLATALOGUE_UNITS )
{
	ui.setupUi(this);
	QList<QString> frequencyUnitList;
	frequencyUnitList << "Hz" << "KHz" << "MHz" << "GHz" << "A" << "nm"
			<< "um" << "mm" << "cm" << "m";
	for ( int i = 0; i < frequencyUnitList.size(); i++ ){
		ui.rangeUnitComboBox->addItem( frequencyUnitList[i]);
	}
	ui.redShiftUnitLabel->setText( Util::toHTML(unitStr ));
	ui.rangeUnitComboBox->setCurrentIndex(frequencyUnitList.indexOf(SPLATALOGUE_UNITS));
	connect( ui.rangeUnitComboBox, SIGNAL(currentIndexChanged(const QString&)), this, SLOT( searchUnitsChanged(const QString&)));

	QDoubleValidator* validator = new QDoubleValidator( this );
	ui.rangeMinLineEdit->setValidator( validator );
	ui.rangeMaxLineEdit->setValidator( validator );
	ui.redShiftLineEdit->setValidator( validator );
	ui.redShiftLineEdit->setText( QString::number(0) );


	connect( ui.searchButton, SIGNAL(clicked()), this, SLOT(search()));
	connect( ui.openButton, SIGNAL(clicked()), this, SLOT(openCatalog()));
	connect( ui.applyButton, SIGNAL(clicked()), this, SIGNAL(moleculesSelected()));
	connect( ui.closeButton, SIGNAL(clicked()), this, SLOT(reject()));

	initializeTable();

	Bool foundDatabase = Aipsrc::find(defaultDatabasePath, "user.ephemerides.SplatDefault.tbl");
	if( !foundDatabase ){
		foundDatabase = Aipsrc::findDir(defaultDatabasePath, "data/ephemerides/SplatDefault.tbl");
	}
	if ( foundDatabase ){
		ui.catalogLineEdit->setText( "Splatalogue");
	}
}

//-------------------------------------------------------------------------------------
//                        Initialization
//-------------------------------------------------------------------------------------

void SearchMoleculesDialog::initializeTable(){
	QStringList tableHeaders(QStringList() << "Species" << "Chemical Name" <<
				"Frequency" << "Resolved QNs" << "Intensity" << "EL");
	ui.searchResultsTable->setColumnCount( COLUMN_COUNT );
	ui.searchResultsTable->setHorizontalHeaderLabels( tableHeaders );
	ui.searchResultsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
	//ui.searchResultsTable->setSelectionMode( QAbstractItemView::SingleSelection );
}

void SearchMoleculesDialog::setRange( float min, float max, QString units ){
	if ( unitStr != units ){
		Converter* converter = Converter::getConverter( units, unitStr, &spectralCoordinate );
		min = converter->convert( min );
		max = converter->convert( max );
		delete converter;
	}
	ui.rangeMinLineEdit->setText( QString::number( min ));
	ui.rangeMaxLineEdit->setText( QString::number( max ));
}

void SearchMoleculesDialog::setSpectralCoordinate( SpectralCoordinate coordinate ){
	spectralCoordinate = coordinate;
}
//--------------------------------------------------------------------------------------
//                        Signal/Slot
//--------------------------------------------------------------------------------------

void SearchMoleculesDialog::openCatalog(){
	QString dPath = QFileDialog::getExistingDirectory( this,
			"Select Spectral Line Catalog",
			"/home");
	databasePath = dPath.toStdString();
	ui.catalogLineEdit->setText( dPath );
}

void SearchMoleculesDialog::convertRangeLineEdit( QLineEdit* lineEdit, Converter* converter ){
	QString editText = lineEdit->text();
	if ( editText != NULL && !editText.isEmpty()){
		if ( converter != NULL ){
			double val = editText.toDouble();
			val = converter->convert( val );
			lineEdit->setText( QString::number( val ));
		}
	}
}


void SearchMoleculesDialog::searchUnitsChanged( const QString& newUnits ){
	//If the units have changed
	if ( unitStr != newUnits ){
		//Change the redshift to the new units
		ui.redShiftUnitLabel->setText( newUnits );

		//Readjust the text fields giving the search lookup range & redshift
		Converter* converter = Converter::getConverter( unitStr, newUnits, &spectralCoordinate );
		convertRangeLineEdit(ui.rangeMinLineEdit, converter);
		convertRangeLineEdit(ui.rangeMaxLineEdit, converter);
		convertRangeLineEdit(ui.redShiftLineEdit, converter);
		delete converter;
	}
	unitStr = newUnits;
}



//------------------------------------------------------------------------------------
//                 Performing the search and displaying the results
//-----------------------------------------------------------------------------------

void SearchMoleculesDialog::initializeSearchRange( QLineEdit* lineEdit, Double& value, double redShift ){
	QString valueStr = lineEdit->text();
	if ( !valueStr.isEmpty() ){
		value = valueStr.toDouble();
		value = value + redShift;
		QString redShiftStr = ui.redShiftLineEdit->text();
		if ( unitStr != SPLATALOGUE_UNITS ){
			Converter* converter = Converter::getConverter(unitStr, SPLATALOGUE_UNITS, &spectralCoordinate );
			value = converter->convert( value );
			delete converter;
		}
	}
	//Just adjust the default values by the redshift
	else {
		value = value + redShift;
	}
}

void SearchMoleculesDialog::search(){

	//First decide if we have a database to search
	//We will try to use a user specified one.  Otherwise, we
	//will try the default.
	if ( databasePath.length() == 0 ){
		databasePath = defaultDatabasePath;
	}

	if ( databasePath.length() > 0 ){

		//Get the search parameters
		QString searchList = ui.searchLineEdit->text();
		QList<QString> moleculeList;
		if ( ! searchList.isEmpty() ){
			moleculeList = searchList.split(",");
		}
		Vector<String> chemNames( moleculeList.size());
		for ( int i = 0; i < moleculeList.size(); i++ ){
			chemNames[i] = moleculeList[i].trimmed().toStdString();
			//qDebug() << "Chemical name="<<chemNames[i].c_str();
		}

		//Create a temporary file for the search results
		QTemporaryFile tmpFile( "SearchMoleculesResults" );
		QString path = tmpFile.fileName();
		String resultTableName( path.toStdString());

		Vector<String> species;
		Vector<String> qns;
		Double intensityLow = -1;
		Double intensityHigh = -1;
		Double smu2Low = -1;
		Double smu2High = -1;
		Double logaLow = -1;
		Double logaHigh = -1;
		Double elLow = -1;
		Double elHigh = -1;
		Double euLow = -1;
		Double euHigh = -1;
		Bool includeRRLs = true;
		Bool onlyRRLs = false;

		//Set the range for the search
		Double minValue = SPLATALOGUE_DEFAULT_MIN;
		Double maxValue = SPLATALOGUE_DEFAULT_MAX;
		double redShift = getRedShiftAdjustment();
		initializeSearchRange( ui.rangeMinLineEdit, minValue, redShift );
		initializeSearchRange( ui.rangeMaxLineEdit, maxValue, redShift );

		try {
			SplatalogueTable splatalogueTable( databasePath );
			const SearchEngine searcher( &splatalogueTable, false, "", false );
			SplatalogueTable *resTable = searcher.search( resultTableName, minValue, maxValue,
				species, false, chemNames, qns, intensityLow, intensityHigh,
				smu2Low, smu2High, logaLow, logaHigh,
				elLow, elHigh, euLow, euHigh,
				includeRRLs, onlyRRLs );
			Record resultsRecord = resTable->toRecord();
			displaySearchResults( resultsRecord );
			delete resTable;
		}
		catch( AipsError& err ){
			QString msg( "Search was unsuccessful.\nPlease check that a valid local database has been specified.");
			Util::showUserMessage( msg, this );
		}
	}
	else {
		//Tell the user we could not find a database to search.
		if ( databasePath.length() == 0 ){
			QString msg( "Could not find a local Splatalogue database!/nTry specifying one in the local file system.");
			Util::showUserMessage(msg, this );
		}
		else {
			QString msg( "Local splatalogue database was not valid.");
			Util::showUserMessage( msg, this );
		}
	}
}

void SearchMoleculesDialog::displaySearchResults( const Record& results ){
	int resultCount = results.size();
	if ( resultCount == 0 ){
		QString msg( "There were no search results matching the given criteria.");
		Util::showUserMessage( msg, this );
	}
	ui.searchResultsTable->setRowCount( resultCount );
	//qDebug() << "Number of search results is " << resultCount;
	for ( int i = 0; i < resultCount; i++ ){

		Record line = results.asRecord("*" + String::toString(i) );

		//Species
		String species = line.asString( SplatalogueTable::RECORD_SPECIES );
		setTableValueHTML( i, COL_SPECIES, species.c_str() );
		//bool recommended = line.asBool(SplatalogueTable::RECORD_RECOMMENDED);

		//Chemical Name
		String chemName = line.asString(SplatalogueTable::RECORD_CHEMNAME);
		setTableValue( i, COL_CHEMICAL, chemName.c_str() );

		//Frequency
		Record frequencyRecord = line.asRecord(SplatalogueTable::RECORD_FREQUENCY);
		String freqUnits = frequencyRecord.asString( SplatalogueTable::RECORD_UNIT );
		//qDebug() << "Freq units="<<freqUnits.c_str();
		//Default frequency units seem to be GHz
		double freqValue = frequencyRecord.asdouble( SplatalogueTable::RECORD_VALUE );
		setTableValue( i, COL_FREQUENCY, freqValue );

		//QNS
		String qns = line.asString(SplatalogueTable::RECORD_QNS);
		setTableValueHTML( i, COL_QN, qns.c_str());

		//Intensity
		float intensity = line.asfloat(SplatalogueTable::RECORD_INTENSITY);
		setTableValue( i, COL_INTENSITY, intensity );

		Record smu2Record = line.asRecord(SplatalogueTable::RECORD_SMU2);
		String smu2Units = smu2Record.asString( SplatalogueTable::RECORD_UNIT );
		//double smu2Value = smu2Record.asdouble( SplatalogueTable::RECORD_VALUE );
		//qDebug() << "SMU2 value=" << smu2Value << " units " << smu2Units.c_str();

		//float logaValue = line.asfloat(SplatalogueTable::RECORD_LOGA );
		//qDebug() << "logavalue="<<logaValue;
		/*Record qelRecord = line.asRecord( SplatalogueTable::RECORD_EL );
		String elUnits = qelRecord.asString( SplatalogueTable::RECORD_UNIT );
		//qDebug() << "EL units are " << elUnits.c_str();
		double elValue = qelRecord.asdouble( SplatalogueTable::RECORD_VALUE );
		setTableValue( i, COL_EL, elValue );*/
		Record queRecord = line.asRecord( SplatalogueTable::RECORD_EU );
		String queUnits = queRecord.asString( SplatalogueTable::RECORD_UNIT );
		//double queValue = queRecord.asdouble( SplatalogueTable::RECORD_VALUE );
		//qDebug() << "EU value=" << queValue << " units="<<queUnits.c_str();

	}
}

double SearchMoleculesDialog::getRedShiftAdjustment() const{
	QString redShiftStr = ui.redShiftLineEdit->text();
	double redShift = 0;
	if ( ! redShiftStr.isEmpty() ){
		redShift = redShiftStr.toDouble();
	}
	return redShift;
}

//-----------------------------------------------------------------------------
//                        Accessors
//-----------------------------------------------------------------------------

QString SearchMoleculesDialog::getUnit() const {
	return unitStr;
}

QList<int> SearchMoleculesDialog::getLineIndices() const{
	QList<QTableWidgetSelectionRange> ranges = ui.searchResultsTable->selectedRanges();
	QList<int> lineIndices;
	for ( int i = 0; i < ranges.size(); i++ ){
		int rangeRowCount = ranges[i].rowCount();
		int lowerIndex = ranges[i].bottomRow();
		for ( int j = 0; j < rangeRowCount; j++ ){
			lineIndices.append( lowerIndex + j );
		}
	}
	return lineIndices;
}

void SearchMoleculesDialog::getLine(int lineIndex, Float& peak, Float& center,
		QString& molecularName ) const {
	double redshift = getRedShiftAdjustment();

	//Name
	QLabel* speciesItem = dynamic_cast<QLabel*>(ui.searchResultsTable->cellWidget( lineIndex, COL_SPECIES ));
	if ( speciesItem != NULL ){
		molecularName = speciesItem->text();
	}
	else {
		qDebug() << "speciesItem was null";
	}

	//Frequency
	QTableWidgetItem* frequencyItem = ui.searchResultsTable->item(lineIndex, COL_FREQUENCY );
	if ( frequencyItem != NULL ){
		QString freqStr = frequencyItem->text();
		center = freqStr.toFloat();
		center = center - redshift;
	}

	//Intensity
	QTableWidgetItem* peakItem = ui.searchResultsTable->item(lineIndex, COL_INTENSITY );
	if ( peakItem != NULL ){
		QString peakStr = peakItem->text();
		peak = peakStr.toFloat();
	}
}


//--------------------------------------------------------------------------------------
//                   Utiltiy - Search Results Table
//--------------------------------------------------------------------------------------

void SearchMoleculesDialog::setTableValue(int row, int col, double val ){
	QString numberStr = QString::number( val );
	setTableValue( row, col, numberStr );
}

void SearchMoleculesDialog::setTableValue( int row, int col, const QString& val ){
	QTableWidgetItem* tableItem = new QTableWidgetItem();
	tableItem -> setText( val );
	ui.searchResultsTable->setItem( row, col, tableItem );
}

void SearchMoleculesDialog::setTableValueHTML( int row, int col, const QString& val ){
	QLabel* label = new QLabel( this );
	label->setTextFormat(Qt::RichText);
	QString htmlStr = "<html>"+val+"</html>";
	label->setText( val );
	ui.searchResultsTable->setCellWidget( row, col, label );
}

SearchMoleculesDialog::~SearchMoleculesDialog()
{

}
}
