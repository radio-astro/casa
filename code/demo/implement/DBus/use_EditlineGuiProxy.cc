#include <string>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include "EditlineGuiProxy.h"

using casa::EditlineGuiProxy;

int main ( int argc, char *argv[ ] ) {
    char buf[2048];
    struct timeval tv = {0,0};
    EditlineGuiProxy *el = casa::dbus::launch<EditlineGuiProxy>( );
    if ( el == 0 ) { exit(1); }
    std::string v = el->get( );
    for ( int i=0; i < 10; ++i ) {
	gettimeofday( &tv, 0 );
	strftime( buf, 2048, "%F %T", localtime(&tv.tv_sec) );
	el->set( buf );
	sleep(1);
    }
}
