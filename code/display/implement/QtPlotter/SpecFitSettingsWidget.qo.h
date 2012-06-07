#ifndef SPECFITSETTINGSWIDGET_QO_H
#define SPECFITSETTINGSWIDGET_QO_H

#include <QtGui/QWidget>
#include <display/QtPlotter/SpecFitSettingsWidget.ui.h>

namespace casa {


class QtCanvas;
class LogIO;
class SpecFitMonitor;
class SpecFitter;

class SpecFitSettingsWidget : public QWidget
{
    Q_OBJECT

public:
    void setCanvas( QtCanvas* pCanvas );
    void setFitMonitor( SpecFitMonitor* fitMonitor );
    void resetSpectralFitter( bool optical );
    void setLogger( LogIO* log );
    void setUnits( QString units );
    void setRange( float start, float end );
    bool isOptical() const;
    SpecFitSettingsWidget(QWidget *parent = 0);
    ~SpecFitSettingsWidget();



private slots:
    void setFitRange(float start, float end );

private:
    QtCanvas* pixelCanvas;
    SpecFitter* specFitter;
    LogIO* logger;
    SpecFitMonitor* specFitMonitor;
    Ui::SpecFitSettingsWidget ui;
    bool opticalFitter;

};
}
#endif // SPECFITSETTINGSWIDGET_QO_H
