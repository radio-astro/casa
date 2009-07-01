// The Private declarations of the PlotMS component.

// Static //

// Constants for how long to wait for the casapy application to launch, in
// microseconds.
// <group>
static const unsigned int LAUNCH_WAIT_INTERVAL_US;
static const unsigned int LAUNCH_TOTAL_WAIT_US;
// </group>

// Implementation of casapy_watcher for use with plotms component.
class plotms_watcher : public CasapyWatcher {
public:
    // Constructor which takes parent.
    plotms_watcher(plotms& pl) : p(pl) { CasapyWatcher::registerWatcher(this); }
    
    // Destructor.
    ~plotms_watcher() { }
    
    // Overrides casapy_watcher::logChanged().
    // <group>
    void logChanged(const String& sinkLocation) {
        p.setLogFilename(sinkLocation); }
    void logChanged(LogMessage::Priority filterPriority) {
        p.setLogFilter(LogMessage::toString(filterPriority).c_str()); }
    // </group>
    
    // Overrides casapy_watcher::casapyClosing().
    void casapyClosing() { p.closeApp(); }
    
private:
    // Plotms parent.
    plotms& p;
};


// Non-Static //

// Casapy watcher.
plotms_watcher itsWatcher_;

// DBus name of the plotms application we're communicating with.
String itsDBusName_;

// Log parameters that are set before the application is launched.
// <group>
String itsLogFilename_, itsLogFilter_;
// </group>


// Launches the dbus plotms application IF it is not already launched.
void launchApp();

// Closes the launched dbus plotms application if needed.
void closeApp();

// Helper method for calling an async method.
void callAsync(const String& methodName);

// Helper method for setting the MS selection.
void setPlotMSSelection_(const PlotMSSelection& selection,
        const bool updateImmediately, const int plotIndex);

// Helper method for setting the MS averaging.
void setPlotMSAveraging_(const PlotMSAveraging& averaging,
        const bool updateImmediately, const int plotIndex);

// Helper method for setting the flag extension.
void setFlagging_(const PlotMSFlagging& flagging);
