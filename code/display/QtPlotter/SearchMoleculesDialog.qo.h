#ifndef SEARCHMOLECULES_QO_H
#define SEARCHMOLECULES_QO_H

#include <QtGui/QDialog>
#include <display/QtPlotter/SearchMoleculesDialog.ui.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <display/QtPlotter/conversion/Converter.h>
#include <casa/Containers/Record.h>

namespace casa {

class SearchMoleculesDialog : public QDialog
{
    Q_OBJECT

public:
    SearchMoleculesDialog(QWidget *parent = 0);
    QList<int> getLineIndices() const;
    void getLine(int lineIndex, Float& peak, Float& center, QString& molecularName ) const;
    void setSpectralCoordinate( SpectralCoordinate coordinate );
    void setRange( float min, float max, QString units );
    QString getUnit() const;
    ~SearchMoleculesDialog();

signals:
	void moleculesSelected();

private slots:
    void openCatalog();
    void search();
    void searchUnitsChanged( const QString& searchUnits );

private:
    void displaySearchResults( const Record& results );
    void initializeTable();
    void setTableValue( int row, int col, const QString& val );
    void setTableValueHTML( int row, int col, const QString& val );
    void setTableValue( int row, int col, double val );
    void convertRangeLineEdit( QLineEdit* lineEdit, Converter* converter );
    void initializeSearchRange( QLineEdit* lineEdit, Double& value, double redshift );
    double getRedShiftAdjustment() const;
    enum ResultColumns{ COL_SPECIES, COL_CHEMICAL, COL_FREQUENCY,
    	COL_QN, COL_INTENSITY, COL_EL, COLUMN_COUNT};
    Ui::SearchMoleculesDialog ui;
    String defaultDatabasePath;
    String databasePath;
    SpectralCoordinate spectralCoordinate;
    QString unitStr;
    static const QString SPLATALOGUE_UNITS;
    static const double SPLATALOGUE_DEFAULT_MIN;
    static const double SPLATALOGUE_DEFAULT_MAX;
};
}
#endif // SEARCHMOLECULES_QO_H
