#include "SearchMoleculesDialog.qo.h"
#include <spectrallines/Splatalogue/SearchEngine.h>
#include <spectrallines/Splatalogue/SplatalogueTable.h>

#include <QFileDialog>
#include <QDebug>

namespace casa {

SearchMoleculesDialog::SearchMoleculesDialog(QWidget *parent)
    : QDialog(parent)
{
	ui.setupUi(this);
	QList<QString> frequencyUnitList;
	frequencyUnitList << "Hz" << "KHz" << "MHz" << "GHz" << "A" << "nm"
			<< "um" << "mm" << "cm" << "m";
	for ( int i = 0; i < frequencyUnitList.size(); i++ ){
		ui.rangeUnitComboBox->addItem( frequencyUnitList[i]);
	}

	QDoubleValidator* validator = new QDoubleValidator( this );
	ui.rangeMinLineEdit->setValidator( validator );
	ui.rangeMaxLineEdit->setValidator( validator );
	connect( ui.searchButton, SIGNAL(clicked()), this, SLOT(search));
	connect( ui.openButton, SIGNAL(clicked()), this, SLOT(openCatalog));
}

void SearchMoleculesDialog::openCatalog(){
	QString fileName = QFileDialog::getOpenFileName( this, "Select Spectral Line Catalog",
			"/home", "Catalogs(*.tsv)");
	qDebug() << "Splatalog file is " << fileName;
	SplatalogueTable splatalogueTable( fileName.toStdString() );
	Record resultsRecord = splatalogueTable.toRecord();
	displaySearchResults( resultsRecord );
}

void SearchMoleculesDialog::displaySearchResults( const Record& results ){
	int resultCount = results.size();
	qDebug() << "Number of search results is " << resultCount;
	for ( int i = 0; i < resultCount; i++ ){

		Record line = results.asRecord("*" + String::toString(i) );
		String species = line.asString( SplatalogueTable::RECORD_SPECIES );
		bool recommended = line.asBool(SplatalogueTable::RECORD_RECOMMENDED);
		String chemName = line.asString(SplatalogueTable::RECORD_CHEMNAME);
		Record frequencyRecord = line.asRecord(SplatalogueTable::RECORD_FREQUENCY);
		String freqUnits = frequencyRecord.asString( SplatalogueTable::RECORD_UNIT );
		double freqValue = frequencyRecord.asdouble( SplatalogueTable::RECORD_VALUE );
		String qns = line.asString(SplatalogueTable::RECORD_QNS);
		float intensity = line.asfloat(SplatalogueTable::RECORD_INTENSITY);
		Record smu2Record = line.asRecord(SplatalogueTable::RECORD_SMU2);
		/*		line.defineRecord(RECORD_SMU2, qSmu2);
				line.define(RECORD_LOGA, loga(idx));
				qel.define(RECORD_VALUE, el(idx));
				line.defineRecord(RECORD_EL, qel);
				qeu.define(RECORD_VALUE, eu(idx));
				line.defineRecord(RECORD_EU, qeu);
				line.define(RECORD_LINE_LIST, linelist(idx));
*/
	}
}

void SearchMoleculesDialog::search(){
	QString searchList = ui.searchLineEdit->text();
	QList<QString> moleculeList = searchList.split(",");
	Vector<String> chemNames( moleculeList.size());
	for ( int i = 0; i < moleculeList.size(); i++ ){
		chemNames[i] = moleculeList[i].trimmed().toStdString();
	}
	String resultTableName( "SearchMoleculesResults");
	Vector<String> species;
	Vector<String> qns;
	Double intensityLow = 0;
	Double intensityHigh = 0;
	Double smu2Low = 0;
	Double smu2High = 0;
	Double logaLow = 0;
	Double logaHigh = 0;
	Double elLow = 0;
	Double elHigh = 0;
	Double euLow = 0;
	Double euHigh = 0;
	Bool includeRRLs = true;
	Bool onlyRRLs = false;
	double minValue = ui.rangeMinLineEdit->text().toDouble();
	double maxValue = ui.rangeMaxLineEdit->text().toDouble();
	QString unitStr = ui.rangeUnitComboBox->currentText();

	String defaultDatabasePath = "/home/uniblab/casa/active/data/ephemerides/SplatDefault.tbl";
	SplatalogueTable splatalogueTable( defaultDatabasePath );
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

SearchMoleculesDialog::~SearchMoleculesDialog()
{

}
}
