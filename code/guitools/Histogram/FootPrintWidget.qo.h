#ifndef FOOTPRINTWIDGET_QO_H
#define FOOTPRINTWIDGET_QO_H

#include <QtGui/QWidget>
#include <guitools/Histogram/FootPrintWidget.ui.h>

namespace casa {

class FootPrintWidget : public QWidget
{
    Q_OBJECT

public:
    FootPrintWidget(QWidget *parent = 0);
    ~FootPrintWidget();
    enum PlotMode {REGION_MODE,IMAGE_MODE,REGION_ALL_MODE};
public slots:
	void setPlotMode( int mode );
signals:
	void plotModeChanged( int );
private slots:
	void imageModeSelected( bool selected );
	void regionModeSelected( bool selected );
	void regionAllModeSelected( bool selected );

private:
    Ui::FootPrintWidgetClass ui;
};
}

#endif // FOOTPRINTWIDGET_QO_H
