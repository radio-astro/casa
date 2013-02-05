#include <casadbus/synthesis/ImagerControl.h>
#include <casadbus/session/DBusSession.h>
#include <casadbus/utilities/BusAccess.h>
#include <casadbus/utilities/Conversion.h>

namespace casa {
    ImagerControl::ImagerControl( const std::string &name )
#ifdef INTERACTIVE_ITERATION
			: DBus::ObjectProxy( DBusSession::instance().connection( ), dbus::object(name).c_str(), dbus::path(name).c_str() )
#endif
	{ }

	ImagerControl::~ImagerControl( ) {  fprintf( stderr, "\t\tImagerControl::~ImagerControl( )\n" ); }

}
