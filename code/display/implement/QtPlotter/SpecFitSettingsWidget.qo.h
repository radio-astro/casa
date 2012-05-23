#ifndef SPECFITSETTINGSWIDGET_QO_H
#define SPECFITSETTINGSWIDGET_QO_H

#include <QtGui/QWidget>
#include <display/QtPlotter/GaussFitEstimate.h>
#include <display/QtPlotter/SpecFitSettingsWidget.ui.h>

namespace casa {

class SpecFitSettingsPolynomialWidget;
class SpecFitSettingsGaussWidget;
class SpecFitSettingsRangeWidget;
class QtCanvas;

class SpecFitSettingsWidget : public QWidget
{
    Q_OBJECT

public:
    void setCanvas( QtCanvas* pCanvas );
    SpecFitSettingsWidget(QWidget *parent = 0);
    ~SpecFitSettingsWidget();

private slots:
	void showGaussEstimateDialog( int estimateId = -1 );
    void specLineFit();
    void showMainCurve();
    void setCollapseRange(float start, float end );

private:


    const int NEW_ESTIMATE;
    SpecFitSettingsPolynomialWidget* polyWidget;
    SpecFitSettingsGaussWidget* gaussWidget;
    SpecFitSettingsRangeWidget* rangeWidget;
    Ui::SpecFitSettingsWidget ui;
	//QValidator* doubleValidator;
    QtCanvas* pixelCanvas;
};
}
#endif // SPECFITSETTINGSWIDGET_QO_H
