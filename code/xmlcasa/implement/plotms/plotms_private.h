// The Private declarations of the PlotMS component.

// Friend class declarations.
friend class plotms_watcher;


// Simple helper class to implement PlotMSParametersWatcher and
// PlotMSPlotManagerWatcher and redirect the calls to plotms class.
class plotms_watcher : public PlotMSParametersWatcher,
                       public PlotMSPlotManagerWatcher {
public:
    // Constructor which takes the plotms parent object.
    plotms_watcher(plotms* parent) : itsParent_(parent) { }
    
    // Destructor.
    ~plotms_watcher() { }
    
    // Implements PlotMSParametersWatcher::parametersHaveChanged().
    void parametersHaveChanged(const PlotMSWatchedParameters& params,
            int updateFlag, bool redrawRequired) {
        itsParent_->parametersHaveChanged(params, updateFlag, redrawRequired);}
    
    // Implements PlotMSPlotManagerWatcher::plotsChanged().
    void plotsChanged(const PlotMSPlotManager& manager) {
        itsParent_->plotsChanged(manager); }
    
private:
    // Parent plotms object.
    plotms* itsParent_;
};


// Currently running PlotMS, or NULL for none.
PlotMS* itsCurrentPlotMS_;

// PlotMS watcher object.
plotms_watcher itsWatcher_;

// Set PlotMS parameters that haven't yet been transferred to the current
// PlotMS.
PlotMSParameters itsParameters_;

// Set PlotMSSinglePlot parameters that haven't yet been transfered to the
// current PlotMS.
vector<PlotMSSinglePlotParameters> itsPlotParameters_;


// Called by plotms_watcher whenever any watched plot parameters changed.
void parametersHaveChanged(const PlotMSWatchedParameters& params,
        int updateFlag, bool redrawRequired);

// Called by plotms_watcher whenever any watched plots changed.
void plotsChanged(const PlotMSPlotManager& manager);

// Adjusts the given plot index to be an acceptable, and returns whether
// the parameters were resized or not.
bool plotParameters(int& plotIndex) const;

// Helper for setting MS selection.
void setPlotMSSelection(const PlotMSSelection& selection,
        bool updateImmediately, int plotIndex);

// Helper for setting MS averaging.
void setPlotMSAveraging(const PlotMSAveraging& averaging,
        bool updateImmediately, int plotIndex);

// Helper for setting plot axes.
void setPlotAxes(PMS::Axis xAxis, PMS::Axis yAxis, PMS::DataColumn xDataColumn,
        PMS::DataColumn yDataColumn, bool useX, bool useY,
        bool updateImmediately, int plotIndex);
