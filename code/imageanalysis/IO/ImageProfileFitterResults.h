//# tSubImage.cc: Test program for class SubImage
//# Copyright (C) 1998,1999,2000,2001,2003
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This program is free software; you can redistribute it and/or modify it
//# under the terms of the GNU General Public License as published by the Free
//# Software Foundation; either version 2 of the License, or (at your option)
//# any later version.
//#
//# This program is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
//# more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with this program; if not, write to the Free Software Foundation, Inc.,
//# 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#


#ifndef IMAGEANALYSIS_IMAGEPROFILEFITTERRESULTS_H
#define IMAGEANALYSIS_IMAGEPROFILEFITTERRESULTS_H

#include <components/SpectralComponents/GaussianMultipletSpectralElement.h>
#include <imageanalysis/ImageAnalysis/ImageFit1D.h>
#include <images/Images/TempImage.h>

#include <casa/namespace.h>

#include <tr1/memory>

namespace casa {

class LogFile;

class ImageProfileFitterResults {
	// <summary>
	// </summary>

	// <reviewed reviewer="" date="" tests="" demos="">
	// </reviewed>

	// <prerequisite>
	// </prerequisite>

	// <etymology>
	// </etymology>

	// <synopsis>
	// </synopsis>

	// <example>
	// <srcblock>
	// </srcblock>
	// </example>

public:
	ImageProfileFitterResults(
		const std::tr1::shared_ptr<LogIO>& log, const CoordinateSystem& csysIm,
		const Array<ImageFit1D<Float> > * const &fitters, const SpectralList& nonPolyEstimates,
		const SubImage<Float> * const &subImage, Int fitAxis, Int polyOrder,
		uInt nGaussSinglets, uInt nGaussMultiplets, uInt nLorentzSinglets, uInt nPLPCoeffs,
		Bool logResults, Bool multiFit, const std::tr1::shared_ptr<LogFile>& logfile,
		const String& xUnit, const String& summaryHeader
	);

	~ImageProfileFitterResults();

	// create all the products. Should be run after the object has been entirely configured
	void createResults();

    // get the result Record
	Record getResults() const;

    inline String getClass() const { return _class; };

    // stream results to logger? This can be a lot of output.
    inline void setLogResults(const Bool logResults) { _logResults = logResults; }

    // <group>
    // gaussian amplitude image name
    inline void setAmpName(const String& s) { _ampName = s; }
    // gaussian amplitude error image name
    inline void setAmpErrName(const String& s) { _ampErrName = s; }
    // gaussian center image name
    inline void setCenterName(const String& s) { _centerName = s; }
    // gaussian center error image name
    inline void setCenterErrName(const String& s) { _centerErrName = s; }
    // gaussian fwhm image name
    inline void setFWHMName(const String& s) { _fwhmName = s; }
    // gaussian fwhm error image name
    inline void setFWHMErrName(const String& s) { _fwhmErrName = s; }
    // gaussian integral image name
    inline void setIntegralName(const String& s) { _integralName = s; }
    // gaussian integral error image name
    inline void setIntegralErrName(const String& s) { _integralErrName = s; }
    // </group>

    // set the name of the power logarithmic polynomial image. A separate image
    // representing each coefficient will be written. The c0 image will have
    // "_c0" appended to the name. The others will have "_alpha", "_beta", ...
    // appended to the name.
    inline void setPLPName(const String& s) { _plpName = s; }

    // set the name of the power logarithmic polynomial image. A separate image
    // representing each coefficient will be written. The c0 image will have
    // "_c0" appended to the name. The others will have "_alpha", "_beta", ...
    // appended to the name.
    inline void setPLPErrName(const String& s) { _plpErrName = s; }

    inline void setOutputSigmaImage(const String& s) { _sigmaName = s; }
    // </group>


    const static String _CONVERGED;
    const static String _SUCCEEDED;
    const static String _VALID;

    const Array<ImageFit1D<Float> >& getFitters() const;
    // Returns the center, in pixels of the indexth fit.
    const Vector<Double> getPixelCenter( uint index ) const;
    //Converts a pixel value into a world value either in velocity, wavelength, or
    //frequency units.
    Double getWorldValue(
    	double pixelVal, const IPosition& imPos, const String& units,
        bool velocity, bool wavelength
    ) const;

    void setPLPDivisor(const String& x) { _plpDivisor = x; }

private:
	enum gaussSols {
	    AMP, CENTER, FWHM, INTEGRAL, AMPERR, CENTERERR,
	    FWHMERR, INTEGRALERR, NGSOLMATRICES
	};

	enum plpSols {
		PLPSOL, PLPERR, NPLPSOLMATRICES
	};

	enum axisType {
		LONGITUDE, LATITUDE, FREQUENCY, POLARIZATION, NAXISTYPES
	};
    
    const static String _class;
    Bool _logResults, _multiFit;
   	String _xUnit, _centerName, _centerErrName, _fwhmName,
		_fwhmErrName, _ampName, _ampErrName,
		_integralName, _integralErrName, _plpName, _plpErrName, _sigmaName,
		_summaryHeader;
    uInt _nGaussSinglets, _nGaussMultiplets, _nLorentzSinglets,
		_nPLPCoeffs;
    const Array<ImageFit1D<Float> > * const  _fitters;
	SpectralList _nonPolyEstimates;
 // subimage contains the region of the original image
	// on which the fit is performed.
	const SubImage<Float> * const _subImage;
	Int _polyOrder, _fitAxis;
	vector<axisType> _axisTypes;
	Matrix<String> _worldCoords;
	Record _results;
	const static uInt _nOthers, _gsPlane, _lsPlane;
	std::tr1::shared_ptr<LogFile> _logfile;
	std::tr1::shared_ptr<LogIO> _log;
   	Vector<Double> _goodAmpRange, _goodCenterRange, _goodFWHMRange;
   	const CoordinateSystem _csysIm;
   	String _plpDivisor;

    void _setResults();

    void _resultsToLog();

    String _getTag(const uInt i) const;

    std::auto_ptr<vector<vector<Matrix<Double> > > > _createPCFMatrices() const;

    String _elementToString(
    	const Double value, const Double error,
    	const String& unit, Bool isFixed
    ) const;

    String _pcfToString(
    	const PCFSpectralElement *const &pcf, const CoordinateSystem& csys,
    	const Vector<Double> world, const IPosition imPos, const Bool showTypeString=True,
    	const String& indent=""
    ) const;

    String _gaussianMultipletToString(
    	const GaussianMultipletSpectralElement& gm,
    	const CoordinateSystem& csys, const Vector<Double> world,
    	const IPosition imPos
    ) const;

    Bool _setAxisTypes();

    String _polynomialToString(
    	const PolynomialSpectralElement& poly, const CoordinateSystem& csys,
    	const Vector<Double> imPix, const Vector<Double> world
    ) const;

    String _powerLogPolyToString(
    	const PowerLogPolynomialSpectralElement& plp
    ) const;

    void _marshalFitResults(
    	Array<Bool>& attemptedArr, Array<Bool>& successArr,
    	Array<Bool>& convergedArr, Array<Bool>& validArr,
    	Matrix<String>& typeMat, Array<Int>& niterArr,
    	Array<Int>& nCompArr, std::auto_ptr<vector<vector<Matrix<Double> > > >& pcfMatrices,
    	vector<Matrix<Double> >& plpMatrices, Bool returnDirection,
        Vector<String>& directionInfo, Array<Bool>& mask
    );

    static void _makeSolutionImage(
    	const String& name, const CoordinateSystem& csys,
    	const Array<Double>& values, const String& unit,
    	const Array<Bool>& mask
    );

    void _insertPCF(
    	vector<vector<Matrix<Double> > >& pcfMatrices, /* Bool& isSolutionSane,*/
    	const uInt idx, const PCFSpectralElement& pcf,
    	const uInt row, const uInt col, const IPosition& pos,
    	const Double increment/*, const uInt npix*/
    ) const;

    void _writeImages(
    	const CoordinateSystem& csys,
    	const Array<Bool>& mask, const String& yUnit
    ) const;

    Double _fitAxisIncrement() const;

    Double _centerWorld(
    	const PCFSpectralElement& solution, const IPosition& imPos
    ) const;

    Bool _inVelocitySpace() const;

    Vector< Vector<Double> > _pixelPositions;

    void _writeLogfile(const String& str, Bool open, Bool close);
};
}

#endif
