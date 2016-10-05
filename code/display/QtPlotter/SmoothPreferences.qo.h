#ifndef SMOOTHPREFERENCES_QO_H
#define SMOOTHPREFERENCES_QO_H

#include <QtGui/QDialog>
#include <casa/Arrays/Vector.h>
#include <display/QtPlotter/SmoothPreferences.ui.h>
#include <scimath/Mathematics/VectorKernel.h>
namespace casa {

class SmoothSpinBox;

class SmoothPreferences : public QDialog
{
    Q_OBJECT

public:
    SmoothPreferences(QWidget *parent = 0);
    casacore::Vector<float> applySmoothing( casacore::Vector<float> values) const;
    QString toString() const;
    ~SmoothPreferences();

signals:
	void smoothingChanged();

private slots:
	void smoothingAccepted();
	void smoothingRejected();
	void smoothingMethodChanged();

private:
    void initialize();
    void persist();
    void reset();
    casacore::Vector<float> doConvolve( const casacore::Vector<float>& input,
    		int baseKernelSize, casacore::VectorKernel::KernelTypes kernelType ) const;
    Ui::SmoothPreferencesClass ui;
    QString smoothMethod;
    int smoothRadius;
    SmoothSpinBox* smoothSpinBox;
    const QString SMOOTH_NONE;
    const QString SMOOTH_BOXCAR;
    const QString SMOOTH_HANNING;
    const static QString APPLICATION;
    const static QString ORGANIZATION;
    const static QString SMOOTH_METHOD;
    const static QString SMOOTH_RADIUS;

};
}
#endif // SMOOTHPREFERENCES_QO_H
