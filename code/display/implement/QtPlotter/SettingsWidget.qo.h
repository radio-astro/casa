#ifndef SETTINGSWIDGET_QO_H
#define SETTINGSWIDGET_QO_H

#include <QtGui/QWidget>
#include <display/QtPlotter/SettingsWidget.ui.h>
#include <casa/Arrays/Vector.h>

namespace casa {


class QtCanvas;
class LogIO;
class ProfileTaskMonitor;
class ProfileTaskFacilitator;

class SettingsWidget : public QWidget
{
    Q_OBJECT

public:
    void setCollapseVals(const Vector<Float> &spcVals);
    void setCanvas( QtCanvas* pCanvas );
    void setTaskMonitor( ProfileTaskMonitor* fitMonitor );
    void reset( bool taskChanged = false );
    void setLogger( LogIO* log );
    void setUnits( QString units );
    void setRange( float start, float end );
    static bool isOptical();
    static void setOptical( bool optical );
    void setTaskSpecLineFitting( bool specLineFitting );
    void pixelsChanged(int, int );
    SettingsWidget(QWidget *parent = 0);
    ~SettingsWidget();



private slots:
	void clear();
    void setFitRange(float start, float end );

private:
    QtCanvas* pixelCanvas;
    ProfileTaskFacilitator* taskHelper;
    ProfileTaskFacilitator* taskHelperOptical;
    ProfileTaskFacilitator* taskHelperRadio;
    LogIO* logger;
    ProfileTaskMonitor* taskMonitor;
    Ui::SettingsWidget ui;
    static bool optical;
    bool specLineFitting;
    bool newCollapseVals;

};
}
#endif // SPECFITSETTINGSWIDGET_QO_H
