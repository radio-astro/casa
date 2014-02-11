//# PlotMSPlotParameterGroups.h: Implementations of plot subparameter groups.
//# Copyright (C) 2009
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
//# $Id: $
#ifndef PLOTMSPLOTPARAMETERGROUPS_H_
#define PLOTMSPLOTPARAMETERGROUPS_H_

#include <plotms/Plots/PlotMSPlotParameters.h>

#include <plotms/PlotMS/PlotMSAveraging.h>
#include <plotms/PlotMS/PlotMSExportParam.h>
#include <plotms/PlotMS/PlotMSIterParam.h>
#include <plotms/PlotMS/PlotMSSelection.h>
#include <plotms/PlotMS/PlotMSTransformations.h>
#include <plotms/PlotMS/PlotMSLabelFormat.h>

#include <casa/namespace.h>

namespace casa {

// Container class to hold constants for groups.
class PMS_PP {

public:
	// Update flag for redrawing.
	// <group>
	static const String UPDATE_REDRAW_NAME;
	static const int UPDATE_REDRAW;
	// </group>

	// Update flag for MS data group.
	// <group>
	static const String UPDATE_MSDATA_NAME;
	static const int UPDATE_MSDATA;
	// </group>

	// Update flag for cache group.
	// <group>
	static const String UPDATE_CACHE_NAME;
	static const int UPDATE_CACHE;
	// </group>

	// Update flag for axes group.
	// <group>
	static const String UPDATE_AXES_NAME;
	static const int UPDATE_AXES;
	// </group>

	// Update flag for canvas group.
	// <group>
	static const String UPDATE_CANVAS_NAME;
	static const int UPDATE_CANVAS;
	// </group>

	// Update flag for display group.
	// <group>
	static const String UPDATE_DISPLAY_NAME;
	static const int UPDATE_DISPLAY;
	// </group>

	// Update flag for iteration group.
	// <group>
	static const String UPDATE_ITERATION_NAME;
	static const int UPDATE_ITERATION;
	// </group>

	// Update flag for log group.
	// <group>
	static const String UPDATE_LOG_NAME;
	static const int UPDATE_LOG;
	// </group>
	//
	// Update flag for plotms_options group.
	// <group>
	static const String UPDATE_PLOTMS_OPTIONS_NAME;
	static const int UPDATE_PLOTMS_OPTIONS;
	// </group>

private:
	// Disable constructor.
	PMS_PP() {
	}
};




// Subclass of PlotMSPlotParameters::Group to handle subparameters for MS data.
// Currently includes:
// * filename
// * selection
// * averaging
//
class PMS_PP_MSData : public PlotMSPlotParameters::Group {

public:
	/* Constructor which takes a factory */
	PMS_PP_MSData (PlotFactoryPtr factory);

	/* Copy constructor.  See operator=(). */
	PMS_PP_MSData (const PMS_PP_MSData & copy);
	~PMS_PP_MSData();


	/* Overrides PlotMSPlotParameters::Group::operator=(). */
	Group *clone() const {
		return new PMS_PP_MSData (*this);
	}

	/* Implements PlotMSPlotParameters::Group::name(). */
	const String & name() const {
		static String groupName = PMS_PP::UPDATE_MSDATA_NAME;
		return groupName;
	}

	/* Implements PlotMSPlotParameters::Group::toRecord(). */
	Record toRecord() const;

	/* Implements PlotMSPlotParameters::Group::fromRecord(). */
	void fromRecord (const Record & record);

	/* Implements PlotMSPlotParameters::Group::requiresRedrawOnChanged(). */
	bool requiresRedrawOnChange() const {
		return true;
	}

	/* Overrides PlotMSPlotParameters::Group::operator=(). */
	Group & operator= (const Group & other);

	/* Overrides PlotMSPlotParameters::Group::operator==(). */
	bool operator== (const Group & other) const;


	bool isSet() const {
		return !itsFilename_.empty();
	}

	const String & filename() const {
		return itsFilename_;
	}
	void setFilename (const String & value) {
		if (itsFilename_ != value) {
			itsFilename_ = value;
			updated();
		}
	}


	const PlotMSSelection & selection() const {
		return itsSelection_;
	}
	void setSelection (const PlotMSSelection & value) {
		if (itsSelection_ != value) {
			itsSelection_ = value;
			updated();
		}
	}


	const PlotMSAveraging & averaging() const {
		return itsAveraging_;
	}
	void setAveraging (const PlotMSAveraging & value) {
		if (itsAveraging_ != value) {
			itsAveraging_ = value;
			updated();
		}
	}


	const PlotMSTransformations & transformations() const {
		return itsTransformations_;
	}
	void setTransformations (const PlotMSTransformations & value) {
		if (itsTransformations_ != value) {
			itsTransformations_ = value;
			updated();
		}
	}

private:
	/* Parameters' values */
	PlotMSSelection itsSelection_;
	String itsFilename_;
	PlotMSAveraging itsAveraging_;
	PlotMSTransformations itsTransformations_;

	/* Key strings for Record */
	static const String REC_SELECTION;
	static const String REC_FILENAME;
	static const String REC_AVERAGING;
	static const String REC_TRANSFORMATIONS;


	void setDefaults();
};






// Subclass of PlotMSPlotParameters::Group to handle cache parameters.
// Currently includes:
// * x and y axes
// * x and y data columns
// Parameters are vector-based, on a per-plot basis.
//
class PMS_PP_Cache : public PlotMSPlotParameters::Group {

public:
	/* Constructor which takes a factory */
	PMS_PP_Cache (PlotFactoryPtr factory);

	/* Copy constructor.  See operator=(). */
	PMS_PP_Cache (const PMS_PP_Cache & copy);

	~PMS_PP_Cache();

	/* Implements PlotMSPlotParameters::Group::clone(). */
	Group *clone() const {
		return new PMS_PP_Cache (*this);
	}

	/* Implements PlotMSPlotParameters::Group::name(). */
	const String & name() const {
		static String groupName = PMS_PP::UPDATE_CACHE_NAME;
		return groupName;
	}

	/* Implements PlotMSPlotParameters::Group::toRecord(). */
	Record toRecord() const;

	/* Implements PlotMSPlotParameters::Group::fromRecord(). */
	void fromRecord (const Record & record);

	/* Implements PlotMSPlotParameters::Group::requiresRedrawOnChanged(). */
	bool requiresRedrawOnChange() const {
		return true;
	}

	/* Overrides PlotMSPlotParameters::Group::operator=(). */
	Group & operator= (const Group & other);

	/* Overrides PlotMSPlotParameters::Group::operator==(). */
	bool operator== (const Group & other) const;



	// Gets how many axes and data columns there are.
	// <group>
	unsigned int numXAxes() const;
	unsigned int numYAxes() const;
	// </group>


	void setXAxis (const PMS::Axis & axis, const PMS::DataColumn & data,
			unsigned int index = 0) {
		setAxes (axis, yAxis (index), data, yDataColumn (index), index);
	}
	void setYAxis (const PMS::Axis & axis, const PMS::DataColumn & data,
			unsigned int index = 0) {
		setAxes (xAxis (index), axis, xDataColumn (index), data, index);
	}
	void setAxes (const PMS::Axis & xAxis, const PMS::Axis & yAxis,
			const PMS::DataColumn & xData,
			const PMS::DataColumn & yData, unsigned int index = 0);


	const vector<PMS::Axis> &xAxes() const {
		return itsXAxes_;
	}
	void setXAxes (const vector<PMS::Axis> &value) {
		if (itsXAxes_ != value) {
			itsXAxes_ = value;
			updated();
		}
	}
	PMS::Axis xAxis (unsigned int index = 0) const {
		if (index >= itsXAxes_.size())
			const_cast< vector<PMS::Axis>& >(itsXAxes_).resize (index + 1);
		return itsXAxes_[index];
	}
	void setXAxis (const PMS::Axis & value, unsigned int index = 0) {
		if (index >= itsXAxes_.size())
			itsXAxes_.resize (index + 1);
		if (itsXAxes_[index] != value) {
			itsXAxes_[index] = value;
			updated();
		}
	}


	const vector<PMS::Axis>&yAxes() const {
		return itsYAxes_;
	}
	void setYAxes (const vector<PMS::Axis> &value) {
		if (itsYAxes_ != value) {
			itsYAxes_ = value;
			updated();
		}
	}
	PMS::Axis yAxis (unsigned int index = 0) const {
		if (index >= itsYAxes_.size())
			const_cast< vector<PMS::Axis> &>(itsYAxes_).resize (index + 1);
		return itsYAxes_[index];
	}
	void setYAxis (const PMS::Axis & value, unsigned int index = 0) {
		if (index >= itsYAxes_.size())
			itsYAxes_.resize (index + 1);
		if (itsYAxes_[index] != value)   {
			itsYAxes_[index] = value;
			updated();
		}
	}


	const vector<PMS::DataColumn> &xDataColumns() const {
		return itsXData_;
	}
	void setXDataColumns (const vector < PMS::DataColumn > &value) {
		if (itsXData_ != value)   {
			itsXData_ = value;
			updated();
		}
	}
	PMS::DataColumn xDataColumn (unsigned int index = 0) const {
		if (index >= itsXData_.size())
			const_cast < vector < PMS::DataColumn >
		&>(itsXData_).resize (index + 1);
		return itsXData_[index];
	}
	void setXDataColumn (const PMS::DataColumn & value, unsigned int index =
			0) {
		if (index >= itsXData_.size())
			itsXData_.resize (index + 1);
		if (itsXData_[index] != value)   {
			itsXData_[index] = value;
			updated();
		}
	}


	const vector < PMS::DataColumn > &yDataColumns() const {
		return itsYData_;
	}
	void setYDataColumns (const vector < PMS::DataColumn > &value) {
		if (itsYData_ != value) {
			itsYData_ = value;
			updated();
		}
	}
	PMS::DataColumn yDataColumn (unsigned int index = 0) const {
		if (index >= itsYData_.size())
			const_cast < vector < PMS::DataColumn >
		&>(itsYData_).resize (index + 1);
		return itsYData_[index];
	}
	void setYDataColumn (const PMS::DataColumn & value, unsigned int index =
			0) {
		if (index >= itsYData_.size())
			itsYData_.resize (index + 1);
		if (itsYData_[index] != value)   {
			itsYData_[index] = value;
			updated();
		}
	}


private:
	/* Parameters' values */
	vector<PMS::Axis> itsXAxes_;
	vector<PMS::Axis> itsYAxes_;
	vector<PMS::DataColumn> itsXData_;
	vector<PMS::DataColumn> itsYData_;

	/* Key strings for Record */
	static const String REC_XAXES;
	static const String REC_YAXES;
	static const String REC_XDATACOLS;
	static const String REC_YDATACOLS;


	void setDefaults();
};






// Subclass of PlotMSPlotParameters::Group to handle axes parameters.
// Currently includes:
// * canvas attach axes
// * axes ranges, if any
// Parameters are vector-based, on a per-plot basis.
//
class PMS_PP_Axes : public PlotMSPlotParameters::Group {

public:
	/* Constructor which takes a factory */
	PMS_PP_Axes (PlotFactoryPtr factory);

	/* Copy constructor.  See operator=(). */
	PMS_PP_Axes (const PMS_PP_Axes & copy);

	~PMS_PP_Axes();


	/* Implements PlotMSPlotParameters::Group::clone(). */
	Group *clone() const {
		return new PMS_PP_Axes (*this);
	}

	/* Implements PlotMSPlotParameters::Group::name(). */
	const String & name() const {
		static String groupName = PMS_PP::UPDATE_AXES_NAME;
		return groupName;
	}

	/* Implements PlotMSPlotParameters::Group::toRecord(). */
	Record toRecord() const;

	/* Implements PlotMSPlotParameters::Group::fromRecord(). */
	void fromRecord (const Record & record);

	/* Implements PlotMSPlotParameters::Group::requiresRedrawOnChanged(). */
	bool requiresRedrawOnChange() const {
		return true;
	}

	/* Overrides PlotMSPlotParameters::Group::operator=(). */
	Group & operator= (const Group & other);

	/* Overrides PlotMSPlotParameters::Group::operator==(). */
	bool operator== (const Group & other) const;


	// Gets how many axes there are.
	// <group>
	unsigned int numXAxes() const;
	unsigned int numYAxes() const;
	// </group>


	// Sets single versions of the parameters for the given index.
	// <group>
	void setAxes (const PlotAxis & xAxis, const PlotAxis & yAxis,
			unsigned int index = 0);
	void setXRange (const bool & set, const prange_t & range,
			unsigned int index = 0) {
		setRanges (set, yRangeSet (index), range, yRange (index), index);
	}
	void setYRange (const bool & set, const prange_t & range,
			unsigned int index = 0) {
		setRanges (xRangeSet (index), set, xRange (index), range, index);
	}
	void setRanges (const bool & xSet, const bool & ySet,
			const prange_t & xRange, const prange_t & yRange,
			unsigned int index = 0);
	// </group>


	const vector < PlotAxis > &xAxes() const {
		return itsXAxes_;
	}
	void setXAxes (const vector < PlotAxis > &value) {
		if (itsXAxes_ != value) {
			itsXAxes_ = value;
			updated();
		}
	}


	PlotAxis xAxis (unsigned int index = 0) const {
		if (index >= itsXAxes_.size())
			const_cast < vector < PlotAxis > &>(itsXAxes_).resize (index + 1);
		return itsXAxes_[index];
	}
	void setXAxis(const PlotAxis & value, unsigned int index = 0) {
		if (index >= itsXAxes_.size())
			itsXAxes_.resize (index + 1);
		if (itsXAxes_[index] != value) {
			itsXAxes_[index] = value;
			updated();
		}
	}


	const vector<PlotAxis> &yAxes() const {
		return itsYAxes_;
	}
	void setYAxes (const vector < PlotAxis > &value) {
		if (itsYAxes_ != value) {
			itsYAxes_ = value;
			updated();
		}
	}


	PlotAxis yAxis (unsigned int index = 0) const {
		if (index >= itsYAxes_.size())
			const_cast < vector < PlotAxis > &>(itsYAxes_).resize (index + 1);
		return itsYAxes_[index];
	}
	void setYAxis (const PlotAxis & value, unsigned int index = 0) {
		if (index >= itsYAxes_.size())
			itsYAxes_.resize (index + 1);
		if (itsYAxes_[index] != value) {
			itsYAxes_[index] = value;
			updated();
		}
	}


	const vector<bool> &xRangesSet() const {
		return itsXRangesSet_;
	}
	void setXRanges (const vector < bool > &value) {
		if (itsXRangesSet_ != value) {
			itsXRangesSet_ = value;
			updated();
		}
	}


	bool xRangeSet (unsigned int index = 0) const {
		if (index >= itsXRangesSet_.size())
			const_cast < vector < bool > &>(itsXRangesSet_).resize (index + 1);
		return itsXRangesSet_[index];
	}
	void setXRange (const bool & value, unsigned int index = 0) {
		if (index >= itsXRangesSet_.size())
			itsXRangesSet_.resize (index + 1);
		if (itsXRangesSet_[index] != value) {
			itsXRangesSet_[index] = value;
			updated();
		}
	}


	const vector < bool > &yRangesSet() const {
		return itsYRangesSet_;
	}
	void setYRanges (const vector < bool > &value) {
		if (itsYRangesSet_ != value) {
			itsYRangesSet_ = value;
			updated();
		}
	}


	bool yRangeSet (unsigned int index = 0) const {
		if (index >= itsYRangesSet_.size())
			const_cast < vector < bool > &>(itsYRangesSet_).resize (index + 1);
		return itsYRangesSet_[index];
	}
	void setYRange (const bool & value, unsigned int index = 0) {
		if (index >= itsYRangesSet_.size())
			itsYRangesSet_.resize (index + 1);
		if (itsYRangesSet_[index] != value) {
			itsYRangesSet_[index] = value;
			updated();
		}
	}



	const vector < prange_t > &xRanges() const {
		return itsXRanges_;
	}
	void setXRanges (const vector < prange_t > &value) {
		if (itsXRanges_ != value) {
			itsXRanges_ = value;
			updated();
		}
	}


	const prange_t & xRange (unsigned int index = 0) const {
		return itsXRanges_[index];
	}
	void setXRange (const prange_t & value, unsigned int index = 0) {
		if (itsXRanges_[index] != value) {
			itsXRanges_[index] = value;
			updated();
		}
	}


	const vector < prange_t > &yRanges() const {
		return itsYRanges_;
	}
	void setYRanges (const vector < prange_t > &value) {
		if (itsYRanges_ != value) {
			itsYRanges_ = value;
			updated();
		}
	}


	const prange_t & yRange (unsigned int index = 0) const {
		return itsYRanges_[index];
	}
	void setYRange (const prange_t & value, unsigned int index = 0) {
		if (itsYRanges_[index] != value) {
			itsYRanges_[index] = value;
			updated();
		}
	}

private:
	/* Parameters' values */
	vector<PlotAxis> itsXAxes_;
	vector<PlotAxis> itsYAxes_;
	vector<bool> itsXRangesSet_;
	vector<bool> itsYRangesSet_;
	vector<prange_t> itsXRanges_;
	vector<prange_t> itsYRanges_;

	/* Key strings for Record */
	static const String REC_XAXES;
	static const String REC_YAXES;
	static const String REC_XRANGESSET;
	static const String REC_YRANGESSET;
	static const String REC_XRANGES;
	static const String REC_YRANGES;


	void setDefaults();
};





// Subclass of PlotMSPlotParameters::Group to handle canvas parameters.
// Currently includes:
// * axes label formats
// * whether to show the canvas axes or not
// * whether to show the legend or not, and its position
// * canvas title label format
// * whether to show grid lines, and their properties
// Parameters are vector-based, on a per-canvas basis.
//
class PMS_PP_Canvas : public PlotMSPlotParameters::Group {

public:
	/* Constructor which takes a factory */
	PMS_PP_Canvas (PlotFactoryPtr factory);

	/* Copy constructor.  See operator=(). */
	PMS_PP_Canvas (const PMS_PP_Canvas & copy);

	~PMS_PP_Canvas();

	/* Implements PlotMSPlotParameters::Group::clone(). */
	Group *clone() const {
		return new PMS_PP_Canvas (*this);
	}

	/* Implements PlotMSPlotParameters::Group::name(). */
	const String & name() const {
		static String groupName = PMS_PP::UPDATE_CANVAS_NAME;
		return groupName;
	}

	/* Implements PlotMSPlotParameters::Group::toRecord(). */
	Record toRecord() const;

	/* Implements PlotMSPlotParameters::Group::fromRecord(). */
	void fromRecord (const Record & record);

	/* Implements PlotMSPlotParameters::Group::requiresRedrawOnChanged(). */
	bool requiresRedrawOnChange() const {
		return true;
	}

	/* Overrides PlotMSPlotParameters::Group::operator=(). */
	Group & operator= (const Group & other);

	/* Overrides PlotMSPlotParameters::Group::operator==(). */
	bool operator== (const Group & other) const;


	// Gets how many canvases there are.
	unsigned int numCanvases() const;

	// Sets single versions of the parameters for the given index.
	// <group>
	void setLabelFormats (const PlotMSLabelFormat & xFormat,
			const PlotMSLabelFormat & yFormat,
			unsigned int index = 0);
	void showAxes (const bool & xShow, const bool & yShow,
			unsigned int index = 0);
	void showLegend (const bool & show,
			const PlotCanvas::LegendPosition & pos,
			unsigned int index = 0);

	void showGridMajor (const bool & show, const PlotLinePtr & line,
			unsigned int index = 0) {
		showGrid (show, gridMinorShown (index), line, gridMinorLine (index),
				index);
	}
	void showGridMinor (const bool & show, const PlotLinePtr & line,
			unsigned int index = 0) {
		showGrid (gridMajorShown (index), show, gridMajorLine (index), line,
				index);
	}
	void showGrid (const bool & showMajor, const bool & showMinor,
			const PlotLinePtr & majorLine,
			const PlotLinePtr & minorLine, unsigned int index = 0);
	// </group>



	const vector < PlotMSLabelFormat > &xLabelFormats() const {
		return itsXLabels_;
	}
	void setXLabelFormats (const vector < PlotMSLabelFormat > &value) {
		if (itsXLabels_ != value) {
			itsXLabels_ = value;
			updated();
		}
	}
	const PlotMSLabelFormat & xLabelFormat (unsigned int index = 0) const {
		return itsXLabels_[index];
	}
	void setXLabelFormat (const PlotMSLabelFormat & value,
			unsigned int index = 0) {
		if (itsXLabels_[index] != value) {
			itsXLabels_[index] = value;
			updated();
		}
	}


	const vector < PlotMSLabelFormat > &yLabelFormats() const {
		return itsYLabels_;
	}
	void setYLabelFormats (const vector < PlotMSLabelFormat > &value) {
		if (itsYLabels_ != value) {
			itsYLabels_ = value;
			updated();
		}
	}
	const PlotMSLabelFormat & yLabelFormat (unsigned int index = 0) const {
		return itsYLabels_[index];
	}
	void setYLabelFormat (const PlotMSLabelFormat & value,
			unsigned int index = 0) {
		if (itsYLabels_[index] != value) {
			itsYLabels_[index] = value;
			updated();
		}
	}


	const vector < bool > &xAxesShown() const {
		return itsXAxesShown_;
	}
	void showXAxes (const vector < bool > &value) {
		if (itsXAxesShown_ != value) {
			itsXAxesShown_ = value;
			updated();
		}
	}
	bool xAxisShown (unsigned int index = 0) const {
		if (index >= itsXAxesShown_.size())
			const_cast < vector < bool > &>(itsXAxesShown_).resize (index + 1);
		return itsXAxesShown_[index];
	}
	void showXAxis (const bool & value, unsigned int index = 0) {
		if (index >= itsXAxesShown_.size())
			itsXAxesShown_.resize (index + 1);
		if (itsXAxesShown_[index] != value) {
			itsXAxesShown_[index] = value;
			updated();
		}
	}


	const vector < bool > &yAxesShown() const {
		return itsYAxesShown_;
	}
	void showYAxes (const vector < bool > &value) {
		if (itsYAxesShown_ != value) {
			itsYAxesShown_ = value;
			updated();
		}
	}
	bool yAxisShown (unsigned int index = 0) const {
		if (index >= itsYAxesShown_.size())
			const_cast < vector < bool > &>(itsYAxesShown_).resize (index + 1);
		return itsYAxesShown_[index];
	}
	void showYAxis (const bool & value, unsigned int index = 0) {
		if (index >= itsYAxesShown_.size())
			itsYAxesShown_.resize (index + 1);
		if (itsYAxesShown_[index] != value) {
			itsYAxesShown_[index] = value;
			updated();
		}
	}


	const vector < bool > &legendsShown() const {
		return itsLegendsShown_;
	}
	void showLegends (const vector < bool > &value) {
		if (itsLegendsShown_ != value) {
			itsLegendsShown_ = value;
			updated();
		}
	}
	bool legendShown (unsigned int index = 0) const {
		if (index >= itsLegendsShown_.size())
			const_cast < vector < bool > &>(itsLegendsShown_).resize (index + 1);
		return itsLegendsShown_[index];
	}
	void showLegend (const bool & value, unsigned int index = 0) {
		if (index >= itsLegendsShown_.size())
			itsLegendsShown_.resize (index + 1);
		if (itsLegendsShown_[index] != value) {
			itsLegendsShown_[index] = value;
			updated();
		}
	}


	const vector < PlotCanvas::LegendPosition > &legendPositions() const {
		return itsLegendsPos_;
	}
	void showLegends (const vector < PlotCanvas::LegendPosition > &value) {
		if (itsLegendsPos_ != value) {
			itsLegendsPos_ = value;
			updated();
		}
	}
	PlotCanvas::LegendPosition legendPosition (unsigned int index = 0) const {
		if (index >= itsLegendsPos_.size())
			const_cast < vector < PlotCanvas::LegendPosition >
		&>(itsLegendsPos_).resize (index + 1);
		return itsLegendsPos_[index];
	}
	void showLegend (const PlotCanvas::LegendPosition & value,
			unsigned int index = 0) {
		if (index >= itsLegendsPos_.size())
			itsLegendsPos_.resize (index + 1);
		if (itsLegendsPos_[index] != value) {
			itsLegendsPos_[index] = value;
			updated();
		}
	}


	const vector < PlotMSLabelFormat > &titleFormats() const {
		return itsTitles_;
	}
	void setTitleFormats (const vector < PlotMSLabelFormat > &value) {
		if (itsTitles_ != value) {
			itsTitles_ = value;
			updated();
		}
	}
	const PlotMSLabelFormat & titleFormat (unsigned int index = 0) const {
		return itsTitles_[index];
	}
	void setTitleFormat (const PlotMSLabelFormat & value, unsigned int index =
			0) {
		if (itsTitles_[index] != value) {
			itsTitles_[index] = value;
			updated();
		}
	}


	const vector < bool > &gridMajorsShown() const {
		return itsGridMajsShown_;
	}
	void showGridMajors (const vector < bool > &value) {
		if (itsGridMajsShown_ != value) {
			itsGridMajsShown_ = value;
			updated();
		}
	}
	bool gridMajorShown (unsigned int index = 0) const {
		if (index >= itsGridMajsShown_.size())
			const_cast < vector < bool > &>(itsGridMajsShown_).resize (index + 1);
		return itsGridMajsShown_[index];
	}
	void showGridMajor (const bool & value, unsigned int index = 0) {
		if (index >= itsGridMajsShown_.size())
			itsGridMajsShown_.resize (index + 1);
		if (itsGridMajsShown_[index] != value) {
			itsGridMajsShown_[index] = value;
			updated();
		}
	}


	const vector < bool > &gridMinorsShown() const {
		return itsGridMinsShown_;
	}
	void showGridMinors (const vector < bool > &value) {
		if (itsGridMinsShown_ != value) {
			itsGridMinsShown_ = value;
			updated();
		}
	}
	bool gridMinorShown (unsigned int index = 0) const {
		if (index >= itsGridMinsShown_.size())
			const_cast < vector < bool > &>(itsGridMinsShown_).resize (index + 1);
		return itsGridMinsShown_[index];
	}
	void showGridMinor (const bool & value, unsigned int index = 0) {
		if (index >= itsGridMinsShown_.size())
			itsGridMinsShown_.resize (index + 1);
		if (itsGridMinsShown_[index] != value) {
			itsGridMinsShown_[index] = value;
			updated();
		}
	}


	const vector < PlotLinePtr > &gridMajorLines() const {
		return itsGridMajLines_;
	}
	void setGridMajorLines (const vector < PlotLinePtr > &value) {
		if (itsGridMajLines_ != value) {
			itsGridMajLines_ = value;
			updated();
		}
	}
	PlotLinePtr gridMajorLine (unsigned int index = 0) const {
		if (index >= itsGridMajLines_.size())
			const_cast < vector < PlotLinePtr >
		&>(itsGridMajLines_).resize (index + 1);
		return itsGridMajLines_[index];
	}
	void setGridMajorLine (const PlotLinePtr & value, unsigned int index = 0) {
		if (index >= itsGridMajLines_.size())
			itsGridMajLines_.resize (index + 1);
		if (itsGridMajLines_[index] != value) {
			itsGridMajLines_[index] = value;
			updated();
		}
	}


	const vector < PlotLinePtr > &gridMinorLines() const {
		return itsGridMinLines_;
	}
	void setGridMinorLines (const vector < PlotLinePtr > &value) {
		if (itsGridMinLines_ != value) {
			itsGridMinLines_ = value;
			updated();
		}
	}
	PlotLinePtr gridMinorLine (unsigned int index = 0) const {
		if (index >= itsGridMinLines_.size())
			const_cast < vector < PlotLinePtr >
		&>(itsGridMinLines_).resize (index + 1);
		return itsGridMinLines_[index];
	}
	void setGridMinorLine (const PlotLinePtr & value, unsigned int index = 0) {
		if (index >= itsGridMinLines_.size())
			itsGridMinLines_.resize (index + 1);
		if (itsGridMinLines_[index] != value) {
			itsGridMinLines_[index] = value;
			updated();
		}
	}


private:
	/* Parameters' values */
	vector<PlotMSLabelFormat> itsXLabels_;
	vector<PlotMSLabelFormat> itsYLabels_;
	vector<bool> itsXAxesShown_;
	vector<bool> itsYAxesShown_;
	vector<bool> itsLegendsShown_;
	vector<PlotCanvas::LegendPosition > itsLegendsPos_;
	vector<PlotMSLabelFormat> itsTitles_;
	vector<bool> itsGridMajsShown_;
	vector<bool> itsGridMinsShown_;
	vector<PlotLinePtr> itsGridMajLines_;
	vector<PlotLinePtr> itsGridMinLines_;

	/* Key strings for Record */
	static const String REC_XLABELS;
	static const String REC_YLABELS;
	static const String REC_SHOWXAXES;
	static const String REC_SHOWYAXES;
	static const String REC_SHOWLEGENDS;
	static const String REC_LEGENDSPOS;
	static const String REC_TITLES;
	static const String REC_SHOWGRIDMAJS;
	static const String REC_SHOWGRIDMINS;
	static const String REC_GRIDMAJLINES;
	static const String REC_GRIDMINLINES;


	void setDefaults();
};





// Subclass of PlotMSPlotParameters::Group to handle display parameters.
// Currently includes:
// * flagged and unflagged symbols
// * plot title format
// * colorize flag and axis
// Parameters are vector-based, on a per-plot basis.
//
class PMS_PP_Display : public PlotMSPlotParameters::Group {

public:
	/* Constructor which takes a factory */
	PMS_PP_Display (PlotFactoryPtr factory);

	/* Copy constructor.  See operator=(). */
	PMS_PP_Display (const PMS_PP_Display & copy);

	~PMS_PP_Display();


	/* Implements PlotMSPlotParameters::Group::clone(). */
	Group *clone() const {
		return new PMS_PP_Display (*this);
	}

	/* Implements PlotMSPlotParameters::Group::name(). */
	const String & name() const {
		static String groupName = PMS_PP::UPDATE_DISPLAY_NAME;
		return groupName;
	}

	/* Implements PlotMSPlotParameters::Group::toRecord(). */
	Record toRecord() const;

	/* Implements PlotMSPlotParameters::Group::fromRecord(). */
	void fromRecord (const Record & record);

	/* Implements PlotMSPlotParameters::Group::requiresRedrawOnChanged(). */
	bool requiresRedrawOnChange() const {
		return true;
	}

	/* Overrides PlotMSPlotParameters::Group::operator=(). */
	Group & operator= (const Group & other);

	/* Overrides PlotMSPlotParameters::Group::operator==(). */
	bool operator== (const Group & other) const;


	void setColorize (const bool & colorize, const PMS::Axis & axis,
			unsigned int index = 0);


	void resizeVectors (unsigned int newSize);


	const vector < PlotSymbolPtr > &unflaggedSymbols() const {
		return itsUnflaggedSymbols_;
	}
	void setUnflaggedSymbols (const vector < PlotSymbolPtr > &value) {
		if (itsUnflaggedSymbols_ != value) {
			itsUnflaggedSymbols_ = value;
			updated();
		}
	}
	PlotSymbolPtr unflaggedSymbol (unsigned int index = 0) const {
		if (index >= itsUnflaggedSymbols_.size())
			const_cast < vector < PlotSymbolPtr >
		&>(itsUnflaggedSymbols_).resize (index + 1);
		return itsUnflaggedSymbols_[index];
	}
	void setUnflaggedSymbol (const PlotSymbolPtr & value, unsigned int index =
			0) {
		if (index >= itsUnflaggedSymbols_.size())
			itsUnflaggedSymbols_.resize (index + 1);
		if (itsUnflaggedSymbols_[index] != value) {
			Record newValueRecord = value->toRecord();
			//itsUnflaggedSymbols_[index] = value;
			itsUnflaggedSymbols_[index]->fromRecord( newValueRecord );
			updated();
		}
	}


	const vector < PlotSymbolPtr > &flaggedSymbols() const {
		return itsFlaggedSymbols_;
	}
	void setFlaggedSymbols (const vector < PlotSymbolPtr > &value) {
		if (itsFlaggedSymbols_ != value) {
			itsFlaggedSymbols_ = value;
			updated();
		}
	}
	PlotSymbolPtr flaggedSymbol (unsigned int index = 0) const {
		if (index >= itsFlaggedSymbols_.size())
			const_cast < vector < PlotSymbolPtr >
		&>(itsFlaggedSymbols_).resize (index + 1);
		return itsFlaggedSymbols_[index];
	}
	void setFlaggedSymbol (const PlotSymbolPtr & value, unsigned int index =
			0) {
		if (index >= itsFlaggedSymbols_.size())
			itsFlaggedSymbols_.resize (index + 1);
		if (itsFlaggedSymbols_[index] != value) {
			//itsFlaggedSymbols_[index] = value;
			Record valueRecord = value->toRecord();
			itsFlaggedSymbols_[index]->fromRecord( valueRecord );
			updated();
		}
	}


	const vector < PlotMSLabelFormat > &titleFormats() const {
		return itsTitleFormats_;
	}
	void setTitleFormats (const vector < PlotMSLabelFormat > &value) {
		if (itsTitleFormats_ != value) {
			itsTitleFormats_ = value;
			updated();
		}
	}
	const PlotMSLabelFormat & titleFormat (unsigned int index = 0) const {
		return itsTitleFormats_[index];
	}
	void setTitleFormat (const PlotMSLabelFormat & value, unsigned int index =
			0) {
		if (itsTitleFormats_[index] != value) {
			itsTitleFormats_[index] = value;
			updated();
		}
	}


	const vector < bool > &colorizeFlags() const {
		return itsColorizeFlags_;
	}
	void setColorize (const vector < bool > &value) {
		if (itsColorizeFlags_ != value) {
			itsColorizeFlags_ = value;
			updated();
		}
	}
	bool colorizeFlag (unsigned int index = 0) const {
		if (index >= itsColorizeFlags_.size())
			const_cast < vector < bool > &>(itsColorizeFlags_).resize (index + 1);
		return itsColorizeFlags_[index];
	}
	void setColorize (const bool & value, unsigned int index = 0) {
		if (index >= itsColorizeFlags_.size())
			itsColorizeFlags_.resize (index + 1);
		if (itsColorizeFlags_[index] != value) {
			itsColorizeFlags_[index] = value;
			updated();
		}
	}


	const vector < PMS::Axis > &colorizeAxes() const {
		return itsColorizeAxes_;
	}
	void setColorize (const vector < PMS::Axis > &value) {
		if (itsColorizeAxes_ != value) {
			itsColorizeAxes_ = value;
			updated();
		}
	}
	PMS::Axis colorizeAxis (unsigned int index = 0) const {
		if (index >= itsColorizeAxes_.size())
			const_cast < vector < PMS::Axis >
		&>(itsColorizeAxes_).resize (index + 1);
		return itsColorizeAxes_[index];
	}
	void setColorize (const PMS::Axis & value, unsigned int index = 0) {
		if (index >= itsColorizeAxes_.size())
			itsColorizeAxes_.resize (index + 1);
		if (itsColorizeAxes_[index] != value) {
			itsColorizeAxes_[index] = value;
			updated();
		}
	}




private:
	/* Parameters' values */
	vector<PlotSymbolPtr> itsUnflaggedSymbols_;
	vector<PlotSymbolPtr> itsFlaggedSymbols_;
	vector<PlotMSLabelFormat> itsTitleFormats_;
	vector<bool> itsColorizeFlags_;
	vector<PMS::Axis> itsColorizeAxes_;


	/* Key strings for Record */
	static const String REC_UNFLAGGEDS;
	static const String REC_FLAGGEDS;
	static const String REC_TITLES;
	static const String REC_COLFLAGS;
	static const String REC_COLAXES;

	void setDefaults();
};




// Subclass of PlotMSPlotParameters::Group to handle iteration parameters.
// Currently includes:
// * which axis to use
// * rows, columns to fit onto each page
// Parameters apply to whole set of plots
//
class PMS_PP_Iteration : public PlotMSPlotParameters::Group {



public:

	/* Constructor which takes a factory */
	PMS_PP_Iteration (PlotFactoryPtr factory);

	/* Copy constructor.  See operator=(). */
	PMS_PP_Iteration (const PMS_PP_Iteration & copy);

	~PMS_PP_Iteration();


	/* Implements PlotMSPlotParameters::Group::clone(). */
	Group *clone() const {
		return new PMS_PP_Iteration (*this);
	}

	/* Implements PlotMSPlotParameters::Group::name(). */
	const String & name() const {
		static String groupName = PMS_PP::UPDATE_ITERATION_NAME;
		return groupName;
	}

	/* Implements PlotMSPlotParameters::Group::toRecord(). */
	Record toRecord() const;

	/* Implements PlotMSPlotParameters::Group::fromRecord(). */
	void fromRecord (const Record & record);

	/* Implements PlotMSPlotParameters::Group::requiresRedrawOnChanged(). */
	bool requiresRedrawOnChange() const {
		return true;
	}

	/* Overrides PlotMSPlotParameters::Group::operator=(). */
	Group & operator= (const Group & other);

	/* Overrides PlotMSPlotParameters::Group::operator==(). */
	bool operator== (const Group & other) const;

	//Returns whether or not we are iterating on an axis.
	bool isIteration() const;

	const PlotMSIterParam& iterParam() const {
		return itsIterParam_;
	}
	void setIterParam(PlotMSIterParam iterparam) {
		if (itsIterParam_ != iterparam) {
			itsIterParam_=iterparam;
			updated();
		}
	}

	PMS::Axis  iterationAxis() const {
		return itsIterParam_.iterAxis();
	}

	void setIterationAxis (const PMS::Axis & value) {
		if (iterationAxis()!=value) {
			itsIterParam_.setIterAxis(value);
			updated();
		}
	}


	int getGridRow() const  {
		return itsIterParam_.getGridRow();
	}
	void setGridRow(const int &value)  {
		if (getGridRow() != value) {
			itsIterParam_.setGridRow(value);
			updated();
		}
	}

	int getGridCol() const  {
		return itsIterParam_.getGridCol();
	}
	void setGridCol(const int &value)  {
		if (getGridCol() != value) {
			itsIterParam_.setGridCol(value);
			updated();
		}
	}

	Bool isCommonAxisX() const {
		return itsIterParam_.isCommonAxisX();
	}
	void setCommonAxisX( bool commonAxis ){
		if ( isCommonAxisX() != commonAxis ){
			bool validValue = false;
			if ( isGlobalScaleX()){
				validValue = true;
			}
			else if ( !commonAxis ){
				validValue = true;
			}

			if ( validValue ){
				itsIterParam_.setCommonAxisX( commonAxis );
				updated();
			}
		}
	}
	Bool isCommonAxisY() const {
		return itsIterParam_.isCommonAxisY();
	}
	void setCommonAxisY( bool commonAxis ){
		if ( isCommonAxisY() != commonAxis ){
			bool validValue = false;
			if ( isGlobalScaleY() ){
				validValue = true;
			}
			else if ( !commonAxis ){
				validValue = true;
			}
			if ( validValue ){
				itsIterParam_.setCommonAxisY( commonAxis );
				updated();
			}
		}
	}
	Bool isGlobalScaleX() const {
			return itsIterParam_.isGlobalAxisX();
		}
		void setGlobalScaleX( bool globalAxis ){
			if ( isGlobalScaleX() != globalAxis ){
				itsIterParam_.setGlobalScaleX( globalAxis );
				updated();
			}
		}
		Bool isGlobalScaleY() const {
			return itsIterParam_.isGlobalAxisY();
		}
		void setGlobalScaleY( bool globalAxis ){
			if ( isGlobalScaleY() != globalAxis ){
				itsIterParam_.setGlobalScaleY( globalAxis );
				updated();
			}
		}
private:
	/* Parameters' values */
	PlotMSIterParam itsIterParam_;
	void setDefaults();
};




}

#endif /* PLOTMSPLOTPARAMETERGROUPS_H_ */
