#ifndef SPECFITSETTINGSFIXEDTABLECELL_QO_H
#define SPECFITSETTINGSFIXEDTABLECELL_QO_H

#include <QtGui/QWidget>
#include <display/QtPlotter/SpecFitSettingsFixedTableCell.ui.h>

class SpecFitSettingsFixedTableCell : public QWidget
{
    Q_OBJECT

public:
    SpecFitSettingsFixedTableCell(QWidget *parent = 0);
    ~SpecFitSettingsFixedTableCell();
    QString getFixedStr() const;
    QSize sizeHint() const;

private:
    Ui::SpecFitSettingsFixedTableCell ui;
    const QString GAUSSIAN_ESTIMATE_FIXED_CENTER;
    const QString GAUSSIAN_ESTIMATE_FIXED_PEAK;
    const QString GAUSSIAN_ESTIMATE_FIXED_FWHM;
};

#endif // SPECFITSETTINGSFIXEDTABLECELL_H
