#ifndef SPECFITSETTINGSWIDGETOPTICAL_QO_H
#define SPECFITSETTINGSWIDGETOPTICAL_QO_H

#include <QtGui/QWidget>
#include <display/QtPlotter/ProfileTaskFacilitator.h>
#include <display/QtPlotter/SpecFitSettingsWidgetOptical.ui.h>

namespace casa {

class SpectralFitter;

class SpecFitSettingsWidgetOptical : public QWidget, public ProfileTaskFacilitator
{
    Q_OBJECT

public:
    SpecFitSettingsWidgetOptical(QWidget *parent = 0);
    ~SpecFitSettingsWidgetOptical();
    void setRange(float start, float end );
    void setUnits( QString units );
    void reset();
    void clear();

private slots:
	void specLineFit();
	void clean();

private:
    Ui::SpecFitSettingsWidgetOptical ui;
    SpectralFitter* fitter;
};

}
#endif // SPECFITSETTINGSWIDGETOPTICAL_H
