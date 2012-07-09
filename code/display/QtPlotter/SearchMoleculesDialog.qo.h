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
    ~SearchMoleculesDialog();

private slots:
    void openCatalog();
    void search();

private:
    void displaySearchResults( const Record& results );
    Ui::SearchMoleculesDialog ui;
};
}
#endif // SEARCHMOLECULES_QO_H
