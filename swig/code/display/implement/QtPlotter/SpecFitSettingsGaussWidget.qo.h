#ifndef SPECFITSETTINGSGAUSSWIDGET_QO_H
#define SPECFITSETTINGSGAUSSWIDGET_QO_H

#include <QtGui/QWidget>
#include <display/QtPlotter/SpecFitSettingsGaussWidget.ui.h>

namespace casa {

class SpecFitSettingsGaussWidget : public QWidget
{
    Q_OBJECT

public:
    SpecFitSettingsGaussWidget(QWidget *parent = 0);
    ~SpecFitSettingsGaussWidget();

private:
    Ui::SpecFitSettingsGaussWidgetClass ui;
    enum TableHeaders {PEAK,CENTER,FWHM,FIXED,END_COLUMN};

private slots:
	void adjustTableRowCount( int count );
};
}
#endif // SPECFITSETTINGSGAUSSWIDGET_H
