#ifndef PREFERENCESFUNCTION_QO_H
#define PREFERENCESFUNCTION_QO_H

#include <QtGui/QWidget>
#include <QSettings>
#include <guitools/Feather/PreferencesFunction.ui.h>
#include <guitools/Feather/CurveDisplay.h>

namespace casa {

/**
 * Displays, handles persistence, and stores display properties for a
 * particular curve that can be shown on the plot.
 */

class PreferencesFunction : public QWidget
{
    Q_OBJECT

public:
    PreferencesFunction(int index, QWidget *parent = 0);
    ~PreferencesFunction();

    //Initialization of the default color
    void setColor( QColor color );
    void setDisplayed( bool visible );
    void setScatterEligible( bool eligible );
    void setDisplayHidden();
    void setName( const QString& name );


    //Getters
    /**
     * Whether it is displayed from the permanent, persistent
     * settings.
     */
    bool isDisplayed() const;
    /**
     * Displayed in the GUI.  The GUI state will not be persisted
     * until the "Apply" button is pressed.
     */
    bool isDisplayedGUI() const;
    /**
     * Can this curve be included in the scatter plot
     */
    bool isScatterEligible() const;
    QString getName() const;

    //Initializes the defaults from user settings.
    void initialize( QSettings& settings);

    //Writes what is currently displayed into the user settings.
    void persist( QSettings& settings );

    //Copies the defaults into what is displayed.
    void reset();

    const CurveDisplay getFunctionPreferences() const;

signals:
	void displayStatusChanged();

private slots:
	void visibilityChanged();
	void showColorDialog();
private:
	int id;
	bool scatterEligible;
	CurveDisplay curveSettings;
	QString getBaseStorageId() const;
	void setButtonColor( QColor color );
	const QString COLOR_KEY;
	const QString VISIBILITY_KEY;
	QColor getButtonColor() const;
	static const QString FUNCTION_COLOR;
    Ui::PreferencesFunctionClass ui;
};

}

#endif // PREFERENCESFUNCTION_QO_H
