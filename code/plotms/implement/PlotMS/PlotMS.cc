//# PlotMS.cc: Main controller for plotms.
//# Copyright (C) 2008
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
//# $Id: $
#include <plotms/PlotMS/PlotMS.h>

namespace casa {

// TODO casaplotms: add ms selection to command-line parameters
// TODO PlotMSAction: implement flag, unflag, iteration, release cache.  action
//      for new plots.
// TODO PlotMSCache: multi-region locate
// TODO PlotMSData: have stack list of caches so that common MS/Selections can
//                  be shared across different plots
// TODO PlotMSLogger: log source (std out, text widget, casapy logger), better
//                    locate message, log message for parameters updated, log
//                    message for # of plotted points
// TODO PlotMSParameters: canvas background, fonts, grids, spacing
// TODO PlotMSPlot: different colors within one plot, use threading
// TODO PlotMSPlotManager: unused canvases
// TODO PlotMSPlotter: range padding, customize toolbars, override close event
//                     to call PlotMS::close() in case there's cleanup
// TODO PlotMSThread: background, pause/resume, cancel; export thread
// TODO PlotMSWidgets: disable size for pixel, label creator
// TODO PlotTool: make scrolling on standard tool group still do stack movement
//                even when none tool is selected, display tracker value as
//                time when needed, set tracker font

////////////////////////
// PLOTMS DEFINITIONS //
////////////////////////

// Static //

const String PlotMS::CLASS_NAME = "PlotMS";

const String PlotMS::LOG_INITIALIZE_GUI = "initialize_gui";
const String PlotMS::LOG_LOAD_CACHE = "load_cache";
const String PlotMS::LOG_LOCATE = "locate";


// Constructors/Destructors //

PlotMS::PlotMS() { initialize(); }

PlotMS::PlotMS(const PlotMSParameters& params) : itsParameters_(params) {
    // Update internal state to reflect parameters.
    parametersHaveChanged(itsParameters_, PlotMSWatchedParameters::ALL, false);
    
    // We have to make a temporary PlotLogger to get measurements for the GUI
    // initialization, since PlotMSLogger is waiting for the PlotMSPlotter to
    // be built first to get a pointer to its logger!
    PlotLogger* tempLogger = NULL;
    if(itsLogMeasurementFlag_ & PlotMSLogger::INITIALIZE_GUI)
        tempLogger = new PlotLogger(NULL); // should be okay to pass NULL for a
                                           // Plotter since we're just
                                           // generating event messsages..
    
    PlotLogGeneric msg = (tempLogger == NULL) ? PlotLogGeneric("", "", "") :
        tempLogger->markMeasurement(CLASS_NAME, LOG_INITIALIZE_GUI, false);
    
    initialize();
    
    // Post message if necessary, and clean up.
    if(tempLogger != NULL) {
        itsLogger_.postMessage(msg);
        itsLogger_.postMessage(tempLogger->releaseMeasurement(false));
        delete tempLogger;
    }
}

PlotMS::~PlotMS() {
    itsPlotManager_.clearPlotsAndCanvases();
    delete itsPlotter_;
}


// Public Methods //

void PlotMS::parametersHaveChanged(const PlotMSWatchedParameters& params,
            int updateFlag, bool redrawRequired) {
    if(&params == &itsParameters_) {
        // We only care about PlotMS's parameters.
        itsLogMeasurementFlag_ = PlotMSLogger::levelToMeasurementFlag(
                                 itsParameters_.logLevel());
        itsLogger_.setMeasurementEvents_(itsLogMeasurementFlag_);
    }
}

PlotMSLogger* PlotMS::loggerFor(PlotMSLogger::MeasurementEvent event) {
    if(itsLogMeasurementFlag_ & event) return &itsLogger_;
    else                               return NULL;
}

void PlotMS::triggerAction(PlotMSAction::Type type) {
    PlotMSAction* act = PlotMSAction::action(type, itsPlotter_);
    if(act == NULL) return;
    act->doAction(this);
    delete act;
}


// Private Methods //

void PlotMS::initialize() {
    itsParameters_.addWatcher(this);
    
    itsPlotter_ = new PlotMSPlotter(this);
    itsPlotter_->showIterationButtons(false);
    itsPlotter_->setStatusText("Opened blank plotter.");
    
    itsLogger_.setParent(this);
    itsPlotManager_.setParent(this);
    
    // Update internal state to reflect parameters.
    parametersHaveChanged(itsParameters_, PlotMSWatchedParameters::ALL, false);
}


// OLD STUFF //

/*
void PlotMS::setParameters(const PlotMSParameters& params) {
    bool updated = false;
    stringstream status;
    
    // update rows/cols
    unsigned int rows = opts.nrows(), cols = opts.ncols();
    if(rows != itsOptions_.nrows() || cols != itsOptions_.ncols()) {
        itsPlotter_->setGridDimensions(rows, cols);
        itsData_.setRowsCols(rows, cols, false);
        status << "Updated canvas grid to " << rows << " x " << cols << ".";
        updated = true;
    }
    
    unsigned int spacing = opts.spacing();
    if(spacing != itsOptions_.spacing()) itsPlotter_->setGridSpacing(spacing);
    
    unsigned int index = 0;
    vector<vector<bool> > updateMS(rows), updateAxes(rows),
                          updateSettings(rows);
    for(unsigned int r = 0; r < rows; r++) {
        updateMS[r].resize(cols, true);
        updateAxes[r].resize(cols, true);
        updateSettings[r].resize(cols, true);
        for(unsigned int c = 0; c < cols; c++) {
            if(index < itsOptions_.nrows() * itsOptions_.ncols()) {
                updateMS[r][c] = !opts.parameters(r, c).equalsMS(
                                 itsOptions_.parameters(index));
                updateAxes[r][c] = !opts.parameters(r, c).equalsAxes(
                                   itsOptions_.parameters(index));
                updateSettings[r][c] = !opts.parameters(r, c).equalsSettings(
                                        itsOptions_.parameters(index));
                index++;
            } else {
                updateMS[r][c]= updateAxes[r][c]= opts.parameters(r,c).isSet();
            }
        }
    }
    
    if(opts.logLevel() != itsOptions_.logLevel())
        setLogLevel(opts.logLevel());
        
    itsOptions_ = opts;

    String result;
    for(unsigned int r = 0; r < rows; r++) {
        for(unsigned int c = 0; c < cols; c++) {
            result = update(updateSettings[r][c], updateMS[r][c],
                            updateAxes[r][c], r, c);
            if(!result.empty()) {
                if(updated) status << "  ";
                status << result;
                updated = true;
            }
        }
    }
    
    itsPlotter_->updateOptions();
    if(updated) itsPlotter_->setStatusText(status.str());
    
    return true;
}

String PlotMS::update(bool settings, bool ms, bool axes, unsigned int r,
        unsigned int c) {
    if(!settings && !ms && !axes) return "";

    stringstream ss;
    bool ssupdated = false;
    
    const PlotMSPlotParameters& params = itsOptions_.parameters(r, c);
    bool set = params.isSet();
    PMS::Axis xAxis, yAxis;
    String color = "blue";
    String title;
    
    bool isHeld = itsPlotter_->drawingIsHeld(r, c);
    if(!isHeld) itsPlotter_->holdDrawing(r, c);
    
    if(settings) {
        itsPlotter_->setCanvasTitle(params.getCanvasTitle(), r, c);
        itsPlotter_->setGrid(set && params.getGridXMajor(),
                             set && params.getGridXMinor(),
                             set && params.getGridYMajor(),
                             set && params.getGridYMinor(), r, c);
        itsPlotter_->showAxes(set && params.showXAxis(),
                              set && params.showYAxis(), r, c);
        itsPlotter_->showLegend(r, c, set && params.showLegend(),
                                params.getLegendPosition());
    }
    
    if(ms || axes) {
        // Clear plot.
        if(!set) {
            itsPlotter_->clear(r, c);
            itsData_.clearCache(r, c);
            
            if(ssupdated) ss << " ";
            ss << "Cleared plot items";
            if(itsOptions_.nrows() > 1 || itsOptions_.ncols() > 1)
                ss << " on (" << r << ", " << c << ")";
            ss << ".";
            ssupdated = true;
            
            if(!isHeld) itsPlotter_->releaseDrawing(r, c);
            
            return ss.str();            
        }
        
        // Changed MS/Selection and/or axes.
        PlotMSLogger* log = loggerFor(PlotMSLogger::LOAD_CACHE);
        if(log != NULL) log->markMeasurement(CLASS_NAME, LOG_LOAD_CACHE);
            
        // Only open MS if filename or selection has changed.
        if(ms) {
<<<<<<< .mine
            itsData_.clearCache(r, c);
            String openError = openMS(r, c);
            if(!openError.empty()) {
=======
            try {
                // IMPORTANT: clear cache
                itsData_.clearCache(r, c);
                
                // Open MS and lock until we're done.
                itsMS_ = MeasurementSet(params.getFilename(),
                        TableLock(TableLock::AutoLocking), Table::Update);

                // Apply the MS selection.
		Matrix<Int> chansel;
                params.getSelection().apply(itsMS_, itsSelectedMS_,chansel);
                    
                // Sort appropriately.
                double solint(DBL_MAX);
                double interval(max(solint, DBL_MIN));
                if(solint < 0) interval = 0;
                  
                Block<Int> columns(5);
                columns[0]=MS::ARRAY_ID;
                columns[1]=MS::SCAN_NUMBER;  // force scan boundaries
                columns[2]=MS::FIELD_ID;      
                columns[3]=MS::DATA_DESC_ID;  // force 
                columns[4]=MS::TIME;
                    
                // Open VisSet.
                if(itsVisSet_ != NULL) delete itsVisSet_;
                itsVisSet_ = new VisSet(itsSelectedMS_, columns, Matrix<int>(),
                                        interval);
		itsVisSet_->selectChannel(chansel);

            } catch(AipsError& err) {
>>>>>>> .r7131
                if(log != NULL) log->releaseMeasurement();
                itsLastError_ = "Could not open MS: " + openError;
                if(!isHeld) itsPlotter_->releaseDrawing(r, c);
                showError(itsLastError_);
                return itsLastError_;
            }
        }
        
        // Always ask cache to reload axes if needed.
        xAxis = params.getXAxis(); yAxis = params.getYAxis();
        try {
            itsData_.loadCache(*itsVisSet_, xAxis, yAxis,
                    params.getXDataColumn(), params.getYDataColumn());
            itsData_.setupCache(xAxis, yAxis, r, c);
        } catch(AipsError& er) {
            if(log != NULL) log->releaseMeasurement();
            itsLastError_ = "Could not load cache: " + er.getMesg();
            if(!isHeld) itsPlotter_->releaseDrawing(r, c);
            showError(itsLastError_);
            return itsLastError_;
        }
        if(log != NULL) log->releaseMeasurement();

        // Plot data.
        itsPlotter_->setAxesTypes(PMS::axisScale(xAxis),
                                  PMS::axisScale(yAxis), r, c);
        title = PMS::axis(xAxis) + " vs. " + PMS::axis(yAxis);
        itsPlotter_->plotData(itsData_, color, title, false, r, c);
        
        if(ssupdated) ss << " ";
        ss << "Plotted " << itsData_.size(r, c) << " points";
        if(itsOptions_.nrows() > 1 || itsOptions_.ncols() > 1)
            ss << " on (" << r << ", " << c << ')';
        ss << ".";
        ssupdated = true;
    }
    
    if(!isHeld) itsPlotter_->releaseDrawing(r, c);
    
    return ss.str();
}
*/

/*
PlotMS::PlotMS() :
  ms_p(0), 
  mssel_p(0),
  vs_p(NULL),
  dataStep_p(1),
  //xvals(0),
  //yvals(0),
  valid_vals(false),
  binwidth(1),
  xcalculator(&PlotMS::pc_time),
  ycalculator(&PlotMS::pc_amp),
  axis_unit(AxisUnit::lookup()),
  have_plotted(false),
  niterations(1),                   // HACK!
  itsPlotMSPlotter(NULL),
  itsDataColumn(dcData),
  rownum(&PlotMS::rownum_horizontal),
  colnum(&PlotMS::colnum_horizontal) 
  //histLockCounter_p(), 
  //hist_p(0)
{
  string2calculator["time"]     = &PlotMS::pc_time;
  string2calculator["uvdist"]       = &PlotMS::pc_uvdist;
  string2calculator["channel"]      = &PlotMS::pc_channel;
  string2calculator["corr"]     = &PlotMS::pc_corr;
  string2calculator["frequency"]    = &PlotMS::pc_frequency;
  string2calculator["velocity"]     = &PlotMS::pc_vel_relativistic;
  string2calculator["vel_radio"]    = &PlotMS::pc_vel_radio;
  string2calculator["vel_optical"]  = &PlotMS::pc_vel_optical;
  string2calculator["u"]        = &PlotMS::pc_u;
  string2calculator["v"]        = &PlotMS::pc_v;
  string2calculator["w"]        = &PlotMS::pc_w;
  string2calculator["azimuth"]      = &PlotMS::pc_azimuth;
  string2calculator["elevation"]    = &PlotMS::pc_elevation;
  string2calculator["baseline"]     = &PlotMS::pc_baseline;
  string2calculator["hourangle"]    = &PlotMS::pc_hourangle;
  string2calculator["parallacticangle"] = &PlotMS::pc_parallacticangle;
  string2calculator["amp"]      = &PlotMS::pc_amp;
  string2calculator["phase"]        = &PlotMS::pc_phase;
  string2calculator["real"]     = &PlotMS::pc_real;
  string2calculator["imag"]     = &PlotMS::pc_imag;
  string2calculator["weight"]       = &PlotMS::pc_weight;
  //PlotCalcFunc pc_x;      // antenna positions, like plotants   

  set_pol_selection("I");
  
  itsPlotMSPlotter = new PlotMSPlotter();
}

int PlotMS::execLoop() {        
    //String FnCall = "(x, y, xcolumn, ycolumn, iteration, filename)";
    //String fnname = "plot";
    //log->FnEnter(fnname + FnCall, clname);

    //logSink() << LogMessage::NORMAL2 << "Starting PlotMS::plot("
    //          << x << ", " << y << ")." << LogIO::POST;
    
    // Note that we're not just generating errors.  Inputs are being parsed!
    //std::pair<String, String> errors(parseInputs(x, y));
    //if(errors.first.length() > 0)
    //  logSink() << LogMessage::NORMAL2 << "PlotMS::plot:" << errors.first
    //            << LogIO::POST;
    //if(errors.second.length() > 0){    
   //   logSink() << LogMessage::SEVERE << "PlotMS::plot:" << errors.second
    //            << LogIO::POST;
    //  return false; 
    //}
             
    //String msg = "Preparing data ...      ";
    // MsPlot.h gets log from SLog.  I don't know exactly what a SLog is.
    ////log->out(msg, fnname, clname, LogMessage::NORMAL);


    
    try {
      //Vector<String> selectionStr;
      //String         plottitlebase;
      //String         plottitle;

      //if(niterations > 1){
        //// plottitlebase += iteration_type; // TO-DO: baseline, antenna, etc.
        ////itsPlotMSPlotter->changeGuiButtonState("iternext", "enabled");
      //}
      //else
      //  plottitle = plottitlebase;

      //for (uInt iteration = 0; iteration < niterations; ++iteration){
  //       if(iteration == 1)
  //  itsPlotMSPlotter->changeGuiButtonState("iterprev", "enabled");
  //       if(iteration == niterations - 1)
  //  itsPlotMSPlotter->changeGuiButtonState("iternext", "disabled");
        
        //# Make subselection for iteration.

        //calcpoints();
      
        //logSink() << LogMessage::NORMAL2 << "Plotting " << xvals.size()
        //  << " points." << LogIO::POST;
             
        //FIX if(niterations > 1)
        //  plottitle = plottitlebase << iteration + 1;
        
        //# Note that overplots are allowed with iteration even though
        //# they seem to be at cross-purposes.
        //if(xvals.nelements() == yvals.nelements()) {
      //itsPlotMSPlotter->plotxy(xvals, yvals,
                   //itsPlotOptions.color(iteration),
                  // plottitle, itsPlotOptions.overplot,
                   //(this->*rownum)(iteration),
                   //(this->*colnum)(iteration));
        //}
        //else{
      //if((yvals.nelements() % xvals.nelements()) == 0){
        //uInt nxs = xvals.nelements();
        //uInt nys = yvals.nelements() / nxs;
        //Vector<double> everynyth;
        
        // yvals contains multiple correlations, so split it up.
        //for(uInt i = 0; i < nys; ++i){
          //everynyth.reference(yvals(Slice(i, nxs, nys)));
          
          //itsPlotMSPlotter->plotxy(xvals, everynyth,
          //             itsPlotOptions.color(iteration),
          //             plottitle, itsPlotOptions.overplot,
          //             (this->*rownum)(iteration),
          //             (this->*colnum)(iteration));
        //}
      //}
      //else if((xvals.nelements() % yvals.nelements()) == 0){
      //  uInt nys = yvals.nelements();
      //  uInt nxs = xvals.nelements() / nys;
      //  Vector<double> everynxth;

        // xvals contains multiple correlations, so split it up.
     //   for(uInt i = 0; i < nys; ++i){
      //    everynxth.reference(xvals(Slice(i, nys, nxs)));
      //    itsPlotMSPlotter->plotxy(everynxth, yvals,
      //                 itsPlotOptions.color(iteration),
      //                 plottitle, itsPlotOptions.overplot,
      //                 (this->*rownum)(iteration),
       //                (this->*colnum)(iteration));
      //  }
    //  }
     // else{
      //  throw(AipsError(String("The number of xs (") + xvals.nelements()
      //          + String(") does not match the number of ys (")
       //         + yvals.nelements() + String(").")));
    //  }
     //   }
     // }
    } catch(AipsError& ae) {
  ////     if(iteration.nelements() == 0)
  ////       cleanupOptions();
      itsLastError_ = ae.getMesg();
      return -1;
    }

    return itsPlotter_->execLoop();
    
    //if(rstat) rstat = itsPlotMSPlotter->execLoop() == 0;
    
    //if(!rstat){
    //  //# TO-DO?? reset various flags because there is a serious
    //  //# error and we need to redo things??
    //}
      
  ////   if(iteration.nelements() == 0)   
  ////     cleanupOptions();

    ////log->FnExit(fnname, clname);
    //return rstat;
}

bool PlotMS::loadData(unsigned int row, unsigned int col) {
    const PlotMSPlotParameters& params = itsOptions_.parameters(row, col);
    
    try { 
        VisIter& vi = itsVisSet_->iter();
        VisBuffer vb(vi);
        
        // A "blank" vector that gets passed by reference to, and returned by,
        // the vi functions called by (x, y)calculator.  I assume they do that
        // to avoid instantiating and resizing a Vector all the time.  They
        // will resize data as needed.
        Vector<Double> data;

        // Ensure correlations restored to original order?
        // (this is a no-op if no sort necessary)
        //// vb.unSortCorr();

        // Get count so we'll only need to size once.
        unsigned int xcount = 0, ycount = 0;
        for(vi.originChunks(); vi.moreChunks(); vi.nextChunk()) {
            for(vi.origin(); vi.more(); vi++) {
                xcount += getVisBufferDataCount(vi, params.getXAxis(),
                                                itsPolSel_);
                ycount += getVisBufferDataCount(vi, params.getYAxis(),
                                                itsPolSel_);
            }
        }
        
        Vector<double>& xvals = itsData_.getX(row, col);
        Vector<double>& yvals = itsData_.getY(row, col);
        xvals.resize(xcount);
        yvals.resize(ycount);
        unsigned int xindex = 0, yindex = 0;
        
        // Pass each timestamp (VisBuffer) to xcalculator and ycalculator.
        for(vi.originChunks(); vi.moreChunks(); vi.nextChunk()) {
            ////      Vector<Int> scans;
            ////      vi.scan(scans);
            ////      cout << " scan = " << scans(0)
            ////       << " spw = " << vi.spectralWindow() 
            ////       << " fld = " << vi.fieldId() 
            ////       << endl;

            for(vi.origin(); vi.more(); vi++) {
                getVisBufferData(vb, params.getXAxis(),params.getXDataColumn(),
                                 itsPolSel_, data);
                for(unsigned int i = 0; i < data.size(); i++)
                    xvals[xindex++] = data[i];
              
                getVisBufferData(vb, params.getYAxis(),params.getYDataColumn(),
                                 itsPolSel_, data);
                for(unsigned int i = 0; i < data.size(); i++)
                    yvals[yindex++] = data[i];
              
                  //joinVectors(xvals, (this->*xcalculator)(vi, d_v));
                  //joinVectors(yvals, (this->*ycalculator)(vi, d_v));
                  ////vi.setFlag(vb.flag());
            }
        }
        // Flush to disk
        ////vs_p->flush();

        //valid_vals = true;
        return true;
        
    } catch(AipsError& er) {
        itsLastError_ = er.getMesg();
        return false;
    } 
}

void PlotMS::getVisBufferData(const VisBuffer& vb, PMS::Axis axis,
        PMS::DataColumn dataColumn, const PlotMSPolSelection& pol_selection,
        Vector<double>& data) {
    switch(axis) {
    
    case PMS::TIME: data.reference(vb.time()); break;
    
    case PMS::UVDIST: {
        Vector<RigidVector<Double, 3> > uvwvec(vb.uvw());
        unsigned int nrows = vb.nRow();
        
        data.resize(nrows);
        double u, v;
        for(unsigned int row = 0; row < nrows; row++) {
          u = uvwvec(row)(0);
          v = uvwvec(row)(1);
          
          data(row) = sqrt(u * u + v * v);
        }
        
        break; }
    
    case PMS::CHANNEL: case PMS::CORR: {
        // Is corrType RR, I, XY, etc?  If so, it can't be cast to doubles, and
        // makes no sense for plotting.
        const Vector<Int>& v = (axis == PMS::CHANNEL) ?
                               vb.channel() : vb.corrType();
        data.resize(v.size());
        for(unsigned int i = 0; i < v.size(); i++) data[i] = v[i];
        break; }
        
    case PMS::FREQUENCY: data.reference(vb.frequency()); break;
        
    case PMS::VEL_RADIO: case PMS::VEL_OPTICAL: case PMS::VEL_RELATIVISTIC: {
        const Vector<Double>& f = vb.frequency();
        const Vector<Double>& lsr = vb.lsrFrequency();
        unsigned int n = f.size();
        if(lsr.size() < n) n = lsr.size();
        data.resize(n);
        
        for(unsigned int i = 0; i < n; i++) {
            if(axis == PMS::VEL_RADIO)
                data[i] = C::c * (1.0 - (f[i] / lsr[i]));
            else if(axis == PMS::VEL_OPTICAL)
                data[i] = C::c * ((lsr[i] / f[i]) - 1.0);
            else if(axis == PMS::VEL_RELATIVISTIC) {
                data[i] = f[i] / lsr[i];
                data[i] *= data[i];
                data[i] = C::c * (1.0 - data[i]) / (1.0 + data[i]);
            }
        }
        break; }
        
    case PMS::U: case PMS::V: case PMS::W: {
        Vector<RigidVector<Double, 3> > uvwvec(vb.uvw());
        unsigned int nrows = vb.nRow();        
        data.resize(nrows);
        
        unsigned int            index = 0;
        if(axis == PMS::V)      index = 1;
        else if(axis == PMS::W) index = 2;
        
        for(unsigned int row = 0; row < nrows; row++)
            data(row) = uvwvec(row)(index);
        break; }
        
    case PMS::AZIMUTH: // TO-DO data
        // To save you hunting for MVDirection.h: a getAngle() returns
        // a Quantum 2-vector, (uInt) returns the indicated element, and getValue
        // returns the direction cosine vector.
        //
        // So vb.azel(vb.time(d_v))(0) should work, and Quanta/MVPosition.h suggests
        // that it is a Vector<double>.
        //
        // FIX: vb.azel takes a single time but returns a vector.
        // FIX: This doesn't even return that vector.
        //data = vb.azel(vb.time()(0))(0);
        data.resize(0);
        break;
        
    case PMS::ELEVATION: // TO-DO data
        // FIX: vb.azel takes a single time but returns a vector.
        // FIX: This doesn't even return that vector.
        //data = vb.azel(vb.time()(0))(1);
        data.resize(0);
        break;
        
    case PMS::BASELINE: // TO-DO data
        // How does plotxy figure out the baseline number?  It's too hard to follow
        // after it disappears through the "xaxis" rabbit hole, but it'd be nice if
        // there's a standard definition for baseline number as a function of antenna1
        // and antenna2.  If the baseline number is a physical count of where the
        // baseline is in the array, and antenna1 and 2 do not match, this will
        // fail!
        //unsigned int nrows = vb.nRow();
        
        // Check 0 or 1 basedness for both baseline # and antenna #.
        //data = nrows * (vb.antenna1() - 1) + vb.antenna2();
        data.resize(0);
        break;
        
    case PMS::HOURANGLE: { // TO-DO data
        // HA = LST - RA

        // Get RA from average of direction1() and direction2(), just somehow from
        // the field direction, assuming that the antennas are pointing correctly?
        // Averaging the antennas for each visibility, as opposed to using a single
        // number for the field, may be one place where plotxy slows down.

        // I sure hope there's a function to get LST from the time and observatory
        // location.  I don't even know if vb.time() is sidereal or solar.  Maybe I
        // should work out the HA from the azimuth.

        // plotxy gets it from MSDerivedValues::hourAngle():
        // #include <ms/MeasurementSets/MSDerivedValues.h>
        //MSDerivedValues msd;
        //  msd.setAntennas(vb.antenna(d_v));

        // Is it faster to do this, or call vb.time() each time to refer to a single
        // copy?  Giving it d_v might be wrong since d_v gets reused.
        // Does the explicit reference do the trick?  (Or is it mandatory?)
        //Vector<double> timev(vb.time());
        //Vector<MDirection> dir(vb.direction1());  // Assumes dir1 == dir2.

        //uInt nrows = vb.nRow();

        //FIX: This appears to be bogus.  I can't find it anywhere.
        // MEpoch ep = MS::epochMeasure(timev(0));  // Should it go here or 2 lines down?
      //   for(Int row = 0; row < nrows; ++row){
      //     ep.set(MVEpoch(Quantity(timev(row), "s")));
      //     msd.setEpoch(ep);

      //     msd.setFieldCenter(dir(row));
      //     // msd.setVelocityFrame(MRadialVelocity::LSRK);

      //     // It gets converted from radians into degrees instead of hours, a la
      //     // plotxy.  Personally, I'd prefer hours.
      //     d_v[row] = msd.hourAngle() / C::degree;
      //  }
        data.resize(0);
        break; }
        
    case PMS::PARALLACTICANGLE: // TO-DO data
        // Note that this returns the parallactic angle of the first antenna of each
        // visibility.

        // The feed pa seems to depend on a MSDerivedValues anyway, so there may not
        // be much speed advantage here over plotxy.
        //return static_cast<Vector<double> >(vb.feed1_pa());
        // FIX floatiness.  return vb.feed1_pa();
        data.resize(0);
        break;
        
    case PMS::AMP: case PMS::PHASE: case PMS::REAL: case PMS::IMAG: {
        Matrix<CStokesVector> vis;
        switch(dataColumn) {
        case PMS::CORRECTED: vis.reference(vb.correctedVisibility()); break;
        case PMS::MODEL:     vis.reference(vb.modelVisibility());     break;
        case PMS::RESIDUAL: {
            const Matrix<CStokesVector>& c = vb.correctedVisibility();
            const Matrix<CStokesVector>& m = vb.modelVisibility();
            
            unsigned int nrow = c.nrow(), ncol = c.ncolumn();
            if(m.nrow() < nrow) nrow = m.nrow();
            if(m.ncolumn() < ncol) ncol = m.ncolumn();
            vis.resize(nrow, ncol);
            
            for(unsigned int row = 0; row < nrow; row++) {
                for(unsigned int col = 0; col < ncol; col++) {
                    vis(row, col) = c(row, col);
                    vis(row, col) -= m(row, col);
                }
            }
            break; }
        
        case PMS::DATA: default: vis.reference(vb.visibility()); break;
        }

        unsigned int nrows  = vb.nRow();
        unsigned int nchans = vb.nChannel();
        
        // A StokesVector always has 4 entries, but we don't necessarily want
        // to plot all of them.
        data.resize(nrows * nchans * pol_selection.npols());
        
        uLong index = 0;
        for(uInt row = 0; row < nrows; row++) {
            for(uInt chn = 0; chn < nchans; chn++) {
                for(uShort corr = 0; corr < pol_selection.npols(); corr++) {
                    if(axis == PMS::AMP)
                        // No rotation of polarization bases yet, just
                        // selection from the offered correlations.
                        data[index++]= abs(vis(chn, row)(pol_selection(corr)));
                    else if(axis == PMS::PHASE)
                        data(index++)= arg(vis(chn, row)(corr)) / C::degree;
                    else if(axis == PMS::REAL)
                        data[index++]=vis(chn,row)(pol_selection(corr)).real();
                    else if(axis == PMS::IMAG)
                        data[index++]=vis(chn,row)(pol_selection(corr)).imag();
                }
            }
        }

        break; }

//// antenna positions, like plotants.  Does not work for yaxis.
//// Not implemented yet, and I'm not sure it should be done here.
//// PlotCalcFunc
//const Vector<double>& PlotMS::pc_x(const VisBuffer& vb, Vector<double>& d_v)
//{
//  return x;
//}
        
    default: throw AipsError("Axis not recognized!  (Shouldn't happen.)");
    }
}

unsigned int PlotMS::getVisBufferDataCount(const VisBuffer& vb, PMS::Axis axis,
        const PlotMSPolSelection& pol_selection) {
    switch(axis) {    
    case PMS::TIME: return vb.time().size();
    
    case PMS::UVDIST: case PMS::U: case PMS::V: case PMS::W:
        return vb.uvw().size();
    
    case PMS::CHANNEL: return vb.channel().size();        
    case PMS::CORR: return vb.corrType().size();        
    case PMS::FREQUENCY: return vb.frequency().size();
        
    case PMS::VEL_RADIO: case PMS::VEL_OPTICAL: case PMS::VEL_RELATIVISTIC: {
        unsigned int f = vb.frequency().size(), lsr = vb.lsrFrequency().size();
        if(f <= lsr) return f;
        else         return lsr; }
        
    case PMS::AZIMUTH: // TO-DO data size
        return 0;//vb.azel(vb.time()(0))(0).size();
        
    case PMS::ELEVATION: // TO-DO data size
        return 0;//vb.azel(vb.time()(0))(1).size();
        
    case PMS::BASELINE: // TO-DO data size
        return 0;//vb.antenna1().size();
        
    case PMS::HOURANGLE: // TO-DO data size
        return 0;//vb.nRow();
        
    case PMS::PARALLACTICANGLE: // TO-DO data size
        return 0;//vb.feed1_pa().size();
        
    case PMS::AMP: case PMS::PHASE: case PMS::REAL: case PMS::IMAG:
        return vb.nRow() * vb.nChannel() * pol_selection.npols();
        
    default: throw AipsError("Axis not recognized!  (Shouldn't happen.)");
    }
}
*/

/*
LogIO& PlotMS::logSink() {return sink_p;};

String PlotMS::timerString(){
  ostringstream o;
  o << " [user:   " << timer_p.user () << 
       "  system: " << timer_p.system () <<
       "  real:   " << timer_p.real () << "]"; 
  timer_p.mark();
  return o;
};

// Bool PlotMS::initFlagset(const Int& flagset) 
// {

//   //  logSink() << LogOrigin("PlotMS","initFlagset") << LogIO::NORMAL3;

//   if(vs_p){
//     vs_p->initFlagset(flagset);
//     return True;
//   }
//   else {
//     throw(AipsError("PlotMS cannot initFlagset"));
//     return False;
//   }
// }

Bool PlotMS::applyPlotOptions()
{
  if(itsPlotOptions.ncells == 1){
    rownum = &PlotMS::panelnum_single;
    colnum = &PlotMS::panelnum_single;
  }
  else if(itsPlotOptions.fastaxis == PlotMSParameters::Horizontal){
    rownum = &PlotMS::rownum_horizontal;
    colnum = &PlotMS::colnum_horizontal;
  }
  else{
    rownum = &PlotMS::rownum_vertical;
    colnum = &PlotMS::colnum_vertical;
  }
  return true;
}

std::pair<String, String> PlotMS::parseInputs(const String& xaxis,
					      const String& yaxis)
{
  // errors[0] is for fairly harmless warnings.
  // Anything in errors[1] will cause the plot attempt to be aborted after
  // being reported as SEVERE.
  std::pair<String, String> errors("", "");
  
  if(!ms_p)
    errors.second += "The measurement set is not open.\n";

  try{
    xcalculator = string2calculator[xaxis];
  }
  catch(...){
    errors.second += "The requested x axis is unrecognized.\n";
  }
  try{
    ycalculator = string2calculator[yaxis];
  }
  catch(...){
    errors.second += "The requested y axis is unrecognized.\n";
  }
  
  if(itsPlotOptions.overplot && have_plotted){ // Do some consistency checks.
    if(xaxis != old_xaxis){
      try{
	if(axis_unit[xaxis] != axis_unit[old_xaxis]){
	  errors.second += "Overplot is on and the requested new x axis has\n";
	  errors.second += "different units from the old one.\n";
	}
	else
	  errors.first += "Were you aware that you are plotting " + xaxis +
	                 " as the new x axis on top of " + old_xaxis + "?\n";
      }
      catch(AxisUnitException& aue){
	errors.second += "Overplot is on and the requested new x axis has\n";
	errors.second += "an unknown AxisUnit.\n";
      }
    }
    if(yaxis != old_yaxis){
      try{
	if(axis_unit[yaxis] != axis_unit[old_yaxis]){
	  errors.second += "Overplot is on and the requested new y axis has\n";
	  errors.second += "different units from the old one.\n";
	}
	else
	  errors.first += "Plotting " + yaxis + " as the new y axis on top of "
	                + old_yaxis + ".\n";
      }
      catch(AxisUnitException& aue){
	errors.second += "Overplot is on and the requested new y axis has\n";
	errors.second += "an unknown AxisUnit.\n";
      }
    }
  }

  // Setup old_plot_params
  old_xaxis = xaxis;
  old_yaxis = yaxis;
  have_plotted = true;

  return errors;
}

//#////////////////////////////////////////////////////////////////////////////
//# Plot the specified portion of the measurement set
//#
//# Valid x and y values are:
//#         ARRAY, AZIMUTH, BASELINE, CHAN, CORR, DATA, ELEVATION, HOURANGLE,
//#         PARALLACTICANGLE, TIME, U, UVCOVERAGE, UVDIST, V, and W.
//# 
//# Valid column values are:
//#         CORRECTED, MODEL, RESIDUAL, WEIGHTEDDATA, WEIGHTEDCORRECTED,
//#         WEIGHTEDMODEL, WEIGHTEDRESIDUAL (some of these not fully
//#         implemented yet).  Also doesn't error if it is something else
//#         it falls through hoping that it is a recognizable data column.
//#
//# Valid iteration values -- see checkInterationAxes
//#         BASELINE, ANTENNA, ANTENNA1, ANTENNA2, ARRAY, ARRAY_ID,
//#         CHAN, CHANNEL, CORR, CORRELATION, DATA_DESC_ID, FEED, 
//#      FEED1, FIELD, FIELD_ID, SCAN, SCAN_NUMBER, SPW, SPECTRAL_WINDOW
//#         also anything that is a column in the Main MS table will work.
Bool PlotMS::plot(const String& x, const String& y)
{
  Bool rstat = true;  // Let's be optimistic.
  
  String FnCall = "(x, y, xcolumn, ycolumn, iteration, filename)";
  String fnname = "plot";
  //log->FnEnter(fnname + FnCall, clname);

  logSink() << LogMessage::NORMAL2 << "Starting PlotMS::plot("
            << x << ", " << y << ")." << LogIO::POST;

  // Note that we're not just generating errors.  Inputs are being parsed!
  std::pair<String, String> errors(parseInputs(x, y));
  if(errors.first.length() > 0)
    logSink() << LogMessage::NORMAL2 << "PlotMS::plot:" << errors.first
              << LogIO::POST;
  if(errors.second.length() > 0){    
    logSink() << LogMessage::SEVERE << "PlotMS::plot:" << errors.second
              << LogIO::POST;
    return false; 
  }
           
  String msg = "Preparing data ...      ";
  // MsPlot.h gets log from SLog.  I don't know exactly what a SLog is.
  //log->out(msg, fnname, clname, LogMessage::NORMAL);
    
  try{
    Vector<String> selectionStr;
    String         plottitlebase;
    String         plottitle;    

    if(itsPlotMSPlotter == NULL)
      itsPlotMSPlotter = new PlotMSPlotter(itsPlotOptions.nrows,
					   itsPlotOptions.ncols);

    if(niterations > 1){
      // plottitlebase += iteration_type; // TO-DO: baseline, antenna, etc.
      //itsPlotMSPlotter->changeGuiButtonState("iternext", "enabled");
    }
    else
      plottitle = plottitlebase;

    for (uInt iteration = 0; iteration < niterations; ++iteration){
//TO-DO       if(iteration == 1)
// 	itsPlotMSPlotter->changeGuiButtonState("iterprev", "enabled");
//       if(iteration == niterations - 1)
// 	itsPlotMSPlotter->changeGuiButtonState("iternext", "disabled");
      
      //# Make subselection for iteration.

      calcpoints();
    
      logSink() << LogMessage::NORMAL2 << "Plotting " << xvals.size()
		<< " points." << LogIO::POST;
           
      //FIX if(niterations > 1)
      //  plottitle = plottitlebase << iteration + 1;
      
      //# Note that overplots are allowed with iteration even though
      //# they seem to be at cross-purposes.
      if(xvals.nelements() == yvals.nelements()){
	itsPlotMSPlotter->plotxy(xvals, yvals,
				 itsPlotOptions.color(iteration),
				 plottitle, itsPlotOptions.overplot,
				 (this->*rownum)(iteration),
				 (this->*colnum)(iteration));
      }
      else{
	if((yvals.nelements() % xvals.nelements()) == 0){
	  uInt nxs = xvals.nelements();
	  uInt nys = yvals.nelements() / nxs;
	  Vector<double> everynyth;
	  
	  // yvals contains multiple correlations, so split it up.
	  for(uInt i = 0; i < nys; ++i){
	    everynyth.reference(yvals(Slice(i, nxs, nys)));
	    
	    itsPlotMSPlotter->plotxy(xvals, everynyth,
				     itsPlotOptions.color(iteration),
				     plottitle, itsPlotOptions.overplot,
				     (this->*rownum)(iteration),
				     (this->*colnum)(iteration));
	  }
	}
	else if((xvals.nelements() % yvals.nelements()) == 0){
	  uInt nys = yvals.nelements();
	  uInt nxs = xvals.nelements() / nys;
	  Vector<double> everynxth;

	  // xvals contains multiple correlations, so split it up.
	  for(uInt i = 0; i < nys; ++i){
	    everynxth.reference(xvals(Slice(i, nys, nxs)));
	    itsPlotMSPlotter->plotxy(everynxth, yvals,
				     itsPlotOptions.color(iteration),
				     plottitle, itsPlotOptions.overplot,
				     (this->*rownum)(iteration),
				     (this->*colnum)(iteration));
	  }
	}
	else{
	  throw(AipsError(String("The number of xs (") + xvals.nelements()
			  + String(") does not match the number of ys (")
			  + yvals.nelements() + String(").")));
	}
      }
      
      // //# MAJOR HACK ALERT -- in order to get the multiple spw
      // //# plot to work we need to set up the Convert Fn differently
      // //# for each table, we need to make sure the correct spwId is
      // //# in the the first index of the TableList.
      // if(itsTableVectors.nelements() > 1){
      // 	if(!upcase(x).compare("CHANFREQ") 
      // 	   || !upcase(x).compare("CHANNEL_FREQ")
      // 	   || !upcase(x).compare("CHANNEL_FREQUENCY")
      // 	   || !upcase(x).compare("CHAN_FREQ") 
      // 	   || !upcase(x).compare("CHANFREQ") 
      // 	   || !upcase(x).compare("CHANNEL_FREQ")
      // 	   || !upcase(x).compare("CHANNEL_FREQUENCY")
      // 	   || !upcase(x).compare("CHAN_FREQ")){
      // 	  if(itsAveMode.length() < 1 || 
      // 	     (itsAveChan <= 1 && itsAveTime <= itsMinTimeInterval)){
      // 	    setupConvertFn(x, 'x');
      // 	    setupConvertFn(y, 'y');
      //
      // 	    Vector<int> spws(itsTableVectors.nelements());
      // 	    for(uInt z = 0; z < itsTableVectors.nelements(); z++)
      // 	      spws[z] = spwId;
      // 	    if(itsPlotOptions.Convert != NULL){
      // 	      PlotMSConvertChanToFreq *conv = 
      // 		(PlotMSConvertChanToFreq*)itsPlotOptions.Convert;
      // 	      conv->setSpwIds(spws);
      // 	    }
      // 	    else{
      // 	      ostringstream os;
      // 	      os << "Internal Error: Unexected NULL for conversion function pointer" 
      // 		 << endl;
      // 	      log->out(os, fnname, clname, LogMessage::SEVERE);
      // 	    }
      // 	  }
      // 	}
	
      // 	if(!upcase(x).compare("CHANVELOCITY") 
      // 	   || !upcase(x).compare("CHANNEL_VELOCITY")
      // 	   || !upcase(x).compare("CHAN_VELOCITY") 
      // 	   || !upcase(x).compare("CHANVELOCITY")){
      // 	  if(itsAveMode.length() < 1 || 
      // 	     (itsAveChan <= 1 && itsAveTime <= itsMinTimeInterval)){
      // 	    setupConvertFn(x, 'x');
      // 	    setupConvertFn(y, 'y');

      // 	    Vector<int> spws(itsTableVectors.nelements());
      // 	    for(uInt z = 0; z < itsTableVectors.nelements(); z++)
      // 	      spws[z] = spwId;
      // 	    if(itsPlotOptions.Convert != NULL){
      // 	      PlotMSConvertChanToVelocity *conv = 
      // 		(PlotMSConvertChanToVelocity*)itsPlotOptions.Convert;
      // 	      conv->setSpwIds(spws);
      // 	    }
      // 	    else{
      // 	      ostringstream os;
      // 	      os << "Internal Error: Unexected NULL for conversion function pointer" 
      // 		 << endl;
      // 	      log->out(os, fnname, clname, LogMessage::SEVERE);
      // 	    }
      // 	  }
      // 	}
      // }
    }
  }
  catch(AipsError& ae){
//     if(iteration.nelements() == 0)
//       cleanupOptions();
    rstat = False;
    throw;
  }

  if(rstat) rstat = itsPlotMSPlotter->execLoop() == 0;
  
  if(!rstat){
    //# TO-DO?? reset various flags because there is a serious
    //# error and we need to redo things??
  }
    
//   if(iteration.nelements() == 0)   
//     cleanupOptions();

  //log->FnExit(fnname, clname);
  return rstat;
}

// Select data
void PlotMS::setdata(const String& mode, 
		     const Int& nchan, const Int& start, const Int& step,
		     const MRadialVelocity& mStart,
		     const MRadialVelocity& mStep,
		     const String& msSelect)
{
// Define primary measurement set selection criteria
// Inputs:
//    mode         const String&            Frequency/velocity selection mode
//                                          ("channel", "velocity" or 
//                                           "opticalvelocity")
//    nchan        const Int&               No of channels to select
//    start        const Int&               Start channel to select
//    step         const Int&               Channel increment
//    mStart       const MRadialVelocity&   Start radial vel. to select
//    mStep        const MRadialVelocity&   Radial velocity increment
//    msSelect     const String&            MS selection string (TAQL)
// Output to private data:
//
  logSink() << LogOrigin("PlotMS", "setdata") << LogIO::NORMAL3;

  try{    
    logSink() << "Selecting data" << LogIO::POST;
    
    // Apply selection to the original MeasurementSet
    logSink() << "Performing selection on MeasurementSet" << LogIO::POST;

    // Delete VisSet and selected MS
    if(vs_p){
      delete vs_p;
      vs_p = 0;
    };
//     if(mssel_p){
//       delete mssel_p;
//       mssel_p = 0;
//     };

    // Force a re-sort of the MS
    if(ms_p->keywordSet().isDefined("SORTED_TABLE"))
      ms_p->rwKeywordSet().removeField("SORTED_TABLE");
    if(ms_p->keywordSet().isDefined("SORT_COLUMNS"))
      ms_p->rwKeywordSet().removeField("SORT_COLUMNS");

    // Re-make the sorted table as necessary
    if(!ms_p->keywordSet().isDefined("SORTED_TABLE")){
      Block<int> sort(0);
      Matrix<Int> noselection;
      VisSet vs(*ms_p, sort, noselection);
    }
    Table sorted = ms_p->keywordSet().asTable("SORTED_TABLE");
      
    Int len    = msSelect.length();
    Int nspace = msSelect.freq (' ');
    Bool nullSelect = (msSelect.empty() || nspace==len);
    if(!nullSelect){
      // Apply the TAQL selection string, to remake the selected MS
      String parseString = "select from $1 where " + msSelect;
      mssel_p = new MeasurementSet(tableCommand(parseString,sorted));
      AlwaysAssert(mssel_p, AipsError);

      // Rename the selected MS as SELECTED_TABLE
      mssel_p->rename(msname_p + "/SELECTED_TABLE", Table::Scratch);
      nullSelect = (mssel_p->nrow() == 0);

      // Null selection wasn't intended!
      if(nullSelect)
	throw(AipsError("Specified msselect failed to select any data."));
    };

    if(nullSelect){
      // Selection of whole MS intended
      if(mssel_p){
	delete mssel_p; 
	mssel_p = 0;
      };
      logSink() << LogIO::NORMAL
		<< "Selection is empty: reverting to sorted MeasurementSet"
		<< LogIO::POST;
      mssel_p = new MeasurementSet(sorted);
    }
    else{
      mssel_p->flush();
    }

    if(mssel_p->nrow() != ms_p->nrow()){
      logSink() << "By selection " << ms_p->nrow() << 
	" rows are reduced to " << mssel_p->nrow() << LogIO::POST;
    }
    else{
      logSink() << "Selection did not drop any rows" << LogIO::POST;
    }
    
    valid_vals = false;

    // Now, re-create the associated VisSet
    if(vs_p) delete vs_p; vs_p = 0;
    Block<int> sort(0);
    Matrix<Int> noselection;
    vs_p = new VisSet(*mssel_p,sort,noselection);
    AlwaysAssert(vs_p, AipsError);

    // Now do channel selection
    selectChannel(mode,nchan,start,step,mStart,mStep);
  }
  catch(AipsError& x){
    // Re-open with the existing MS
    logSink() << LogOrigin("PlotMS","setdata",WHERE) 
	      << LogIO::SEVERE << "Caught exception: " << x.getMesg()
	      << LogIO::POST;
    open(*ms_p,False);
    throw(AipsError("Error in PlotMS::setdata()"));
  }
}
*/

// Select data (using MSSelection syntax)
/*
void PlotMS::selectvis(const String&		time,
		       const String&		spw,
		       const String&		scan,
		       const String&		field,
		       const String&		baseline,
		       const String&		uvrange,
		       const String&		chanmode,
		       const Int&		nchan,
		       const Int&		start, 
		       const Int&		step,
		       const MRadialVelocity&	mStart,
		       const MRadialVelocity&	mStep,
	        const String&		msSelect)
	        */
/*
void PlotMS::selectvis(const PlotMSSelection& selection)
{
// Define primary measurement set selection criteria
// Inputs:
//    time
//    spw
//    scan
//    field
//    baseline
//    uvrange
//    chanmode     const String&            Frequency/velocity selection mode
//                                          ("channel", "velocity" or 
//                                           "opticalvelocity")
//    nchan        const Int&               No of channels to select
//    start        const Int&               Start channel to select
//    step         const Int&               Channel increment
//    mStart       const MRadialVelocity&   Start radial vel. to select
//    mStep        const MRadialVelocity&   Radial velocity increment
//    msSelect     const String&            MS selection string (TAQL)
// Output to private data:
//
  logSink() << LogOrigin("PlotMS", "setdata") << LogIO::NORMAL3;
  
  try{ 
    //cout << "time     = " << time << " " << time.length() <<endl;
    //cout << "spw      = " << spw << " " << spw.length() <<endl;
    //cout << "scan     = " << scan << " " << scan.length() <<endl;
    //cout << "field    = " << field << " " << field.length() <<endl;
    //cout << "baseline = " << baseline << " " << baseline.length() << endl;
    //cout << "uvrange  = " << uvrange << " " << uvrange.length() << endl;

    logSink() << "Selecting data" << LogIO::POST;
    
    // Apply selection to the original MeasurementSet
    logSink() << "Performing selection on MeasurementSet" << LogIO::POST;
    
    // Delete VisSet and selected MS
    if(vs_p){
      delete vs_p;
      vs_p = 0;
    };
    if(mssel_p){
      delete mssel_p;
      mssel_p = 0;
    };
    
    // Force a re-sort of the MS
    if(ms_p->keywordSet().isDefined("SORTED_TABLE"))
      ms_p->rwKeywordSet().removeField("SORTED_TABLE");
    if(ms_p->keywordSet().isDefined("SORT_COLUMNS"))
      ms_p->rwKeywordSet().removeField("SORT_COLUMNS");
    
    // Re-make the sorted table as necessary
    if(!ms_p->keywordSet().isDefined("SORTED_TABLE")){
      Block<int> sort(0);
      Matrix<Int> noselection;
      VisSet vs(*ms_p, sort, noselection);
    }
    Table sorted = ms_p->keywordSet().asTable("SORTED_TABLE");
    
    Bool nontrivsel = False;

    // Assume no selection, for starters
    mssel_p = new MeasurementSet(sorted);

    cout << "Applying selection." << endl;

    // Apply user-supplied selection
    nontrivsel = mssSetData(MeasurementSet(sorted),
			    *mssel_p, "",
			    selection.time(), selection.baseline(),
			    selection.field(), selection.spw(),
			    selection.uvrange(), selection.msSelect(),
			    "", selection.scan());

    // If non-trivial MSSelection invoked and nrow reduced:
    if(nontrivsel && mssel_p->nrow() < ms_p->nrow()){
      // Escape if no rows selected
      if(mssel_p->nrow() == 0) 
	throw(AipsError("Specified selection selects zero rows!"));

      // ...otherwise report how many rows are selected
      logSink() << "By selection " << ms_p->nrow() 
		<< " rows are reduced to " << mssel_p->nrow() 
		<< LogIO::POST;
    }
    else{
      // Selection did nothing:
      logSink() << "Selection did not drop any rows" << LogIO::POST;
    }

    valid_vals = false;

    // Now, re-create the associated VisSet
    if(vs_p) delete vs_p; vs_p=0;
    Block<int> sort(0);
    Matrix<Int> noselection;
    vs_p = new VisSet(*mssel_p,sort,noselection);
    AlwaysAssert(vs_p, AipsError);

    // Attempt to use MSSelection for channel selection
    //  if user not using the old way
    if(selection.chanmode()=="none"){
      selectChannel(selection.spw());
    }
    else {
      // Reluctantly use the old-fashioned way
      logSink() << LogIO::WARN 
		<< "You have used the old-fashioned mode parameter" << endl
		<< "for channel selection.  It still works, for now," << endl
		<< "but this will be eliminated in the near future." << endl
		<< "Please begin using the new channel selection" << endl
		<< "syntax in the spw parameter." << LogIO::POST;
      selectChannel(selection.chanmode(),selection.nchan(),selection.start(),
                    selection.step(),selection.mStart(),selection.mStep());
    }
  }
  catch(MSSelectionError& x){
    // Re-open with the existing MS
    logSink() << LogOrigin("PlotMS","selectvis",WHERE) 
	      << LogIO::SEVERE << "Caught exception: " << x.getMesg()
	      << LogIO::POST;
    open(*ms_p,False);
    throw(AipsError("Error in data selection specification."));
  } 
  catch(AipsError& x){
    // Re-open with the existing MS
    logSink() << LogOrigin("PlotMS","selectvis",WHERE) 
	      << LogIO::SEVERE << "Caught exception: " << x.getMesg()
	      << LogIO::POST;
    open(*ms_p,False);
    throw(AipsError("Error in PlotMS::selectvis()"));
  } 
}

// Returns the number of points that would be plotted with the current setup.
// Ideally this should be able to return a reasonable estimate _before_ any
// expensive operations, to allow the user to tweak the selection and binning
// ahead of time.
uInt PlotMS::npoints()
{
  if(!ok()){
    return 0;
  }
  else if(valid_vals){
    return xvals.nelements();
  }
  else{
    uInt nbins = mssel_p->nrow() / binwidth;  // Estimate, since there may not
					      // be a constant # of
					      // visibilities per bin.
    uInt nplottedperbin = 1;
    
    // if( MINMAXMED || MINMAXMEAN )
    //    nplottedperbin = 3;
    return nbins * nplottedperbin;
  }
}

void PlotMS::apply_flagset()
{
  logSink() << LogOrigin("PlotMS", "Applying flagset") << LogIO::NORMAL;

  // TO-DO iterate through flagset and apply the flags.
  // Ideally the start and endpoints, and maybe even stepsize, for vi should be
  // calculated from fs. 
  // for(vi.origin(); vi.more(); vi++)
  //   for(FlagSet fs = flagset.start(); fs < flagset.end(); ++fs)
  //     if(vb in fs)
  // 	vi.setFlag(vb.flag());

  // Mark flagset as applied.
  flagset_p.resize(0);
}

void PlotMS::close(const Bool apply=false)
{
  // Switching ms'es means any existing flagset can't hang around.
  cleanup_flagset(apply);

  // Delete derived dataset stuff
  delete vs_p; 		vs_p	= 0;
  delete mssel_p; 	mssel_p = 0;
  delete ms_p; 		ms_p	= 0;
}

void PlotMS::cleanup_flagset(const Bool apply)
{
  if(flagset_p.nelements() > 0){
    if(apply){
      apply_flagset();
    }
    else{
      logSink() << LogOrigin("PlotMS",
			     "Deleting the flagset without applying it")
	        << LogIO::NORMAL;
      flagset_p.resize(0);   // CHECK  Is this enough?
    }
  }
  else
    logSink() << LogOrigin("PlotMS", "The flagset was empty") << LogIO::NORMAL;
}

// Remove the last flagging command from flagset.
Bool PlotMS::undo_flag()
{
  logSink() << LogOrigin("PlotMS", "undo_flag") << LogIO::NORMAL;
  
  try{
    Int last_cmd_idx = flagset_p.nelements() - 1;
    // if(last_cmd_idx >= 0){
//FIX       flagset_p.remove(last_cmd_idx);

//       // Update display.
//    }
    return true;
  }
  catch(AipsError& x){
    logSink() << LogIO::SEVERE << "Caught exception: " << x.getMesg() 
 	      << LogIO::POST;
    throw(AipsError("Error in PlotMS::undo_flag."));
    return false;
  }
  return false;
}

// Iterates through the ms and sets xvals and yvals using xcalculator and
// ycalculator.
Bool PlotMS::calcpoints()
{  
  logSink() << LogOrigin("PlotMS","calcpoints") << LogIO::NORMAL;
  
  try{
    if(!ok())
      throw(AipsError("PlotMS not prepared for calcpoints!"));

    // Arrange for iteration over data
    Block<Int> columns;
    // include scan iteration
    columns.resize(5);
    columns[0] = MS::ARRAY_ID;
    columns[1] = MS::SCAN_NUMBER;
    columns[2] = MS::FIELD_ID;
    columns[3] = MS::DATA_DESC_ID;
    columns[4] = MS::TIME;
    vs_p->resetVisIter(columns, 0.0);
    VisIter& vi(vs_p->iter());
    VisBuffer vb(vi);

    // A "blank" vector that gets passed by reference to, and returned by, the
    // vi functions called by (x, y)calculator.  I assume they do that to avoid
    // instatiating and resizing a Vector all the time.  They will resize d_v
    // as needed.
    Vector<Double> d_v;
    
    //// Ensure correlations restored to original order?
    //// (this is a no-op if no sort necessary)
    // vb.unSortCorr();

    // Pass each timestamp (VisBuffer) to xcalculator and ycalculator.
    for(vi.originChunks(); vi.moreChunks(); vi.nextChunk()){
      //      Vector<Int> scans;
      //      vi.scan(scans);
      //      cout << " scan = " << scans(0)
      //	   << " spw = " << vi.spectralWindow() 
      //	   << " fld = " << vi.fieldId() 
      //	   << endl;

      for(vi.origin(); vi.more(); vi++){
	joinVectors(xvals, (this->*xcalculator)(vi, d_v));
	joinVectors(yvals, (this->*ycalculator)(vi, d_v));
	//vi.setFlag(vb.flag());
      }
    }
    // Flush to disk
    //vs_p->flush();

    valid_vals = true;

    return True;
  }
  catch(AipsError& x){
    logSink() << LogIO::SEVERE << "Caught exception: " << x.getMesg() 
	      << LogIO::POST;
    throw(AipsError("Error in PlotMS::calcpoints."));
    return False;
  } 
  return False;
}

// Returns a guess at the polarization basis of vb.
// It is a guess because it only looks at the first element of
// vb.fillCorrType(), but CStokesVector assumes a certain order for the
// correlations anyway.  However, George says the transformations should be
// done inside VisBuffer, and CStokesVector should be avoided.
//
// Also, the number of correlations can change within an MS, i.e. one buffer might have
// RR, RL, LR, & LL,
// and the next
// RR & LL,
// and the next just LL.  (At this rate the next buffer will be completely flagged...)
//
PlotMS::PolBasisEnum PlotMS::pol_basis_guess(const VisBuffer& vb)
{
  switch(vb.corrType()[0]){
  case Stokes::I:
    return pbStokes;
  case Stokes::RR:
    return pbCircular;
  case Stokes::XX:
    return pbLinear;
  default:
    return pbUnrecognized;  // Default to no rotation of the polarization vector.
  }
}

void PlotMS::selectChannel(const String& spw)
{

  cout << "PlotMS::selectChannel" << endl;

  Matrix<Int> chansel = getChanIdx(spw);
  uInt nselspw = chansel.nrow();

  if(nselspw == 0){
    logSink() << "Frequency selection: Selecting all channels in all spws." 
	      << LogIO::POST;
  }
  else{
    logSink() << "Frequency selection: " << LogIO::POST;

    // Trap non-unit step (for now)
    // Calibrater.cc uses if(nselspw != ntrue(chansel.column(3) == 1)){
    // but I couldn't get PlotMS.cc to not use a version of ntrue() that
    // doesn't return a TableExpressionNode.  So I count the channelsteps here:
    uInt neq1 = 0;
    for(uInt i = 0; i < nselspw; ++i){
      if(chansel(i, 3) == 1)
	++neq1;
    }
    if(neq1 != nselspw){
      logSink() << LogIO::WARN
		<< "plotms does not support non-unit channel stepping; "
		<< "Using step = 1."
		<< LogIO::POST;
      chansel.column(3) = 1;
    }

    Vector<Bool> spwdone(vs_p->numberSpw(),False);
    logSink() << LogIO::NORMAL;
    for(uInt i = 0; i < nselspw; ++i){
      Int& spw = chansel(i, 0);

      if(!spwdone(spw)){
	spwdone(spw) = True;
	
	Int  nchan = chansel(i, 2) - chansel(i, 1) + 1;
	Int& start = chansel(i, 1);
	Int& end   = chansel(i, 2);
	Int& step  = chansel(i, 3);
	
	logSink() << ".  Spw " << spw << ":"
		  << start << "~" << end 
		  << " (" << nchan << " channels,"
		  << " step by " << step << ")"
		  << endl;
	
	// Call via VisSet (avoid call to VisIter::origin)
	vs_p->selectChannel(1, start, nchan, step, spw, False);
      }
      else{
	logSink() << LogIO::POST;
	throw(AipsError("Data selection for calibration supports only one channel selection per spw."));
      }
    } // i
  } // non-triv spw selection
  logSink() << LogIO::POST;

  // For testing:
  if(False){
    VisIter& vi(vs_p->iter());
    VisBuffer vb(vi);
    
    // Pass each timestamp (VisBuffer) to VisEquation for calcpointsion
    for(vi.originChunks(); vi.moreChunks(); vi.nextChunk()){
      vi.origin();
      //      for(vi.origin(); vi.more(); vi++)
	cout << vb.spectralWindow() << " "
	     << vb.nChannel() << " "
	     << vb.channel() << " "
	     << vb.visCube().shape()
	     << endl;
    }
  }
}

// Select on channel in the VisSet
void PlotMS::selectChannel(const String& mode, 
			       const Int& nchan, 
			       const Int& start, const Int& step,
			       const MRadialVelocity& mStart,
			       const MRadialVelocity& mStep)
{  
  // Set data selection variables
  dataMode_p  = mode;
  dataNchan_p = nchan > 0 ? nchan : 0;
  dataStart_p = start > 0 ? start : 0;
  dataStep_p  = step > 1 ? step : 1;
  
  mDataStart_p = mStart;
  mDataStep_p  = mStep;
  
  // Select on frequency channel
  if(dataMode_p == "channel"){
    // *** this bit here is temporary till we unifomize data selection
    //Getting the selected SPWs
    ROMSMainColumns msc(*mssel_p);
    Vector<Int> dataDescID = msc.dataDescId().getColumn();
    Bool dum;
    Sort sort(dataDescID.getStorage(dum), sizeof(Int));
    sort.sortKey((uInt)0, TpInt);
    Vector<uInt> index, uniq;
    sort.sort(index,dataDescID.nelements());
    uInt nSpw = sort.unique(uniq,index);
    
    Vector<Int> selectedSpw(nSpw);
    Vector<Int> nChan(nSpw);
    for(uInt k=0; k < nSpw; ++k){
      selectedSpw[k] = dataDescID[index[uniq[k]]];
      nChan[k] = vs_p->numberChan()(selectedSpw[k]);      
    }
    if(dataNchan_p == 0)
      dataNchan_p = vs_p->numberChan()(selectedSpw[0]);
    if(dataStart_p < 0)
      logSink() << LogIO::SEVERE << "Illegal start pixel = " 
		<< dataStart_p << LogIO::POST;

    Int end = Int(dataStart_p) + Int(dataNchan_p) * Int(dataStep_p);
    for(uInt k = 0; k < selectedSpw.nelements(); ++k){
      if(end < 1 || end > nChan[k])
	logSink() << LogIO::SEVERE << "Illegal step pixel = " << dataStep_p
		  << " in Spw " << selectedSpw[k]
		  << LogIO::POST;
      logSink() << "Selecting "<< dataNchan_p
		<< " channels, starting at visibility channel "
		<< dataStart_p  << " stepped by "
		<< dataStep_p << " in Spw " << selectedSpw[k] << LogIO::POST;
      
      // Set frequency channel selection for all spectral window id's
      Int nch;
      //Vector<Int> nChan=vs_p->numberChan();
      //Int nSpw=vs_p->numberSpw();
      nch = dataNchan_p == 0 ? nChan(k) : dataNchan_p;

      vs_p->selectChannel(1,dataStart_p,nch,dataStep_p,selectedSpw[k]);      
    }
  }
  else if(dataMode_p == "velocity"){  			// Select on velocity
    MVRadialVelocity mvStart(mDataStart_p.get("m/s"));
    MVRadialVelocity mvStep(mDataStep_p.get("m/s"));
    MRadialVelocity::Types
      vType((MRadialVelocity::Types)mDataStart_p.getRefPtr()->getType());
    logSink() << "Selecting "<< dataNchan_p
	      << " channels, starting at radio velocity " << mvStart
	      << " stepped by " << mvStep << ", reference frame is "
	      << MRadialVelocity::showType(vType) << LogIO::POST;
    vs_p->iter().selectVelocity(Int(dataNchan_p), mvStart, mvStep,
				vType, MDoppler::RADIO);
  }
  else if(dataMode_p == "opticalvelocity"){		// Select on optical velocity
    MVRadialVelocity mvStart(mDataStart_p.get("m/s"));
    MVRadialVelocity mvStep(mDataStep_p.get("m/s"));
    MRadialVelocity::Types
      vType((MRadialVelocity::Types)mDataStart_p.getRefPtr()->getType());
    logSink() << "Selecting "<< dataNchan_p
	      << " channels, starting at optical velocity " << mvStart
	      << " stepped by " << mvStep << ", reference frame is "
	      << MRadialVelocity::showType(vType) << LogIO::POST;
    vs_p->iter().selectVelocity(Int(dataNchan_p), mvStart, mvStep,
				vType, MDoppler::OPTICAL);
  }  
}

Vector<Int> PlotMS::getAntIdx(const String& antenna)
{
  MSSelection msselect;
  
  msselect.setAntennaExpr(antenna);
  return msselect.getAntenna1List(mssel_p);
}

// Interpret field indices (MSSelection)
Vector<Int> PlotMS::getFieldIdx(const String& fields)
{
  MSSelection mssel;

  mssel.setFieldExpr(fields);
  return mssel.getFieldList(mssel_p);
}

// Interpret spw indices (MSSelection)
Vector<Int> PlotMS::getSpwIdx(const String& spws)
{
  MSSelection mssel;

  mssel.setSpwExpr(spws);
  return mssel.getSpwList(mssel_p);
}

Matrix<Int> PlotMS::getChanIdx(const String& spw)
{
  MSSelection mssel;

  mssel.setSpwExpr(spw);
  return mssel.getChanList(mssel_p);
}

Bool PlotMS::ok()
{
  if(vs_p && ms_p && mssel_p && xcalculator && ycalculator){
    return True;
  }
  else{
    logSink() << "PlotMS is not yet opened" << LogIO::POST;
    return False;
  }
}

// void PlotMS::writeHistory(LogIO& os, Bool cliCommand)
// {
//   if(!historytab_p.isNull()){
//     if(histLockCounter_p == 0)
//       historytab_p.lock(False);
//     ++histLockCounter_p;

//     os.postLocally();
//     if(cliCommand)
//       hist_p->cliCommand(os);
//     else
//       hist_p->addMessage(os);

//     if(histLockCounter_p == 1)
//       historytab_p.unlock();
    
//     if(histLockCounter_p > 0)
//       --histLockCounter_p;
//   }
//   else
//     os << LogIO::SEVERE << "PlotMS is not yet opened" << LogIO::POST;
// }

unsigned int PlotMS::panelnum_single(const uInt iteration) const
{
  return 0;
}

unsigned int PlotMS::rownum_horizontal(const uInt iteration) const
{
  // ncells = itsPlotOptions.nrows * itsPlotOptions.ncols;
  return (iteration % itsPlotOptions.ncells) / itsPlotOptions.ncols;
}
  
unsigned int PlotMS::rownum_vertical(const uInt iteration) const
{
  return iteration % itsPlotOptions.nrows;
}
  
unsigned int PlotMS::colnum_horizontal(const uInt iteration) const
{
  return iteration % itsPlotOptions.ncols;
}
  
unsigned int PlotMS::colnum_vertical(const uInt iteration) const
{
  // ncells = itsPlotOptions.nrows * itsPlotOptions.ncols;
  return (iteration % itsPlotOptions.ncells) / itsPlotOptions.nrows;
}

// Returns whether or not any change was made.
// Side-effect: destroys any existing itsPlotMSPlotter (setting it to NULL)
//              if a change is made.
Bool PlotMS::change_nrows_ncols(const uInt newnrows, const uInt newncols)
{
  Bool madechange = itsPlotOptions.change_nrows_ncols(newnrows, newncols);
  
  if(madechange){
      if(itsPlotMSPlotter != NULL) delete itsPlotMSPlotter;
    itsPlotMSPlotter = NULL;
  }
  return madechange;
}
*/

/*
////////////////////////////////////
// PLOTMSPOLSELECTION DEFINITIONS //
////////////////////////////////////

PlotMSPolSelection::PlotMSPolSelection(const String& polselstr) {
    setPolSelection(polselstr);
}

PlotMSPolSelection::~PlotMSPolSelection() { }

void PlotMSPolSelection::setPolSelection(const String& str) {
    // CStokesVectors only have 4 correlations, but polselstr can include spaces
    // and commas (highly recommended when not using I, Q, U, or V).
    
    itsPolSel_.resize(0);
    for(unsigned int i = 0; i < str.size() && itsPolSel_.size() <= 4; i++) {
        switch(toupper(str[i])) {
        case 'I': itsPolSel_.push_back(0); break;
        case 'Q': itsPolSel_.push_back(1); break; // This assumes a certain
                                                  // order, but so does
                                                  // CStokesVector when it
                                                  // transforms bases.
        case 'V': itsPolSel_.push_back(2); break;
        case 'R': itsPolSel_.push_back((i < str.size() - 1 && toupper(str[++i])
                                        == 'R') ? 0 : 1); break;
        case 'L': itsPolSel_.push_back((i < str.size() - 1 && toupper(str[++i])
                                        == 'R') ? 2 : 3); break;
        case 'X': itsPolSel_.push_back((i < str.size() - 1 && toupper(str[++i])
                                        == 'X') ? 0 : 1); break;
        case 'Y': itsPolSel_.push_back((i < str.size() - 1 && toupper(str[++i])
                                        == 'X') ? 2 : 3); break;
                                        
        default: break;
        }
    }
}

const String& PlotMSPolSelection::polselstr() const { return itsPolSelStr_; } 
unsigned int PlotMSPolSelection::npols() const { return itsPolSel_.size(); }
int PlotMSPolSelection::pol_selection(unsigned int index) const {
    if(index >= itsPolSel_.size()) return 0;
    else                           return itsPolSel_[index];
}


const String PlotMSPolSelection::DEFAULT_POLSELSTR = "I";
*/

/*
//////////////////////////
// AXISUNIT DEFINITIONS //
//////////////////////////
 
AxisUnit::AxisUnit()
{
  quant2u["amp"]          = FluxDensity;
  quant2u["azimuth"]          = Angle;
  quant2u["baseline"]         = Dimensionless;
  quant2u["channel"]          = Dimensionless;
  quant2u["corr"]         = Dimensionless;
  quant2u["elevation"]        = Angle;
  quant2u["frequency"]        = Freq;
  quant2u["hourangle"]        = Angle;
  quant2u["imag"]         = FluxDensity;
  quant2u["parallacticangle"] = Angle;
  quant2u["phase"]        = Angle;
  quant2u["real"]         = FluxDensity;
  quant2u["time"]         = Time;  
  quant2u["u"]            = Wavelengths;
  quant2u["uvdist"]       = Wavelengths;
  quant2u["v"]            = Wavelengths;
  quant2u["velocity"]         = Velocity;
  quant2u["w"]            = Wavelengths;
  quant2u["weight"]       = Dimensionless;  // May not be right.
}
  
AxisUnitEnum AxisUnit::operator[](const String& quant)
{
  S2UMap::iterator it = quant2u.find(quant);
  
  if(it == quant2u.end())
    throw AxisUnitException(quant);
  
  return it->second;  
}

// You want the AxisUnit, you go through this.  Call like AxisUnit::lookup().['time'].
AxisUnit& AxisUnit::lookup()
{
  static AxisUnit au;  // THE instantiation of AxisUnit.
  return au;
}
 */

}

