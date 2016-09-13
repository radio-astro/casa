#include <casadbus/synthesis/ImagerControl.h>
#include <casadbus/session/DBusSession.h>
#include <casadbus/utilities/BusAccess.h>
#include <casadbus/utilities/Conversion.h>
#define DEBUG 1
namespace casa {
    ImagerControl::ImagerControl( const std::string &connection_name_, const std::string &object_path_ )
#ifdef INTERACTIVE_ITERATION
			: DBus::ObjectProxy( casa::DBusSession::instance().connection( ), object_path_.c_str(), connection_name_.c_str( ) )
#endif
	{
#if DEBUG
		cout << "<ImagerControl::ImagerControl>: connecting to " << object_path_ << " in service " << connection_name_ << endl;
#endif
	}

	ImagerControl::~ImagerControl( ) {  fprintf( stderr, "\t\tImagerControl::~ImagerControl( )\n" ); }

}
