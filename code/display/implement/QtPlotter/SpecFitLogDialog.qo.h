#ifndef SPECFITLOGDIALOG_QO_H
#define SPECFITLOGDIALOG_QO_H

#include <QtGui/QDialog>
#include <display/QtPlotter/SpecFitLogDialog.ui.h>

namespace casa {

class SpecFitLogDialog : public QDialog
{
    Q_OBJECT

public:
    SpecFitLogDialog(QWidget *parent = 0);
    ~SpecFitLogDialog();
    void setLogResults( const QString& results );

private:
    Ui::SpecFitLogDialog ui;
};
}
#endif // SPECFITLOGDIALOG_H
