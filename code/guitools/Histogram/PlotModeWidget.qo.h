#ifndef PLOTMODEWIDGET_QO_H
#define PLOTMODEWIDGET_QO_H

#include <QtGui/QWidget>
#include <guitools/Histogram/PlotModeWidget.ui.h>

namespace casa {
class PlotModeWidget : public QWidget {
    Q_OBJECT

public:
    PlotModeWidget(QWidget *parent = 0);
    ~PlotModeWidget();

private:
    Ui::PlotModeWidgetClass ui;
};
}
#endif // PLOTMODEWIDGET_QO_H
