#ifndef SEARCHMOLECULES_QO_H
#define SEARCHMOLECULES_QO_H

#include <QtGui/QDialog>
#include <display/QtPlotter/SearchMoleculesDialog.ui.h>
#include <casa/Containers/Record.h>

namespace casa {

class SearchMoleculesDialog : public QDialog
{
    Q_OBJECT

public:
    SearchMoleculesDialog(QWidget *parent = 0);
    QList<int> getLineIndices() const;
    void getLine(int lineIndex, Float& peak, Float& center, QString& molecularName ) const;
    ~SearchMoleculesDialog();

private slots:
    void openCatalog();
    void search();

private:
    void displaySearchResults( const Record& results );
    void initializeTable();
    void setTableValue( int row, int col, const QString& val );
    void setTableValueHTML( int row, int col, const QString& val );
    void setTableValue( int row, int col, double val );
    enum ResultColumns{ COL_SPECIES, COL_CHEMICAL, COL_FREQUENCY,
    	COL_QN, COL_INTENSITY, COL_EL, COLUMN_COUNT};
    Ui::SearchMoleculesDialog ui;
    String defaultDatabasePath;
    String databasePath;
};
}
#endif // SEARCHMOLECULES_QO_H
