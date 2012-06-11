#ifndef SPECFITSETTINGSWIDGETRADIO_QO_H
#define SPECFITSETTINGSWIDGETRADIO_QO_H

#include <QtGui/QWidget>
#include <display/QtPlotter/SpecFitSettingsWidgetRadio.ui.h>
#include <display/QtPlotter/ProfileTaskFacilitator.h>

namespace casa {

class ImageProfileFitter;
class SpectralList;

class SpecFitSettingsWidgetRadio : public QWidget, public ProfileTaskFacilitator
{
    Q_OBJECT

public:
    SpecFitSettingsWidgetRadio(QWidget *parent = 0);
    ~SpecFitSettingsWidgetRadio();
    void setUnits( QString units );
    void setRange(float start, float end );
    void reset();


private slots:
	void polyFitChanged( int state );
	void adjustTableRowCount( int count );
	void clean();
	void specLineFit();
	void setOutputLogFile();
	void viewOutputLogFile();
	void saveOutputChanged( int state );
	void specFitEstimateSpecified(double xValue,double yValue, bool centerPeak);

private:
	/**
	 * Places the passed in Gaussian estimate val into the indicated row and
	 * column of the initial Gaussian estimate table.
	 */
	void setEstimateValue( int row, int col, double val );
	bool isValidEstimate( QString& peakStr, QString& centerStr,
			QString& fwhmStr, QString& fixedStr, int rowIndex );
	SpectralList buildSpectralList( int nGauss, Bool& validList );
	bool isValidFitSpecification( int gaussCount, bool polyFit );
	void setCanvas( QtCanvas* canvas );
	void doFit( float startVal, float endVal, uint gaussCount, bool fitPoly, int polyN );
	String getChannels( float startVal, float endVal, const Vector<Float>& specValues ) const;
	enum TableHeaders {PEAK,CENTER,FWHM,FIXED,END_COLUMN};
    Ui::SpecFitSettingsWidgetRadio ui;
    ImageProfileFitter* fitter;
    QString outputLogPath;
};
}
#endif // SPECFITSETTINGSWIDGETRADIO_H
