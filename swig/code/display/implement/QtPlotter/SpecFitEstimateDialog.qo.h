#ifndef SPECFITESTIMATEDIALOG_QO_H_APRIL
#define SPECFITESTIMATEDIALOG_QO_H_APRIL

#include <QtGui/QWidget>
#include <QtGui/QDialog>
#include <display/QtPlotter/GaussFitEstimate.h>
#include <display/QtPlotter/SpecFitEstimateDialog.ui.h>

namespace casa {

class SpecFitEstimateDialog : public QDialog
{
    Q_OBJECT

public:
    SpecFitEstimateDialog(  const GaussFitEstimate* const estimate, QWidget *parent = 0 );
    void updateEstimate( GaussFitEstimate* const estimate ) const ;
    ~SpecFitEstimateDialog();

private:
    Ui::SpecFitEstimateDialog ui;
};

} // end namespace casa

#endif // SPECFITESTIMATEDIALOG_QO_H_APRIL
