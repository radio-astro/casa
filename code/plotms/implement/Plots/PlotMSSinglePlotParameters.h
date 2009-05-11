//# PlotMSSinglePlotParameters.h: Subclass for PlotMSSinglePlot.
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
#ifndef PLOTMSSINGLEPLOTPARAMETERS_H_
#define PLOTMSSINGLEPLOTPARAMETERS_H_

#include <graphics/GenericPlotter/PlotFactory.h>
#include <plotms/PlotMS/PlotMSLabelFormat.h>
#include <plotms/Plots/PlotMSPlotParameters.h>

#include <casa/namespace.h>

namespace casa {

// Specialized parameters for PlotMSSinglePlot.  Canvas parameters apply to all
// canvases associated with the plot.  Contains the following parameters:
// * which two plot axes (and data columns) are being used.
// * which two canvas axes the plot is attached to,
// * custom axes ranges if desired,
// * the formats for the two canvas axes labels,
// * if the two canvas axes are shown/hidden,
// * legend hidden/shown and position,
// * the format for the canvas title,
// * the normal and masked plot symbols, and
// * the format for the plot title.
class PlotMSSinglePlotParameters : public PlotMSPlotParameters {    
public:
    // Constructor which uses the given factory to set the parameters to
    // defaults.
    PlotMSSinglePlotParameters(PlotFactoryPtr factory,
            const String& msFilename = "");
    
    // Constructor which uses the given parent to get a PlotFactory to set the
    // parameters to defaults.
    PlotMSSinglePlotParameters(PlotMS* parent, const String& msFilename = "");
    
    // Copy constructor.  See operator=().
    PlotMSSinglePlotParameters(const PlotMSSinglePlotParameters& copy);
    
    // Destructor.
    ~PlotMSSinglePlotParameters();
    
    
    // Include overloaded methods.
    using PlotMSPlotParameters::operator=;
    
    
    // Overrides PlotMSPlotParameters::equals().  Will return false if the
    // other parameters are not of type PlotMSSinglePlotParameters.  Checks
    // PlotMSPlotParameters::equals(), then all set parameters.
    bool equals(const PlotMSWatchedParameters& other, int updateFlags) const;
    
    
    // Gets/Sets plot axes.
    // <group>
    PMS::Axis xAxis() const;
    PMS::Axis yAxis() const;
    void setXAxis(PMS::Axis axis) { setAxes(axis, yAxis()); }
    void setYAxis(PMS::Axis axis) { setAxes(xAxis(), axis); }
    void setAxes(PMS::Axis x, PMS::Axis y) {
        setAxesAndDataColumns(x, xDataColumn(), y, yDataColumn()); }
    // </group>
    
    // Gets/Sets axes data columns.
    // <group>
    PMS::DataColumn xDataColumn() const;
    PMS::DataColumn yDataColumn() const;
    void setXDataColumn(PMS::DataColumn data) {
        setDataColumns(data, yDataColumn()); }
    void setYDataColumn(PMS::DataColumn data) {
        setDataColumns(xDataColumn(), data); }
    void setDataColumns(PMS::DataColumn x, PMS::DataColumn y) {
        setAxesAndDataColumns(xAxis(), x, yAxis(), y); }
    // </group>
    
    // Sets plot axes and data columns.
    void setAxesAndDataColumns(PMS::Axis x, PMS::DataColumn xData, PMS::Axis y,
            PMS::DataColumn yData);    
    
    // Gets/Sets which canvas axes the plot is attached to.
    // <group>
    PlotAxis canvasXAxis() const;
    PlotAxis canvasYAxis() const;
    void setCanvasXAxis(PlotAxis axis) { setCanvasAxes(axis, canvasYAxis()); }
    void setCanvasYAxis(PlotAxis axis) { setCanvasAxes(canvasXAxis(), axis); }
    void setCanvasAxes(PlotAxis xAxis, PlotAxis yAxis);
    // </group>
    
    // Gets/Sets axes reference values.
    // <group>
    bool xReferenceValueSet() const;
    bool yReferenceValueSet() const;
    double xReferenceValue() const;
    double yReferenceValue() const;
    void setXReferenceValue(bool set, double value = 0) {
    	setReferenceValues(set,value,yReferenceValueSet(),yReferenceValue()); }
    void setYReferenceValue(bool set, double value = 0) {
    	setReferenceValues(xReferenceValueSet(),xReferenceValue(),set,value); }
    void setReferenceValues(bool xSet, double xValue, bool ySet,double yValue);
    // </group>
    
    // Gets/Sets axes ranges.
    // <group>
    bool xRangeSet() const;
    bool yRangeSet() const;
    pair<double, double> xRange() const;
    pair<double, double> yRange() const;
    void setXRange(bool set, pair<double, double> range) {
        setRanges(set, range, yRangeSet(), yRange()); }
    void setYRange(bool set, pair<double, double> range) {
        setRanges(xRangeSet(), xRange(), set, range); }
    void setXRange(pair<double, double> range) { setXRange(true, range); }
    void setYRange(pair<double, double> range) { setYRange(true, range); }
    void setRanges(bool xSet, pair<double, double> xRange, bool ySet,
            pair<double, double> yRange);
    // </group>
    
    
    // Gets/Sets the formats for generating the canvas axes labels.
    // <group>
    const PlotMSLabelFormat& canvasXAxisLabelFormat() const;
    const PlotMSLabelFormat& canvasYAxisLabelFormat() const;
    void setCanvasXAxisLabelFormat(const PlotMSLabelFormat& format) {
        setCanvasAxesLabelFormats(format, canvasYAxisLabelFormat()); }
    void setCanvasXAxisLabelFormat(const String& format) {
        setCanvasXAxisLabelFormat(PlotMSLabelFormat(format)); }
    void setCanvasYAxisLabelFormat(const PlotMSLabelFormat& format) {
        setCanvasAxesLabelFormats(canvasXAxisLabelFormat(), format); }
    void setCanvasYAxisLabelFormat(const String& format) {
        setCanvasYAxisLabelFormat(PlotMSLabelFormat(format)); }
    void setCanvasAxesLabelFormats(const PlotMSLabelFormat& xFormat,
            const PlotMSLabelFormat& yFormat);
    void setCanvasAxesLabelFormats(const String& xForm, const String& yForm) {
        setCanvasAxesLabelFormats(PlotMSLabelFormat(xForm),
                                  PlotMSLabelFormat(yForm));
    }
    // </group>
    
    // Gets the canvas axes labels using the set formats and axes.
    // <group>
    String canvasXAxisLabel() const;
    String canvasYAxisLabel() const;
    // </group>
    
    // Gets/Sets whether the canvas axes are shown or hidden.
    // <group>
    bool showXAxis() const;
    bool showYAxis() const;
    void setShowXAxis(bool show) { setShowAxes(show, showYAxis()); }
    void setShowYAxis(bool show) { setShowAxes(showXAxis(), show); }
    void setShowAxes(bool showX, bool showY);
    // </group>
    
    // Gets/Sets whether the canvas legend is shown and its position.
    // <group>
    bool showLegend() const;
    PlotCanvas::LegendPosition legendPosition() const;
    void setShowLegend(bool show) { setLegend(show, legendPosition()); }
    void setLegendPosition(PlotCanvas::LegendPosition pos) {
        setLegend(showLegend(), pos); }
    void setLegend(bool show, PlotCanvas::LegendPosition position);
    // </group>
    
    // Gets/Sets the format for generating the canvas's title.
    // <group>
    const PlotMSLabelFormat& canvasTitleFormat() const;
    void setCanvasTitleFormat(const PlotMSLabelFormat& format);
    void setCanvasTitleFormat(const String& format) {
        setCanvasTitleFormat(PlotMSLabelFormat(format)); }
    // </group>
    
    // Gets the canvas's title using the set format and axes.
    String canvasTitle() const;
    
    // Gets/Sets whether grid major/minor lines are shown.
    // <group>
    bool showGridMajor() const;
    bool showGridMinor() const;
    void setShowGrid(bool major, bool minor);
    // </group>
    
    // Gets/Sets grid major/minor lines.
    // <group>
    PlotLinePtr gridMajorLine() const;
    PlotLinePtr gridMinorLine() const;
    void setGridLines(PlotLinePtr major, PlotLinePtr minor);
    // </group>

    
    // Gets/Sets the symbol for normal and masked points.
    // <group>
    PlotSymbolPtr symbol() const;
    PlotSymbolPtr maskedSymbol() const;
    void setSymbol(PlotSymbolPtr sym) { setSymbols(sym, maskedSymbol()); }
    void setMaskedSymbol(PlotSymbolPtr sym) { setSymbols(symbol(), sym); }
    void setSymbols(PlotSymbolPtr normal, PlotSymbolPtr masked);
    // </group>
    
    // Gets/Sets the format for generating the plot's title.
    // <group>
    const PlotMSLabelFormat& plotTitleFormat() const;
    void setPlotTitleFormat(const PlotMSLabelFormat& format);
    void setPlotTitleFormat(const String& format) {
        setPlotTitleFormat(PlotMSLabelFormat(format)); }
    // </group>
    
    // Gets the plot's title using the set format and axes.
    String plotTitle() const;
    
    
    // Copy operator.  See PlotMSPlotParameters::operator=().
    PlotMSSinglePlotParameters& operator=(const PlotMSSinglePlotParameters& p);    
    
protected:
    // Sets the parameters to their defaults.
    void setDefaults(PlotFactoryPtr factory);    
    
private:
    // Plot axes.
    PMS::Axis itsXAxis_, itsYAxis_;
    
    // Plot axes data columns.
    PMS::DataColumn itsXDataColumn_, itsYDataColumn_;
    
    // Canvas axes.
    PlotAxis itsXCanvasAxis_, itsYCanvasAxis_;
    
    // Axes reference values.
    // <group>
    bool itsXReferenceValueSet_, itsYReferenceValueSet_;
    double itsXReferenceValue_, itsYReferenceValue_;
    // </group>
    
    // Axes ranges.
    // <group>
    bool itsXRangeSet_, itsYRangeSet_;
    pair<double, double> itsXRange_, itsYRange_;
    // </group>
    
    // Axes label formats.
    PlotMSLabelFormat itsCanvasXAxisLabelFormat_, itsCanvasYAxisLabelFormat_;
    
    // Show axes flags.
    bool itsShowXAxis_, itsShowYAxis_;
    
    // Legend flag.
    bool itsShowLegend_;
    
    // Legend position.
    PlotCanvas::LegendPosition itsLegendPos_;
    
    // Canvas title format.
    PlotMSLabelFormat itsCanvasTitleFormat_;
    
    // Show grid major/minor lines.
    bool itsCanvasGridMajor_, itsCanvasGridMinor_;
    
    // Grid major/minor lines.
    PlotLinePtr itsCanvasGridMajorLine_, itsCanvasGridMinorLine_;
    
    // Normal and masked symbols.
    PlotSymbolPtr itsSymbol_, itsMaskedSymbol_;
    
    // Plot title format.
    PlotMSLabelFormat itsPlotTitleFormat_;
};

}

#endif /* PLOTMSSINGLEPLOTPARAMETERS_H_ */
