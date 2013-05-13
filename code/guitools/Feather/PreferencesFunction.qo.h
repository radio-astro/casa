#ifndef PREFERENCESFUNCTION_QO_H
#define PREFERENCESFUNCTION_QO_H

#include <QtGui/QWidget>
#include <QSettings>
#include <guitools/Feather/PreferencesFunction.ui.h>

namespace casa {

class PreferencesFunction : public QWidget
{
    Q_OBJECT

public:
    PreferencesFunction(int index, QWidget *parent = 0);
    ~PreferencesFunction();
    QColor getColor() const;
    void setColor( QColor color );
    QString readCustomColor( QSettings& settings, const QString& baseLookup);
    void storeCustomColor( QSettings& settings, const QString& baseLookup );
    //Transferring the color back and forth from the 'official'
    //defaultColor and that shown on the button.
    void storeColor();
    void resetColor();

private slots:
	void visibilityChanged();
	void showColorDialog();
private:
	int id;
	QColor defaultColor;
	void setButtonColor( QColor color );

	QColor getButtonColor() const;
    Ui::PreferencesFunctionClass ui;
};

}

#endif // PREFERENCESFUNCTION_QO_H
