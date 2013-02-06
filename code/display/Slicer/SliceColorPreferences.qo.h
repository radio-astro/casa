#ifndef SLICECOLORPREFERENCES_QO_H
#define SLICECOLORPREFERENCES_QO_H

#include <QtGui/QDialog>
#include <QSettings>
#include <display/Slicer/SliceColorPreferences.ui.h>

namespace casa {

class SliceColorPreferences : public QDialog
{
    Q_OBJECT

public:
    SliceColorPreferences(QWidget *parent = 0);
    QColor getSliceColor() const;
    ~SliceColorPreferences();

signals:
	void colorsChanged();

private slots:
	void resetColors();
	void selectSliceColor();
	void colorsAccepted();
	void colorsRejected();

private:
	void persistColors();
	void showColorDialog( QPushButton* source );
    void initializeUserColors();
    QColor getButtonColor( QPushButton* button ) const;
    void setButtonColor( QPushButton* button, QColor color );
    QString readCustomColor( QSettings& settings,
    		const QString& identifier, const QString& defaultColor);
    static const QString APPLICATION;
    static const QString ORGANIZATION;
    static const QString SLICE_COLOR;
    QColor sliceColor;
    Ui::SliceColorPreferencesClass ui;
};

}
#endif // SLICECOLORPREFERENCES_QO_H
