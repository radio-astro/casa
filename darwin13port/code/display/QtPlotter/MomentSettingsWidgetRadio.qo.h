//# Copyright (C) 2005
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
#ifndef MOMENTSETTINGSWIDGETRADIO_QO_H
#define MOMENTSETTINGSWIDGETRADIO_QO_H

#include <QtGui/QWidget>
#include <QMap>
#include <QThread>
#include <QProgressDialog>
#include <casa/Quanta/Quantum.h>
#include <display/QtPlotter/ProfileTaskFacilitator.h>
#include <display/QtPlotter/MomentSettingsWidgetRadio.ui.h>
#include <imageanalysis/ImageAnalysis/ImageMomentsProgressMonitor.h>
#include <images/Images/ImageInterface.h>

namespace casa {

	class ImageAnalysis;
	class MomentCollapseThreadRadio;
	class ThresholdingBinPlotDialog;
	class Converter;
	template <class T> class ImageInterface;

	class CollapseResult {

	public:
		CollapseResult( const String& outputName, bool tmp, ImageInterface<Float>* img ):
			outputFileName(outputName),
			temporary( tmp ),
			image(img) {}
		String getOutputFileName() const {
			return outputFileName;
		}
		bool isTemporaryOutput() const {
			return temporary;
		}
		std::tr1::shared_ptr<ImageInterface<Float> > getImage() const {
			return image;
		}

	private:
		String outputFileName;
		bool temporary;

		std::tr1::shared_ptr<ImageInterface<Float> > image;
	};


//Note:  ImageMomentsProgressMonitor is an interface that provides this class
//with updates concerning the progress of the moment calculation task.

	/**
	 * Responsible for running the collapse algorithm in
	 * the background so that we don't freeze the GUI.
	 */
	class MomentCollapseThreadRadio : public QThread, public ImageMomentsProgressMonitor {
		Q_OBJECT
	public:
		MomentCollapseThreadRadio( ImageAnalysis* imageAnalysis );
		bool isSuccess() const;
		void setChannelStr( String str );
		void setMomentNames( const Vector<QString>& momentNames );
		void setOutputFileName( QString name );
		String getErrorMessage() const;
		std::vector<CollapseResult> getResults() const;
		void setData(const Vector<Int>& moments, const Int axis, Record& region,
		             const String& mask, const Vector<String>& method,
		             const Vector<Int>& smoothaxes,
		             const Vector<String>& smoothtypes,
		             const Vector<Quantity>& smoothwidths,
		             const Vector<Float>& includepix,
		             const Vector<Float>& excludepix,
		             const Double peaksnr, const Double stddev,
		             const String& doppler = "RADIO", const String& baseName = "");
		void run();
		void halt();
		//Methods from the ImageMomentsProgressMonitor interface
		void setStepCount( int count );
		void setStepsCompleted( int count );
		void done();
		~MomentCollapseThreadRadio();

	signals:
		void stepCountChanged( int count );
		void stepsCompletedChanged( int count );

	private:
		bool getOutputFileName( String& outName, int moment, const String& channelStr ) const;
		ImageAnalysis* analysis;
		Vector<Int> moments;
		Vector<QString> momentNames;
		Int axis;
		Record region;
		String mask;
		String channelStr;
		Vector<String> method;
		Vector<Int> smoothaxes;
		Vector<String> smoothtypes;
		Vector<Quantity> smoothwidths;
		Vector<Float> includepix;
		Vector<Float> excludepix;
		Double peaksnr;
		Double stddev;
		String doppler;
		String baseName;
		QString outputFileName;
		int stepSize;
		std::vector<CollapseResult> collapseResults;
		String errorMsg;
		bool collapseError;
		bool stopImmediately;
	};

//Note: ProfileTaskFacilitator abstracts out some of the common functionality
//needed for calculating moments and spectral line fitting into a single
//base class


	class MomentSettingsWidgetRadio : public QWidget, public ProfileTaskFacilitator {
		Q_OBJECT

	public:
		MomentSettingsWidgetRadio(QWidget *parent = 0);

		void setUnits( QString units );
		void setRange( double min, double max );
		void reset();
		~MomentSettingsWidgetRadio();

	signals:
		void updateProgress(int);
		void momentsFinished();

	private slots:
		void setStepCount( int count );
		void setStepsCompleted( int count );
		void thresholdingChanged();
		void thresholdSpecified();
		void adjustTableRows( int count );
		void collapseImage();
		void setCollapsedImageFile();
		void collapseDone();
		void graphicalThreshold();
		void symmetricThresholdChanged( int checkedState );
		void thresholdTextChanged( const QString& text );
		void stopMoments();

	private:
		enum SummationIndex {MEAN, INTEGRATED, WEIGHTED_MEAN, DISPERSION, MEDIAN,
		                     MEDIAN_VELOCITY, STDDEV,  RMS, ABS_MEAN_DEV, MAX, MAX_VELOCITY, MIN,
		                     MIN_VELOCITY, END_INDEX
		                    };
		QMap<SummationIndex, int> momentMap;
		Ui::MomentSettingsWidgetRadio ui;
		ImageAnalysis* imageAnalysis;
		MomentCollapseThreadRadio* collapseThread;
		ThresholdingBinPlotDialog* thresholdingBinDialog;
		QString outputFileName;
		QList<QString> momentOptions;
		QProgressDialog progressBar;

		//Progress Monitor functionality
		int momentCount;
		int cycleCount;
		int baseIncrement;
		int previousCount;

		void setTableValue(int row, int col, float val );
		void getChannelMinMax( int channelIndex, QString& minStr, QString& maxStr ) const;
		void convertChannelRanges( const QString& oldUnits, const QString& newUnits );
		void convertChannelValue( const QString& channelStr, const QString& channelIdentifier,
		                          Converter* converter, int row, int col, bool toPixels );
		String makeChannelInterval( float startChannelIndex,float endChannelIndex ) const;
		Vector<Int> populateMoments( Vector<QString>& momentNames );
		Vector<String> populateMethod() const;
		String populateChannels(uInt * nSelectedChannels);
		bool populateThresholds( Vector<Float>& includeThreshold, Vector<Float>& excludeThreshold );
		bool populateThreshold( Vector<Float>& threshold );
	};

}

#endif // MOMENTSETTINGSWIDGETRADIO_QO_H
