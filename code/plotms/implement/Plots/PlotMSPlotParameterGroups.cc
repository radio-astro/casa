//# PlotMSPlotParameterGroups.cc: Implementations of plot subparameter groups.
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
#include <plotms/Plots/PlotMSPlotParameterGroups.h>

#include <plotms/PlotMS/PlotMSLabelFormat.h>

namespace casa
{

////////////////////////
// PMS_PP DEFINITIONS //
////////////////////////

const String PMS_PP::UPDATE_REDRAW_NAME = "REDRAW";
const int PMS_PP::UPDATE_REDRAW =
    PlotMSWatchedParameters::REGISTER_UPDATE_FLAG(UPDATE_REDRAW_NAME);

const String PMS_PP::UPDATE_MSDATA_NAME = "MSDATA";
const int PMS_PP::UPDATE_MSDATA =
    PlotMSWatchedParameters::REGISTER_UPDATE_FLAG(UPDATE_MSDATA_NAME);

const String PMS_PP::UPDATE_CACHE_NAME = "CACHE";
const int PMS_PP::UPDATE_CACHE =
    PlotMSWatchedParameters::REGISTER_UPDATE_FLAG(UPDATE_CACHE_NAME);

const String PMS_PP::UPDATE_AXES_NAME = "AXES";
const int PMS_PP::UPDATE_AXES =
    PlotMSWatchedParameters::REGISTER_UPDATE_FLAG(UPDATE_AXES_NAME);

const String PMS_PP::UPDATE_CANVAS_NAME = "CANVAS";
const int PMS_PP::UPDATE_CANVAS =
    PlotMSWatchedParameters::REGISTER_UPDATE_FLAG(UPDATE_CANVAS_NAME);

const String PMS_PP::UPDATE_DISPLAY_NAME = "DISPLAY";
const int PMS_PP::UPDATE_DISPLAY =
    PlotMSWatchedParameters::REGISTER_UPDATE_FLAG(UPDATE_DISPLAY_NAME);

const String PMS_PP::UPDATE_ITERATION_NAME = "ITERATION";
const int PMS_PP::UPDATE_ITERATION =
    PlotMSWatchedParameters::REGISTER_UPDATE_FLAG(UPDATE_ITERATION_NAME);

const String PMS_PP::UPDATE_LOG_NAME = "LOG";
const int PMS_PP::UPDATE_LOG =
    PlotMSWatchedParameters::REGISTER_UPDATE_FLAG(UPDATE_LOG_NAME);

const String PMS_PP::UPDATE_PLOTMS_OPTIONS_NAME = "PLOTMS_OPTIONS";
const int PMS_PP::UPDATE_PLOTMS_OPTIONS =
    PlotMSWatchedParameters::REGISTER_UPDATE_FLAG(UPDATE_PLOTMS_OPTIONS_NAME);





///////////////////////////////
// PMS_PP_MSDATA DEFINITIONS //
///////////////////////////////

// PMS_PP_MSData record keys.
const String PMS_PP_MSData::REC_FILENAME = "filename";
const String PMS_PP_MSData::REC_SELECTION = "selection";
const String PMS_PP_MSData::REC_AVERAGING = "averaging";
const String PMS_PP_MSData::REC_TRANSFORMATIONS = "transformations";


PMS_PP_MSData::PMS_PP_MSData(PlotFactoryPtr factory) 
	:  PlotMSPlotParameters::Group(factory)
{
    setDefaults();
} 



PMS_PP_MSData::PMS_PP_MSData(const PMS_PP_MSData& copy) : PlotMSPlotParameters::Group(copy)
{
    setDefaults();
    operator=(copy);
} 
    

    
PMS_PP_MSData::~PMS_PP_MSData()   { }




Record PMS_PP_MSData::toRecord() const
    {
    Record rec;
    rec.define(REC_FILENAME, itsFilename_);
    rec.defineRecord(REC_SELECTION, itsSelection_.toRecord());
    rec.defineRecord(REC_AVERAGING, itsAveraging_.toRecord());
    rec.defineRecord(REC_TRANSFORMATIONS, itsTransformations_.toRecord());
    return rec;
    }


void PMS_PP_MSData::fromRecord(const Record& record)
    {
    bool valuesChanged = false;
    if (record.isDefined(REC_FILENAME) && record.dataType(REC_FILENAME) == TpString && itsFilename_ != record.asString(REC_FILENAME))
        {
        itsFilename_ = record.asString(REC_FILENAME);
        valuesChanged = true;
        }
    if (record.isDefined(REC_SELECTION) && record.dataType(REC_SELECTION) == TpRecord)
        {
        PlotMSSelection tmp(itsSelection_);
        tmp.fromRecord(record.asRecord(REC_SELECTION));
        if (itsSelection_ != tmp)
            {
            itsSelection_ = tmp;
            valuesChanged = true;
            }
        }
    if (record.isDefined(REC_AVERAGING) && record.dataType(REC_AVERAGING) == TpRecord)
        {
        PlotMSAveraging tmp(itsAveraging_);
        tmp.fromRecord(record.asRecord(REC_AVERAGING));
        if (itsAveraging_ != tmp)
            {
            itsAveraging_ = tmp;
            valuesChanged = true;
            }
        }
    if (record.isDefined(REC_TRANSFORMATIONS) && record.dataType(REC_TRANSFORMATIONS) == TpRecord)
        {
        PlotMSTransformations tmp(itsTransformations_);
        tmp.fromRecord(record.asRecord(REC_TRANSFORMATIONS));
        if (itsTransformations_ != tmp)
            {
            itsTransformations_ = tmp;
            valuesChanged = true;
            }
        }



    if (valuesChanged) updated();
    }



PlotMSPlotParameters::Group& PMS_PP_MSData::operator=(const Group& other)
    {
    const PMS_PP_MSData* o = dynamic_cast<const PMS_PP_MSData*>(&other);
    if (o != NULL && *this != *o)
        {
        itsFilename_ = o->itsFilename_;
        itsSelection_ = o->itsSelection_;
        itsAveraging_ = o->itsAveraging_;
        itsTransformations_ = o->itsTransformations_;
        updated();
        }
    return *this;
    }


bool PMS_PP_MSData::operator==(const Group& other) const
    {
    const PMS_PP_MSData* o = dynamic_cast<const PMS_PP_MSData*>(&other);
    if (o == NULL) return false;
    if (itsFilename_ != o->itsFilename_) return false;
    if (itsSelection_ != o->itsSelection_) return false;
    if (itsAveraging_ != o->itsAveraging_) return false;
    if (itsTransformations_ != o->itsTransformations_) return false;
    return true;
    }



void PMS_PP_MSData::setDefaults()
    {
    itsFilename_ = "";
    itsSelection_ = PlotMSSelection();
    itsAveraging_ = PlotMSAveraging();
    itsTransformations_ = PlotMSTransformations();
    }





//////////////////////////////
// PMS_PP_CACHE DEFINITIONS //
//////////////////////////////


// PMS_PP_Cache record keys.
const String PMS_PP_Cache::REC_XAXES = "xaxes";
const String PMS_PP_Cache::REC_YAXES = "yaxes";
const String PMS_PP_Cache::REC_XDATACOLS = "xdatacolumns";
const String PMS_PP_Cache::REC_YDATACOLS = "ydatacolumns";


PMS_PP_Cache::PMS_PP_Cache(PlotFactoryPtr factory) : PlotMSPlotParameters::Group(factory)
    {
    setDefaults();
    } PMS_PP_Cache::PMS_PP_Cache(const PMS_PP_Cache& copy) : PlotMSPlotParameters::Group(copy)
    {
    setDefaults();
    operator=(copy);
    } PMS_PP_Cache::~PMS_PP_Cache() { }


Record PMS_PP_Cache::toRecord() const
    {
    Record rec;
    rec.define(REC_XAXES, PMS::toIntVector<PMS::Axis>(itsXAxes_));
    rec.define(REC_YAXES, PMS::toIntVector<PMS::Axis>(itsYAxes_));
    rec.define(REC_XDATACOLS, PMS::toIntVector<PMS::DataColumn>(itsXData_));
    rec.define(REC_YDATACOLS, PMS::toIntVector<PMS::DataColumn>(itsYData_));
    return rec;
    }


void PMS_PP_Cache::fromRecord(const Record& record)
    {
    bool valuesChanged = false;
    if (record.isDefined(REC_XAXES) && record.dataType(REC_XAXES) == TpArrayInt)
        {
        vector<PMS::Axis> tmp = PMS::fromIntVector<PMS::Axis>(record.asArrayInt(REC_XAXES));
        if (itsXAxes_ != tmp)
            {
            itsXAxes_ = tmp;
            valuesChanged = true;
            }
        }
    if (record.isDefined(REC_YAXES) && record.dataType(REC_YAXES) == TpArrayInt)
        {
        vector<PMS::Axis> tmp = PMS::fromIntVector<PMS::Axis>(record.asArrayInt(REC_YAXES));
        if (itsYAxes_ != tmp)
            {
            itsYAxes_ = tmp;
            valuesChanged = true;
            }
        }
    if (record.isDefined(REC_XDATACOLS) && record.dataType(REC_XDATACOLS) == TpArrayInt)
        {
        vector<PMS::DataColumn> tmp = PMS::fromIntVector<PMS::DataColumn>(record.asArrayInt(REC_XDATACOLS));
        if (itsXData_ != tmp)
            {
            itsXData_ = tmp;
            valuesChanged = true;
            }
        }
    if (record.isDefined(REC_YDATACOLS) && record.dataType(REC_YDATACOLS) == TpArrayInt)
        {
        vector<PMS::DataColumn> tmp = PMS::fromIntVector<PMS::DataColumn>(record.asArrayInt(REC_YDATACOLS));
        if (itsYData_ != tmp)
            {
            itsYData_ = tmp;
            valuesChanged = true;
            }
        }
    if (valuesChanged) updated();
    }


PlotMSPlotParameters::Group& PMS_PP_Cache::operator=(const Group& other)
    {
    const PMS_PP_Cache* o = dynamic_cast<const PMS_PP_Cache*>(&other);
    if (o != NULL && *this != *o)
        {
        itsXAxes_ = o->itsXAxes_;
        itsYAxes_ = o->itsYAxes_;
        itsXData_ = o->itsXData_;
        itsYData_ = o->itsYData_;
        updated();
        }
    return *this;
    }


bool PMS_PP_Cache::operator==(const Group& other) const
    {
    const PMS_PP_Cache* o = dynamic_cast<const PMS_PP_Cache*>(&other);
    if (o == NULL) return false;
    if (itsXAxes_ != o->itsXAxes_) return false;
    if (itsYAxes_ != o->itsYAxes_) return false;
    if (itsXData_ != o->itsXData_) return false;
    if (itsYData_ != o->itsYData_) return false;
    return true;
    }


void PMS_PP_Cache::setDefaults()
    {
    itsXAxes_ = vector<PMS::Axis>(1, PMS::DEFAULT_XAXIS);
    itsYAxes_ = vector<PMS::Axis>(1, PMS::DEFAULT_YAXIS);
    itsXData_ = vector<PMS::DataColumn>(1, PMS::DEFAULT_DATACOLUMN);
    itsYData_ = vector<PMS::DataColumn>(1, PMS::DEFAULT_DATACOLUMN);
    }

unsigned int PMS_PP_Cache::numXAxes() const
    {
    return itsXAxes_.size();
    }
unsigned int PMS_PP_Cache::numYAxes() const
    {
    return itsYAxes_.size();
    }

void PMS_PP_Cache::setAxes(const PMS::Axis& xAxis, const PMS::Axis& yAxis,
                           const PMS::DataColumn& xData, const PMS::DataColumn& yData,
                           unsigned int index)
    {
    if (itsXAxes_[index] != xAxis || itsYAxes_[index] != yAxis ||
            itsXData_[index] != xData || itsYData_[index] != yData)
        {
        itsXAxes_[index] = xAxis;
        itsYAxes_[index] = yAxis;
        itsXData_[index] = xData;
        itsYData_[index] = yData;
        updated();
        }
    }





/////////////////////////////
// PMS_PP_AXES DEFINITIONS //
/////////////////////////////

// PMS_PP_Axes record keys.
const String PMS_PP_Axes::REC_XAXES = "canvasXAxes";
const String PMS_PP_Axes::REC_YAXES = "canvasYAxes";
const String PMS_PP_Axes::REC_XRANGESSET = "xRangesSet";
const String PMS_PP_Axes::REC_YRANGESSET = "yRangesSet";
const String PMS_PP_Axes::REC_XRANGES = "xRanges";
const String PMS_PP_Axes::REC_YRANGES = "yRanges";


PMS_PP_Axes::PMS_PP_Axes(PlotFactoryPtr factory) : PlotMSPlotParameters::Group(factory)
    {
    setDefaults();
    } PMS_PP_Axes::PMS_PP_Axes(const PMS_PP_Axes& copy) : PlotMSPlotParameters::Group(copy)
    {
    setDefaults();
    operator=(copy);
    } PMS_PP_Axes::~PMS_PP_Axes() { }


Record PMS_PP_Axes::toRecord() const
    {
    Record rec;
    rec.define(REC_XAXES, PMS::toIntVector<PlotAxis>(itsXAxes_));
    rec.define(REC_YAXES, PMS::toIntVector<PlotAxis>(itsYAxes_));
    rec.define(REC_XRANGESSET, Vector<bool>(itsXRangesSet_));
    rec.define(REC_YRANGESSET, Vector<bool>(itsYRangesSet_));

    vector<double> firsts(itsXRanges_.size()), seconds(itsXRanges_.size());
    for (unsigned int i = 0; i < itsXRanges_.size(); i++)
        {
        firsts[i] = itsXRanges_[i].first;
        seconds[i] = itsXRanges_[i].second;
        }
        {
        Record tmpRec;
        for (unsigned int i = 0; i < firsts.size(); i++) tmpRec.define(i, firsts[i]);
        rec.defineRecord(REC_XRANGES + ".first", tmpRec);
        }
        {
        Record tmpRec;
        for (unsigned int i = 0; i < seconds.size(); i++) tmpRec.define(i, seconds[i]);
        rec.defineRecord(REC_XRANGES + ".second", tmpRec);
        }

    firsts.resize(itsYRanges_.size());
    seconds.resize(itsYRanges_.size());
    for (unsigned int i = 0; i < itsYRanges_.size(); i++)
        {
        firsts[i] = itsYRanges_[i].first;
        seconds[i] = itsYRanges_[i].second;
        }
        {
        Record tmpRec;
        for (unsigned int i = 0; i < firsts.size(); i++) tmpRec.define(i, firsts[i]);
        rec.defineRecord(REC_YRANGES + ".first", tmpRec);
        }
        {
        Record tmpRec;
        for (unsigned int i = 0; i < seconds.size(); i++) tmpRec.define(i, seconds[i]);
        rec.defineRecord(REC_YRANGES + ".second", tmpRec);
        }
    return rec;
    }


void PMS_PP_Axes::fromRecord(const Record& record)
    {
    bool valuesChanged = false;
    if (record.isDefined(REC_XAXES) && record.dataType(REC_XAXES) == TpArrayInt)
        {
        vector<PlotAxis> tmp = PMS::fromIntVector<PlotAxis>(record.asArrayInt(REC_XAXES));
        if (itsXAxes_ != tmp)
            {
            itsXAxes_ = tmp;
            valuesChanged = true;
            }
        }
    if (record.isDefined(REC_YAXES) && record.dataType(REC_YAXES) == TpArrayInt)
        {
        vector<PlotAxis> tmp = PMS::fromIntVector<PlotAxis>(record.asArrayInt(REC_YAXES));
        if (itsYAxes_ != tmp)
            {
            itsYAxes_ = tmp;
            valuesChanged = true;
            }
        }
    if (record.isDefined(REC_XRANGESSET) && record.dataType(REC_XRANGESSET) == TpArrayBool)
        {
        vector<bool> tmp;
        record.asArrayBool(REC_XRANGESSET).tovector(tmp);
        if (itsXRangesSet_ != tmp)
            {
            itsXRangesSet_ = tmp;
            valuesChanged = true;
            }
        }
    if (record.isDefined(REC_YRANGESSET) && record.dataType(REC_YRANGESSET) == TpArrayBool)
        {
        vector<bool> tmp;
        record.asArrayBool(REC_YRANGESSET).tovector(tmp);
        if (itsYRangesSet_ != tmp)
            {
            itsYRangesSet_ = tmp;
            valuesChanged = true;
            }
        }

    vector<double> firsts(itsXRanges_.size()), seconds(itsXRanges_.size());
    for (unsigned int i = 0; i < itsXRanges_.size(); i++)
        {
        firsts[i] = itsXRanges_[i].first;
        seconds[i] = itsXRanges_[i].second;
        }
    if (record.isDefined(REC_XRANGES + ".first") && record.dataType(REC_XRANGES + ".first") == TpRecord)
        {
        const Record& tmpRec = record.asRecord(REC_XRANGES + ".first");
        firsts.resize(tmpRec.nfields());
        for (unsigned int i= 0; i < firsts.size() && i < tmpRec.nfields(); i++)
            {
            if (tmpRec.dataType(i) == TpDouble && firsts[i] != tmpRec.asDouble(i))
                {
                firsts[i] = tmpRec.asDouble(i);
                valuesChanged = true;
                }
            }
        }
    if (record.isDefined(REC_XRANGES + ".second") && record.dataType(REC_XRANGES + ".second") == TpRecord)
        {
        const Record& tmpRec = record.asRecord(REC_XRANGES + ".second");
        seconds.resize(tmpRec.nfields());
        for (unsigned int i= 0; i < seconds.size() && i < tmpRec.nfields(); i++)
            {
            if (tmpRec.dataType(i) == TpDouble && seconds[i] != tmpRec.asDouble(i))
                {
                seconds[i] = tmpRec.asDouble(i);
                valuesChanged = true;
                }
            }
        }
    itsXRanges_.resize(min((uInt)firsts.size(), (uInt)seconds.size()));
    for (unsigned int i = 0; i < itsXRanges_.size(); i++)
        {
        itsXRanges_[i].first = firsts[i];
        itsXRanges_[i].second = seconds[i];
        }

    firsts.resize(itsYRanges_.size());
    seconds.resize(itsYRanges_.size());
    for (unsigned int i = 0; i < itsYRanges_.size(); i++)
        {
        firsts[i] = itsYRanges_[i].first;
        seconds[i] = itsYRanges_[i].second;
        }
    if (record.isDefined(REC_YRANGES + ".first") && record.dataType(REC_YRANGES + ".first") == TpRecord)
        {
        const Record& tmpRec = record.asRecord(REC_YRANGES + ".first");
        firsts.resize(tmpRec.nfields());
        for (unsigned int i= 0; i < firsts.size() && i < tmpRec.nfields(); i++)
            {
            if (tmpRec.dataType(i) == TpDouble && firsts[i] != tmpRec.asDouble(i))
                {
                firsts[i] = tmpRec.asDouble(i);
                valuesChanged = true;
                }
            }
        }
    if (record.isDefined(REC_YRANGES + ".second") && record.dataType(REC_YRANGES + ".second") == TpRecord)
        {
        const Record& tmpRec = record.asRecord(REC_YRANGES + ".second");
        seconds.resize(tmpRec.nfields());
        for (unsigned int i= 0; i < seconds.size() && i < tmpRec.nfields(); i++)
            {
            if (tmpRec.dataType(i) == TpDouble && seconds[i] != tmpRec.asDouble(i))
                {
                seconds[i] = tmpRec.asDouble(i);
                valuesChanged = true;
                }
            }
        }
    itsYRanges_.resize(min((uInt)firsts.size(), (uInt)seconds.size()));
    for (unsigned int i = 0; i < itsYRanges_.size(); i++)
        {
        itsYRanges_[i].first = firsts[i];
        itsYRanges_[i].second = seconds[i];
        }
    if (valuesChanged) updated();
    }


PlotMSPlotParameters::Group& PMS_PP_Axes::operator=(const Group& other)
    {
    const PMS_PP_Axes* o = dynamic_cast<const PMS_PP_Axes*>(&other);
    if (o != NULL && *this != *o)
        {
        itsXAxes_ = o->itsXAxes_;
        itsYAxes_ = o->itsYAxes_;
        itsXRangesSet_ = o->itsXRangesSet_;
        itsYRangesSet_ = o->itsYRangesSet_;
        itsXRanges_ = o->itsXRanges_;
        itsYRanges_ = o->itsYRanges_;
        updated();
        }
    return *this;
    }


bool PMS_PP_Axes::operator==(const Group& other) const
    {
    const PMS_PP_Axes* o = dynamic_cast<const PMS_PP_Axes*>(&other);
    if (o == NULL) return false;
    if (itsXAxes_ != o->itsXAxes_) return false;
    if (itsYAxes_ != o->itsYAxes_) return false;
    if (itsXRangesSet_.size() != o->itsXRangesSet_.size() || itsXRanges_.size() != o->itsXRanges_.size() || itsXRangesSet_.size() != itsXRanges_.size()) return false;
    for (unsigned int i = 0; i < itsXRangesSet_.size(); i++) if (itsXRangesSet_[i] != o->itsXRangesSet_[i] || (itsXRangesSet_[i] && itsXRanges_[i] != o->itsXRanges_[i])) return false;
    if (itsYRangesSet_.size() != o->itsYRangesSet_.size() || itsYRanges_.size() != o->itsYRanges_.size() || itsYRangesSet_.size() != itsYRanges_.size()) return false;
    for (unsigned int i = 0; i < itsYRangesSet_.size(); i++) if (itsYRangesSet_[i] != o->itsYRangesSet_[i] || (itsYRangesSet_[i] && itsYRanges_[i] != o->itsYRanges_[i])) return false;
    return true;
    }


void PMS_PP_Axes::setDefaults()
    {
    itsXAxes_ = vector<PlotAxis>(1, PMS::DEFAULT_CANVAS_XAXIS);
    itsYAxes_ = vector<PlotAxis>(1, PMS::DEFAULT_CANVAS_YAXIS);
    itsXRangesSet_ = vector<bool>(1, false);
    itsYRangesSet_ = vector<bool>(1, false);
    itsXRanges_ = vector<prange_t>(1, prange_t(0.0, 0.0));
    itsYRanges_ = vector<prange_t>(1, prange_t(0.0, 0.0));
    }

unsigned int PMS_PP_Axes::numXAxes() const
    {
    return itsXAxes_.size();
    }
unsigned int PMS_PP_Axes::numYAxes() const
    {
    return itsYAxes_.size();
    }

void PMS_PP_Axes::setAxes(const PlotAxis& xAxis, const PlotAxis& yAxis,
                          unsigned int index)
    {
    if (itsXAxes_[index] != xAxis || itsYAxes_[index] != yAxis)
        {
        itsXAxes_[index] = xAxis;
        itsYAxes_[index] = yAxis;
        updated();
        }
    }

void PMS_PP_Axes::setRanges(const bool& xSet, const bool& ySet,
                            const prange_t& xRange, const prange_t& yRange, unsigned int index)
    {
    if (itsXRangesSet_[index] != xSet || itsYRangesSet_[index] != ySet ||
            (xSet && itsXRanges_[index] != xRange) ||
            (ySet && itsYRanges_[index] != yRange))
        {
        itsXRangesSet_[index] = xSet;
        itsYRangesSet_[index] = ySet;
        itsXRanges_[index] = xRange;
        itsYRanges_[index] = yRange;
        }
    }






///////////////////////////////
// PMS_PP_CANVAS DEFINITIONS //
///////////////////////////////

// PMS_PP_Canvas record keys.
const String PMS_PP_Canvas::REC_XLABELS = "xLabelFormats";
const String PMS_PP_Canvas::REC_YLABELS = "yLabelFormats";
const String PMS_PP_Canvas::REC_SHOWXAXES = "showXAxes";
const String PMS_PP_Canvas::REC_SHOWYAXES = "showYAxes";
const String PMS_PP_Canvas::REC_SHOWLEGENDS = "showLegends";
const String PMS_PP_Canvas::REC_LEGENDSPOS = "legendPositions";
const String PMS_PP_Canvas::REC_TITLES = "canvasTitleFormats";
const String PMS_PP_Canvas::REC_SHOWGRIDMAJS = "showGridMajors";
const String PMS_PP_Canvas::REC_SHOWGRIDMINS = "showGridMinors";
const String PMS_PP_Canvas::REC_GRIDMAJLINES = "gridMajorLines";
const String PMS_PP_Canvas::REC_GRIDMINLINES = "gridMinorLines";


PMS_PP_Canvas::PMS_PP_Canvas(PlotFactoryPtr factory) : PlotMSPlotParameters::Group(factory)
    {
    setDefaults();
    } PMS_PP_Canvas::PMS_PP_Canvas(const PMS_PP_Canvas& copy) : PlotMSPlotParameters::Group(copy)
    {
    setDefaults();
    operator=(copy);
    } PMS_PP_Canvas::~PMS_PP_Canvas() { }


Record PMS_PP_Canvas::toRecord() const
    {
    Record rec;
        {
        Record tmpRec;
        for (unsigned int i = 0; i < itsXLabels_.size(); i++) tmpRec.define(i, itsXLabels_[i] .format);
        rec.defineRecord(REC_XLABELS, tmpRec);
        }
        {
        Record tmpRec;
        for (unsigned int i = 0; i < itsYLabels_.size(); i++) tmpRec.define(i, itsYLabels_[i] .format);
        rec.defineRecord(REC_YLABELS, tmpRec);
        }
    rec.define(REC_SHOWXAXES, Vector<bool>(itsXAxesShown_));
    rec.define(REC_SHOWYAXES, Vector<bool>(itsYAxesShown_));
    rec.define(REC_SHOWLEGENDS, Vector<bool>(itsLegendsShown_));
    rec.define(REC_LEGENDSPOS, PMS::toIntVector<PlotCanvas::LegendPosition>(itsLegendsPos_));
        {
        Record tmpRec;
        for (unsigned int i = 0; i < itsTitles_.size(); i++) tmpRec.define(i, itsTitles_[i] .format);
        rec.defineRecord(REC_TITLES, tmpRec);
        }
    rec.define(REC_SHOWGRIDMAJS, Vector<bool>(itsGridMajsShown_));
    rec.define(REC_SHOWGRIDMINS, Vector<bool>(itsGridMinsShown_));
        {
        Record tmpRec;
        for (unsigned int i = 0; i < itsGridMajLines_.size(); i++) tmpRec.defineRecord(i, itsGridMajLines_[i] ->toRecord());
        rec.defineRecord(REC_GRIDMAJLINES, tmpRec);
        }
        {
        Record tmpRec;
        for (unsigned int i = 0; i < itsGridMinLines_.size(); i++) tmpRec.defineRecord(i, itsGridMinLines_[i] ->toRecord());
        rec.defineRecord(REC_GRIDMINLINES, tmpRec);
        }
    return rec;
    }


void PMS_PP_Canvas::fromRecord(const Record& record)
    {
    bool valuesChanged = false;
    if (record.isDefined(REC_XLABELS) && record.dataType(REC_XLABELS) == TpRecord)
        {
        const Record& tmpRec = record.asRecord(REC_XLABELS);
        itsXLabels_.resize(tmpRec.nfields());
        for (unsigned int i= 0; i < itsXLabels_.size() && i < tmpRec.nfields(); i++)
            {
            if (tmpRec.dataType(i) == TpString && itsXLabels_[i] != tmpRec.asString(i))
                {
                itsXLabels_[i] = tmpRec.asString(i);
                valuesChanged = true;
                }
            }
        }
    if (record.isDefined(REC_YLABELS) && record.dataType(REC_YLABELS) == TpRecord)
        {
        const Record& tmpRec = record.asRecord(REC_YLABELS);
        itsYLabels_.resize(tmpRec.nfields());
        for (unsigned int i= 0; i < itsYLabels_.size() && i < tmpRec.nfields(); i++)
            {
            if (tmpRec.dataType(i) == TpString && itsYLabels_[i] != tmpRec.asString(i))
                {
                itsYLabels_[i] = tmpRec.asString(i);
                valuesChanged = true;
                }
            }
        }
    if (record.isDefined(REC_SHOWXAXES) && record.dataType(REC_SHOWXAXES) == TpArrayBool)
        {
        vector<bool> tmp;
        record.asArrayBool(REC_SHOWXAXES).tovector(tmp);
        if (itsXAxesShown_ != tmp)
            {
            itsXAxesShown_ = tmp;
            valuesChanged = true;
            }
        }
    if (record.isDefined(REC_SHOWYAXES) && record.dataType(REC_SHOWYAXES) == TpArrayBool)
        {
        vector<bool> tmp;
        record.asArrayBool(REC_SHOWYAXES).tovector(tmp);
        if (itsYAxesShown_ != tmp)
            {
            itsYAxesShown_ = tmp;
            valuesChanged = true;
            }
        }
    if (record.isDefined(REC_SHOWLEGENDS) && record.dataType(REC_SHOWLEGENDS) == TpArrayBool)
        {
        vector<bool> tmp;
        record.asArrayBool(REC_SHOWLEGENDS).tovector(tmp);
        if (itsLegendsShown_ != tmp)
            {
            itsLegendsShown_ = tmp;
            valuesChanged = true;
            }
        }
    if (record.isDefined(REC_LEGENDSPOS) && record.dataType(REC_LEGENDSPOS) == TpArrayInt)
        {
        vector<PlotCanvas::LegendPosition> tmp = PMS::fromIntVector<PlotCanvas::LegendPosition>(record.asArrayInt(REC_LEGENDSPOS));
        if (itsLegendsPos_ != tmp)
            {
            itsLegendsPos_ = tmp;
            valuesChanged = true;
            }
        }
    if (record.isDefined(REC_TITLES) && record.dataType(REC_TITLES) == TpRecord)
        {
        const Record& tmpRec = record.asRecord(REC_TITLES);
        itsTitles_.resize(tmpRec.nfields());
        for (unsigned int i= 0; i < itsTitles_.size() && i < tmpRec.nfields(); i++)
            {
            if (tmpRec.dataType(i) == TpString && itsTitles_[i] != tmpRec.asString(i))
                {
                itsTitles_[i] = tmpRec.asString(i);
                valuesChanged = true;
                }
            }
        }
    if (record.isDefined(REC_SHOWGRIDMAJS) && record.dataType(REC_SHOWGRIDMAJS) == TpArrayBool)
        {
        vector<bool> tmp;
        record.asArrayBool(REC_SHOWGRIDMAJS).tovector(tmp);
        if (itsGridMajsShown_ != tmp)
            {
            itsGridMajsShown_ = tmp;
            valuesChanged = true;
            }
        }
    if (record.isDefined(REC_SHOWGRIDMINS) && record.dataType(REC_SHOWGRIDMINS) == TpArrayBool)
        {
        vector<bool> tmp;
        record.asArrayBool(REC_SHOWGRIDMINS).tovector(tmp);
        if (itsGridMinsShown_ != tmp)
            {
            itsGridMinsShown_ = tmp;
            valuesChanged = true;
            }
        }
    if (record.isDefined(REC_GRIDMAJLINES) && record.dataType(REC_GRIDMAJLINES) == TpRecord)
        {
        const Record& tmpRec = record.asRecord(REC_GRIDMAJLINES);
        itsGridMajLines_.resize(tmpRec.nfields(), tmpRec.nfields() > 0 ? factory()->line(*itsGridMajLines_[0]) : NULL);
        PlotLinePtr tmp= itsGridMajLines_.size() > 0 ? factory()->line(*itsGridMajLines_[0]) : NULL;
        for (unsigned int i= 0; i < itsGridMajLines_.size() && i < tmpRec.nfields(); i++)
            {
            if (tmpRec.dataType(i) == TpRecord)
                {
                tmp->fromRecord(tmpRec.asRecord(i));
                if (*itsGridMajLines_[i] != *tmp)
                    {
                    *itsGridMajLines_[i] = *tmp;
                    valuesChanged = true;
                    }
                }
            }
        }
    if (record.isDefined(REC_GRIDMINLINES) && record.dataType(REC_GRIDMINLINES) == TpRecord)
        {
        const Record& tmpRec = record.asRecord(REC_GRIDMINLINES);
        itsGridMinLines_.resize(tmpRec.nfields(), tmpRec.nfields() > 0 ? factory()->line(*itsGridMinLines_[0]) : NULL);
        PlotLinePtr tmp= itsGridMinLines_.size() > 0 ? factory()->line(*itsGridMinLines_[0]) : NULL;
        for (unsigned int i= 0; i < itsGridMinLines_.size() && i < tmpRec.nfields(); i++)
            {
            if (tmpRec.dataType(i) == TpRecord)
                {
                tmp->fromRecord(tmpRec.asRecord(i));
                if (*itsGridMinLines_[i] != *tmp)
                    {
                    *itsGridMinLines_[i] = *tmp;
                    valuesChanged = true;
                    }
                }
            }
        }
    if (valuesChanged) updated();
    }


PlotMSPlotParameters::Group& PMS_PP_Canvas::operator=(const Group& other)
    {
    const PMS_PP_Canvas* o = dynamic_cast<const PMS_PP_Canvas*>(&other);
    if (o != NULL && *this != *o)
        {
        itsXLabels_ = o->itsXLabels_;
        itsYLabels_ = o->itsYLabels_;
        itsXAxesShown_ = o->itsXAxesShown_;
        itsYAxesShown_ = o->itsYAxesShown_;
        itsLegendsShown_ = o->itsLegendsShown_;
        itsLegendsPos_ = o->itsLegendsPos_;
        itsTitles_ = o->itsTitles_;
        itsGridMajsShown_ = o->itsGridMajsShown_;
        itsGridMinsShown_ = o->itsGridMinsShown_;
        itsGridMajLines_.resize(o->itsGridMajLines_.size());
        for (unsigned int i = 0; i < itsGridMajLines_.size(); i++) itsGridMajLines_[i] = factory()->line(*o->itsGridMajLines_[i]);
        itsGridMinLines_.resize(o->itsGridMinLines_.size());
        for (unsigned int i = 0; i < itsGridMinLines_.size(); i++) itsGridMinLines_[i] = factory()->line(*o->itsGridMinLines_[i]);
        updated();
        }
    return *this;
    }


bool PMS_PP_Canvas::operator==(const Group& other) const
    {
    const PMS_PP_Canvas* o = dynamic_cast<const PMS_PP_Canvas*>(&other);
    if (o == NULL) return false;
    if (itsXLabels_ != o->itsXLabels_) return false;
    if (itsYLabels_ != o->itsYLabels_) return false;
    if (itsXAxesShown_ != o->itsXAxesShown_) return false;
    if (itsYAxesShown_ != o->itsYAxesShown_) return false;
    if (itsLegendsShown_.size() != o->itsLegendsShown_.size() || itsLegendsPos_.size() != o->itsLegendsPos_.size() || itsLegendsShown_.size() != itsLegendsPos_.size()) return false;
    for (unsigned int i = 0; i < itsLegendsShown_.size(); i++) if (itsLegendsShown_[i] != o->itsLegendsShown_[i] || (itsLegendsShown_[i] && itsLegendsPos_[i] != o->itsLegendsPos_[i])) return false;
    if (itsTitles_ != o->itsTitles_) return false;
    if (itsGridMajsShown_.size() != o->itsGridMajsShown_.size() || itsGridMajLines_.size() != o->itsGridMajLines_.size() || itsGridMajsShown_.size() != itsGridMajLines_.size()) return false;
    for (unsigned int i = 0; i < itsGridMajsShown_.size(); i++) if (itsGridMajsShown_[i] != o->itsGridMajsShown_[i] || (itsGridMajsShown_[i] && *itsGridMajLines_[i] != *o->itsGridMajLines_[i])) return false;
    if (itsGridMinsShown_.size() != o->itsGridMinsShown_.size() || itsGridMinLines_.size() != o->itsGridMinLines_.size() || itsGridMinsShown_.size() != itsGridMinLines_.size()) return false;
    for (unsigned int i = 0; i < itsGridMinsShown_.size(); i++) if (itsGridMinsShown_[i] != o->itsGridMinsShown_[i] || (itsGridMinsShown_[i] && *itsGridMinLines_[i] != *o->itsGridMinLines_[i])) return false;
    return true;
    }


void PMS_PP_Canvas::setDefaults()
    {
    itsXLabels_ = vector<PlotMSLabelFormat>(1, PlotMSLabelFormat(PMS::DEFAULT_CANVAS_AXIS_LABEL_FORMAT));
    itsYLabels_ = vector<PlotMSLabelFormat>(1, PlotMSLabelFormat(PMS::DEFAULT_CANVAS_AXIS_LABEL_FORMAT));
    itsXAxesShown_ = vector<bool>(1, PMS::DEFAULT_SHOWAXIS);
    itsYAxesShown_ = vector<bool>(1, PMS::DEFAULT_SHOWAXIS);
    itsLegendsShown_ = vector<bool>(1, PMS::DEFAULT_SHOWLEGEND);
    itsLegendsPos_ = vector<PlotCanvas::LegendPosition>(1, PMS::DEFAULT_LEGENDPOSITION);
    itsTitles_ = vector<PlotMSLabelFormat>(1, PlotMSLabelFormat(PMS::DEFAULT_TITLE_FORMAT));
    itsGridMajsShown_ = vector<bool>(1, PMS::DEFAULT_SHOW_GRID);
    itsGridMinsShown_ = vector<bool>(1, PMS::DEFAULT_SHOW_GRID);
    itsGridMajLines_ = vector<PlotLinePtr>(1, PMS::DEFAULT_GRID_LINE(factory()));
    itsGridMinLines_ = vector<PlotLinePtr>(1, PMS::DEFAULT_GRID_LINE(factory()));
    }

unsigned int PMS_PP_Canvas::numCanvases() const
    {
    return itsXLabels_.size();
    }

void PMS_PP_Canvas::setLabelFormats(const PlotMSLabelFormat& xFormat,
                                    const PlotMSLabelFormat& yFormat, unsigned int index)
    {
    if (itsXLabels_[index] != xFormat || itsYLabels_[index] != yFormat)
        {
        itsXLabels_[index] = xFormat;
        itsYLabels_[index] = yFormat;
        updated();
        }
    }

void PMS_PP_Canvas::showAxes(const bool& xShow, const bool& yShow,
                             unsigned int index)
    {
    if (itsXAxesShown_[index] != xShow || itsYAxesShown_[index] != yShow)
        {
        itsXAxesShown_[index] = xShow;
        itsYAxesShown_[index] = yShow;
        updated();
        }
    }

void PMS_PP_Canvas::showLegend(const bool& show,
                               const PlotCanvas::LegendPosition& pos, unsigned int index)
    {
    if (itsLegendsShown_[index]!= show|| (show && itsLegendsPos_[index]!= pos))
        {
        itsLegendsShown_[index] = show;
        itsLegendsPos_[index] = pos;
        updated();
        }
    }

void PMS_PP_Canvas::showGrid(const bool& showMajor, const bool& showMinor,
                             const PlotLinePtr& majorLine, const PlotLinePtr& minorLine,
                             unsigned int index)
    {
    if (itsGridMajsShown_[index] != showMajor ||
            (showMajor && *itsGridMajLines_[index] != *majorLine) ||
            itsGridMinsShown_[index] != showMinor ||
            (showMinor && *itsGridMinLines_[index] != *minorLine))
        {
        itsGridMajsShown_[index] = showMajor;
        itsGridMinsShown_[index] = showMinor;
        *itsGridMajLines_[index] = *majorLine;
        *itsGridMinLines_[index] = *minorLine;
        updated();
        }
    }




////////////////////////////////
// PMS_PP_DISPLAY DEFINITIONS //
////////////////////////////////

// PMS_PP_Display record keys.
const String PMS_PP_Display::REC_UNFLAGGEDS = "unflaggedSymbols";
const String PMS_PP_Display::REC_FLAGGEDS = "flaggedSymbols";
const String PMS_PP_Display::REC_TITLES = "titles";
const String PMS_PP_Display::REC_COLFLAGS = "colorizeFlags";
const String PMS_PP_Display::REC_COLAXES = "colorizeAxes";


PMS_PP_Display::PMS_PP_Display(PlotFactoryPtr factory) : PlotMSPlotParameters::Group(factory)
    {
    setDefaults();
    } PMS_PP_Display::PMS_PP_Display(const PMS_PP_Display& copy) : PlotMSPlotParameters::Group(copy)
    {
    setDefaults();
    operator=(copy);
    } PMS_PP_Display::~PMS_PP_Display() { }


Record PMS_PP_Display::toRecord() const
    {
    Record rec;
        {
        Record tmpRec;
        for (unsigned int i = 0; i < itsUnflaggedSymbols_.size(); i++) tmpRec.defineRecord(i, itsUnflaggedSymbols_[i] ->toRecord());
        rec.defineRecord(REC_UNFLAGGEDS, tmpRec);
        }
        {
        Record tmpRec;
        for (unsigned int i = 0; i < itsFlaggedSymbols_.size(); i++) tmpRec.defineRecord(i, itsFlaggedSymbols_[i] ->toRecord());
        rec.defineRecord(REC_FLAGGEDS, tmpRec);
        }
        {
        Record tmpRec;
        for (unsigned int i = 0; i < itsTitleFormats_.size(); i++) tmpRec.define(i, itsTitleFormats_[i] .format);
        rec.defineRecord(REC_TITLES, tmpRec);
        }
    rec.define(REC_COLFLAGS, Vector<bool>(itsColorizeFlags_));
    rec.define(REC_COLAXES, PMS::toIntVector<PMS::Axis>(itsColorizeAxes_));
    return rec;
    }


void PMS_PP_Display::fromRecord(const Record& record)
    {
    bool valuesChanged = false;
    if (record.isDefined(REC_UNFLAGGEDS) && record.dataType(REC_UNFLAGGEDS) == TpRecord)
        {
        const Record& tmpRec = record.asRecord(REC_UNFLAGGEDS);
        itsUnflaggedSymbols_.resize(tmpRec.nfields(), tmpRec.nfields() > 0 ? factory()->symbol(*itsUnflaggedSymbols_[0]) : NULL);
        PlotSymbolPtr tmp= itsUnflaggedSymbols_.size() > 0 ? factory()->symbol(*itsUnflaggedSymbols_[0]) : NULL;
        for (unsigned int i= 0; i < itsUnflaggedSymbols_.size() && i < tmpRec.nfields(); i++)
            {
            if (tmpRec.dataType(i) == TpRecord)
                {
                tmp->fromRecord(tmpRec.asRecord(i));
                if (*itsUnflaggedSymbols_[i] != *tmp)
                    {
                    *itsUnflaggedSymbols_[i] = *tmp;
                    valuesChanged = true;
                    }
                }
            }
        }
    if (record.isDefined(REC_FLAGGEDS) && record.dataType(REC_FLAGGEDS) == TpRecord)
        {
        const Record& tmpRec = record.asRecord(REC_FLAGGEDS);
        itsFlaggedSymbols_.resize(tmpRec.nfields(), tmpRec.nfields() > 0 ? factory()->symbol(*itsFlaggedSymbols_[0]) : NULL);
        PlotSymbolPtr tmp= itsFlaggedSymbols_.size() > 0 ? factory()->symbol(*itsFlaggedSymbols_[0]) : NULL;
        for (unsigned int i= 0; i < itsFlaggedSymbols_.size() && i < tmpRec.nfields(); i++)
            {
            if (tmpRec.dataType(i) == TpRecord)
                {
                tmp->fromRecord(tmpRec.asRecord(i));
                if (*itsFlaggedSymbols_[i] != *tmp)
                    {
                    *itsFlaggedSymbols_[i] = *tmp;
                    valuesChanged = true;
                    }
                }
            }
        }
    if (record.isDefined(REC_TITLES) && record.dataType(REC_TITLES) == TpRecord)
        {
        const Record& tmpRec = record.asRecord(REC_TITLES);
        itsTitleFormats_.resize(tmpRec.nfields());
        for (unsigned int i= 0; i < itsTitleFormats_.size() && i < tmpRec.nfields(); i++)
            {
            if (tmpRec.dataType(i) == TpString && itsTitleFormats_[i] != tmpRec.asString(i))
                {
                itsTitleFormats_[i] = tmpRec.asString(i);
                valuesChanged = true;
                }
            }
        }
    if (record.isDefined(REC_COLFLAGS) && record.dataType(REC_COLFLAGS) == TpArrayBool)
        {
        vector<bool> tmp;
        record.asArrayBool(REC_COLFLAGS).tovector(tmp);
        if (itsColorizeFlags_ != tmp)
            {
            itsColorizeFlags_ = tmp;
            valuesChanged = true;
            }
        }
    if (record.isDefined(REC_COLAXES) && record.dataType(REC_COLAXES) == TpArrayInt)
        {
        vector<PMS::Axis> tmp = PMS::fromIntVector<PMS::Axis>(record.asArrayInt(REC_COLAXES));
        if (itsColorizeAxes_ != tmp)
            {
            itsColorizeAxes_ = tmp;
            valuesChanged = true;
            }
        }
    if (valuesChanged) updated();
    }


PlotMSPlotParameters::Group& PMS_PP_Display::operator=(const Group& other)
    {
    const PMS_PP_Display* o = dynamic_cast<const PMS_PP_Display*>(&other);
    if (o != NULL && *this != *o)
        {
        itsUnflaggedSymbols_.resize(o->itsUnflaggedSymbols_.size());
        for (unsigned int i = 0; i < itsUnflaggedSymbols_.size(); i++) itsUnflaggedSymbols_[i] = factory()->symbol(*o->itsUnflaggedSymbols_[i]);
        itsFlaggedSymbols_.resize(o->itsFlaggedSymbols_.size());
        for (unsigned int i = 0; i < itsFlaggedSymbols_.size(); i++) itsFlaggedSymbols_[i] = factory()->symbol(*o->itsFlaggedSymbols_[i]);
        itsTitleFormats_ = o->itsTitleFormats_;
        itsColorizeFlags_ = o->itsColorizeFlags_;
        itsColorizeAxes_ = o->itsColorizeAxes_;
        updated();
        }
    return *this;
    }


bool PMS_PP_Display::operator==(const Group& other) const
    {
    const PMS_PP_Display* o = dynamic_cast<const PMS_PP_Display*>(&other);
    if (o == NULL) return false;
    if (itsUnflaggedSymbols_.size() != o->itsUnflaggedSymbols_.size()) return false;
    for (unsigned int i = 0; i < itsUnflaggedSymbols_.size(); i++) if (*itsUnflaggedSymbols_[i] != *o->itsUnflaggedSymbols_[i]) return false;
    if (itsFlaggedSymbols_.size() != o->itsFlaggedSymbols_.size()) return false;
    for (unsigned int i = 0; i < itsFlaggedSymbols_.size(); i++) if (*itsFlaggedSymbols_[i] != *o->itsFlaggedSymbols_[i]) return false;
    if (itsTitleFormats_ != o->itsTitleFormats_) return false;
    if (itsColorizeFlags_.size() != o->itsColorizeFlags_.size() || itsColorizeAxes_.size() != o->itsColorizeAxes_.size() || itsColorizeFlags_.size() != itsColorizeAxes_.size()) return false;
    for (unsigned int i = 0; i < itsColorizeFlags_.size(); i++) if (itsColorizeFlags_[i] != o->itsColorizeFlags_[i] || (itsColorizeFlags_[i] && itsColorizeAxes_[i] != o->itsColorizeAxes_[i])) return false;
    return true;
    }


void PMS_PP_Display::setDefaults()
    {
    itsUnflaggedSymbols_ = vector<PlotSymbolPtr>(1, PMS::DEFAULT_UNFLAGGED_SYMBOL(factory()));
    itsFlaggedSymbols_ = vector<PlotSymbolPtr>(1, PMS::DEFAULT_FLAGGED_SYMBOL(factory()));
    itsTitleFormats_ = vector<PlotMSLabelFormat>(1, PlotMSLabelFormat(PMS::DEFAULT_TITLE_FORMAT));
    itsColorizeFlags_ = vector<bool>(1, false);
    itsColorizeAxes_ = vector<PMS::Axis>(1, PMS::DEFAULT_COLOR_AXIS);
    }

void PMS_PP_Display::setColorize(const bool& colorize, const PMS::Axis& axis,
                                 unsigned int index)
    {
    if (itsColorizeFlags_[index]!= colorize || itsColorizeAxes_[index]!= axis)
        {
        itsColorizeFlags_[index] = colorize;
        itsColorizeAxes_[index] = axis;
        updated();
        }
    }

void PMS_PP_Display::resizeVectors(unsigned int newSize)
    {
    if (newSize == 0) newSize = 1;
    itsUnflaggedSymbols_.resize(newSize);
    itsFlaggedSymbols_.resize(newSize);
    itsTitleFormats_.resize(newSize, PlotMSLabelFormat(PMS::DEFAULT_TITLE_FORMAT));
    itsColorizeFlags_.resize(newSize, false);
    itsColorizeAxes_.resize(newSize, PMS::DEFAULT_COLOR_AXIS);

    for (unsigned int i = 0; i < newSize; i++)
        {
        if (itsUnflaggedSymbols_[i].null())
            itsUnflaggedSymbols_[i] = PMS::DEFAULT_UNFLAGGED_SYMBOL(factory());
        if (itsFlaggedSymbols_[i].null())
            itsFlaggedSymbols_[i] = PMS::DEFAULT_FLAGGED_SYMBOL(factory());
        }
    }





//////////////////////////////////
// PMS_PP_ITERATION DEFINITIONS //
//////////////////////////////////

// PMS_PP_Iteration record keys.
const String PMS_PP_Iteration::REC_ENABLEITERATION = "enableIteration";
const String PMS_PP_Iteration::REC_ITERATIONAXIS = "iterationAxis";
const String PMS_PP_Iteration::REC_NUMROWS = "numRows";
const String PMS_PP_Iteration::REC_NUMCOLUMNS = "numColumns";
const String PMS_PP_Iteration::REC_XAXISSHARINGMODE = "xAxisSharingMode";
const String PMS_PP_Iteration::REC_YAXISSHARINGMODE = "yAxisSharingMode";



PMS_PP_Iteration::PMS_PP_Iteration(PlotFactoryPtr factory) : PlotMSPlotParameters::Group(factory)
    {
    setDefaults();
    } PMS_PP_Iteration::PMS_PP_Iteration(const PMS_PP_Iteration& copy) : PlotMSPlotParameters::Group(copy)
    {
    setDefaults();
    operator=(copy);
    } PMS_PP_Iteration::~PMS_PP_Iteration() { }



Record PMS_PP_Iteration::toRecord() const
    {
    Record rec;
    rec.define(REC_ENABLEITERATION, itsEnableIteration_);
    rec.define(REC_ITERATIONAXIS, itsIterationAxis_);
    rec.define(REC_NUMROWS, itsNumRows_);
    rec.define(REC_NUMCOLUMNS, itsNumColumns_);
    rec.define(REC_XAXISSHARINGMODE, itsXAxisSharingMode_);
    rec.define(REC_YAXISSHARINGMODE, itsYAxisSharingMode_);
    return rec;
    }



void PMS_PP_Iteration::fromRecord(const Record& record)
    {
    bool valuesChanged = false;
    if (record.isDefined(REC_ENABLEITERATION) && record.dataType(REC_ENABLEITERATION) == TpBool && itsEnableIteration_ != record.asBool(REC_ENABLEITERATION))
        {
        itsEnableIteration_ = record.asBool(REC_ENABLEITERATION);
        valuesChanged = true;
        }
    if (record.isDefined(REC_ITERATIONAXIS) && record.dataType(REC_ITERATIONAXIS) == TpInt && itsIterationAxis_ != (PMS::Axis)record.asInt(REC_ITERATIONAXIS))
        {
        itsIterationAxis_ = (PMS::Axis)record.asInt(REC_ITERATIONAXIS);
        valuesChanged = true;
        }
    if (record.isDefined(REC_NUMROWS) && record.dataType(REC_NUMROWS) == TpInt && itsNumRows_ != record.asInt(REC_NUMROWS))
        {
        itsNumRows_ = record.asInt(REC_NUMROWS);
        valuesChanged = true;
        }
    if (record.isDefined(REC_NUMCOLUMNS) && record.dataType(REC_NUMCOLUMNS) == TpInt && itsNumColumns_ != record.asInt(REC_NUMCOLUMNS))
        {
        itsNumColumns_ = record.asInt(REC_NUMCOLUMNS);
        valuesChanged = true;
        }
    if (record.isDefined(REC_XAXISSHARINGMODE) && record.dataType(REC_XAXISSHARINGMODE) == TpInt && itsXAxisSharingMode_ != record.asInt(REC_XAXISSHARINGMODE))
        {
        itsXAxisSharingMode_ = (AxisSharingMode)record.asInt(REC_XAXISSHARINGMODE);
        valuesChanged = true;
        }
    if (record.isDefined(REC_YAXISSHARINGMODE) && record.dataType(REC_YAXISSHARINGMODE) == TpInt && itsYAxisSharingMode_ != record.asInt(REC_YAXISSHARINGMODE))
        {
        itsYAxisSharingMode_ = (AxisSharingMode)record.asInt(REC_YAXISSHARINGMODE);
        valuesChanged = true;
        }
    if (valuesChanged) updated();
    }



PlotMSPlotParameters::Group& PMS_PP_Iteration::operator=(const Group& other)
    {
    const PMS_PP_Iteration* o = dynamic_cast<const PMS_PP_Iteration*>(&other);
    if (o != NULL && *this != *o)
        {
        itsEnableIteration_ = o->itsEnableIteration_;
        itsIterationAxis_ = o->itsIterationAxis_;
        itsNumRows_ = o->itsNumRows_;
        itsNumColumns_ = o->itsNumColumns_;
        itsXAxisSharingMode_ = o->itsXAxisSharingMode_;
        itsYAxisSharingMode_ = o->itsYAxisSharingMode_;
        updated();
        }
    return *this;
    }



bool PMS_PP_Iteration::operator==(const Group& other) const
    {
    const PMS_PP_Iteration* o = dynamic_cast<const PMS_PP_Iteration*>(&other);
    if (o == NULL) return false;
    if (itsEnableIteration_ != o->itsEnableIteration_) return false;
    if (itsIterationAxis_ != o->itsIterationAxis_) return false;
    if (itsNumRows_ != o->itsNumRows_) return false;
    if (itsNumColumns_ != o->itsNumColumns_) return false;
    if (itsXAxisSharingMode_ != o->itsXAxisSharingMode_) return false;
    if (itsYAxisSharingMode_ != o->itsYAxisSharingMode_) return false;
    return true;
    }



void PMS_PP_Iteration::setDefaults()
    {
    itsEnableIteration_ = 0;
    itsIterationAxis_ = PMS::DEFAULT_COLOR_AXIS;
    itsNumRows_ = 1;
    itsNumColumns_ = 1;
    itsXAxisSharingMode_ = COMMON_AXIS;
    itsYAxisSharingMode_ = COMMON_AXIS;
    }


}
