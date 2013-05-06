#include <casadbus/synthesis/ImagerControl.h>
#include <casadbus/session/DBusSession.h>
#include <casadbus/utilities/BusAccess.h>
#include <casadbus/utilities/Conversion.h>

namespace casa {
    ImagerControl::ImagerControl( const std::string &connection_name_, const std::string &object_name_ )
#ifdef INTERACTIVE_ITERATION
			: DBus::ObjectProxy( casa::DBusSession::instance().connection( ), object_name_.c_str(), connection_name_.c_str( ) )
#endif
	{ }

	ImagerControl::~ImagerControl( ) {  fprintf( stderr, "\t\tImagerControl::~ImagerControl( )\n" ); }

}
