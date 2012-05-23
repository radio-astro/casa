#ifndef SPECFITSETTINGSRANGEWIDGET_QO_H
#define SPECFITSETTINGSRANGEWIDGET_QO_H

#include <QtGui/QWidget>
#include "display/QtPlotter/SpecFitSettingsRangeWidget.ui.h"
class QDoubleValidator;
namespace casa {

class SpecFitSettingsRangeWidget : public QWidget
{
    Q_OBJECT

public:
    SpecFitSettingsRangeWidget(QWidget *parent = 0);
    ~SpecFitSettingsRangeWidget();
    void setCollapseRange( float min, float max );

private:
    QDoubleValidator* doubleValidator;
    Ui::SpecFitsettingsRangeWidgetClass ui;
};
}
#endif // SPECFITSETTINGSRANGEWIDGET_QO_H
