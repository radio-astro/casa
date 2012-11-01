#ifndef PREFERENCESCOLOR_QO_H
#define PREFERENCESCOLOR_QO_H

#include <QtGui/QDialog>
#include <QMap>
#include <QSettings>
#include <guitools/Feather/PreferencesColor.ui.h>

namespace casa {

class PreferencesColor : public QDialog
{
    Q_OBJECT

public:
    PreferencesColor(QWidget *parent = 0);
    enum FunctionColor {SD_WEIGHT_COLOR,SD_ORIGINAL_COLOR,SD_OUTPUT_COLOR,
    	INT_WEIGHT_COLOR,INT_ORIGINAL_COLOR, INT_OUTPUT_COLOR, END_COLOR };
    QMap<FunctionColor,QColor> getFunctionColors() const;
    QColor getScatterPlotColor() const;
    ~PreferencesColor();

signals:
	void colorsChanged();

private slots:
	void selectSDWeightColor();
	void selectSDOriginalColor();
	void selectSDOutputColor();
	void selectINTWeightColor();
	void selectINTOriginalColor();
	void selectINTOutputColor();
	void selectScatterPlotColor();
	void colorsAccepted();
	void colorsRejected();

private:
	void initializeUserColors();
	void storeCustomColor( QSettings& settings, FunctionColor index );
	void storeMapColor( QPushButton* button, FunctionColor colorType );
	QString readCustomColor( QSettings& settings, int index);
    QColor getButtonColor( QPushButton* button ) const;
    void setButtonColor( QPushButton* button, QColor color );
    void showColorDialog( QPushButton* source );
    void resetColors();
    void persistColors();

    Ui::PreferencesColorClass ui;
    QMap<FunctionColor,QColor> colorMap;
    QColor scatterPlotColor;
    const int SCATTER_INDEX;

    static const QString FUNCTION_COLOR;
};
}

#endif // PREFERENCESCOLOR_QO_H
