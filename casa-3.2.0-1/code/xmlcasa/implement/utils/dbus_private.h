// The Private declarations of the dbus component.

// See documentation for dbus::asyncCallMethod().
static bool asyncCallMethod_(const string& objectName, const string& methodName,
        const record& parameters);


// The Public declarations of the dbus component.
public:
    
// "From" name to use when sending CASA DBus messages.
static const String FROM_NAME;
