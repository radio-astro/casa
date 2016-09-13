#ifndef SPECTRALPOSITIONINGWIDGET_QO_H
#define SPECTRALPOSITIONINGWIDGET_QO_H

#include <QtGui/QWidget>
#include <display/QtPlotter/SpectralPositioningWidget.ui.h>
#include <casa/Arrays/Vector.h>
namespace casa {

	class ProfileTaskMonitor;
	class LogIO;

	class SpectralPositioningWidget : public QWidget {
		Q_OBJECT

	public:
		SpectralPositioningWidget(QWidget *parent = 0);
		void setTaskMonitor( ProfileTaskMonitor* monitor );
		void setLogger( LogIO* logger );

		void updateRegion( const Vector<Double> px, const Vector<Double> py,
		                   const Vector<Double> wx, const Vector<Double> wy );
		~SpectralPositioningWidget();

	private slots:
		void boxSpecChanged( int index );
		void locationSelectionTypeChanged( int index );
		void locationUnitsChanged( int index );
		void setPosition();

	private:
		void updateUI();
		void updateUIWorldBox();
		void updateUIWorldPoint();
		void updateUIPixelBox();
		void updateUIPixelPoint();
		/**
		  * Initializes the spectrum positioning tab.
		  */
		void initSpectrumPosition();
		void pageUpdate( int selectionIndex, int unitIndex );

		bool populateWorlds( const QList<int> &pixelX, const QList<int> &pixelY,
		                     QList<double> &worldX, QList<double> &worldY );
		bool fillPointWorld( QList<double> &worldX, QList<double> &worldY );
		void fillPointPixel( QList<int> &pixelX, QList<int>&pixelY )const;
		bool fillBoxPixel( QList<int> &pixelX, QList<int>&pixelY );
		bool fillBoxWorld( QList<double> &worldX, QList<double> & worldY );
		bool fillBasedOnBoxSpecification(  const double*  const firstXPix, const double * const firstYPix,
		                                   const double* const secondXPix, const double* const secondYPix,
		                                   double* const blcxPix, double* const blcyPix,
		                                   double* const trcxPix, double* const trcYPix, bool pixels=true );
		double toRadians( Bool& valid, QLineEdit * lineEdit );
		void switchBoxLabels( int index, int pageIndex, QLabel* const x1Label, QLabel* const y1Label,
		                      QLabel* const x2Label, QLabel* const y2Label );
		void setPixelLineEdits( double topLeft, double bottomLeft,
		                        double topRight, double bottomRight );
		void setWorldEdits( double topLeft, double bottomLeft,
		                    double topRight, double bottomRight );
		void adjustPoint( const Vector<Double>& newX, const Vector<Double>& newY,
		                  Vector<Double>& xValues, Vector<Double>& yValues );
		Ui::SpectralPositioningWidgetClass ui;

		enum PositionTypeIndex { POINT, BOX, END_POSITION_TYPE };
		enum UnitIndex {RADIAN, PIXEL, END_UNIT };
		QIntValidator* pixelValidator;
		enum StackPages { POINT_PIXEL, POINT_RA_DEC, BOX_PIXEL, BOX_RA_DEC };
		enum BoxSpecificationIndex { TL_LENGTH_HEIGHT, CENTER_LENGTH_HEIGHT, TL_BR, BL_TR,
		                             TL_LENGTH_HEIGHT_WORLD, CENTER_LENGTH_HEIGHT_WORLD, TL_BR_WORLD, BL_TR_WORLD, END_SPEC
		                           };
		QMap<BoxSpecificationIndex,QList<QString> > boxLabelMap;
		ProfileTaskMonitor* profileTaskMonitor;
		LogIO* logger;
		Vector<Double> pixelXValues;
		Vector<Double> pixelYValues;
		Vector<Double> worldXValues;
		Vector<Double> worldYValues;
	};
}
#endif // SPECTRALPOSITIONINGWIDGET_H
