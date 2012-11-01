#ifndef PREFERENCES_QO_H
#define PREFERENCES_QO_H

#include <QtGui/QDialog>
#include <guitools/Feather/Preferences.ui.h>

namespace casa {

class Preferences : public QDialog
{
    Q_OBJECT

public:
    const static QString ORGANIZATION;
    const static QString APPLICATION;
    Preferences(QWidget *parent = 0);
    bool isDisplayOriginalFunctions() const;
    bool isDisplayOriginalLegend() const;
    bool isDisplayOutputFunctions() const;
    bool isDisplayOutputLegend() const;
    int getLineThickness() const;
    ~Preferences();


signals:
	void preferencesChanged();

private slots:
	void preferencesAccepted();
	void preferencesRejected();
	void originalPlotVisibilityChanged( int checked );
	void outputPlotVisibilityChanged( int checked );

private:
	void initializeCustomSettings();
	void persist();
	void reset();

	const static QString LINE_THICKNESS;
	const static QString DISPLAY_ORIGINAL_FUNCTIONS;
	const static QString DISPLAY_ORIGINAL_LEGEND;
	const static QString DISPLAY_OUTPUT_FUNCTIONS;
	const static QString DISPLAY_OUTPUT_LEGEND;

    Ui::PreferencesClass ui;
    int lineThickness;
    bool displayOriginalFunctions;
    bool displayOriginalLegend;
    bool displayOutputFunctions;
    bool displayOutputLegend;
};

}

#endif // PREFERENCES_H
