#ifndef SPECFITSETTINGSPOLYNOMIALWIDGET_QO_H
#define SPECFITSETTINGSPOLYNOMIALWIDGET_QO_H

#include <QtGui/QWidget>
#include <display/QtPlotter/SpecFitSettingsPolynomialWidget.ui.h>
namespace casa {
class SpecFitSettingsPolynomialWidget : public QWidget
{
    Q_OBJECT

public:
    SpecFitSettingsPolynomialWidget(QWidget *parent = 0);
    ~SpecFitSettingsPolynomialWidget();

private slots:
	void enableWidgets( int state );
private:
    Ui::SpecFitSettingsPolynomialWidgetClass ui;


};
}
#endif // SPECFITSETTINGSPOLYNOMIALWIDGET_H
