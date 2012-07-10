#ifndef MOMENTSETTINGSWIDGETOPTICAL_QO_H
#define MOMENTSETTINGSWIDGETOPTICAL_QO_H

#include <QtGui/QWidget>
#include <display/QtPlotter/MomentSettingsWidgetOptical.ui.h>
#include <display/QtPlotter/ProfileTaskFacilitator.h>
#include <imageanalysis/ImageAnalysis/SpectralCollapser.h>
namespace casa {


class MomentSettingsWidgetOptical : public QWidget, public ProfileTaskFacilitator
{
    Q_OBJECT

public:
    MomentSettingsWidgetOptical(QWidget *parent = 0);
    void setUnits( QString units );
    void setRange( float min, float max );
    void reset();
    void clear();
    ~MomentSettingsWidgetOptical();

private slots:
    void changeCollapseType(QString text=QString(""));
	void changeCollapseError(QString text=QString(""));
	void collapseImage();

private:
    Ui::MomentSettingsWidgetOptical ui;
    SpectralCollapser* collapser;
    SpectralCollapser::CollapseType  itsCollapseType;
    SpectralCollapser::CollapseError itsCollapseError;
};

}

#endif // MOMENTSETTINGSWIDGETOPTICAL_QO_H
