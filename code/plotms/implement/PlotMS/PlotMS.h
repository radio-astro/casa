//# PlotMS.h: Main controller for plotms.
//# Copyright (C) 2008
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
#ifndef PLOTMS_H_
#define PLOTMS_H_

#include <plotms/PlotMS/PlotMSParameters.h>
#include <plotms/Plots/PlotMSPlotManager.h>

namespace casa {

// Version definitions.
// <group>
#define PLOTMS_VERSION 0x002250;
#define PLOTMS_VERSION_STR "2.25";
// </group>


//# Forward declarations.
class PlotMSDBusApp;
class PlotMSPlotter;


// Controller class for plotms.  Handles interactions between the UI and plots.
class PlotMS : public PlotMSParametersWatcher {
public:    
    // Default constructor that uses default options.  If connectToDBus is
    // true, then the application registers itself with CASA's DBus server
    // using the PlotMSDBusApp::dbusName() with the current process ID.
    PlotMS(bool connectToDBus = false);
    
    // Constructor which takes the given parameters.  If connectToDBus is true,
    // then the application registers itself with CASA's DBus server using the
    // PlotMSDBusApp::dbusName() with the current process ID.
    PlotMS(const PlotMSParameters& params, bool connectToDBus = false);
    
    // Destructor
    ~PlotMS();
  
    
    // Plotter Methods //
    
    // Returns the PlotMSPlotter associated with this PlotMS. 
    PlotMSPlotter* getPlotter();
    
    // See PlotMSPlotter::showGUI().
    void showGUI(bool show = true);
    
    // See PlotMSPlotter::guiShown().
    bool guiShown() const;
    
    // See PlotMSPlotter::execLoop().
    int execLoop();
    
    // See PlotMSPlotter::showAndExec().
    int showAndExec(bool show= true);
    
    // See PlotMSPlotter::close().
    void close();
    
    // See PlotMSPlotter::showError().
    // <group>
    void showError(const String& message, const String& title = "PlotMS Error",
            bool isWarning = false);
    void showWarning(const String& message,
            const String& title = "PlotMS Warning");
    // </group>
    
    // See PlotMSPlotter::showMessage().
    void showMessage(const String& message,
            const String& title = "PlotMS Message");
    
    
    // Parameter Methods //
    
    // Gets/Sets the parameters for this PlotMS.
    // <group>
    PlotMSParameters& getParameters();
    void setParameters(const PlotMSParameters& params);
    // </group>
    
    // Implements PlotMSParametersWatcher::parametersHaveChanged().
    void parametersHaveChanged(const PlotMSWatchedParameters& params,
                int updateFlag);
    
    
    // Logger Methods //
    
    // Gets the logger associated with this PlotMS.
    PlotLoggerPtr getLogger();
    
    
    // Plot Management Methods //
    
    // Returns the PlotMSPlotManager associated with this PlotMS.
    PlotMSPlotManager& getPlotManager();
    
    // See PlotMSPlotManager::addSinglePlot().
    PlotMSSinglePlot* addSinglePlot(const PlotMSPlotParameters* p = NULL);
    
private:
    // Plotter GUI.
    PlotMSPlotter* itsPlotter_;
    
    // Current parameters.
    PlotMSParameters itsParameters_;
    
    // Logger.
    PlotLoggerPtr itsLogger_;
    
    // Plot manager.
    PlotMSPlotManager itsPlotManager_;
    
    // DBus application, or NULL if one is not needed.
    PlotMSDBusApp* itsDBus_;

    
    // Initializes a new PlotMS object, to be called from constructor.
    void initialize(bool connectToDBus);    
    
    // Disable copy constructor and operator for now.
    // <group>
    PlotMS(const PlotMS& copy);
    PlotMS& operator=(const PlotMS& copy);
    // </group>
    
    // Old stuff:
    /*
public:
    // Open the PlotMS object from an input MeasurementSet.
    // Optional compression of the calibration columns (MODEL_DATA,
    // CORRECTED_DATA and IMAGING_WEIGHT) is supported.
    bool open(MeasurementSet& inputMS, Bool compress=true);
    
    void close(const Bool apply);

  // Main plot function.
  Bool plot(const String& x, const String& y);

  // Set uv-data selection criteria
  void setdata(const String& mode="none", 
	       const Int& nchan=1,
	       const Int& start=0, 
	       const Int& step=1,
	       const MRadialVelocity& mStart=MRadialVelocity(),
	       const MRadialVelocity& mStep=MRadialVelocity(),
	       const String& msSelect="");

  // Set uv-data selection via MSSelection
  void selectvis(const PlotMSSelection& selection);

  void selectvis(const String&		time	 = "",
		 const String&		spw	 = "",
		 const String&		scan	 = "",
		 const String&		field	 = "",
		 const String&		baseline = "",
		 const String&		uvrange	 = "",
		 const String&		chanmode = "none",
		 const Int&		nchan	 = 1,
		 const Int&		start	 = 0, 
		 const Int&		step	 = 1,
		 const MRadialVelocity& mStart	 = MRadialVelocity(),
		 const MRadialVelocity& mStep	 = MRadialVelocity(),
		 const String& 		msSelect = "");

  // Returns the number of points that would be plotted with the current setup.
  uInt npoints();
  
  PlotMSParameters itsPlotOptions;
  
  // Iterate through ms and set (x, y)vals using (x, y)calculator.
  Bool calcpoints();

  Bool add_flag();  
  Bool remove_flag();
  Bool undo_flag();
  // TBD:   Bool unset_flags(const String& type);  // by type?
  
  // Apply all set flags
  void apply_flagset();

  // Returns the number of unapplied flagsets.
  uInt flag_state() const { return flagset_p.nelements(); }

  // Returns whether or not any change was made.
  // Side-effect: destroys any existing itsPlotMSPlotter (setting it to NULL)
  //              if a change is made.
  Bool change_nrows_ncols(const uInt newnrows, const uInt newncols);

  //// Method to update MS HISTORY Table...this should really be done by
  //// flagging, right?
  //void writeHistory(LogIO& os, Bool cliCommand=False);
 
  // Returns True if the PlotMS object is in a valid state.
  Bool ok();
 private:
  // Copy constructor and assignment operator (disabled - don't define them).
  PlotMS(const PlotMS&);
  PlotMS& operator=(const PlotMS&);

  // Input MeasurementSet and derived selected MeasurementSet
  String		 msname_p;
  MeasurementSet	*ms_p;
  MeasurementSet	*mssel_p;

  // VisSet derived from the input MeasurementSet
  VisSet *vs_p;

  // MeasurementSet selection parameters
  String		dataMode_p;
  Int 			dataNchan_p, dataStart_p, dataStep_p;
  MRadialVelocity	mDataStart_p, mDataStep_p;

  // (xval, yval)s is what gets plotted.
  Vector<double> xvals;
  Vector<double> yvals;
  
  // true if xvals & yvals are ready for plotting, false otherwise.
  Bool valid_vals;

  uInt binwidth;  // Have to get this.

  uInt npols;
  Vector<Int> pol_selection;
  Bool set_pol_selection(const String& polselstr);

  // Functions for calculating xvals or yvals from a VisBuffer.
  const Vector<double>& pc_time(const VisBuffer& vb, Vector<double>& d_v);
  const Vector<double>& pc_uvdist(const VisBuffer& vb, Vector<double>& d_v);
  const Vector<double>& pc_channel(const VisBuffer& vb, Vector<double>& d_v);
  const Vector<double>& pc_corr(const VisBuffer& vb, Vector<double>& d_v);
  const Vector<double>& pc_frequency(const VisBuffer& vb, Vector<double>& d_v);
  const Vector<double>& pc_vel_radio(const VisBuffer& vb, Vector<double>& d_v);
  const Vector<double>& pc_vel_optical(const VisBuffer& vb, Vector<double>& d_v);
  const Vector<double>& pc_vel_relativistic(const VisBuffer& vb, Vector<double>& d_v);
  const Vector<double>& pc_u(const VisBuffer& vb, Vector<double>& d_v);
  const Vector<double>& pc_v(const VisBuffer& vb, Vector<double>& d_v);
  const Vector<double>& pc_w(const VisBuffer& vb, Vector<double>& d_v);
  const Vector<double>& pc_azimuth(const VisBuffer& vb, Vector<double>& d_v);
  const Vector<double>& pc_elevation(const VisBuffer& vb, Vector<double>& d_v);
  const Vector<double>& pc_baseline(const VisBuffer& vb, Vector<double>& d_v);
  const Vector<double>& pc_hourangle(const VisBuffer& vb, Vector<double>& d_v);
  const Vector<double>& pc_parallacticangle(const VisBuffer& vb, Vector<double>& d_v);
  const Vector<double>& pc_amp(const VisBuffer& vb, Vector<double>& d_v);
  const Vector<double>& pc_phase(const VisBuffer& vb, Vector<double>& d_v);
  const Vector<double>& pc_real(const VisBuffer& vb, Vector<double>& d_v);
  const Vector<double>& pc_imag(const VisBuffer& vb, Vector<double>& d_v);
  const Vector<double>& pc_weight(const VisBuffer& vb, Vector<double>& d_v);
  //PlotCalcFunc pc_x;		// antenna positions, like plotants

  // Pointers to specific PlotCalcFuncs for calculating xvals and yvals,
  // respectively.
  // PlotCalcFuncPtr is a type for pointers to functions that take
  // (VisBuffer, Vector<Double>&) and return Vector<double>&.
  //typedef Vector<double> (*PlotCalcFunc)(const VisBuffer&, const int, const int);
  typedef const Vector<double>& (PlotMS::*PlotCalcFuncPtr)(const VisBuffer&,
							Vector<double>& d_v);

  std::map<String, PlotCalcFuncPtr> string2calculator;

  PlotCalcFuncPtr xcalculator;
  PlotCalcFuncPtr ycalculator;

  AxisUnit& axis_unit;
  
  String old_xaxis;
  String old_yaxis;
  Bool   have_plotted;
  
  uInt niterations;
  
  // The actual plotter.
  PlotMSPlotter* itsPlotMSPlotter;

  // What kind of visibilities to use:
  // data (raw), corrected, model, or residual (corrected - model).
  enum DataColumnEnum {dcResidual, dcCorrected, dcModel, dcData};
  DataColumnEnum itsDataColumn;

  // The polarization basis of the visibilities.
  enum PolBasisEnum {pbUnrecognized, pbStokes, pbCircular, pbLinear};
  PolBasisEnum pol_basis_guess(const VisBuffer& vb);

  Vector<FlagSet> flagset_p;

  std::pair<String, String> parseInputs(const String& xaxis, const String& yaxis);

  const Matrix<CStokesVector> get_vis_matrix(const VisBuffer& vb);

  void cleanup_flagset(const Bool apply);

  // Log functions and variables
  LogIO sink_p;
  LogIO& logSink();

  // Time functions and variables
  String timerString();
  Timer timer_p;

  void joinVectors(Vector<double>& u, const Vector<double>& v)
  {
    uInt usize = u.nelements();
    uInt newsize = usize + v.nelements();
    
    u.resize(newsize);
    //u(Slice(usize, newsize - 1, 1)) = v;
    for(unsigned int i = 0; i < v.size(); i++) u[usize + i] = v[i];
  }
  
  // Select on channel using MSSelection
  void selectChannel(const String& spw);

  // Select on channel in the VisSet
  void selectChannel(const String& mode, 
		     const Int& nchan, const Int& start, const Int& step,
		     const MRadialVelocity& mStart,
		     const MRadialVelocity& mStep);
  
  Vector<Int> getAntIdx(const String& antenna);

  // Interpret field indices (MSSelection)
  Vector<Int> getFieldIdx(const String& fields);

  // Interpret spw indices (MSSelection)
  Vector<Int> getSpwIdx(const String& spws);

  // Interpret spw indices (MSSelection)
  Matrix<Int> getChanIdx(const String& spws);
  
  Bool applyPlotOptions();

  unsigned int (PlotMS::*rownum)(const uInt) const;
  unsigned int (PlotMS::*colnum)(const uInt) const;
  
  // Provide a shortcut for the common case of only one visible plot at a time.
  unsigned int panelnum_single(const uInt iteration) const;
  
  unsigned int rownum_horizontal(const uInt iteration) const;
  unsigned int rownum_vertical(const uInt iteration) const;
  unsigned int colnum_horizontal(const uInt iteration) const;
  unsigned int colnum_vertical(const uInt iteration) const;
  */
};

/*
// Temporary class for polarization selection, until I understand it better.
// (Not currently used.)
class PlotMSPolSelection {
public:
    PlotMSPolSelection(const String& polselstr = DEFAULT_POLSELSTR);
    
    ~PlotMSPolSelection();
    
    void setPolSelection(const String& polselstr);
    
    const String& polselstr() const;    
    unsigned int npols() const;    
    int pol_selection(unsigned int index) const;
    
    int operator()(unsigned int index) const { return pol_selection(index); }
    
private:
    String itsPolSelStr_;
    vector<int> itsPolSel_;
    
    static const String DEFAULT_POLSELSTR;
};
 */

// Rob's AxisUnit stuff, not currently used.
/*
enum AxisUnitEnum {Angle,         // Not Dimensionless because it can
                              // include degrees, mas, radians, etc.
           Dimensionless,     // Things that you'd have to stretch to
                              // tack a unit onto.
           FluxDensity,       // (m, u)Jy
           Freq,          // VisSet.h typedefs Frequency to Double.
           Intensity,         // Jy/beam, Jy/sq. arcsec, etc.
           Time,
           Velocity,
                   Wavelengths};      // (u, v, w) distances.

class AxisUnit
{
public:
  AxisUnitEnum operator[](const String& quant);

  // You want the AxisUnit, you go through this.  Call like AxisUnit::lookup().['time'].
  static AxisUnit& lookup();
  
private:
  // Default constructor.  Because it is private, instantiation of AxisUnits is suppressed.
  AxisUnit();

  // There can only be one AxisUnit, so there is no copy constructor or
  // assignment operator.

  // Destructor
  ~AxisUnit() {}

  typedef std::map<String, AxisUnitEnum> S2UMap;
  S2UMap quant2u;
};
  
class AxisUnitException: public AipsError {
public:
  //
  // Creates an AxisUnitException and initializes the error message from
  // the parameter
  // <group>
  AxisUnitException(const Char *str) : AipsError(str, AipsError::INVALID_ARGUMENT) {}
  AxisUnitException(const String &str) : AipsError(str,
                           AipsError::INVALID_ARGUMENT) {}
  
  // </group>

  // Destructor which does nothing.
  ~AxisUnitException() throw() {}
};
 */

}

#endif /* PLOTMS_H_ */
